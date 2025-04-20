#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "FileManager.hpp"

using namespace std;
class Client {
private:
    string username; // Nome do usuário
    string server_ip; // Endereço IP do servidor
    int port; // Porta do servidor
    FileManager fileManager; // File Manager (inotify) -> modificações locais
    // Communication Manager (pushes) -> modificações externas

    // Push:
    // File Manager detecta modificação, sinaliza o communication manager para atualizar o servidor

    // Pull:
    // Communication Manager notifica o File Manager que teve alterações

    vector<string> splitCommand(const string &command);
    void processCommand(const vector<string> &tokens);

public:
    // Construtor
    Client(string username, string server_ip, int port);

    void run();
    void connect_to_server(string server_ip, int porta);
};

#endif // CLIENT_HPP
