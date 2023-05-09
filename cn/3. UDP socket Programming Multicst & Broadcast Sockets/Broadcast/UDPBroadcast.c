#include <stdio.h>
#include <winsock2.h>

#define PORT 8888
#define BUFLEN 512
#define BROADCASTIP "192.168.1.255"

void die(char *s)
{
    perror(s);
    exit(1);
}

int main(void)
{
    struct sockaddr_in si_me, si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
    {
        die("Subroto Rakshit -> socket");
    }

    int broadcastEnable = 1;
    int ret = setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char *) &broadcastEnable, sizeof(broadcastEnable));
    if (ret < 0)
    {
        die("setsockopt");
    }

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr*)&si_me, sizeof(si_me)) == SOCKET_ERROR)
    {
        die("bind");
    }

    while(1)
    {
        printf("Waiting for data...\n");
        fflush(stdout);

        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR)
        {
            die("recvfrom");
        }

        printf("Received packet from %s:%d\nData: %s\n\n",
               inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
    }

    closesocket(s);
    WSACleanup();

    return 0;
}

