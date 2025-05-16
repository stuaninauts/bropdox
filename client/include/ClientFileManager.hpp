#ifndef CLIENT_FILE_MANAGER_HPP
#define CLIENT_FILE_MANAGER_HPP

#include <filesystem>
#include <string>
#include <iostream>
#include <FileDisplayFormatter.hpp>

namespace fs = std::filesystem;

class ClientFileManager {
public:
    ClientFileManager(const std::string& sync_dir_path = "./sync_dir");
    
    // Cria o diretório de sincronização se não existir
    void create_sync_dir();
    

    // Remove o diretório antigo para criar novo
    void remove_sync_dir();
    
    // Lista arquivos no diretório de sincronização
    void list_files();
    
    // Retorna uma string formatada com a listagem dos arquivos do cliente
    std::string get_files_list();
    
    // Monitora mudanças no diretório de sincronização
    void watch();

private:
    std::string sync_dir_path;
};

#endif