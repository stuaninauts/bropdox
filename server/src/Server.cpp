#include <Server.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>

#define PORT 4000

using namespace std;

bool Server::setup_server(int port) {
	struct sockaddr_in serv_addr, cli_addr;
	
	if ((server_socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("ERROR opening setup socket");
        return false;
    } 

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if(bind(server_socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("ERROR on binding setup socket");
        return false;
    }

    listen(server_socketfd, 6);

    return true;
}

void Server::connect_to_client() {
    int n;
    char buffer[256];
    while(true) {
        bzero(buffer, 256);
        n = read(client_socketfd, buffer, 256);
        if (n < 0) 
            printf("ERROR reading from socket");
        else if(n == 0)
            continue; 

        printf("Here is the message: %s\n", buffer);
        n = write(client_socketfd,"I got your message", 18);
        if (n < 0) 
            printf("ERROR writing to socket");
    }
}

void Server::run() {
    printf("oi");
    bool setup_status = setup_server(PORT);
    if(setup_status == false)
        return;


    while(true) {
        client_len = sizeof(struct sockaddr_in);
        client_socketfd = accept(server_socketfd, (struct sockaddr *) &client_address, &client_len);

        if(client_socketfd == 0){
            std::thread client_thread(&Server::connect_to_client, this);
            client_thread.detach();
        }
    }
    return;
}