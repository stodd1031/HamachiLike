#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define MSG_CONFIRM 0
// #define MSG_WAITALL 0
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/socket.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <thread>

#include <iostream>
#include <bitset>

#include <time.h>
#include <algorithm>

// #define MAXLEN 1024
#define PORT 8080

#define KEY 1
#define KEY_SOLVED 2
#define CONNECTED 3
#define MESSAGE 4

struct Key
{
    char type;
    int a;
    int b;
    int c;
};

struct Message
{
    char type;
    char text[512];
};

struct Connected
{
    char type;
};

union Send
{
    Send()
    {
        memset(this, 0, sizeof(Send));
    }
    char type;
    Key key;
    Message message;
};

/* #region  vars */
#ifdef WIN32
WSADATA wsa;
int len;
SOCKET sockfd;
#else
socklen_t len;
int sockfd;
#endif

struct sockaddr_in myAddr;
struct sockaddr_in recAddr;
struct sockaddr_in conAddr;
struct sockaddr_in finAddr;

std::string myIP;

bool connected = false;
Key myKey;
int myAnswer;
/* #endregion */

void recLoop();
void openSocket();
void bindSocket();

int main()
{
    memset(&myAddr, 0, sizeof(sockaddr_in));
    memset(&conAddr, 0, sizeof(sockaddr_in));
    memset(&recAddr, 0, sizeof(sockaddr_in));
    memset(&finAddr, 0, sizeof(sockaddr_in));

    openSocket();
    bindSocket();

    srand(time(NULL));
    myKey.a = rand() % 256 * 256 * 256 * 256;
    myKey.b = rand() % 256 * 256 * 256 * 256;
    myAnswer = myKey.a ^ myKey.b;

    std::cout << "What's the ip(0.0.0.0): " << std::flush;
    std::string ip;
    std::getline(std::cin, ip);
    std::cout << ip << std::endl;

    conAddr.sin_family = AF_INET;
    conAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    conAddr.sin_port = htons(PORT);

    std::thread t(recLoop);

    Send send;
    send.key = myKey;
    send.type = KEY;

    for (int index = 0; index < 5; index++)
    {
        sendto(sockfd, (char *)&send, sizeof(Send), 0, (sockaddr *)&conAddr, sizeof(conAddr));
        usleep(1 * 1000 * 1000);
    }

    // for(int index = 0; index < 256*256-1 && !connected; index++)
    // {
    //     // memset(&conAddr, 0, sizeof(conAddr));
    //     // conAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    //     // conAddr.sin_family = AF_INET;
    //     conAddr.sin_port = htons(index);
    //     sendto(sockfd, (char*)&send, sizeof(Send), 0, (sockaddr *)&conAddr, sizeof(conAddr));
    // }
    std::cout << "end" << std::endl;
    while (1)
    {
        std::string input;
        std::getline(std::cin, input);
        Send send;
        send.type = MESSAGE;
        memcpy(send.message.text, input.c_str(), std::min((int)input.size(), 512));
        sendto(sockfd, (char *)&send, sizeof(Send), 0, (sockaddr *)&finAddr, sizeof(finAddr));
    }
}

void recLoop()
{
    Send send;
    while (1)
    {
        memset(&send, 0, sizeof(Send));
        memset(&recAddr, 0, sizeof(recAddr));
        int n = recvfrom(sockfd, (char *)&send, sizeof(Send), 0, (struct sockaddr *)&recAddr,
                         &len);
        std::cout << "rec" << std::endl;
        if (send.type == KEY)
        {
            send.type = KEY_SOLVED;
            send.key.c = send.key.a ^ send.key.b;
            if (send.key.a == myKey.a && send.key.b == myKey.b && send.key.c == myAnswer)
            {
                continue;
            }
            sendto(sockfd, (char *)&send, sizeof(Send), 0, (struct sockaddr *)&recAddr, sizeof(recAddr));
        }
        else if (send.type == KEY_SOLVED)
        {
            if (send.key.c == myAnswer)
            {
                memcpy(&finAddr, &recAddr, sizeof(finAddr));
                std::cout << "Connected to " << inet_ntoa(recAddr.sin_addr) << ":" << ntohs(recAddr.sin_port) << std::endl;
                Send sendConnected;
                sendConnected.type = CONNECTED;
                sendto(sockfd, (char *)&sendConnected, sizeof(Send), 0, (struct sockaddr *)&finAddr, sizeof(finAddr));
            }
        }
        else if (send.type == CONNECTED)
        {
            memcpy(&finAddr, &recAddr, sizeof(finAddr));
            std::cout << "Connected to " << inet_ntoa(recAddr.sin_addr) << ":" << ntohs(recAddr.sin_port) << std::endl;
        }
        else if (send.type == MESSAGE)
        {
            memcpy(&finAddr, &recAddr, sizeof(finAddr));
            std::cout << inet_ntoa(recAddr.sin_addr) << ":" << ntohs(recAddr.sin_port) << "\n"
                      << send.message.text << std::endl;
        }
    }
}

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
