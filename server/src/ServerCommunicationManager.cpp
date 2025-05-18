#include "ServerCommunicationManager.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>
#include <FileManager.hpp>

std::mutex access_devices;
std::mutex access_files;
std::mutex access_download;

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

void ServerCommunicationManager::run_client_session(int socket_cmd, std::string username, std::shared_ptr<ClientsDevices> devices, std::string user_dir_path) {
    this->socket_cmd = socket_cmd;
    this->devices = devices;
    this->username = username;
    this->user_dir_path = user_dir_path;
    bool suicide;

    if(!connect_socket_to_client(&socket_upload, &port_upload)) {
        std::cout << "Failed to connect upload socket" << std::endl;
        close_sockets();
        return;
    }
    
    if(!connect_socket_to_client(&socket_download, &port_download)) {
        std::cout << "Failed to connect download socket" << std::endl;
        close_sockets();
        return;
    }

    this->session_name = "[" + this->username + "](" + std::to_string(socket_download) + ") -> ";

    access_devices.lock();
    {
        suicide = !devices->add_client_socket(username, socket_download);
    }
    access_devices.unlock();

    if (suicide) {
        std::string error_msg = "USER_MAX_CONNECTIONS_REACHED";
        Packet errorPacket(static_cast<uint16_t>(Packet::Type::ERROR), 0, 0, error_msg.size(), error_msg);
        
        try {
            errorPacket.send(socket_cmd);
            close(socket_download);
            close(socket_upload);
            return;
        } catch (const std::exception& e) {
            std::cout << "Failed to send error packet: " << e.what() << std::endl;
            close(socket_download);
            close(socket_upload);
            return;
        }
    }

    std::thread thread_cmd(&ServerCommunicationManager::handle_client_cmd, this);
    std::thread thread_sync_client(&ServerCommunicationManager::handle_client_update, this);
    
    thread_cmd.join();
    thread_sync_client.join();
}

// ========================================= //
// ================ THREADS ================ //
// ========================================= //

void ServerCommunicationManager::handle_client_cmd() {
    while (socket_cmd > 0) {
        Packet packet = Packet::receive(socket_cmd);
        std::cout << session_name << "Packet received: " << packet.to_string() << std::endl;

        std::vector<string> tokens = split_command(packet.payload);
        std::string command = tokens[0];
        
        if (command == "upload") {
            std::cout << session_name << "{CMD} client upload" << std::endl;
        }
        else if (command == "download") {
            std::cout << session_name << "{CMD} client download" << std::endl;
            handle_client_download(tokens[1]);
        } else if (command == "delete") {
            std::cout << session_name << "{CMD} client delete" << std::endl;
            handle_client_delete(tokens[1]);
        } else if (command == "list_server") {
            handle_list_server();
        } else if (packet.payload == "exit") {
            handle_exit();
        } else if (packet.payload == "get_sync_dir") {
            handle_get_sync_dir();        
        } else {
            std::cout << session_name << "Unknown command: " << packet.payload << std::endl;
        }
    }
}

void ServerCommunicationManager::handle_client_update() {
    while (socket_upload != -1) {
        try {
            std::cout << session_name << "Receiving client pushes" << std::endl;
            Packet meta_packet = Packet::receive(socket_upload);

            std::cout << session_name << "Received meta_packet from client: " << meta_packet.payload << std::endl;

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                continue;
            }

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::DELETE)) {
                handle_client_delete(meta_packet.payload);
                continue;
            }

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::DATA)) {
                handle_client_upload(meta_packet.payload, meta_packet.total_size);
                continue;
            }

            throw std::runtime_error(
                "Unexpected packet type " + std::to_string(meta_packet.type) + 
                " received from client (expected DATA, DELETE, or ERROR)"
            );

        } catch (const std::runtime_error& e) {
            std::cout << session_name << "Client update failed: " << e.what() << std::endl;
            return;
        }
    }
}

// ========================================= //
// ================ PRIVATE ================ //
// ========================================= //

void ServerCommunicationManager::close_sockets() {
    if (socket_cmd > 0) close(socket_cmd);
    if (socket_download > 0) close(socket_download);
    if (socket_upload > 0) close(socket_upload);
}

bool ServerCommunicationManager::connect_socket_to_client(int *sockfd, int *port) {
    struct sockaddr_in serv_addr;
    socklen_t len = sizeof(serv_addr);

    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = 0;

    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "Failed to create socket" << std::endl;
        return false;
    }

    if (bind(*sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Bind error" << std::endl;
        return false;
    }

    if (listen(*sockfd, 5) < 0) {
        std::cout << "Listen error" << std::endl;
        return false;
    }

    if (getsockname(*sockfd, (struct sockaddr *)&serv_addr, &len) == -1) {
        std::cout << "getsockname error" << std::endl;
        return false;
    }

    std::cout << "Socket listening on IP: " << inet_ntoa(serv_addr.sin_addr) << std::endl;
    
    *port = ntohs(serv_addr.sin_port);
    std::string port_str = std::to_string(*port);

    if ((write(socket_cmd, port_str.c_str(), port_str.length())) < 0) {
        std::cout << "Failed to send port to client" << std::endl;
        return false;
    }

    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);
    if((*sockfd = accept(*sockfd, (struct sockaddr*) &client_address, &client_address_len)) < 0){
        std::cout << "Failed to accept client" << std::endl;
        return false;
    }
    return true;
}

void ServerCommunicationManager::handle_client_download(const std::string filename) {
    access_download.lock();
    {
        if(!Packet::send_file(socket_download, user_dir_path / filename))
            Packet::send_error(socket_download);
    }
    access_download.unlock();
}

void ServerCommunicationManager::handle_client_upload(const std::string filename, uint32_t total_packets) {
    int socket_download_other_device;
    std::cout << session_name << "handle_client_upload " << filename << std::endl;
    access_files.lock();
    {
        Packet::receive_file(socket_upload, filename, user_dir_path, total_packets);
    }
    access_files.unlock();
    access_devices.lock();
    {
        socket_download_other_device = devices->get_other_device_socket(username, socket_download);
    }
    access_devices.unlock();

    std::cout << session_name << "get_other_device_socket " << socket_download_other_device << std::endl;
    if(socket_download_other_device == -1)
        return;
    access_download.lock();
    {
        if(!Packet::send_file(socket_download_other_device, user_dir_path / filename))
            Packet::send_error(socket_download_other_device);
    }
    access_download.unlock();
    std::cout << session_name << "repropagate " << filename << std::endl;
}

void ServerCommunicationManager::handle_client_delete(const std::string filename) {
    int socket_download_other_device;
    std::cout << session_name << "handle_client_delete " << filename << std::endl;
    access_files.lock();
    {
        FileManager::delete_file(user_dir_path / filename);
    }
    access_files.unlock();
    access_devices.lock();
    {
        socket_download_other_device = devices->get_other_device_socket(username, socket_download);
    }
    access_devices.unlock();
    std::cout << session_name << "get_other_device_socket " << socket_download_other_device << std::endl;
    if(socket_download_other_device == -1)
        return;
    access_download.lock();
    {
        Packet delete_packet(static_cast<uint16_t>(Packet::Type::DELETE), 0, 0, filename.size(), filename);
        delete_packet.send(socket_download_other_device);
    }
    access_download.unlock();
    std::cout << session_name << "repropagate delete " << filename << std::endl;
}

void ServerCommunicationManager::handle_get_sync_dir(){
    access_download.lock();
    {
        if(!Packet::send_multiple_files(socket_download, username))
            std::cout << "Failed to send multiple files" << std::endl;
    }
    access_download.unlock();
}

void ServerCommunicationManager::handle_exit() {
    close_sockets();
    access_devices.lock();
    {
        devices->remove_client_socket(username, socket_download);
    }
    access_devices.unlock();
}

void ServerCommunicationManager::handle_list_server() {
    std::cout << session_name << "handle_list_server" << std::endl;
    std::string file_list = FileManager::get_formatted_file_list(user_dir_path);
    Packet response_packet(static_cast<uint16_t>(Packet::Type::DATA), 0, 1, file_list.size(), file_list);
    response_packet.send(socket_download);
}

vector<string> ServerCommunicationManager::split_command(const string &command) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(command);
    
    while (tokenStream >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}
