#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fstream>

#define BUFFER_SIZE 1024
#define CLIENT_FILES_DIRECTORY "client_files"

void saveFile(const char* filename, const char* file_data, size_t file_size) {
    std::string clientFilePath = std::string(CLIENT_FILES_DIRECTORY) + "/" + filename;

    std::ofstream outfile(clientFilePath, std::ios::binary);
    if (outfile.is_open()) {
        outfile.write(file_data, file_size);
        outfile.close();
    } else {
        std::cerr << "Error creating file: " << clientFilePath << std::endl;
    }
}

int main() {
    int port = 12342;
    const char* serverIp = "127.0.0.1";

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error creating socket");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIp, &(serverAddr.sin_addr));

    if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        perror("Connect failed");
        close(clientSocket);
        return 1;
    }

    while (true) {
        std::cout << "Enter command (e.g., GET <filename>, LIST, PUT <filename>, DELETE <filename>, INFO <filename>, EXIT): ";
        std::string userInput;
        std::getline(std::cin, userInput);

        const char* message = userInput.c_str();
        send(clientSocket, message, strlen(message), 0);

        if (userInput == "EXIT") {
            break;
        }


        if (userInput.substr(0, 3) == "PUT") {

            std::string filename = userInput.substr(4);


            std::ifstream file(std::string(CLIENT_FILES_DIRECTORY) + "/" + filename, std::ios::binary | std::ios::ate);
            if (file.is_open()) {
                std::streamsize file_size = file.tellg();
                file.seekg(0, std::ios::beg);


                std::string size_str = std::to_string(file_size);
                send(clientSocket, size_str.c_str(), size_str.size(), 0);


                char* file_buffer = new char[file_size];
                file.read(file_buffer, file_size);
                send(clientSocket, file_buffer, file_size, 0);

                delete[] file_buffer;

                file.close();


                char response_buffer[BUFFER_SIZE];
                memset(response_buffer, 0, BUFFER_SIZE);
                ssize_t responseBytesReceived = recv(clientSocket, response_buffer, sizeof(response_buffer) - 1, 0);

                if (responseBytesReceived > 0) {
                    std::cout << "Received from server: " << response_buffer << std::endl;
                }
            } else {
                std::cerr << "Error opening file: " << filename << std::endl;
            }
        } else {
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

            if (bytesReceived > 0) {
                if (userInput.substr(0, 3) == "GET") {
                    saveFile(userInput.substr(4).c_str(), buffer, bytesReceived);
                    std::cout << "Received and saved file: " << userInput.substr(4) << std::endl;
                } else {
                    std::cout << "Received from server: " << buffer << std::endl;
                }
            }
        }
    }

    close(clientSocket);

    return 0;
}
