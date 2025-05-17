#include "FileManager.hpp"
#include "FileDisplayFormatter.hpp"

bool FileManager::copy_file(const fs::path& source_path, const fs::path& destination_path) {
    try {
        fs::copy_file(source_path, destination_path, fs::copy_options::overwrite_existing);
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Erro ao copiar arquivo: " << e.what() << '\n';
        return false;
    }
}

bool FileManager::delete_file(const fs::path& file_path) {
    try {
        return fs::remove(file_path);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Erro ao deletar arquivo: " << e.what() << '\n';
        return false;
    }
}

bool FileManager::delete_all_files_in_directory(const fs::path& directory_path) {
    try {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            std::cerr << "Caminho inválido ou não é um diretório: " << directory_path << '\n';
            return false;
        }

        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                fs::remove(entry.path());
            }
        }
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Erro ao deletar arquivos do diretório: " << e.what() << '\n';
        return false;
    }
}

std::string FileManager::get_formatted_file_list(const fs::path& directory_path) {
    std::stringstream ss;

    try {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            ss << "Caminho inválido ou não é um diretório: " << directory_path << '\n';
            return ss.str();
        }

        ss << FileDisplayFormatter::get_table_header_string();

        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                ss << FileDisplayFormatter::get_file_info_string(entry);
            }
        }
    } catch (const fs::filesystem_error& e) {
        ss << "Erro ao listar arquivos: " << e.what() << '\n';
    }

    return ss.str();
}
