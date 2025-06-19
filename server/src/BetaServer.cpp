#include <BetaServer.hpp>

#define PORT_BETA 8085


void BetaServer::run() {
    std::cout << "Setting up BETA server..." << std::endl;
    alfa_socket_fd = Network::connect_socket_ipv4(ip_alfa, port_alfa);
    ring_port = Network::get_available_port();
    ring_socket_fd = Network::setup_socket_ipv4(ring_port);
    if(alfa_socket_fd == -1 || ring_socket_fd == -1)
        exit(1);

    Packet packet = Packet(static_cast<uint16_t>(Packet::Type::DATA), 0, 0, std::to_string(ring_port).length(), std::to_string(ring_port).c_str());
    packet.send(alfa_socket_fd);
    

    devices = std::make_shared<ClientsDevices>();
    backup_dir_path = fs::path("./sync_dir_backup_" + std::to_string(alfa_socket_fd));
    FileManager::create_directory(backup_dir_path);

    std::thread sync_thread = std::thread(&BetaServer::handle_alfa_updates, this);
    std::thread ring_thread = std::thread(&BetaServer::accept_ring_connection, this);
    std::cout << "Connected to ALFA server!" << std::endl;

    sync_thread.join();
    ring_thread.join();
}

void BetaServer::handle_alfa_updates() {
    try {
        while (alfa_socket_fd > 0) {
            Packet meta_packet = Packet::receive(alfa_socket_fd);

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                std::cout << "Received ERROR packet: " << meta_packet.payload << std::endl;
                continue;
            }

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::USERNAME)) {
                handle_client_updates(meta_packet.payload);
                continue;
            }

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::SERVER)) {
                connect_next_beta(meta_packet.payload);
                continue;
            }
            
            throw std::runtime_error(
                "Unexpected packet type " + std::to_string(meta_packet.type) + 
                " received from alfa server (expected USERNAME, SERVER, or ERROR)"
            );
        }
    } catch (const std::runtime_error& e) {
        close(alfa_socket_fd);
        exit(1);
    }
}

void BetaServer::handle_client_updates(std::string username) {
    Packet update_meta_packet = Packet::receive(alfa_socket_fd);

    std::cout << "Received meta_packet from alfa server: " << update_meta_packet.payload << std::endl;

    if (update_meta_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
        std::cout << "Received ERROR packet: " << update_meta_packet.payload << std::endl;
        return;
    }

    if (update_meta_packet.type == static_cast<uint16_t>(Packet::Type::DELETE)) {
        handle_client_delete(update_meta_packet.payload, username);
        return;
    }

    if (update_meta_packet.type == static_cast<uint16_t>(Packet::Type::DATA)) {
        handle_client_upload(update_meta_packet.payload, username, update_meta_packet.total_size);
        return;
    }

    if (update_meta_packet.type == static_cast<uint16_t>(Packet::Type::IP)) {
        handle_new_client(update_meta_packet.payload, username);
        return;
    }
    
    throw std::runtime_error(
        "Unexpected packet type " + std::to_string(update_meta_packet.type) + 
        " received from alfa server (expected DATA, DELETE, IP or ERROR)"
    );
}

void BetaServer::handle_client_upload(const std::string filename, const std::string username, uint32_t total_packets) {
    std::cout << "[" << username << "]" << "handle_client_upload: " << filename << std::endl;
    FileManager::create_directory(backup_dir_path / username);
    Packet::receive_file(alfa_socket_fd, filename, backup_dir_path / username, total_packets);
}

void BetaServer::handle_client_delete(const std::string filename, const std::string username) {
    std::cout << "[" << username << "]" << "handle_client_delete: " << filename << std::endl;
    FileManager::create_directory(backup_dir_path / username);
    FileManager::delete_file(backup_dir_path / username / filename);
}

void BetaServer::handle_new_client(const std::string ip, const std::string username) {
    devices->add_client(username, -1, ip, 4000);
}

void BetaServer::connect_next_beta(std::string next_beta_ip) {
    Packet port_packet = Packet::receive(alfa_socket_fd);
    std::cout << "Connecting new next BETA Server: " << next_beta_ip << std::endl;
    next_beta_socket_fd = Network::connect_socket_ipv4(next_beta_ip, stoi(port_packet.payload.c_str()));
    std::cout << "Connected to new BETA!" << std::endl;
}

void BetaServer::accept_ring_connection() {
    int new_prev_beta_socket_fd;
    int old_socket_fd;
    struct sockaddr_in prev_beta_address;
    socklen_t prev_beta_address_len = sizeof(struct sockaddr_in);
    std::cout << "Handling RING Connection..." << std::endl;

    while (true) {
        new_prev_beta_socket_fd = accept(ring_socket_fd, (struct sockaddr*) &prev_beta_address, &prev_beta_address_len);

        if (new_prev_beta_socket_fd == -1) {
            std::cerr << "ERROR: Failed to accept new BETA" << std::endl;
            continue;
        }
        
        std::cout << "Accepted new BETA!" << std::endl;

        old_socket_fd = prev_beta_socket_fd.exchange(new_prev_beta_socket_fd);
        if (old_socket_fd > 0) {
            close(old_socket_fd); // This will cause old thread to exit
            std::cout << "Closed old BETA connection" << std::endl;
        }

        std::thread handle_beta_updates_thread(&BetaServer::handle_beta_updates, this);
        handle_beta_updates_thread.detach();
        std::cout << "NEW handle_beta_updates thread" << std::endl;
    }
}

void BetaServer::handle_beta_updates() {
    int my_socket = prev_beta_socket_fd.load();
    
    try {
        while (my_socket > 0) {
            if (prev_beta_socket_fd.load() != my_socket) {
                std::cout << "This beta connection was replaced, exiting thread" << std::endl;
                break;
            }
            
            Packet meta_packet = Packet::receive(my_socket);
            
            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                std::cout << "Received ERROR packet: " << meta_packet.payload << std::endl;
                continue;
            }
            
            // Handle other BETA messages here
            
            throw std::runtime_error(
                "Unexpected packet type " + std::to_string(meta_packet.type) +
                " received from beta server"
            );
        }
    } catch (const std::runtime_error& e) {
        std::cout << "Beta connection error: " << e.what() << std::endl;
        int expected = my_socket;
        if (prev_beta_socket_fd.compare_exchange_strong(expected, -1)) {
            close(my_socket);
        }
    }
    
    std::cout << "handle_beta_updates thread exiting" << std::endl;
}