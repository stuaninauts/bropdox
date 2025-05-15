#include "Packet.hpp"
#include <sstream>
#include <cstring>
#include <vector>
#include <iostream>
#include <fstream> 
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

// void Packet::set_type(uint16_t type) {
//     this->type = type;
// }

// void Packet::set_length(uint16_t length) {
//     if (payload.size() != length) {
//         throw std::invalid_argument("New length doesn't match current payload size");
//     }
//     this->length = length;
// }

// void Packet::set_payload(const std::string& payload) {
//     if (payload.size() != length) {
//         throw std::invalid_argument("Payload length doesn't match specified length");
//     }
//     this->payload = payload;
// }

bool Packet::send_file(int socket_fd, const string& filePath) {
    ifstream file(filePath, ios::binary);
    if (!file) {
        cerr << "Erro ao abrir o arquivo: " << filePath << endl;
        return false;
    }
    
    // Obtém o nome do arquivo
    string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
    
    // Tamanho máximo do payload
    const size_t MAX_PAYLOAD_SIZE = 4096;
    
    // Primeiro pacote com o nome do arquivo
    Packet metaPacket;
    metaPacket.type = 1; // Tipo metadados
    metaPacket.seqn = 0;
    metaPacket.payload = fileName;
    metaPacket.length = fileName.length();
    metaPacket.total_size = 0; // Será calculado depois
    
    // Buffer para ler o arquivo
    vector<char> buffer(MAX_PAYLOAD_SIZE);
    vector<Packet> packets;
    uint16_t seqn = 1;
    
    // Lê o arquivo em partes e cria pacotes
    while (file) {
        file.read(buffer.data(), MAX_PAYLOAD_SIZE);
        size_t bytesRead = file.gcount();
        
        if (bytesRead > 0) {
            Packet dataPacket;
            dataPacket.type = 2; // Tipo dados
            dataPacket.seqn = seqn++;
            dataPacket.payload = string(buffer.data(), bytesRead);
            dataPacket.length = bytesRead;
            
            packets.push_back(dataPacket);
        }
        
        if (bytesRead < MAX_PAYLOAD_SIZE) {
            break; // Fim do arquivo
        }
    }
    
    // Atualiza o total de pacotes
    metaPacket.total_size = packets.size();
    for (auto& packet : packets) {
        packet.total_size = packets.size();
    }
    
    // Envia o pacote de metadados
    try {
        metaPacket.send(socket_fd);
        
        // Envia os pacotes de dados
        for (const auto& packet : packets) {
            packet.send(socket_fd);
        }
        
        cout << "Arquivo " << fileName << " enviado com sucesso em " << packets.size() << " pacotes." << endl;
        return true;
    } catch (const std::runtime_error& e) {
        cerr << "Erro ao enviar arquivo: " << e.what() << endl;
        return false;
    }
}

bool Packet::receive_file(int socket_fd, const string& outputDir) {
    try {
        // Recebe o pacote de metadados
        Packet metaPacket = Packet::receive(socket_fd);
        
        if (metaPacket.type == static_cast<uint16_t>(Packet::Type::ERROR)) {
            cerr << "Erro ao receber arquivo: " << metaPacket.payload << endl;
            return false;
        }

        // Extrai informações do pacote
        string fileName = metaPacket.payload;
        uint32_t totalPackets = metaPacket.total_size;
        
        // Cria o arquivo de saída
        string outputPath = outputDir;
        if (!outputDir.empty() && outputDir.back() != '/') {
            outputPath += '/';
        }
        outputPath += fileName;
        
        ofstream outFile(outputPath, ios::binary);
        if (!outFile) {
            cerr << "Erro ao criar arquivo de saída: " << outputPath << endl;
            return false;
        }
        
        // Recebe os pacotes de dados
        for (uint32_t i = 0; i < totalPackets; i++) {
            Packet dataPacket = Packet::receive(socket_fd);
            
            // Verifica se o número de sequência está correto
            if (dataPacket.seqn != i+1) {
                cerr << "Erro: Pacote fora de ordem. Esperado " << i+1 
                     << ", recebido " << dataPacket.seqn << endl;
                outFile.close();
                return false;
            }
            
            // Escreve o payload no arquivo
            outFile.write(dataPacket.payload.c_str(), dataPacket.payload.length());
        }
        
        outFile.close();
        cout << "Arquivo " << fileName << " recebido com sucesso e salvo em " << outputPath << endl;
        return true;
    } catch (const std::runtime_error& e) {
        cerr << "Erro ao receber arquivo: " << e.what() << endl;
        return false;
    }
}

// TODO:
// Packet { Type: 4, Seqn: 0, TotalSize: 0, Length: 0, Payload: "" }
// ATENCAO!!!! DEBUGAR TODO CÓDIGO!!! ADSFASFDSA Error sending error packet: Error writing to socket
// 5
// Exceção: Connection closed by peer
// Error connecting to server
void Packet::send_error(int socket_fd) {
    Packet errorPacket(static_cast<uint16_t>(Packet::Type::ERROR), 0, 0, 5, "ERROR");
    std::cout << errorPacket.to_string() << std::endl;
    try {
        errorPacket.send(socket_fd);
    } catch (const std::exception& e) {
        std::cerr << "ATENCAO!!!! DEBUGAR TODO CÓDIGO!!! ADSFASFDSA Error sending error packet: " << e.what() << std::endl;
    }
}

void Packet::send_ack(int socket_fd) {
    Packet ackPacket(static_cast<uint16_t>(Packet::Type::ACK), 0, 0, 3, "ACK");
    try {
        ackPacket.send(socket_fd);
    } catch (const std::exception& e) {
        std::cerr << "ATENCAO!!!! DEBUGAR TODO CÓDIGO!!! Error sending ack packet: " << e.what() << std::endl;
    }
}