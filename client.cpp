#include <iostream>
#include <mutex>
#include <vector>
#include <cstring>
#include <thread>
#include <winsock2.h> 
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

std::mutex outputMutex;

void clientTask(int clientId, const std::vector<int>& data) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed\n";
        return;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cerr << "Client " << clientId << ": Socket creation failed\n";
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cerr << "Client " << clientId << ": Connection failed\n";
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    int size = data.size();
    send(clientSocket, (char*)&size, sizeof(size), 0);
    send(clientSocket, (char*)data.data(), size * sizeof(int), 0);

    double singleThreadTime, multiThreadTime;
    recv(clientSocket, (char*)&singleThreadTime, sizeof(double), 0);
    recv(clientSocket, (char*)&multiThreadTime, sizeof(double), 0);

    std::vector<int> sortedData(size);
    recv(clientSocket, (char*)sortedData.data(), size * sizeof(int), 0);

    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << "Client " << clientId << " - Sorted data: ";
    for (int num : sortedData) std::cout << num << " ";
    std::cout << "\n";
    std::cout << "Client " << clientId << " - Single thread sort time: " << singleThreadTime << " ms\n";
    std::cout << "Client " << clientId << " - Multi thread sort time: " << multiThreadTime << " ms\n";
    
    closesocket(clientSocket);
    WSACleanup();
}

int main() {
    std::vector<int> data1 = {23, 34, 27, 10, 2, 3, 47, 85, 37, 27};
    std::vector<int> data2 = {99, -88, -77, 66, 55};
    std::vector<int> data3 = {};
    std::vector<int> data4 = {1};

    std::thread client1(clientTask, 1, data1);
    std::thread client2(clientTask, 2, data2);
    std::thread client3(clientTask, 3, data3);
    std::thread client4(clientTask, 4, data4);

    client1.join();
    client2.join();
    client3.join();
    client4.join();

    return 0;
} 