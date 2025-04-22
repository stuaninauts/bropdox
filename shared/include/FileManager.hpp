#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP

#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;
class FileManager {
private:
    // SyncIn: Detecta modificações locais (loop)
    // Pull: Recebe modificações do servidor (chamado pelo communication manager)
public:
    string username; // Nome do usuário

    // Construtor
    FileManager(string username);
    void create_sync_dir(string path);
    static void list_files_in_directory(const std::string& path);
};

#endif // FILEMANAGER_HPP

