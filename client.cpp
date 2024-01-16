#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main() {

    int port = 12343;
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


        char buffer[BUFFER_SIZE];
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << "Received from server: " << buffer << std::endl;
        }
    }


    close(clientSocket);

    return 0;
}
