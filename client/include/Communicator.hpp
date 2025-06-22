#ifndef COMMUNICATOR_HPP
#define COMMUNICATOR_HPP

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
#include <Network.hpp>
#include <atomic>

using namespace std;

namespace fs = std::filesystem;

class Communicator {
    
public:
    ~Communicator() {
        close_sockets();
    }

    Communicator(const std::string& server_ip, int port, const std::string& username, const fs::path sync_dir_path, int port_backup)
        :   server_ip(server_ip), port_cmd(port), username(username), sync_dir_path(sync_dir_path), port_backup(port_backup),
            socket_upload(-1), socket_cmd(-1), socket_download(-1), port_upload(0), port_download(0) {};

    bool connect_to_server(int socket_new_alpha = -1);

    void handle_server_update();
    void get_sync_dir();
    void exit_server();
    void list_server();
    void watch_directory();

    void close_sockets();
    
    // ip
    std::string server_ip;

    // ports
    int port_cmd;
    int port_upload;
    int port_download;
    int port_backup;

    // sockets
    int socket_cmd;
    int socket_upload;
    int socket_download;

    // username
    std::string username;
    fs::path sync_dir_path;
    
    // ignored files inotify
    std::set<std::string> ignored_files;

    // handle server update
    void handle_server_delete(const std::string filename);
    void handle_server_upload(const std::string filename, uint32_t total_packets);

    // command
    void send_command(const std::string command);

    // connection setup
    bool send_initial_information();
    bool connect_socket_to_server(int sockfd, int* port);
    bool confirm_connection();

    // Ponteiro para o atomic<bool> running do Client
    std::atomic<bool>* running_ptr = nullptr;

    // Permite setar o ponteiro para running
    void set_running_ptr(std::atomic<bool>* ptr) { running_ptr = ptr; }
};

#endif // COMMUNICATOR_HPP
