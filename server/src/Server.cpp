#include <Server.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <FileManager.hpp>

std::mutex accept_connections;

void Server::handle_client(int socket) {
    char buffer[256];
    bzero(buffer, 256);
    std::string username = "";
    int n = read(socket, buffer, 255);
    
    if(n <= 0) {
        std::cerr << "Error reading client's username" << std::endl;
        close(socket);
        accept_connections.unlock();
        return;
    }
    username = std::string(buffer); // <-- fixed: assignment instead of redeclaration
    std::cout << "Username: " << username << std::endl;
    std::string user_dir_path = server_dir_path / ("sync_dir_" + username);
    
    std::unique_ptr<ClientSession> client_session = std::make_unique<ClientSession>(socket, username, devices, user_dir_path);

    FileManager::create_directory(server_dir_path);
    FileManager::create_directory(user_dir_path);

    client_session->connect_sockets();
    accept_connections.unlock();
    client_session->run();
}

bool Server::setup() {
    std::cout << "Setting up server..." << std::endl;

    struct sockaddr_in server_address;
    
    if ((initial_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cerr << "SETUP ERROR: Failed to open socket" << std::endl;
        return false;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_address.sin_zero), 8);
    std::cout << "Server listening on port " << port << std::endl;

    if (bind(initial_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0){
        std::cerr << "SETUP ERROR: Failed to bind socket" << std::endl;
        return false;
    }
    
    std::cout << "Server waiting for connections..." << std::endl;

    if (listen(initial_socket, 5) < 0) {
        std::cerr << "SETUP ERROR: Failed to listen on socket" << std::endl;
        return false;
    }

    return true;
}

void Server::run_alfa() {
    std::cout << "I'm ALFA" << std::endl;
    
    devices = std::make_shared<ClientsDevices>();
    
    // Handle clients
    int client_socket;
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);

    while (true) {
        client_socket = accept(initial_socket, (struct sockaddr*) &client_address, &client_address_len);

        if (client_socket == -1)
            std::cerr << "ERROR: Failed to accept new client" << std::endl;
        
        if (client_socket >= 0) {
            accept_connections.lock();
            std::cout << "Client connected" << std::endl; 
            std::thread client_thread(&Server::handle_client, this, client_socket);
            client_thread.detach();
        }
    }
}

void Server::run_beta() {
    std::cout << "I'm BETA!" << std::endl;
    std::cout << ip_primary_server << " is the ALFA!" << std::endl;
}

void Server::run() {
    if (!setup())
        exit(1);

    if(alfa)
        run_alfa();

    if(!alfa)
        run_beta();

}
