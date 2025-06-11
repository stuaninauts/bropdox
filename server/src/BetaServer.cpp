#include <BetaServer.hpp>


bool BetaServer::connect_to_alfa() {
    struct hostent* server;
    struct sockaddr_in serv_addr{};
    int socket_fd;

    if ((server = gethostbyname(ip_alfa.c_str())) == nullptr) {
        std::cerr << "ERROR: No such host\n";
        return false;
    }
    
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "ERROR: Opening socket\n";
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_alfa);
    serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "ERROR: Connecting to server\n";
        return false;
    }

    return true;
}

void BetaServer::run() {
    std::cout << "Setting up BETA server..." << std::endl;
    if(!connect_to_alfa())
        exit(1);
    std::cout << "Connected to ALFA server!" << std::endl;
}
