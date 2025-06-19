#include <BetaManager.hpp>

void BetaManager::add_beta(int socket_fd) {
    std::unique_lock<std::shared_mutex> lock(access_beta_sockets);
    beta_sockets.push_back(socket_fd);
    std::cout << "Added BETA SOCKET: " << socket_fd << std::endl;
}

void BetaManager::remove_beta(int socket_fd) {
    std::lock_guard<std::shared_mutex> lock(access_beta_sockets);
    auto it = std::find(beta_sockets.begin(), beta_sockets.end(), socket_fd);

    if (it != beta_sockets.end()) {
        int removed_socket = *it;
        beta_sockets.erase(it);
        std::cout << "Removed BETA SOCKET: " << removed_socket << std::endl;
    } else {
        std::cout << "BETA SOCKET " << socket_fd << " not found for removal." << std::endl;
    }
}

void BetaManager::print_beta_sockets() const {
    std::vector<int> sockets_copy;
    {
        std::shared_lock<std::shared_mutex> lock(access_beta_sockets);
        sockets_copy = beta_sockets;
    }

    std::cout << "BETAS SOCKETS:" << std::endl;
    for (int socket_fd : sockets_copy) {
        std::cout << "> " << socket_fd << std::endl;
    }
}

void BetaManager::send_file(const fs::path filepath, const std::string username) const {
    std::vector<int> sockets_copy;
    {
        std::shared_lock<std::shared_mutex> lock(access_beta_sockets);
        sockets_copy = beta_sockets;
    
    }

    Packet username_packet(static_cast<uint16_t>(Packet::Type::USERNAME), 0, 0, username.length(), username.c_str());
    for (int socket_fd : sockets_copy) {
        username_packet.send(socket_fd);
        if (!Packet::send_file(socket_fd, filepath)) {
            Packet::send_error(socket_fd);
        }
    }
}

void BetaManager::send_client_device(const std::string ip, const std::string username) const {
    std::cout << "sending client device, ip = " << ip << " | username " << username << std::endl;
    std::vector<int> sockets_copy;
    {
        std::shared_lock<std::shared_mutex> lock(access_beta_sockets);
        sockets_copy = beta_sockets;
    
    }

    Packet username_packet(static_cast<uint16_t>(Packet::Type::USERNAME), 0, 0, username.length(), username.c_str());
    Packet ip_packet(static_cast<uint16_t>(Packet::Type::IP), 0, 0, ip.length(), ip.c_str());
    for (int socket_fd : sockets_copy) {
        username_packet.send(socket_fd);
        ip_packet.send(socket_fd);
    }
}

void BetaManager::delete_file(const std::string filename, const std::string username) const {
    std::vector<int> sockets_copy;
    {
        std::shared_lock<std::shared_mutex> lock(access_beta_sockets);
        sockets_copy = beta_sockets;
    }

    Packet username_packet(static_cast<uint16_t>(Packet::Type::USERNAME), 0, 0, username.length(), username.c_str());
    Packet delete_packet(static_cast<uint16_t>(Packet::Type::DELETE), 0, 0, filename.length(), filename.c_str());
    for (int socket_fd : sockets_copy) {
        username_packet.send(socket_fd);
        delete_packet.send(socket_fd);
    }
}

void BetaManager::send_new_beta_server(const std::string ip, int ring_port) const {
    std::vector<int> sockets_copy;
    {
        std::shared_lock<std::shared_mutex> lock(access_beta_sockets);
        sockets_copy = beta_sockets;
    }
    std::string ring_port_str = std::to_string(ring_port);
    std::string id_str = std::to_string(sockets_copy.size());

    Packet ip_packet(static_cast<uint16_t>(Packet::Type::SERVER), 0, 0, ip.length(), ip.c_str());
    Packet ring_port_packet(static_cast<uint16_t>(Packet::Type::SERVER), 0, 0, ring_port_str.length(), ring_port_str.c_str());
    Packet id_packet(static_cast<uint16_t>(Packet::Type::SERVER), 0, 0, id_str.length(), id_str.c_str());
    for (int socket_fd : sockets_copy) {
        ip_packet.send(socket_fd);
        ring_port_packet.send(socket_fd);
        id_packet.send(socket_fd);
    }
}