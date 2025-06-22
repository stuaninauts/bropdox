#include <iostream>
#include <AlfaServer.hpp>
#include <BetaServer.hpp>


#define PORT 8080

int main(int argc, char* argv[]) {
   int port;
   std::string ip_alfa;
   
   // Alfa
   if (argc < 3) {
      port = argc == 1 ? PORT : stoi(argv[1]);
      AlfaServer alfa(port);
      alfa.run();
      return 0;
   }
   
   // Beta
   if (argc == 3) {
      port = stoi(argv[1]);
      ip_alfa = argv[2];
      BetaServer beta(port, ip_alfa);
      beta.run();

      std::cout<< "Become Alfa: " << beta.become_alfa << std::endl;

      if(beta.become_alfa) {
         AlfaServer alfa(8088);
         alfa.become_alfa(beta.clients, beta.betas);
      }
      return 0;
   }
      
   if (argc > 3) {
      std::cout << "Usage for alfa server: ./server <port> or ./server" << std::endl;
      std::cout << "Usage for beta server: ./server <port> <ip-primary-server>" << std::endl;
      return 1;
   }
}