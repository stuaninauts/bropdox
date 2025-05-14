#include <ServerFileManager.hpp>

void ServerFileManager::create_sync_dir() {
    int status;
    
    // cria pasta do bropdox
    std::string folder_name = "./server/bropdox";
    status = mkdir(folder_name.c_str(), 0777);
    if(status !=0) {
        if (errno != EEXIST) {
             std::cerr << "[ERRO] Falha ao criar pasta " << std::endl;
        } 
    }

    // cria sync_dir do usuário
    folder_name += "/sync_dir_" + username;
    status = mkdir(folder_name.c_str(), 0777);
    if(status !=0) {
        if (errno != EEXIST) {
             std::cerr << "[ERRO] Falha ao criar pasta " << std::endl;
        } 
    }
}

ServerFileManager::ServerFileManager(const std::string& username_) {
    username = username_;
    server_dir_path = "./server/bropdox/sync_dir_" + username_;
    
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

void ServerFileManager::list_files() {
    std::string user_dir = "./server/bropdox/sync_dir_" + username;
    std::cout << "Listando arquivos no diretório: " << user_dir << std::endl;
    
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

std::string ServerFileManager::get_files_list() {
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
