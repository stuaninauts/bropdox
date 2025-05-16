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

    // TODO: checagem de erro TYPE ERROR
    // funcao para cada bloco do if (handle_upload)
    if (command == "upload" && tokens.size() == 2)
        comm_manager.upload_file(tokens[1]);
    else if (command == "download" && tokens.size() == 2)
        comm_manager.download_file(tokens[1]);
    else if (command == "delete" && tokens.size() == 2)
        comm_manager.delete_file(tokens[1]);
    else if (command == "list_server")
        comm_manager.list_server();
    else if (command == "list_client")
        file_manager.list_files();
    else if (command == "get_sync_dir")
        std::cout << "Creating sync_dir and starting synchronization" << std::endl;
    else if (command == "exit") {
        std::cout << "Closing session with server" << std::endl;
        comm_manager.exit_server();
    }
    else {
        cout << "Invalid command or missing arguments. Available commands:" << endl;
        cout << "# upload <path/filename.ext>" << endl;
        cout << "# download <filename.ext>" << endl;
        cout << "# delete <filename.ext>" << endl;
        cout << "# list_server" << endl;
        cout << "# list_client" << endl;
        cout << "# get_sync_dir" << endl;
        cout << "# exit" << endl;
    }
}
