#include <Server.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>

#define PORT 4003


void Server::handle_client(int socket) {
    char buffer[256];
    bzero(buffer, 256);
    int n = read(socket, buffer, 255);
    if(n > 0) {
        std::string username(buffer);
        std::cout << "Username: " << username << std::endl;
        fileManager.create_sync_dir(username);
    }

    commMananger.create_sockets(socket);
}

bool Server::setup() {
    std::cout << "Setup server..." << endl;

    struct sockaddr_in server_address;
    
    if ((initial_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cout << "SETUP ERROR opening socket" << std::endl;
        return false;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_address.sin_zero), 8);

    if (bind(initial_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0){
        std::cout << "SETUP ERROR on binding" << std::endl;
        return false;
    }
    
    std::cout << "Server waiting connections..." << endl;

    if (listen(initial_socket, 5) < 0) {
        std::cerr << "SETUP ERROR on listen" << std::endl;
        return false;
    }

    return true;
}

void Server::run() {
    if (!setup())
        exit(1);

    // Handle clients
    int client_socket;
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);

    while (true) {
        client_socket = accept(initial_socket, (struct sockaddr*) &client_address, &client_address_len);

        if (client_socket == -1)
            std::cout << "ERROR on accept new client" << std::endl;
        
        if (client_socket >= 0) {
            std::cout << "Client conected" << std::endl; 
            std::thread client_thread(&Server::handle_client, this, client_socket);
            client_thread.detach();
        }
    }
}