#include "FileManager.hpp"
#include "FileDisplayFormatter.hpp"

bool FileManager::copy_file(const fs::path& source_path, const fs::path& destination_path) {
    try {
        fs::copy_file(source_path, destination_path, fs::copy_options::overwrite_existing);
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cout << "Error copying file: " << e.what() << '\n';
        return false;
    }
}

bool FileManager::delete_file(const fs::path& file_path) {
    try {
        return fs::remove(file_path);
    } catch (const fs::filesystem_error& e) {
        std::cout << "Error deleting file: " << e.what() << '\n';
        return false;
    }
}

bool FileManager::delete_all_files_in_directory(const fs::path& directory_path) {
    try {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            std::cout << "Invalid path or not a directory: " << directory_path << '\n';
            return false;
        }

        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                fs::remove(entry.path());
            }
        }
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cout << "Error deleting files from directory: " << e.what() << '\n';
        return false;
    }
}

std::string FileManager::get_formatted_file_list(const fs::path& directory_path) {
    std::stringstream ss;

    try {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            ss << "Invalid path or not a directory: " << directory_path << '\n';
            return ss.str();
        }

        ss << FileDisplayFormatter::get_table_header_string();

        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                ss << FileDisplayFormatter::get_file_info_string(entry);
            }
        }
    } catch (const fs::filesystem_error& e) {
        ss << "Error listing files: " << e.what() << '\n';
    }

    return ss.str();
}

bool FileManager::create_directory(const fs::path& directory_path) {
    try {
        if (fs::create_directories(directory_path)) {
            std::cout << "Directory created: " + directory_path.string() << std::endl;
            return true;
        } else {
            std::cout << "Directory already exists: " + directory_path.string() << std::endl;
            return false;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating directory: " + std::string(e.what()) << std::endl;
        return false;
    }
}

bool FileManager::remove_directory(const fs::path& directory_path) {
    try {
        if (fs::remove(directory_path)) {
            std::cout << "Directory removed: " + directory_path.string() << std::endl;
            return true;
        } else {
            std::cout << "Directory does not exist: " + directory_path.string() << std::endl;
            return false;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error removing directory: " + std::string(e.what()) << std::endl;
        return false;
    }
}
