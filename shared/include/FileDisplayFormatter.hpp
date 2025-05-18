#ifndef FILE_DISPLAY_FORMATTER_HPP
#define FILE_DISPLAY_FORMATTER_HPP

#include <string>
#include <filesystem>
#include <sys/stat.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <ctime>

namespace fs = std::filesystem;

class FileDisplayFormatter {
public:
    static void print_table_header(std::ostream& out = std::cout);
    static std::string format_time(std::time_t raw_time);
    static bool get_file_stat(const fs::path& path, struct stat& st);
    static void print_file_info(const fs::directory_entry& entry, std::ostream& out = std::cout);
    static std::string get_file_info_string(const fs::directory_entry& entry);
    static std::string get_table_header_string();
    
    static const int FILE_COLUMN_WIDTH = 30;
    static const int TIME_COLUMN_WIDTH = 30;
};

#endif // FILE_DISPLAY_FORMATTER_HPP