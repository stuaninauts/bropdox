#include <ClientsDevices.hpp>

bool ClientsDevices::add_client(const std::string &username, int socket_fd, const std::string ip) {
    auto it = clients.find(username);
    if (it == clients.end() || clients[username].size() <= 1) {
        clients[username].push_back(Device(socket_fd, ip));
        print_clients();
        return true;
    }

    std::cout << "Maximum of 2 connections reached for user " << username << ".\n";
    close(socket_fd);
    print_clients(); 
    return false;
}

void ClientsDevices::remove_client(const std::string &username, int socket_fd) {
    auto it = clients.find(username);

    if (it == clients.end()) {
        std::cout << "User " << username << " not found for removal." << std::endl;
        print_clients();
        return;
    }

    auto &user_devices = it->second;
    std::erase_if(user_devices, [socket_fd] (const auto& device) { return device.socket_fd == socket_fd; });
    std::cout << "Socket " << socket_fd << " removed from user " << username << ".\n";

    if (user_devices.empty()) {
        clients.erase(it);
        std::cout << "User " << username << " removed from map (no active connections).\n";
    }

    print_clients();
}

void ClientsDevices::print_clients() {
    if (clients.empty()) {
        std::cout << "The client map is empty!" << std::endl;
        return;
    }

    for (const auto &pair : clients) {
        const std::string &username = pair.first;
        auto &user_devices = pair.second;

        std::cout << "User: " << username << " - Download connections: ";

        for (auto device : user_devices) {
            std::cout << device.socket_fd << "(" << device.ip << ") | ";
        }
        std::cout << std::endl;
    }
}

int ClientsDevices::get_other_device_socket(const std::string &username, int current_socket_fd) {
    auto it = clients.find(username);
    if (it == clients.end() || it->second.size() <= 1) {
        return -1;
    }

    for (auto device : it->second) {
        if (device.socket_fd != current_socket_fd) {
            return device.socket_fd;
        }
    }
}
