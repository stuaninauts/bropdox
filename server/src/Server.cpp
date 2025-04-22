#include <Server.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>

#define PORT 2424

using namespace std;

Server::Server() {}

void handle_client_pushes (FileManager& fileManager, int client_server_socket){

    while(true){
        char buffer[256];
        bzero(buffer, 256);
        int n = read(client_server_socket, buffer, 255);
        if(n > 0) {
            printf("Socket client_server diz: %s", buffer);
        }
    }
};

void start_watcher(FileManager& fileManager, int client_sockfd) {
    try {
        string path = "server/sync_dir_" + fileManager.username;
        // Cria socket de escrita server-client
        auto [socket, ip, port] = Utils::create_and_listen_socket(client_sockfd);

        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int server_client_socket = accept(socket, (struct sockaddr*)&cli_addr, &clilen);
        
        if (server_client_socket < 0) {
            perror("Erro no accept");
            return;
        }
        else {
            cout << "Socket server_client ativo" << endl;
            string msg_teste = "Sou o servidor e estou mandando um socket via server_client (write)";
            int n = write(server_client_socket, msg_teste.c_str(), strlen(msg_teste.c_str()));
        };

        // Passa o socket para o watcher
        Watcher watcher(path, fileManager);
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Erro na thread: " << e.what() << std::endl;
    }
}

void handle_client(int client_sockfd) {
    char buffer[256];
    bzero(buffer, 256);

    int n = read(client_sockfd, buffer, 255);
    if (n > 0) {
        printf("Conexão realizada, cliente: %s\n", buffer);
        std::string client_name(buffer, n);
        FileManager fileManager(client_name);
        fileManager.create_sync_dir("server/");

        // Inicia a thread so watcher INOTIFY
        std::thread(start_watcher, std::ref(fileManager), client_sockfd).detach();

        auto [socket, ip, port] = Utils::create_and_listen_socket(client_sockfd);

        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int client_server_socket = accept(socket, (struct sockaddr*)&cli_addr, &clilen);
        
        if (client_server_socket < 0) {
            perror("Erro no accept");
            return;
        }
        else {
            cout << "Socket client_server ativo" << endl;
        };

        std::thread(handle_client_pushes, std::ref(fileManager), client_server_socket).detach();
    }
     
}

void Server::run() {
   cout << "Iniciando servidor..." << endl;
   // Criação do socket principal
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