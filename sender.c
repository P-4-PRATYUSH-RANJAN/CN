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
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    char packets[WINDOW_SIZE][PACKET_SIZE];
    int ack[WINDOW_SIZE] = { 0 };
    int base = 0, nextSeqNum = 0;

    while (1) {
        // send packets in the window
        while (nextSeqNum < base + WINDOW_SIZE) {
            int index = nextSeqNum % WINDOW_SIZE;
            memset(packets[index], 0, PACKET_SIZE);
            sprintf(packets[index], "Packet %d", nextSeqNum);
            printf("Sending packet %d\n", nextSeqNum);
            sendto(sock, packets[index], PACKET_SIZE, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
            nextSeqNum++;
        }

        // receive acks
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        char ackPacket[PACKET_SIZE];
        iResult = recvfrom(sock, ackPacket, PACKET_SIZE, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (iResult == SOCKET_ERROR) {
            printf("recvfrom failed: %d\n", WSAGetLastError());
            break;
        }
        int ackNum = atoi(ackPacket);
        printf("Received ACK for packet %d\n", ackNum);

        // update acks and base
        int index = ackNum % WINDOW_SIZE;
        if (ack[index] == 0) {
            ack[index] = 1;
            while (ack[base % WINDOW_SIZE] == 1) {
                ack[base % WINDOW_SIZE] = 0;
                base++;
            }
        }

        if (base == nextSeqNum) {
            printf("All packets have been acknowledged.\n");
            break;
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
//credit gpt
