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
#include <shared_mutex>
#include <netinet/in.h>
#include <thread> 
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <Packet.hpp>

namespace fs = std::filesystem;

class BetaManager {

public:
    BetaManager() = default;
    void add_beta(int sockfd);
    void remove_beta(int sockfd);
    void send_client_device(const std::string ip, const std::string username) const;
    void send_file(const fs::path filepath, const std::string username) const;
    void delete_file(const std::string filename, const std::string username) const;
    void print_beta_sockets() const;
    void send_new_beta_server(const std::string ip, int ring_port) const;

private:
    std::vector<int> beta_sockets;
    mutable std::shared_mutex access_beta_sockets;

};
#endif // BETALIST_HPP