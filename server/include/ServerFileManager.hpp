#ifndef SERVERFILEMANAGER_HPP
#define SERVERFILEMANAGER_HPP

#include <iostream>
#include <iomanip>
#include <filesystem>
#include <chrono>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>

namespace fs = std::filesystem;

class ServerFileManager {
public:
    void create_sync_dir(std::string username);

private:
};

#endif // ServerFILEMANAGER_HPP