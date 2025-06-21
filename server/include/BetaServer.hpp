#ifndef BETASERVER_HPP
#define BETASERVER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <mutex>
#include <thread> 
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <Packet.hpp>
#include <filesystem>
#include <FileManager.hpp>
#include <ClientsDevices.hpp>
#include <Network.hpp>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <BetaManager.hpp>
#include <BetaAddress.hpp>

namespace fs = std::filesystem;

class BetaServer {

public:
    BetaServer(int port_alfa, std::string ip_alfa) :
        port_alfa(port_alfa),
        ip_alfa(ip_alfa),
        heartbeat_received(false),
        become_alfa(false),
        my_id(-1) {}  // ID será definido quando receber do alfa

    void run(int new_socket_fd = -1);
    bool become_alfa;
    std::shared_ptr<ClientsDevices> devices;
    std::vector<BetaAddress> betas;

    // public method for testing election
    void trigger_election() { start_election(); }

private:

    int alfa_socket_fd;
    int port_alfa;
    std::string ip_alfa;
    fs::path backup_dir_path;


    // ring next beta variables
    std::string ip_next_beta;
    int next_beta_port;
    int next_beta_socket_fd;

    // heartbeat variables
    std::mutex heartbeat_mutex;
    std::condition_variable heartbeat_cv;
    bool heartbeat_received;

    // ring connection variables
    int ring_socket_fd;
    int ring_port;

    std::atomic<int> prev_beta_socket_fd{-1};
    std::atomic<bool> running;

    // election variables
    std::atomic<bool> election_in_progress{false};
    std::atomic<bool> is_participant{false};
    std::atomic<int> elected_coordinator{-1};  // ID do coordenador eleito (-1 = indefinido)
    std::atomic<bool> is_coordinator{false};   // Se este servidor é o coordenador
    int my_id;  // ID único deste servidor beta
    std::mutex election_mutex;

    void handle_alfa_updates();
    void handle_client_delete(const std::string filename, const std::string username);
    void handle_client_upload(const std::string filename, const std::string username, uint32_t total_packets);
    void handle_new_clients(const std::string ip_first_client, const std::string username_first_client, int total_clients, int port_first_client = -1);
    void handle_client_updates(Packet meta_packet);
    void handle_new_betas(Packet meta_packet);
    void accept_ring_connection();
    void handle_beta_updates();
    void heartbeat_timeout();
    void close_sockets();
    void accept_new_alfa_connection(int coordinator_id);

    // election methods
    void start_election();
    void handle_election_message(int candidate_id);
    void handle_elected_message(int coordinator_id);
    void send_election_message(int candidate_id);
    void send_elected_message(int coordinator_id);
    int get_next_beta_socket();
    void become_coordinator();
    void setup_as_alfa_server();

};

#endif // BETASERVER_HPP
