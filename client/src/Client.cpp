#include <Client.hpp>
#include <Packet.hpp>
#include <filesystem>
#include <FileManager.hpp>
#include <atomic>

// ======================================== //
// ================= PUBLIC =============== //
// ======================================== //

void Client::run(int initial_socket) {
    std::cout << "Connecting to server..." << std::endl;
    running.store(true);
    communicator.running_ptr = &running;

    if (!communicator.connect_to_server(initial_socket)) {
        std::cerr << "Error connecting to server" << std::endl;
        exit(1);
    }

    if (initial_socket < 0) {
        FileManager::delete_all_files_in_directory(sync_dir_path);
        FileManager::create_directory(sync_dir_path);
        communicator.get_sync_dir();
    }

    // Só inicializa o socket de escuta se for a primeira execução (não reconexão)
    if (initial_socket < 0) {
        initial_socket_new_alpha = Network::setup_socket_ipv4(port_backup, SOMAXCONN);
        if (initial_socket_new_alpha < 0) {
            std::cerr << "Erro ao inicializar socket de escuta para ALPHA na porta " << port_backup << std::endl;
            exit(1);
        }
    }

    std::cout << "Client connected to server successfully." << std::endl;

    std::thread thread_sync_remote(&Client::sync_remote, this);
    std::thread thread_sync_local(&Client::sync_local, this);
    std::thread thread_user_interface(&Client::user_interface, this);
    std::thread thread_new_alpha(&Client::handle_new_alpha_connection, this);

    thread_sync_remote.join();
    thread_sync_local.join();
    thread_user_interface.join();
    thread_new_alpha.join();
}

// ========================================= //
// ================= THREADS =============== //
// ========================================= //

void Client::sync_local() {
    communicator.watch_directory();
    std::cout << "[INOTIFY] Quitting" << std::endl;
}

void Client::sync_remote() {
    communicator.handle_server_update();
    std::cout << "[HANDLE SERVER UPDATE] Quitting" << std::endl;
}

void Client::user_interface() {
    while (communicator.socket_upload != -1 && running.load()) {
        std::string input;
        std::cout << "> ";
        getline(cin, input);
        
        std::vector<string> tokens = split_command(input);
        if (!tokens.empty()) {
            process_command(tokens);
        }
    }
}

// ========================================= //
// ================= PRIVATE =============== //
// ========================================= //

vector<string> Client::split_command(const string &command) {
    std::vector<string> tokens;
    std::string token;
    std::istringstream tokenStream(command);
    
    while (tokenStream >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

void Client::process_command(const std::vector<string> &tokens) {
    if (tokens.empty()) return;

    std::string command = tokens[0];
    transform(command.begin(), command.end(), command.begin(), ::tolower);

    std::filesystem::path file_path;
    if (tokens.size() == 2) {
        file_path = tokens[1];
    }

    if (command == "upload" && tokens.size() == 2) {
        FileManager::copy_file(file_path, sync_dir_path / file_path.filename());

    } else if (command == "download" && tokens.size() == 2) {
        FileManager::copy_file(sync_dir_path / file_path.filename(), std::filesystem::current_path() / file_path.filename());

    } else if (command == "delete" && tokens.size() == 2) {
        FileManager::delete_file(sync_dir_path / file_path.filename());

    } else if (command == "list_server") {
        communicator.list_server();

    } else if (command == "list_client") {
        std::cout << FileManager::get_formatted_file_list(sync_dir_path) << std::endl;

    } else if (command == "exit") {
        std::cout << "Closing session with server" << std::endl;
        communicator.exit_server();

    } else {
        std::cout << "Invalid command or missing arguments. Available commands:" << std::endl;
        std::cout << "# upload <path/filename.ext>" << std::endl;
        std::cout << "# download <filename.ext>" << std::endl;
        std::cout << "# delete <filename.ext>" << std::endl;
        std::cout << "# list_server" << std::endl;
        std::cout << "# list_client" << std::endl;
        std::cout << "# exit" << std::endl;
    }
}

void Client::handle_new_alpha_connection() {
    int new_alpha_socket;
    struct sockaddr_in new_alpha_address;
    socklen_t new_alpha_address_len = sizeof(struct sockaddr_in);
    std::cout << "Handling new ALPHA connection in port " << port_backup << "..." << std::endl;

    while (running.load()) {
        new_alpha_socket = accept(initial_socket_new_alpha, (struct sockaddr*) &new_alpha_address, &new_alpha_address_len);
        communicator.server_ip = Network::get_ipv4(new_alpha_socket);

        if (new_alpha_socket == -1) {
            std::cerr << "ERROR: Failed to accept new client" << std::endl;
            continue;
        }

        if (new_alpha_socket >= 0) {
            std::cout << "Nova conexão alfa recebida. Reinicializando cliente..." << std::endl;
            communicator.close_sockets();
            running.store(false);
            std::this_thread::sleep_for(std::chrono::seconds(3));

            run(new_alpha_socket);
            break;
        }
    }
}
