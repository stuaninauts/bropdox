#include <Network.hpp>

int Network::connect_socket_ipv4(const std::string ip, int port) {
    struct addrinfo hints{};
    struct addrinfo *result;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int s = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (s != 0) {
        std::cerr << "getaddrinfo: " << std::string(gai_strerror(s)) << std::endl;
        return -1;
    }

    int socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socket_fd < 0) {
        freeaddrinfo(result);
        std::cerr << "Failed to create socket." << std::endl;
        return -1;
    }

    if (connect(socket_fd, result->ai_addr, result->ai_addrlen) == -1) {
        close(socket_fd); 
        freeaddrinfo(result);
        std::cerr << "Failed to connect to " + ip + ":" + std::to_string(port) << std::endl;
        return -1;
    }

    freeaddrinfo(result);

    return socket_fd;
}

int Network::setup_socket_ipv4(int port, int backlog) {
    struct addrinfo hints{};
    struct addrinfo *result;
    int yes = 1;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int s = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &result);
    if (s != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(s) << std::endl;
        return -1;
    }

    int listen_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listen_fd < 0) {
        freeaddrinfo(result);
        std::cerr << "Failed to create socket" << std::endl;
        return -1;
    }
    
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        close(listen_fd);
        freeaddrinfo(result);
        std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;
        return -1;
    }

    if (bind(listen_fd, result->ai_addr, result->ai_addrlen) < 0){
        close(listen_fd);
        freeaddrinfo(result);
        std::cerr << "Failed to bind socket" << std::endl;
        return -1;
    }

    freeaddrinfo(result);
    
    std::cout << "Listening on port " << port << std::endl;
    if (listen(listen_fd, backlog) < 0) {
        close(listen_fd);
        std::cerr << "Failed to listen on socket" << std::endl;
        return -1;
    }
    std::cout << "Waiting for connections..." << std::endl;

    return listen_fd;
}