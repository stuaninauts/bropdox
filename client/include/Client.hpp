#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <thread> 
#include <iostream>
#include <algorithm>
#include <filesystem>

#include <ClientCommunicationManager.hpp>

using namespace std;

namespace fs = std::filesystem;

class Client {
public:
    Client(const std::string& server_ip, int port, const std::string& username, const std::string sync_dir_path)
          : sync_dir_path(sync_dir_path),
            comm_manager(server_ip, port, username, sync_dir_path) {}

    void run();

private:
    ClientCommunicationManager comm_manager;

    void sync_local();
    void sync_remote();
    void user_interface();

    fs::path sync_dir_path;

    vector<string> split_command(const string &command);
    void process_command(const vector<string> &tokens);
};

#endif // CLIENT_HPP


    
