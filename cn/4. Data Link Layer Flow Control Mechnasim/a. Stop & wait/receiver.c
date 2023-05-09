#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080
#define PACKET_SIZE 1024

typedef struct {
    int seq_num;
    char data[PACKET_SIZE];
} Packet;

int main(int argc, char *argv[]) {
    WSADATA wsa;
    SOCKET socket_fd;
    struct sockaddr_in server_addr, client_addr;
    int server_addr_len = sizeof(server_addr);
    int client_addr_len = sizeof(client_addr);
    int recv_len, send_len;
    int seq_num = 0;
    char data[PACKET_SIZE];
    Packet packet;

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

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to server address
    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Failed to bind socket to address\n");
        return 1;
    }

    // Start receiving packets
    while (1) {
        // Receive packet
        memset(&packet, 0, sizeof(packet));
        recv_len = recvfrom(socket_fd, (char *)&packet, sizeof(packet), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len == SOCKET_ERROR) {
            printf("Failed to receive packet\n");
            continue;
        }

        // Check packet sequence number
        if (packet.seq_num != seq_num) {
            printf("Received packet with wrong sequence number\n");

            // Send ACK packet with previous sequence number
            packet.seq_num = (seq_num + 1) % 2;
            send_len = sendto(socket_fd, (char *)&packet, sizeof(packet), 0, (struct sockaddr *)&client_addr, client_addr_len);
            if (send_len == SOCKET_ERROR) {
                printf("Failed to send ACK packet\n");
                continue;
            }

            continue;
        }

        // Print received data
        printf("Received data: %s\n", packet.data);

        // Send ACK packet
        packet.seq_num = seq_num;
        send_len = sendto(socket_fd, (char *)&packet, sizeof(packet), 0, (struct sockaddr *)&client_addr, client_addr_len);
        if (send_len == SOCKET_ERROR) {
            printf("Failed to send ACK packet\n");
            continue;
        }

        // Update sequence number
        seq_num = (seq_num + 1) % 2;
    }

    closesocket(socket_fd);
    WSACleanup();

    return 0;
}

