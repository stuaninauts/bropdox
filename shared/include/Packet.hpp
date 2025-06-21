#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint>
#include <string>
#include <stdexcept>

class Packet {
public:
    // Tipos de pacote como enum class para maior segurança
    enum class Type : uint16_t {
        DATA = 1,
        CMD = 2, 
        ACK = 3,
        ERROR = 4,
        DELETE = 5, 
        EMPTY = 6,
        IP = 7,
        USERNAME = 8,
        SERVER = 9,
        CLIENT = 10,
        PORT = 11,
        ID = 12,
        HEARTBEAT = 13,
        DIRECTORY = 14,
        ELECTION = 15,
        ELECTED = 16,
    };

    // Construtor
    Packet(uint16_t type, uint16_t seqn, uint32_t total_size, 
           uint16_t length, const std::string& payload);
    
    Packet() = default;

    uint16_t type;
    uint16_t seqn;
    uint32_t total_size;
    uint16_t length;
    std::string payload;

    static bool send_file(int socket_fd, const std::string& filePath);
    static std::string build_output_path(const std::string& fileName, const std::string& outputDir);
    static bool receive_file(int socket_fd, const std::string& fileName, const std::string& outputDir, uint32_t totalPackets);
    static bool send_multiple_files(int socket_fd, const std::string& username);
    static bool receive_multiple_files(int socket_fd, const std::string& output_dir);
    static void send_error(int socket_fd);

    // Enviar e receber pacotes
    void send(int socket_fd) const;
    static Packet receive(int socket_fd);

    // Representação textual para depuração
    std::string to_string() const;
    
    // Constantes para tamanhos dos campos (usadas na serialização)
    static constexpr size_t HEADER_SIZE = sizeof(type) + sizeof(seqn) + 
                                         sizeof(total_size) + sizeof(length);

private:
    // Serialização
    std::string serialize() const;
    static Packet deserialize(const std::string& data);

    // Funções de leitura e escrita em socket
    static void read_socket(int socket_fd, char* buffer, size_t size);
    static void write_socket(int socket_fd, const char* buffer, size_t size);

    // Validação de comprimento do payload
    void validate_length() const {
        if (payload.length() != length) {
            throw std::logic_error("Payload length mismatch");
        }
    }
};

#endif