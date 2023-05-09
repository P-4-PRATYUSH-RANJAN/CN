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
    struct sockaddr_in server_addr;
    int server_addr_len = sizeof(server_addr);
    int seq_num = 0;
    int base = 0;
    int next_seq_num = 0;
    int ack_received[WINDOW_SIZE];
    int i;
    int send_len;
    Packet packet;
    Packet recv_packet;

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

    // Send data
    while (1) {
        // Send packets
        while (next_seq_num < base + WINDOW_SIZE) {
            // Read data from file
            memset(&packet, 0, sizeof(Packet));
            send_len = fread(packet.data, 1, PACKET_SIZE, stdin);
            if (send_len == 0) {
                // End of file
                break;
            }
            packet.seq_num = seq_num;

            // Send packet
            send_len = sendto(socket_fd, (char *)&packet, sizeof(Packet), 0, (struct sockaddr *)&server_addr, server_addr_len);
            if (send_len == SOCKET_ERROR) {
                printf("Failed to send packet\n");
                return 1;
            }

            // Update variables
            next_seq_num++;
            seq_num++;
        }

        // Receive ACKs
        memset(&recv_packet, 0, sizeof(Packet));
        recv_packet.seq_num = -1;
        recv_len = recvfrom(socket_fd, (char *)&recv_packet, sizeof(Packet), 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (recv_len == SOCKET_ERROR) {
            printf("Failed to receive ACK packet\n");
            return 1;
        }

        if (recv_packet.seq_num >= base && recv_packet.seq_num < base + WINDOW_SIZE) {
            // Update ACK received array
            ack_received[recv_packet.seq_num - base] = 1;

            // Slide window
            while (ack_received[0] == 1) {
                // Update variables
                base++;
                next_seq_num--;

                // Remove first ACK received from array
                for (i = 0; i < WINDOW_SIZE - 1; i++) {
                    ack_received[i] = ack_received[i+1];
                }
                ack_received[WINDOW_SIZE - 1] = 0;
            }
        }
    }

    closesocket(socket_fd);
    WSACleanup();

    return 0;
}

