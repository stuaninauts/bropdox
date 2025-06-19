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

namespace fs = std::filesystem;
class AlfaServer {

public:
    AlfaServer(int port_client) :
        port_client(port_client),
        port_beta(8081), // HARD CODED
        initial_socket_client(-1),
        initial_socket_beta(-1),
        socket_first_beta(-1),
        socket_last_beta(-1) {};

    std::shared_ptr<ClientsDevices> devices;
    std::shared_ptr<BetaManager> betas;

    void run();

private:

    int initial_socket_client;
    int initial_socket_beta;
    int port_client;
    int port_beta;

    int socket_first_beta;
    int port_first_beta;
    std::string ip_first_beta;

    int socket_last_beta;


    void handle_client_session(int socket_fd);
    void handle_beta_session(int new_beta_socket_fd, struct sockaddr_in new_beta_beta_address);
    void handle_beta_connection();
    void handle_client_connection();

    fs::path server_dir_path = "./sync_dir_server";

};

#endif // ALFASERVER_HPP
