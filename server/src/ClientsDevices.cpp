#include <ClientsDevices.hpp>

bool ClientsDevices::add_client_socket(const std::string& username, int sockfd)
{
    auto it = clients_sockets.find(username);
    if (it != clients_sockets.end()) {
        if (clients_sockets[username].size() >= 2) {
            std::cout << "Maximum of 2 connections reached for user " << username << ".\n";
            close(sockfd);
            // SEND MESSAGE TO TERMINATE CLIENT
            return false;
        }
        clients_sockets[username].push_back(sockfd);
    } else {
        clients_sockets[username].push_back(sockfd);
    }

    print_clients_sockets();  // <-- DANGEROUS IF USING THE SAME MUTEX
    return true;
}

void ClientsDevices::remove_client_socket(const std::string& username, int sockfd)
{
    auto it = clients_sockets.find(username);
    if (it != clients_sockets.end()) {
        auto& sockets = it->second;

        // Remove sockfd from vector
        sockets.erase(std::remove(sockets.begin(), sockets.end(), sockfd), sockets.end());

        std::cout << "Socket " << sockfd << " removed from user " << username << ".\n";

        // If no sockets remain, remove the user from the map
        if (sockets.empty()) {
            clients_sockets.erase(it);
            std::cout << "User " << username << " removed from map (no active connections).\n";
        }
    } else {
        std::cout << "User " << username << " not found for removal.\n";
    }

    print_clients_sockets();
}

void ClientsDevices::print_clients_sockets() {
    // Check if the map is empty
    if (clients_sockets.empty()) {
        std::cout << "The client map is empty!" << std::endl;
    }

    // Iterate through the unordered_map
    for (const auto& pair : clients_sockets) {
        const std::string& username = pair.first;
        const std::vector<int>& sockets = pair.second;

        std::cout << "User: " << username << " - Download connections: ";

        for (int socket : sockets) {
            std::cout << socket << " ";
        }
        std::cout << std::endl;
    }
}

int ClientsDevices::get_other_device_socket(const std::string& username, int current_sockfd) {
    auto it = clients_sockets.find(username);
    if (it != clients_sockets.end() && it->second.size() > 1) {
        for (int sockfd : it->second) {
            if (sockfd != current_sockfd) {
                return sockfd;
            }
        }
    }
    return -1;
}
