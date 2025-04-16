#include "Client.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

Client::Client(string username, string server_ip, int port)
    : username(username), server_ip(server_ip), port(port) {};


void Client::run() {
    while (true) {
        string input;
        cout << "> ";
        getline(cin, input);
        
        vector<string> tokens = splitCommand(input);
        if (!tokens.empty()) {
            processCommand(tokens);
        }
    }
}


vector<string> Client::splitCommand(const string &command) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(command);
    
    while (tokenStream >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

void Client::processCommand(const vector<string> &tokens) {
    if (tokens.empty()) return;
    
    string command = tokens[0];
    transform(command.begin(), command.end(), command.begin(), ::tolower);
    
    if (command == "upload" && tokens.size() == 2) {
        string filepath = tokens[1];
        cout << "Uploading file: " << filepath << " to server's sync_dir" << endl;
        // Implement upload functionality here
    }
    else if (command == "download" && tokens.size() == 2) {
        string filename = tokens[1];
        cout << "Downloading file: " << filename << " from server to local directory" << endl;
        // Implement download functionality here
    }
    else if (command == "delete" && tokens.size() == 2) {
        string filename = tokens[1];
        cout << "Deleting file: " << filename << " from sync_dir" << endl;
        // Implement delete functionality here
    }
    else if (command == "list_server") {
        cout << "Listing files on server:" << endl;
        // Implement server listing functionality here
    }
    else if (command == "list_client") {
        cout << "Listing files in sync_dir:" << endl;
        // Implement client listing functionality here
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