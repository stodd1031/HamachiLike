// #include <linux/if.h>
#include <net/if.h>
#include <linux/if_addr.h>
#include <sys/socket.h>
// #include <sys/ioctl.h>
#include <arpa/inet.h>
// #include <net/if.h>
// #include <string.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <memory.h>
#include <stdio.h>

struct
{
    struct nlmsghdr nlm;
    struct ifaddrmsg ifamsg;
    struct rtattr rta1;
    unsigned int ip1;
    struct rtattr rta2;
    __u32 ip2;
    struct rtattr rta3;
    __u32 ip3;
    struct rtattr rta4;
    struct ifa_cacheinfo cache;
    // struct rtattr rta5;
    // __u32 ip5;
    
}io;

struct nlattr fjsablce;
int main()
{

    struct rtattr t;

    // memset(&ifamsg, 0, sizeof(ifamsg));
    int rtnetlink_socket = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);

    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));

    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    msg.msg_name = &addr;

    // bind (rtnetlink_socket, (const struct sockaddr * )&addr, 12);

    msg.msg_namelen = sizeof(addr);

    struct iovec iov;

    io.nlm.nlmsg_len = sizeof(io);
    io.nlm.nlmsg_type = RTM_DELADDR; // NEW DEL GET
    // removed NLM_F_ACK for testing
    io.nlm.nlmsg_flags = NLM_F_REQUEST  | NLM_F_EXCL | NLM_F_CREATE;
    io.nlm.nlmsg_seq = 0;
    io.nlm.nlmsg_pid = 0;

    io.ifamsg.ifa_family = AF_INET;
    io.ifamsg.ifa_prefixlen = 0;
    io.ifamsg.ifa_flags = 0;
    io.ifamsg.ifa_scope = RT_SCOPE_UNIVERSE;
    // io.ifamsg.ifa_index = if_nametoindex("enp0s31f6");
    io.ifamsg.ifa_index = if_nametoindex("wlan0");

    io.rta1.rta_len = 8;
    io.rta1.rta_type = IFA_LOCAL; // add IFA_BROADCAST
    io.ip1 = inet_addr("1.1.1.1");

    io.rta2.rta_len = 8;
    io.rta2.rta_type = IFA_FLAGS;
    io.ip2 = IFA_F_NOPREFIXROUTE;

    io.rta3.rta_len = 8;
    io.rta3.rta_type = IFA_ADDRESS;
    io.ip3 = inet_addr("172.217.15.110");

    io.rta4.rta_len = 20;
    io.rta4.rta_type = IFA_CACHEINFO;
    io.cache.ifa_prefered = 6000; // becomes deprecated after this timer is out; 0 for always
                               // deprecated ips aren't used for sending packets
    io.cache.ifa_valid = 6000; // seconds for life of ip
    // io.cache.cstamp = time(NULL); // doesn't matter?
    // io.cache.tstamp = time(NULL); // doesn't matter?

    // io.rta3.rta_len = 8;
    // io.rta3.rta_type = IFA_BROADCAST;
    // io.ip3 = inet_addr("10.140.159.255");


    iov.iov_base = &io;
    iov.iov_len = sizeof(io);

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    msg.msg_controllen = 0;
    msg.msg_flags = 0;

    char * ptr = (char*)&io;

    for (int i = 0; i < sizeof(io); i++)
	{
		printf("%X ", ptr[i]);
	}

    sendmsg(rtnetlink_socket, &msg, 0);
}