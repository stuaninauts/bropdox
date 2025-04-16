#include "Packet.hpp"
#include <sstream>
#include <iostream>

using namespace std;

// Construtor
Packet::Packet(uint16_t type, uint16_t seqn, uint32_t totalSize, uint16_t length, const string& payload)
    : type(type), seqn(seqn), totalSize(totalSize), length(length), payload(payload) {}

// Getters
uint16_t Packet::getType() const {
    return type;
}

uint16_t Packet::getSeqn() const {
    return seqn;
}

uint32_t Packet::getTotalSize() const {
    return totalSize;
}

uint16_t Packet::getLength() const {
    return length;
}

const string& Packet::getPayload() const {
    return payload;
}

// Setters
void Packet::setType(uint16_t type) {
    this->type = type;
}

void Packet::setSeqn(uint16_t seqn) {
    this->seqn = seqn;
}

void Packet::setTotalSize(uint32_t totalSize) {
    this->totalSize = totalSize;
}

void Packet::setLength(uint16_t length) {
    this->length = length;
}

void Packet::setPayload(const string& payload) {
    this->payload = payload;
}

// Função para retornar uma representação do pacote como string (para depuração ou envio)
string Packet::toString() const {
    stringstream ss;
    ss << "Packet { Type: " << type
       << ", Seqn: " << seqn
       << ", TotalSize: " << totalSize
       << ", Length: " << length
       << ", Payload: \"" << payload << "\" }";
    return ss.str();
}
