/*
 * Copyright (c) 2001, Adam Dunkels.
 * Copyright (c) 2009, 2010 Joakim Eriksson, Niclas Finne, Dogan Yazar.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

 /* Below define allows importing saved output into Wireshark as "Raw IP" packet type */
#define WIRESHARK_IMPORT_FORMAT 1
#include "contiki.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <err.h>

#include "net/netstack.h"
#include "net/packetbuf.h"
#include "cmd.h"
#include "border-router-cmds.h"

extern int slip_config_verbose;
extern int slip_config_flowcontrol;
extern const char *slip_config_siodev;
extern const char *slip_config_host;
extern const char *slip_config_port;
extern uint16_t slip_config_basedelay;
extern speed_t slip_config_b_rate;

#ifdef SLIP_DEV_CONF_SEND_DELAY
#define SEND_DELAY SLIP_DEV_CONF_SEND_DELAY
#else
#define SEND_DELAY 0
#endif

int devopen(const char *dev, int flags);

static FILE *inslip;

/* for statistics */
long slip_sent = 0;
long slip_received = 0;

int slipfd = 0;

//#define PROGRESS(s) fprintf(stderr, s)
#define PROGRESS(s) do { } while(0)

#define SLIP_END     0300
#define SLIP_ESC     0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335

/*---------------------------------------------------------------------------*/
static void *
get_in_addr(struct sockaddr *sa)
{
  if(sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
/*---------------------------------------------------------------------------*/
static int
connect_to_server(const char *host, const char *port)
{
  /* Setup TCP connection */
  struct addrinfo hints, *servinfo, *p;
  char s[INET6_ADDRSTRLEN];
  int rv, fd;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
    err(1, "getaddrinfo: %s", gai_strerror(rv));
    return -1;
  }

  /* loop through all the results and connect to the first we can */
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if(connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
      close(fd);
      perror("client: connect");
      continue;
    }
    break;
  }

  if(p == NULL) {
    err(1, "can't connect to ``%s:%s''", host, port);
    return -1;
  }

  fcntl(fd, F_SETFL, O_NONBLOCK);

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	    s, sizeof(s));

  /* all done with this structure */
  freeaddrinfo(servinfo);
  return fd;
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
void
slip_packet_input(unsigned char *data, int len)
{
  packetbuf_copyfrom(data, len);
  if(slip_config_verbose > 0) {
    printf("Packet input over SLIP: %d\n", len);
  }
  NETSTACK_RDC.input();
}
/*---------------------------------------------------------------------------*/
/*
 * Read from serial, when we have a packet call slip_packet_input. No output
 * buffering, input buffered by stdio.
 */
void
serial_input(FILE *inslip)
{
  unsigned char inbuf[2048];
  static int inbufptr = 0;
  int ret,i;
  unsigned char c;

#ifdef linux
  ret = fread(&c, 1, 1, inslip);
  if(ret == -1 || ret == 0) err(1, "serial_input: read");
  goto after_fread;
#endif

 read_more:
  if(inbufptr >= sizeof(inbuf)) {
     fprintf(stderr, "*** dropping large %d byte packet\n", inbufptr);
     inbufptr = 0;
  }
  ret = fread(&c, 1, 1, inslip);
#ifdef linux
 after_fread:
#endif
  if(ret == -1) {
    err(1, "serial_input: read");
  }
  if(ret == 0) {
    clearerr(inslip);
    return;
  }
  slip_received++;
  switch(c) {
  case SLIP_END:
    if(inbufptr > 0) {
      if(inbuf[0] == '!') {
	command_context = CMD_CONTEXT_RADIO;
	cmd_input(inbuf, inbufptr);
      } else if(inbuf[0] == '?') {
#define DEBUG_LINE_MARKER '\r'
      } else if(inbuf[0] == DEBUG_LINE_MARKER) {
	fwrite(inbuf + 1, inbufptr - 1, 1, stdout);
      } else if(is_sensible_string(inbuf, inbufptr)) {
        if(slip_config_verbose == 1) {   /* strings already echoed below for verbose>1 */
          fwrite(inbuf, inbufptr, 1, stdout);
        }
      } else {
        if(slip_config_verbose > 2) {
          printf("Packet from SLIP of length %d - write TUN\n", inbufptr);
          if(slip_config_verbose > 4) {
#if WIRESHARK_IMPORT_FORMAT
            printf("0000");
	    for(i = 0; i < inbufptr; i++) printf(" %02x", inbuf[i]);
#else
            printf("         ");
            for(i = 0; i < inbufptr; i++) {
              printf("%02x", inbuf[i]);
              if((i & 3) == 3) printf(" ");
              if((i & 15) == 15) printf("\n         ");
            }
#endif
            printf("\n");
          }
        }
	slip_packet_input(inbuf, inbufptr);
      }
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
    inbuf[inbufptr++] = c;

    /* Echo lines as they are received for verbose=2,3,5+ */
    /* Echo all printable characters for verbose==4 */
    if(slip_config_verbose == 4) {
      if(c == 0 || c == '\r' || c == '\n' || c == '\t' || (c >= ' ' && c <= '~')) {
	fwrite(&c, 1, 1, stdout);
      }
    } else if(slip_config_verbose >= 2) {
      if(c == '\n' && is_sensible_string(inbuf, inbufptr)) {
        fwrite(inbuf, inbufptr, 1, stdout);
        inbufptr = 0;
      }
    }
    break;
  }

  goto read_more;
}

unsigned char slip_buf[2048];
int slip_end, slip_begin, slip_packet_end, slip_packet_count;
static struct timer send_delay_timer;
/* delay between slip packets */
static clock_time_t send_delay = SEND_DELAY;
/*---------------------------------------------------------------------------*/
static void
slip_send(int fd, unsigned char c)
{
  if(slip_end >= sizeof(slip_buf)) {
    err(1, "slip_send overflow");
  }
  slip_buf[slip_end] = c;
  slip_end++;
  slip_sent++;
  if(c == SLIP_END) {
    /* Full packet received. */
    slip_packet_count++;
    if(slip_packet_end == 0) {
      slip_packet_end = slip_end;
    }
  }
}
/*---------------------------------------------------------------------------*/
int
slip_empty()
{
  return slip_packet_end == 0;
}
/*---------------------------------------------------------------------------*/
void
slip_flushbuf(int fd)
{
  int n;

  if(slip_empty()) {
    return;
  }

  n = write(fd, slip_buf + slip_begin, slip_packet_end - slip_begin);

  if(n == -1 && errno != EAGAIN) {
    err(1, "slip_flushbuf write failed");
  } else if(n == -1) {
    PROGRESS("Q");		/* Outqueue is full! */
  } else {
    slip_begin += n;
    if(slip_begin == slip_packet_end) {
      slip_packet_count--;
      if(slip_end > slip_packet_end) {
        memcpy(slip_buf, slip_buf + slip_packet_end,
               slip_end - slip_packet_end);
      }
      slip_end -= slip_packet_end;
      slip_begin = slip_packet_end = 0;
      if(slip_end > 0) {
        /* Find end of next slip packet */
        for(n = 1; n < slip_end; n++) {
          if(slip_buf[n] == SLIP_END) {
            slip_packet_end = n + 1;
            break;
          }
        }
        /* a delay between slip packets to avoid losing data */
        if(send_delay > 0) {
          timer_set(&send_delay_timer, send_delay);
        }
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
write_to_serial(int outfd, const uint8_t *inbuf, int len)
{
  const uint8_t *p = inbuf;
  int i;

  if(slip_config_verbose > 2) {
#ifdef __CYGWIN__
    printf("Packet from WPCAP of length %d - write SLIP\n", len);
#else
    printf("Packet from TUN of length %d - write SLIP\n", len);
#endif
    if(slip_config_verbose > 4) {
#if WIRESHARK_IMPORT_FORMAT
      printf("0000");
      for(i = 0; i < len; i++) printf(" %02x", p[i]);
#else
      printf("         ");
      for(i = 0; i < len; i++) {
        printf("%02x", p[i]);
        if((i & 3) == 3) printf(" ");
        if((i & 15) == 15) printf("\n         ");
      }
#endif
      printf("\n");
    }
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
  PROGRESS("t");
}
/*---------------------------------------------------------------------------*/
/* writes an 802.15.4 packet to slip-radio */
void
write_to_slip(const uint8_t *buf, int len)
{
  if(slipfd > 0) {
    write_to_serial(slipfd, buf, len);
  }
}
/*---------------------------------------------------------------------------*/
static void
stty_telos(int fd)
{
  struct termios tty;
  speed_t speed = slip_config_b_rate;
  int i;

  if(tcflush(fd, TCIOFLUSH) == -1) err(1, "tcflush");

  if(tcgetattr(fd, &tty) == -1) err(1, "tcgetattr");

  cfmakeraw(&tty);

  /* Nonblocking read. */
  tty.c_cc[VTIME] = 0;
  tty.c_cc[VMIN] = 0;
  if(slip_config_flowcontrol) {
    tty.c_cflag |= CRTSCTS;
  } else {
    tty.c_cflag &= ~CRTSCTS;
  }
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
}
/*---------------------------------------------------------------------------*/
static int
set_fd(fd_set *rset, fd_set *wset)
{
  /* Anything to flush? */
  if(!slip_empty() && (send_delay == 0 || timer_expired(&send_delay_timer))) {
    FD_SET(slipfd, wset);
  }

  FD_SET(slipfd, rset);	/* Read from slip ASAP! */
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
handle_fd(fd_set *rset, fd_set *wset)
{
  if(FD_ISSET(slipfd, rset)) {
    serial_input(inslip);
  }

  if(FD_ISSET(slipfd, wset)) {
    slip_flushbuf(slipfd);
  }
}
/*---------------------------------------------------------------------------*/
static const struct select_callback slip_callback = { set_fd, handle_fd };
/*---------------------------------------------------------------------------*/
void
slip_init(void)
{
  setvbuf(stdout, NULL, _IOLBF, 0); /* Line buffered output. */

  if(slip_config_host != NULL) {
    if(slip_config_port == NULL) {
      slip_config_port = "60001";
    }
    slipfd = connect_to_server(slip_config_host, slip_config_port);
    if(slipfd == -1) {
      err(1, "can't connect to ``%s:%s''", slip_config_host, slip_config_port);
    }

  } else if(slip_config_siodev != NULL) {
    if(strcmp(slip_config_siodev, "null") == 0) {
      /* Disable slip */
      return;
    }
    slipfd = devopen(slip_config_siodev, O_RDWR | O_NONBLOCK);
    if(slipfd == -1) {
      err(1, "can't open siodev ``/dev/%s''", slip_config_siodev);
    }

  } else {
    static const char *siodevs[] = {
      "ttyUSB0", "cuaU0", "ucom0" /* linux, fbsd6, fbsd5 */
    };
    int i;
    for(i = 0; i < 3; i++) {
      slip_config_siodev = siodevs[i];
      slipfd = devopen(slip_config_siodev, O_RDWR | O_NONBLOCK);
      if(slipfd != -1) {
	break;
      }
    }
    if(slipfd == -1) {
      err(1, "can't open siodev");
    }
  }

  select_set_callback(slipfd, &slip_callback);

  if(slip_config_host != NULL) {
    fprintf(stderr, "********SLIP opened to ``%s:%s''\n", slip_config_host,
	    slip_config_port);
  } else {
    fprintf(stderr, "********SLIP started on ``/dev/%s''\n", slip_config_siodev);
    stty_telos(slipfd);
  }

  timer_set(&send_delay_timer, 0);
  slip_send(slipfd, SLIP_END);
  inslip = fdopen(slipfd, "r");
  if(inslip == NULL) {
    err(1, "main: fdopen");
  }
}
/*---------------------------------------------------------------------------*/
