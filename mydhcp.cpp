#include <sys/socket.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <ell/dhcp.h>
#include <memory.h>
#include <stdio.h>
#include <ell/dhcp.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if.h>
#include <netinet/ether.h>

#include <stddef.h> /* offsetof */

// #include <libnet/libnet-headers.h>

struct dhcpv4_hdr
{
    uint8_t dhcp_opcode; /* opcode */
#define DHCP_REQUEST 0x1
#define DHCP_REPLY 0x2
    uint8_t dhcp_htype;      /* hardware address type */
    uint8_t dhcp_hlen;       /* hardware address length */
    uint8_t dhcp_hopcount;   /* used by proxy servers */
    uint32_t dhcp_xid;       /* transaction ID */
    uint16_t dhcp_secs;      /* number of seconds since trying to bootstrap */
    uint16_t dhcp_flags;     /* flags for DHCP, unused for BOOTP */
    uint32_t dhcp_cip;       /* client's IP */
    uint32_t dhcp_yip;       /* your IP */
    uint32_t dhcp_sip;       /* server's IP */
    uint32_t dhcp_gip;       /* gateway IP */
    uint8_t dhcp_chaddr[16]; /* client hardware address, len is dhcp_hlen */
    char dhcp_sname[64];     /* server host name, null terminated string */
    char dhcp_file[128];     /* boot file name, null terminated string */
    uint32_t dhcp_magic;     /* BOOTP magic header */
/* #region dhcpDefines */
#define DHCP_MAGIC 0x63825363
#define BOOTP_MIN_LEN 0x12c
#define DHCP_PAD 0x00
#define DHCP_SUBNETMASK 0x01
#define DHCP_TIMEOFFSET 0x02
#define DHCP_ROUTER 0x03
#define DHCP_TIMESERVER 0x04
#define DHCP_NAMESERVER 0x05
#define DHCP_DNS 0x06
#define DHCP_LOGSERV 0x07
#define DHCP_COOKIESERV 0x08
#define DHCP_LPRSERV 0x09
#define DHCP_IMPSERV 0x0a
#define DHCP_RESSERV 0x0b
#define DHCP_HOSTNAME 0x0c
#define DHCP_BOOTFILESIZE 0x0d
#define DHCP_DUMPFILE 0x0e
#define DHCP_DOMAINNAME 0x0f
#define DHCP_SWAPSERV 0x10
#define DHCP_ROOTPATH 0x11
#define DHCP_EXTENPATH 0x12
#define DHCP_IPFORWARD 0x13
#define DHCP_SRCROUTE 0x14
#define DHCP_POLICYFILTER 0x15
#define DHCP_MAXASMSIZE 0x16
#define DHCP_IPTTL 0x17
#define DHCP_MTUTIMEOUT 0x18
#define DHCP_MTUTABLE 0x19
#define DHCP_MTUSIZE 0x1a
#define DHCP_LOCALSUBNETS 0x1b
#define DHCP_BROADCASTADDR 0x1c
#define DHCP_DOMASKDISCOV 0x1d
#define DHCP_MASKSUPPLY 0x1e
#define DHCP_DOROUTEDISC 0x1f
#define DHCP_ROUTERSOLICIT 0x20
#define DHCP_STATICROUTE 0x21
#define DHCP_TRAILERENCAP 0x22
#define DHCP_ARPTIMEOUT 0x23
#define DHCP_ETHERENCAP 0x24
#define DHCP_TCPTTL 0x25
#define DHCP_TCPKEEPALIVE 0x26
#define DHCP_TCPALIVEGARBAGE 0x27
#define DHCP_NISDOMAIN 0x28
#define DHCP_NISSERVERS 0x29
#define DHCP_NISTIMESERV 0x2a
#define DHCP_VENDSPECIFIC 0x2b
#define DHCP_NBNS 0x2c
#define DHCP_NBDD 0x2d
#define DHCP_NBTCPIP 0x2e
#define DHCP_NBTCPSCOPE 0x2f
#define DHCP_XFONT 0x30
#define DHCP_XDISPLAYMGR 0x31
#define DHCP_DISCOVERADDR 0x32
#define DHCP_LEASETIME 0x33
#define DHCP_OPTIONOVERLOAD 0x34
#define DHCP_MESSAGETYPE 0x35
#define DHCP_SERVIDENT 0x36
#define DHCP_PARAMREQUEST 0x37
#define DHCP_MESSAGE 0x38
#define DHCP_MAXMSGSIZE 0x39
#define DHCP_RENEWTIME 0x3a
#define DHCP_REBINDTIME 0x3b
#define DHCP_CLASSSID 0x3c
#define DHCP_CLIENTID 0x3d
#define DHCP_NISPLUSDOMAIN 0x40
#define DHCP_NISPLUSSERVERS 0x41
#define DHCP_MOBILEIPAGENT 0x44
#define DHCP_SMTPSERVER 0x45
#define DHCP_POP3SERVER 0x46
#define DHCP_NNTPSERVER 0x47
#define DHCP_WWWSERVER 0x48
#define DHCP_FINGERSERVER 0x49
#define DHCP_IRCSERVER 0x4a
#define DHCP_STSERVER 0x4b
#define DHCP_STDASERVER 0x4c
#define DHCP_END 0xff

#define DHCP_MSGDISCOVER 0x01
#define DHCP_MSGOFFER 0x02
#define DHCP_MSGREQUEST 0x03
#define DHCP_MSGDECLINE 0x04
#define DHCP_MSGACK 0x05
#define DHCP_MSGNACK 0x06
#define DHCP_MSGRELEASE 0x07
#define DHCP_MSGINFORM 0x08
    /* #endregion */
};

struct my_iphdr
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ihl : 4;
    unsigned int version : 4;
#elif __BYTE_ORDER == __BIG_ENDIAN
    unsigned int version : 4;
    unsigned int ihl : 4;
#else
#error "Please fix <bits/endian.h>"
#endif
    uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint16_t frag_off : 13;
    uint8_t flags : 3;
#elif __BYTE_ORDER == __BIG_ENDIAN
    uint8_t flags : 3;
    uint16_t frag_off : 13;
#else
#error "Please fix <bits/endian.h>"
#endif
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
    /*The options start here. */
};

struct dhcp_pack
{
    struct ether_header eth;
    my_iphdr ip;
    udphdr udp;
    struct dhcpv4_hdr dhcp;
};

void getInterfaceIp(char *&ip)
{
    ifreq ifr;
    IFNAMSIZ;

    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {

        if (ifa->ifa_addr == NULL)
            continue;

        s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if ((ifa->ifa_addr->sa_family == AF_INET))
        {
            if (s)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("\tInterface : <%s>\n", ifa->ifa_name);
            printf("\t  Address : <%s>\n", host);
            // str compare, 10 and 192.168
            if (strncmp(host, "10.", 3) == 0 || strncmp(host, "192.168.", 8) == 0)
            {
                ip = (char *)malloc(strlen(host));
                strcpy(ip, host);
                break;
            }
        }
    }
    freeifaddrs(ifaddr);
}

dhcp_pack setupDhcpPacket()
{
    struct dhcp_pack pack;
    memset(&pack, 0, sizeof(struct dhcp_pack));

    pack.eth.ether_type = htons(ETH_P_IP);
    pack.eth.ether_shost[0] = 0x40;
    pack.eth.ether_shost[1] = 0x8d;
    pack.eth.ether_shost[2] = 0x5c;
    pack.eth.ether_shost[3] = 0xe1;
    pack.eth.ether_shost[4] = 0xd2;
    pack.eth.ether_shost[5] = 0xd8;

    memset(&pack.eth, 10, sizeof(struct ether_header));

    pack.ip.version = 4;
    pack.ip.ihl = 5;
    pack.ip.tos = 0;
    pack.ip.tot_len = htons(sizeof(dhcp_pack) + 4);
    pack.ip.id = 29845;
    pack.ip.flags = 0;
    pack.ip.frag_off = 0;
    pack.ip.ttl = 255;
    pack.ip.protocol = 17;
    // checksum
    pack.ip.saddr = 0;
    pack.ip.daddr = ~0;

    pack.udp.dest = htons(67);
    pack.udp.source = htons(68);
    pack.udp.len = htons(8 + sizeof(struct dhcp_pack) + 4);
    // pack.udp.check =

    pack.dhcp.dhcp_opcode = 1;
    pack.dhcp.dhcp_htype = 1;
    pack.dhcp.dhcp_hlen = 6;
    pack.dhcp.dhcp_hopcount = 0;
    // pack.dhcp.dhcp_xid = 256 * 256 * 12 | 256 * 6 | 106;
    pack.dhcp.dhcp_xid = htonl(0x19f05a00);
    pack.dhcp.dhcp_secs = htons(4);
    pack.dhcp.dhcp_flags = 0;
    pack.dhcp.dhcp_cip = 0;
    pack.dhcp.dhcp_yip = 0;
    pack.dhcp.dhcp_sip = 0;
    pack.dhcp.dhcp_gip = 0;
    // pack.dhcp.dhcp_chaddr[0] = 0xc0;
    // pack.dhcp.dhcp_chaddr[1] = 0xb5;
    // pack.dhcp.dhcp_chaddr[2] = 0xd7;
    // pack.dhcp.dhcp_chaddr[3] = 0x09;
    // pack.dhcp.dhcp_chaddr[4] = 0xf6;
    // pack.dhcp.dhcp_chaddr[5] = 0xf7;
    pack.dhcp.dhcp_chaddr[0] = 0xc0;
    pack.dhcp.dhcp_chaddr[1] = 0xb5;
    pack.dhcp.dhcp_chaddr[2] = 0xd7;
    pack.dhcp.dhcp_chaddr[3] = 0x09;
    pack.dhcp.dhcp_chaddr[4] = 0xf6;
    pack.dhcp.dhcp_chaddr[5] = 0xf7;
    pack.dhcp.dhcp_magic = htonl(DHCP_MAGIC);

    return pack;
}

int main()
{
    dhcp_pack pak = setupDhcpPacket();
    // char *ip = nullptr;
    // getInterfaceIp(ip);
    // printf("%s\n", ip);

    int sockfd;
    // sock_raw receives ip header and udp/tcp... header
    // sock_raw requires udp/tcp... header tp send
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0)
    {
        perror("socket(): ");
        return 1;
    }
    int on = 1;
    // requires ip header and udp/tcp... header
    // setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    // setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));

    char host[255];
    gethostname(host, 255);

    struct hostent *host_entity;
    host_entity = gethostbyname(host);

    // printf("%s\n", inet_ntoa(*((struct in_addr *)host_entity->h_addr)));

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    // addr.sin_addr = *((struct in_addr *)host_entity->h_addr);
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    // addr.sin_port = htons(67);
    // addr.sin_port = 0;

    // if ((bind(sockfd, (const sockaddr *)&addr, sizeof(addr))) < 0)
    // {
    //     perror("bind(): ");
    //     return 1;
    // }

    memset(&addr, 0, sizeof(addr));

    socklen_t len = sizeof(addr);

    unsigned char buf[1428];

    pak.udp.check = 0;
    pak.udp.len = htons(0x014a);
    pak.ip.check = 0;
    pak.ip.tot_len = htons(0x015e);

    memcpy(buf, &pak, sizeof(pak));

    int size = 82;
    // unsigned char val[] = {0x35, 0x1, 0x1, 0xff};
    // size 82 V
    unsigned char val[] = {0x35, 0x1, 0x1, 0x3d, 0x7, 0x1, 0xa4, 0x97, 0xb1, 0x39, 0xa3, 0x87, 0x32, 0x4, 0xa, 0x8c, 0x8a, 0x85, 0xc, 0xf, 0x4c, 0x41, 0x50, 0x54, 0x4f, 0x50, 0x2d, 0x39, 0x32, 0x34, 0x4e, 0x39, 0x55, 0x41, 0x39, 0x51, 0x12, 0x0, 0x0, 0x0, 0x4c, 0x41, 0x50, 0x54, 0x4f, 0x50, 0x2d, 0x39, 0x32, 0x34, 0x4e, 0x39, 0x55, 0x41, 0x39, 0x3c, 0x8, 0x4d, 0x53, 0x46, 0x54, 0x20, 0x35, 0x2e, 0x30, 0x37, 0xe, 0x1, 0x3, 0x6, 0xf, 0x1f, 0x21, 0x2b, 0x2c, 0x2e, 0x2f, 0x77, 0x79, 0xf9, 0xfc, 0xff};

    memcpy(buf + sizeof(pak), val, size);

    sendto(sockfd, buf, sizeof(pak) + size, 0, (const sockaddr *)&addr, len);

    for (int index = 0; index < sizeof(pak) + size; index++)
    {
        if (index == 34 || index == 42)
        // if (index == 8)
        {
            printf("| ");
        }
        printf("%x ", buf[index]);
    }
    printf("\n");
    printf("source: %u.%u.%u.%u:%u\n", buf[30], buf[31], buf[32], buf[33], (buf[34] << 8) + buf[35]);
    printf("dest: %u.%u.%u.%u:%u\n", buf[30], buf[31], buf[32], buf[33], (buf[36] << 8) + buf[37]);

    while (1)
    {
        int r = recvfrom(sockfd, buf, 1428, 0, (sockaddr *)&addr, &len);
        if (r < 0)
        {
            return 1;
        }
        if ((buf[20] << 8) + buf[21] != 67 && (buf[20] << 8) + buf[21] != 68)
        {
            continue;
        }
        dhcp_pack tmp;
        memcpy(&tmp, buf, r);
        // if (buf[56] == 0xf7 || buf[56] == 0xc0)
        // {

        // }
        // else
        // {
        //     continue;
        // }
        if (buf[12] == 0)
        {
            continue;
        }
        if (tmp.dhcp.dhcp_xid == htonl(0x19f05a02) || tmp.dhcp.dhcp_xid == 0x19f05a02)
        {
            printf("AAHAHAHAH: %X\n", htonl(tmp.dhcp.dhcp_xid));
            printf("AAHAHAHAH: %X\n", tmp.dhcp.dhcp_xid);
        }
        // printf("%X\n", tmp.dhcp.dhcp_xid);
        // printf("%X\n", htonl(tmp.dhcp.dhcp_xid));
        for (int index = 0; index < r; index++)
        {
            if (index == 20 || index == 28)
            // if (index == 8)
            {
                printf("| ");
            }
            printf("%x ", buf[index]);
        }
        printf("\n");
        printf("source: %u.%u.%u.%u:%u\n", buf[12], buf[13], buf[14], buf[15], (buf[20] << 8) + buf[21]);
        printf("dest: %u.%u.%u.%u:%u\n", buf[16], buf[17], buf[18], buf[19], (buf[22] << 8) + buf[23]);

        // printf("%i\n", addr.sin_port);
        // memset(buf + 20, 0, 2);
        // sendto(sockfd, buf + 20, r - 20, 0, (const sockaddr *)&addr, len);
        // sendto(sockfd, buf, r, 0, (const sockaddr *)&addr, len);
        // break;
    }
    int r = recvfrom(sockfd, buf, 1400, 0, (sockaddr *)&addr, &len);
    for (int index = 0; index < r; index++)
    {
        if (index == 20 || index == 28)
        // if (index == 8)
        {
            printf("| ");
        }
        printf("%x ", buf[index]);
    }
    printf("\n");
    printf("source: %u.%u.%u.%u:%u\n", buf[12], buf[13], buf[14], buf[15], (buf[20] << 8) + buf[21]);
    printf("dest: %u.%u.%u.%u:%u\n", buf[16], buf[17], buf[18], buf[19], (buf[22] << 8) + buf[23]);
}