#include <BetaManager.hpp>

void BetaManager::add_beta(int socket_fd) {
    std::lock_guard<std::mutex> lock(access_beta_sockets);
    beta_sockets.push_back(socket_fd);
    std::cout << "Added BETA SOCKET: " << socket_fd << std::endl;
}

void BetaManager::remove_beta(int socket_fd) {
    std::lock_guard<std::mutex> lock(access_beta_sockets);
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
        std::lock_guard<std::mutex> lock(access_beta_sockets);
        sockets_copy = beta_sockets;
    }

    std::cout << "BETAS SOCKETS:" << std::endl;
    for (int socket_fd : sockets_copy) {
        std::cout << "> " << socket_fd << std::endl;
    }
}

void BetaManager::send_file(const fs::path filepath) const {
    std::vector<int> sockets_copy;
    {
        std::lock_guard<std::mutex> lock(access_beta_sockets);
        sockets_copy = beta_sockets;
    }

    for (int socket_fd : sockets_copy) {
        if (!Packet::send_file(socket_fd, filepath)) {
            Packet::send_error(socket_fd);
        }
    }
}

void BetaManager::delete_file(const std::string filename) const {
    std::vector<int> sockets_copy;
    {
        std::lock_guard<std::mutex> lock(access_beta_sockets);
        sockets_copy = beta_sockets;
    }

    Packet packet(static_cast<uint16_t>(Packet::Type::DELETE), 0, 0, filename.length(), filename.c_str());
    for (int socket_fd : sockets_copy) {
        packet.send(socket_fd);
    }
}

