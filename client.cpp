#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>

constexpr int PORT = 50000;

void receiveMessages(int clientSocket) {
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(clientSocket, buffer, 1024, 0);

        if (bytes <= 0) {
            std::cout << "Disconnected from server\n";
            break;
        }
        std::cout << buffer << std::endl;
    }
}

int main() {

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket == -1) {
        std::cout << "Socket created: failed\n";
        return 1;
    }
    
    std::cout << "Socket created: succcess \n";

    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr. sin_addr);

    if(connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cout << "Connection failed\n";
        return 1;
    }

    std::cout << "Connected to server\n";

    std::cout << "Enter your nickname: ";
    std::string nick;
    std::getline(std::cin, nick);

    send(clientSocket, nick.c_str(), nick.size(), 0);

    std::thread receiver(receiveMessages, clientSocket);
    std::string message;

    while(true) {

        std::getline(std::cin, message);

        if(message == "exit") {
            break;
        }

        send(clientSocket, message.c_str(), message.size(), 0);
    }

    close(clientSocket);

    return 0;
}
