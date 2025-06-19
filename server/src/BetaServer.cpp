#include <BetaServer.hpp>

#define PORT_BETA 8085


void BetaServer::run() {
    std::cout << "[ SETUP ] " << "Setting up BETA server..." << std::endl;
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
    std::cout << "[ SETUP ] " << "Setup finished!" << std::endl;

    sync_thread.join();
    ring_thread.join();
}

void BetaServer::handle_alfa_updates() {
    std::cout << "[ ALFA THREAD ] " << "Handling ALFA updates..." << std::endl;
    try {
        while (alfa_socket_fd > 0) {
            Packet meta_packet = Packet::receive(alfa_socket_fd);

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                std::cout << "[ ALFA THREAD ] " << "Received ERROR packet: " << meta_packet.payload << std::endl;
                continue;
            }

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::CLIENT)) {
                handle_client_updates(meta_packet);
                continue;
            }

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::SERVER)) {
                handle_new_betas(meta_packet);
                continue;
            }
            
            throw std::runtime_error(
                "[ ALFA THREAD ] Unexpected packet type " + std::to_string(meta_packet.type) + 
                " received from alfa server (expected USERNAME, SERVER, or ERROR)"
            );
        }
    } catch (const std::runtime_error& e) {
        close(alfa_socket_fd);
        exit(1);
    }
}

void BetaServer::handle_client_updates(Packet meta_packet) {
    Packet username_packet = Packet::receive(alfa_socket_fd);
    if (username_packet.type != static_cast<uint16_t>(Packet::Type::USERNAME)) {
        std::cout << "[ ALFA THREAD ] " << "ERROR: Client username not received from alfa server" << std::endl;
        return;
    }

    Packet update_packet = Packet::receive(alfa_socket_fd);

    std::cout << "[ ALFA THREAD ] " << "Received update_packet from alfa server: " << update_packet.payload << std::endl;

    if (update_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
        std::cout << "[ ALFA THREAD ] " << "Received ERROR packet: " << update_packet.payload << std::endl;
        return;
    }

    if (update_packet.type == static_cast<uint16_t>(Packet::Type::DELETE)) {
        handle_client_delete(update_packet.payload, username_packet.payload);
        return;
    }

    if (update_packet.type == static_cast<uint16_t>(Packet::Type::DATA)) {
        handle_client_upload(update_packet.payload, username_packet.payload, update_packet.total_size);
        return;
    }

    if (update_packet.type == static_cast<uint16_t>(Packet::Type::IP)) {
        handle_new_clients(update_packet.payload, username_packet.payload, meta_packet.total_size);
        return;
    }
    
    throw std::runtime_error(
        "[ ALFA THREAD ] Unexpected packet type " + std::to_string(update_packet.type) + 
        " received from alfa server (expected DATA, DELETE, IP or ERROR)"
    );
}

void BetaServer::handle_client_upload(const std::string filename, const std::string username, uint32_t total_packets) {
    std::cout << "[" << username << "]" << "handle_client_upload: " << filename << std::endl;
    FileManager::create_directory(backup_dir_path / username);
    Packet::receive_file(alfa_socket_fd, filename, backup_dir_path / username, total_packets);
}

void BetaServer::handle_client_delete(const std::string filename, const std::string username) {
    std::cout << "[ ALFA THREAD ] " << "[" << username << "] " << "handle_client_delete: " << filename << std::endl;
    FileManager::create_directory(backup_dir_path / username);
    FileManager::delete_file(backup_dir_path / username / filename);
}

void BetaServer::handle_new_clients(const std::string ip_first_client, const std::string username_first_client, int total_clients) {
    std::cout << "[ ALFA THREAD ] " << "Added new client device" << std::endl;
    devices->add_client(username_first_client, -1, ip_first_client);
    while(--total_clients > 0) {
        Packet username_packet = Packet::receive(alfa_socket_fd);
        if (username_packet.type != static_cast<uint16_t>(Packet::Type::USERNAME)) {
            std::cout << "[ ALFA THREAD ] " << "ERROR: Client username not received from alfa server" << std::endl;
            return;
        }
        Packet ip_packet = Packet::receive(alfa_socket_fd);
        if (ip_packet.type != static_cast<uint16_t>(Packet::Type::IP)) {
            std::cout << "[ ALFA THREAD ] " << "ERROR: Client IP not received from alfa server" << std::endl;
            return;
        }
        std::cout << "[ ALFA THREAD ] " << "Added new client device" << std::endl;
        devices->add_client(username_packet.payload, -1, ip_packet.payload);
    }
}

void BetaServer::handle_new_betas(Packet meta_packet) {
    int total_betas = meta_packet.total_size;
    std::cout << total_betas << std::endl;
    while(total_betas-- > 0) {    
        Packet ip_packet = Packet::receive(alfa_socket_fd);
        if (ip_packet.type != static_cast<uint16_t>(Packet::Type::IP)) {
            std::cout << "[ ALFA THREAD ] " << "ERROR: Beta server IP not received from alfa server" << std::endl;
            return;
        }
        Packet ring_port_packet = Packet::receive(alfa_socket_fd);
        if (ring_port_packet.type != static_cast<uint16_t>(Packet::Type::PORT)) {
            std::cout << "[ ALFA THREAD ] " << "ERROR: Beta server ring PORT not received from alfa server" << std::endl;
            return;
        }
        Packet id_packet = Packet::receive(alfa_socket_fd);
        if (id_packet.type != static_cast<uint16_t>(Packet::Type::ID)) {
            std::cout << "[ ALFA THREAD ] " << "ERROR: Beta server ID not received from alfa server" << std::endl;
            return;
        }

        int ring_port = stoi(ring_port_packet.payload);
        int id = stoi(id_packet.payload);

        BetaAddress beta(ip_packet.payload, ring_port, id);
        betas.push_back(beta);
        std::cout << "[ ALFA THREAD ] " << "NEW BETA ADDED: " << "[" << beta.id << "] " << beta.ip << ":" << beta.ring_port << std::endl;
    }

    // print all betas
    std::cout << "[ ALFA THREAD ] " << "BETA ADDRESSES:" << std::endl;
    for (BetaAddress& beta : betas) {
        std::cout << "[ ALFA THREAD ] " << "[" << beta.id << "] " << beta.ip << ":" << beta.ring_port << std::endl;
    }
}

void BetaServer::accept_ring_connection() {
    int new_prev_beta_socket_fd;
    int old_socket_fd;
    struct sockaddr_in prev_beta_address;
    socklen_t prev_beta_address_len = sizeof(struct sockaddr_in);
    std::cout << "[ RING THREAD ] " << "Handling RING Connection..." << std::endl;

    while (true) {
        new_prev_beta_socket_fd = accept(ring_socket_fd, (struct sockaddr*) &prev_beta_address, &prev_beta_address_len);

        if (new_prev_beta_socket_fd == -1) {
            std::cerr << "[ RING THREAD ] " << "ERROR: Failed to accept new BETA" << std::endl;
            continue;
        }
        
        std::cout << "[ RING THREAD ] " << "Accepted new BETA!" << std::endl;

        old_socket_fd = prev_beta_socket_fd.exchange(new_prev_beta_socket_fd);
        if (old_socket_fd > 0) {
            close(old_socket_fd); // This will cause old thread to exit
            std::cout << "[ RING THREAD ] " << "Closed old BETA connection" << std::endl;
        }

        std::thread handle_new_betas_thread(&BetaServer::handle_beta_updates, this);
        handle_new_betas_thread.detach();
        std::cout << "[ RING THREAD ] " << "NEW handle_new_betas thread" << std::endl;
    }
}

void BetaServer::handle_beta_updates() {
    int my_socket = prev_beta_socket_fd.load();
    
    try {
        while (my_socket > 0) {
            if (prev_beta_socket_fd.load() != my_socket) {
                std::cout << "[ RING THREAD ] " << "This beta connection was replaced, exiting thread" << std::endl;
                break;
            }
            
            Packet meta_packet = Packet::receive(my_socket);
            
            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                std::cout << "[ RING THREAD ] " << "Received ERROR packet: " << meta_packet.payload << std::endl;
                continue;
            }
            
            // Handle other BETA messages here
            
            throw std::runtime_error(
                "[ RING THREAD ] Unexpected packet type " + std::to_string(meta_packet.type) +
                " received from beta server"
            );
        }
    } catch (const std::runtime_error& e) {
        std::cout << "[ RING THREAD ] " << "Beta connection error: " << e.what() << std::endl;
        int expected = my_socket;
        if (prev_beta_socket_fd.compare_exchange_strong(expected, -1)) {
            close(my_socket);
        }
    }
    
    std::cout << "[ RING THREAD ] " << "handle_new_betas thread exiting" << std::endl;
}