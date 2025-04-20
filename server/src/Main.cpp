#include <iostream>
#include <Server.hpp>

using namespace std;

int main(int argc, char* argv[]) {
   cout << "Iniciando servidor" << endl;
   Server server;
   // Conexao principal com o servidor
   server.run();
   return 0;
}