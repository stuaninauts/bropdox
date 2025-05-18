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
#include <ServerCommunicationManager.hpp>
#include <ClientsDevices.hpp>

using namespace std;
namespace fs = std::filesystem;
class Server {

public:
    Server() = default;

    void run();

private:

    int initial_socket = -1;

    bool setup();
    void handle_client(int socket);

    std::shared_ptr<ClientsDevices> devices;
    fs::path server_dir_path = "./sync_dir_server";
};

#endif // SERVER_HPP
