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

namespace fs = std::filesystem;
class AlfaServer {

public:
    AlfaServer(int port_client) :
        port_client(port_client),
        port_beta(8081), // HARD CODED
        initial_socket_client(-1),
        initial_socket_beta(-1) {};


    void run();

private:

    int initial_socket_client;
    int initial_socket_beta;
    int port_client;
    int port_beta;

    void handle_client_session(int socket);
    void handle_beta_session(int socket);
    void handle_beta_connection();
    void handle_client_connection();
    int setup_socket(int port);

    std::shared_ptr<ClientsDevices> devices;
    std::shared_ptr<BetaManager> betas;
    fs::path server_dir_path = "./sync_dir_server";
    
};

#endif // ALFASERVER_HPP
