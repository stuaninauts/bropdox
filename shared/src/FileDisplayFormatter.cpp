#include "FileDisplayFormatter.hpp"

void FileDisplayFormatter::print_table_header(std::ostream& out) {
    out << std::left 
        << std::setw(FILE_COLUMN_WIDTH) << "File"
        << std::setw(TIME_COLUMN_WIDTH) << "Last Access (atime)"
        << std::setw(TIME_COLUMN_WIDTH) << "Modification (mtime)"
        << "Creation/Metadata (ctime)" << std::endl;
}

std::string FileDisplayFormatter::format_time(std::time_t raw_time) {
    std::string time_str = std::ctime(&raw_time);
    if (!time_str.empty() && time_str.back() == '\n') {
        time_str.pop_back(); // remove '\n'
    }
    return time_str;
}

bool FileDisplayFormatter::get_file_stat(const fs::path& path, struct stat& st) {
    return stat(path.c_str(), &st) == 0;
}

void FileDisplayFormatter::print_file_info(const fs::directory_entry& entry, std::ostream& out) {
    struct stat st{};
    if (!get_file_stat(entry.path(), st)) {
        out << "Error accessing: " << entry.path() << std::endl;
        return;
    }

    std::string atime = format_time(st.st_atime);
    std::string mtime = format_time(st.st_mtime);
    std::string ctime = format_time(st.st_ctime);

    out << std::left 
        << std::setw(FILE_COLUMN_WIDTH) << entry.path().filename().string()
        << std::setw(TIME_COLUMN_WIDTH) << atime
        << std::setw(TIME_COLUMN_WIDTH) << mtime
        << ctime << std::endl;
}

std::string FileDisplayFormatter::get_file_info_string(const fs::directory_entry& entry) {
    std::stringstream ss;
    print_file_info(entry, ss);
    return ss.str();
}

std::string FileDisplayFormatter::get_table_header_string() {
    std::stringstream ss;
    print_table_header(ss);
    return ss.str();
}
