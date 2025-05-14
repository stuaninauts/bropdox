#ifndef SERVERCOMMUNICATIONMANAGER_HPP
#define SERVERCOMMUNICATIONMANAGER_HPP

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <ClientsDevices.hpp>
#include <mutex>
#include <thread>
#include <memory>

class ServerCommunicationManager {

public:
    void run_client_session(int socket_cmd, std::string username, std::shared_ptr<ClientsDevices> devices);
    std::shared_ptr<ClientsDevices> devices;

private:
    std::string username;
    
    // sockets
    int socket_upload;
    int socket_download;
    int socket_cmd;
    // ports
    int port_upload;
    int port_download;
    int port_cmd;

    bool connect_socket_to_client(int *sockfd, int *port);
    void close_sockets();
};

#endif // SERVERCOMMUNICATIONMANAGER_HPP
