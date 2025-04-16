#include "Server.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

Server::Server() {}

void Server::run() {
    while (true) {
        accept(); // returns socket
        // pthread create
    }
}


void Server::accept() {}