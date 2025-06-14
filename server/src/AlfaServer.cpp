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

void AlfaServer::handle_beta_session(int socket_fd, struct sockaddr_in beta_address) {
    betas->add_beta(socket_fd);
    
    // Get the ip of the new beta server
    char ip_buffer[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &beta_address.sin_addr, ip_buffer, sizeof(ip_buffer)) == NULL) {
        std::cerr << "Failed to convert BETA Server IP address" << std::endl;
        return;
    }
    std::string new_beta_ip(ip_buffer);


    if(socket_first_beta == -1 && socket_last_beta == -1) {
        socket_first_beta = socket_fd;
        socket_last_beta = socket_fd;
    } else {
        // Send the IP of the new beta server to the current last server in the ring
        Packet server_ip_packet(static_cast<uint16_t>(Packet::Type::SERVER), 0, 0, new_beta_ip.length(), new_beta_ip.c_str());
        server_ip_packet.send(socket_last_beta);

        // Send the IP of the current first server in the ring to the new beta server
        Packet server_ip_packet(static_cast<uint16_t>(Packet::Type::SERVER), 0, 0, ip_first_beta.length(), ip_first_beta.c_str());
        server_ip_packet.send(socket_fd);
    }
    
    // Set the new beta server as the first server in the ring
    ip_first_beta = new_beta_ip;
    
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

    initial_socket_client = setup_socket(port_client);
    initial_socket_beta = setup_socket(port_beta);

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


int AlfaServer::setup_socket(int port) {
    int new_socket;
    struct sockaddr_in server_address;
    
    if ((new_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cerr << "SETUP ERROR: Failed to open socket" << std::endl;
        return false;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_address.sin_zero), 8);
    std::cout << "Server listening on port " << port << std::endl;

    if (bind(new_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0){
        std::cerr << "SETUP ERROR: Failed to bind socket" << std::endl;
        return -1;
    }
    
    std::cout << "Server waiting for connections..." << std::endl;

    if (listen(new_socket, 5) < 0) {
        std::cerr << "SETUP ERROR: Failed to listen on socket" << std::endl;
        return -1;
    }

    return new_socket;
}