#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080
#define PACKET_SIZE 1024
#define TIMEOUT 5000

typedef struct {
    int seq_num;
    char data[PACKET_SIZE];
} Packet;

int main(int argc, char *argv[]) {
    WSADATA wsa;
    SOCKET socket_fd;
    struct sockaddr_in server_addr;
    int server_addr_len = sizeof(server_addr);
    int send_len, recv_len;
    int seq_num = 0;
    char data[PACKET_SIZE];
    Packet packet;
    int timeout = TIMEOUT;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Failed to initialize Winsock\n");
        return 1;
    }

    // Create socket
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Failed to create socket\n");
        return 1;
    }

    // Set socket send timeout
    setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Start sending packets
    while (1) {
        // Read data from input
        printf("Enter data to send: ");
        fgets(data, PACKET_SIZE, stdin);
        data[strlen(data) - 1] = '\0';

        // Prepare packet
        memset(&packet, 0, sizeof(packet));
        packet.seq_num = seq_num;
        strcpy(packet.data, data);

        // Send packet
        send_len = sendto(socket_fd, (char *)&packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, server_addr_len);
        if (send_len == SOCKET_ERROR) {
            printf("Failed to send packet\n");
            continue;
        }

        // Wait for ACK packet
        memset(&packet, 0, sizeof(packet));
        recv_len = recvfrom(socket_fd, (char *)&packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (recv_len == SOCKET_ERROR) {
            printf("Failed to receive ACK packet\n");
            continue;
        }

        // Check ACK packet sequence number
        if (packet.seq_num != seq_num) {
            printf("Received ACK packet with wrong sequence number\n");
            continue;
        }

        // Print success message
        printf("Successfully sent packet with sequence number %d\n", seq_num);

        // Update sequence number
        seq_num = (seq_num + 1) % 2;
    }

    closesocket(socket_fd);
    WSACleanup();

    return 0;
}

