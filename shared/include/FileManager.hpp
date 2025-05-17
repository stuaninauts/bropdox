#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <filesystem>
#include <sys/stat.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <ctime>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool copy_file(const fs::path& source_path, const fs::path& destination_path);
    static bool delete_file(const fs::path& file_path);
    static bool delete_all_files_in_directory(const fs::path& directory_path);
    static std::string get_formatted_file_list(const fs::path& directory_path);
};

#endif // FILE_MANAGER_HPP
