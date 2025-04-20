#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP

#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;
class FileManager {
private:
    string username; // Nome do usuário
    // SyncIn: Detecta modificações locais (loop)
    // Pull: Recebe modificações do servidor (chamado pelo communication manager)
public:
    // Construtor
    FileManager(string username);
    void create_sync_dir();
};

#endif // FILEMANAGER_HPP

