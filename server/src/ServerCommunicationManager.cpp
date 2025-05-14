#include "ServerCommunicationManager.hpp"
#include <algorithm> // Para std::transform
#include <cctype>    // Para ::tolower
#include <sstream>   // Para std::istringstream
#include <vector>    // Para std::vector

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

void ServerCommunicationManager::create_sockets(int socket_cmd) {
    this->socket_cmd = socket_cmd;

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
}

void ServerCommunicationManager::receive_packet() {
    Packet packet;
    try {
        packet = Packet::receive(socket_upload);
        std::cout << "Pacote recebido: " << packet.to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Erro ao receber pacote: " << e.what() << std::endl;
        return;
    }
}

void ServerCommunicationManager::read_cmd() {
    Packet packet;

    try {
        packet = Packet::receive(socket_cmd);
        std::cout << "Pacote recebido: " << packet.to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Erro ao receber pacote: " << e.what() << std::endl;
        return;
    }

    // Extrair comando do payload
    std::string payload = packet.payload;
    std::istringstream iss(payload);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) return;

    std::string command = tokens[0];
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    if (command == "upload" && tokens.size() == 2) {
        std::string filename = tokens[1];
        std::cout << "[Server] Iniciando upload do arquivo: " << filename << std::endl;
        // Chame função para lidar com upload
        // ex: handle_upload(filename);
    } else if (command == "download" && tokens.size() == 2) {
        std::string filename = tokens[1];
        std::cout << "[Server] Iniciando download do arquivo: " << filename << std::endl;
        // Chame função para lidar com download
    } else if (command == "delete" && tokens.size() == 2) {
        std::string filename = tokens[1];
        std::cout << "[Server] Deletando arquivo: " << filename << std::endl;
        // Chame função para deletar
    } else if (command == "list_server") {
        std::cout << "[Server] Listando arquivos no servidor:" << std::endl;
        handle_list_server();
    } else if (command == "exit") {
        std::cout << "[Server] Encerrando sessão com o cliente." << std::endl;
        // Feche conexão ou finalize
    } else {
        std::cerr << "[Server] Comando desconhecido: " << command << std::endl;
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

    std::cout << "Socket escutando em IP: " << inet_ntoa(serv_addr.sin_addr) << std::endl;
    
    // ENVIA PORTA
    *port = ntohs(serv_addr.sin_port);
    std::string port_str = std::to_string(*port);
    std::cout << "Enviando porta: " << port_str << std::endl;
    if ((write(socket_cmd, port_str.c_str(), port_str.length())) < 0) {
        std::cerr << "Erro ao enviar porta para o cliente" << std::endl;
        return false;
    }
    std::cout << "Porta enviada" << std::endl;

    // ACEITA CONEXÃO
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);
    if((*sockfd = accept(*sockfd, (struct sockaddr*) &client_address, &client_address_len)) < 0){
        std::cerr << "Erro ao aceitar cliente" << std::endl;
        return false;
    }
    std::cout << "Socket conectado" << std::endl;

    return true;
}

void ServerCommunicationManager::handle_list_server() {
    // Primeiro, lista os arquivos no console do servidor
    file_manager.list_files();
    
    // Em seguida, obtém a lista formatada para enviar ao cliente
    std::string file_list = file_manager.get_files_list();
    
    // Cria um pacote com a lista de arquivos
    Packet response_packet;
    response_packet.type = static_cast<uint16_t>(Packet::Type::DATA); // Tipo DATA
    response_packet.total_size = 1;          // Apenas um pacote para a lista
    response_packet.length = file_list.size();
    response_packet.payload = file_list;
    
    try {
        // Valida o pacote antes de enviar
        response_packet.validate_length();
        
        // Envia a resposta para o cliente pelo socket de download
        response_packet.send(socket_download);
        std::cout << "[Server] Lista de arquivos enviada ao cliente." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Server] Erro ao enviar lista de arquivos: " << e.what() << std::endl;
    }
}
