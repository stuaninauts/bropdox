#ifndef CLIENTSDEVICES_HPP
#define CLIENTSDEVICES_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <mutex>
#include <netinet/in.h>
#include <thread> 
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <shared_mutex>
#include <Packet.hpp>

using namespace std;

struct Device {
    int socket_fd;
    std::string ip;
    int port_backup;
    Device(int socket_fd, std::string ip, int port_backup) : socket_fd(socket_fd), ip(ip), port_backup(port_backup) {}
};

class ClientsDevices {

public:
    ClientsDevices() = default;
    bool add_client(const std::string& username, int socket_fd, const std::string ip, int port_backup);
    void remove_client(const std::string& username, int socket_fd);
    int get_other_device_socket(const std::string& username, int current_socket_fd) const;
    void print_clients() const;
    void send_all_devices_to_beta(int beta_socket_fd) const;
    
private:
    int device_count;
    std::unordered_map<std::string, std::vector<Device>> clients;
    mutable std::shared_mutex access_clients;
    void print_clients_unlocked() const;
};
#endif // CLIENTSDEVICES_HPP
