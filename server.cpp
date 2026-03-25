#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>

constexpr int PORT = 50000;
std::vector<int> clients;
std::mutex clientsMutex;

void broadcast(const char* message, int senderSocket) {
    std::lock_guard<std::mutex> lock(clientsMutex);

    for (int client : clients) {
        if (client != senderSocket) {
            send(client, message, strlen(message), 0);
        }
    }
}

void handleClient(int clientSocket) {
    char buffer[1024];
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(clientSocket, buffer, 1024, 0);

        if (bytes <= 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        }

        std::cout << "Message: " << buffer << std::endl;
        broadcast(buffer, clientSocket);
    }

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
    }

    close(clientSocket);
}

int main() {

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket == -1) {
        std::cout << "Socket created: failed\n";
        return 1;
    }

    std::cout << "Socket created: succcess \n";

    sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    
    if(bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cout << "Bind error\n";
        return 1;
    }

    std::cout << "Bind OK\n";

    listen(serverSocket, 10);

    std::cout << "Server is listening...\n";

    while(true) {
        int clientSocket = accept(serverSocket, NULL, NULL);

        if(clientSocket < 0) {
            std::cout << "Accept failed\n";
            continue;
        }

        std::cout << "Cilent is connected!\n";
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        std::thread t(handleClient, clientSocket);
        t.detach();
    }
    close(serverSocket);
}
