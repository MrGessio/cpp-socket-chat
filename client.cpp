#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <algorithm>

constexpr int PORT = 50000;

void receiveMessages(int clientSocket, std::string* myNick) {
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(clientSocket, buffer, 1024, 0);

        if (bytes <= 0) {
            std::cout << "Disconnected from server\n";
            break;
        }

        std::string message(buffer);
        std::string prefix = "[" + *myNick + "]:";

        if (message.rfind(prefix, 0) == 0) {
            std::cout << "[me]: " << message.substr(prefix.size()) << std::endl;
        } 
        else {
            std::cout << message << std::endl;
        }
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
    nick.erase(std::remove(nick.begin(), nick.end(), '\n'), nick.end());

    send(clientSocket, nick.c_str(), nick.size(), 0);

    std::thread receiver(receiveMessages, clientSocket, &nick);
    std::string message;

    while(true) {
        std::cout << "me: ";
        std::getline(std::cin, message);
        message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());

        if(message == "exit") {
            break;
        }

        if (message.rfind("/nick ", 0) == 0) {
        nick = message.substr(6);
        }

        send(clientSocket, message.c_str(), message.size(), 0);

        if (message.rfind("/", 0) != 0) {
        std::cout << "[me]: " << message << std::endl;
        }
    }

    close(clientSocket);

    return 0;
}
