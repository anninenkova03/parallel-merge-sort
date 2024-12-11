#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

void merge(std::vector<int>& arr, int start, int mid, int end) {
    std::vector<int> left(arr.begin() + start, arr.begin() + mid + 1);
    std::vector<int> right(arr.begin() + mid + 1, arr.begin() + end + 1);
    int i = 0, j = 0, k = start;

    while (i < left.size() && j < right.size()) {
        arr[k++] = (left[i] <= right[j]) ? left[i++] : right[j++];
    }
    while (i < left.size()) arr[k++] = left[i++];
    while (j < right.size()) arr[k++] = right[j++];
}

void mergeSortSingleThread(std::vector<int>& arr, int start, int end) {
    if (start >= end) return;

    int mid = start + (end - start) / 2;
    mergeSortSingleThread(arr, start, mid);
    mergeSortSingleThread(arr, mid + 1, end);
    merge(arr, start, mid, end);
}

void mergeSortMultiThread(std::vector<int>& arr, int start, int end, int threadCount = 0) {
    if (start >= end) return;

    int mid = start + (end - start) / 2;

    if (threadCount < std::thread::hardware_concurrency()) {
        std::thread leftThread(mergeSortMultiThread, std::ref(arr), start, mid, threadCount + 1);
        std::thread rightThread(mergeSortMultiThread, std::ref(arr), mid + 1, end, threadCount + 1);
        leftThread.join();
        rightThread.join();
    } else {
        mergeSortSingleThread(arr, start, mid);
        mergeSortSingleThread(arr, mid + 1, end);
    }

    merge(arr, start, mid, end);
}

void handleClient(SOCKET clientSocket) {
    int size;
    recv(clientSocket, (char*)&size, sizeof(size), 0);
    std::vector<int> data(size);
    recv(clientSocket, (char*)data.data(), size * sizeof(int), 0);

    std::vector<int> dataSingleThread = data; 

    auto startSingleThread = std::chrono::high_resolution_clock::now();
    mergeSortSingleThread(data, 0, size - 1);
    auto endSingleThread = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationSingleThread = endSingleThread - startSingleThread;
    double singleThreadTime = durationSingleThread.count() * 1000;
    send(clientSocket, (char*)&singleThreadTime, sizeof(double), 0);

    auto startMultiThread = std::chrono::high_resolution_clock::now();
    mergeSortMultiThread(data, 0, size - 1);
    auto endMultiThread = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationMultiThread = endMultiThread - startMultiThread;
    double multiThreadTime = durationMultiThread.count() * 1000;
    send(clientSocket, (char*)&multiThreadTime, sizeof(double), 0);

    send(clientSocket, (char*)data.data(), size * sizeof(int), 0);
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is running on port 8080...\n";

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;
        }
        std::thread(handleClient, clientSocket).detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
