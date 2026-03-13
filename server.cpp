#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

constexpr int PORT = 50000;

int main() {

    
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket == -1){
        std::cout << "Socket created: failed\n";
        return 1;
    }

    std::cout << "Socket created: succcess \n";

    
    sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    
    if(bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) < 0){
        std::cout << "Bind error\n";
        return 1;
    }

    std::cout << "Bind OK\n";

    listen(serverSocket, 10);

    std::cout << "Server is listening...\n";

    
    int clientSocket = accept(serverSocket, NULL, NULL);

    if(clientSocket < 0){
        std::cout << "Accept failed\n";
        return 1;
    }

    std::cout << "Cilent is connected!\n";

    char buffer[1024];

    while(true){

        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(clientSocket, buffer, 1024, 0);

        if(bytes <= 0){
            break;
        }

        std::cout << "Message: " << buffer << std::endl;
    }

    close(clientSocket);
    close(serverSocket);

}