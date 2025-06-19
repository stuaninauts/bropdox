#include <Client.hpp>
#include <iostream>
#include <Network.hpp>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: ./client <username> <server_ip> <port>" << std::endl;
        return 1;
    }

    string username = argv[1];
    string ip = argv[2];
    int port = stoi(argv[3]);
    int port_backup = Network::get_available_port();

    if (port_backup == -1) {
        std::cerr << "Error: Could not find an available port for backup server." << std::endl;
        return 1;
    }

    Client client(ip, port, username, "./sync_dir/", port_backup);
    std::cout << "Starting client with username: " << username << ", ip: " << ip << ", port: " << port <<", backup port" << port_backup << std::endl;
    client.run();

    return 0;
}
