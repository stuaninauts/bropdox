#include <Server.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>

#define PORT 2000

using namespace std;

void Server::handle_client(int socket) {
    std::cout << "test"; 
}

void Server::run() {
    // SETUP socker principal
    cout << "Iniciando servidor..." << endl;
    int sockfd, newsockfd, n;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");

    // Configuração bind socket
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);         


    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("ERROR on binding");
    }
    
    cout << "Servidor esperando conexões..." << endl;
    listen(sockfd, 5);

    // Handle clients
    clilen = sizeof(struct sockaddr_in);

    while (true) {
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
            
        if (newsockfd == -1) 
            std::cout << "ERROR on accept" << std::endl;
        
        if (newsockfd >= 0) {
            std::cout << "client conected" << std::endl; 
            std::thread t(&Server::handle_client, this, newsockfd);
            t.detach();
        }
    }
}