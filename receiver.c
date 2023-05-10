#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define WINDOW_SIZE 4
#define PACKET_SIZE 100

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        printf("socket creation failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    iResult = bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    char packets[WINDOW_SIZE][PACKET_SIZE];
    int received[WINDOW_SIZE] = { 0 };
    int base = 0;

    while (1) {
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        char recvPacket[PACKET_SIZE];
        iResult = recvfrom(sock, recvPacket, PACKET_SIZE, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (iResult == SOCKET_ERROR) {
            printf("recvfrom failed: %d\n", WSAGetLastError());
            break;
        }
        int seqNum = atoi(recvPacket + 7);
        printf("Received packet %d\n", seqNum);

        if (seqNum >= base && seqNum < base + WINDOW_SIZE) {
            int index = seqNum % WINDOW_SIZE;
            if (received[index] == 0) {
                memcpy(packets[index], recvPacket, PACKET_SIZE);
                received[index] = 1;

                // send acknowledgement
                char ackPacket[PACKET_SIZE];
                sprintf(ackPacket, "ACK %d", seqNum);
                printf("Sending ACK for packet %d\n", seqNum);
                sendto(sock, ackPacket, PACKET_SIZE, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
            }
        }

        // update base
        while (received[base % WINDOW_SIZE] == 1) {
            printf("Delivering packet %d\n", base);
            printf("%s\n", packets[base % WINDOW_SIZE]);
            received[base % WINDOW_SIZE] = 0;
            base++;
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

