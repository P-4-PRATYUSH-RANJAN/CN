#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080
#define WINDOW_SIZE 4
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
    int expected_seq_num = 0;
    int recv_len, send_len;
    Packet recv_packet, ack_packet;

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
    if (bind(socket_fd, (struct sockaddr *)&server_addr, server_addr_len) == SOCKET_ERROR) {
        printf("Failed to bind socket\n");
        return 1;
    }

    // Receive data
    while (1) {
        // Receive packet
        memset(&recv_packet, 0, sizeof(Packet));
        recv_len = recvfrom(socket_fd, (char *)&recv_packet, sizeof(Packet), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len == SOCKET_ERROR) {
            printf("Failed to receive packet\n");
            return 1;
        }

        // Check if packet is expected
        if (recv_packet.seq_num == expected_seq_num) {
            // Send ACK packet
            memset(&ack_packet, 0, sizeof(Packet));
            ack_packet.seq_num = expected_seq_num;
            send_len = sendto(socket_fd, (char *)&ack_packet, sizeof(Packet), 0, (struct sockaddr *)&client_addr, client_addr_len);
            if (send_len == SOCKET_ERROR) {
                printf("Failed to send ACK packet\n");
                return 1;
            }

            // Print data
            printf("%s\n", recv_packet.data);

            expected_seq_num++;
        }
        else {
            // Send ACK packet for previous packet
            memset(&ack_packet, 0, sizeof(Packet));
            ack_packet.seq_num = expected_seq_num - 1;
            send_len = sendto(socket_fd, (char *)&ack_packet, sizeof(Packet), 0, (struct sockaddr *)&client_addr, client_addr_len);
            if (send_len == SOCKET_ERROR) {
                printf("Failed to send ACK packet\n");
                return 1;
            }
        }
    }

    closesocket(socket_fd);
    WSACleanup();

    return 0;
}

