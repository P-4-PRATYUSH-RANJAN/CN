#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 5000
#define GROUP "239.255.255.250"

int main() {
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server, client;
    int slen = sizeof(client), recv_len;
    char buf[1024];
    struct ip_mreq mreq;

    //Initialise winsock
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
    printf("Initialised.\n");

    //Create a socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
        printf("Could not create socket : %d" , WSAGetLastError());
    }

    //Allow multiple sockets to use the same PORT number
    int optval = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(int)) < 0) {
        perror("setsockopt");
        return -1;
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    //Bind socket to port
    if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    //Set up multicast group
    mreq.imr_multiaddr.s_addr = inet_addr(GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        return -1;
    }

    //Listen for incoming messages
    while (1) {
        memset(buf, 0, sizeof(buf));
        if ((recv_len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&client, &slen)) == SOCKET_ERROR) {
            printf("recvfrom() failed with error code : %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        printf("Received packet from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        printf("Data: %s\n", buf);
    }

    //Close the socket
    closesocket(s);
    WSACleanup();
    return 0;
}

