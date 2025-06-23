#include <BetaServer.hpp>
#include <Network.hpp>
#include <BetaManager.hpp>
#include <AlfaServer.hpp>

#define PORT_BETA 8085
#define HEARTBEAT_TIMEOUT 5

void BetaServer::run(int new_socket_fd) {
    if(new_socket_fd == -1){
        std::cout << "[ BETA SERVER ] " << "[ SETUP ] " << "Setting up BETA server..." << std::endl;
        alfa_socket_fd = Network::connect_socket_ipv4(ip_alfa, port_alfa);
        ring_port = Network::get_available_port();
        ring_socket_fd = Network::setup_socket_ipv4(ring_port);
        if(alfa_socket_fd == -1 || ring_socket_fd == -1)
            exit(1);
    } else {
        alfa_socket_fd = new_socket_fd;
    }

    Packet packet = Packet(static_cast<uint16_t>(Packet::Type::DATA), 0, 0, std::to_string(ring_port).length(), std::to_string(ring_port).c_str());
    packet.send(this->alfa_socket_fd);
    
    backup_dir_path = fs::path("./sync_dir_backup_" + std::to_string(this->alfa_socket_fd));
    FileManager::create_directory(backup_dir_path);

    running.store(true);
    std::thread sync_thread = std::thread(&BetaServer::handle_alfa_updates, this);
    std::thread ring_thread = std::thread(&BetaServer::accept_ring_connection, this);
    std::thread heartbeat_thread = std::thread(&BetaServer::heartbeat_timeout, this);
    std::cout << "[ BETA SERVER ] " << "[ SETUP ] " << "Setup finished!" << std::endl;

    sync_thread.join();
    ring_thread.join();
    heartbeat_thread.join();
    
    std::cout << "[ BETA SERVER ] " << "[ MAIN ] " << "All threads finished" << std::endl;
    
    if (become_alfa) {
        std::cout << "[ BETA SERVER ] " << "[ MAIN ] " << "Transitioning to ALFA server..." << std::endl;
        AlfaServer alfa(8088);
        alfa.become_alfa(clients, betas);
    }
}

void BetaServer::heartbeat_timeout() {
    Packet heartbeat_packet(static_cast<uint16_t>(Packet::Type::HEARTBEAT), 0, 0, 0, "");
    bool ok;
    std::cout << "[ BETA SERVER ] " << "[ HEARTBEAT THREAD ] " << "Starting heartbeat..." << std::endl;
    try {
        while (alfa_socket_fd > 0 && running.load() && !become_alfa && !reconnecting.load()) {

            heartbeat_packet.send(alfa_socket_fd);
            {
                std::unique_lock<std::mutex> lock(heartbeat_mutex);
                ok = heartbeat_cv.wait_for(lock, std::chrono::seconds(HEARTBEAT_TIMEOUT), [&]{ return heartbeat_received; });
                heartbeat_received = false;
            }

            if(ok) {
                std::cout << "[ BETA SERVER ] " << "[ HEARTBEAT THREAD ] " << "Alfa server is UP" << std::endl;
                continue;
            }
            // Se não recebeu heartbeat, considera alfa como caído
            throw std::runtime_error("Heartbeat timeout");
        }
    } catch (const std::runtime_error& e) {
        if (!reconnecting.load()) {
            close(alfa_socket_fd);
            std::cerr << "[ BETA SERVER ] " << "[ ERROR ] [ HEARTBEAT THREAD ] " <<  e.what() << std::endl;
        }
    }
    
    if (become_alfa) {
        std::cout << "[ BETA SERVER ] " << "[ HEARTBEAT THREAD ] " << "Becoming ALFA, exiting heartbeat thread..." << std::endl;
        return;
    }
    
    if (reconnecting.load()) {
        std::cout << "[ BETA SERVER ] " << "[ HEARTBEAT THREAD ] " << "Reconnecting in progress, exiting heartbeat thread..." << std::endl;
        return;
    }
    
    std::cout << "[ BETA SERVER ] " << "[ HEARTBEAT THREAD ] " << "Alfa server is DOWN, STARTING ELECTION..." << std::endl;
    start_election();
}

void BetaServer::handle_alfa_updates() {
    std::cout << "[ BETA SERVER ] " << "[ ALFA THREAD ] " << "Handling ALFA updates..." << std::endl;
    try {
        while (alfa_socket_fd > 0 && running.load() && !become_alfa && !reconnecting.load()) {
            Packet meta_packet = Packet::receive(alfa_socket_fd);

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                std::cout << "[ BETA SERVER ] " << "[ ALFA THREAD ] " << "Received ERROR packet: " << meta_packet.payload << std::endl;
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

            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::HEARTBEAT)) {
                {
                    std::unique_lock<std::mutex> lock(heartbeat_mutex);
                    heartbeat_received = true; 
                }
                // heartbeat_cv.notify_one();
                continue;
            }
            
            throw std::runtime_error(
                " Unexpected packet type " + std::to_string(meta_packet.type) + 
                " received from alfa server (expected CLIENT, SERVER, HEARTBEAT, or ERROR)"
            );
        }
    } catch (const std::runtime_error& e) {
        if (!reconnecting.load()) {
            close(alfa_socket_fd);
            std::cerr << "[ BETA SERVER ] " << "[ ERROR ] [ ALFA THREAD ] " << e.what() << std::endl;
        }
    }
    
    if (become_alfa) {
        std::cout << "[ BETA SERVER ] " << "[ ALFA THREAD ] " << "Becoming ALFA, exiting alfa thread..." << std::endl;
    } else if (reconnecting.load()) {
        std::cout << "[ BETA SERVER ] " << "[ ALFA THREAD ] " << "Reconnecting in progress, exiting alfa thread..." << std::endl;
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

    if (update_packet.type == static_cast<uint16_t>(Packet::Type::REMOVE)) {
        handle_removed_client(username_packet.payload);
        return;
    }

    if (update_packet.type == static_cast<uint16_t>(Packet::Type::DIRECTORY)) {
        fs::path user_sync_dir = backup_dir_path / ("sync_dir_" + username_packet.payload);
        FileManager::delete_all_files_in_directory(user_sync_dir);
        Packet::receive_multiple_files(alfa_socket_fd, user_sync_dir);
        return;
    }

    if (update_packet.type == static_cast<uint16_t>(Packet::Type::IP)) {
        Packet port_packet = Packet::receive(alfa_socket_fd);

        handle_new_clients(update_packet.payload, username_packet.payload, meta_packet.total_size, std::stoi(port_packet.payload));
        return;
    }

    throw std::runtime_error(
        " Unexpected packet type " + std::to_string(update_packet.type) + 
        " received from alfa server (expected DATA, DELETE, IP or ERROR)"
    );
}

void BetaServer::handle_client_upload(const std::string filename, const std::string username, uint32_t total_packets) {
    std::cout << "[ BETA SERVER ] " << "[" << username << "]" << "handle_client_upload: " << filename << std::endl;
    Packet::receive_file(alfa_socket_fd, filename, backup_dir_path / ("sync_dir_" + username), total_packets);
}

void BetaServer::handle_client_delete(const std::string filename, const std::string username) {
    std::cout << "[ BETA SERVER ] " << "[ ALFA THREAD ] " << "[" << username << "] " << "handle_client_delete: " << filename << std::endl;
    FileManager::delete_file(backup_dir_path / ("sync_dir_" + username) / filename);
}

void BetaServer::handle_removed_client(const std::string username) {
    Packet ip_packet = Packet::receive(alfa_socket_fd);
    if (ip_packet.type != static_cast<uint16_t>(Packet::Type::IP))
        throw std::runtime_error("Beta server IP not received from alfa server");

    Packet reconnection_port_packet = Packet::receive(alfa_socket_fd);
    if (reconnection_port_packet.type != static_cast<uint16_t>(Packet::Type::PORT))
        throw std::runtime_error("ERROR: Beta server ring PORT not received from alfa server");

    std::string ip = ip_packet.payload;
    int reconnection_port = stoi(reconnection_port_packet.payload);

    auto new_end = std::remove_if(clients.begin(), clients.end(), [&](auto device) {
        return device.username == username && 
            device.ip == ip && 
            device.port == reconnection_port;
    });

    std::cout << "[ ALFA THREAD ] " << "removed client device: " << username << " | " << ip << ":" << reconnection_port << std::endl;
    clients.erase(new_end, clients.end());

    std::cout << "[ ALFA THREAD ] DEVICES" << std::endl;
    for(int i = 0; i < clients.size(); i++) {
        std::cout << "[" << i << "] " << clients[i].username << " | " << clients[i].ip << ":" << clients[i].port << std::endl; 
    }
}

void BetaServer::handle_new_clients(const std::string ip_first_client, const std::string username_first_client, int total_clients, int port_first_client) {
    std::cout << "[ BETA SERVER ] " << "[ ALFA THREAD ] " << "Added new client device" << std::endl;
    clients.push_back(ClientAddress(username_first_client, ip_first_client, port_first_client));
    FileManager::create_directory(backup_dir_path / ("sync_dir_" + username_first_client));
    while(--total_clients > 0) {
        Packet username_packet = Packet::receive(alfa_socket_fd);
        if (username_packet.type != static_cast<uint16_t>(Packet::Type::USERNAME))
            throw std::runtime_error("Client username not received from alfa server");
        std::cout << "[ BETA SERVER ] " << "[ ALFA THREAD ] " << "Received new client username: " << username_packet.payload << std::endl;
        
        Packet ip_packet = Packet::receive(alfa_socket_fd);
        if (ip_packet.type != static_cast<uint16_t>(Packet::Type::IP))
            throw std::runtime_error("Client IP not received from alfa server");
        std::cout << "[ BETA SERVER ] " << "[ ALFA THREAD ] " << "Received new client IP: " << ip_packet.payload << std::endl;

        Packet port_packet = Packet::receive(alfa_socket_fd);
        if (port_packet.type != static_cast<uint16_t>(Packet::Type::PORT))
            throw std::runtime_error("Client PORT not received from alfa server");
        std::cout << "[ BETA SERVER ] " << "[ ALFA THREAD ] " << "Received new client PORT: " << port_packet.payload << std::endl;
        
        std::cout << "[ BETA SERVER ] " << "[ ALFA THREAD ] " << "Added new client device" << std::endl;
        clients.push_back(ClientAddress(username_packet.payload, ip_packet.payload, std::stoi(port_packet.payload)));
        FileManager::create_directory(backup_dir_path / ("sync_dir_" + username_packet.payload));
    }
}

void BetaServer::handle_new_betas(Packet meta_packet) {
    int total_betas = meta_packet.total_size;
    std::cout << "[ BETA SERVER ] " << total_betas << std::endl;
    
    // debug - O primeiro beta na lista é sempre este servidor
    bool first_beta = true;
    
    while(total_betas-- > 0) {    
        Packet ip_packet = Packet::receive(alfa_socket_fd);
        if (ip_packet.type != static_cast<uint16_t>(Packet::Type::IP))
            throw std::runtime_error("Beta server IP not received from alfa server");

        Packet ring_port_packet = Packet::receive(alfa_socket_fd);
        if (ring_port_packet.type != static_cast<uint16_t>(Packet::Type::PORT))
            throw std::runtime_error("ERROR: Beta server ring PORT not received from alfa server");
        
        Packet id_packet = Packet::receive(alfa_socket_fd);
        if (id_packet.type != static_cast<uint16_t>(Packet::Type::ID))
            throw std::runtime_error("Beta server ID not received from alfa server");

        int ring_port = stoi(ring_port_packet.payload);
        int id = stoi(id_packet.payload);

        BetaAddress beta(ip_packet.payload, ring_port, id);
        betas.push_back(beta);
        
        // debug - Se é o primeiro beta e tem a mesma porta, é este servidor
        if (first_beta && ring_port == this->ring_port) {
            my_id = id;
            std::cout << "[ BETA SERVER ] " << "[ ALFA THREAD ] " << "MY ID SET TO: " << my_id << " (port: " << ring_port << ")" << std::endl;
            first_beta = false;
        }
        
        std::cout << "[ BETA SERVER ] " << "[ ALFA THREAD ] " << "NEW BETA ADDED: " << "[" << beta.id << "] " << beta.ip << ":" << beta.ring_port << std::endl;
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
    std::cout << "[ BETA SERVER ] " << "[ RING THREAD ] " << "Handling RING Connection..." << std::endl;

    while (running.load() && !become_alfa) {
        new_prev_beta_socket_fd = accept(ring_socket_fd, (struct sockaddr*) &prev_beta_address, &prev_beta_address_len);

        if (new_prev_beta_socket_fd == -1) {
            std::cerr << "[ BETA SERVER ] " << "[ RING THREAD ] " << "ERROR: Failed to accept new BETA" << std::endl;
            continue;
        }
        
        std::cout << "[ BETA SERVER ] " << "[ RING THREAD ] " << "Accepted new BETA!" << std::endl;

        old_socket_fd = prev_beta_socket_fd.exchange(new_prev_beta_socket_fd);
        if (old_socket_fd > 0) {
            close(old_socket_fd); // This will cause old thread to exit
            std::cout << "[ BETA SERVER ] " << "[ RING THREAD ] " << "Closed old BETA connection" << std::endl;
        }

        std::thread handle_new_betas_thread(&BetaServer::handle_beta_updates, this);
        handle_new_betas_thread.detach();
        std::cout << "[ BETA SERVER ] " << "[ RING THREAD ] " << "NEW handle_new_betas thread" << std::endl;
    }
}

void BetaServer::handle_beta_updates() {
    int my_socket = prev_beta_socket_fd.load();
    
    try {
        while (my_socket > 0 && running.load() && !become_alfa) {
            if (prev_beta_socket_fd.load() != my_socket) {
                std::cout << "[ BETA SERVER ] " << "[ RING THREAD ] " << "This beta connection was replaced, exiting thread" << std::endl;
                break;
            }
            
            Packet meta_packet = Packet::receive(my_socket);
            
            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
                std::cout << "[ BETA SERVER ] " << "[ RING THREAD ] " << "Received ERROR packet: " << meta_packet.payload << std::endl;
                continue;
            }
            
            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::ELECTION)) {
                int candidate_id = stoi(meta_packet.payload);
                std::cout << "[ BETA SERVER ] " << "[ RING THREAD ] " << "Received ELECTION message with candidate ID: " << candidate_id << std::endl;
                handle_election_message(candidate_id);
                continue;
            }
            
            if (meta_packet.type == static_cast<uint16_t>(Packet::Type::ELECTED)) {
                int coordinator_id = stoi(meta_packet.payload);
                std::cout << "[ BETA SERVER ] " << "[ RING THREAD ] " << "Received ELECTED message with coordinator ID: " << coordinator_id << std::endl;
                handle_elected_message(coordinator_id);
                continue;
            }
            
            // Handle other BETA messages here
            
            throw std::runtime_error(
                "[ RING THREAD ] Unexpected packet type " + std::to_string(meta_packet.type) +
                " received from beta server"
            );
        }
    } catch (const std::runtime_error& e) {
        std::cout << "[ BETA SERVER ] " << " [ ERROR ] [ RING THREAD ] " << e.what() << std::endl;
        int expected = my_socket;
        if (prev_beta_socket_fd.compare_exchange_strong(expected, -1)) {
            close(my_socket);
        }
    }
    
    std::cout << "[ BETA SERVER ] " << "[ RING THREAD ] " << "handle_new_betas thread exiting" << std::endl;
}

void BetaServer::close_sockets() {
    if (alfa_socket_fd > 0) close(alfa_socket_fd);
    if (ring_socket_fd > 0) close(ring_socket_fd);
    if (next_beta_socket_fd > 0) close(next_beta_socket_fd);
    int prev_fd = prev_beta_socket_fd.load();
    if (prev_fd > 0) close(prev_fd);
    std::cout << "[ BETA SERVER ] " << "[ CLOSING ] " << "Closed all sockets" << std::endl;
}

// ========================================= //
// ============= ELECTION METHODS ========== //
// ========================================= //

void BetaServer::start_election() {
    std::lock_guard<std::mutex> lock(election_mutex);
    
    std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "Starting election with my ID: " << my_id << std::endl;
    
    if (election_in_progress.load()) {
        std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "Election already in progress, ignoring..." << std::endl;
        return;
    }
    
    election_in_progress.store(true);
    is_participant.store(true);
    elected_coordinator.store(-1);  // Reset coordinator
    
    send_election_message(my_id);
    std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "Sent initial election message with my ID: " << my_id << std::endl;
}

void BetaServer::handle_election_message(int candidate_id) {
    std::lock_guard<std::mutex> lock(election_mutex);
    
    std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "Processing election message. Candidate: " << candidate_id << ", My ID: " << my_id << std::endl;
    
    if (candidate_id == my_id) {
        become_coordinator();
        return;
    }
    
    if (candidate_id > my_id) {
        send_election_message(candidate_id);
        return;
    }
    
    if (candidate_id < my_id && !is_participant.load()) {
        is_participant.store(true);
        send_election_message(my_id);
        return;
    }
    
}

void BetaServer::handle_elected_message(int coordinator_id) {
    std::lock_guard<std::mutex> lock(election_mutex);
    
    if (coordinator_id == my_id) {
        election_in_progress.store(false);
        is_participant.store(false);
        return;
    }
    
    elected_coordinator.store(coordinator_id);
    is_coordinator.store(false);
    election_in_progress.store(false);
    is_participant.store(false);
    
    std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "New coordinator elected: " << coordinator_id << ". Forwarding elected message." << std::endl;
    send_elected_message(coordinator_id);
    accept_new_alfa_connection(coordinator_id);
}

void BetaServer::accept_new_alfa_connection(int coordinator_id) {
    std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "Connecting to new coordinator with ID: " << coordinator_id << std::endl;
    
    reconnecting.store(true);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string coordinator_ip;
    bool found = false;
    for (auto beta : betas) {
        if(beta.id == coordinator_id) {
            coordinator_ip = beta.ip;
            found = true;
            break;
        }
    }

    betas = std::vector<BetaAddress>();

    if (!found) {
        std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "ERROR: Coordinator IP not found for ID: " << coordinator_id << std::endl;
        reconnecting.store(false);
        return;
    }

    std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "Attempting to connect to new coordinator at: " << coordinator_ip << ":" << port_alfa << std::endl;

    int new_alpha_socket;
    int max_attempts = 10;
    int attempts = 0;

    while (attempts < max_attempts) {
        new_alpha_socket = Network::connect_socket_ipv4(coordinator_ip, port_alfa);

        if (new_alpha_socket >= 0) {
            
            if (alfa_socket_fd > 0) {
                close(alfa_socket_fd);
                std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "Closed old alfa connection" << std::endl;
            }
            
            alfa_socket_fd = new_alpha_socket;
            
            Packet packet = Packet(static_cast<uint16_t>(Packet::Type::DATA), 0, 0, std::to_string(ring_port).length(), std::to_string(ring_port).c_str());
            packet.send(this->alfa_socket_fd);
            
            {
                std::unique_lock<std::mutex> lock(heartbeat_mutex);
                heartbeat_received = false;
            }
            
            reconnecting.store(false);
            
            std::thread sync_thread = std::thread(&BetaServer::handle_alfa_updates, this);
            std::thread heartbeat_thread = std::thread(&BetaServer::heartbeat_timeout, this);
            
            sync_thread.detach();
            heartbeat_thread.detach();
            
            std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "Successfully connected to new coordinator!" << std::endl;
            return;
        }

        attempts++;
        std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "Connection attempt " << attempts << " failed, retrying..." << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "ERROR: Failed to connect to new coordinator after " << max_attempts << " attempts" << std::endl;
    reconnecting.store(false);
}

void BetaServer::send_election_message(int candidate_id) {
    int next_socket = get_next_beta_socket();
    if (next_socket <= 0) {
        std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "ERROR: No next beta socket available!" << std::endl;
        return;
    }
    
    std::string payload = std::to_string(candidate_id);
    Packet election_packet(static_cast<uint16_t>(Packet::Type::ELECTION), 0, 0, payload.length(), payload);
    
    try {
        election_packet.send(next_socket);
    } catch (const std::exception& e) {
        std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "ERROR sending election message: " << e.what() << std::endl;
    }
    
    close(next_socket);
}

void BetaServer::send_elected_message(int coordinator_id) {
    int next_socket = get_next_beta_socket();
    if (next_socket <= 0) {
        std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "ERROR: No next beta socket available!" << std::endl;
        return;
    }
    
    std::string payload = std::to_string(coordinator_id);
    Packet elected_packet(static_cast<uint16_t>(Packet::Type::ELECTED), 0, 0, payload.length(), payload);
    
    try {
        elected_packet.send(next_socket);
    } catch (const std::exception& e) {
        std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "ERROR sending elected message: " << e.what() << std::endl;
    }
    
    close(next_socket);
}

int BetaServer::get_next_beta_socket() {
    if (betas.empty()) {
        std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "No betas available in ring!" << std::endl;
        return -1;
    }
    
    int my_position = -1;
    for (size_t i = 0; i < betas.size(); i++) {
        if (betas[i].id == my_id) {
            my_position = i;
            break;
        }
    }
    
    if (my_position == -1) {
        std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "Could not find myself in beta list!" << std::endl;
        return -1;
    }

    int i = 1;
    int next_socket = -1;
    while (i <= betas.size()) {
        int next_position = (my_position + i) % betas.size();
        BetaAddress& next_beta = betas[next_position];
        
        std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "Connecting to next beta [" << next_beta.id << "] " << next_beta.ip << ":" << next_beta.ring_port << std::endl;
        
        next_socket = Network::connect_socket_ipv4(next_beta.ip, next_beta.ring_port);
        
        if (next_socket > 0) 
            break;

        std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "ERROR: Could not connect to next beta! Trial " << i << std::endl;
        i++;
        
        if (i > betas.size()) {
            std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "ERROR: Failed to connect to any beta in the ring" << std::endl;
            return -1;
        }
    }
    
    return next_socket;
}

void BetaServer::become_coordinator() {
    std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "=== BECOMING COORDINATOR ===" << std::endl;
    std::cout << "[ BETA SERVER ] " << "[ ELECTION ] " << "Beta server with ID: " << my_id << " will become ALFA!" << std::endl;
    
    is_coordinator.store(true);
    elected_coordinator.store(my_id);
    is_participant.store(false);
    
    send_elected_message(my_id);
    
    // Setup alfa server
    become_alfa = true;
    FileManager::move_files_between_directories(backup_dir_path, fs::path("./sync_dir_server"));
    running.store(false);
    close_sockets();
}