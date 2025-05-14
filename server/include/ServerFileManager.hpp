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

    void create_sync_dir(const std::string& username);
    
    // Lista os arquivos no diretório do servidor
    void list_files();
    
    // Retorna uma string formatada com a listagem dos arquivos
    std::string get_files_list();

private:
    std::string username;
};

#endif