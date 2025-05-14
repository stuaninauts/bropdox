#include <ServerFileManager.hpp>

void ServerFileManager::create_sync_dir() {
    int status;
    
    // cria pasta base do bropdox
    std::string base_folder = "./server/bropdox";
    status = mkdir(base_folder.c_str(), 0777);
    if(status != 0 && errno != EEXIST) {
        std::cerr << "[ERRO] Falha ao criar pasta base: " << base_folder << std::endl;
        return;
    }

    // cria sync_dir do usuário
    server_dir_path = base_folder + "/sync_dir_" + username;
    status = mkdir(server_dir_path.c_str(), 0777);
    if(status != 0 && errno != EEXIST) {
        std::cerr << "[ERRO] Falha ao criar pasta do usuário: " << server_dir_path << std::endl;
    } else {
        std::cout << "Diretório do usuário criado/verificado: " << server_dir_path << std::endl;
    }
}

ServerFileManager::ServerFileManager(const std::string& username) {
    // Construtor agora recebe username e cria o caminho correto
    server_dir_path = "./server/bropdox/sync_dir_" + username;
    
    // Verifica se o diretório existe, se não, cria
    try {
        if (!fs::exists(server_dir_path)) {
            fs::create_directories(server_dir_path);
            std::cout << "Diretório do usuário criado: " << server_dir_path << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Erro ao verificar/criar diretório do usuário: " << e.what() << std::endl;
    }
}

void ServerFileManager::list_files(const std::string& username) {
    std::string user_dir = "./server/bropdox/sync_dir_" + username;
    
    if (!fs::exists(user_dir) || !fs::is_directory(user_dir)) {
        std::cerr << "Diretório do usuário não encontrado: " << user_dir << std::endl;
        return;
    }

    FileDisplayFormatter::print_table_header();

    for (const auto& entry : fs::directory_iterator(user_dir)) {
        if (entry.is_regular_file()) {
            FileDisplayFormatter::print_file_info(entry);
        }
    }
}

std::string ServerFileManager::get_files_list(const std::string& username) {
    std::string user_dir = "./server/bropdox/sync_dir_" + username;
    
    if (!fs::exists(user_dir) || !fs::is_directory(user_dir)) {
        return "Diretório do usuário não encontrado: " + user_dir;
    }

    std::stringstream ss;
    ss << FileDisplayFormatter::get_table_header_string();

    for (const auto& entry : fs::directory_iterator(user_dir)) {
        if (entry.is_regular_file()) {
            FileDisplayFormatter::print_file_info(entry, ss);
        }
    }

    return ss.str();
}
