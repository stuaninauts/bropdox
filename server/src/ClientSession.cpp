#include "ClientSession.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>
#include <FileManager.hpp>
#include <Addresses.hpp>

std::mutex access_files;
std::mutex access_download;

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

void ClientSession::connect_sockets() {
    bool suicide;

    if(!connect_socket_to_client(&socket_upload, &port_upload)) {
        std::cerr << "[ ALFA SERVER ] " << "Failed to connect upload socket" << std::endl;
        close_sockets();
        return;
    }
    
    if(!connect_socket_to_client(&socket_download, &port_download)) {
        std::cerr << "[ ALFA SERVER ] " << "Failed to connect download socket" << std::endl;
        close_sockets();
        return;
    }

    suicide = !devices->add_client(username, socket_download, client_ip, port_backup);
    
    if (suicide) {
        std::string error_msg = "USER_MAX_CONNECTIONS_REACHED";
        Packet errorPacket(static_cast<uint16_t>(Packet::Type::ERROR), 0, 0, error_msg.size(), error_msg);
        
        try {
            errorPacket.send(socket_cmd);
            close(socket_download);
            close(socket_upload);
            return;
        } catch (const std::exception& e) {
            std::cerr << "[ ALFA SERVER ] " << "Failed to send error packet: " << e.what() << std::endl;
            close(socket_download);
            close(socket_upload);
            return;
        }
    }
    
    this->session_name = "[" + this->username + "](" + std::to_string(socket_download) + ") -> ";
    betas->send_client_device(client_ip, username, port_backup);
}

void ClientSession::run() {
    std::thread thread_cmd(&ClientSession::handle_client_cmd, this);
    std::thread thread_sync_client(&ClientSession::handle_client_update, this);
    
    thread_cmd.join();
    thread_sync_client.join();
}

// ========================================= //
// ================ THREADS ================ //
// ========================================= //

void ClientSession::handle_client_cmd() {
    try {
        while (socket_cmd > 0) {
            Packet packet = Packet::receive(socket_cmd);
            std::cout << "[ ALFA SERVER ] " << session_name << "Packet received: " << packet.to_string() << std::endl;

            if (packet.payload == "list_server") {
                std::cout << "[ ALFA SERVER ] " << session_name << "{CMD} client list_server" << std::endl;
                handle_list_server();
            } else if (packet.payload == "exit") {
                std::cout << "[ ALFA SERVER ] " << session_name << "{CMD} client exit" << std::endl;
                handle_exit(); // garante cleanup
                break;         
            } else if (packet.payload == "get_sync_dir") {
                handle_get_sync_dir();        
            } else {
                std::cout << "[ ALFA SERVER ] " << session_name << "Unknown command: " << packet.payload << std::endl;
            }
        }
    } catch (const std::runtime_error& e) {
        handle_exit(); // garante cleanup em qualquer erro
    }
}

void ClientSession::handle_client_update() {
    try {
        while (socket_upload > 0) { // TODO: revisar antes tava -1
            Packet meta_packet = Packet::receive(socket_upload);

            std::cout << "[ ALFA SERVER ] " << session_name << "Received meta_packet from client: " << meta_packet.payload << std::endl; // opcional

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                std::cout << "[ ALFA SERVER ] " << session_name << "Received ERROR packet on update socket: " << meta_packet.payload << std::endl;
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
            
            // se for apenas printar erro
            // std::cout << "[ ALFA SERVER ] " << session_name << "Unexpected packet type " << meta_packet.type 
            //           << " on update socket. Payload: " << meta_packet.payload << std::endl;
            // se for erro fatal pra fechar
            throw std::runtime_error(
                "Unexpected packet type " + std::to_string(meta_packet.type) + 
                " received from client (expected DATA, DELETE, or ERROR)"
            );
        }
    } catch (const std::runtime_error& e) {
        handle_exit(); // Ensure cleanup on any error
    }
}

// ========================================= //
// ================ PRIVATE ================ //
// ========================================= //

void ClientSession::close_sockets() {
    std::cout << "[ ALFA SERVER ] " << session_name << "Closing sockets." << std::endl;
    if (socket_cmd > 0) {
        close(socket_cmd);
        socket_cmd = 0;
    }
    if (socket_download > 0) {
        close(socket_download);
        socket_download = 0;
    }
    if (socket_upload > 0) {
        close(socket_upload);
        socket_upload = 0;
    }
}

bool ClientSession::connect_socket_to_client(int *sockfd, int *port) {
    struct sockaddr_in serv_addr;
    socklen_t len = sizeof(serv_addr);

    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = 0;

    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "[ ALFA SERVER ] " << "Failed to create socket" << std::endl;
        return false;
    }

    if (bind(*sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "[ ALFA SERVER ] " << "Bind error" << std::endl;
        return false;
    }

    if (listen(*sockfd, 5) < 0) {
        std::cerr << "[ ALFA SERVER ] " << "Listen error" << std::endl;
        return false;
    }

    if (getsockname(*sockfd, (struct sockaddr *)&serv_addr, &len) == -1) {
        std::cerr << "[ ALFA SERVER ] " << "getsockname error" << std::endl;
        return false;
    }

    std::cout << "[ ALFA SERVER ] " << "Socket listening on IP: " << inet_ntoa(serv_addr.sin_addr) << std::endl;
    
    *port = ntohs(serv_addr.sin_port);
    std::string port_str = std::to_string(*port);

    if ((write(socket_cmd, port_str.c_str(), port_str.length())) < 0) {
        std::cout << "[ ALFA SERVER ] " << "Failed to send port to client" << std::endl;
        return false;
    }

    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);
    if((*sockfd = accept(*sockfd, (struct sockaddr*) &client_address, &client_address_len)) < 0){
        std::cerr << "[ ALFA SERVER ] " << "Failed to accept client" << std::endl;
        return false;
    }

    char ip_buffer[INET_ADDRSTRLEN];

    // We get the address from client_address.sin_addr.
    if (inet_ntop(AF_INET, &client_address.sin_addr, ip_buffer, sizeof(ip_buffer)) == NULL) {
        std::cerr << "[ ALFA SERVER ] " << "Failed to convert client IP address" << std::endl;
        return false;
    }

    client_ip = std::string(ip_buffer);
    std::cout << "[ ALFA SERVER ] " << "Successfully accepted connection from client with IP: " << client_ip << std::endl;
    return true;
}

void ClientSession::handle_client_download(const std::string filename) {
    access_download.lock();
    {
        if(!Packet::send_file(socket_download, user_dir_path / filename))
            Packet::send_error(socket_download);
    }
    access_download.unlock();
}

void ClientSession::handle_client_upload(const std::string filename, uint32_t total_packets) {
    int socket_download_other_device;
    std::cout << "[ ALFA SERVER ] " << session_name << "handle_client_upload " << filename << std::endl;
    access_files.lock();
    {
        Packet::receive_file(socket_upload, filename, user_dir_path, total_packets);
    }
    access_files.unlock();


    betas->send_file(user_dir_path / filename, username);
    std::cout << "Arquivo: " << user_dir_path / filename << std::endl;


    socket_download_other_device = devices->get_other_device_socket(username, socket_download);


    std::cout << "[ ALFA SERVER ] " << session_name << "get_other_device_socket " << socket_download_other_device << std::endl;
    if(socket_download_other_device == -1) {
        std::cout << "[ ALFA SERVER ] " << session_name << "No other device connected for user " << username << std::endl;
        return;
    }
    access_download.lock();
    {
        if(!Packet::send_file(socket_download_other_device, user_dir_path / filename))
            Packet::send_error(socket_download_other_device);
    }
    access_download.unlock();
    std::cout << "[ ALFA SERVER ] " << session_name << "repropagate " << filename << std::endl;
}

void ClientSession::handle_client_delete(const std::string filename) {
    int socket_download_other_device;
    std::cout << "[ ALFA SERVER ] " << session_name << "handle_client_delete " << filename << std::endl;
    access_files.lock();
    {
        FileManager::delete_file(user_dir_path / filename);
    }
    access_files.unlock();

    betas->delete_file(filename, username);

    socket_download_other_device = devices->get_other_device_socket(username, socket_download);
    
    std::cout << "[ ALFA SERVER ] " << session_name << "get_other_device_socket " << socket_download_other_device << std::endl;
    if(socket_download_other_device == -1)
        return;
    access_download.lock();
    {
        Packet delete_packet(static_cast<uint16_t>(Packet::Type::DELETE), 0, 0, filename.size(), filename);
        delete_packet.send(socket_download_other_device);
    }
    access_download.unlock();
    std::cout << "[ ALFA SERVER ] " << session_name << "repropagate delete " << filename << std::endl;
}

void ClientSession::handle_get_sync_dir(){
    access_download.lock();
    {
        if(!Packet::send_multiple_files(socket_download, user_dir_path))
            std::cerr << "[ ALFA SERVER ] " << "Failed to send multiple files" << std::endl;
    }
    access_download.unlock();
}

void ClientSession::handle_exit() {
    // TODO: revisar
    // Capture the download socket descriptor used for device registration before closing.
    // This assumes this->socket_fd holds the relevant descriptor.
    // If close_sockets() has already run, this->socket_fd might be 0.
    // It's better if remove_client could be called with the original descriptor
    // or if ClientsDevices handles already closed sockets gracefully.
    // For now, we rely on the fact that socket_fd was added.
    int download_socket_id_for_removal = this->socket_download;

    close_sockets();

    if (download_socket_id_for_removal > 0) { // apenas tenta remover se é um socket válido
        if (devices) { // Ensure devices pointer is valid
            ClientAddress removed_client = devices->remove_client(this->username, download_socket_id_for_removal);
            betas->send_removed_client_device(removed_client.ip, removed_client.username, removed_client.port);
        }
    }
}

void ClientSession::handle_list_server() {
    const size_t max_payload = 4096;

    std::cout << "[ ALFA SERVER ] " << session_name << "handle_list_server" << std::endl;

    std::string file_list = FileManager::get_formatted_file_list(user_dir_path);
    size_t total_size = file_list.size();
    int total_packets = total_size / max_payload + 1;

    for (int seqn = 0; seqn < total_packets; seqn++) {
        size_t start = seqn * max_payload;
        size_t end = std::min(start + max_payload, total_size);
        std::string payload = file_list.substr(start, end - start);

        Packet response_packet(static_cast<uint16_t>(Packet::Type::DATA), seqn, total_packets, payload.size(), payload);
        response_packet.send(socket_cmd);
    }
}