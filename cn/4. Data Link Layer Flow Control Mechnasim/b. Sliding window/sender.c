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
    int window_start = 0;
    int window_end = 0;
    int unacknowledged = 0;
    int data_len, send_len, recv_len;
    char data[PACKET_SIZE];
    Packet window[WINDOW_SIZE];
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
        // Send packets in the window
        while (window_end < window_start + WINDOW_SIZE && fgets(data, PACKET_SIZE, stdin) != NULL) {
            // Remove newline character
            data_len = strlen(data);
            if (data[data_len-1] == '\n') {
                data[data_len-1] = '\0';
                data_len--;
            }

            // Create packet
            memset(&window[window_end % WINDOW_SIZE], 0, sizeof(Packet));
            window[window_end % WINDOW_SIZE].seq_num = window_end;
            memcpy(window[window_end % WINDOW_SIZE].data, data, data_len);

            // Send packet
            send_len = sendto(socket_fd, (char *)&window[window_end % WINDOW_SIZE], sizeof(Packet), 0, (struct sockaddr *)&server_addr, server_addr_len);
            if (send_len == SOCKET_ERROR) {
                printf("Failed to send packet\n");
                return 1;
            }

            window_end++;
        }

        // Receive ACK packets
        memset(&recv_packet, 0, sizeof(Packet));
        recv_len = recvfrom(socket_fd, (char *)&recv_packet, sizeof(Packet), 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (recv_len == SOCKET_ERROR) {
            printf("Failed to receive ACK packet\n");
            return 1;
        }

        // Update window and unacknowledged packet
        if (recv_packet.seq_num >= window_start && recv_packet.seq_num < window_end) {
            unacknowledged = recv_packet.seq_num;
            window_start = unacknowledged + 1;
        }

        // Check if all packets have been acknowledged
        if (unacknowledged == window_end - 1) {
            break;
        }
    }

    closesocket(socket_fd);
    WSACleanup();

    return 0;
}

