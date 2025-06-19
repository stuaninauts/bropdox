#ifndef CLIENTSESSION_HPP
#define CLIENTSESSION_HPP

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
#include <BetaManager.hpp>
#include <mutex>
#include <thread>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;
class ClientSession {

public:
    ClientSession(int socket_cmd, std::string username, std::shared_ptr<ClientsDevices> devices, std::shared_ptr<BetaManager> betas, fs::path user_dir_path, int port_backup)
        :   socket_cmd(socket_cmd), username(username), devices(devices), betas(betas), user_dir_path(user_dir_path), port_backup(port_backup) {};
        
    ~ClientSession() {
        close_sockets();
    }
    
    void connect_sockets();
    void run();
    void handle_client_update();
    void handle_client_cmd();

private:
    std::shared_ptr<ClientsDevices> devices;
    std::shared_ptr<BetaManager> betas;
    
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

    std::string client_ip;
    int port_backup;

    bool connect_socket_to_client(int *sockfd, int *port);
    void close_sockets();

    void handle_client_download(const std::string filename);    
    void handle_client_upload(const std::string filename, uint32_t total_packets);
    void handle_client_delete(const std::string filename);
    void handle_exit();
    void handle_get_sync_dir();
    void handle_list_server();
};

#endif // CLIENTSESSION_HPP
