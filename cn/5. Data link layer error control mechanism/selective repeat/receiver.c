#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define MAX_PACKET_SIZE 1024
#define WINDOW_SIZE 5
#define TIMEOUT 5000

typedef struct packet {
    int seq_num;
    char data[MAX_PACKET_SIZE];
} packet_t;

int main(int argc, char *argv[]) {
    WSADATA wsa;
    SOCKET client_sock;
    struct sockaddr_in server_addr;
    int server_len = sizeof(server_addr);
    packet_t send_pkt, recv_ack;
    int packets_sent = 0, ack_num = 0;
    int window_base = 0;
    int window_top = WINDOW_SIZE - 1;

    if (argc != 3) {
        printf("Usage: %s <ip address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock\n");
        exit(EXIT_FAILURE);
    }

    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Failed to create socket: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Failed to connect to server: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    printf("Connected to server: %s:%d\n", argv[1], atoi(argv[2]));

    while (1) {
        if (packets_sent < window_top) {
            memset(&send_pkt, 0, sizeof(send_pkt));
            send_pkt.seq_num = packets_sent;
            sprintf(send_pkt.data, "Packet %d", packets_sent);

            if (send(client_sock, (char *)&send_pkt, sizeof(send_pkt), 0) == SOCKET_ERROR) {
                printf("Failed to send data: %d\n", WSAGetLastError());
                exit(EXIT_FAILURE);
            }

            printf("Sent packet %d\n", packets_sent);
            packets_sent++;
        }

        if (recv(client_sock, (char *)&recv_ack, sizeof(recv_ack), 0) == SOCKET_ERROR) {
            printf("Failed to receive data: %d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }

        printf("Received ACK %d\n", recv_ack.seq_num);

        if (recv_ack.seq_num >= window_base && recv_ack.seq_num <= window_top) {
            ack_num++;

            if (recv_ack.seq_num == window_base) {
                while (ack_num > 0) {
                    ack_num--;
                    window_base++;
                    window_top++;
                }
            }
        }

        if (packets_sent == WINDOW_SIZE && window_base == packets_sent) {
            break;
        }
    }

    printf("All packets sent and acknowledged\n");

    closesocket(client_sock);
    WSACleanup();

    return 0;
}
