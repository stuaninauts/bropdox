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

class ServerCommunicationManager {
public:

    void create_sockets(int socket_cmd);
private:
    int socket_upload;
    int socket_download;
    int socket_cmd;

    int port_upload;
    int port_download;
    int port_cmd;

    bool connect_to_client(int *sockfd, int *port);

    void close_sockets();
    void send_username();
};

#endif // SERVERCOMMUNICATIONMANAGER_HPP
