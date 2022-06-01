#include "dhcpfinal.h"

uint16_t udp_checksum(struct udphdr *p_udp_header, size_t len, uint32_t src_addr, uint32_t dest_addr)
{
    const uint16_t *buf = (const uint16_t *)p_udp_header;
    uint16_t *ip_src = (void *)&src_addr, *ip_dst = (void *)&dest_addr;
    uint32_t sum;
    size_t length = len;

    // Calculate the sum
    sum = 0;
    while (len > 1)
    {
        sum += *buf++;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        len -= 2;
    }

    

    if (len & 1)
        // Add the padding if the packet lenght is odd
        sum += *((uint8_t *)buf);

    // Add the pseudo-header
    sum += *(ip_src++);
    sum += *ip_src;

    sum += *(ip_dst++);
    sum += *ip_dst;

    printf("%X\n", sum);

    sum += htons(IPPROTO_UDP);
    sum += htons(length);

    printf("%X\n", sum);

    // Add the carries
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    // Return the one's complement of sum
    return (uint16_t)~sum;
}

int main()
{
    unsigned char buf[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x8D, 0x5C, 0xE1, 0xD2, 0xD8, 0x08, 0x00, 0x45, 0x10, 0x01, 0x48, 0x00, 0x00, 0x00, 0x00, 0x80, 0x11, 0x39, 0x96, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x44, 0x00, 0x43, 0x01, 0x34, 0x8F, 0x8E, 0x01, 0x01, 0x06, 0x00, 0x34, 0x99, 0x09, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x8D, 0x5C, 0xE1, 0xD2, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x82, 0x53, 0x63, 0x35, 0x01, 0x03, 0x36, 0x4, 0xC0, 0xA8, 0xB, 0x1, 0x32, 0x4, 0xC0, 0xA8, 0xB, 0x43, 0x37, 0x7, 0x1, 0x1C, 0x2, 0x3, 0xF, 0x6, 0xC, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int length = 342;

    unsigned short bufSize = 0;

    struct ether_header *eth = (struct ether_header *)buf;
    bufSize += sizeof(struct ether_header);
    struct my_iphdr *ip = (struct my_iphdr *)(buf + bufSize);
    bufSize += sizeof(struct my_iphdr);
    struct udphdr *udp = (struct udphdr *)(buf + bufSize);
    bufSize += sizeof(struct udphdr);
    struct dhcpv4_hdr *dhcp = (struct dhcpv4_hdr *)(buf + bufSize);
    bufSize += sizeof(struct dhcpv4_hdr);

    unsigned short check = (ip->check);
    // ip->check = 0;

    { // ip checksum
        int sum = 0;
        unsigned short overflowMark = 0xffff;

        for (int index = 0; index < 10; index++)
        {
            sum += htons(((unsigned short *)ip)[index]);
        }
        while (sum > overflowMark)
        {
            sum = (sum & 0x0000ffff) + ((sum & 0xffff0000) >> 16);
        }
        // printf("%X\n", ~(unsigned short)sum & 0x0000ffff);
        // printf("%X\n", (~(unsigned short)sum & 0x0000ffff) + check);
        printf("%X\n", sum);
    }

    // udp_checksum(udp, htons(udp->len), ip->saddr, ip->daddr);
    printf("%X\n", udp->check);
    udp->check = 0;
    {
        unsigned long sum = 0;
        for (unsigned short *ptr = (unsigned short *)&ip->saddr; (unsigned char *  )ptr < buf + length; ptr++)
        {
            sum += *ptr;
        }

        sum+=htons(IPPROTO_UDP);
        sum+=udp->len;

        while(sum >> 16)
        {
            sum = (sum & 0xffff) + (sum >> 16);
        }
        printf("%X\n", ~sum&0xffff);
        udp->check = ~sum;
    }
    {
        unsigned long sum = 0;
        for (unsigned short *ptr = (unsigned short *)&ip->saddr; (unsigned char *  )ptr < buf + length; ptr++)
        {
            sum += *ptr;
        }

        sum+=htons(IPPROTO_UDP);
        sum+=udp->len;

        while(sum >> 16)
        {
            sum = (sum & 0xffff) + (sum >> 16);
        }
        printf("%X\n", sum);
    }

    return 0;
}