#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint>
#include <string>
#include <stdexcept>

class Packet {
public:
    // Tipos de pacote como enum class para maior segurança
    enum class Type : uint16_t {
        DATA = 1,      // Pacote de dados
        CMD = 2,       // Pacote de comando
        ACK = 3,      // Pacote de confirmação
        ERROR = 4,  // Pacote de erro
        DELETE = 5,      // Pacote de delete
        EMPTY = 6      // Pacote de empty
    };

    // Construtor
    Packet(uint16_t type, uint16_t seqn, uint32_t total_size, 
           uint16_t length, const std::string& payload);
    
    Packet() = default;

    uint16_t type;         // Tipo do pacote (convertível para Type)
    uint16_t seqn;         // Número de sequência
    uint32_t total_size;   // Número total de fragmentos
    uint16_t length;       // Comprimento do payload
    std::string payload;   // Dados do pacote

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