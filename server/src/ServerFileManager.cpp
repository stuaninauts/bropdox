#include <ServerFileManager.hpp>

void ServerFileManager::create_sync_dir(std::string username) {
    std::string folder_name = "./server/bropdox/sync_dir_" + username;

    int status = mkdir(folder_name.c_str(), 0777);
    if(status !=0) {
        if (errno != EEXIST) {
             std::cerr << "[ERRO] Falha ao criar pasta " << std::endl;
        } 
    }
}