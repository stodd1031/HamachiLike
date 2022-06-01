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
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
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
  char buf[1600];

  fd = tun_open("tun0"); /* devname = ifr.if_name = "tun0" */
  // system("ip addr add 10.0.3.0 dev tun0");
  int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  // int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
  // int sock_fd = socket(AF_INET, SOCK_RAW, IP_HDRINCL);

  int on = 1;
  int off = 0;
  if (setsockopt(sock_fd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
  {
      perror("opt: ");
      exit(1);
  }

  // IPPROTO_RAW
  // 10.140.140.28/19

  char tun_buf[MTU], udp_buf[MTU];
  bzero(tun_buf, MTU);
  bzero(udp_buf, MTU);

  int r;

  sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(client_addr));
  // client_addr.sin_port = htons(54345);
  // client_addr.sin_family = AF_INET;
  // client_addr.sin_addr.s_addr = inet_addr("172.253.124.101");
  client_addr.sin_addr.s_addr = 1;

  socklen_t client_addrlen = sizeof(client_addr);

  sockaddr_in my_addr;
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_port = htons(54345);
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = inet_addr("10.140.149.187");
  // my_addr.sin_addr.s_addr = INADDR_ANY;

  bind(sock_fd, (const sockaddr *)&my_addr, sizeof(my_addr));

  bool first = 0;


  printf("Device tun0 opened\n");
  while (1)
  {
    // nbytes = read(fd, buf, sizeof(buf));
    // printf("Read %d bytes from tun0\n", nbytes);

    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(fd, &readset);
    FD_SET(sock_fd, &readset);
    int max_fd = std::max(fd, sock_fd);

    if (-1 == select(max_fd, &readset, NULL, NULL, NULL))
    {
      perror("select error");
      break;
    }

    if (FD_ISSET(fd, &readset))
    {
      r = read(fd, tun_buf, MTU);
      if (r < 0)
      {
        // TODO: ignore some errno
        perror("read from tun_fd error");
        break;
      }

      unsigned char meaddr[4] = {10, 140, 141, 138};
      // if (strncmp(tun_buf+12, (const char *)meaddr, 4)==0 && first == 0)
      // {
        for (int i = 0; i < r; ++i)
        std::cout << +(unsigned char)(tun_buf[i]) << " ";
      std::cout << std::endl;

      // return 0;

      printf("Writing to udp %d bytes ...\n", r);
      // }
      first = 1;
      

      // memcpy(&client_addr.sin_addr.s_addr, tun_buf + 16, 4);
      // r = sendto(sock_fd, tun_buf, r, 0, (const struct sockaddr *)&client_addr, client_addrlen);
      // r = write(sock_fd, tun_buf, r);
    }

    if (FD_ISSET(sock_fd, &readset))
    {
      r =  recvfrom(sock_fd, udp_buf, MTU, 0, (struct sockaddr *)&client_addr, &client_addrlen);
      // r = read(sock_fd, udp_buf, MTU);
      if (r < 0)
      {
        // TODO: ignore some errno
        perror("recvfrom udp_fd error");
        break;
      }

      for (int i = 0; i < r; ++i)
        std::cout << +(unsigned char)(udp_buf[i]) << " ";
      std::cout << std::endl;

      printf("Writing to tun %hu bytes ...\n", r);

      r = write(fd, udp_buf, r);
      if (r < 0)
      {
        // TODO: ignore some errno
        perror("write tun_fd error");
        break;
      }
    }
  }

  return 0;
}