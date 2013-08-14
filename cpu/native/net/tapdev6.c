/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "net/uip.h"
#include "net/uipopt.h"

#if UIP_CONF_IPV6

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/socket.h>


#ifdef linux
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#define DEVTAP "/dev/net/tun"
#else  /* linux */
#define DEVTAP "/dev/tap0"
#endif /* linux */

#ifdef __APPLE__
#include <net/if.h>
#include <netinet/in.h>
#include <netinet6/in6_var.h>
#include <netinet6/nd6.h> // ND6_INFINITE_LIFETIME
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <net/if_dl.h> // struct sockaddr_dl
#include <net/route.h> // AF_ROUTE things
#endif

#include "tapdev6.h"
#include "contiki-net.h"

#define DROP 0

#if DROP
static int drop = 0;
#endif

static int fd = -1;

static unsigned long lasttime;

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#endif

static void do_send(void);
uint8_t tapdev_send(uip_lladdr_t *lladdr);

/*---------------------------------------------------------------------------*/
int
tapdev_fd(void)
{
  return fd;
}


uint16_t
tapdev_poll(void)
{
  fd_set fdset;
  struct timeval tv;
  int ret;
  
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  
  FD_ZERO(&fdset);
  if(fd > 0) {
    FD_SET(fd, &fdset);
  }

  ret = select(fd + 1, &fdset, NULL, NULL, &tv);

  if(ret == 0) {
    return 0;
  }
  ret = read(fd, uip_buf, UIP_BUFSIZE);

  PRINTF("tapdev6: read %d bytes (max %d)\n", ret, UIP_BUFSIZE);
  
  if(ret == -1) {
    perror("tapdev_poll: read");
  }
  return ret;
}
/*---------------------------------------------------------------------------*/
#if defined(__APPLE__)
static int reqfd = -1, sfd = -1, interface_index;

static void
tapdev_init_darwin_routes(void)
{
  struct stat st;

  if(-1 == fstat(fd, &st)) {
    perror("tapdev: fstat failed.");
    exit(EXIT_FAILURE);
  }

  /************* Add address *************/

  struct in6_aliasreq addreq6 = { };
  reqfd = socket(AF_INET6, SOCK_DGRAM, 0);

  if(-1 == fcntl(reqfd, F_SETFD, FD_CLOEXEC)) {
    perror("tapdev: fcntl failed.");
    exit(EXIT_FAILURE);
  }

  devname_r(st.st_rdev, S_IFCHR, addreq6.ifra_name,
            sizeof(addreq6.ifra_name));

  addreq6.ifra_addr.sin6_family = AF_INET6;
  addreq6.ifra_addr.sin6_len = sizeof(addreq6.ifra_addr);
  addreq6.ifra_addr.sin6_addr.__u6_addr.__u6_addr16[0] = UIP_HTONS(0xAAAA);
  addreq6.ifra_addr.sin6_addr.__u6_addr.__u6_addr16[7] = UIP_HTONS(0x0001);

  addreq6.ifra_prefixmask.sin6_family = AF_INET6;
  addreq6.ifra_prefixmask.sin6_len = sizeof(addreq6.ifra_prefixmask);
  addreq6.ifra_prefixmask.sin6_addr.__u6_addr.__u6_addr16[0] =
    UIP_HTONS(0xFFFF);
  addreq6.ifra_prefixmask.sin6_addr.__u6_addr.__u6_addr16[1] =
    UIP_HTONS(0xFFFF);
  addreq6.ifra_prefixmask.sin6_addr.__u6_addr.__u6_addr16[2] =
    UIP_HTONS(0xFFFF);
  addreq6.ifra_prefixmask.sin6_addr.__u6_addr.__u6_addr16[3] =
    UIP_HTONS(0xFFFF);

  addreq6.ifra_lifetime.ia6t_vltime = ND6_INFINITE_LIFETIME;
  addreq6.ifra_lifetime.ia6t_pltime = ND6_INFINITE_LIFETIME;
  addreq6.ifra_lifetime.ia6t_expire = ND6_INFINITE_LIFETIME;
  addreq6.ifra_lifetime.ia6t_preferred = ND6_INFINITE_LIFETIME;

  if(-1 == ioctl(reqfd, SIOCAIFADDR_IN6, &addreq6)) {
    perror("tapdev: Uable to add address, call to ioctl failed.");
    exit(EXIT_FAILURE);
  }

  /************* Add route *************/

  int s = socket(AF_ROUTE, SOCK_RAW, AF_INET6);

  if(s == -1) {
    perror("tapdev: Unable to add route, call to socket() failed.");

    // Failing to add the route is not fatal, so just return.
    return;
  }

  sfd = s;
  interface_index = if_nametoindex(devname(st.st_rdev, S_IFCHR));

  PRINTF("tapdev: if_nametoindex(devname(st.st_rdev, S_IFCHR)) = %d\n",
         interface_index);
  PRINTF("tapdev: devname(st.st_rdev, S_IFCHR) = %s\n",
         devname(st.st_rdev, S_IFCHR));

  struct {
    struct rt_msghdr hdr;
    struct sockaddr_in6 dst;
    struct sockaddr_dl gw;
    struct sockaddr_in6 mask;
  } msg = {};

  msg.hdr.rtm_msglen = sizeof(msg);
  msg.hdr.rtm_version = RTM_VERSION;
  msg.hdr.rtm_type = RTM_ADD;
  msg.hdr.rtm_index = interface_index;
  msg.hdr.rtm_flags = RTF_UP | RTF_STATIC;
  msg.hdr.rtm_addrs = RTA_DST | RTA_GATEWAY | RTA_NETMASK;
  msg.hdr.rtm_pid = getpid();
  msg.hdr.rtm_seq = 0;

  msg.dst.sin6_family = AF_INET6;
  msg.dst.sin6_len = sizeof(msg.dst);
  msg.dst.sin6_addr.__u6_addr.__u6_addr16[0] = UIP_HTONS(0xAAAA);

  msg.gw.sdl_family = AF_LINK;
  msg.gw.sdl_len = sizeof(msg.gw);
  msg.gw.sdl_index = interface_index;

  msg.mask.sin6_family = AF_INET6;
  msg.mask.sin6_len = sizeof(msg.mask);
  msg.mask.sin6_addr.__u6_addr.__u6_addr16[0] = UIP_HTONS(0xFFFF);
  msg.mask.sin6_addr.__u6_addr.__u6_addr16[1] = UIP_HTONS(0xFFFF);
  msg.mask.sin6_addr.__u6_addr.__u6_addr16[2] = UIP_HTONS(0xFFFF);
  msg.mask.sin6_addr.__u6_addr.__u6_addr16[3] = UIP_HTONS(0xFFFF);

  if(-1 == write(s, &msg, sizeof(msg))) {
    perror("tapdev: Unable to add route, call to write() failed.");

    // Failing to add the route is not fatal, so just return.
    return;
  }
}
/*---------------------------------------------------------------------------*/
static void
tapdev_cleanup_darwin_routes(void)
{
  struct {
    struct rt_msghdr hdr;
    struct sockaddr_in6 dst;
    struct sockaddr_dl gw;
    struct sockaddr_in6 mask;
  } msg = {};

  msg.hdr.rtm_msglen = sizeof(msg);
  msg.hdr.rtm_version = RTM_VERSION;
  msg.hdr.rtm_type = RTM_DELETE;
  msg.hdr.rtm_index = interface_index;
  msg.hdr.rtm_flags = RTF_UP | RTF_STATIC;
  msg.hdr.rtm_addrs = RTA_DST | RTA_GATEWAY | RTA_NETMASK;
  msg.hdr.rtm_pid = getpid();
  msg.hdr.rtm_seq = 0;

  msg.dst.sin6_family = AF_INET6;
  msg.dst.sin6_len = sizeof(msg.dst);
  msg.dst.sin6_addr.__u6_addr.__u6_addr16[0] = UIP_HTONS(0xAAAA);

  msg.gw.sdl_family = AF_LINK;
  msg.gw.sdl_len = sizeof(msg.gw);
  msg.gw.sdl_index = interface_index;

  msg.mask.sin6_family = AF_INET6;
  msg.mask.sin6_len = sizeof(msg.mask);
  msg.mask.sin6_addr.__u6_addr.__u6_addr16[0] = UIP_HTONS(0xFFFF);
  msg.mask.sin6_addr.__u6_addr.__u6_addr16[1] = UIP_HTONS(0xFFFF);
  msg.mask.sin6_addr.__u6_addr.__u6_addr16[2] = UIP_HTONS(0xFFFF);
  msg.mask.sin6_addr.__u6_addr.__u6_addr16[3] = UIP_HTONS(0xFFFF);
  if(-1 == write(sfd, &msg, sizeof(msg))) {
    perror("tapdev: Unable to delete route");
    exit(EXIT_FAILURE);
  }

  close(reqfd);
  close(sfd);
}
#endif // defined(__APPLE__)
/*---------------------------------------------------------------------------*/
void
tapdev_init(void)
{
  char buf[1024];
  
  fd = open(DEVTAP, O_RDWR);
  if(fd == -1) {
    perror("tapdev: tapdev_init: open");
    return;
  }

#ifdef linux
  {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP|IFF_NO_PI;
    if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
      perror(buf);
      exit(1);
    }
  }
#endif /* Linux */

#ifdef __APPLE__
  tapdev_init_darwin_routes();
#endif

  /* Linux (ubuntu)
     snprintf(buf, sizeof(buf), "ip link set tap0 up");
     system(buf);
     PRINTF("%s\n", buf);
     snprintf(buf, sizeof(buf), "ip -6 address add fc00::231/7 dev tap0");
     system(buf);
     PRINTF("%s\n", buf);
     snprintf(buf, sizeof(buf), "ip -6 route add fc00::0/7 dev tap0");
     system(buf);
     PRINTF("%s\n", buf);
  */
  /* freebsd */
  snprintf(buf, sizeof(buf), "ifconfig tap0 up");
  system(buf);
  printf("%s\n", buf);
  
  /*  */
  lasttime = 0;
  
  /*  gdk_input_add(fd, GDK_INPUT_READ,
      read_callback, NULL);*/

  atexit(&tapdev_exit);
}
/*---------------------------------------------------------------------------*/
static void
do_send(void)
{
  int ret;

  if(fd <= 0) {
    return;
  }

 
  PRINTF("tapdev_send: sending %d bytes\n", uip_len);
  /*  check_checksum(uip_buf, size);*/
#if DROP
  drop++;
  if(drop % 8 == 7) {
    PRINTF("Dropped an output packet!\n");
    return;
  }
#endif /* DROP */

  ret = write(fd, uip_buf, uip_len);

  if(ret == -1) {
    perror("tap_dev: tapdev_send: writev");
    exit(1);
  }
}
/*---------------------------------------------------------------------------*/
uint8_t tapdev_send(uip_lladdr_t *lladdr)
{
  /*
   * If L3 dest is multicast, build L2 multicast address
   * as per RFC 2464 section 7
   * else fill with th eaddrsess in argument
   */
  if(lladdr == NULL) {
    /* the dest must be multicast */
    (&BUF->dest)->addr[0] = 0x33;
    (&BUF->dest)->addr[1] = 0x33;
    (&BUF->dest)->addr[2] = IPBUF->destipaddr.u8[12];
    (&BUF->dest)->addr[3] = IPBUF->destipaddr.u8[13];
    (&BUF->dest)->addr[4] = IPBUF->destipaddr.u8[14];
    (&BUF->dest)->addr[5] = IPBUF->destipaddr.u8[15];
  } else {
    memcpy(&BUF->dest, lladdr, UIP_LLADDR_LEN);
  }
  memcpy(&BUF->src, &uip_lladdr, UIP_LLADDR_LEN);
  BUF->type = UIP_HTONS(UIP_ETHTYPE_IPV6); //math tmp
   
  uip_len += sizeof(struct uip_eth_hdr);
  do_send();
  return 0;
}

/*---------------------------------------------------------------------------*/
void
tapdev_do_send(void)
{
  do_send();
}
/*---------------------------------------------------------------------------*/
// math added function
void
tapdev_exit(void)
{
  PRINTF("tapdev: Closing...\n");

#ifdef __APPLE__
  tapdev_cleanup_darwin_routes();
#endif

  close(fd);
}
/*---------------------------------------------------------------------------*/

#endif /* UIP_CONF_IPV6 */
