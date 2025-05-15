#include <ClientsDevices.hpp>


bool ClientsDevices::add_client_socket(const std::string& username, int sockfd)
{
    auto it = clients_sockets.find(username);
    if (it != clients_sockets.end()) {
        if (clients_sockets[username].size() >= 2) {
            std::cout << "Máximo de 2 conexões alcançado para o usuário " << username << ".\n";
            close(sockfd);
            // MANDAR MSG PRA MATAR CLIENTE
            return false;
        }
        clients_sockets[username].push_back(sockfd);
    } else {
        clients_sockets[username].push_back(sockfd);
    }

    print_clients_sockets();  // <-- PERIGOSO SE USAR O MESMO MUTEX
    return true;
}

void ClientsDevices::remove_client_socket(const std::string& username, int sockfd)
{
    auto it = clients_sockets.find(username);
    if (it != clients_sockets.end()) {
        auto& sockets = it->second;

        // Remove o sockfd do vetor
        sockets.erase(std::remove(sockets.begin(), sockets.end(), sockfd), sockets.end());

        std::cout << "Socket " << sockfd << " removido do usuário " << username << ".\n";

        // Se não restarem mais sockets, remove o usuário do mapa
        if (sockets.empty()) {
            clients_sockets.erase(it);
            std::cout << "Usuário " << username << " removido do mapa (sem conexões ativas).\n";
        }
    } else {
        std::cout << "Usuário " << username << " não encontrado para remoção.\n";
    }

    print_clients_sockets();
}

void ClientsDevices::print_clients_sockets() {
    // Verifica se o mapa está vazio
    if (clients_sockets.empty()) {
        std::cout << "O mapa de clientes está vazio!" << std::endl;
    }

    // Itera sobre o unordered_map
    for (const auto& pair : clients_sockets) {
        const std::string& username = pair.first;
        const std::vector<int>& sockets = pair.second;

        std::cout << "Usuário: " << username << " - Conexões de download: ";

        for (int socket : sockets) {
            std::cout << socket << " ";
        }
        std::cout << std::endl;
    }
}