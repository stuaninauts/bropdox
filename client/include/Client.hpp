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
#include <unistd.h>
#include <algorithm>

#include <ClientFileManager.hpp>
#include <ClientCommunicationManager.hpp>

using namespace std;
class Client {
public:
    Client(const std::string& server_ip, int port, const std::string& username)
        : server_ip(server_ip), port(port), username(username) {};

    void run();

private:
    int port;
    std::string username;
    std::string server_ip;

    ClientFileManager file_manager;
    ClientCommunicationManager comm_manager;

    void sync_local();
    void sync_remote();
    void user_interface();
    vector<string> split_command(const string &command);
    void process_command(const vector<string> &tokens);
};

#endif // CLIENT_HPP


    
