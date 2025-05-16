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

class ClientsDevices {

public:
    ClientsDevices() = default;
    bool add_client_socket(const std::string& username, int sockfd);
    void remove_client_socket(const std::string& username, int sockfd);
    int get_other_device_socket(const std::string& username, int sockfd);
    void print_clients_sockets();
    
private:
    std::unordered_map<std::string, std::vector<int>> clients_sockets;
};
#endif // CLIENTSDEVICES_HPP