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
 * $Id: tapdev.c,v 1.2 2007/03/22 18:59:34 adamdunkels Exp $
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

#include "net/uip.h"
#include "net/uip_arp.h"
#include "net/uip-fw.h"
#include "net/tapdev.h"

#include "net/tcpip.h"

#define DROP 0

static int drop = 0;
static int fd;

static unsigned long lasttime;
static struct timezone tz;

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

static void do_send(void);

u16_t
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

  snprintf(buf, sizeof(buf), "ifconfig tap0 inet 192.168.1.1");
  system(buf);
  snprintf(buf, sizeof(buf), "route add -net 172.16.0.0 192.168.1.2");
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
  char tmpbuf[UIP_BUFSIZE];
  int i;

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
  
  for(i = 0; i < 40 + UIP_LLH_LEN; i++) {
    tmpbuf[i] = uip_buf[i];
  }

  
  for(; i < uip_len; i++) {
    tmpbuf[i] = ((char *)uip_appdata)[i - 40 - UIP_LLH_LEN];
  }
  
  ret = write(fd, tmpbuf, uip_len);
  
  if(ret == -1) {
    perror("tap_dev: tapdev_send: writev");
    exit(1);
  }
}
/*---------------------------------------------------------------------------*/
u8_t
tapdev_send(void)
{
  uip_arp_out();

  do_send();
  uip_len -= UIP_LLH_LEN;
  return UIP_FW_OK;
}
/*---------------------------------------------------------------------------*/
void
tapdev_send_raw(void)
{
  do_send();
}
/*---------------------------------------------------------------------------*/
