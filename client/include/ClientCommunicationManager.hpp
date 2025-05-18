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
#include <set>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;
class ClientCommunicationManager {
public:

    bool connect_to_server(const std::string server_ip, int port, const std::string username, const std::string sync_dir_path);
    void send_command(const std::string command, const std::string filename = "");

    void handle_server_update();
    void get_sync_dir();
    void upload_file(const std::string filename);
    void download_file(const std::string filename);
    void delete_file(const std::string filename);
    void exit_server();
    void list_server();
    void watch();


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
    fs::path sync_dir_path;

    std::set<std::string> ignored_files;

    void close_sockets();

    bool send_username();
    bool connect_socket_cmd();
    bool connect_socket_to_server(int sockfd, int* port);
    bool confirm_connection();

    void handle_server_delete(const std::string filename);
    void handle_server_upload(const std::string filename, uint32_t total_packets);
};

#endif // CLIENTCOMMUNICATIONMANAGER_HPP
