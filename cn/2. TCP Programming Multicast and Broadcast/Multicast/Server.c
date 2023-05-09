#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <winsock2.h>

#ifndef IP_MULTICAST_TTL
#define IP_MULTICAST_TTL 10
#endif


#pragma comment(lib, "ws2_32.lib")

#define MULTICAST_GROUP "239.0.0.1"
#define MULTICAST_PORT 12345
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET serverSocket;
    SOCKADDR_IN serverAddr, multicastAddr;
    char buffer[BUFFER_SIZE];
    int bytesSent, bytesRecv, multicastTtl = 1;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    // Create server socket
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        printf("socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Set socket options
    setsockopt(serverSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&multicastTtl, sizeof(multicastTtl));

    // Bind server socket to any available IP address and port
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(MULTICAST_PORT);
    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Set up multicast address
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    multicastAddr.sin_port = htons(MULTICAST_PORT);

    while (1) {
        // Receive data from client
        bytesRecv = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (bytesRecv == SOCKET_ERROR) {
            printf("recvfrom failed: %d\n", WSAGetLastError());
            break;
        }

        // Send data to multicast group
        bytesSent = sendto(serverSocket, buffer, bytesRecv, 0, (SOCKADDR*)&multicastAddr, sizeof(multicastAddr));
        if (bytesSent == SOCKET_ERROR) {
            printf("sendto failed: %d\n", WSAGetLastError());
            break;
        }
    }

    // Clean up
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}

