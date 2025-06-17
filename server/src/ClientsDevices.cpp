#include <ClientsDevices.hpp>

bool ClientsDevices::add_client(const std::string &username, int socket_fd, const std::string ip) {
    std::unique_lock<std::shared_mutex> lock(access_clients);

    auto it = clients.find(username);
    if (it == clients.end() || it->second.size() <= 1) {
        clients[username].push_back(Device(socket_fd, ip));
        print_clients_unlocked();
        return true;
    }

    std::cout << "Maximum of 2 connections reached for user " << username << ".\n";
    close(socket_fd);
    print_clients_unlocked();
    return false;
}

void ClientsDevices::remove_client(const std::string &username, int socket_fd) {
    std::unique_lock<std::shared_mutex> lock(access_clients);

    auto it = clients.find(username);

    if (it == clients.end()) {
        std::cout << "User " << username << " not found for removal." << std::endl;
        print_clients_unlocked();
        return;
    }

    auto &user_devices = it->second;
    std::erase_if(user_devices, [socket_fd] (const auto& device) {
        return device.socket_fd == socket_fd;
    });
    std::cout << "Socket " << socket_fd << " removed from user " << username << ".\n";

    if (user_devices.empty()) {
        clients.erase(it);
        std::cout << "User " << username << " removed from map (no active connections).\n";
    }

    print_clients_unlocked();
}


void ClientsDevices::print_clients() const {
    std::shared_lock<std::shared_mutex> lock(access_clients);
    print_clients_unlocked();
}

int ClientsDevices::get_other_device_socket(const std::string &username, int current_socket_fd) const {
    std::shared_lock<std::shared_mutex> lock(access_clients);

    auto it = clients.find(username);
    if (it == clients.end() || it->second.size() <= 1) {
        return -1;
    }

    for (const auto& device : it->second) {
        if (device.socket_fd != current_socket_fd) {
            return device.socket_fd;
        }
    }

    return -1;
}

void ClientsDevices::print_clients_unlocked() const {
    if (clients.empty()) {
        std::cout << "The client map is empty!" << std::endl;
        return;
    }

    std::cout << "--- Client Map Status ---" << std::endl;
    for (const auto &pair : clients) {
        const std::string &username = pair.first;
        const auto &user_devices = pair.second;

        std::cout << "User: " << username << " - Devices: ";

        for (const auto& device : user_devices) {
            std::cout << device.socket_fd << "(" << device.ip << ") | ";
        }
        std::cout << std::endl;
    }
    std::cout << "-------------------------" << std::endl;
}
