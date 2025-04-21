#include <Server.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>

#define PORT 6969

using namespace std;

Server::Server() {}


void handle_client(int client_sockfd) {
    char buffer[256];
    bzero(buffer, 256);

    int n = read(client_sockfd, buffer, 255);
    if (n > 0) {
        printf("Conexão realizada, cliente: %s\n", buffer);
		// Criar fileManager para o eduardo
		// threads leitura e escrita ( INOTIFY)
        write(client_sockfd, "Recebido", 8);
    }

    close(client_sockfd);
}

void Server::run() {
   cout << "Iniciando servidor..." << endl;
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
	
  	cout << "Servidor esperando conexões..." << endl;
	listen(sockfd, 5);
	
	clilen = sizeof(struct sockaddr_in);

	while (true) {
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
			
		if ( newsockfd == -1) 
			printf("ERROR on accept");
        
		if (newsockfd >= 0) {
            std::thread(handle_client, newsockfd).detach();
        }
    }


	
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