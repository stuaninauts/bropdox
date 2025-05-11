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
    Client(const std::string& ip, int port, const std::string& username) : commManager(ip, port, username) {}

    void run();

private:
    ClientFileManager fileManager;
    ClientCommunicationManager commManager;
    int socketfd;

    void sync_local();
    void sync_remote();
    void user_interface();
    vector<string> splitCommand(const string &command);
    void processCommand(const vector<string> &tokens);
};

#endif // CLIENT_HPP


    
