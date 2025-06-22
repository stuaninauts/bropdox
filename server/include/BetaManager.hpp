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

struct BetaInfo {
    int socket_fd;
    std::string ip;
    int ring_port;
    int id;
    BetaInfo(int socket_fd, const std::string ip, int ring_port, int id) : socket_fd(socket_fd), ip(ip), ring_port(ring_port), id(id) {};
};

class BetaManager {

public:
    std::vector<BetaInfo> betas;

    BetaManager() : next_beta_id(0) {};
    void add_beta(int new_beta_socket_fd, const std::string new_beta_ip, int new_beta_ring_port, int id = -1);
    void remove_beta(int sockfd);
    void send_client_device(const std::string ip, const std::string username, int port) const;
    void send_file(const fs::path filepath, const std::string username) const;
    void delete_file(const std::string filename, const std::string username) const;
    void print_betas() const;
    void send_new_beta_server(BetaInfo new_beta) const;
    void send_all_betas_to_new_beta(int new_beta_socket_fd) const;
    void send_heartbeat(int beta_socket_fd) const;
    void send_removed_client_device(const std::string ip, const std::string username, int reconnection_port) const;
    

private:
    mutable std::shared_mutex access_betas;
    mutable std::mutex write_beta_socket;
    int next_beta_id;

};
#endif // BETALIST_HPP