#include <Network.hpp>

int Network::connect_socket(const std::string ip, int port) {
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
