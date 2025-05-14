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
#include <Packet.hpp>
#include <ServerFileManager.hpp>

class ServerCommunicationManager {
public:
    ServerCommunicationManager(ServerFileManager& file_manager_) 
        : file_manager(file_manager_) {
    }
    
    void create_sockets(int socket_cmd);
    void receive_packet();
    void read_cmd();
    void handle_list_server();

    private:
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

    ServerFileManager& file_manager;
};

#endif // SERVERCOMMUNICATIONMANAGER_HPP
