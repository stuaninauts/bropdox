#include "Utils.hpp"
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <ctime>
#include <iomanip>

void Utils::list_files_in_directory(const std::string& path) {
    DIR* dir;
    struct dirent* entry;

    dir = opendir(path.c_str());
    if (dir == nullptr) {
        std::cerr << "Erro ao abrir o diretório: " << path << std::endl;
        return;
    }

    std::cout << std::left << std::setw(30) << "Arquivo"
              << std::setw(30) << "Último Acesso (atime)"
              << std::setw(30) << "Modificação (mtime)"
              << "Criação/Metadata (ctime)" << std::endl;

    while ((entry = readdir(dir)) != nullptr) {
        if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0)
            continue;

        std::string full_path = path + "/" + entry->d_name;

        struct stat file_stat;
        if (stat(full_path.c_str(), &file_stat) == 0) {
            // Remove o \n da string retornada por ctime
            auto format_time = [](time_t raw_time) {
                std::string time_str = std::ctime(&raw_time);
                time_str.pop_back(); // remove o '\n'
                return time_str;
            };

            std::string atime = format_time(file_stat.st_atime);
            std::string mtime = format_time(file_stat.st_mtime);
            std::string ctime = format_time(file_stat.st_ctime);

            std::cout << std::left << std::setw(30) << entry->d_name
                      << std::setw(30) << atime
                      << std::setw(30) << mtime
                      << ctime << std::endl;
        } else {
            std::cerr << "Erro ao obter informações de: " << full_path << std::endl;
        }
    }

    closedir(dir);
}
