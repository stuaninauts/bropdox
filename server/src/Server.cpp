#include <Server.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>

#define PORT 4002

std::mutex mutexHashClientes;

void Server::handle_client(int socket) {
    char buffer[256];
    bzero(buffer, 256);
    std::string username = "";
    int n = read(socket, buffer, 255);

    if(n > 0) {
        username = std::string(buffer); // <-- corrigido: atribuição em vez de nova declaração
        std::cout << "Username: " << username << std::endl;
        fileManager.create_sync_dir(username);
    }

    int download_socket = commMananger.create_sockets(socket);
    std::cout << "Esperando atribuição da conexão" << std::endl;
    addClientSocket(username, download_socket);
    std::cout << "Conexão atribuida" << std::endl;
    sleep(10);
    removeClientSocket(username, download_socket);
}

void Server::addClientSocket(const std::string& username, int socketFd)
{
    std::lock_guard<std::mutex> lock(mutexHashClientes);  
    // Acesso ao mapa protegido
    auto it = clientsSockets.find(username);
    if (it != clientsSockets.end()) {
        if (clientsSockets[username].size() >= 2) {
            std::cout << "Máximo de 2 conexões alcançado para o usuário " << username << ".\n";
            close(socketFd);
            mutexHashClientes.unlock();  // libera antes do return
            // TODO ENVIAR PARA O CLIENTE  UM SINAL PARA ELE SE MATAR
            return;
        }
        clientsSockets[username].push_back(socketFd);
    } else {
        clientsSockets[username].push_back(socketFd);
    }

    printClientsSockets();  // <-- PERIGOSO SE USAR O MESMO MUTEX
}

void Server::removeClientSocket(const std::string& username, int socketFd)
{
    mutexHashClientes.lock();

    auto it = clientsSockets.find(username);
    if (it != clientsSockets.end()) {
        auto& sockets = it->second;

        // Remove o socketFd do vetor
        sockets.erase(std::remove(sockets.begin(), sockets.end(), socketFd), sockets.end());

        std::cout << "Socket " << socketFd << " removido do usuário " << username << ".\n";

        // Se não restarem mais sockets, remove o usuário do mapa
        if (sockets.empty()) {
            clientsSockets.erase(it);
            std::cout << "Usuário " << username << " removido do mapa (sem conexões ativas).\n";
        }
    } else {
        std::cout << "Usuário " << username << " não encontrado para remoção.\n";
    }

    mutexHashClientes.unlock();  // libera o mutex
    printClientsSockets();
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

void Server::printClientsSockets() {
    // Verifica se o mapa está vazio
    if (clientsSockets.empty()) {
        std::cout << "O mapa de clientes está vazio!" << std::endl;
    }

    // Itera sobre o unordered_map
    for (const auto& pair : clientsSockets) {
        const std::string& username = pair.first;
        const std::vector<int>& sockets = pair.second;

        std::cout << "Usuário: " << username << " - Conexões de download: ";

        for (int socket : sockets) {
            std::cout << socket << " ";
        }
        std::cout << std::endl;
    }
}