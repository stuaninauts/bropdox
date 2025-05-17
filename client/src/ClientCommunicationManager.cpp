#include <ClientCommunicationManager.hpp>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <Packet.hpp>
#include <filesystem>
#include <sys/inotify.h>
#include <limits.h>
#include <sys/stat.h>
#include <ClientFileManager.hpp>


std::mutex access_ignored_files;

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

bool ClientCommunicationManager::connect_to_server(const std::string server_ip, int port, const std::string username) {
    this->server_ip = server_ip;
    this->port_cmd = port;
    this->username = username;
    try {
        // Initialization of the main connection
        if (!connect_socket_cmd()) {
            close_sockets();
            return false;
        }

        if (!send_username()) {
            close_sockets();
            return false;
        }

        // Create upload socket
        if ((socket_upload = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cout << "Error creating upload socket";
            close_sockets();
            return false;
        }

        // Create download socket
        if ((socket_download = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cout << "Error creating download socket";
            close_sockets();
            return false;
        }

        // Connect upload socket
        if (!connect_socket_to_server(socket_upload, &port_upload)){
            std::cout << "Error connecting upload socket";
            close_sockets();
            return false;
        }

        // Connect download socket
        if (!connect_socket_to_server(socket_download, &port_download)){
            std::cout << "Error connecting download socket";
            close_sockets();
            return false;
        }

        // Error checking and connection confirmation
        if (!confirm_connection()) {
            close_sockets();
            return false;
        }

        get_sync_dir();

        return true;

    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        close_sockets(); // TODO: review
        return false;
    }
}

void ClientCommunicationManager::watch(const std::string sync_dir_path) {
    bool ignore = false;
    int inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        std::cout << "Error initializing inotify\n";
        return;
    }
    
    int wd = inotify_add_watch(inotify_fd, sync_dir_path.c_str(), IN_CREATE | IN_DELETE | IN_CLOSE_WRITE | IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF);
    if (wd < 0) {
        std::cout << "Error adding watch for " << sync_dir_path << ": " << strerror(errno) << "\n";
        close(inotify_fd);
        return;
    }
    
    const size_t EVENT_SIZE = sizeof(struct inotify_event);
    const size_t BUF_LEN = 1024 * (EVENT_SIZE + NAME_MAX + 1);
    char buffer[BUF_LEN];
    std::cout << "Watching directory: " << sync_dir_path << std::endl;
    
    while (true) {
        int length = read(inotify_fd, buffer, BUF_LEN);
        if (length < 0) {
            std::cout << "Error reading from inotify\n";
            break;
        }
        
        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];
            if (!event->len) continue;

            if (event->mask & IN_CREATE)
                std::cout << "[INOTIFY] IN_CREATE: " << event->name << std::endl;
            if (event->mask & IN_DELETE)
                std::cout << "[INOTIFY] IN_DELETE: " << event->name << std::endl;
            if (event->mask & IN_CLOSE_WRITE)
                std::cout << "[INOTIFY] IN_CLOSE_WRITE: " << event->name << std::endl;
            if (event->mask & IN_MOVED_FROM)
                std::cout << "[INOTIFY] IN_MOVED_FROM: " << event->name << std::endl;
            if (event->mask & IN_MOVED_TO)
                std::cout << "[INOTIFY] IN_MOVED_TO: " << event->name << std::endl;
            if (event->mask & IN_DELETE_SELF) {
                std::cout << "[INOTIFY] IN_DELETE_SELF: " << event->name << std::endl;
                close(inotify_fd);
            }

            access_ignored_files.lock();
            {
                auto it = ignored_files.find(event->name);
                if(ignore = it != ignored_files.end()) {
                    ignored_files.erase(it);
                    std::cout << "Removed file from ignored_files: " << *it << std::endl;
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
                Packet::send_file(socket_upload, sync_dir_path + event->name);
            }
        }
    }
    
    inotify_rm_watch(inotify_fd, wd);
    close(inotify_fd);
}

void ClientCommunicationManager::handle_server_update() {
    while (socket_upload != -1) {
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
            std::cout << "Session update failed: " << e.what() << std::endl;
            return;
        }
    }
}

void ClientCommunicationManager::get_sync_dir(){
    std::string file_path = "./sync_dir/";
    send_command("get_sync_dir");
    if(!Packet::receive_multiple_files(socket_download, file_path))
        std::cout << "Error performing get_sync_dir" << std::endl;
}

void ClientCommunicationManager::exit_server() {
    std::cout << "Disconnecting from server..." << std::endl;
    send_command("exit");
    close_sockets();
    exit(0);
}

void ClientCommunicationManager::upload_file(const std::string filepath) {
    std::string filename = std::filesystem::path(filepath).filename().string();
    send_command("upload", filename);
    // If the file cannot be sent, the client must send an error packet
    // to the server to unlock it, as it is waiting for a file.
    if (!Packet::send_file(socket_upload, filepath))
        Packet::send_error(socket_upload);
}

void ClientCommunicationManager::download_file(const std::string filename) {
    send_command("download", filename);
}

void ClientCommunicationManager::delete_file(const std::string filename) {
    send_command("delete", filename);
}

void ClientCommunicationManager::list_server() {
    std::cout << "Listing files on server:" << std::endl;
    send_command("list_server");
    Packet packet = Packet::receive(socket_cmd);
    std::cout << packet.payload << std::endl;
}

// ========================================= //
// ================ PRIVATE ================ //
// ========================================= //

void ClientCommunicationManager::handle_server_delete(const std::string filename) {
    access_ignored_files.lock();
    {
        ignored_files.insert(filename);
    }
    access_ignored_files.unlock();
    // TEST WITH DOCKER
    ClientFileManager::delete_local_file(filename);
}

void ClientCommunicationManager::handle_server_upload(const std::string filename, uint32_t total_packets) {
    access_ignored_files.lock();
    {
        ignored_files.insert(filename);
    }
    access_ignored_files.unlock();
    // TEST WITH DOCKER
    std::string tmp = "./tmp/";
    std::filesystem::create_directory(tmp);
    Packet::receive_file(socket_download, filename, tmp, total_packets);
    std::string tmp_filepath = tmp + filename;
    std::string filepath = "./sync_dir/" + filename;
    rename(tmp_filepath.c_str(), filepath.c_str());
}

void ClientCommunicationManager::send_command(const std::string command, const std::string filename) {
    std::string payload = command;
    if(filename != "") payload += " " + filename; //secret TODO ugly bad
    Packet command_packet(static_cast<uint16_t>(Packet::Type::CMD), 0, 1, payload.size(), payload);
    command_packet.send(socket_cmd);
}

bool ClientCommunicationManager::send_username() {
    int n = write(socket_cmd, username.c_str(), username.length());
    if (n < 0) {
        std::cout << "ERROR: Writing username to socket\n";
        return false;
    }
    return true;
}

bool ClientCommunicationManager::connect_socket_cmd() {
    struct hostent* server;
    struct sockaddr_in serv_addr{};

    if ((server = gethostbyname(server_ip.c_str())) == nullptr) {
        std::cout << "ERROR: No such host\n";
        return false;
    }
    
    if ((socket_cmd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "ERROR: Opening socket\n";
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_cmd);
    serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(socket_cmd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "ERROR: Connecting to server\n";
        return false;
    }

    return true;
}

void ClientCommunicationManager::close_sockets() {
    if (socket_cmd > 0) close(socket_cmd);
    if (socket_download > 0) close(socket_download);
    if (socket_upload > 0) close(socket_upload);
}

bool ClientCommunicationManager::connect_socket_to_server(int sockfd, int* port) {    
    struct sockaddr_in serv_addr;

    char buffer[256];
    bzero(buffer, 256);

    if (read(socket_cmd, buffer, 255) <= 0) {
        std::cout << "ERROR: Can't read upload port\n";
        return false;
    }

    *port = std::stoi(buffer);

    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(*port);

    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cout << "Invalid IP address: " << server_ip << std::endl;
        return false;
    }
    std::cout << "server ip: " << server_ip << std::endl;

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Error connecting to server";
        return false;
    }

    return true;
}

bool ClientCommunicationManager::confirm_connection() {
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
