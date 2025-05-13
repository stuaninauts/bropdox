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
#include <ServerFileManager.hpp>
#include <ServerCommunicationManager.hpp>

using namespace std;
class Server {

public:
    Server() = default;

    void run();

private:
    std::unordered_map<std::string, std::vector<int>> clients;

    void handle_client(int socket);

    ServerFileManager fileManager;
    ServerCommunicationManager commMananger;
};

#endif // SERVER_HPP
