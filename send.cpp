#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <iostream>


int main()
{
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket(): ");
        exit(1);
    }
    // int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    struct hostent *host_entity = gethostbyname("127.0.0.1");

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    // addr.sin_addr.s_addr = inet_addr("192.168.11.53");
    addr.sin_addr.s_addr = *(long *)host_entity->h_addr;
    // addr.sin_addr.s_addr = inet_addr("10.140.141.138");
    // addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(54032);

    socklen_t len = sizeof(addr);

    // bind(sockfd, (const sockaddr*)&addr, len);

    char buf[1000];

    sendto(sockfd, "hello", 5, 0, (const sockaddr*)&addr, len);
}