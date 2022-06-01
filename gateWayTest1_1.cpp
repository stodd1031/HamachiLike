#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <thread>
#include <iostream>
#include <bitset>
#include <time.h>
#include <algorithm>

#define PORT 8080

std::string myIP;
struct sockaddr_in myAddr;
struct sockaddr_in serAddr;
socklen_t len;
int sockfd;

void bindSocket()
{
#ifdef WIN32
    // Prepare the sockaddr_in structure
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = inet_addr(myIP.c_str());
    // myAddr.sin_addr.s_addr = INADDR_ANY;
    myAddr.sin_port = htons(PORT);

    // Bind
    if (bind(sockfd, (struct sockaddr *)&myAddr, sizeof(myAddr)) ==
        SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

#else
    // Filling server information
    myAddr.sin_family = AF_INET; // IPv4
    myAddr.sin_addr.s_addr = inet_addr(myIP.c_str());
    myAddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&myAddr, sizeof(myAddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
#endif
}

void openSocket()
{
    // Creating socket file descriptor
#ifdef WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
    }

    // unsigned int tv = 1000;
    // char tvchar[sizeof(tv)];
    // memcpy(tvchar, &tv, sizeof(tv));
    // setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
#else
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // struct timeval tv;
    // tv.tv_sec = 1;
    // tv.tv_usec = 0;
    // setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
    len = sizeof(sockaddr_in);

    char host[256];
    gethostname(host, sizeof(host)); // find the host name

    struct hostent *host_entry;
    host_entry = gethostbyname(host); // find host information

    in_addr **addr_list;
    addr_list = (in_addr **)host_entry->h_addr_list;

    for (int i = 0; addr_list[i] != NULL; i++)
    {
        std::cout << inet_ntoa(*addr_list[i]) << std::endl;
    }
    myIP = inet_ntoa(*addr_list[0]);
    std::cout << inet_ntoa(*addr_list[0]) << std::endl;

    struct addrinfo hints;
    struct addrinfo *result, *rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    getaddrinfo(host, "8080", &hints, &result);

    // for (rp = result; rp != NULL; rp = rp->ai_next)
    // {
    //     int sfd = socket(rp->ai_family, rp->ai_socktype,
    //                  rp->ai_protocol);
    //     if (sfd == -1)
    //         continue;

    //     if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
    //         break; /* Success */

    //     close(sfd);
    // }

    // for (rp = result; rp != NULL; rp = rp->ai_next)
    // {
    //     sockaddr_in *addr_in = (struct sockaddr_in *)rp->ai_addr;
    //     char *s = inet_ntoa(addr_in->sin_addr);
    //     printf("IP address: %s\n", s);
    // }

    // for (rp = result; rp != NULL; rp = rp->ai_next)
    // {
    //     char 
    //     inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
    //                 s, maxlen);
    //     printf("IP address: %s\n", s);
    // }
}

int main()
{
    openSocket();
    memset(&serAddr, 0, sizeof(serAddr));
    serAddr.sin_family = AF_INET;
    serAddr.sin_addr.s_addr = inet_addr(myIP.c_str());
    serAddr.sin_port = htons(PORT);
    sendto(sockfd, "hi", 2, 0, (const sockaddr*)&serAddr, sizeof(serAddr));
}