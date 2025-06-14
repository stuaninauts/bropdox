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

using namespace std;

struct Device {
    int socket_fd;
    std::string ip;
    Device(int socket_fd, std::string ip) : socket_fd(socket_fd), ip(ip) {}
};

class ClientsDevices {

public:
    ClientsDevices() = default;
    bool add_client(const std::string& username, int sockfd, const std::string ip);
    void remove_client(const std::string& username, int sockfd);
    int get_other_device_socket(const std::string& username, int sockfd);
    void print_clients();
    
private:
    std::unordered_map<std::string, std::vector<Device>> clients;
};
#endif // CLIENTSDEVICES_HPP