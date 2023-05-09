#include <stdio.h>
#include <winsock2.h>

int main()
{
    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Failed to initialize Winsock\n");
        return 1;
    }

    // Create a socket
    SOCKET client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Failed to create socket\n");
        return 1;
    }

    // Connect to the server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
    server_addr.sin_port = htons(12345);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Failed to connect to server\n");
        return 1;
    }

    // Send data to server
    const char *message = "Hello, server! -> Subroto Rakshit";
    int send_len = send(client_socket, message, strlen(message), 0);
    if (send_len == SOCKET_ERROR) {
        printf("Failed to send data to server\n");
        return 1;
    }

    // Receive data from server
    char buffer[1024];
    int recv_len = recv(client_socket, buffer, sizeof(buffer), 0);
    if (recv_len == SOCKET_ERROR) {
        printf("Failed to receive data from server\n");
        return 1;
    }

    // Process received data
    printf("Received %d bytes: %.*s\n", recv_len, recv_len, buffer);

    // Cleanup
    closesocket(client_socket);
    WSACleanup();

    return 0;
}
