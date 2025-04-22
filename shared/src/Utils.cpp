#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Utils.hpp"

std::tuple<int, std::string, int> Utils::create_and_listen_socket(int client_sockfd) {
    int sockfd;
    struct sockaddr_in serv_addr;
    socklen_t len = sizeof(serv_addr);

    // Cria socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Zera a estrutura e define família, IP e porta = 0 (porta automática)
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = 0;

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erro no bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 5) < 0) {
        perror("Erro no listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Descobre IP e porta reais usados
    if (getsockname(sockfd, (struct sockaddr *)&serv_addr, &len) == -1) {
        perror("Erro no getsockname");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    std::string ip = inet_ntoa(serv_addr.sin_addr); // Pode retornar 0.0.0.0
    int port = ntohs(serv_addr.sin_port);

    // Envia a porta ao socket principal do cliente
    std::string port_str = std::to_string(port);
    int n = write(client_sockfd, port_str.c_str(), port_str.length());
    if (n < 0) {
        perror("Erro ao enviar porta para o cliente");
    }

    std::cout << "Socket escutando em IP: " << ip << ", porta: " << port << std::endl;

    return std::make_tuple(sockfd, ip, port);
}

int Utils::connect_to_socket(const std::string& /*ip*/, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;

    // Cria socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erro ao criar socket");
        return -1;
    }

    // Prepara estrutura do servidor
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Força uso de localhost (127.0.0.1)
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Endereço IP inválido: 127.0.0.1" << std::endl;
        close(sockfd);
        return -1;
    }

    // Tenta conectar
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        close(sockfd);
        return -1;
    }

    std::cout << "Conectado com sucesso a 127.0.0.1:" << port << std::endl;
    return sockfd;
}
