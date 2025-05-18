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
#include <ClientsDevices.hpp>
#include <mutex>
#include <thread>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;
class ServerCommunicationManager {

public:
    ServerCommunicationManager(int socket_cmd, std::string username, std::shared_ptr<ClientsDevices> devices, fs::path user_dir_path)
        :   socket_cmd(socket_cmd), username(username), devices(devices), user_dir_path(user_dir_path) {};
        
    ~ServerCommunicationManager() {
        close_sockets();
    }
    
    void run_client_session();
    void handle_client_update();
    void handle_client_cmd();

private:
    std::shared_ptr<ClientsDevices> devices;
    std::string username;
    std::string session_name;
    fs::path user_dir_path;
    
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
    void handle_client_upload(const std::string filename, uint32_t total_packets);
    void handle_client_delete(const std::string filename);
    void handle_exit();
    void handle_get_sync_dir();
    void handle_list_server();
};

#endif // SERVERCOMMUNICATIONMANAGER_HPP
