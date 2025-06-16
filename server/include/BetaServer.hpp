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

namespace fs = std::filesystem;

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

    std::string ip_next_beta;
    int next_beta_socket_fd;
    int prev_beta_socket_fd;

    bool connect_to_alfa();
    void sync();
    void handle_client_delete(const std::string filename, const std::string username);
    void handle_client_upload(const std::string filename, const std::string username, uint32_t total_packets);
    void handle_new_client(const std::string ip, const std::string username);
    void handle_client_updates(std::string username);
    void connect_next_beta(std::string next_beta_ip);

};

#endif // BETASERVER_HPP
