#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctime>
#include <fstream>

#define BUFFER_SIZE 1024
#define SERVER_FILES_DIRECTORY "server_files"

class Server {
public:
    Server(int port);
    ~Server();
    void start();

private:
    int createSocket();
    void bindSocket();
    void listenForConnections();
    void handleClient(int clientSocket);
    void sendFile(int clientSocket, const char* filename);
    void saveFile(const char* filename, const char* file_data, size_t file_size);
    void listFiles(int clientSocket);
    std::string getFormattedTime(const char* filename);

    int serverSocket;
    int port;
};

Server::Server(int port) : port(port) {
    serverSocket = createSocket();
    bindSocket();
    listenForConnections();
}

Server::~Server() {
    close(serverSocket);
}

void Server::start() {
    sockaddr_in clientAddr{};
    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
    if (clientSocket == -1) {
        perror("Accept failed");
        return;
    }

    std::cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port)
              << std::endl;

    handleClient(clientSocket);

    close(clientSocket);
}

int Server::createSocket() {
    int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketDescriptor == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    return socketDescriptor;
}

void Server::bindSocket() {
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
}

void Server::listenForConnections() {
    if (listen(serverSocket, SOMAXCONN) == -1) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << port << std::endl;
}

void Server::handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    while (true) {
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cout << "Client disconnected." << std::endl;
            break;
        }

        std::cout << "Received from client: " << buffer << std::endl;

        std::string command(buffer);
        size_t spacePos = command.find(' ');
        std::string action = command.substr(0, spacePos);
        std::string argument = command.substr(spacePos + 1);

        if (action == "GET") {
            sendFile(clientSocket, argument.c_str());
        } else if (action == "LIST") {
            listFiles(clientSocket);
        } else if (action == "PUT") {
            recv(clientSocket, buffer, sizeof(buffer), 0);
            long file_size = std::stol(buffer);

            char* file_buffer = new char[file_size];
            recv(clientSocket, file_buffer, file_size, 0);

            saveFile(argument.c_str(), file_buffer, file_size);

            const char* success_message = "File uploaded successfully.";
            send(clientSocket, success_message, strlen(success_message), 0);

            delete[] file_buffer;
        } else if (action == "DELETE") {
            std::string fullFilePath = SERVER_FILES_DIRECTORY;
            fullFilePath += "/";
            fullFilePath += argument;

            if (remove(fullFilePath.c_str()) == 0) {
                const char* success_message = "File deleted successfully.";
                send(clientSocket, success_message, strlen(success_message), 0);
            } else {
                const char* error_message = "Error deleting file.";
                send(clientSocket, error_message, strlen(error_message), 0);
            }
        } else if (action == "INFO") {
            std::string fullFilePath = SERVER_FILES_DIRECTORY;
            fullFilePath += "/";
            fullFilePath += argument;

            struct stat file_stat;
            if (stat(fullFilePath.c_str(), &file_stat) == 0) {
                std::string info_message = "File size: " + std::to_string(file_stat.st_size) +
                                           "\nLast modified: " + getFormattedTime(fullFilePath.c_str());
                send(clientSocket, info_message.c_str(), info_message.length(), 0);
            } else {
                const char* error_message = "Error retrieving file information.";
                send(clientSocket, error_message, strlen(error_message), 0);
            }
        } else if (action == "EXIT") {
            std::cout << "Client requested to exit. Disconnecting..." << std::endl;
            break;
        } else {
            const char* error_message = "Unknown command.";
            send(clientSocket, error_message, strlen(error_message), 0);
        }

        memset(buffer, 0, BUFFER_SIZE);
    }
}

void Server::sendFile(int clientSocket, const char* filename) {
    std::string serverFilePath = SERVER_FILES_DIRECTORY;
    serverFilePath += "/";
    serverFilePath += filename;

    FILE* file = fopen(serverFilePath.c_str(), "rb");
    if (!file) {
        const char* error_message = "File not found.";
        send(clientSocket, error_message, strlen(error_message), 0);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char* file_buffer = new char[file_size];
    fread(file_buffer, 1, file_size, file);

    send(clientSocket, file_buffer, file_size, 0);

    fclose(file);
    delete[] file_buffer;
}

void Server::saveFile(const char* filename, const char* file_data, size_t file_size) {
    std::string serverFilePath = SERVER_FILES_DIRECTORY;
    serverFilePath += "/";
    serverFilePath += filename;

    std::ofstream outfile(serverFilePath, std::ios::binary);
    if (outfile.is_open()) {
        outfile.write(file_data, file_size);
        outfile.close();
    } else {
        std::cerr << "Error creating file on server: " << serverFilePath << std::endl;
    }
}

void Server::listFiles(int clientSocket) {
    DIR* dir;
    struct dirent* ent;

    if ((dir = opendir(SERVER_FILES_DIRECTORY)) != nullptr) {
        std::string file_list;
        while ((ent = readdir(dir)) != nullptr) {
            if (ent->d_name[0] != '.') {
                file_list += ent->d_name;
                file_list += "\n";
            }
        }

        closedir(dir);

        send(clientSocket, file_list.c_str(), file_list.length(), 0);
    } else {
        const char* error_message = "Unable to open directory.";
        send(clientSocket, error_message, strlen(error_message), 0);
    }
}

std::string Server::getFormattedTime(const char* filename) {
    struct stat file_stat;
    if (stat(filename, &file_stat) == 0) {
        struct tm* timeinfo = localtime(&file_stat.st_mtime);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return std::string(buffer);
    }
    return "Error retrieving file information.";
}

int main() {
    int port = 12341;
    Server server(port);
    server.start();

    return 0;
}
