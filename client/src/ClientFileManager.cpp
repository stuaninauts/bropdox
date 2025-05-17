#include "ClientFileManager.hpp"
#include <unistd.h>
#include <limits.h>
#include <cstring>
#include <sstream>

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

void ClientFileManager::create_sync_dir(const std::string& sync_dir_path) {
    this->sync_dir_path = sync_dir_path;
    remove_sync_dir();
    try {
        if (!fs::exists(sync_dir_path)) {
            std::cout<< "create" << sync_dir_path << std::endl;
            fs::create_directories(sync_dir_path);
            // std::cout << "Diretório criado: " << sync_dir_path << std::endl;
        } else {
           // std::cout << "Diretório já existe: " << sync_dir_path << std::endl;
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

    FileDisplayFormatter::print_table_header();

    for (const auto& entry : fs::directory_iterator(sync_dir_path)) {
        if (entry.is_regular_file()) {
            FileDisplayFormatter::print_file_info(entry);
        }
    }
}

std::string ClientFileManager::get_files_list() {
    if (!fs::exists(sync_dir_path) || !fs::is_directory(sync_dir_path)) {
        return "Diretório inválido: " + sync_dir_path;
    }

    std::stringstream ss;
    
    // Adiciona cabeçalho
    ss << FileDisplayFormatter::get_table_header_string();

    // Adiciona informações dos arquivos
    for (const auto& entry : fs::directory_iterator(sync_dir_path)) {
        if (entry.is_regular_file()) {
            FileDisplayFormatter::print_file_info(entry, ss);
        }
    }

    return ss.str();
}

void ClientFileManager::remove_sync_dir(){
       try {
        if (fs::exists(sync_dir_path)) {
            fs::remove_all(sync_dir_path);
            // std::cout << "Diretório deletado: " << sync_dir_path << std::endl;
        } else {
            // std::cout << "Diretório não existe: " << sync_dir_path << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Erro ao deletar diretório: " << e.what() << std::endl;
    } 
};

bool ClientFileManager::delete_local_file(const std::string filename) {
    std::filesystem::path filePath = "./sync_dir";
    filePath /= filename;

    try {
        if (std::filesystem::exists(filePath)) {
            std::filesystem::remove(filePath);
            return true;
        } else {
            std::cerr << "Arquivo " << filePath << " não encontrado." << std::endl;
            return false;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Erro ao deletar o arquivo: " << e.what() << std::endl;
        return false;
    }
}

bool ClientFileManager::upload_local_file(const std::string& file_path) {
    std::filesystem::path sourcePath(file_path);
    std::filesystem::path destinationDir = "./sync_dir/";
    std::filesystem::path destinationPath = destinationDir / sourcePath.filename(); // Copia com o mesmo nome

    try {
        // Verifica se o arquivo de origem existe
        if (!std::filesystem::exists(sourcePath)) {
            std::cerr << "Arquivo de origem não encontrado: " << sourcePath << std::endl;
            return false;
        }

        // Cria o diretório de destino se ele não existir
        if (!std::filesystem::exists(destinationDir)) {
            std::filesystem::create_directories(destinationDir);
        }

        // Copia o arquivo
        std::filesystem::copy_file(sourcePath, destinationPath, std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Erro ao fazer upload do arquivo: " << e.what() << std::endl;
        return false;
    }
}

bool ClientFileManager::download_local_file(const std::string& filename) {
    std::filesystem::path sourcePath = "./sync_dir/";
    sourcePath /= filename;

    std::filesystem::path destinationPath = std::filesystem::current_path() / filename;

    try {
        if (!std::filesystem::exists(sourcePath)) {
            std::cerr << "Arquivo não encontrado no diretório de sincronização: " << sourcePath << std::endl;
            return false;
        }

        std::filesystem::copy_file(sourcePath, destinationPath, std::filesystem::copy_options::overwrite_existing);

        std::cout << "Arquivo copiado com sucesso para: " << destinationPath << std::endl;
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cout << "Erro ao fazer download do arquivo: " << e.what() << std::endl;
        return false;
    }
}
