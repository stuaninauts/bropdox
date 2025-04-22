#ifndef UTILS_HPP
#define UTILS_HPP

#include <tuple>
#include <string>

class Utils {
public:
    static std::tuple<int, std::string, int> create_and_listen_socket(int client_sockfd); // So o server usa
    static int connect_to_socket(const std::string& ip, int port); // So o client usa
    static std::pair<std::string, int> parse_ip_and_port(const std::string& msg);
};

#endif // UTILS_HPP
