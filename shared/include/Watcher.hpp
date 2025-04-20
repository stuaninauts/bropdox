#ifndef WATCHER_HPP
#define WATCHER_HPP

#include <string>
#include "FileManager.hpp"

class Watcher {
private:
    std::string path;
    FileManager& fileManager;

public:
    Watcher(const std::string& path, FileManager& fileManager);
    void start(); // Inicia a monitoração da pasta
};

#endif // WATCHER_HPP
