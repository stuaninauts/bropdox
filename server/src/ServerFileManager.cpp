#include <ServerFileManager.hpp>
#include <Packet.hpp>

void ServerFileManager::create_sync_dir() {
    int status;

    // creates bropdox folder
    std::string folder_name = "./sync_dir_server";
    status = mkdir(folder_name.c_str(), 0777);
    if (status != 0) {
        if (errno != EEXIST) {
            std::cerr << "[ERROR] Failed to create folder." << std::endl;
        } 
    }

    // creates user's sync_dir
    folder_name += "/sync_dir_" + username;
    status = mkdir(folder_name.c_str(), 0777);
    if (status != 0) {
        if (errno != EEXIST) {
            std::cerr << "[ERROR] Failed to create folder." << std::endl;
        } 
    }
}

ServerFileManager::ServerFileManager(const std::string& username_) {
    username = username_;
    server_dir_path = "./sync_dir_server/sync_dir_" + username_;

    // Checks if directory exists; if not, creates it
    try {
        if (!fs::exists(server_dir_path)) {
            fs::create_directories(server_dir_path);
            std::cout << "User directory created: " << server_dir_path << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error checking/creating user directory: " << e.what() << std::endl;
    }
}

void ServerFileManager::write_file(int socket_fd, const std::string filename, uint32_t total_packets) {
    Packet::receive_file(socket_fd, filename, server_dir_path, total_packets);
}

void ServerFileManager::delete_file(const std::string filename) {
    std::string filepath = server_dir_path + "/" + filename;

    if (std::remove(filepath.c_str()) == 0) {
        std::cout << "File " << filepath << " successfully deleted." << std::endl;
    } else {
        std::cerr << "Error deleting file: " << filepath << std::endl;
    }
}

void ServerFileManager::list_files() {
    std::string user_dir = "./sync_dir_server/sync_dir_" + username;
    std::cout << "Listing files in directory: " << user_dir << std::endl;

    if (!fs::exists(user_dir) || !fs::is_directory(user_dir)) {
        std::cerr << "User directory not found: " << user_dir << std::endl;
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
    std::string user_dir = "./sync_dir_server/sync_dir_" + username;

    if (!fs::exists(user_dir) || !fs::is_directory(user_dir)) {
        return "User directory not found: " + user_dir;
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
