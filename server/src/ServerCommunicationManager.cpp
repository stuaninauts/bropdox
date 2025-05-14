#include <ServerCommunicationManager.hpp>

std::mutex add_device;
std::mutex remove_device;

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

// Retornar o download_socket para ser salvo na hash de usuários
void ServerCommunicationManager::run_client_session(int socket_cmd, std::string username, std::shared_ptr<ClientsDevices> devices) {
    this->socket_cmd = socket_cmd;
    this->devices = devices;

    if(!connect_socket_to_client(&socket_upload, &port_upload)) {
        std::cerr << "Erro ao conectar socket de upload" << std::endl;
        close_sockets();
        return;
    }
    
    if(!connect_socket_to_client(&socket_download, &port_download)) {
        std::cerr << "Erro ao conectar socket de download" << std::endl;
        close_sockets();
        return;
    }

    add_device.lock();
    {
        devices->add_client_socket(username, socket_download);
    }
    add_device.unlock();

    sleep(10);

    add_device.lock();
    {
        devices->remove_client_socket(username, socket_download);
    }
    add_device.unlock();

    while(true);
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

    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Erro ao criar socket" << std::endl;
        return false;
    }

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

    // std::cout << "Socket escutando em IP: " << inet_ntoa(serv_addr.sin_addr) << std::endl;
    
    // ENVIA PORTA
    *port = ntohs(serv_addr.sin_port);
    std::string port_str = std::to_string(*port);
    // std::cout << "Enviando porta: " << port_str << std::endl;
    if ((write(socket_cmd, port_str.c_str(), port_str.length())) < 0) {
        std::cerr << "Erro ao enviar porta para o cliente" << std::endl;
        return false;
    }
    // std::cout << "Porta enviada" << std::endl;

    // ACEITA CONEXÃO
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);
    if((*sockfd = accept(*sockfd, (struct sockaddr*) &client_address, &client_address_len)) < 0){
        std::cerr << "Erro ao aceitar cliente" << std::endl;
        return false;
    }
    
    return true;
}