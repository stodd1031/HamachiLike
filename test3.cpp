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

// unsigned char raw[] = {69, 0, 0, 64, 132, 77, 64, 0, 64, 1, 97, 94, 192, 168, 11, 53, 173, 194, 219, 113, 8, 0, 106, 34, 0, 3, 0, 1, 73, 40, 11, 98, 0, 0, 0, 0, 53, 74, 9, 0, 0, 0, 0, 0, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};
unsigned char raw[] = {69, 0, 0, 64, 127, 150, 64, 0, 64, 1, 67, 101, 10, 140, 156, 237, 142, 250, 65, 78, 8, 0, 69, 33, 0, 20, 0, 1, 210, 173, 11, 98, 0, 0, 0, 0, 208, 180, 9, 0, 0, 0, 0, 0, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};
                    
struct ping_pkt
{
    struct icmphdr hdr;
    char msg[64 - (sizeof(struct icmphdr))];
};

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main()
{
    struct hostent *host_entity = gethostbyname("google.com");
    // struct hostent *host_entity = gethostbyname("142.250.65.78");
    // struct hostent *host_entity = gethostbyname("192.168.11.53");

    if (host_entity == NULL)
    {
        std::cout << "fail" << std::endl;
    }

    struct sockaddr_in addr_con;
    memset(&addr_con, 0, sizeof(addr_con));
    addr_con.sin_family = host_entity->h_addrtype;
    addr_con.sin_port = htons(0);
    addr_con.sin_addr.s_addr = *(long *)host_entity->h_addr;
    // addr_con.sin_addr.s_addr = 1;

    // int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    // if(sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)<0)
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP))<0)
    {
        perror("socket: ");
        exit(1);
    }

    int on = 1;
    int off = 0;
    // setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    struct ping_pkt pckt;

    bzero(&pckt, sizeof(pckt));

    pckt.hdr.type = ICMP_ECHO;
    pckt.hdr.un.echo.id = getpid();

    for (int i = 0; i < sizeof(pckt.msg) - 1; i++)
        pckt.msg[i] = i + '0';

    pckt.msg[sizeof(pckt) - 1] = 0;
    pckt.hdr.un.echo.sequence = 0;
    pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

    socklen_t addr_len = sizeof(addr_con);

    // memcpy(&pckt, raw, sizeof(ping_pkt));

    sendto(sockfd, &pckt, sizeof(pckt), 0,
           (struct sockaddr *)&addr_con,
           addr_len);

    // send(sockfd, &pckt, sizeof(pckt), 0);

    std::string from;
    std::string to;

    for (int i = 0; i < 4; i++)
    {
        from += std::to_string((int)(*((unsigned char *)&pckt + 12 + i)));
        to += std::to_string((int)(*((unsigned char *)&pckt + 16 + i)));
        if (i != 3)
        {
            from += ".";
            to += ".";
        }
    }

    // std::cout << "From: " << from << std::endl;
    // std::cout << "To: " << to << std::endl;

    memset(&pckt, 0, sizeof(pckt));

    recvfrom(sockfd, &pckt, sizeof(pckt), 0,
             (struct sockaddr *)&addr_con, &addr_len);

    // read (sockfd, &pckt, sizeof(pckt));

    std::string from2;
    std::string to2;

    for (int i = 0; i < 4; i++)
    {
        from2 += std::to_string((int)(*((unsigned char *)&pckt + 12 + i)));
        to2 += std::to_string((int)(*((unsigned char *)&pckt + 16 + i)));
        if (i != 3)
        {
            from2 += ".";
            to2 += ".";
        }
    }

    // std::cout << "From: " << from2 << std::endl;
    // std::cout << "To: " << to2 << std::endl;

    if (to == from2 && from == to2)
    {
        std::cout << "success" << std::endl;
    }
    else
    {
        std::cout << "failure" << std::endl;

        for (int i = 0; i < 64; i++)
        {
            std::cout << +(*(((unsigned char *)raw) + i)) << " " << std::flush;
        }
        std::cout << std::endl << std::endl;

        for (int i = 0; i < 64; i++)
        {
            std::cout << +(*(((unsigned char *)&pckt) + i)) << " " << std::flush;
        }
        std::cout << std::endl;
    }

    close(sockfd);

    return 0;
}