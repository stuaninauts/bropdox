#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdint>
#include <string>
#include <vector>

using namespace std;
class Server {
private:
    // Lista encadeada de processos (?)
    // Processos se conectam com communication manager e file manager
    // Hashtable (key = username) com tuplas de processos

    void accept();
public:
    // Construtor
    Server();
    
    // File Manager -> modifica os diretórios locais
    // Communication Manager (pushes) -> modificações externas

    // Communication Manager recebe um push, e passa para o File Manager atualizar o arquivo,
    // ao finalizar sinaliza o Communication Manager para atualizar os outros clientes


    void run();
};

#endif // SERVER_HPP
