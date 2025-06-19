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
#include <thread> 
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <Packet.hpp>
#include <filesystem>
#include <FileManager.hpp>
#include <ClientsDevices.hpp>
#include <Network.hpp>
#include <condition_variable>
#include <chrono>

namespace fs = std::filesystem;

struct BetaAddress {
    std::string ip;
    int ring_port;
    int id;
    BetaAddress(const std::string ip, int ring_port, int id) : ip(ip), ring_port(ring_port), id(id) {};
};

class BetaServer {

public:
    BetaServer(int port_alfa, std::string ip_alfa) :
        port_alfa(port_alfa),
        ip_alfa(ip_alfa) {}

    void run();

private:
    int alfa_socket_fd;
    int port_alfa;
    std::string ip_alfa;
    fs::path backup_dir_path;
    std::shared_ptr<ClientsDevices> devices;

    // ring next beta variables
    std::string ip_next_beta;
    int next_beta_port;
    int next_beta_socket_fd;

    // heartbeat variables
    std::mutex heartbeat_mutex;
    std::condition_variable heartbeat_cv;
    bool heartbeat_received;

    // ring connection variables
    int ring_socket_fd;
    int ring_port;

    std::atomic<int> prev_beta_socket_fd{-1};
    std::vector<BetaAddress> betas;

    void handle_alfa_updates();
    void handle_client_delete(const std::string filename, const std::string username);
    void handle_client_upload(const std::string filename, const std::string username, uint32_t total_packets);
    void handle_new_clients(const std::string ip_first_client, const std::string username_first_client, int total_clients);
    void handle_client_updates(Packet meta_packet);
    void handle_new_betas(Packet meta_packet);
    void accept_ring_connection();
    void handle_beta_updates();
    void heartbeat_timeout();

};

#endif // BETASERVER_HPP
