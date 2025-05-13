#ifndef CLIENTCOMMUNICATIONMANAGER_HPP
#define CLIENTCOMMUNICATIONMANAGER_HPP

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string>

class ClientCommunicationManager {
public:

    bool get_sockets_ports();
    int connect_to_server(const std::string server_ip, int port, const std::string username);

private:
    std::string username;
    // ip
    std::string server_ip;

    // ports
    int port_cmd;
    int port_upload;
    int port_download;

    // sockets
    int socket_cmd;
    int socket_upload;
    int socket_download;


    void close_sockets();

    bool send_username();
    bool connect_socket_to_server(int sockfd, int port);
    bool connect_socket_cmd();
};

#endif // CLIENTCOMMUNICATIONMANAGER_HPP
