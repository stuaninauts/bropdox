#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdint>
#include <string>
#include <vector>

using namespace std;
class Client {
private:
    string username; // Nome do usuário
    string server_ip; // Endereço IP do servidor
    int port; // Porta do servidor

    vector<string> splitCommand(const string &command);
    void processCommand(const vector<string> &tokens);

public:
    // Construtor
    Client(string username, string server_ip, int port);

    void run();
};

#endif // CLIENT_HPP
