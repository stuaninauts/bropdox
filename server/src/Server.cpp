#include <Server.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>

#define PORT 8081

void Server::handle_client(int socket) {
    char buffer[256];
    bzero(buffer, 256);
    std::string username = "";
    int n = read(socket, buffer, 255);
    
    if(n <= 0) {
        std::cout << "Error reading client's username" << std::endl;
        return;
    }
    username = std::string(buffer); // <-- fixed: assignment instead of redeclaration
    std::cout << "Username: " << username << std::endl;
    
    try {
        std::unique_ptr<ServerFileManager> file_manager = std::make_unique<ServerFileManager>(username);
        std::unique_ptr<ServerCommunicationManager> comm_manager = std::make_unique<ServerCommunicationManager>(*file_manager);

        file_manager->create_sync_dir();
        comm_manager->run_client_session(socket, username, devices);

    } catch(const std::exception& e) {
        std::cout << "Error creating server file manager: " << e.what() << std::endl;
    }
}

bool Server::setup() {
    std::cout << "Setting up server..." << std::endl;

    struct sockaddr_in server_address;
    
    if ((initial_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cout << "SETUP ERROR: Failed to open socket" << std::endl;
        return false;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_address.sin_zero), 8);

    if (bind(initial_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0){
        std::cout << "SETUP ERROR: Failed to bind socket" << std::endl;
        return false;
    }
    
    std::cout << "Server waiting for connections..." << std::endl;

    if (listen(initial_socket, 5) < 0) {
        std::cout << "SETUP ERROR: Failed to listen on socket" << std::endl;
        return false;
    }

    return true;
}

void Server::run() {
    if (!setup())
        exit(1);

    devices = std::make_shared<ClientsDevices>();

    // Handle clients
    int client_socket;
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);

    while (true) {
        client_socket = accept(initial_socket, (struct sockaddr*) &client_address, &client_address_len);

        if (client_socket == -1)
            std::cout << "ERROR: Failed to accept new client" << std::endl;
        
        if (client_socket >= 0) {
            std::cout << "Client connected" << std::endl; 
            std::thread client_thread(&Server::handle_client, this, client_socket);
            client_thread.detach();
        }
    }
}
