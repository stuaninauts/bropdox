#include <ServerCommunicationManager.hpp>

void ServerCommunicationManager::create_sockets(int socket_cmd) {
    this->socket_cmd = socket_cmd;

    if ((socket_upload = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "Erro ao criar socket";
        return;
    }
    if ((socket_download = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "Erro ao criar socket";
        return;
    }

    if(!connect_to_client(&socket_upload, &port_upload)) {
        close_sockets();
        return;
    }
    
    if(!connect_to_client(&socket_download, &port_upload)) {
        close_sockets();
        return;
    }
}

void ServerCommunicationManager::close_sockets() {
    close(socket_cmd);
    close(socket_upload);
    close(socket_download);
}

bool ServerCommunicationManager::connect_to_client(int *sockfd, int *port) {
    struct sockaddr_in serv_addr;
    socklen_t len = sizeof(serv_addr);

    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = 0;

    if (bind(*sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Erro no bind";
        return false;
    }

    if (listen(*sockfd, 5) < 0) {
        perror("Erro no listen");
        std::cout << "Erro no listen";
        return false;
    }

    if (getsockname(*sockfd, (struct sockaddr *)&serv_addr, &len) == -1) {
        std::cout << "Erro no getsockname";
        return false;
    }

    std::string ip = inet_ntoa(serv_addr.sin_addr);
    *port = ntohs(serv_addr.sin_port);
    
    // write in the cmd socket
    std::string port_str = std::to_string(*port);
    std::cout << std::endl << "enviando porta: " << port_str << std::endl;
    int n = write(socket_cmd, port_str.c_str(), port_str.length());
    if (n < 0) {
        std::cout << "Erro ao enviar porta para o cliente";
        return false;
    }
    std::cout << "porta enviada" << std::endl;
    return true;
}