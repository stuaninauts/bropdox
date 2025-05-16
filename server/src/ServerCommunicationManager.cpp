#include "ServerCommunicationManager.hpp"
#include <algorithm> // Para std::transform
#include <cctype>    // Para ::tolower
#include <sstream>   // Para std::istringstream
#include <vector>    // Para std::vector

std::mutex access_devices;
std::mutex access_files;
std::mutex access_download;

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

// Retornar o download_socket para ser salvo na hash de usuários
void ServerCommunicationManager::setup_client_session(int socket_cmd, std::string username, std::shared_ptr<ClientsDevices> devices) {
    this->socket_cmd = socket_cmd;
    this->devices = devices;
    this->username = username;
    bool suicide; // cliente SE MATA!

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

    access_devices.lock();
    {
        suicide = !devices->add_client_socket(username, socket_download);
    }
    access_devices.unlock();

    // mata
    if (suicide) {
        std::cout << "Socket download " << socket_download << std::endl;
        Packet::send_error(socket_download);
        sleep(10);
        close_sockets();
        return;
    }

    Packet::send_ack(socket_download);

    std::thread thread_cmd(&ServerCommunicationManager::read_cmd, this);
    std::thread thread_sync_client(&ServerCommunicationManager::sync_client, this);
    
    thread_cmd.join();
    thread_sync_client.join();
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

vector<string> split_command(const string &command) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(command);
    
    while (tokenStream >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

void ServerCommunicationManager::read_cmd() {
    while (socket_cmd > 0) {
        Packet packet;

        try {
            packet = Packet::receive(socket_cmd);
            std::cout << "Pacote recebido: " << packet.to_string() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Erro ao receber pacote: " << e.what() << std::endl;
            return;
        }

        // Extrair comando do payload
        std::vector<string> tokens = split_command(packet.payload);
        std::string command = tokens[0];
        
        if (command == "upload")
            handle_client_upload(tokens[1]);
        else if (command == "download")
            handle_client_download(tokens[1]);
        else if (command == "delete")
            handle_client_delete(tokens[1]);
        else if (command == "list_server")
            handle_list_server();
        else if (command == "exit")
            handle_exit();
        else
            std::cerr << "[Server] Comando desconhecido: " << command << std::endl;
    }
    
}

void ServerCommunicationManager::sync_client() {
    // server_dir_path
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

    if ((write(socket_cmd, port_str.c_str(), port_str.length())) < 0) {
        std::cerr << "Erro ao enviar porta para o cliente" << std::endl;
        return false;
    }

    // ACEITA CONEXÃO
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);
    if((*sockfd = accept(*sockfd, (struct sockaddr*) &client_address, &client_address_len)) < 0){
        std::cerr << "Erro ao aceitar cliente" << std::endl;
        return false;
    }
    return true;
}

void ServerCommunicationManager::handle_client_download(const std::string filename) {
    access_download.lock();
    {
        if(!Packet::send_file(socket_download, file_manager.server_dir_path + "/" + filename))
            Packet::send_error(socket_download);
    }
    access_download.unlock();

}

void ServerCommunicationManager::handle_client_upload(const std::string filename) {
    int socket_download_other_device;
    std::cout << "[ FILENAME ]: " << filename << std::endl;
    access_files.lock();
    {
        file_manager.write_file(socket_upload);
    }
    access_files.unlock();
    access_devices.lock();
    {
        socket_download_other_device = devices->get_other_device_socket(username, socket_download);
    }
    access_devices.unlock();
    // Se não existe outra sessão do cliente, não envia o arquivo
    std::cout << "[ SOCKET OTHER DEVICE ]: " << socket_download_other_device << std::endl;
    if(socket_download_other_device < 0)
        return;
    access_download.lock();
    {
        if(!Packet::send_file(socket_download_other_device, file_manager.server_dir_path + "/" + filename))
            Packet::send_error(socket_download_other_device);
    }
    access_download.unlock();
}

void ServerCommunicationManager::handle_client_delete(const std::string filename) {
    // int socket_download_other_device;
    // access_files.lock();
    // {
    //     file_manager.delete_file(filename)
    // }
    // access_files.lock();
    // access_devices.lock();
    // {
    //     socket_download_other_device = devices->get_other_device_socket(username, socket_download);
    // }
    // access_devices.unlock();
    // std::cout << socket_download_other_device << std::endl;
}

void ServerCommunicationManager::handle_exit() {
    // TO DO
}

void ServerCommunicationManager::handle_list_server() {
    std::cout << "[Server] Listando arquivos no servidor:" << std::endl;
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
        response_packet.send(socket_cmd);
        std::cout << "[Server] Lista de arquivos enviada ao cliente." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Server] Erro ao enviar lista de arquivos: " << e.what() << std::endl;
    }
}

