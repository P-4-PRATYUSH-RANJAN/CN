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
    struct sockaddr_in receiver_addr;
    struct sockaddr_in sender_addr;
    int sender_addr_len = sizeof(sender_addr);
    int expected_seq_num = 0;
    int recv_len;
    Packet packet;
    Packet ack_packet;

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

    // Bind socket to port
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr = INADDR_ANY;
    receiver_addr.sin_port = htons(PORT);
    if (bind(socket_fd, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) == SOCKET_ERROR) {
        printf("Failed to bind socket\n");
        return 1;
    }

    // Receive data
    while (1) {
        memset(&packet, 0, sizeof(Packet));
        memset(&ack_packet, 0, sizeof(Packet));
        recv_len = recvfrom(socket_fd, (char *)&packet, sizeof(Packet), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
        if (recv_len == SOCKET_ERROR) {
            printf("Failed to receive packet\n");
            return 1;
        }

        if (packet.seq_num == expected_seq_num) {
            // Send ACK
            ack_packet.seq_num = expected_seq_num;
            if (sendto(socket_fd, (char *)&ack_packet, sizeof(Packet), 0, (struct sockaddr *)&sender_addr, sender_addr_len) == SOCKET_ERROR) {
                printf("Failed to send ACK packet\n");
                return 1;
            }

            // Output data
            fwrite(packet.data, 1, recv_len - sizeof(int), stdout);
            expected_seq_num++;
        } else {
            // Send ACK for last correctly received packet
            ack_packet.seq_num = expected_seq_num - 1;
            if (sendto(socket_fd, (char *)&ack_packet, sizeof(Packet), 0, (struct sockaddr *)&sender_addr, sender_addr_len) == SOCKET_ERROR) {
                printf("Failed to send ACK packet\n");
                return 1;
            }
        }
    }

    closesocket(socket_fd);
    WSACleanup();

    return 0;
}

