#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <string.h>
#include <linux/if.h>
#include <net/if.h>
#include <linux/if_addr.h>


int main(int argc, const char *argv[]) {
    struct ifreq ifr;
    const char * name = "enp6s0";
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;

    strncpy(ifr.ifr_name, name, IFNAMSIZ);

    ifr.ifr_addr.sa_family = AF_INET;
    inet_pton(AF_INET, "10.12.0.2", &(addr->sin_addr));
    // addr->sin_addr.s_addr = inet_addr("10.12.0.1");
    ioctl(fd, SIOCSIFADDR, &ifr);

    

    // sendmsg(rtnetlink_socket, )
    

    // inet_pton(AF_INET, "10.12.0.3", &(addr->sin_addr));
    // ioctl(fd, SIOCAIFADDR, &ifr);

    // struct ifaliasreq ifra;

    // inet_pton(AF_INET, "255.255.0.0", &addr->sin_addr);
    // ioctl(fd, SIOCSIFNETMASK, &ifr);

    ioctl(fd, SIOCGIFFLAGS, &ifr);
    strncpy(ifr.ifr_name, name, IFNAMSIZ);
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);


    ioctl(fd, SIOCSIFFLAGS, &ifr);

    return 0;
}