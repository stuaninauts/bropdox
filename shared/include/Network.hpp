#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <iostream>
#include <string>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

class Network {
public:
    static int connect_socket_ipv4(std::string ip, int port);
    static int setup_socket_ipv4(int port, int backlog = 10);
    static int get_available_port();
};

#endif // NETWORK_HPP
