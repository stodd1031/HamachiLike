#include "dhcpfinal.h"

// returns linked list of interfaces that have already recieved an ip from router
struct ifmacs *getifmacsWithIps()
{
    struct ifaddrs *addrs;
    getifaddrs(&addrs);

    struct ifmacs *ifs = NULL;
    // the retrun pointer, the first item in the linked list
    struct ifmacs *backup = NULL;

    // used for setting up frist item in linked list
    bool first = 1;

    // loop through addrs
    for (struct ifaddrs *tmp = addrs; tmp; tmp = tmp->ifa_next)
    {
        printf("%s\n", tmp->ifa_name);
        // if interface is ethernet
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_PACKET)
        {
            // loop through addrs again
            for (struct ifaddrs *tmp2 = addrs; tmp2; tmp2 = tmp2->ifa_next)
            {
                unsigned char *addrPtr = (unsigned char *)&((struct sockaddr_in *)tmp2->ifa_addr)->sin_addr;
                // if interface is ip and has same name as previous interface
                // and if ip is private (starts with 10. or 192.168.)
                if (tmp2->ifa_addr->sa_family == AF_INET && strncmp(tmp2->ifa_name, tmp->ifa_name, IFNAMSIZ) == 0 &&
                    (strncmp(addrPtr, CHAR_ARRAY{0x0a}, 1) == 0 || strncmp(addrPtr, CHAR_ARRAY{0xC0, 0xA8}, 2) == 0))
                {
                    if (first)
                    {
                        ifs = malloc(sizeof(struct ifmacs));
                        backup = ifs;
                        first = 0;
                    }
                    else
                    {
                        ifs->next = malloc(sizeof(struct ifmacs));
                        ifs = ifs->next;
                    }
                    ifs->next = NULL;
                    memcpy(ifs->addr, ((struct sockaddr_ll *)tmp->ifa_addr)->sll_addr, ETHER_ADDR_LEN);
                    memcpy(ifs->name, tmp->ifa_name, IFNAMSIZ);
                    ifs->index = if_nametoindex(tmp->ifa_name);
                    break;
                }
            }
        }
    }
    freeifaddrs(addrs);
    return backup;
}

short setupEthernetHeader(struct ether_header *eth, unsigned char *macDst, unsigned char *macSrc)
{
    memcpy(eth->ether_dhost, macDst, ETHER_ADDR_LEN);
    memcpy(eth->ether_shost, macSrc, ETHER_ADDR_LEN);
    eth->ether_type = htons(ETH_P_IP);

    return sizeof(struct ether_header);
}

// TODO ADD IP SRC AND DST PARAMETER
short setupIpHeader(struct my_iphdr *ip)
{
    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0x10;
    ip->id = 0;
    ip->frag_offset = 0;
    ip->ttl = 255;
    ip->protocol = IPPROTO_UDP;
    ip->saddr = 0;
    ip->daddr = ~0;

    return sizeof(struct my_iphdr);
}

// TODO SELECT SOURCE AND DESTINATION PORT
short setupUdpHeader(struct udphdr *udp)
{
    udp->source = htons(68);
    udp->dest = htons(67);

    return sizeof(struct udphdr);
}

short setupMainDhcp(struct dhcpv4_hdr *dhcp, unsigned char *macSrc, unsigned char *clientIP, unsigned char *yourIP, unsigned char *serverIP, unsigned char *gatewayIP)
{
    dhcp->dhcp_opcode = 1;
    dhcp->dhcp_htype = 1;
    dhcp->dhcp_hlen = 6;
    dhcp->dhcp_hopcount = 0;
    dhcp->dhcp_xid = htonl(XID);
    dhcp->dhcp_secs = 0;
    dhcp->dhcp_flags = 0;
    memcpy(&dhcp->dhcp_cip, clientIP, 4);
    memcpy(&dhcp->dhcp_yip, yourIP, 4);
    memcpy(&dhcp->dhcp_sip, serverIP, 4);
    memcpy(&dhcp->dhcp_gip, gatewayIP, 4);
    memcpy(dhcp->dhcp_chaddr, macSrc, ETHER_ADDR_LEN);
    dhcp->dhcp_magic = htonl(DHCP_MAGIC);

    return sizeof(struct dhcpv4_hdr);
}

short addDhcpOption(unsigned char *start, unsigned char option, unsigned char length, unsigned char *data)
{
    struct dhcp_option *op = (struct dhcp_option *)(start);
    op->data = start + 2;
    op->option = option;
    op->length = length;
    memcpy(op->data, data, length);

    return 2 + length;
}

void ipCecksum(struct my_iphdr *ip)
{
    int sum = 0;

    for (int index = 0; index < sizeof(struct my_iphdr) / 2; index++)
    {
        sum += (((unsigned short *)ip)[index]);
    }
    while (sum >> 16)
    {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    ip->check = (~(sum & 0xffff));
}

void udpCecksum(struct my_iphdr *ip, struct udphdr *udp)
{
    unsigned long sum = 0;
    for (unsigned short *ptr = (unsigned short *)&ip->saddr; (unsigned char *)ptr < (((unsigned char *)udp) + htons(udp->len)); ptr++)
    {
        sum += *ptr;
    }

    sum += htons(IPPROTO_UDP);
    sum += udp->len;

    while (sum >> 16)
    {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    udp->check = ~sum;
}

short dhcpPacket(unsigned char *buf,
                 unsigned char *macDst, unsigned char *macSrc,
                 unsigned char *clientIP, unsigned char *yourIP,
                 unsigned char *serverIP, unsigned char *gatewayIP,
                 unsigned char *dhcpOptions, unsigned short dhcpOptionsSize)
{
    unsigned short bufSize = 0;

    // ethernet header
    struct ether_header *eth = (struct ether_header *)buf;
    bufSize += setupEthernetHeader(eth, macDst, macSrc);

    // ip header
    struct my_iphdr *ip = (struct my_iphdr *)(buf + bufSize);
    bufSize += setupIpHeader(ip);

    // udp header
    struct udphdr *udp = (struct udphdr *)(buf + bufSize);
    bufSize += setupUdpHeader(udp);

    // dhcp information
    struct dhcpv4_hdr *dhcp = (struct dhcpv4_hdr *)(buf + bufSize);
    bufSize += setupMainDhcp(dhcp, macSrc, clientIP, yourIP, serverIP, gatewayIP);

    // write dhcp options
    memcpy(buf + bufSize, dhcpOptions, dhcpOptionsSize);
    bufSize += dhcpOptionsSize;

    // DHCP final byte
    buf[bufSize++] = 0xff;

    // ip length field
    ip->tot_len = htons(bufSize - sizeof(struct ether_header));
    // udp length field
    udp->len = htons(bufSize - sizeof(struct ether_header) - sizeof(struct my_iphdr));

    ipCecksum(ip);

    udpCecksum(ip, udp);

    // print hex of entire message
    for (int index = 0; index < bufSize && index < 1500; index++)
    {
        printf("%02X ", buf[index]);
    }
    printf("\n");

    return bufSize;
}

short dhcpDiscover(unsigned char *buf, unsigned char *mac)
{
    unsigned short dhcpOptionSize = 0;
    unsigned char dhcpOptionBuf[1500];
    memset(dhcpOptionBuf, 0, 1500);

    // DHCP options

    // DHCP Msg Type: DHCP_DISCOVER
    dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 0x35, 1, CHAR_ARRAY{0x01});

    // dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 61, 7, CHAR_ARRAY{0x01, 0x5D, 0xE1, 0xD2, 0xD8, 0x00, 0x04, 0x03, 0x8D, 0x02, 0x40, 0x04, 0x5C, 0x05, 0xE1, 0xD2, 0x06, 0xD8, 0x07, 0x00, 0x08, 0x00, 0x09});
    // dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 61, 2, CHAR_ARRAY{0x00, 0x6a});

    // dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 50, 4, CHAR_ARRAY{192,168,11,79});
    // dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 12, 7, CHAR_ARRAY{0x48, 0x65, 0x72, 0x74, 0x7A, 0x69, 0x67});
    
    // dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 54, 4, CHAR_ARRAY{192,168,11,1});
    // dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 145, 1, CHAR_ARRAY{0x01});

    // Rest of DHCP packet

    return dhcpPacket(buf, BROADCAST_ADDR, mac,
                      EMPTY_ADDR, EMPTY_ADDR,
                      EMPTY_ADDR, EMPTY_ADDR,
                      dhcpOptionBuf, dhcpOptionSize);
    //   return dhcpPacket(buf, BROADCAST_ADDR, mac,
    //   EMPTY_ADDR, requestip,
    //   CHAR_ARRAY{192,168,1,1}, EMPTY_ADDR,
    //   dhcpOptionBuf, dhcpOptionSize);
}

// DHCPLEASEQUERY dont use
short dhcpLeaseQuery(unsigned char *buf, unsigned char *mac)
{
    unsigned short dhcpOptionSize = 0;
    unsigned char dhcpOptionBuf[1500];
    memset(dhcpOptionBuf, 0, 1500);

    // DHCP options

    // DHCP Msg Type: DHCP_DISCOVER
    dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 0x35, 1, CHAR_ARRAY{10});

    dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 55, 1, CHAR_ARRAY{51});

    // Rest of DHCP packet

    // CHAR_ARRAY {0x08, 0x36, 0xC9, 0x8E, 0xFD, 0x0C}

    return dhcpPacket(buf, BROADCAST_ADDR, mac,
                      CHAR_ARRAY{192, 168, 1, 100}, EMPTY_ADDR,
                      EMPTY_ADDR, EMPTY_ADDR,
                      dhcpOptionBuf, dhcpOptionSize);

    // return dhcpPacket(buf, BROADCAST_ADDR, mac,
    //                   CHAR_ARRAY{192, 168, 11, 68}, EMPTY_ADDR,
    //                   EMPTY_ADDR, EMPTY_ADDR,
    //                   dhcpOptionBuf, dhcpOptionSize);
}

short dhcpRequest(unsigned char *buf, unsigned char *mac)
{
    unsigned short dhcpOptionSize = 0;
    unsigned char dhcpOptionBuf[1500];
    memset(dhcpOptionBuf, 0, 1500);

    // DHCP options

    // DHCP Msg Type: DHCP_DISCOVER
    dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 0x35, 1, CHAR_ARRAY{3});

    dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 50, 4, requestip);

    dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 61, 2, CHAR_ARRAY{0x00, 0x03});

    // 91 01 01

    // dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 0x37, 0x0e, CHAR_ARRAY{0x01, 0x79, 0x03, 0x06, 0x0C, 0x0F, 0x1A, 0x1C, 0x21, 0x33, 0x36, 0x3A, 0x3B, 0x77});

    //
    // 36 04 C0 A8 01 01

    // dhcpOptionSize += addDhcpOption(dhcpOptionBuf + dhcpOptionSize, 0x36, 0x04, CHAR_ARRAY{0xC0, 0xA8, 0x01, 0x01});

    // Rest of DHCP packet

    // return dhcpPacket(buf, BROADCAST_ADDR, mac,
    //                   CHAR_ARRAY{192,168,11,73}, EMPTY_ADDR,
    //                   EMPTY_ADDR, EMPTY_ADDR,
    //                   dhcpOptionBuf, dhcpOptionSize);

    return dhcpPacket(buf, BROADCAST_ADDR, mac,
                      EMPTY_ADDR, EMPTY_ADDR,
                      EMPTY_ADDR, EMPTY_ADDR,
                      dhcpOptionBuf, dhcpOptionSize);
}

int main()
{

    struct ifmacs *ifs = getifmacsWithIps();

    // prints all mac addresses of interfaces that have a private ip
    for (struct ifmacs *interface = ifs; interface; interface = interface->next)
    {
        for (int index = 0; index < ETHER_ADDR_LEN; index++)
        {
            printf("%02X ", interface->addr[index]);
        }
        printf("\n");
    }

    unsigned char buf[1500];
    memset(buf, 0, 1500);
    unsigned short bufSize = dhcpDiscover(buf, TMP_USE);
    // unsigned short bufSize = dhcpRequest(buf, TMP_USE);
    // unsigned short bufSize = dhcpLeaseQuery(buf, TMP_USE);

    // ethernet level socket, raw means header is expected, type ip(doesn't seem to really matter for sending)
    int sockfd;
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0)
    {
        perror("socket()");
        return 1;
    }

    // socket information
    struct sockaddr_ll socket_address;
    memset(&socket_address, 0, sizeof(struct sockaddr_ll));

    // socket_address.sll_ifindex = ifs->index;
    socket_address.sll_ifindex = if_nametoindex("enp0s31f6");

    // send buf
    if (sendto(sockfd, buf, bufSize, 0, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll)) < 0)
    {
        perror("sendto");
    }

    return 0;
}