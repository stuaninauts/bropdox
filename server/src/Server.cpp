#include <Server.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>

#define PORT 6969

using namespace std;

Server::Server() {}

void Server::run() {

   // Criação do socket principal
   int sockfd, newsockfd, n;
   socklen_t clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");

	// Configuração bind socket
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);         


	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");
	
    // Log informações iniciais do servidor
    log(serv_addr);
	listen(sockfd, 5);
	
	clilen = sizeof(struct sockaddr_in);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	if ( newsockfd == -1) 
		printf("ERROR on accept");


    // buffer onde o socket principal irá ler
	bzero(buffer, 256);
	
	/* read from the socket */
	n = read(newsockfd, buffer, 256);
	if (n < 0) 
		printf("ERROR reading from socket");
	printf("Here is the message: %s\n", buffer);
	
	/* write in the socket */ 
	n = write(newsockfd,"I got your message", 18);
	if (n < 0) 
		printf("ERROR writing to socket");
	// close(newsockfd);
	// close(sockfd);
   /* while (true) {
        accept(); // returns socket
        // pthread create
    }   
*/
	
	close(newsockfd);

}

void Server::log(const sockaddr_in& serv_addr){
    // Logs das configurações do servidor
    std::cout << "sin_family: " << serv_addr.sin_family << std::endl;
    std::cout << "sin_port: " << ntohs(serv_addr.sin_port) << std::endl;

    // Para o endereço, como é em formato numérico, pode-se converter para string
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(serv_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
    std::cout << "sin_addr: " << ip_str << std::endl;
}

void Server::accept_connection() {}