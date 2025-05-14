#include <Client.hpp>
#include <Packet.hpp>

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

    file_manager.create_sync_dir();

    std::thread thread_sync_local(&Client::sync_local, this);
    std::thread thread_user_interface(&Client::user_interface, this);

    thread_sync_local.join();
    thread_user_interface.join();
}

void Client::sync_local() {
    file_manager.watch();
}

void Client::sync_remote() {
    // Changes changes;
    // Changes {
    //  std::string type;
    //  std::string filename;
    // }
    while(true) {
        // changes = commManager.pull();
        // if(changes != nullptr) {
        //     fileManager.update(changes)
        // }
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
        string filepath = tokens[1];
        cout << "Uploading file: " << filepath << " to server's sync_dir" << endl;
        comm_manager.send_command("upload");
        Packet::send_file(comm_manager.socket_upload, filepath);
    }
    else if (command == "download" && tokens.size() == 2) {
        string filename = tokens[1];
        cout << "Downloading file: " << filename << " from server to local directory" << endl;
        Packet::receive_file(comm_manager.socket_download, filename);
    }
    else if (command == "delete" && tokens.size() == 2) {
        string filename = tokens[1];
        cout << "Deleting file: " << filename << " from sync_dir" << endl;
        // Implement delete functionality here
    }
    else if (command == "list_server") {
        cout << "Listing files on server:" << endl;
        comm_manager.send_command("list_server");
        comm_manager.receive_packet();
    }
    else if (command == "list_client") {
        file_manager.list_files();
    }
    else if (command == "get_sync_dir") {
        cout << "Creating sync_dir and starting synchronization" << endl;
        // Implement sync dir creation here
    }
    else if (command == "exit") {
        cout << "Closing session with server" << endl;
        // Implement exit functionality here
        exit(0);
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
