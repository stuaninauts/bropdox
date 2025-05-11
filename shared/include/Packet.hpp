#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint>
#include <string>

using namespace std;

class Packet {
public:
    // Construtor
    Packet(uint16_t type, uint16_t seqn, uint32_t totalSize, uint16_t length, const string& payload);

    // Getters
    uint16_t getType() const;
    uint16_t getSeqn() const;
    uint32_t getTotalSize() const;
    uint16_t getLength() const;
    const string& getPayload() const;

    // Setters
    void setType(uint16_t type);
    void setSeqn(uint16_t seqn);
    void setTotalSize(uint32_t totalSize);
    void setLength(uint16_t length);
    void setPayload(const string& payload);

    // Função para retornar um pacote como string (para depuração ou transmissão)
    string toString() const;

private:
    uint16_t type;         // Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;         // Número de sequência
    uint32_t totalSize;    // Número total de fragmentos
    uint16_t length;       // Comprimento do payload
    string payload;   // Dados do pacote
};

#endif // PACKET_HPP
