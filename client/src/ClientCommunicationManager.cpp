#include <ClientCommunicationManager.hpp>

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

int ClientCommunicationManager::connect_to_server() {
    try {
        struct hostent* server = resolve_hostname(server_ip);

        create_sockets();

        struct sockaddr_in serv_addr = create_server_address(server);

        connect_sockets(serv_addr);

        return socketfd;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}

void ClientCommunicationManager::send_username() {
    std::cout << "writing...";
    int n = write(socketfd, username.c_str(), username.length());
    if (n < 0) {
        std::cout << "ERROR: Writing username to socket";
    }
}

// ========================================= //
// ================ PRIVATE ================ //
// ========================================= //

struct hostent* ClientCommunicationManager::resolve_hostname(const std::string& hostname) {
    struct hostent* server = gethostbyname(hostname.c_str());
    if (server == nullptr) {
        std::cout << "ERROR: No such host";
    }
    return server;
}

void ClientCommunicationManager::create_sockets() {
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        std::cout << "ERROR: Opening socket";
    }
}

void ClientCommunicationManager::connect_sockets(struct sockaddr_in serv_addr) {
    if (connect(socketfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "ERROR: Connecting to server";
    } else {
        std::cout << "Conexão com servidor estabelecida" << std::endl;
    }
}

struct sockaddr_in ClientCommunicationManager::create_server_address(struct hostent* server) {
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); // Network byte order
    serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8); // Zero out padding
    return serv_addr;
}
