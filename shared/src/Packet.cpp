#include "Packet.hpp"
#include <sstream>
#include <cstring>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>

using namespace std;

// Constructor with uint16_t type
Packet::Packet(uint16_t type, uint16_t seqn, uint32_t total_size, uint16_t length, const std::string& payload)
    : type(type), seqn(seqn), total_size(total_size), length(length), payload(payload) {
    validate_length();
}

void Packet::send(int socket_fd) const {
    std::string serialized = serialize();
    write_socket(socket_fd, serialized.data(), serialized.size());
}

Packet Packet::receive(int socket_fd) {
    // Read header
    char header_buffer[HEADER_SIZE];
    read_socket(socket_fd, header_buffer, HEADER_SIZE);

    // Extract payload length
    uint16_t payload_length;
    memcpy(&payload_length, header_buffer + sizeof(type) + sizeof(seqn) + sizeof(total_size), sizeof(length));
    payload_length = ntohs(payload_length);

    // Read payload
    std::string payload;
    if (payload_length > 0) {
        payload.resize(payload_length);
        read_socket(socket_fd, &payload[0], payload_length);
    }

    // Combine header and payload for deserialization
    std::string full_data(header_buffer, HEADER_SIZE);
    full_data += payload;
    
    return deserialize(full_data);
}

std::string Packet::serialize() const {
    std::string result;
    result.resize(HEADER_SIZE + payload.size());

    uint16_t net_type = htons(type);
    uint16_t net_seqn = htons(seqn);
    uint32_t net_total_size = htonl(total_size);
    uint16_t net_length = htons(length);

    size_t offset = 0;
    memcpy(&result[offset], &net_type, sizeof(type)); offset += sizeof(type);
    memcpy(&result[offset], &net_seqn, sizeof(seqn)); offset += sizeof(seqn);
    memcpy(&result[offset], &net_total_size, sizeof(total_size)); offset += sizeof(total_size);
    memcpy(&result[offset], &net_length, sizeof(length)); offset += sizeof(length);
    memcpy(&result[offset], payload.data(), payload.size());

    return result;
}

Packet Packet::deserialize(const std::string& data) {
    if (data.size() < HEADER_SIZE) {
        throw std::runtime_error("Insufficient data for deserialization");
    }

    uint16_t net_type, net_seqn, net_length;
    uint32_t net_total_size;
    size_t offset = 0;

    memcpy(&net_type, &data[offset], sizeof(net_type)); offset += sizeof(net_type);
    memcpy(&net_seqn, &data[offset], sizeof(net_seqn)); offset += sizeof(net_seqn);
    memcpy(&net_total_size, &data[offset], sizeof(net_total_size)); offset += sizeof(net_total_size);
    memcpy(&net_length, &data[offset], sizeof(net_length)); offset += sizeof(net_length);

    uint16_t type = ntohs(net_type);
    uint16_t seqn = ntohs(net_seqn);
    uint32_t total_size = ntohl(net_total_size);
    uint16_t length = ntohs(net_length);

    if (data.size() < HEADER_SIZE + length) {
        throw std::runtime_error("Payload size doesn't match header information");
    }

    std::string payload = data.substr(offset, length);
    return Packet(type, seqn, total_size, length, payload);
}

void Packet::read_socket(int socket_fd, char* buffer, size_t size) {
    size_t total_read = 0;
    while (total_read < size) {
        ssize_t n = read(socket_fd, buffer + total_read, size - total_read);
        if (n <= 0) {
            if (n == 0) {
                throw std::runtime_error("Connection closed by peer");
            }
            throw std::runtime_error("Error reading from socket");
        }
        total_read += n;
    }
}

void Packet::write_socket(int socket_fd, const char* buffer, size_t size) {
    size_t total_sent = 0;
    while (total_sent < size) {
        ssize_t n = write(socket_fd, buffer + total_sent, size - total_sent);
        if (n <= 0) {
            throw std::runtime_error("Error writing to socket");
        }
        total_sent += n;
    }
}

string Packet::to_string() const {
    stringstream ss;
    ss << "Packet { Type: " << type
       << ", Seqn: " << seqn
       << ", TotalSize: " << total_size
       << ", Length: " << length
       << ", Payload: \"" << payload << "\" }";
    return ss.str();
}

void Packet::set_type(uint16_t type) {
    this->type = type;
}

void Packet::set_length(uint16_t length) {
    if (payload.size() != length) {
        throw std::invalid_argument("New length doesn't match current payload size");
    }
    this->length = length;
}

void Packet::set_payload(const std::string& payload) {
    if (payload.size() != length) {
        throw std::invalid_argument("Payload length doesn't match specified length");
    }
    this->payload = payload;
}