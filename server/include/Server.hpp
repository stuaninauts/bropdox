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
#include <mutex>
#include <netinet/in.h>
#include <thread> 
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <ClientSession.hpp>
#include <ClientsDevices.hpp>

enum class ServerRole {
    ALFA, 
    BETA
};

namespace fs = std::filesystem;
class Server {

public:
    Server(int port_client) :
        current_role(ServerRole::ALFA),
        port_client(port_client),
        port_beta(8081), // HARD CODED
        initial_socket_client(-1),
        initial_socket_beta(-1) {};

    Server(int port_client, std::string ip_primary_server) :
        current_role(ServerRole::BETA),    
        port_client(port_client),
        port_beta(8081),
        ip_primary_server(ip_primary_server),
        initial_socket_client(-1),
        initial_socket_beta(-1) {};

    void run();

private:

    int initial_socket_client;
    int initial_socket_beta;
    int port_client;
    int port_beta;
    std::string ip_primary_server;

    ServerRole current_role;

    void handle_client_session(int socket);
    void run_alfa();
    void run_beta();
    void handle_beta_connection();
    void handle_client_connection();
    int setup_socket(int port);

    std::shared_ptr<ClientsDevices> devices;
    fs::path server_dir_path = "./sync_dir_server";
    
};

#endif // SERVER_HPP
