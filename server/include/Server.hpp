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

using namespace std;
namespace fs = std::filesystem;
class Server {

public:
    Server(int port) : port(port), initial_socket(-1), alfa(true) {};
    Server(int port, std::string ip_primary_server) : port(port), ip_primary_server(ip_primary_server), alfa(false), initial_socket(-1) {};

    void run();

private:

    int initial_socket;
    int port;
    std::string ip_primary_server;

    bool alfa;

    bool setup();
    void handle_client_session(int socket);
    void run_alfa();
    void run_beta();
    void handle_beta_connection();
    void handle_client_connection();

    std::shared_ptr<ClientsDevices> devices;
    fs::path server_dir_path = "./sync_dir_server";
    
};

#endif // SERVER_HPP
