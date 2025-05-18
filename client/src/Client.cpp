#include <Client.hpp>
#include <Packet.hpp>
#include <filesystem>
#include <FileManager.hpp>

// ======================================== //
// ================= PUBLIC =============== //
// ======================================== //

void Client::run() {
    FileManager::delete_all_files_in_directory(sync_dir_path);
    FileManager::create_directory(sync_dir_path);

    cout << "Connecting to server..." << endl;
    if (!comm_manager.connect_to_server(server_ip, port, username, sync_dir_path)) {
        cout << "Error connecting to server" << endl;
        exit(1);
    }

    std::thread thread_sync_remote(&Client::sync_remote, this);
    std::thread thread_sync_local(&Client::sync_local, this);
    std::thread thread_user_interface(&Client::user_interface, this);

    thread_sync_remote.join();
    thread_sync_local.join();
    thread_user_interface.join();
}

// ========================================= //
// ================= THREADS ============== //
// ========================================= //

void Client::sync_local() {
    comm_manager.watch();
}

void Client::sync_remote() {
    comm_manager.handle_server_update();
}

void Client::user_interface() {
    while (true) {
        std::string input;
        cout << "> ";
        getline(cin, input);
        
        std::vector<string> tokens = split_command(input);
        if (!tokens.empty()) {
            process_command(tokens);
        }
    }
}

// ========================================= //
// ================= PRIVATE ============== //
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
        std::cout << "File uploaded via command" << std::endl;

    } else if (command == "download" && tokens.size() == 2) {
        FileManager::copy_file(sync_dir_path / file_path.filename(), std::filesystem::current_path() / file_path.filename());
        std::cout << "File downloaded via command" << std::endl;

    } else if (command == "delete" && tokens.size() == 2) {
        FileManager::delete_file(sync_dir_path / file_path.filename());
        std::cout << "File deleted via command" << std::endl;

    } else if (command == "list_server") {
        comm_manager.list_server();

    } else if (command == "list_client") {
        FileManager::get_formatted_file_list(sync_dir_path);

    } else if (command == "get_sync_dir") {
        std::cout << "Creating sync_dir and starting synchronization" << std::endl;

    } else if (command == "exit") {
        std::cout << "Closing session with server" << std::endl;
        comm_manager.exit_server();

    } else {
        std::cout << "Invalid command or missing arguments. Available commands:" << std::endl;
        std::cout << "# upload <path/filename.ext>" << std::endl;
        std::cout << "# download <filename.ext>" << std::endl;
        std::cout << "# delete <filename.ext>" << std::endl;
        std::cout << "# list_server" << std::endl;
        std::cout << "# list_client" << std::endl;
        std::cout << "# get_sync_dir" << std::endl;
        std::cout << "# exit" << std::endl;
    }
}
