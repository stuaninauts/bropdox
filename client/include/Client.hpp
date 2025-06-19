#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <atomic>
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
#include <Communicator.hpp>

namespace fs = std::filesystem;

class Client {
public:
    Client(const std::string& server_ip, int port, const std::string& username, const std::string sync_dir_path)
          : sync_dir_path(sync_dir_path),
            communicator(server_ip, port, username, sync_dir_path) {}

    void run(int initial_socket = -1);

    Communicator communicator;
    int initial_socket_new_alpha = -1;
    int port_new_alpha = 8002;

    void sync_local();
    void sync_remote();
    void user_interface();
    void handle_new_alpha_connection();

    fs::path sync_dir_path;

    vector<string> split_command(const string &command);
    void process_command(const vector<string> &tokens);

    std::atomic<bool> running{true};
};

#endif // CLIENT_HPP



