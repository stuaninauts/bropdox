#include "../include/Client.hpp"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Uso: ./client <username> <ip_servidor> <porta>\n";
        return 1;
    }

    const char* username = argv[1];
    const char* ip = argv[2];
    int port = stoi(argv[3]);

    Client client(username, ip, port);
    client.run();

    return 0;
}