#ifndef CLIENTFILEMANAGER_HPP
#define CLIENTFILEMANAGER_HPP

#include <iostream>
#include <iomanip>
#include <filesystem>
#include <chrono>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>

namespace fs = std::filesystem;

class ClientFileManager {
public:
    void list_files();
    void create_sync_dir();

private:
    std::string sync_dir_path = "./client/sync_dir";

    void print_table_header();
    std::string format_time(std::time_t raw_time);
    bool get_file_stat(const fs::path& path, struct stat& st);
    void print_file_info(const fs::directory_entry& entry);
};

#endif // CLIENTFILEMANAGER_HPP