#ifndef WATCHER_HPP
#define WATCHER_HPP

#include <string>

class Watcher {
private:
    std::string path;

public:
    Watcher(const std::string& path);
    void start(); // Inicia a monitoração da pasta
};

#endif // WATCHER_HPP
