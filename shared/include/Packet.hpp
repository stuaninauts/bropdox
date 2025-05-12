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
        ACK = 3,       // Pacote de confirmação
        ERROR = 4      // Pacote de erro
    };

    // Construtor
    Packet(uint16_t type, uint16_t seqn, uint32_t total_size, 
           uint16_t length, const std::string& payload);
    

    // Getters
    uint16_t get_type() const noexcept { return type; }
    Type get_enum_type() const noexcept { return static_cast<Type>(type); }
    uint16_t get_seqn() const noexcept { return seqn; }
    uint32_t get_total_size() const noexcept { return total_size; }
    uint16_t get_length() const noexcept { return length; }
    const std::string& get_payload() const noexcept { return payload; }

    // Setters com validação básica
    void set_type(uint16_t type);
    void set_type(Type type) noexcept { this->type = static_cast<uint16_t>(type); }
    void set_seqn(uint16_t seqn) noexcept { this->seqn = seqn; }
    void set_total_size(uint32_t total_size) noexcept { this->total_size = total_size; }
    void set_length(uint16_t length);
    void set_payload(const std::string& payload);

    // Serialização
    std::string serialize() const;
    static Packet deserialize(const std::string& data);

    
    // Enviar e receber pacotes
    void send(int socket_fd) const;
    static Packet receive(int socket_fd);

    // Representação textual para depuração
    std::string to_string() const;

    // Funções de leitura e escrita em socket
    static void read_socket(int socket_fd, char* buffer, size_t size);
    static void write_socket(int socket_fd, const char* buffer, size_t size);

private:
    uint16_t type;         // Tipo do pacote (convertível para Type)
    uint16_t seqn;         // Número de sequência
    uint32_t total_size;   // Número total de fragmentos
    uint16_t length;       // Comprimento do payload
    std::string payload;   // Dados do pacote

    // Constantes para tamanhos dos campos (usadas na serialização)
    static constexpr size_t HEADER_SIZE = sizeof(type) + sizeof(seqn) + 
                                         sizeof(total_size) + sizeof(length);


    // Validação de comprimento do payload
    void validate_length() const {
        if (payload.length() != length) {
            throw std::logic_error("Payload length mismatch");
        }
    }
};

#endif