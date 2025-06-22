#include <FileManager.hpp>
#include <FileDisplayFormatter.hpp>

bool FileManager::copy_file(const fs::path& source_path, const fs::path& destination_path) {
    try {
        fs::copy_file(source_path, destination_path, fs::copy_options::overwrite_existing);
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[ FILE MANAGER ] " << "Error copying file: " << e.what() << '\n';
        return false;
    }
}

bool FileManager::delete_file(const fs::path& file_path) {
    try {
        return fs::remove(file_path);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[ FILE MANAGER ] " << "Error deleting file: " << e.what() << '\n';
        return false;
    }
}

bool FileManager::delete_all_files_in_directory(const fs::path& directory_path) {
    try {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            std::cerr << "[ FILE MANAGER ] " << "Invalid path or not a directory: " << directory_path << '\n';
            return false;
        }

        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                fs::remove(entry.path());
            }
        }
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[ FILE MANAGER ] " << "Error deleting files from directory: " << e.what() << '\n';
        return false;
    }
}

std::string FileManager::get_formatted_file_list(const fs::path& directory_path) {
    std::stringstream ss;

    try {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            ss << "[ FILE MANAGER ] " << "Invalid path or not a directory: " << directory_path << '\n';
            return ss.str();
        }

        ss << FileDisplayFormatter::get_table_header_string();

        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                ss << FileDisplayFormatter::get_file_info_string(entry);
            }
        }
    } catch (const fs::filesystem_error& e) {
        ss << "[ FILE MANAGER ] " << "Error listing files: " << e.what() << '\n';
    }

    return ss.str();
}

bool FileManager::create_directory(const fs::path& directory_path) {
    try {
        if (fs::create_directories(directory_path)) {
            std::cout << "[ FILE MANAGER ] " << "Directory created: " + directory_path.string() << std::endl;
            return true;
        } else {
            std::cout << "[ FILE MANAGER ] " << "Directory already exists: " + directory_path.string() << std::endl;
            return false;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[ FILE MANAGER ] " << "Error creating directory: " + std::string(e.what()) << std::endl;
        return false;
    }
}

bool FileManager::remove_directory(const fs::path& directory_path) {
    try {
        if (fs::remove(directory_path)) {
            std::cout << "[ FILE MANAGER ] " << "Directory removed: " + directory_path.string() << std::endl;
            return true;
        } else {
            std::cout << "[ FILE MANAGER ] " << "Directory does not exist: " + directory_path.string() << std::endl;
            return false;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[ FILE MANAGER ] " << "Error removing directory: " + std::string(e.what()) << std::endl;
        return false;
    }
}

bool FileManager::move_files_between_directories(const fs::path& src_directory, const fs::path& dest_directory) {
    try {
        if (!fs::exists(src_directory) || !fs::is_directory(src_directory)) {
            std::cerr << "[ FILE MANAGER ] " << "Source directory does not exist or is not a directory: " << src_directory << '\n';
            return false;
        }

        if (!fs::exists(dest_directory)) {
            fs::create_directories(dest_directory);
        }

        for (const auto& entry : fs::recursive_directory_iterator(src_directory)) {
            const auto& src_path = entry.path();
            auto relative_path = fs::relative(src_path, src_directory);
            fs::path dest_path = dest_directory / relative_path;

            if (fs::is_directory(src_path)) {
                fs::create_directories(dest_path);
            } else if (fs::is_regular_file(src_path)) {
                fs::create_directories(dest_path.parent_path());
                fs::copy_file(src_path, dest_path, fs::copy_options::overwrite_existing);
            }
        }
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[ FILE MANAGER ] " << "Error copying files: " << e.what() << '\n';
        return false;
    }
}