#include "Client.hpp"
#include "FileManager.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

Client::Client(string username, string server_ip, int port)
    : username(username), server_ip(server_ip), port(port), fileManager(username) {} 

void start_watcher(FileManager& fileManager) {
    try {
        string path = "client/sync_dir_" + fileManager.username;
        Watcher watcher(path, fileManager);
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Erro na thread: " << e.what() << std::endl;
    }
}


void Client::run() {
    connect_to_server(this->server_ip, this->port);    
    std::thread(start_watcher, std::ref(this->fileManager)).detach();

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

void Client::connect_to_server(string server_ip, int porta){
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    server = gethostbyname(server_ip.c_str());

	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("ERROR opening socket\n");
        exit(0);
    }   

	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(porta);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	
    bzero(&(serv_addr.sin_zero), 8);     
	
    
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        printf("ERROR connecting\n");
    else {
        // Cria o sync_dir no client
        this->fileManager.create_sync_dir();
        cout << "Conexão com servidor estabelecida" << endl;
    };
    
    printf("Enter the message: ");
    bzero(buffer, 256);
    fgets(buffer, 256, stdin);
    
	/* write in the socket */
	n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
		printf("ERROR writing to socket\n");

    bzero(buffer,256);
	
	/* read from the socket */
    n = read(sockfd, buffer, 256);
    if (n < 0) 
		printf("ERROR reading from socket\n");

    printf("%s\n",buffer);
    
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
        this->fileManager.list_files_in_directory("client/sync_dir_" + this->username);
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