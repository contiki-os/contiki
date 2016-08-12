/*
 * Copyright (c) 2012, Thingsquare, www.thingsquare.com.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
 * This file is part of the Contiki operating system.
 *
 * Author: Fredrik Osterlind <fredrik@thingsquare.com>
 * Based on wpcapslip.c by : Oliver Schmidt <ol.sc@web.de>
 */

/* This is a stripped-down version of wpcapslip: Instead of reading from and writing
 * to a serial port, this program just reads and writes to stdin/stdout. To
 * achieve the same functionality as wpcapslip, this program can hence be connected
 * to serialdump. But, in contrast to wpcapslip, this program can easily be connected
 * to networked or simulated serial ports. -- Fredrik, 2012 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __CYGWIN__
#include <alloca.h>
#else /* __CYGWIN__ */
#include <malloc.h>
#endif /* __CYGWIN__ */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/select.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <err.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define PROGRESS(x)

void wpcap_start(char *ethifaddr, char *netaddr, char *netmask, int logging);

void wpcap_send(void *buf, int len);
void raw_send(void *buf, int len);

uint16_t wpcap_poll(char **buf, int eth);

#include "net/tcpdump.h"
static int should_print = 0;

static int send_eth = 0; /* Sends ethernet frames or IP packets */

#define IP_HLEN 20

/*---------------------------------------------------------------------------*/
uint16_t
uip_htons(uint16_t val)
{
  return UIP_HTONS(val);
}
/*---------------------------------------------------------------------------*/
uint32_t
uip_htonl(uint32_t val)
{
  return UIP_HTONL(val);
}
/*---------------------------------------------------------------------------*/
static void
print_packet(char* prefix, uint8_t *packet, int len)
{
  char buf[2000];
  if(should_print) {
    tcpdump_format(packet, len, buf, sizeof(buf));
    fprintf(stderr, "%s: %s\n", prefix, buf);
  }
}
/*---------------------------------------------------------------------------*/
void cleanup(void);
/*---------------------------------------------------------------------------*/
void
sigcleanup(int signo)
{
  fprintf(stderr, "signal %d\n", signo);
  exit(0);      /* exit(0) will call cleanup() */
}
/*---------------------------------------------------------------------------*/
#define SLIP_END     0300
#define SLIP_ESC     0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335

struct ip {
  u_int8_t ip_vhl;    /* version and header length */
#define IP_V4 0x40
#define IP_V  0xf0
#define IP_HL 0x0f
  u_int8_t ip_tos;                    /* type of service */
  u_int16_t ip_len;                     /* total length */
  u_int16_t ip_id;                      /* identification */
  u_int16_t ip_off;                     /* fragment offset field */
#define IP_RF 0x8000                    /* reserved fragment flag */
#define IP_DF 0x4000                    /* dont fragment flag */
#define IP_MF 0x2000                    /* more fragments flag */
#define IP_OFFMASK 0x1fff               /* mask for fragmenting bits */
  u_int8_t ip_ttl;                    /* time to live */
  u_int8_t ip_p;                      /* protocol */
  u_int16_t ip_sum;                     /* checksum */
  u_int32_t ip_src, ip_dst;      /* source and dest address */
  u_int16_t uh_sport;   /* source port */
  u_int16_t uh_dport;   /* destination port */
  u_int16_t uh_ulen;    /* udp length */
  u_int16_t uh_sum;   /* udp checksum */
};

static int ip_id, last_id;

int
ssystem(const char *fmt, ...) __attribute__((__format__ (__printf__, 1, 2)));

int
ssystem(const char *fmt, ...)
{
  char cmd[128];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(cmd, sizeof(cmd), fmt, ap);
  va_end(ap);
  fprintf(stderr, "%s\n", cmd);
  fflush(stderr);
  return system(cmd);
}
/*---------------------------------------------------------------------------*/
int
is_sensible_string(const unsigned char *s, int len)
{
  int i;
  for(i = 1; i < len; i++) {
    if(s[i] == 0 || s[i] == '\r' || s[i] == '\n' || s[i] == '\t') {
      continue;
    } else if(s[i] < ' ' || '~' < s[i]) {
      return 0;
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
u_int16_t
ip4sum(u_int16_t sum, const void *_p, u_int16_t len)
{
  u_int16_t t;
  const u_int8_t *p = _p;
  const u_int8_t *end = p + len;

  while(p < (end - 1)) {
    t = (p[0] << 8) + p[1];
    sum += t;
    if(sum < t)
      sum++;
    p += 2;
  }
  if(p < end) {
    t = (p[0] << 8) + 0;
    sum += t;
    if(sum < t)
      sum++;
  }
  return sum;
}
/*---------------------------------------------------------------------------*/
static uint16_t
chksum(const void *p, uint16_t len)
{
  uint16_t sum = ip4sum(0, p, len);
  return (sum == 0) ? 0xffff : uip_htons(sum);
}
/*---------------------------------------------------------------------------*/
int
check_ip(const struct ip *ip, unsigned ip_len)
{
  u_int16_t sum, ip_hl;

  if(send_eth) {
    return 0;
  }

  /* Check IP version and length. */
  if((ip->ip_vhl & IP_V) != IP_V4) {
    return -1;
  }

  if(uip_ntohs(ip->ip_len) > ip_len) {
    return -2;
  }

  if(uip_ntohs(ip->ip_len) < ip_len) {
    return -3;
  }

  /* Check IP header. */
  ip_hl = 4 * (ip->ip_vhl & IP_HL);
  sum = ip4sum(0, ip, ip_hl);
  if(sum != 0xffff && sum != 0x0) {
    return -4;
  }

  if(ip->ip_p == 6 || ip->ip_p == 17) { /* Check TCP or UDP header. */
    u_int16_t tcp_len = ip_len - ip_hl;

    /* Sum pseudoheader. */
    sum = ip->ip_p + tcp_len; /* proto and len, no carry */
    sum = ip4sum(sum, &ip->ip_src, 8); /* src and dst */

    /* Sum TCP/UDP header and data. */
    sum = ip4sum(sum, (u_int8_t*)ip + ip_hl, tcp_len);

    /* Failed checksum test? */
    if(sum != 0xffff && sum != 0x0) {
      if(ip->ip_p == 6) { /* TCP == 6 */
        return -5;
      } else { /* UDP */
        /* Deal with disabled UDP checksums. */
        if(ip->uh_sum != 0) {
          return -6;
        }
      }
    }
  } else if(ip->ip_p == 1) { /* ICMP */
    u_int16_t icmp_len = ip_len - ip_hl;

    sum = ip4sum(0, (u_int8_t*)ip + ip_hl, icmp_len);
    if(sum != 0xffff && sum != 0x0) {
      return -7;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/*
 * Read a single character from stdin. When we have a full packet, write it to
 * the network interface.
 */
void
serial_to_wpcap(void)
{
  static union {
    unsigned char inbuf[2000];
    struct ip iphdr;
  } uip;
  static int inbufptr = 0;

  int ret;
  unsigned char c;

  if(inbufptr >= sizeof(uip.inbuf)) {
    inbufptr = 0;
    return;
  }
  ret = read(STDIN_FILENO, &c, 1);

  if(ret <= 0) {
    err(1, "serial_to_wpcap: read");
    inbufptr = 0;
    return;
  }

  switch (c) {
  case SLIP_END:
    if(inbufptr > 0) {
      /*
       * Sanity checks.
       */
#define DEBUG_LINE_MARKER '\r'
      int ecode;

      ecode = check_ip(&uip.iphdr, inbufptr);
      if(ecode < 0 && inbufptr == 8
          && strncmp((const char*)uip.inbuf, "=IPA", 4) == 0) {
        static struct in_addr ipa;

        inbufptr = 0;
        if(memcmp(&ipa, &uip.inbuf[4], sizeof(ipa)) == 0) {
          break;
        }

        memcpy(&ipa, &uip.inbuf[4], sizeof(ipa));
        break;
      } else if(ecode < 0) {
        /*
         * If sensible ASCII string, print it as debug info!
         */
        /*  printf("----------------------------------\n");*/
        if(uip.inbuf[0] == DEBUG_LINE_MARKER) {
          fwrite(uip.inbuf + 1, inbufptr - 1, 1, stderr);
        } else if(is_sensible_string(uip.inbuf, inbufptr)) {
          fwrite(uip.inbuf, inbufptr, 1, stderr);
        } else {
          fprintf(stderr, "serial_to_wpcap: drop packet len=%d ecode=%d\n",
                  inbufptr, ecode);
        }
        inbufptr = 0;
        break;
      }
      PROGRESS("s");

      if(send_eth) {
        raw_send(uip.inbuf, inbufptr);
      } else {
        /*      printf("Sending to wpcap\n");*/
        if(uip.iphdr.ip_id != last_id) {
          last_id = uip.iphdr.ip_id;
          print_packet("to wpcap: ", uip.inbuf, inbufptr);
          wpcap_send(uip.inbuf, inbufptr);
        } else {
          /*print_packet("IGNORED to wpcap: ", uip.inbuf, inbufptr);*/
        }
      }
      /*      printf("After sending to wpcap\n");*/
      inbufptr = 0;
    }
    break;

  case SLIP_ESC:
    /* TODO We do not actually check that we have another byte incoming, so
     * we may block here */
    read(STDIN_FILENO, &c, 1);

    switch (c) {
    case SLIP_ESC_END:
      c = SLIP_END;
      break;
    case SLIP_ESC_ESC:
      c = SLIP_ESC;
      break;
    }

    /* FALLTHROUGH */
    default:
      uip.inbuf[inbufptr++] = c;
      break;
  }
}
/*---------------------------------------------------------------------------*/
unsigned char stdout_buf[2000];
int stdout_buf_cnt;
/*---------------------------------------------------------------------------*/
void
stdout_write(unsigned char c)
{
  if(stdout_buf_cnt >= sizeof(stdout_buf)) {
    err(1, "stdout_write overflow");
  }
  stdout_buf[stdout_buf_cnt] = c;
  stdout_buf_cnt++;
}
/*---------------------------------------------------------------------------*/
int
stdout_buf_empty(void)
{
  return stdout_buf_cnt == 0;
}
/*---------------------------------------------------------------------------*/
void
stdout_flushbuf(void)
{
  if(stdout_buf_empty()) {
    return;
  }

  fwrite(stdout_buf, stdout_buf_cnt, 1, stdout);
  stdout_buf_cnt = 0;
  fflush(stdout);
}
/*---------------------------------------------------------------------------*/
void
write_slip_stdout(void *inbuf, int len)
{
  u_int8_t *p = inbuf;
  int i, ecode;
  struct ip *iphdr = inbuf;

  if(!send_eth) {
  /*
   * Sanity checks.
   */
  /*fprintf(stderr, "write_slip_stdout: %d\n", len);*/
  ecode = check_ip(inbuf, len);
  if(ecode < 0) {
    fprintf(stderr, "write_slip_stdout: drop packet %d\n", ecode);
    return;
  }

  if(iphdr->ip_id == 0 && iphdr->ip_off & IP_DF) {
    uint16_t nid = uip_htons(ip_id++);
    iphdr->ip_id = nid;
    nid = ~nid; /* negate */
    iphdr->ip_sum += nid; /* add */
    if(iphdr->ip_sum < nid) { /* 1-complement overflow? */
      iphdr->ip_sum++;
    }
    ecode = check_ip(inbuf, len);
    if(ecode < 0) {
      fprintf(stderr, "write_slip_stdout: drop packet %d\n", ecode);
      return;
    }
  }

  iphdr->ip_ttl = uip_htons(uip_htons(iphdr->ip_ttl) + 1);
  if(iphdr->ip_ttl == 0) {
    fprintf(stderr, "Packet with ttl %d dropped\n", iphdr->ip_ttl);
    return;
  }
  iphdr->ip_sum = 0;
  iphdr->ip_sum = ~chksum(iphdr, 4 * (iphdr->ip_vhl & IP_HL));
  ecode = check_ip(inbuf, len);
  if(ecode < 0) {
    fprintf(stderr, "write_slip_stdout: drop packet %d\n", ecode);
    return;
  }
  }

  for(i = 0; i < len; i++) {
    switch (p[i]) {
    case SLIP_END:
      stdout_write(SLIP_ESC);
      stdout_write(SLIP_ESC_END);
      break;
    case SLIP_ESC:
      stdout_write(SLIP_ESC);
      stdout_write(SLIP_ESC_ESC);
      break;
    default:
      stdout_write(p[i]);
      break;
    }
  }
  stdout_write(SLIP_END);
  /*  printf("slip end\n");*/
  PROGRESS("t");
}
/*---------------------------------------------------------------------------*/
/*const char *ipaddr;*/
/*const char *netmask;*/
static int got_sigalarm;
void
sigalarm(int signo)
{
  got_sigalarm = 1;
  return;
}
/*---------------------------------------------------------------------------*/
void
sigalarm_reset(void)
{
#ifdef linux
#define TIMEOUT (997*1000)
#else
#define TIMEOUT (2451*1000)
#endif
  ualarm(TIMEOUT, TIMEOUT);
  got_sigalarm = 0;
}
/*---------------------------------------------------------------------------*/
int
main(int argc, char **argv)
{
  int c;
  int ret;
  char buf[4000];
  int logging = 0;

  ip_id = getpid() * time(NULL);

  while((c = getopt(argc, argv, "E:D:hl:t:T")) != -1) {
    switch (c) {
    case 'E':
      send_eth = 1;
      break;

    case 'T':
      should_print = 1;
      break;

    case 'l':
      logging = 1;
      break;

    case '?':
    case 'h':
    default:
      err(1,
          "usage: wpcapstdio [-E] [-l] [-T] <IP address of local Ethernet card> <IP address of SLIP network> <netmask of SLIP network>");
      break;
    }
  }
  argc -= (optind - 1);
  argv += (optind - 1);

  if(argc != 4) {
    err(1, "usage: wpcapstdio [-E] [-T] <IP address of local Ethernet card> <IP address of SLIP network> <netmask of SLIP network>");
  }

  wpcap_start(argv[1], argv[2], argv[3], logging);
  stdout_write(SLIP_END);

  atexit(cleanup);
  signal(SIGHUP, sigcleanup);
  signal(SIGTERM, sigcleanup);
  signal(SIGINT, sigcleanup);
  signal(SIGALRM, sigalarm);

  while(1) {
    if(got_sigalarm) {
      /* Send "?IPA". */
      stdout_write('?');
      stdout_write('I');
      stdout_write('P');
      stdout_write('A');
      stdout_write(SLIP_END);
      got_sigalarm = 0;
    }

    if(stdout_buf_empty()) {
      char *pbuf = buf;

      ret = wpcap_poll(&pbuf, send_eth);
      if(ret > 0) {
        if(send_eth) {
          write_slip_stdout(pbuf, ret);
          stdout_flushbuf();
        } else {
          struct ip *iphdr = (struct ip *)pbuf;
          if(iphdr->ip_id != last_id) {
            /*last_id = iphdr->ip_id;*/
            print_packet("to stdout: ", (uint8_t*)pbuf, ret);
            write_slip_stdout(pbuf, ret);
            stdout_flushbuf();
          } else {
            /*print_packet("IGNORED to stdout: ", (uint8_t*)pbuf, ret);*/
          }
        }
      }
    }

    if(!stdout_buf_empty()) {
      stdout_flushbuf();
    }

    {
      fd_set s_rd;
      struct timeval tv;

      tv.tv_sec = 0;
      tv.tv_usec = 100;

      do {
        FD_ZERO(&s_rd);
        FD_SET(fileno(stdin), &s_rd);
        select(fileno(stdin) + 1, &s_rd, NULL, NULL, &tv);
        if(FD_ISSET(fileno(stdin), &s_rd)) {
          serial_to_wpcap();
        }
      } while(FD_ISSET(fileno(stdin), &s_rd));
    }
  }
}
/*---------------------------------------------------------------------------*/
