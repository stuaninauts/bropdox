#include <Communicator.hpp>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>        // Para fcntl, F_GETFL, F_SETFL, O_NONBLOCK
#include <Packet.hpp>
#include <filesystem>
#include <sys/inotify.h>
#include <sys/select.h>   // Para select, fd_set, FD_ZERO, FD_SET
#include <limits.h>
#include <sys/stat.h>
#include <FileManager.hpp>

std::mutex access_ignored_files;

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

bool Communicator::connect_to_server(int initial_socket_new_alpha) {
    try {
        if (initial_socket_new_alpha == -1) {
            socket_cmd = Network::connect_socket_ipv4(server_ip, port_cmd);
            if (socket_cmd == -1) {
                close_sockets();
                return false;
            }

            if (!send_initial_information()) {
                close_sockets();
                return false;
            }
        } else {
            socket_cmd = initial_socket_new_alpha;
            
        }

        if ((socket_upload = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Error creating upload socket" << std::endl;
            close_sockets();
            return false;
        }

        if ((socket_download = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Error creating download socket" << std::endl;
            close_sockets();
            return false;
        }

        if (!connect_socket_to_server(socket_upload, &port_upload)){
            std::cerr << "Error connecting upload socket" << std::endl;
            close_sockets();
            return false;
        }

        if (!connect_socket_to_server(socket_download, &port_download)){
            std::cerr << "Error connecting download socket" << std::endl;
            close_sockets();
            return false;
        }

        if (!confirm_connection()) {
            close_sockets();
            return false;
        }

        std::cout << "--- All sockets connected successfully ---" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Exception in connect_to_server: " << e.what() << std::endl;
        close_sockets();
        return false;
    }
}

void Communicator::watch_directory() {
    bool ignore = false;
    int inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        std::cerr << "Error initializing inotify\n";
        return;
    }
    
    int flags = fcntl(inotify_fd, F_GETFL, 0);
    fcntl(inotify_fd, F_SETFL, flags | O_NONBLOCK);
    
    int wd = inotify_add_watch(inotify_fd, sync_dir_path.c_str(), IN_DELETE | IN_CLOSE_WRITE | IN_MOVED_FROM | IN_MOVED_TO);
    if (wd < 0) {
        std::cerr << "Error adding watch for " << sync_dir_path << ": " << strerror(errno) << "\n";
        close(inotify_fd);
        return;
    }
    
    const size_t EVENT_SIZE = sizeof(struct inotify_event);
    const size_t BUF_LEN = 1024 * (EVENT_SIZE + NAME_MAX + 1);
    char buffer[BUF_LEN];
    
    while (socket_upload != -1 && running_ptr && running_ptr->load()) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(inotify_fd, &rfds);
        
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 2000; // 2ms
        
        int retval = select(inotify_fd + 1, &rfds, NULL, NULL, &tv);
        if (retval == -1) {
            std::cerr << "[ INOTIFY ] Error in select(): " << strerror(errno) << std::endl;
            break;
        } else if (retval == 0) {
            continue;
        }
        int length = read(inotify_fd, buffer, BUF_LEN);
        
        if (length < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            std::cerr << "[ INOTIFY ] Error reading inotify: " << strerror(errno) << std::endl;
            break;
        }
        
        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];
            if (!event->len) continue;

            if (event->mask & IN_DELETE_SELF) {
                close(inotify_fd);
            }

            access_ignored_files.lock();
            {
                auto it = ignored_files.find(event->name);
                if(it != ignored_files.end()) {
                    std::cout << "Removed file from ignored_files: " << *it << std::endl;
                    ignored_files.erase(it);
                    ignore = true;
                } else {
                    ignore = false;
                }
            }
            access_ignored_files.unlock();

            i += EVENT_SIZE + event->len;

            if (ignore) continue;

            if (event->mask & IN_MOVED_FROM || event->mask & IN_DELETE) {
                std::cout << "[INOTIFY] SEND_DELETE: " << event->name << std::endl;
                Packet packet(static_cast<uint16_t>(Packet::Type::DELETE), 0, 0, strlen(event->name), event->name);
                packet.send(socket_upload);
            }
            if (event->mask & IN_CLOSE_WRITE || event->mask & IN_MOVED_TO) {
                std::cout << "[INOTIFY] SEND_FILE: " << event->name << std::endl;
                Packet::send_file(socket_upload, sync_dir_path / event->name);
            }
        }
    }
    
    inotify_rm_watch(inotify_fd, wd);
    close(inotify_fd);
}

void Communicator::handle_server_update() {
    while (socket_upload != -1 && running_ptr && running_ptr->load()) {
        try {
            std::cout << "Receiving server pushes" << std::endl;
            Packet meta_packet = Packet::receive(socket_download);

            std::cout << "Received meta_packet from client: " << meta_packet.payload << std::endl;

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                continue;
            }

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::DELETE)) {
                handle_server_delete(meta_packet.payload);
                continue;
            }

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::DATA)) {
                handle_server_upload(meta_packet.payload, meta_packet.total_size);
                continue;
            }

            throw std::runtime_error(
                "Unexpected packet type " + std::to_string(meta_packet.type) + 
                " received from client (expected DATA, DELETE, or ERROR)"
            );

        } catch (const std::runtime_error& e) {
            std::cerr << "Session update failed: " << e.what() << std::endl;
            return;
        }
    }
}

void Communicator::get_sync_dir(){
    send_command("get_sync_dir");
    if(!Packet::receive_multiple_files(socket_download, sync_dir_path)) {
        std::cerr << "Error performing get_sync_dir" << std::endl;
    }
}

void Communicator::exit_server() {
    std::cout << "Disconnecting from server..." << std::endl;
    send_command("exit");
    close_sockets();
    exit(0);
}

void Communicator::list_server() {
    std::cout << "Listing files on server:" << std::endl;

    send_command("list_server");
    std::string file_list = "";
    Packet packet;
    do {
        packet = Packet::receive(socket_cmd);
        file_list += packet.payload;
    } while (packet.seqn + 1 < packet.total_size);
    std::cout << file_list << std::endl;
}

// ========================================= //
// ================ PRIVATE ================ //
// ========================================= //

void Communicator::handle_server_delete(const std::string filename) {
    access_ignored_files.lock();
    {
        ignored_files.insert(filename);
    }
    access_ignored_files.unlock();
    // TEST WITH DOCKER
    FileManager::delete_file(sync_dir_path / filename);
}

void Communicator::handle_server_upload(const std::string filename, uint32_t total_packets) {
    access_ignored_files.lock();
    {
        ignored_files.insert(filename);
    }
    access_ignored_files.unlock();
    // TEST WITH DOCKER
    std::string tmp = "./.tmp/";
    std::filesystem::create_directory(tmp);
    Packet::receive_file(socket_download, filename, tmp, total_packets);
    std::string tmp_filepath = tmp + filename;
    std::string filepath = sync_dir_path / filename;
    rename(tmp_filepath.c_str(), filepath.c_str());
}

void Communicator::send_command(const std::string command) {
    std::string payload = command;
    Packet command_packet(static_cast<uint16_t>(Packet::Type::CMD), 0, 1, payload.size(), payload);
    command_packet.send(socket_cmd);
}

bool Communicator::send_initial_information() {
    Packet username_packet(static_cast<uint16_t>(Packet::Type::USERNAME), 0, 0, username.length(), username.c_str());
    username_packet.send(socket_cmd);

    std::string port_str = std::to_string(port_backup);
    Packet port_packet(static_cast<uint16_t>(Packet::Type::PORT), 0, 0, port_str.length(), port_str.c_str());
    port_packet.send(socket_cmd);

    return true;
}

void Communicator::close_sockets() {
    if (socket_cmd > 0) {
        close(socket_cmd);
        socket_cmd = -1;
    }
    if (socket_download > 0) {
        close(socket_download);
        socket_download = -1;
    }
    if (socket_upload > 0) {
        close(socket_upload);
        socket_upload = -1;
    }
    std::cout << "All sockets closed" << std::endl;
}

bool Communicator::connect_socket_to_server(int sockfd, int* port) {    
    struct sockaddr_in serv_addr;

    char buffer[256];
    bzero(buffer, 256);

    if (read(socket_cmd, buffer, 255) <= 0) {
        std::cerr << "ERROR: Can't read upload port\n";
        return false;
    }

    *port = std::stoi(buffer);

    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(*port);

    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid IP address: " << server_ip << std::endl;
        return false;
    }
    std::cerr << "server ip: " << server_ip << std::endl;

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error connecting to server";
        return false;
    }

    return true;
}

bool Communicator::confirm_connection() {
    fd_set readfds;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;  // 100ms

    FD_ZERO(&readfds);
    FD_SET(socket_cmd, &readfds);

    if (select(socket_cmd + 1, &readfds, NULL, NULL, &tv) > 0) {
        // There is data to read on socket_cmd
        try {
            Packet errorCheck = Packet::receive(socket_cmd);
            if (errorCheck.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                std::cout << "Server denied connection: " << errorCheck.payload << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            // Ignore exceptions here because an error is not mandatory
        }
    }
    return true;
}

