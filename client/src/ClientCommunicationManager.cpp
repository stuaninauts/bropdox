#include <ClientCommunicationManager.hpp>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <Packet.hpp>

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

bool ClientCommunicationManager::connect_to_server(const std::string server_ip, int port, const std::string username) {
    this->server_ip = server_ip;
    this->port_cmd = port;
    this->username = username;
    try {
        // Inicialização da conexão principal
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

        // Cria socket de upload
        if ((socket_upload = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Erro ao criar socket de upload";
            close_sockets();
            return false;
        }

        // Cria socket de download
        if ((socket_download = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Erro ao criar socket de download";
            close_sockets();
            return false;
        }

        // Conecta socket de upload
        if (!connect_socket_to_server(socket_upload, &port_upload)){
            std::cerr << "Erro ao conectar socket de download";
            close_sockets();
            return false;
        }

        // Conecta socket de download
        if (!connect_socket_to_server(socket_download, &port_download)){
            std::cerr << "Erro ao conectar socket de download";
            close_sockets();
            return false;
        }

        // Verificação de erros e recebimento de confirmação
        if (!check_for_errors_and_confirm()) {
            close_sockets();
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Exceção: " << e.what() << std::endl;
        close_sockets(); // TODO: revisar
        return false;
    }
}

void ClientCommunicationManager::exit_server() {
    std::cout << "Desconexando do servidor..." << std::endl;
    send_command("exit");
    close_sockets();
    exit(0);
}

void ClientCommunicationManager::upload_file(const std::string filepath) {
    std::cout << "Uploading file: " << filepath << " to server's sync_dir" << std::endl;
    
    send_command("upload");

    if (!Packet::send_file(socket_upload, filepath))
        Packet::send_error(socket_upload);
}

void ClientCommunicationManager::download_file(const std::string filename) {
    send_command("download", filename);
    if(!Packet::receive_file(socket_download, "./client/sync_dir/"))
        std::cout << "Não foi possível fazer o download do arquivo" << std::endl;
}

void ClientCommunicationManager::delete_file(const std::string filename) {
    std::cout << "Deleting file: " << filename << " from sync_dir" << std::endl;
    send_command("delete", filename);
}

void ClientCommunicationManager::list_server() {
    std::cout << "Listing files on server:" << std::endl;
    send_command("list_server");
    Packet packet = Packet::receive(socket_download);
    std::cout << packet.to_string() << std::endl;

}
// ========================================= //
// ================ PRIVATE ================ //
// ========================================= //

void ClientCommunicationManager::send_command(const std::string command, const std::string filename) {
    std::string payload = command;
    if(filename != ""){
        payload += " " + filename; //segredo TODO ta ruim feio
    }

    Packet command_packet;
    command_packet.type = static_cast<uint16_t>(Packet::Type::CMD);
    command_packet.total_size = 1;
    command_packet.payload = payload;
    command_packet.length = command_packet.payload.size();

    try {
        command_packet.send(socket_cmd);
    } catch (const std::exception& e) {
        std::cerr << "Error sending list_server command: " << e.what() << std::endl;
    }
}


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
    // std::cout << "Upload port: " << port_upload << std::endl;

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
  //  std::cout << "Upload port: " << *port << std::endl;

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

    return true;
}

bool ClientCommunicationManager::check_for_errors_and_confirm() {
    // Verificar se há mensagens de erro no socket_cmd
    if (!check_command_socket_for_errors()) {
        return false;
    }

    // Verificar se o servidor confirmou a conexão
    if (!receive_server_confirmation()) {
        return false;
    }

    return true;
}

bool ClientCommunicationManager::check_command_socket_for_errors() {
    fd_set readfds;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;  // 100ms

    FD_ZERO(&readfds);
    FD_SET(socket_cmd, &readfds);

    if (select(socket_cmd + 1, &readfds, NULL, NULL, &tv) > 0) {
        // Há dados para ler no socket_cmd
        try {
            Packet errorCheck = Packet::receive(socket_cmd);
            if (errorCheck.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                std::cerr << "O servidor negou a conexão: " << errorCheck.payload << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            // Ignora exceções aqui, pq não é obrigatório ter um erro
        }
    }
    return true;
}

bool ClientCommunicationManager::receive_server_confirmation() {
    struct timeval recv_tv;
    recv_tv.tv_sec = 2;  // 2 seg
    recv_tv.tv_usec = 0;
    setsockopt(socket_download, SOL_SOCKET, SO_RCVTIMEO, &recv_tv, sizeof(recv_tv));

    try {
        Packet ack = Packet::receive(socket_download);
        std::cout << "ACK recebido: " << ack.to_string() << std::endl;
        
        if (ack.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
            std::cerr << "Erro recebido do servidor: " << ack.payload << std::endl;
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erro durante a conexão: " << e.what() << std::endl;
        if (std::string(e.what()).find("Connection closed") != std::string::npos) {
            std::cerr << "O servidor negou a conexão (máximo de clientes atingido)" << std::endl;
        }
        return false;
    }
}