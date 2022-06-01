#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <algorithm>

#define PORT 54345
#define MTU 1400

#define AS_CLIENT YES
#define SERVER_HOST ""

int udp_bind(struct sockaddr *addr, socklen_t *addrlen)
{
    struct addrinfo hints;
    struct addrinfo *result;
    int sock, flags;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    

    // const char *host = "";

    char host[256];
    gethostname(host, sizeof(host));

    if (0 != getaddrinfo(host, NULL, &hints, &result))
    {
        perror("getaddrinfo error");
        return -1;
    }

    ((struct sockaddr_in *)result->ai_addr)->sin_port = htons(PORT);

    memcpy(addr, result->ai_addr, result->ai_addrlen);
    *addrlen = result->ai_addrlen;

    if ((sock = socket(result->ai_family, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("Cannot create socket");
        freeaddrinfo(result);
        return -1;
    }

    freeaddrinfo(result);

    flags = fcntl(sock, F_GETFL, 0);
    if (flags != -1)
    {
        if (-1 != fcntl(sock, F_SETFL, flags | O_NONBLOCK))
            return sock;
    }
    perror("fcntl error");

    close(sock);
    return -1;
}

int tun_alloc()
{
    struct ifreq ifr;
    int fd, e;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
    {
        perror("Cannot open /dev/net/tun");
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, "tun0", IFNAMSIZ);

    if ((e = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0)
    {
        perror("ioctl[TUNSETIFF]");
        close(fd);
        return e;
    }

    return fd;
}

static void run(char *cmd)
{
    printf("Execute `%s`\n", cmd);
    if (system(cmd))
    {
        perror(cmd);
        exit(1);
    }
}

void setup_route_table()
{
    run("sysctl -w net.ipv4.ip_forward=1");

#ifdef AS_CLIENT
    run("iptables -t nat -A POSTROUTING -o tun0 -j MASQUERADE");
    run("iptables -I FORWARD 1 -i tun0 -m state --state RELATED,ESTABLISHED -j ACCEPT");
    run("iptables -I FORWARD 1 -o tun0 -j ACCEPT");
    // char cmd[1024];
    // snprintf(cmd, sizeof(cmd), "ip route add default via $(ip route show 0/0 | sed -e 's/.* via \([^ ]*\).*/\1/')", SERVER_HOST);
    // run(cmd);
    run("ip route add 0/1 dev tun0");
    run("ip route add 128/1 dev tun0");
#else
    run("iptables -t nat -A POSTROUTING -s 10.8.0.0/16 ! -d 10.8.0.0/16 -m comment --comment 'vpndemo' -j MASQUERADE");
    run("iptables -A FORWARD -s 10.8.0.0/16 -m state --state RELATED,ESTABLISHED -j ACCEPT");
    run("iptables -A FORWARD -d 10.8.0.0/16 -j ACCEPT");
#endif
}

/*
 * Cleanup route table
 */
void cleanup_route_table()
{
#ifdef AS_CLIENT
    run("iptables -t nat -D POSTROUTING -o tun0 -j MASQUERADE");
    run("iptables -D FORWARD -i tun0 -m state --state RELATED,ESTABLISHED -j ACCEPT");
    run("iptables -D FORWARD -o tun0 -j ACCEPT");
    // char cmd[1024];
    // snprintf(cmd, sizeof(cmd), "ip route del default", SERVER_HOST);
    // run(cmd);
    run("ip route del 0/1");
    run("ip route del 128/1");
#else
    run("iptables -t nat -D POSTROUTING -s 10.8.0.0/16 ! -d 10.8.0.0/16 -m comment --comment 'vpndemo' -j MASQUERADE");
    run("iptables -D FORWARD -s 10.8.0.0/16 -m state --state RELATED,ESTABLISHED -j ACCEPT");
    run("iptables -D FORWARD -d 10.8.0.0/16 -j ACCEPT");
#endif
}

void encrypt(char *plantext, char *ciphertext, int len) {
  memcpy(ciphertext, plantext, len);
}

void decrypt(char *ciphertext, char *plantext, int len) {
  memcpy(plantext, ciphertext, len);
}


int main()
{
    int tun_fd;
    if ((tun_fd = tun_alloc()) < 0)
    {
        return 1;
    }

    // setup_route_table();

    int udp_fd;
    struct sockaddr_storage client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    if ((udp_fd = udp_bind((struct sockaddr *)&client_addr, &client_addrlen)) < 0)
    {
        return 1;
    }

    char tun_buf[MTU], udp_buf[MTU];
    bzero(tun_buf, MTU);
    bzero(udp_buf, MTU);

    while (1)
    {
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(tun_fd, &readset);
        FD_SET(udp_fd, &readset);
        int max_fd = std::max(tun_fd, udp_fd) + 1;

        if (-1 == select(max_fd, &readset, NULL, NULL, NULL))
        {
            perror("select error");
            break;
        }

        int r;

        if (FD_ISSET(udp_fd, &readset))
        {
            r = recvfrom(udp_fd, udp_buf, MTU, 0, (struct sockaddr *)&client_addr, &client_addrlen);
            if (r < 0)
            {
                // TODO: ignore some errno
                perror("recvfrom udp_fd error");
                break;
            }

            // decrypt(udp_buf, tun_buf, r);
            printf("Writing to tun %d bytes ...\n", r);

            r = write(tun_fd, udp_buf, r);
            if (r < 0)
            {
                // TODO: ignore some errno
                perror("write tun_fd error");
                break;
            }
        }

        if (FD_ISSET(tun_fd, &readset))
        {
            r = read(tun_fd, tun_buf, MTU);
            if (r < 0)
            {
                // TODO: ignore some errno
                perror("read from tun_fd error");
                break;
            }

            // encrypt(tun_buf, udp_buf, r);
            printf("Writing to UDP %d bytes ...\n", r);

            r = sendto(udp_fd, tun_buf, r, 0, (const struct sockaddr *)&client_addr, client_addrlen);
            if (r < 0)
            {
                // TODO: ignore some errno
                perror("sendto udp_fd error");
                break;
            }
        }

        
    }

    close(tun_fd);
    close(udp_fd);

    // cleanup_route_table();

    return 0;
}