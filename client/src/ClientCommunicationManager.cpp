#include <ClientCommunicationManager.hpp>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

bool ClientCommunicationManager::connect_to_server(const std::string server_ip, int port, const std::string username) {
    this->server_ip = server_ip;
    this->port_cmd = port;
    this->username = username;
    try {
        if (!connect_socket_cmd()) {
            close_sockets();
            return false;
        }

        if (!send_username()) {
            close_sockets();
            return false;
        }

        // if (!get_sockets_ports()) {
        //     close_sockets();
        //     return false;
        // }

        // cria socket de upload
        if ((socket_upload = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Erro ao criar socket de upload";
            close_sockets();
            return false;
        }

        // cria socket de download
        if ((socket_download = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Erro ao criar socket de download";
            close_sockets();
            return false;
        }

        if (!connect_socket_to_server(socket_upload, &port_upload)){
            std::cerr << "Erro ao conectar socket de download";
            close_sockets();
            return false;
        }

        if (!connect_socket_to_server(socket_download, &port_download)){
            std::cerr << "Erro ao conectar socket de download";
            close_sockets();
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Exceção: " << e.what() << std::endl;
        return false;
    }
}

// ========================================= //
// ================ PRIVATE ================ //
// ========================================= //

bool ClientCommunicationManager::send_username() {
    int n = write(socket_cmd, username.c_str(), username.length());
    if (n < 0) {
        std::cerr << "ERROR: Writing username to socket\n";
        return false;
    }
    return true;
}

bool ClientCommunicationManager::connect_socket_cmd() {
    struct hostent* server;

    if ((server = gethostbyname(server_ip.c_str())) == nullptr) {
        std::cerr << "ERROR: No such host\n";
        return false;
    }
    
    if ((socket_cmd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "ERROR: Opening socket\n";
        return false;
    }

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_cmd);
    serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(socket_cmd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "ERROR: Connecting to server\n";
        return false;
    }

    return true;
}

bool ClientCommunicationManager::get_sockets_ports() {
    char buffer_upload[256];
    bzero(buffer_upload, 256);
    if (read(socket_cmd, buffer_upload, 255) <= 0) {
        std::cerr << "ERROR: Can't read upload port\n";
        return false;
    }
    port_upload = std::stoi(buffer_upload);
    std::cout << "Upload port: " << port_upload << std::endl;

    char buffer_download[256];
    bzero(buffer_download, 256);
    if (read(socket_cmd, buffer_download, 255) <= 0) {
        std::cerr << "ERROR: Can't read download port\n";
        return false;
    }
    port_download = std::stoi(buffer_download);
    std::cout << "Download port: " << port_download << std::endl;

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
        std::cerr << "ERROR: Can't read upload port\n";
        return false;
    }
    *port = std::stoi(buffer);
    std::cout << "Upload port: " << *port << std::endl;

    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(*port);

    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Endereço IP inválido: " << server_ip << std::endl;
        return false;
    }
    std::cout << "server ip" << server_ip << std::endl;

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Erro ao conectar ao servidor";
        return false;
    }

    std::cout << "Socket conectado no servidor" << std::endl;

    return true;
}