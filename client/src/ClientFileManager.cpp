#include "ClientFileManager.hpp"
#include <unistd.h>
#include <limits.h>
#include <cstring>
#include <sstream>

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

// void ClientFileManager::create_sync_dir(const std::string& sync_dir_path) {
//     this->sync_dir_path = sync_dir_path;
//     remove_sync_dir();
//     try {
//         if (!fs::exists(sync_dir_path)) {
//             std::cout << "Creating directory: " << sync_dir_path << std::endl;
//             fs::create_directories(sync_dir_path);
//             // std::cout << "Directory created: " << sync_dir_path << std::endl;
//         } else {
//             // std::cout << "Directory already exists: " << sync_dir_path << std::endl;
//         }
//     } catch (const fs::filesystem_error& e) {
//         std::cout << "Error while creating directory: " << e.what() << std::endl;
//     }
// }

// void ClientFileManager::list_files() {
//     if (!fs::exists(sync_dir_path) || !fs::is_directory(sync_dir_path)) {
//         std::cout << "Invalid directory: " << sync_dir_path << std::endl;
//         return;
//     }

//     FileDisplayFormatter::print_table_header();

//     for (const auto& entry : fs::directory_iterator(sync_dir_path)) {
//         if (entry.is_regular_file()) {
//             FileDisplayFormatter::print_file_info(entry);
//         }
//     }
// }

// std::string ClientFileManager::get_files_list() {
//     if (!fs::exists(sync_dir_path) || !fs::is_directory(sync_dir_path)) {
//         return "Invalid directory: " + sync_dir_path;
//     }

//     std::stringstream ss;
    
//     // Add header
//     ss << FileDisplayFormatter::get_table_header_string();

//     // Add file information
//     for (const auto& entry : fs::directory_iterator(sync_dir_path)) {
//         if (entry.is_regular_file()) {
//             FileDisplayFormatter::print_file_info(entry, ss);
//         }
//     }

//     return ss.str();
// }

// void ClientFileManager::remove_sync_dir(){
//     try {
//         if (fs::exists(sync_dir_path)) {
//             fs::remove_all(sync_dir_path);
//             // std::cout << "Directory deleted: " << sync_dir_path << std::endl;
//         } else {
//             // std::cout << "Directory does not exist: " << sync_dir_path << std::endl;
//         }
//     } catch (const fs::filesystem_error& e) {
//         std::cout << "Error while deleting directory: " << e.what() << std::endl;
//     } 
// };

bool ClientFileManager::delete_local_file(const std::string filename) {
    std::filesystem::path filePath = "./sync_dir";
    filePath /= filename;

    try {
        if (std::filesystem::exists(filePath)) {
            std::filesystem::remove(filePath);
            return true;
        } else {
            std::cout << "File not found: " << filePath << std::endl;
            return false;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cout << "Error while deleting file: " << e.what() << std::endl;
        return false;
    }
}

bool ClientFileManager::upload_local_file(const std::string& file_path) {
    std::filesystem::path sourcePath(file_path);
    std::filesystem::path destinationDir = "./sync_dir/";
    std::filesystem::path destinationPath = destinationDir / sourcePath.filename(); // Keep the same filename

    try {
        if (!std::filesystem::exists(sourcePath)) {
            std::cout << "Source file not found: " << sourcePath << std::endl;
            return false;
        }

        if (!std::filesystem::exists(destinationDir)) {
            std::filesystem::create_directories(destinationDir);
        }

        std::filesystem::copy_file(sourcePath, destinationPath, std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cout << "Error while uploading file: " << e.what() << std::endl;
        return false;
    }
}

bool ClientFileManager::download_local_file(const std::string& filename) {
    std::filesystem::path sourcePath = "./sync_dir/";
    sourcePath /= filename;

    std::filesystem::path destinationPath = std::filesystem::current_path() / filename;

    try {
        if (!std::filesystem::exists(sourcePath)) {
            std::cout << "File not found in sync directory: " << sourcePath << std::endl;
            return false;
        }

        std::filesystem::copy_file(sourcePath, destinationPath, std::filesystem::copy_options::overwrite_existing);

        std::cout << "File successfully copied to: " << destinationPath << std::endl;
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cout << "Error while downloading file: " << e.what() << std::endl;
        return false;
    }
}
