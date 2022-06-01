#include <sys/socket.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <ell/dhcp.h>
#include <memory.h>
#include <stdio.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <unistd.h>

#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
// #include <linux/if_arp.h>

#include <net/if.h>
#include <linux/if.h>

#include<signal.h>

#define ID 29845
#define DHCP_MAGIC 0x63825363
#define XID 0x19f05a95


#define ETHER_TEST_ADDR (unsigned char[ETHER_ADDR_LEN]){0x40, 0x8d, 0x53, 0xe1, 0xd2, 0xd2}
#define ETHER_WLAN0 (unsigned char[ETHER_ADDR_LEN]){0xc0, 0xb5, 0xd7, 0x09, 0xf6, 0xfa}
#define ETHER_ENP (unsigned char[ETHER_ADDR_LEN]){0x40, 0x8d, 0x5c, 0xe1, 0xd2, 0xd8}

#define requestip CHAR_ARRAY{192,168,1,105}

#define TMP_USE ETHER_ENP
#define REQ_ADDR (unsigned char[4]){10, 140, 141, 49}
#define EMPTY_ADDR (unsigned char[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define BROADCAST_ADDR (unsigned char[]){0xff, 0xff, 0xff, 0xff, 0xff, 0xff}

#define CHAR_ARRAY (unsigned char[])

struct dhcpv4_hdr
{
    uint8_t dhcp_opcode; /* opcode */
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
// #if __BYTE_ORDER == __LITTLE_ENDIAN
    // uint8_t flags : 3;
    uint16_t frag_offset : 16;
// #elif __BYTE_ORDER == __BIG_ENDIAN
//     uint8_t flags : 3;
//     uint16_t frag_off : 13;
// #else
// #error "Please fix <bits/endian.h>"
// #endif
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
    /*The options start here. */
// } __attribute__((packed));
};

struct ifmacs
{
    unsigned char addr[ETHER_ADDR_LEN];
    unsigned char name[IFNAMSIZ];
    unsigned int index;
    struct ifmacs* next;
};

struct dhcp_option
{
    unsigned char option;
    unsigned char length;
    unsigned char *data;
} __attribute__((packed));