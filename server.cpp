#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <map>

constexpr int PORT = 50000;
std::vector<int> clients;
std::mutex clientsMutex;
std::map<int, std::string> nicknames;

void broadcast(const std::string& message, int senderSocket) {
    std::lock_guard<std::mutex> lock(clientsMutex);

    for (int client : clients) {
        if (client != senderSocket) {
            send(client, message.c_str(), message.size(), 0);
        }
    }
}

int getClientByNick(const std::string& nick) {
    for (const auto& pair : nicknames) {
        if (pair.second == nick) {
            return pair.first;
        }
    }
    return -1;
}

void handleClient(int clientSocket) {
    char buffer[1024];

    memset(buffer, 0, sizeof(buffer));
    int bytes = recv(clientSocket, buffer, 1024, 0);
    if (bytes <= 0) {
        close(clientSocket);
        return;
    }
    std::string nick(buffer, bytes);
    nick.erase(std::remove(nick.begin(), nick.end(), '\n'), nick.end());
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        nicknames[clientSocket] = nick;
    }

    std::string joinMsg = "[" + nick + "] joined the chat";
    broadcast(joinMsg, clientSocket);
    std::cout << joinMsg << std::endl;
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(clientSocket, buffer, 1024, 0);

        if (bytes <= 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        }

        std::string input(buffer, bytes);
        input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());

        if (input.rfind("/nick ", 0) == 0) {
            std::string newNick = input.substr(6);

            {
            std::lock_guard<std::mutex> lock(clientsMutex);
            nicknames[clientSocket] = newNick;
            }

            std::string info = nick + " changed nick to " + newNick;
            nick = newNick;

            broadcast("[SERVER]: " + info, clientSocket);
            std::cout << info << std::endl;
            continue;
        }

        if (input.rfind("/msg ", 0) == 0) {
            std::string rest = input.substr(5);

            size_t space = rest.find(' ');
            if (space == std::string::npos) continue;

            std::string targetNick = rest.substr(0, space);
            std::string msg = rest.substr(space + 1);

            int targetSocket;

            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                targetSocket = getClientByNick(targetNick);
            }

            if (targetSocket == -1) {
                std::string error = "[SERVER]: User not found";
                send(clientSocket, error.c_str(), error.size(), 0);
                continue;
            }

            std::string fullMsg = "[PM from " + nick + "]: " + msg;
            send(targetSocket, fullMsg.c_str(), fullMsg.size(), 0);

            std::string selfMsg = "[PM to " + targetNick + "]: " + msg;
            send(clientSocket, selfMsg.c_str(), selfMsg.size(), 0);

            continue;
        }

        if (input == "/list") {
            std::lock_guard<std::mutex> lock(clientsMutex);

            std::string list = "[SERVER]: Users online: ";
            for (auto it = nicknames.begin(); it != nicknames.end(); ++it) {
                list += it->second;

                if (std::next(it) != nicknames.end()) {
                    list += ", ";
                }
            }

            send(clientSocket, list.c_str(), list.size(), 0);
            continue;
        }

        std::string msg = "[" + nick + "]: "+ input;
        broadcast(msg, clientSocket);
        std::cout << msg << std::endl;
    }

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
        nicknames.erase(clientSocket);
    }

    std::string leaveMsg = "[" + nick + "] left the chat";
    broadcast(leaveMsg, clientSocket);
    std::cout << leaveMsg << std::endl;
    
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
