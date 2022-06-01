#include "dhcpfinal.h"

int main()
{
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    struct sockaddr_ll socket_address;
    int len;

    unsigned char buf[1500];

    struct packet_mreq mreq;
    bzero(&mreq, sizeof(struct packet_mreq));

    mreq.mr_ifindex = if_nametoindex("enp0s31f6");
    // mreq.mr_ifindex = if_nametoindex("wlan0");
    mreq.mr_type = PACKET_MR_PROMISC;

    // if (setsockopt(sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    // {
    //     perror("sockopt");
    //     return 1;
    // }

    // if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, "enp0s31f6", IFNAMSIZ-1) == -1)	{
    // 	perror("SO_BINDTODEVICE");
    // 	close(sockfd);
    // 	exit(EXIT_FAILURE);
    // }

    // 08 36 C9 8E FD 0C
    // 40:8d:5c:e1:d2:d6



    while (1)
    {
        memset(buf, 0, 1500);
        int r = recvfrom(sockfd, buf, 1500, 0, (struct sockaddr *)&socket_address, &len);
        // if (strncmp(buf, (unsigned char[6]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 6) == 0)
        // if (htonl(*(unsigned int *)(buf+46)) == 0x19f05a93 || strncmp(buf, (unsigned char[6]){0x40, 0x8d, 0x5c, 0xe1, 0xd2, 0xd6}, 6) == 0 || strncmp(buf, (unsigned char[6]){0x40, 0x8d, 0x5c, 0xe1, 0xd2, 0xd8}, 6) == 0)// || (strncmp(buf, (unsigned char[6]){0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 6) == 0 && strncmp(buf + 6, (unsigned char[6]){0x40, 0x8d, 0x5c, 0xe1, 0xd2, 0xd8}, 6) == 0))
        // if (strncmp(buf, TMP_USE, 6) == 0 || strncmp(buf + 6, TMP_USE, 6) == 0)
        // if(((struct ether_header*)buf)->ether_type == htons(ETH_P_ARP))
        // if (((struct dhcpv4_hdr *)(buf + sizeof(struct my_iphdr) + sizeof(struct udphdr) + sizeof(struct ether_header)))->dhcp_magic == htonl(DHCP_MAGIC))
        // if ((strncmp(buf, TMP_USE, 6) == 0 || strncmp(buf+6, TMP_USE, 6) == 0) && (strncmp(buf, BROADCAST_ADDR, 6) == 0 || strncmp(buf+6, BROADCAST_ADDR, 6) == 0))
        // if (strncmp(buf, ETHER_TEST_ADDR, 6) == 0 || strncmp(buf+6, ETHER_TEST_ADDR, 6) == 0)
        // if (htonl(*(unsigned int *)(buf+46)) == 0x19f05a93)
        {
            for (int index = 0; index < r; index++)
            {
                printf("%02X ", buf[index]);
            }
            printf("\n");
            printf("Dest mac: %02X:%02X:%02X:%02X:%02X:%02X\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
            printf("Source mac: %02X:%02X:%02X:%02X:%02X:%02X\n", buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
            printf("Source IP: %02i.%02i.%02i.%02i\n", buf[26], buf[27], buf[28], buf[29]);
            printf("Dest IP: %02i.%02i.%02i.%02i\n", buf[30], buf[31], buf[32], buf[33]);

            printf("Client IP: %02i.%02i.%02i.%02i\n", buf[54], buf[55], buf[56], buf[57]);
            printf("Your IP: %02i.%02i.%02i.%02i\n", buf[58], buf[59], buf[60], buf[61]);
            printf("Server IP: %02i.%02i.%02i.%02i\n", buf[62], buf[63], buf[64], buf[65]);
            printf("Gateway IP: %02i.%02i.%02i.%02i\n", buf[66], buf[67], buf[68], buf[69]);

            printf("\n");
        }
    }
}