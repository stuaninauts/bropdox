#include <BetaServer.hpp>


bool BetaServer::connect_to_alfa() {
    struct hostent* server;
    struct sockaddr_in serv_addr{};

    if ((server = gethostbyname(ip_alfa.c_str())) == nullptr) {
        std::cerr << "ERROR: No such host\n";
        return false;
    }
    
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "ERROR: Opening socket\n";
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_alfa);
    serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "ERROR: Connecting to server\n";
        return false;
    }

    return true;
}

void BetaServer::handle_client_upload(const std::string filename, const std::string username, uint32_t total_packets) {
    std::cout << "[" << username << "]" << "handle_client_upload: " << filename << std::endl;
    FileManager::create_directory(backup_dir_path / username);
    Packet::receive_file(socket_fd, filename, backup_dir_path / username, total_packets);
}

void BetaServer::handle_client_delete(const std::string filename, const std::string username) {
    std::cout << "[" << username << "]" << "handle_client_delete: " << filename << std::endl;
    FileManager::create_directory(backup_dir_path / username);
    FileManager::delete_file(backup_dir_path / username / filename);
}


void BetaServer::sync() {
    try {
        while (socket_fd > 0) {
            Packet username_packet = Packet::receive(socket_fd);
            Packet meta_packet = Packet::receive(socket_fd);

            std::cout << "Received meta_packet from alfa server: " << meta_packet.payload << std::endl;

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                std::cout << "Received ERROR packet: " << meta_packet.payload << std::endl;
                continue;
            }

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::DELETE)) {
                handle_client_delete(meta_packet.payload, username_packet.payload);
                continue;
            }

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::DATA)) {
                handle_client_upload(meta_packet.payload, username_packet.payload, meta_packet.total_size);
                continue;
            }
            
            throw std::runtime_error(
                "Unexpected packet type " + std::to_string(meta_packet.type) + 
                " received from alfa server (expected DATA, DELETE, or ERROR)"
            );
        }
    } catch (const std::runtime_error& e) {
        close(socket_fd);
        exit(1);
    }
}

void BetaServer::run() {
    std::cout << "Setting up BETA server..." << std::endl;
    if(!connect_to_alfa())
        exit(1);

    backup_dir_path = fs::path("./sync_dir_backup_" + std::to_string(socket_fd));
    FileManager::create_directory(backup_dir_path);

    std::thread sync_thread = std::thread(&BetaServer::sync, this);
    std::cout << "Connected to ALFA server!" << std::endl;

    sync_thread.join();
}
