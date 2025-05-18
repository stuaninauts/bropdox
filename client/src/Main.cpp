#include <Client.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: ./client <username> <server_ip> <port>" << std::endl;
        return 1;
    }

    string username = argv[1];
    string ip = argv[2];
    int port = stoi(argv[3]);

    Client client(ip, port, username, "./sync_dir/");
    std::cout << "Starting client with username: " << username << ", ip: " << ip << ", port: " << port << std::endl;
    client.run();

    return 0;
}
