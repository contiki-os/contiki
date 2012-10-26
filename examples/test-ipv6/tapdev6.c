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

#include "tapdev.h"

#include "contiki-net.h"

#define DROP 0

#if DROP
static int drop = 0;
#endif

static int fd;

static unsigned long lasttime;

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

static void do_send(void);
uint8_t tapdev_send(void);


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

  /*  printf("tapdev6: read %d bytes\n", ret);*/
  
  if(ret == -1) {
    perror("tapdev_poll: read");
  }
  return ret;
}
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

  snprintf(buf, sizeof(buf), "ifconfig tap0 inet6 fc00::123");
  system(buf);
  printf("%s\n", buf);
  snprintf(buf, sizeof(buf), "route add -inet6 fc00::0/64 -interface tap0");
  system(buf);
  printf("%s\n", buf);

  lasttime = 0;

  /*  gdk_input_add(fd, GDK_INPUT_READ,
      read_callback, NULL);*/

}
/*---------------------------------------------------------------------------*/
static void
do_send(void)
{
  int ret;

  if(fd <= 0) {
    return;
  }

 
  /*  printf("tapdev_send: sending %d bytes\n", size);*/
  /*  check_checksum(uip_buf, size);*/
#if DROP
  drop++;
  if(drop % 8 == 7) {
    printf("Dropped an output packet!\n");
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
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#define DEBUG_PRINT6ADDR(addr) DEBUG_PRINTF("%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
uint8_t
tapdev_send(void)
{
  struct uip_neighbor_addr *addr;
  /*  uip_arp_out();*/

  addr = uip_neighbor_lookup(&IPBUF->destipaddr);
  if(addr == NULL) {
    printf("tapdev6: tapdev_send: no matching neighbor found\n");
    DEBUG_PRINT6ADDR(&IPBUF->destipaddr);
    printf("\n");
  } else {
    memcpy(&BUF->dest, addr, 6);
    memcpy(&BUF->src, &uip_ethaddr, 6);
    uip_len += sizeof(struct uip_eth_hdr);
    do_send();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
tapdev_do_send(void)
{
  do_send();
}
/*---------------------------------------------------------------------------*/
