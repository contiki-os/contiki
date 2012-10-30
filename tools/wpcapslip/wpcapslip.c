/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 */


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

uint16_t wpcap_poll(char **buf);

#include "net/tcpdump.h"
static int should_print = 0;

#define IP_HLEN 20

/*---------------------------------------------------------------------------*/
static void
print_packet(uint8_t *packet, int len)
{
  char buf[2000];
  if(should_print) {
    tcpdump_format(packet, len, buf, sizeof(buf));
    printf("%s\n", buf);
  }
}
/*---------------------------------------------------------------------------*/
void cleanup(void);
/*---------------------------------------------------------------------------*/
void
sigcleanup(int signo)
{
  fprintf(stderr, "signal %d\n", signo);
  exit(0);			/* exit(0) will call cleanup() */
}
/*---------------------------------------------------------------------------*/
#define SLIP_END     0300
#define SLIP_ESC     0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335

struct ip {
  u_int8_t ip_vhl;		/* version and header length */
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
  u_int16_t uh_sport;		/* source port */
  u_int16_t uh_dport;		/* destination port */
  u_int16_t uh_ulen;		/* udp length */
  u_int16_t uh_sum;		/* udp checksum */
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
  printf("%s\n", cmd);
  fflush(stdout);
  return system(cmd);
}

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


u_int16_t
ip4sum(u_int16_t sum, const void *_p, u_int16_t len)
{
  u_int16_t t;
  const u_int8_t *p = _p;
  const u_int8_t *end = p + len;

  while(p < (end-1)) {
    t = (p[0] << 8) + p[1];
    sum += t;
    if (sum < t) sum++;
    p += 2;
  }
  if(p < end) {
    t = (p[0] << 8) + 0;
    sum += t;
    if (sum < t) sum++;
  }
  return sum;
}

static uint16_t
chksum(const void *p, uint16_t len)
{
  uint16_t sum = ip4sum(0, p, len);
  return (sum == 0) ? 0xffff : uip_htons(sum);
}

int
check_ip(const struct ip *ip, unsigned ip_len)
{
  u_int16_t sum, ip_hl;
  
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
  ip_hl = 4*(ip->ip_vhl & IP_HL);
  sum = ip4sum(0, ip, ip_hl);
  if(sum != 0xffff && sum != 0x0) {
    return -4;
  }

  if(ip->ip_p == 6 || ip->ip_p == 17) {	/* Check TCP or UDP header. */
    u_int16_t tcp_len = ip_len - ip_hl;

    /* Sum pseudoheader. */
    sum = ip->ip_p + tcp_len; /* proto and len, no carry */
    sum = ip4sum(sum, &ip->ip_src, 8); /* src and dst */

    /* Sum TCP/UDP header and data. */
    sum = ip4sum(sum, (u_int8_t*)ip + ip_hl, tcp_len);
    
    /* Failed checksum test? */
    if(sum != 0xffff && sum != 0x0) {
      if(ip->ip_p == 6) {	/* TCP == 6 */
	return -5;
      } else {			/* UDP */
	/* Deal with disabled UDP checksums. */
	if(ip->uh_sum != 0) {
	  return -6;
	}
      }
    }
  } else if (ip->ip_p == 1) {	/* ICMP */
    u_int16_t icmp_len = ip_len - ip_hl;

    sum = ip4sum(0, (u_int8_t*)ip + ip_hl, icmp_len);
    if(sum != 0xffff && sum != 0x0) {
      return -7;
    }
  }
  return 0;
}

/*
 * Read from serial, when we have a packet write it to tun. No output
 * buffering, input buffered by stdio.
 */
void
serial_to_wpcap(FILE *inslip)
{
  static union {
    unsigned char inbuf[2000];
    struct ip iphdr;
  } uip;
  static int inbufptr = 0;

  int ret;
  unsigned char c;

#ifdef linux
  ret = fread(&c, 1, 1, inslip);
  if(ret == -1 || ret == 0) err(1, "serial_to_tun: read");
  goto after_fread;
#endif

 read_more:
  if(inbufptr >= sizeof(uip.inbuf)) {
    inbufptr = 0;
  }
  ret = fread(&c, 1, 1, inslip);
#ifdef linux
 after_fread:
#endif
  if(ret == -1) {
    err(1, "serial_to_tun: read");
  }
  if(ret == 0) {
    clearerr(inslip);
    return;
    fprintf(stderr, "serial_to_tun: EOF\n");
    exit(1);
  }
  /*  fprintf(stderr, ".");*/
  switch(c) {
  case SLIP_END:
    if(inbufptr > 0) {
      /*
       * Sanity checks.
       */
#define DEBUG_LINE_MARKER '\r'
      int ecode;
      ecode = check_ip(&uip.iphdr, inbufptr);
      if(ecode < 0 && inbufptr == 8 && strncmp(uip.inbuf, "=IPA", 4) == 0) {
	static struct in_addr ipa;

	inbufptr = 0;
	if(memcmp(&ipa, &uip.inbuf[4], sizeof(ipa)) == 0) {
	  break;
	}

	/* New address. */
	if(ipa.s_addr != 0) {
#if 0
#ifdef linux
	  ssystem("route delete -net %s netmask %s dev %s",
		  inet_ntoa(ipa), "255.255.255.255", tundev);
#else
	  ssystem("route delete -net %s -netmask %s -interface %s",
		  inet_ntoa(ipa), "255.255.255.255", tundev);
#endif
#endif /* 0 */
	}

	memcpy(&ipa, &uip.inbuf[4], sizeof(ipa));
	if(ipa.s_addr != 0) {
#if 0
#ifdef linux
	  ssystem("route add -net %s netmask %s dev %s",
		  inet_ntoa(ipa), "255.255.255.255", tundev);
#else
	  ssystem("route add -net %s -netmask %s -interface %s",
		  inet_ntoa(ipa), "255.255.255.255", tundev);
#endif
#endif /* 0 */
	}
	break;
      } else if(ecode < 0) {
	/*
	 * If sensible ASCII string, print it as debug info!
	 */
	/*	printf("----------------------------------\n");*/
	if(uip.inbuf[0] == DEBUG_LINE_MARKER) {
	  fwrite(uip.inbuf + 1, inbufptr - 1, 1, stderr);
	} else if(is_sensible_string(uip.inbuf, inbufptr)) {
	  fwrite(uip.inbuf, inbufptr, 1, stderr);
	} else {
	  fprintf(stderr,
		  "serial_to_tun: drop packet len=%d ecode=%d\n",
		  inbufptr, ecode);
	}
	inbufptr = 0;
	break;
      }
      PROGRESS("s");

#if 0
      if(dhsock != -1) {
	struct ip *ip = (void *)uip.inbuf;
	if(ip->ip_p == 17 && ip->ip_dst == 0xffffffff /* UDP and broadcast */
	    && ip->uh_sport == uip_ntohs(BOOTPC) && ip->uh_dport == uip_ntohs(BOOTPS)) {
	  relay_dhcp_to_server(ip, inbufptr);
	  inbufptr = 0;
	}
      }
#endif /* 0 */
      /*      if(write(outfd, uip.inbuf, inbufptr) != inbufptr) {
	err(1, "serial_to_tun: write");
	}*/
      /*      printf("Sending to wpcap\n");*/
      print_packet(uip.inbuf, inbufptr);
      wpcap_send(uip.inbuf, inbufptr);
      /*      printf("After sending to wpcap\n");*/
      inbufptr = 0;
    }
    break;

  case SLIP_ESC:
    if(fread(&c, 1, 1, inslip) != 1) {
      clearerr(inslip);
      /* Put ESC back and give up! */
      ungetc(SLIP_ESC, inslip);
      return;
    }

    switch(c) {
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

  goto read_more;
}
/*---------------------------------------------------------------------------*/
unsigned char slip_buf[2000];
int slip_end, slip_begin;
/*---------------------------------------------------------------------------*/
void
slip_send(int fd, unsigned char c)
{
  if(slip_end >= sizeof(slip_buf)) {
    err(1, "slip_send overflow");
  }
  slip_buf[slip_end] = c;
  slip_end++;
}
/*---------------------------------------------------------------------------*/
int
slip_empty()
{
  return slip_end == 0;
}
/*---------------------------------------------------------------------------*/
void
slip_flushbuf(int fd)
{
  int n;
  
  if (slip_empty())
    return;
  
  n = write(fd, slip_buf + slip_begin, (slip_end - slip_begin));

  if(n == -1 && errno != EAGAIN) {
    err(1, "slip_flushbuf write failed");
  } else if(n == -1) {
    PROGRESS("Q");		/* Outqueueis full! */
  } else {
    slip_begin += n;
    if(slip_begin == slip_end) {
      slip_begin = slip_end = 0;
    }
  }
}
/*---------------------------------------------------------------------------*/
void
write_to_serial(int outfd, void *inbuf, int len)
{
  u_int8_t *p = inbuf;
  int i, ecode;
  struct ip *iphdr = inbuf;

  /*
   * Sanity checks.
   */
  ecode = check_ip(inbuf, len);
  if(ecode < 0) {
    fprintf(stderr, "write_to_serial: drop packet %d\n", ecode);
    return;
  }

  if(iphdr->ip_id == 0 && iphdr->ip_off & IP_DF) {
    uint16_t nid = uip_htons(ip_id++);
    iphdr->ip_id = nid;
    nid = ~nid;			/* negate */
    iphdr->ip_sum += nid;	/* add */
    if(iphdr->ip_sum < nid) {	/* 1-complement overflow? */
      iphdr->ip_sum++;
    }
    ecode = check_ip(inbuf, len);
    if(ecode < 0) {
      fprintf(stderr, "write_to_serial: drop packet %d\n", ecode);
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
    fprintf(stderr, "write_to_serial: drop packet %d\n", ecode);
    return;
  }

  /* It would be ``nice'' to send a SLIP_END here but it's not
   * really necessary.
   */
  /* slip_send(outfd, SLIP_END); */

  for(i = 0; i < len; i++) {
    switch(p[i]) {
    case SLIP_END:
      slip_send(outfd, SLIP_ESC);
      slip_send(outfd, SLIP_ESC_END);
      break;
    case SLIP_ESC:
      slip_send(outfd, SLIP_ESC);
      slip_send(outfd, SLIP_ESC_ESC);
      break;
    default:
      slip_send(outfd, p[i]);
      break;
    }
    
  }
  slip_send(outfd, SLIP_END);
  /*  printf("slip end\n");*/
  PROGRESS("t");
}
/*---------------------------------------------------------------------------*/
/*
 * Read from tun, write to slip.
 */
#if 0
void
tun_to_serial(int infd, int outfd)
{
  static union {
    unsigned char inbuf[2000];
    struct ip iphdr;
  } uip;
  int size;

  if((size = read(infd, uip.inbuf, 2000)) == -1) {
    err(1, "tun_to_serial: read");
  }

  write_to_serial(outfd, uip.inbuf, size);
}
#endif /* 0 */
/*---------------------------------------------------------------------------*/
#ifndef BAUDRATE
#define BAUDRATE B115200
#endif
speed_t b_rate = BAUDRATE;

void
cfmakeraw(t)
        struct termios *t;
{

        t->c_iflag &= ~(IMAXBEL|IXOFF|INPCK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IGNPAR);
        t->c_iflag |= IGNBRK;
        t->c_oflag &= ~OPOST;
        t->c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL|ICANON|ISIG|IEXTEN|NOFLSH|TOSTOP);
        t->c_cflag &= ~(CSIZE|PARENB);
        t->c_cflag |= CS8|CREAD;
        t->c_cc[VMIN] = 1;
        t->c_cc[VTIME] = 0;
}

void
stty_telos(int fd)
{
  struct termios options;
  speed_t speed = b_rate;

  /*  if(fcntl(fd, F_SETFL, 0) < 0) {
    perror("could not set fcntl");
    exit(-1);
    }*/

  if(tcgetattr(fd, &options) < 0) {
    perror("could not get options");
    exit(-1);
  }

  cfsetispeed(&options, speed);
  cfsetospeed(&options, speed);
  /* Enable the receiver and set local mode */
  options.c_cflag |= (CLOCAL | CREAD);
  /* Mask the character size bits and turn off (odd) parity */
  options.c_cflag &= ~(CSIZE|PARENB|PARODD);
  /* Select 8 data bits */
  options.c_cflag |= CS8;

  /* Raw input */
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  /* Raw output */
  options.c_oflag &= ~OPOST;

  if (tcsetattr(fd, TCSANOW, &options) < 0) {
    perror("could not set options");
    exit(-1);
  }

#if 0
  struct termios tty;
  speed_t speed = b_rate;
  int i;

  if(tcflush(fd, TCIOFLUSH) == -1) err(1, "tcflush");

  if(tcgetattr(fd, &tty) == -1) err(1, "tcgetattr");

  cfmakeraw(&tty);

  /* Nonblocking read. */
  tty.c_cc[VTIME] = 0;
  tty.c_cc[VMIN] = 0;
  tty.c_cflag &= ~CRTSCTS;
  tty.c_cflag &= ~HUPCL;
  tty.c_cflag &= ~CLOCAL;

  cfsetispeed(&tty, speed);
  cfsetospeed(&tty, speed);

  if(tcsetattr(fd, TCSAFLUSH, &tty) == -1) err(1, "tcsetattr");

#if 1
  /* Nonblocking read and write. */
  /* if(fcntl(fd, F_SETFL, O_NONBLOCK) == -1) err(1, "fcntl"); */

  tty.c_cflag |= CLOCAL;
  if(tcsetattr(fd, TCSAFLUSH, &tty) == -1) err(1, "tcsetattr");

  i = TIOCM_DTR;
  if(ioctl(fd, TIOCMBIS, &i) == -1) err(1, "ioctl");
#endif

  usleep(10*1000);		/* Wait for hardware 10ms. */

  /* Flush input and output buffers. */
  if(tcflush(fd, TCIOFLUSH) == -1) err(1, "tcflush");
#endif /* 0 */
}
/*---------------------------------------------------------------------------*/
int
devopen(const char *dev, int flags)
{
  char t[32];
  strcpy(t, "/dev/");
  strcat(t, dev);
  return open(t, flags);
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
sigalarm_reset()
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
  int slipfd, maxfd;
  int ret;
  fd_set rset, wset;
  FILE *inslip;
  const char *siodev = NULL;
  const char *dhcp_server = NULL;
  /*  u_int16_t myport = BOOTPS, dhport = BOOTPS;*/
  int baudrate = -2;
  char buf[4000];
  int logging = 0;

  ip_id = getpid() * time(NULL);

  setvbuf(stdout, NULL, _IOLBF, 0); /* Line buffered output. */

  while((c = getopt(argc, argv, "B:D:hls:t:T")) != -1) {
    switch (c) {
    case 'B':
      baudrate = atoi(optarg);
      break;

    case 'D':
      dhcp_server = optarg;
      break;

    case 's':
      if(strncmp("/dev/", optarg, 5) == 0) {
	siodev = optarg + 5;
      } else {
	siodev = optarg;
      }
      break;

#if 0
    case 't':
      if(strncmp("/dev/", optarg, 5) == 0) {
	strcpy(tundev, optarg + 5);
      } else {
	strcpy(tundev, optarg);
      }
      break;
#endif /* 0 */

    case 'T':
      should_print = 1;
      break;

    case 'l':
      logging = 1;
      break;
      
    case '?':
    case 'h':
    default:
      err(1, "usage: wpcapslip [-B baudrate] [-s siodev] [-D dhcp-server] [-T] ipaddress netmask [dhcp-server]");
      break;
    }
  }
  argc -= (optind - 1);
  argv += (optind - 1);

  if(argc != 4) {
    err(1, "usage: wpcapslip [-s siodev] [-D dhcp-server] [-T] <IP address of local Ethernet card> <IP address of SLIP network> <netmask of SLIP network>");
  }
  /*  ipaddr = argv[1];
      netmask = argv[2];*/
  wpcap_start(argv[1], argv[2], argv[3], logging);
  /*  netaddr = inet_addr(ipaddr) & inet_addr(netmask);*/

  switch(baudrate) {
  case -2:
    break;			/* Use default. */
  case 9600:
    b_rate = B9600;
    break;
  case 19200:
    b_rate = B19200;
    break;
  case 38400:
    b_rate = B38400;
    break;
  case 57600:
    b_rate = B57600;
    break;
  case 115200:
    b_rate = B115200;
    break;
  default:
    err(1, "unknown baudrate %d", baudrate);
    break;
  }

  /*
   * Set up DHCP relay agent socket and find the address of this relay
   * agent.
   */
#if 0
  if(argc == 4) {
    dhcp_server = argv[3];
  }
  if(dhcp_server != NULL) {
    struct sockaddr_in myaddr;
    socklen_t len;
    in_addr_t a;

    if(strchr(dhcp_server, ':') != NULL) {
      dhport = atoi(strchr(dhcp_server, ':') + 1);
      myport = dhport + 1;
      *strchr(dhcp_server, ':') = '\0';
    }
    a = inet_addr(dhcp_server);
    if(a == -1) {
      err(1, "illegal dhcp-server address");
    }
#ifndef linux
    dhaddr.sin_len = sizeof(dhaddr);
#endif
    dhaddr.sin_family = AF_INET;
    dhaddr.sin_port = uip_htons(dhport);
    dhaddr.sin_addr.s_addr = a;

    dhsock = socket(AF_INET, SOCK_DGRAM, 0);
    if(dhsock < 0) {
      err (1, "socket");
    }
    memset(&myaddr, 0x0, sizeof(myaddr));
#ifndef linux
    myaddr.sin_len = sizeof(myaddr);
#endif
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = uip_htons(myport);
    if(bind(dhsock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
      err(1, "bind dhcp-relay");
    }

    if(connect(dhsock, (struct sockaddr *)&dhaddr, sizeof(dhaddr)) < 0) {
      err(1, "connect to dhcp-server");
    }

    len = sizeof(myaddr);
    if(getsockname(dhsock, (struct sockaddr *)&myaddr, &len) < 0) {
      err(1, "getsockname dhsock");
    }

    giaddr = myaddr.sin_addr.s_addr;

    /*
     * Don't want connected socket.
     */
    close(dhsock);
    dhsock = socket(AF_INET, SOCK_DGRAM, 0);
    if(dhsock < 0) {
      err (1, "socket");
    }
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = uip_htons(myport);
    if(bind(dhsock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
      err(1, "bind dhcp-relay");
    }
    fprintf(stderr, "DHCP server at %s:%d\n", dhcp_server, dhport);
  }
#endif /* 0 */
  
  if(siodev != NULL) {
    slipfd = devopen(siodev, O_RDWR | O_NONBLOCK | O_NOCTTY | O_NDELAY | O_DIRECT | O_SYNC );
    if(slipfd == -1) {
      err(1, "can't open siodev ``/dev/%s''", siodev);
    }
  } else {
    static const char *siodevs[] = {
      "ttyUSB0", "cuaU0", "ucom0" /* linux, fbsd6, fbsd5 */
    };
    int i;
    for(i = 0; i < 3; i++) {
      siodev = siodevs[i];
      slipfd = devopen(siodev, O_RDWR | O_NONBLOCK);
      if(slipfd != -1) {
	break;
      }
    }
    if(slipfd == -1) {
      err(1, "can't open siodev");
    }
  }
  fprintf(stderr, "slip started on ``/dev/%s''\n", siodev);
  stty_telos(slipfd);
  slip_send(slipfd, SLIP_END);
  inslip = fdopen(slipfd, "r");
  if(inslip == NULL) {
    err(1, "main: fdopen");
  }

  /*  tunfd = tun_alloc(tundev);
  if(tunfd == -1) err(1, "main: open");
  fprintf(stderr, "opened device ``/dev/%s''\n", tundev);*/

  atexit(cleanup);
  signal(SIGHUP, sigcleanup);
  signal(SIGTERM, sigcleanup);
  signal(SIGINT, sigcleanup);
  signal(SIGALRM, sigalarm);
  /*  ifconf(tundev, ipaddr, netmask);*/

  while(1) {
    maxfd = 0;
    FD_ZERO(&rset);
    FD_ZERO(&wset);

    if(got_sigalarm) {
      /* Send "?IPA". */
      slip_send(slipfd, '?');
      slip_send(slipfd, 'I');
      slip_send(slipfd, 'P');
      slip_send(slipfd, 'A');
      slip_send(slipfd, SLIP_END);
      got_sigalarm = 0;
    }

    if(!slip_empty()) {		/* Anything to flush? */
      FD_SET(slipfd, &wset);
    }

    FD_SET(slipfd, &rset);	/* Read from slip ASAP! */
    if(slipfd > maxfd) {
      maxfd = slipfd;
    }
    
    /* We only have one packet at a time queued for slip output. */
    if(slip_empty()) {
      /*      FD_SET(tunfd, &rset);
	      if(tunfd > maxfd) maxfd = tunfd;
      if(dhsock != -1) {
	FD_SET(dhsock, &rset);
	if(dhsock > maxfd) maxfd = dhsock;
	}*/
    }


    if(slip_empty()) {
      char *pbuf = buf;
      
      ret = wpcap_poll(&pbuf);
      if(ret > 0) {
	struct ip *iphdr = (struct ip *)pbuf;
	if(iphdr->ip_id != last_id) {
	  last_id = iphdr->ip_id;
	  /*	  printf("------ wpcap_poll ret %d\n", ret);*/
	  print_packet(pbuf, ret);
	  write_to_serial(slipfd, pbuf, ret);
	  slip_flushbuf(slipfd);
	  sigalarm_reset();
	}
      }
      /*    } else {
	    printf("!slip_empty\n");*/
    }
    {
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 10;
      ret = select(maxfd + 1, &rset, &wset, NULL, &tv);
    }
    if(ret == -1 && errno != EINTR) {
      err(1, "select");
    } else if(ret > 0) {
      if(FD_ISSET(slipfd, &rset)) {
	/*	printf("serial_to_wpcap\n");*/
        /*serial_to_tun(inslip, tunfd);*/
	serial_to_wpcap(inslip);
	/*	printf("End of serial_to_wpcap\n");*/
      }
      
      if(FD_ISSET(slipfd, &wset)) {
	/*	printf("slip_flushbuf\n");*/
	slip_flushbuf(slipfd);
	sigalarm_reset();
      }

      /*      if(slip_empty() && FD_ISSET(tunfd, &rset)) {
        tun_to_serial(tunfd, slipfd);
	slip_flushbuf(slipfd);
	sigalarm_reset();
	}*/

#if 0
      if(dhsock != -1 && slip_empty() && FD_ISSET(dhsock, &rset)) {
	relay_dhcp_to_client(slipfd);
	slip_flushbuf(slipfd);
      }
#endif /* 0 */
    }
  }
}
/*---------------------------------------------------------------------------*/
