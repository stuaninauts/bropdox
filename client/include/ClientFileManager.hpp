#ifndef CLIENT_FILE_MANAGER_HPP
#define CLIENT_FILE_MANAGER_HPP

#include <filesystem>
#include <string>
#include <iostream>
#include <FileDisplayFormatter.hpp>

namespace fs = std::filesystem;

class ClientFileManager {
public:
    // Cria o diretório de sincronização se não existir
    void create_sync_dir(const std::string& sync_dir_path);
    

    // Remove o diretório antigo para criar novo
    void remove_sync_dir();
    
    // Lista arquivos no diretório de sincronização
    void list_files();
    
    // Retorna uma string formatada com a listagem dos arquivos do cliente
    std::string get_files_list();
    
    // Monitora mudanças no diretório de sincronização
    
    static bool delete_local_file(const std::string filename);

    bool upload_local_file(const std::string& file_path);

    bool download_local_file(const std::string& filename);

    std::string sync_dir_path;
};

#endif