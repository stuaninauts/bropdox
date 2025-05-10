#include <ClientCommunicationManager.hpp>

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

int ClientCommunicationManager::connect_to_server() {
    try {
        struct hostent* server = resolve_hostname(server_ip);

        socketfd = create_socket();

        struct sockaddr_in serv_addr = create_server_address(server);

        if (connect(socketfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            throw std::runtime_error("ERROR: Connecting to server");
        }

        std::cout << "Conexão com servidor estabelecida" << std::endl;

        return socketfd;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}

void ClientCommunicationManager::send_username() {
    int n = write(socketfd, username.c_str(), username.length());
    if (n < 0) {
        throw std::runtime_error("ERROR: Writing username to socket");
    }
}

// ========================================= //
// ================ PRIVATE ================ //
// ========================================= //

struct hostent* ClientCommunicationManager::resolve_hostname(const std::string& hostname) {
    struct hostent* server = gethostbyname(hostname.c_str());
    if (server == nullptr) {
        throw std::runtime_error("ERROR: No such host");
    }
    return server;
}

int ClientCommunicationManager::create_socket() {
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        throw std::runtime_error("ERROR: Opening socket");
    }
    return socketfd;
}

struct sockaddr_in ClientCommunicationManager::create_server_address(struct hostent* server) {
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); // Network byte order
    serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8); // Zero out padding
    return serv_addr;
}
