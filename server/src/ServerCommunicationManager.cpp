#include <ServerCommunicationManager.hpp>

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

void ServerCommunicationManager::create_sockets(int socket_cmd) {
    this->socket_cmd = socket_cmd;

    if ((socket_upload = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Erro ao criar socket" << std::endl;
        close_sockets();
        return;
    }

    if ((socket_download = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Erro ao criar socket" << std::endl;
        close_sockets();
        return;
    }

    if(!connect_socket_to_client(&socket_upload, &port_upload)) {
        std::cerr << "Erro ao conectar socket de upload" << std::endl;
        close_sockets();
        return;
    }
    
    if(!connect_socket_to_client(&socket_download, &port_upload)) {
        std::cerr << "Erro ao conectar socket de download" << std::endl;
        close_sockets();
        return;
    }
}

// ========================================= //
// ================ PRIVATE ================ //
// ========================================= //

void ServerCommunicationManager::close_sockets() {
    if (socket_cmd > 0) close(socket_cmd);
    if (socket_download > 0) close(socket_download);
    if (socket_upload > 0) close(socket_upload);
}

bool ServerCommunicationManager::connect_socket_to_client(int *sockfd, int *port) {
    struct sockaddr_in serv_addr;
    socklen_t len = sizeof(serv_addr);

    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = 0;

    if (bind(*sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Erro no bind" << std::endl;
        return false;
    }

    if (listen(*sockfd, 5) < 0) {
        std::cerr << "Erro no listen" << std::endl;
        return false;
    }

    if (getsockname(*sockfd, (struct sockaddr *)&serv_addr, &len) == -1) {
        std::cerr << "Erro no getsockname" << std::endl;
        return false;
    }

    std::string ip = inet_ntoa(serv_addr.sin_addr);
    *port = ntohs(serv_addr.sin_port);
    
    // write in the cmd socket
    std::string port_str = std::to_string(*port);
    std::cout << "Enviando porta: " << port_str << std::endl;
    int n = write(socket_cmd, port_str.c_str(), port_str.length());
    if (n < 0) {
        std::cerr << "Erro ao enviar porta para o cliente" << std::endl;
        return false;
    }
    std::cout << "Porta enviada" << std::endl;
    return true;
}