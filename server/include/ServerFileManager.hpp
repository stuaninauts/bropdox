#ifndef SERVER_FILE_MANAGER_HPP
#define SERVER_FILE_MANAGER_HPP

#include <string>
#include <filesystem>
#include <iostream>
#include <sstream>
#include "FileDisplayFormatter.hpp"

namespace fs = std::filesystem;

class ServerFileManager {
public:
    ServerFileManager(const std::string& username);
    void create_sync_dir();
    void list_files();
    void write_file(int socket_receive);
    void delete_file(const std::string filename);

    std::string get_files_list();
    std::string username;
    std::string server_dir_path;
};

#endif