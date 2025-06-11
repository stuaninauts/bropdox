#ifndef BETAMANAGER_HPP
#define BETAMANAGER_HPP

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

using namespace std;

class BetaManager {

public:
    BetaManager() = default;
    bool add_beta(int sockfd);
    void remove_beta(int sockfd);
    int send_file();
    int delete_file();
    
private:
    std::vector<std::string, std::vector<int>> clients_sockets;
};
#endif // BETALIST_HPP