#ifndef ALFASERVER_HPP
#define ALFASERVER_HPP

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
#include <ClientSession.hpp>
#include <ClientsDevices.hpp>
#include <BetaManager.hpp>
#include <Network.hpp>
#include <BetaAddress.hpp>

namespace fs = std::filesystem;
class AlfaServer {

public:
    AlfaServer(int port_client) :
        port_client(port_client),
        port_beta(8081), // HARD CODED
        initial_socket_client(-1),
        initial_socket_beta(-1) {};

    AlfaServer(int port_client, int port_beta) :
        port_client(port_client),
        port_beta(port_beta),
        initial_socket_client(-1),
        initial_socket_beta(-1) {};

    std::shared_ptr<ClientsDevices> devices;
    std::shared_ptr<BetaManager> betas;

    void run();
    void become_alfa(std::shared_ptr<ClientsDevices> devices, std::vector<BetaAddress> old_betas_addr);
    void reconnect_betas(std::shared_ptr<BetaManager> old_betas);
    void reconnect_clients(std::shared_ptr<ClientsDevices> old_devices);

private:
    int initial_socket_client;
    int initial_socket_beta;
    int port_client;
    int port_beta;

    void handle_client_session(int socket_fd);
    void handle_beta_session(int new_beta_socket_fd, struct sockaddr_in new_beta_beta_address);
    void handle_beta_connection();
    void handle_client_connection();
    void heartbeat(int beta_socket_fd);
    void send_server_files_to_new_beta(int new_beta_socket_fd);

    fs::path server_dir_path = "./sync_dir_server";

};

#endif // ALFASERVER_HPP
