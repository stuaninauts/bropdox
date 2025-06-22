#ifndef BETAADDRESS_HPP
#define BETAADDRESS_HPP

#include <string>

struct BetaAddress {
    std::string ip;
    int ring_port;
    int id;
    BetaAddress(const std::string ip, int ring_port, int id) : ip(ip), ring_port(ring_port), id(id) {};
};

struct ClientAddress {
    std::string username;
    std::string ip;
    int port;
    ClientAddress(const std::string username, const std::string ip, int port) : username(username), ip(ip), port(port) {};
};

#endif // BETAADDRESS_HPP