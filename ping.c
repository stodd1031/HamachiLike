#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include <stdio.h>

int main()
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP))<0)
    {
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    // addr.sin_addr.s_addr = inet_addr("192.168.99.37");
    // addr.sin_addr.s_addr = inet_addr("10.140.143.152");
    addr.sin_addr.s_addr = inet_addr("10.140.141.49");
    // addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    if (bind(sockfd, (const struct sockaddr*)&addr, sizeof(addr))<0)
    {
        return 1;
    }

    socklen_t len;

    unsigned char in[1000];
    while(1)
    {
        memset(in, 0, 1000);
        int r = recvfrom(sockfd, in, 1000, 0, (struct sockaddr*)&addr, &len);
        printf("from: %i.%i.%i.%i:%i\n", in[12], in[13],in[14],in[15], in[20] << 8 | in[21]);
        for (int i = 0; i < r; i++)
        {
            printf("%X ", in[i]);
        }
        printf("\n");
    }
}