#ifndef SERVERCOMMUNICATIONMANAGER_HPP
#define SERVERCOMMUNICATIONMANAGER_HPP

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <Packet.hpp>
#include <ServerFileManager.hpp>
#include <ClientsDevices.hpp>
#include <mutex>
#include <thread>
#include <memory>

class ServerCommunicationManager {

public:
    ServerCommunicationManager(ServerFileManager& file_manager_) 
        : file_manager(file_manager_) {
    }
    
    void setup_client_session(int socket_cmd, std::string username, std::shared_ptr<ClientsDevices> devices);
    void receive_packet();
    void sync_client();
    
    void read_cmd();
    std::shared_ptr<ClientsDevices> devices;

// private:
    std::string username;
    
    // sockets
    int socket_upload;
    int socket_download;
    int socket_cmd;
    // ports
    int port_upload;
    int port_download;
    int port_cmd;

    bool connect_socket_to_client(int *sockfd, int *port);
    void close_sockets();

    void handle_client_download(const std::string filename);    
    void handle_client_upload();
    void handle_client_delete(const std::string filename);
    void handle_exit();
    void handle_list_server();


    ServerFileManager& file_manager;
};

#endif // SERVERCOMMUNICATIONMANAGER_HPP
