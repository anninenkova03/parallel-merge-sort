#include <iostream>
#include <vector>
#include <cstring>
#include <winsock2.h> 
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed\n";
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed\n";
        return 1;
    }

    std::vector<int> data = {23, 34, 27, 10, 2, 3, 47, 85, 37, 27};
    int size = data.size();

    send(clientSocket, (char*)&size, sizeof(size), 0);
    send(clientSocket, (char*)data.data(), size * sizeof(int), 0);

    double singleThreadTime;
    recv(clientSocket, (char*)&singleThreadTime, sizeof(double), 0);

    double multiThreadTime;
    recv(clientSocket, (char*)&multiThreadTime, sizeof(double), 0);

    recv(clientSocket, (char*)data.data(), size * sizeof(int), 0);

    std::cout << "Sorted data: ";
    for (int num : data) std::cout << num << " ";
    std::cout << "\n";
    std::cout << "Single thread sort time: " << singleThreadTime << " ms\n";
    std::cout << "Multi thread sort time: " << multiThreadTime << " ms\n";

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
