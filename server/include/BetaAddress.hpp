#ifndef BETAADDRESS_HPP
#define BETAADDRESS_HPP

#include <string>

struct BetaAddress {
    std::string ip;
    int ring_port;
    int id;
    BetaAddress(const std::string ip, int ring_port, int id) : ip(ip), ring_port(ring_port), id(id) {};
};

#endif // BETAADDRESS_HPP