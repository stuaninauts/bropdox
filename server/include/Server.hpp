#ifndef SERVER_HPP
#define SERVER_HPP

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
#include <netinet/in.h>

using namespace std;
class Server {
private:
    // Lista encadeada de processos (?)
    // Processos se conectam com communication manager e file manager
    // Hashtable (key = username) com tuplas de processos
    
    void accept_connection();
public:
    // Construtor
    Server();
    
    // File Manager -> modifica os diretórios locais
    // Communication Manager (pushes) -> modificações externas

    // Communication Manager recebe um push, e passa para o File Manager atualizar o arquivo,
    // ao finalizar sinaliza o Communication Manager para atualizar os outros clientes

    void log(const sockaddr_in& serv_addr);
    void run();
};

#endif // SERVER_HPP
