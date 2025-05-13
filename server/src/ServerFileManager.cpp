#include <ServerFileManager.hpp>

void ServerFileManager::create_sync_dir(std::string username) {
    int status;
    
    // cria pasta do bropdox
    std::string folder_name = "./server/bropdox";
    status = mkdir(folder_name.c_str(), 0777);
    if(status !=0) {
        if (errno != EEXIST) {
             std::cerr << "[ERRO] Falha ao criar pasta " << std::endl;
        } 
    }

    // cria sync_dir do usuário
    folder_name += "/sync_dir_" + username;
    status = mkdir(folder_name.c_str(), 0777);
    if(status !=0) {
        if (errno != EEXIST) {
             std::cerr << "[ERRO] Falha ao criar pasta " << std::endl;
        } 
    }
}