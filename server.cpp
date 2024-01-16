#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctime>

#define BUFFER_SIZE 1024

void sendFile(int clientSocket, const char* filename) {
    FILE* file = fopen(filename, "rb");
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

void listFiles(int clientSocket, const char* directoryPath) {
    DIR* dir;
    struct dirent* ent;

    if ((dir = opendir(directoryPath)) != nullptr) {
        std::string file_list;
        while ((ent = readdir(dir)) != nullptr) {
            file_list += ent->d_name;
            file_list += "\n";
        }

        closedir(dir);

        send(clientSocket, file_list.c_str(), file_list.length(), 0);
    } else {
        const char* error_message = "Unable to open directory.";
        send(clientSocket, error_message, strlen(error_message), 0);
    }
}


void handleClient(int clientSocket) {
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
            listFiles(clientSocket, argument.c_str());
        } else if (action == "PUT") {

            const char* success_message = "File uploaded successfully.";
            send(clientSocket, success_message, strlen(success_message), 0);
        } else if (action == "DELETE") {

            if (remove(argument.c_str()) == 0) {
                const char* success_message = "File deleted successfully.";
                send(clientSocket, success_message, strlen(success_message), 0);
            } else {
                const char* error_message = "Error deleting file.";
                send(clientSocket, error_message, strlen(error_message), 0);
            }
        } else if (action == "INFO") {

            struct stat file_stat;
            if (stat(argument.c_str(), &file_stat) == 0) {
                std::string info_message = "File size: " + std::to_string(file_stat.st_size) ;

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

int main() {

    int port = 12343;


    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        return 1;
    }


    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        perror("Bind failed");
        close(serverSocket);
        return 1;
    }


    if (listen(serverSocket, SOMAXCONN) == -1) {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }

    std::cout << "Server listening on port " << port << std::endl;


    sockaddr_in clientAddr{};
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
    if (clientSocket == -1) {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    std::cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port)
              << std::endl;


    handleClient(clientSocket);


    close(clientSocket);
    close(serverSocket);

    return 0;
}
