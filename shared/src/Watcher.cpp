#include "Watcher.hpp"
#include <sys/inotify.h>
#include <unistd.h>
#include <iostream>
#include <limits.h>
#include <cstring>

Watcher::Watcher(const std::string& path) : path(path) {}

void Watcher::start() {
    int fd = inotify_init();
    if (fd < 0) {
        std::cerr << "Erro ao inicializar inotify\n";
        return;
    }

    int wd = inotify_add_watch(fd, path.c_str(), IN_CREATE | IN_MODIFY | IN_DELETE);
    if (wd < 0) {
        std::cerr << "Erro ao adicionar watch para " << path << "\n";
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
