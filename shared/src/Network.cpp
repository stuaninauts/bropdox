#include <Network.hpp>

int Network::connect_socket_ipv4(const std::string ip, int port) {
    struct addrinfo hints{};
    struct addrinfo *result;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int s = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (s != 0) {
        std::cerr << "[ CONNECT SOCKET IPv4 ] getaddrinfo: " << std::string(gai_strerror(s)) << std::endl;
        return -1;
    }

    int socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socket_fd < 0) {
        freeaddrinfo(result);
        std::cerr << "[ CONNECT SOCKET IPv4 ] Failed to create socket." << std::endl;
        return -1;
    }

    if (connect(socket_fd, result->ai_addr, result->ai_addrlen) == -1) {
        close(socket_fd); 
        freeaddrinfo(result);
        std::cerr << "[ CONNECT SOCKET IPv4 ] Failed to connect to " + ip + ":" + std::to_string(port) << std::endl;
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
        std::cerr << "[ SETUP SOCKET IPv4 ] getaddrinfo: " << gai_strerror(s) << std::endl;
        return -1;
    }

    int listen_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listen_fd < 0) {
        freeaddrinfo(result);
        std::cerr << "[ SETUP SOCKET IPv4 ] Failed to create socket" << std::endl;
        return -1;
    }
    
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        close(listen_fd);
        freeaddrinfo(result);
        std::cerr << "[ SETUP SOCKET IPv4 ] setsockopt(SO_REUSEADDR) failed" << std::endl;
        return -1;
    }

    if (bind(listen_fd, result->ai_addr, result->ai_addrlen) < 0){
        close(listen_fd);
        freeaddrinfo(result);
        std::cerr << "[ SETUP SOCKET IPv4 ] Failed to bind socket" << std::endl;
        return -1;
    }

    freeaddrinfo(result);
    
    std::cout << "[ SETUP SOCKET IPv4 ] Listening on port " << port << std::endl;
    if (listen(listen_fd, backlog) < 0) {
        close(listen_fd);
        std::cerr << "[ SETUP SOCKET IPv4 ] Failed to listen on socket" << std::endl;
        return -1;
    }

    return listen_fd;
}

int Network::get_available_port() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        std::cerr << "[ GET PORT ] socket failed" << std::endl;
        return -1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(0);

    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "[ GET PORT ] bind failed" << std::endl;
        close(socket_fd);
        return -1;
    }

    socklen_t addr_len = sizeof(addr);
    if (getsockname(socket_fd, (struct sockaddr*)&addr, &addr_len) == -1) {
        std::cerr << "[ GET PORT ] getsockname failed" <<  std::endl;
        close(socket_fd);
        return -1;
    }

    int assigned_port = ntohs(addr.sin_port);
    close(socket_fd);

    std::cout << "[ GET PORT ] Available PORT: " << assigned_port << std::endl;

    return assigned_port;
}