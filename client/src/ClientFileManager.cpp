#include <ClientFileManager.hpp>

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

void ClientFileManager::create_sync_dir() {
    try {
        if (!fs::exists(sync_dir_path)) {
            fs::create_directories(sync_dir_path);
            std::cout << "Diretório criado: " << sync_dir_path << std::endl;
        } else {
            std::cout << "Diretório já existe: " << sync_dir_path << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Erro ao criar diretório: " << e.what() << std::endl;
    }
}

void ClientFileManager::list_files() {
    if (!fs::exists(sync_dir_path) || !fs::is_directory(sync_dir_path)) {
        std::cerr << "Diretório inválido: " << sync_dir_path << std::endl;
        return;
    }

    print_table_header();

    for (const auto& entry : fs::directory_iterator(sync_dir_path)) {
        if (entry.is_regular_file()) {
            print_file_info(entry);
        }
    }
}

// ========================================= //
// ================ PRIVATE ================ //
// ========================================= //

void ClientFileManager::print_table_header() {
    std::cout << std::left << std::setw(30) << "Arquivo"
              << std::setw(30) << "Último Acesso (atime)"
              << std::setw(30) << "Modificação (mtime)"
              << "Criação/Metadata (ctime)" << std::endl;
}

std::string ClientFileManager::format_time(std::time_t raw_time) {
    std::string time_str = std::ctime(&raw_time);
    time_str.pop_back(); // remove '\n'
    return time_str;
}

bool ClientFileManager::get_file_stat(const fs::path& path, struct stat& st) {
    return stat(path.c_str(), &st) == 0;
}

void ClientFileManager::print_file_info(const fs::directory_entry& entry) {
    struct stat st{};
    if (!get_file_stat(entry.path(), st)) {
        std::cerr << "Erro ao acessar: " << entry.path() << std::endl;
        return;
    }

    std::string atime = format_time(st.st_atime);
    std::string mtime = format_time(st.st_mtime);
    std::string ctime = format_time(st.st_ctime);

    std::cout << std::left << std::setw(30) << entry.path().filename().string()
              << std::setw(30) << atime
              << std::setw(30) << mtime
              << ctime << std::endl;
}