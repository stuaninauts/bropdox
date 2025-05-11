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
    ClientCommunicationManager(const std::string& ip, int port, const std::string& username)
        : server_ip(ip), port(port), username(username), socketfd(-1) {}

    ~ClientCommunicationManager() {
        if (socketfd != -1) {
            close(socketfd); // Make sure to close the socket when done
        }
    }

    void send_username();

    int connect_to_server();
private:
    int socketfd;
    std::string server_ip;
    int port;
    std::string username;

    struct sockaddr_in create_server_address(struct hostent* server);
    void create_sockets();
    void connect_sockets(struct sockaddr_in serv_addr);
    struct hostent* resolve_hostname(const std::string& hostname);
};

#endif // CLIENTCOMMUNICATIONMANAGER_HPP
