#include <AlfaServer.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <FileManager.hpp>

std::mutex accept_connections;
std::mutex write_beta_sockets;

void AlfaServer::handle_client_session(int socket_fd) {
    char buffer[256];
    bzero(buffer, 256);
    std::string username = "";
    int n = read(socket_fd, buffer, 255);
    
    if(n <= 0) {
        std::cerr << "Error reading client's username" << std::endl;
        close(socket_fd);
        accept_connections.unlock();
        return;
    }
    username = std::string(buffer); // <-- fixed: assignment instead of redeclaration
    std::cout << "Username: " << username << std::endl;
    std::string user_dir_path = server_dir_path / ("sync_dir_" + username);

    FileManager::create_directory(user_dir_path);
    
    std::unique_ptr<ClientSession> client_session = std::make_unique<ClientSession>(socket_fd, username, devices, betas, user_dir_path);

    client_session->connect_sockets();
    accept_connections.unlock();
    client_session->run();
}

void AlfaServer::handle_beta_session(int new_beta_socket_fd, struct sockaddr_in new_beta_address) {
    // Get the ip and ring_port of the new beta server
    char ip_buffer[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &new_beta_address.sin_addr, ip_buffer, sizeof(ip_buffer)) == NULL) {
        std::cerr << "Failed to convert BETA Server IP address" << std::endl;
        return;
    }
    std::string new_beta_ip(ip_buffer);
    Packet new_beta_ring_port_packet = Packet::receive(new_beta_socket_fd);
    int new_beta_ring_port = stoi(new_beta_ring_port_packet.payload.c_str());

    std::cout << "New BETA: " << new_beta_ip << ":" << new_beta_ring_port << " | SOCKET: " << new_beta_socket_fd << std::endl;

    {
        std::lock_guard<std::mutex> lock(write_beta_sockets);
        betas->add_beta(new_beta_socket_fd, new_beta_ip, new_beta_ring_port);
        betas->send_all_betas_to_new_beta(new_beta_socket_fd);
        devices->send_all_devices_to_beta(new_beta_socket_fd);
        send_server_files_to_new_beta(new_beta_socket_fd);
    }
    
    heartbeat(new_beta_socket_fd);
}

void AlfaServer::send_server_files_to_new_beta(int new_beta_socket_fd) {
    std::vector<std::string> usernames = devices->get_all_usernames_connected();
    Packet client_packet(static_cast<uint16_t>(Packet::Type::CLIENT), 0, 0, 0, "");
    Packet directory_packet(static_cast<uint16_t>(Packet::Type::DIRECTORY), 0, 0, 0, "");
    for (std::string& username : usernames) {
        std::cout << "Sending " << username << " sync_dir" << std::endl;
        Packet username_packet(static_cast<uint16_t>(Packet::Type::USERNAME), 0, 0, username.length(), username.c_str());
        client_packet.send(new_beta_socket_fd);
        username_packet.send(new_beta_socket_fd);
        directory_packet.send(new_beta_socket_fd);
        Packet::send_multiple_files(new_beta_socket_fd, server_dir_path / ("sync_dir_" + username));
    }
}

void AlfaServer::heartbeat(int beta_socket_fd) {
    std::cout << "Starting heartbeat... Socket : " << beta_socket_fd << std::endl;
    try {
        while (beta_socket_fd > 0) {
            Packet heartbeat_packet = Packet::receive(beta_socket_fd);

            if (heartbeat_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                std::cout << "[ ALFA THREAD ] " << "Received ERROR packet: " << heartbeat_packet.payload << std::endl;
                continue;
            }

            if (heartbeat_packet.type == static_cast<uint16_t>(Packet::Type::HEARTBEAT)) {
                betas->send_heartbeat(beta_socket_fd);
                continue;
            }

            throw std::runtime_error(
                "[ ALFA THREAD ] Unexpected packet type " + std::to_string(heartbeat_packet.type) + 
                " received from alfa server (expected ERROR or HEARTBEAT)"
            );
        }
    } catch (const std::runtime_error& e) {
        close(beta_socket_fd);
    }
}

void AlfaServer::handle_beta_connection() {
    int beta_session_socket;
    struct sockaddr_in beta_address;
    socklen_t beta_address_len = sizeof(struct sockaddr_in);
    std::cout << "Handling BETA Connection..." << std::endl;

    while (true) {
        beta_session_socket = accept(initial_socket_beta, (struct sockaddr*) &beta_address, &beta_address_len);

        if (beta_session_socket == -1)
            std::cerr << "ERROR: Failed to accept new client" << std::endl;
        
        if (beta_session_socket >= 0) {
            std::cout << "BETA connected" << std::endl;
            std::thread beta_session_thread(&AlfaServer::handle_beta_session, this, beta_session_socket, beta_address);
            beta_session_thread.detach();
        }
    }
}

void AlfaServer::handle_client_connection() {
    int client_session_socket;
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);
    std::cout << "Handling CLIENT Connection..." << std::endl;

    while (true) {
        client_session_socket = accept(initial_socket_client, (struct sockaddr*) &client_address, &client_address_len);

        if (client_session_socket == -1)
            std::cerr << "ERROR: Failed to accept new client" << std::endl;
        
        if (client_session_socket >= 0) {
            accept_connections.lock();
            std::cout << "CLIENT connected" << std::endl; 
            std::thread client_session_thread(&AlfaServer::handle_client_session, this, client_session_socket);
            client_session_thread.detach();
        }
    }
}

void AlfaServer::run() {
    std::cout << "Setting up ALFA server..." << std::endl;

    initial_socket_client = Network::setup_socket_ipv4(port_client);
    initial_socket_beta = Network::setup_socket_ipv4(port_beta);

    if (initial_socket_beta == -1 || initial_socket_client == -1)
        exit(1);

    devices = std::make_shared<ClientsDevices>();
    betas = std::make_shared<BetaManager>();

    FileManager::create_directory(server_dir_path);

    std::thread handle_beta_thread(&AlfaServer::handle_beta_connection, this);
    std::thread handle_client_thread(&AlfaServer::handle_client_connection, this);
    
    handle_beta_thread.join();
    handle_client_thread.join();    
}