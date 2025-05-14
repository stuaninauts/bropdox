#include <Client.hpp>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Uso: ./client <username> <ip_servidor> <porta>\n";
        return 1;
    }

    string username = argv[1];
    string ip = argv[2];
    int port = stoi(argv[3]);

    Client client(ip, port, username);
    cout << "Iniciando cliente com username: " << username << ", ip: " << ip << ", porta: " << port << endl;
    client.run();

    return 0;
}