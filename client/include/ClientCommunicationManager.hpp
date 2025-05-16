#ifndef CLIENTCOMMUNICATIONMANAGER_HPP
#define CLIENTCOMMUNICATIONMANAGER_HPP

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <mutex>

class ClientCommunicationManager {
public:

    bool connect_to_server(const std::string server_ip, int port, const std::string username);
    void receive_packet();
    void send_command(const std::string command, const std::string filename = "");

    void fetch();
    void get_sync_dir();
    void upload_file(const std::string filename);
    void download_file(const std::string filename);
    void delete_file(const std::string filename);
    void exit_server();
    void list_server();
    void watch(const std::string sync_dir_path);


// private:
    // ip
    std::string server_ip;

    // ports
    int port_cmd;
    int port_upload;
    int port_download;

    // sockets
    int socket_cmd;
    int socket_upload;
    int socket_download;

    // username
    std::string username;

    std::vector<std::string> files_received_from_server;

    void close_sockets();

    bool send_username();
    bool get_sockets_ports();
    bool connect_socket_cmd();
    bool connect_socket_to_server(int sockfd, int* port);
    bool connection_accepted();
};

#endif // CLIENTCOMMUNICATIONMANAGER_HPP
