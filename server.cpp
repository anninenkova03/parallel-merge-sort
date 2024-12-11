#include <iostream>
#include <vector>
#include <thread>
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

void mergeSort(std::vector<int>& arr, int start, int end) {
    if (start >= end) return;

    int mid = start + (end - start) / 2;
    std::thread leftThread(mergeSort, std::ref(arr), start, mid);
    std::thread rightThread(mergeSort, std::ref(arr), mid + 1, end);

    leftThread.join();
    rightThread.join();
    merge(arr, start, mid, end);
}

void handleClient(SOCKET clientSocket) {
    int size;
    recv(clientSocket, (char*)&size, sizeof(size), 0);
    std::vector<int> data(size);
    recv(clientSocket, (char*)data.data(), size * sizeof(int), 0);

    mergeSort(data, 0, size - 1);

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
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    std::cout << "Server is running on port 8080...\n";

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        std::thread(handleClient, clientSocket).detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
