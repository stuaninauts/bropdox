#include <Client.hpp>
#include <Packet.hpp>
#include <filesystem>

using namespace std;

// ======================================== //
// ================ PUBLIC ================ //
// ======================================== //

void Client::run() {
    cout << "Connecting to server..." << endl;
    if (!comm_manager.connect_to_server(server_ip, port, username)) {
        cerr << "Error connecting to server" << endl;
        exit(1);
    }
    // Remove o sync_dir antigo
    file_manager.remove_sync_dir();
    // Cria novo sync_dir zerado
    file_manager.create_sync_dir();
    comm_manager.get_sync_dir();
    std::thread thread_sync_remote(&Client::sync_remote, this);
    std::thread thread_sync_local(&Client::sync_local, this);
    std::thread thread_user_interface(&Client::user_interface, this);

    thread_sync_remote.join();
    thread_sync_local.join();
    thread_user_interface.join();
}

void Client::sync_local() {
    file_manager.watch();
}

void Client::sync_remote() {
    while(true) {
        comm_manager.fetch();
    }
}

void Client::user_interface() {
    while (true) {
        string input;
        cout << "> ";
        getline(cin, input);
        
        vector<string> tokens = split_command(input);
        if (!tokens.empty()) {
            process_command(tokens);
        }
    }
}

// ========================================= //
// ================ PRIVATE ================ //
// ========================================= //

vector<string> Client::split_command(const string &command) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(command);
    
    while (tokenStream >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

void Client::process_command(const vector<string> &tokens) {
    if (tokens.empty()) return;

    string command = tokens[0];
    transform(command.begin(), command.end(), command.begin(), ::tolower);

    if (command == "upload" && tokens.size() == 2) {
        if (file_manager.upload_local_file(tokens[1])) {
            comm_manager.upload_file(tokens[1]);
        }
    } else if (command == "download" && tokens.size() == 2) {
        if (file_manager.download_local_file(tokens[1])) {
            comm_manager.download_file(tokens[1]);
        }
    } else if (command == "delete" && tokens.size() == 2) {
        if (file_manager.delete_local_file(tokens[1])) {
            comm_manager.delete_file(tokens[1]);
    } else if (command == "list_server") {
        comm_manager.list_server();

    } else if (command == "list_client") {
        file_manager.list_files();

    } else if (command == "get_sync_dir") {
        std::cout << "Creating sync_dir and starting synchronization" << std::endl;
        // Pode chamar alguma função de criação aqui se necessário

    } else if (command == "exit") {
        std::cout << "Closing session with server" << std::endl;
        comm_manager.exit_server();

    } else {
        std::cerr << "Invalid command or missing arguments. Available commands:" << std::endl;
        std::cerr << "# upload <path/filename.ext>" << std::endl;
        std::cerr << "# download <filename.ext>" << std::endl;
        std::cerr << "# delete <filename.ext>" << std::endl;
        std::cerr << "# list_server" << std::endl;
        std::cerr << "# list_client" << std::endl;
        std::cerr << "# get_sync_dir" << std::endl;
        std::cerr << "# exit" << std::endl;
    }
}
