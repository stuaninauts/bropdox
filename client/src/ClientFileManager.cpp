#include "ClientFileManager.hpp"
#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>
#include <cstring>
#include <sstream>

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

ClientFileManager::ClientFileManager(const std::string& sync_dir_path)
    : sync_dir_path(sync_dir_path) {
}

void ClientFileManager::create_sync_dir() {
    try {
        if (!fs::exists(sync_dir_path)) {
            fs::create_directories(sync_dir_path);
            std::cout << "Diretório criado: " << sync_dir_path << std::endl;
        } else {
            std::cout << "Diretório já existe: " << sync_dir_path << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Erro ao criar diretório: " << e.what() << std::endl;
    }
}

void ClientFileManager::list_files() {
    if (!fs::exists(sync_dir_path) || !fs::is_directory(sync_dir_path)) {
        std::cerr << "Diretório inválido: " << sync_dir_path << std::endl;
        return;
    }

    FileDisplayFormatter::print_table_header();

    for (const auto& entry : fs::directory_iterator(sync_dir_path)) {
        if (entry.is_regular_file()) {
            FileDisplayFormatter::print_file_info(entry);
        }
    }
}

std::string ClientFileManager::get_files_list() {
    if (!fs::exists(sync_dir_path) || !fs::is_directory(sync_dir_path)) {
        return "Diretório inválido: " + sync_dir_path;
    }

    std::stringstream ss;
    
    // Adiciona cabeçalho
    ss << FileDisplayFormatter::get_table_header_string();

    // Adiciona informações dos arquivos
    for (const auto& entry : fs::directory_iterator(sync_dir_path)) {
        if (entry.is_regular_file()) {
            FileDisplayFormatter::print_file_info(entry, ss);
        }
    }

    return ss.str();
}

void ClientFileManager::watch() {
    int fd = inotify_init();
    if (fd < 0) {
        std::cerr << "Erro ao inicializar inotify\n";
        return;
    }

    int wd = inotify_add_watch(fd, sync_dir_path.c_str(), IN_CREATE | IN_MODIFY | IN_DELETE);
    if (wd < 0) {
        std::cerr << "Erro ao adicionar watch para " << sync_dir_path << "\n";
        close(fd);
        return;
    }

    const size_t EVENT_SIZE = sizeof(struct inotify_event);
    const size_t BUF_LEN = 1024 * (EVENT_SIZE + NAME_MAX + 1);
    char buffer[BUF_LEN];

    while (true) {
        int length = read(fd, buffer, BUF_LEN);
        if (length < 0) {
            std::cerr << "Erro na leitura do inotify\n";
            break;
        }

        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];
            // TODO -> Possivelmente necessario utilizacao de lock
            if (event->len) {
                if (event->mask & IN_CREATE) {
                    std::cout << "[INOTIFY] Arquivo criado: " << event->name << std::endl;
                } else if (event->mask & IN_MODIFY) {
                    std::cout << "[INOTIFY] Arquivo modificado: " << event->name << std::endl;
                } else if (event->mask & IN_DELETE) {
                    std::cout << "[INOTIFY] Arquivo deletado: " << event->name << std::endl;
                }
            }

            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
}