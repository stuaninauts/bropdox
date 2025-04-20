#include "FileManager.hpp"
#include <sys/stat.h>
#include <sys/types.h> 
#include <cstring>      


FileManager::FileManager(string username) {
    this->username = username;
}

void FileManager::create_sync_dir() {
    string folder_name = "client/sync_dir_" + username;

    int status = mkdir(folder_name.c_str(), 0777);
    if(status !=0) {
        if (errno != EEXIST) {
             cerr << "[ERRO] Falha ao criar pasta: " << strerror(errno) << endl;
        } 
    }
}
