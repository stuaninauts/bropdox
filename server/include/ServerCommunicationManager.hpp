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
    ServerCommunicationManager(const std::string& ip, int port, const std::string& username)
        : server_ip(ip), port(port), username(username), sockfd(-1) {}

    ~ServerCommunicationManager() {
        if (sockfd != -1) {
            close(sockfd); // Make sure to close the socket when done
        }
    }

    int connect_to_server();
private:
    int sockfd;
    std::string server_ip;
    int port;
    std::string username;

    struct sockaddr_in create_server_address(struct hostent* server);
    int create_socket();
    struct hostent* resolve_hostname(const std::string& hostname);
    void send_username();
};

#endif // SERVERCOMMUNICATIONMANAGER_HPP
