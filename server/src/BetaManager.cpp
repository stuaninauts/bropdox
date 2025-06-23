#include <BetaManager.hpp>

void BetaManager::add_beta(int new_beta_socket_fd, const std::string& new_beta_ip, int new_beta_ring_port) {
    next_beta_id++;

    std::cout << "Adding new beta with ID: " << next_beta_id << std::endl;

    BetaInfo new_beta(new_beta_socket_fd, new_beta_ip, new_beta_ring_port, next_beta_id);
    
    send_new_beta_server(new_beta);
    
    {
        std::lock_guard<std::shared_mutex> lock(access_betas);
        betas.push_back(new_beta);
        std::cout << "Added BETA SOCKET: " << new_beta_socket_fd << std::endl;
    }
}

void BetaManager::remove_beta(int socket_fd) {
    std::lock_guard<std::shared_mutex> lock(access_betas);
    auto it = std::remove_if(betas.begin(), betas.end(),
        [socket_fd](auto beta) {
            return beta.socket_fd == socket_fd;
        }
    );

    if (it != betas.end()) {
        auto removed_socket = *it;
        betas.erase(it);
        std::cout << "Removed BETA SOCKET: " << socket_fd << std::endl;
    } else {
        std::cout << "BETA SOCKET " << socket_fd << " not found for removal." << std::endl;
    }
}

void BetaManager::print_betas() const {
    std::vector<BetaInfo> betas_copy;
    {
        std::shared_lock<std::shared_mutex> lock(access_betas);
        betas_copy = betas;
    }

    std::cout << "BETAS SOCKETS:" << std::endl;
    for (BetaInfo& beta : betas_copy) {
        std::cout << "> " << beta.socket_fd << std::endl;
    }
}

void BetaManager::send_file(const fs::path filepath, const std::string username) const {
    std::vector<BetaInfo> betas_copy;
    {
        std::shared_lock<std::shared_mutex> lock(access_betas);
        betas_copy = betas;
    }

    std::cout << "PRINTANDO BETAS COPY: " << filepath << " to all betas" << std::endl;
    for (const BetaInfo& beta : betas_copy) {
        std::cout << "ID: " << beta.id << " Beta socket_fd: " << beta.socket_fd << std::endl;
    }

    std::cout << "PRINTANDO BETAS: " << filepath << " to all betas" << std::endl;
    for (const BetaInfo& beta : betas) {
        std::cout << "ID: " << beta.id << " Beta socket_fd: " << beta.socket_fd << std::endl;
    }

    std::lock_guard<std::mutex> lock(write_beta_socket);
    Packet meta_packet(static_cast<uint16_t>(Packet::Type::CLIENT), 0, 1, 0, "");
    Packet username_packet(static_cast<uint16_t>(Packet::Type::USERNAME), 0, 0, username.length(), username.c_str());
    for (BetaInfo& beta : betas_copy) {
        std::cout << "Sending file from " << filepath << " to beta with socket_fd: " << beta.socket_fd << std::endl;
        meta_packet.send(beta.socket_fd);
        username_packet.send(beta.socket_fd);
        if (!Packet::send_file(beta.socket_fd, filepath)) {
            Packet::send_error(beta.socket_fd);
        }
    }
}

void BetaManager::send_client_device(const std::string ip, const std::string username, int port) const {
    std::cout << "sending client device, ip = " << ip << " | username " << username << std::endl;
    std::vector<BetaInfo> betas_copy;
    {
        std::shared_lock<std::shared_mutex> lock(access_betas);
        betas_copy = betas;
    
    }
    std::lock_guard<std::mutex> lock(write_beta_socket);
    Packet meta_packet(static_cast<uint16_t>(Packet::Type::CLIENT), 0, 1, 0, "");
    Packet username_packet(static_cast<uint16_t>(Packet::Type::USERNAME), 0, 0, username.length(), username.c_str());
    Packet ip_packet(static_cast<uint16_t>(Packet::Type::IP), 0, 0, ip.length(), ip.c_str());
    Packet port_packet(static_cast<uint16_t>(Packet::Type::PORT), 0, 0, std::to_string(port).length(), std::to_string(port).c_str());
    for (BetaInfo& beta : betas_copy) {
        meta_packet.send(beta.socket_fd);
        username_packet.send(beta.socket_fd);
        ip_packet.send(beta.socket_fd);
        port_packet.send(beta.socket_fd);
    }
}

void BetaManager::delete_file(const std::string filename, const std::string username) const {
    std::vector<BetaInfo> betas_copy;
    {
        std::shared_lock<std::shared_mutex> lock(access_betas);
        betas_copy = betas;
    }
    std::lock_guard<std::mutex> lock(write_beta_socket);
    Packet meta_packet(static_cast<uint16_t>(Packet::Type::CLIENT), 0, 1, 0, "");
    Packet username_packet(static_cast<uint16_t>(Packet::Type::USERNAME), 0, 0, username.length(), username.c_str());
    Packet delete_packet(static_cast<uint16_t>(Packet::Type::DELETE), 0, 0, filename.length(), filename.c_str());
    for (BetaInfo& beta : betas_copy) {
        meta_packet.send(beta.socket_fd);
        username_packet.send(beta.socket_fd);
        delete_packet.send(beta.socket_fd);
    }
}

void BetaManager::send_new_beta_server(BetaInfo new_beta) const {
    std::vector<BetaInfo> betas_copy;
    {
        std::shared_lock<std::shared_mutex> lock(access_betas);
        betas_copy = betas;
    }
    std::lock_guard<std::mutex> lock(write_beta_socket);
    std::cout << "send new_beta_server" << std::endl;
    std::string ring_port_str = std::to_string(new_beta.ring_port);
    std::string id_str = std::to_string(new_beta.id);

    Packet meta_packet(static_cast<uint16_t>(Packet::Type::SERVER), 0, 1, 0, "");
    Packet ip_packet(static_cast<uint16_t>(Packet::Type::IP), 0, 0, new_beta.ip.length(), new_beta.ip.c_str());
    Packet ring_port_packet(static_cast<uint16_t>(Packet::Type::PORT), 0, 0, ring_port_str.length(), ring_port_str.c_str());
    Packet id_packet(static_cast<uint16_t>(Packet::Type::ID), 0, 0, id_str.length(), id_str.c_str());
    for (BetaInfo& beta : betas_copy) {
        meta_packet.send(beta.socket_fd);
        ip_packet.send(beta.socket_fd);
        ring_port_packet.send(beta.socket_fd);
        id_packet.send(beta.socket_fd);
    }
}

void BetaManager::send_heartbeat(int beta_socket_fd) const {
    std::lock_guard<std::mutex> lock(write_beta_socket);
    Packet heartbeat_packet(static_cast<uint16_t>(Packet::Type::HEARTBEAT), 0, 0, 0, "");
    heartbeat_packet.send(beta_socket_fd);
}

void BetaManager::send_all_betas_to_new_beta(int new_beta_socket_fd) const {
    std::vector<BetaInfo> betas_copy;
    {
        std::shared_lock<std::shared_mutex> lock(access_betas);
        betas_copy = betas;
    }
    std::lock_guard<std::mutex> lock(write_beta_socket);
    int seqn = 0;
    Packet meta_packet(static_cast<uint16_t>(Packet::Type::SERVER), seqn, betas_copy.size(), 0, "");
    meta_packet.send(new_beta_socket_fd);
    for (BetaInfo& beta : betas_copy) {
        seqn++;
        std::string ring_port_str = std::to_string(beta.ring_port);
        std::string id_str = std::to_string(beta.id);
        Packet ip_packet(static_cast<uint16_t>(Packet::Type::IP), 0, 0, beta.ip.length(), beta.ip.c_str());
        Packet ring_port_packet(static_cast<uint16_t>(Packet::Type::PORT), 0, 0, ring_port_str.length(), ring_port_str.c_str());
        Packet id_packet(static_cast<uint16_t>(Packet::Type::ID), 0, 0, id_str.length(), id_str.c_str());
        ip_packet.send(new_beta_socket_fd);
        ring_port_packet.send(new_beta_socket_fd);
        id_packet.send(new_beta_socket_fd);
    }
}

void BetaManager::send_removed_client_device(const std::string ip, const std::string username, int reconnection_port) const {
    std::cout << "sending removed client device, ip = " << ip << " | username " << username << " | reconnection_port " << reconnection_port << std::endl;
    std::vector<BetaInfo> betas_copy;
    {
        std::shared_lock<std::shared_mutex> lock(access_betas);
        betas_copy = betas;
    }
    std::lock_guard<std::mutex> lock(write_beta_socket);

    std::string reconnection_port_str = std::to_string(reconnection_port);
    Packet meta_packet(static_cast<uint16_t>(Packet::Type::CLIENT), 0, 1, 0, "");
    Packet username_packet(static_cast<uint16_t>(Packet::Type::USERNAME), 0, 0, username.length(), username.c_str());
    Packet remove_packet(static_cast<uint16_t>(Packet::Type::REMOVE), 0, 0, ip.length(), ip.c_str());
    Packet ip_packet(static_cast<uint16_t>(Packet::Type::IP), 0, 0, ip.length(), ip.c_str());
    Packet port_packet(static_cast<uint16_t>(Packet::Type::PORT), 0, 0, reconnection_port_str.length(), reconnection_port_str.c_str());
    for (BetaInfo& beta : betas_copy) {
        meta_packet.send(beta.socket_fd);
        username_packet.send(beta.socket_fd);
        remove_packet.send(beta.socket_fd);
        ip_packet.send(beta.socket_fd);
        port_packet.send(beta.socket_fd);
        std::cout << "sending client device, ip = " << ip << " | username " << username << " | reconnection_port " << reconnection_port << std::endl;
    }
}
