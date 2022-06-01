#include <fcntl.h>     /* O_RDWR */
#include <string.h>    /* memset(), memcpy() */
#include <stdio.h>     /* perror(), printf(), fprintf() */
#include <stdlib.h>    /* exit(), malloc(), free() */
#include <sys/ioctl.h> /* ioctl() */
#include <unistd.h>    /* read(), close() */
#include <netdb.h>     // IPPROTO_UDP

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <assert.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <sys/ioctl.h>
// #include <netdb.h>
// #include <fcntl.h>
// #include <signal.h>
// #include <linux/if.h>
// #include <linux/if_tun.h>

#include <arpa/inet.h>

#include <iostream>
#include <algorithm>

/* includes for struct ifreq, etc */
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#define MTU 1400

int tun_open(char *devname)
{
    struct ifreq ifr;
    int fd, err;

    if ((fd = open("/dev/net/tun", O_RDWR)) == -1)
    {
        perror("open /dev/net/tun");
        exit(1);
    }
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    strncpy(ifr.ifr_name, devname, IFNAMSIZ); // devname = "tun0" or "tun1", etc

    /* ioctl will use ifr.if_name as the name of TUN
     * interface to open: "tun0", etc. */
    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) == -1)
    {
        perror("ioctl TUNSETIFF");
        close(fd);
        exit(1);
    }

    /* After the ioctl call the fd is "connected" to tun device specified
     * by devname ("tun0", "tun1", etc)*/

    return fd;
}

int main(int argc, char *argv[])
{
    int fd, nbytes;

    fd = tun_open("tap0");

    unsigned char tun_buf[MTU];
    bzero(tun_buf, MTU);

    system("ip link set tap0 up");
    system("ip addr add \"10.0.0.5\" dev tap0");
    system("ip route add defualt via 10.0.0.5 dev tap0");

    // system("ip link set tap0 up");
    // system("ip route add defualt dev tap0");

    int r;

    printf("Device tun0 opened\n");
    while (1)
    {
        r = read(fd, tun_buf, MTU);
        if (r < 0)
        {
            // TODO: ignore some errno
            perror("read from tun_fd error");
            break;
        }

        for (int index = 0; index < r; index++)
        {
            if (index == 34 || index == 42 || index == 14)
            // if (index == 8)
            {
                printf("| ");
            }
            printf("%x ", tun_buf[index]);
        }
        printf("\n");
        printf("source: %u.%u.%u.%u:%u\n", tun_buf[12+14], tun_buf[13+14], tun_buf[14+14], tun_buf[15+14], (tun_buf[20+14] << 8) + tun_buf[21+14]);
        printf("dest: %u.%u.%u.%u:%u\n", tun_buf[16+14], tun_buf[17+14], tun_buf[18+14], tun_buf[19+14], (tun_buf[22+14] << 8) + tun_buf[23+14]);
    }

    return 0;
}