#include <iostream>
#include <Server.hpp>


#define PORT 8080

int main(int argc, char* argv[]) {
   int port;
   if (argc == 1) {
      port = 8080;
   } else if (argc == 2) {
      port = stoi(argv[1]);
   } else {
      std::cout << "Usage: ./server <port>" << std::endl;
      return 1;
   }
   Server server(port);
   server.run();
   return 0;
}