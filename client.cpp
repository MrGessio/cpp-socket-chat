#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

constexpr int PORT = 50000;

int main() {

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket == -1){
        std::cout << "Socket created: failed\n";
        return 1;
    }
    
    std::cout << "Socket created: succcess \n";

    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        std::cout << "Connection failed\n";
        return 1;
    }

    std::cout << "Connected to server\n";

    std::string message;

    while(true){

        std::getline(std::cin, message);

        if(message == "exit"){
            break;
        }

        send(clientSocket, message.c_str(), message.size(), 0);
    }

    close(clientSocket);

    return 0;
}