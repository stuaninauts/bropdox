#include <AlfaServer.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <FileManager.hpp>

std::mutex accept_connections;

void AlfaServer::handle_client_session(int socket_fd) {
    char buffer[256];
    bzero(buffer, 256);
    std::string username = "";
    int port_backup = 0;
    int n = read(socket_fd, buffer, 255);

    if(n <= 0) {
        std::cerr << "Error reading client's username" << std::endl;
        close(socket_fd);
        accept_connections.unlock();
        return;
    }

    std::istringstream iss(buffer);
    iss >> username >> port_backup;

    if (username.empty() || port_backup == 0) {
        std::cerr << "Invalid client data received" << std::endl;
        close(socket_fd);
        accept_connections.unlock();
        return;
    }

    std::cout << "Username: " << username << ", Port Beta: " << port_backup << std::endl;
    std::string user_dir_path = server_dir_path / ("sync_dir_" + username);

    FileManager::create_directory(user_dir_path);

    std::unique_ptr<ClientSession> client_session = std::make_unique<ClientSession>(socket_fd, username, devices, betas, user_dir_path, port_backup);

    client_session->connect_sockets();
    accept_connections.unlock();
    client_session->run();
}

void AlfaServer::handle_beta_session(int new_beta_socket_fd, struct sockaddr_in new_beta_address) {
    betas->add_beta(new_beta_socket_fd);
    
    // Get the ip of the new beta server
    char ip_buffer[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &new_beta_address.sin_addr, ip_buffer, sizeof(ip_buffer)) == NULL) {
        std::cerr << "Failed to convert BETA Server IP address" << std::endl;
        return;
    }
    std::string new_beta_ip(ip_buffer);
    
    Packet new_beta_port_packet = Packet::receive(new_beta_socket_fd);
    int new_beta_port = stoi(new_beta_port_packet.payload.c_str());

    if(socket_first_beta == -1 && socket_last_beta == -1) {
        socket_first_beta = new_beta_socket_fd;
        socket_last_beta = new_beta_socket_fd;
        port_first_beta = new_beta_port;
    } else {
        // Send the IP and the PORT of the new beta server to the current last server in the ring
        Packet first_server_ip_packet(static_cast<uint16_t>(Packet::Type::SERVER), 0, 0, new_beta_ip.length(), new_beta_ip.c_str());
        Packet first_server_port_packet(static_cast<uint16_t>(Packet::Type::DATA), 0, 0, std::to_string(new_beta_port).length(), std::to_string(new_beta_port).c_str());
        first_server_ip_packet.send(socket_last_beta);
        first_server_port_packet.send(socket_last_beta);
        
        // Send the IP and the PORT of the current first server in the ring to the new beta server
        Packet last_server_ip_packet(static_cast<uint16_t>(Packet::Type::SERVER), 0, 0, ip_first_beta.length(), ip_first_beta.c_str());
        Packet last_server_port_packet(static_cast<uint16_t>(Packet::Type::DATA), 0, 0, std::to_string(port_first_beta).length(), std::to_string(port_first_beta).c_str());
        last_server_ip_packet.send(new_beta_socket_fd);
        last_server_port_packet.send(new_beta_socket_fd);
    }
    
    // Set the new beta server as the first server in the ring
    ip_first_beta = new_beta_ip;
    port_first_beta = new_beta_port;
    
    std::cout << "New first BETA: " << ip_first_beta << ":" << port_first_beta << " | SOCKET: " << socket_last_beta << std::endl;

    // while (true) listening to beta...
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
