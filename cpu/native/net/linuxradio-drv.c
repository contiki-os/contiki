/*
 * Copyright (c) 2013, Google
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
 * Author: Vladimir Pouzanov <farcaller@gmail.com>
 *
 */

#include "contiki.h"
#include "contiki-conf.h"

#if defined(linux) && NETSTACK_CONF_WITH_IPV6

#include "linuxradio-drv.h"

#include "net/packetbuf.h"
#include "net/netstack.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <linux/sockios.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static int sockfd = -1;
static char *sockbuf;
static int buflen;

#define MAX_PACKET_SIZE 256

static int
init(void)
{
  sockbuf = malloc(MAX_PACKET_SIZE);
  if(sockbuf == 0) {
    return 1;
  }
  return 0;
}
static int
prepare(const void *payload, unsigned short payload_len)
{
  if(payload_len > MAX_PACKET_SIZE) {
    return 0;
  }
  memcpy(sockbuf, payload, payload_len);
  buflen = payload_len;

  return 0;
}
static int
transmit(unsigned short transmit_len)
{
  int sent = 0;
  sent = send(sockfd, sockbuf, buflen, 0);
  if(sent < 0) {
    perror("linuxradio send()");
    return RADIO_TX_ERR;
  }
  buflen = 0;
  return RADIO_TX_OK;
}
static int
my_send(const void *payload, unsigned short payload_len)
{
  int ret = -1;

  if(prepare(payload, payload_len)) {
    return ret;
  }

  ret = transmit(payload_len);

  return ret;
}
static int
my_read(void *buf, unsigned short buf_len)
{
  return 0;
}
static int
channel_clear(void)
{
  return 1;
}
static int
receiving_packet(void)
{
  return 0;
}
static int
pending_packet(void)
{
  return 0;
}
static int
set_fd(fd_set *rset, fd_set *wset)
{
  FD_SET(sockfd, rset);
  return 1;
}
static void
handle_fd(fd_set *rset, fd_set *wset)
{
  if(FD_ISSET(sockfd, rset)) {
    int bytes = read(sockfd, sockbuf, MAX_PACKET_SIZE);
    buflen = bytes;
    memcpy(packetbuf_dataptr(), sockbuf, bytes);
    packetbuf_set_datalen(bytes);
    NETSTACK_RDC.input();
  }
}

static const struct select_callback linuxradio_sock_callback = { set_fd, handle_fd };

static int
on(void)
{
  struct ifreq ifr;
  int err;
  struct sockaddr_ll sll;

  sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IEEE802154));
  if(sockfd < 0) {
    perror("linuxradio socket()");
    return 0;
  } else {
    strncpy((char *)ifr.ifr_name, NETSTACK_CONF_LINUXRADIO_DEV, IFNAMSIZ);
    err = ioctl(sockfd, SIOCGIFINDEX, &ifr);
    if(err == -1) {
      perror("linuxradio ioctl()");
      return 0;
    }
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_IEEE802154);

    if(bind(sockfd, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
      perror("linuxradio bind()");
      return 0;
    }

    select_set_callback(sockfd, &linuxradio_sock_callback);
    return 1;
  }
}
static int
off(void)
{
  close(sockfd);
  sockfd = -1;
  return 1;
}
const struct radio_driver linuxradio_driver =
{
  init,
  prepare,
  transmit,
  my_send,
  my_read,
  channel_clear,
  receiving_packet,
  pending_packet,
  on,
  off,
};

#endif
