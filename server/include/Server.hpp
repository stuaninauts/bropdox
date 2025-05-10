#ifndef SERVER_HPP
#define SERVER_HPP

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
#include <netinet/in.h>
#include <thread> 
#include <unordered_map>

using namespace std;
class Server {

public:
    Server() {};
    ~Server() {
        if (server_socketfd != -1) {
            close(server_socketfd);
        }
        if (client_socketfd != -1) {
            close(client_socketfd);
        }
    }

    void run();

private:
    int server_socketfd;
    int client_address;
    int client_socketfd;
    socklen_t client_len;
    std::unordered_map<std::string, std::vector<int>> clients;
    static const int MAX_VALUES = 2;

    bool setup_server(int port);
    void connect_to_client();
};

#endif // SERVER_HPP
