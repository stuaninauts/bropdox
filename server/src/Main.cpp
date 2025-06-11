#include <iostream>
#include <Server.hpp>


#define PORT 8080

int main(int argc, char* argv[]) {
   int port;
   std::string ip_primary_server;
   
   // Alfa
   if (argc < 3) {
      port = argc == 1 ? PORT : stoi(argv[1]);
      Server server(port);
      server.run();
      return 0;
   }
   
   // Beta
   if (argc == 3) {
      port = stoi(argv[1]);
      ip_primary_server = argv[2];
      Server server(port, ip_primary_server);
      server.run();
      return 0;
   }
      
   if (argc > 3) {
      std::cout << "Usage for primary server: ./server <port> or ./server" << std::endl;
      std::cout << "Usage for backup server: ./server <port> <ip-primary-server>" << std::endl;
      return 1;
   }
}