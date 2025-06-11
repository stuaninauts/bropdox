#ifndef BETASERVER_HPP
#define BETASERVER_HPP

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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

namespace fs = std::filesystem;
class BetaServer {

public:
    BetaServer(int port_alfa, std::string ip_alfa) :
        port_alfa(port_alfa),
        ip_alfa(ip_alfa) {}

    void run();

private:
    int socket_fd;
    int port_alfa;
    std::string ip_alfa;
    bool connect_to_alfa();

    
};

#endif // BETASERVER_HPP
