/*
 * Copyright (c) 2001, Adam Dunkels.
 * Copyright (c) 2009, Joakim Eriksson, Niclas Finne.
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
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: tapslip6.c,v 1.1 2010/10/25 10:42:41 salvopitru Exp $
 */

 /**
 * \file
 *         A driver for the uip6-bridge customized for STM32W. Thanks to
 *         Adam Dunkels, Joakim Eriksson, Niclas Finne for the original
 *         code.
 * \author
 *         Salvatore Pitrulli <salvopitru@users.sourceforge.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>

int ssystem(const char *fmt, ...)
    __attribute__ ((__format__(__printf__, 1, 2)));
void write_to_serial(void *inbuf, int len);

#define PRINTF(...) if(verbose)printf(__VA_ARGS__)

//#define PROGRESS(s) fprintf(stderr, s)
#define PROGRESS(s) do { } while (0)

#define USAGE_STRING "usage: tapslip6 [-B baudrate] [-s siodev] [-t tundev] [-a ipaddress[/prefixlen]|-p 64bit-prefix[/prefixlen]] [-c channel] [-r] [-v] [-h]"
#define HELP_STRING "usage: tapslip6 [-B baudrate] [-s siodev] [-t tundev] [-a ipaddress[/prefixlen]|-p 64bit-prefix[/prefixlen]] [-c channel] [-r] [-v] [-h]\r\n\n\
Options:\r\n\
-B baudrate\tBaudrate of the serial port (default:115200).\r\n\
-s siodev\tDevice that identifies the bridge (default: the first available\r\n\
\t\tamong ttyUSB0, cuaU0 and ucom0)\r\n\
-t tundev\tThe virtual network interface (default: tap0)\r\n\
-c channel\t802.15.4 radio channel.\r\n\
-r\t\tSet sniffer mode. \r\n\
-a ipaddress/[prefixlen]  The address to be assigned to the virtual network\r\n\
\t\tadapter.\r\n\
-p 64bit-prefix\tAutomatic assignment of the IPv6 address from the specified\r\n\
\t\tsubnet prefix. It may be followed by the prefix length\r\n\
\t\tfor the on-link determination.\r\n\
-v\t\tVerbose. Print more infos.\r\n\
-h\t\tShow this help.\r\n"

typedef enum {
    false = 0,
    true = 1,
} bool;

char tundev[32] = { "tap0" };

const char *ipaddr = NULL;
char *ipprefix = NULL;
char autoconf_addr[44] = { 0 };

bool autoconf = false;
bool verbose = false;

struct uip_eth_addr {
    uint8_t addr[6];
};
struct uip_802154_longaddr {
    uint8_t addr[8];
};

static struct uip_eth_addr eth_addr;
const struct uip_eth_addr eth_addr_null = { {0} };

static int request_mac = 1;
static int send_mac = 1;
static int set_sniffer_mode = 1;
static int set_channel = 1;

static int sniffer_mode = 0;
static int channel = 0;


int ssystem(const char *fmt, ...)
    __attribute__ ((__format__(__printf__, 1, 2)));

int ssystem(const char *fmt, ...)
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

#define SLIP_END     0300
#define SLIP_ESC     0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335

static void print_packet(u_int8_t * p, int len)
{
    int i;
    for (i = 0; i < len; i++) {
	printf("%02x", p[i]);
	if ((i & 3) == 3)
	    printf(" ");
	if ((i & 15) == 15)
	    printf("\n");
    }
    printf("\n");
}

int is_sensible_string(const unsigned char *s, int len)
{
    int i;
    for (i = 1; i < len; i++) {
	if (s[i] == 0 || s[i] == '\r' || s[i] == '\n' || s[i] == '\t') {
	    continue;
	}
	else if (s[i] < ' ' || '~' < s[i]) {
	    return 0;
	}
    }
    return 1;
}



/*
 * Read from serial, when we have a packet write it to tun. No output
 * buffering, input buffered by stdio.
 */
void serial_to_tun(FILE * inslip, int outfd)
{
    static union {
	unsigned char inbuf[2000];
    } uip;
    static int inbufptr = 0;

    int ret;
    unsigned char c;

#ifdef linux
    ret = fread(&c, 1, 1, inslip);
    if (ret == -1 || ret == 0)
	err(1, "serial_to_tun: read");
    goto after_fread;
#endif

  read_more:
    if (inbufptr >= sizeof(uip.inbuf)) {
	inbufptr = 0;
    }
    ret = fread(&c, 1, 1, inslip);
#ifdef linux
  after_fread:
#endif
    if (ret == -1) {
	err(1, "serial_to_tun: read");
    }
    if (ret == 0) {
	clearerr(inslip);
	return;
	fprintf(stderr, "serial_to_tun: EOF\n");
	exit(1);
    }
    /*  fprintf(stderr, "."); */
    switch (c) {
      case SLIP_END:
	  if (inbufptr > 0) {
	      if (uip.inbuf[0] == '!') {
		  if (uip.inbuf[1] == 'M') {
		      /* Read gateway MAC address and autoconfigure tap0 interface */
		      char macs[24];
		      int i, pos;
		      struct uip_802154_longaddr dev_addr = { {0} };	// Bridge EUI-64.

		      for (i = 0, pos = 0; i < 16; i++) {
			  macs[pos++] = uip.inbuf[2 + i];
			  if ((i & 1) == 1 && i < 14) {
			      macs[pos++] = ':';
			  }
		      }
		      macs[pos] = '\0';
		      printf("*** Gateway's MAC address: %s\n", macs);
		      
		      {
		    	  
		              int addr_bytes[8]; // sscanf requires int instead of 8-bit for hexadecimal variables.
		              
		              sscanf(macs, "%2X:%2X:%2X:%2X:%2X:%2X:%2X:%2X",
						 &addr_bytes[0],
						 &addr_bytes[1],
						 &addr_bytes[2],
						 &addr_bytes[3],
						 &addr_bytes[4],
						 &addr_bytes[5],
						 &addr_bytes[6],
						 &addr_bytes[7]);
				     
				     for(i=0;i<8;i++){
				     	dev_addr.addr[i] = addr_bytes[i];
				     }      
		      }		      

		      ////////////////////////////////////////
		      /*PRINTF("MAC:\n");
		         for (i = 0; i < 8; i++)
		         PRINTF("%02X ", dev_addr.addr[i]);
		         PRINTF("\n"); */
		      ////////////////////////////////////////

		      // Remember what will be our MAC address (two middle bytes will be elided).
		      memcpy(&eth_addr, &dev_addr, 3);
		      memcpy(&eth_addr.addr[3], &dev_addr.addr[5], 3);

		      if (autoconf) {

			  struct in6_addr ipv6addr;

			  dev_addr.addr[0] |= 0x02;

			  strtok(ipprefix, "/");

			  if (inet_pton(AF_INET6, ipprefix, &ipv6addr) != 1) {
			      printf("Invalid IPv6 address.\n");
			      exit(1);
			  }

			  // Copy modified EUI-64 to the last 64 bits of IPv6 address.
			  memcpy(&ipv6addr.s6_addr[8], &dev_addr, 8);

			  inet_ntop(AF_INET6, &ipv6addr, autoconf_addr, INET6_ADDRSTRLEN);	// To string format.

			  char *substr = strtok(NULL, "/");
			  if (substr != NULL) {	// Add the prefix length.
			      strcat(autoconf_addr, "/");
			      strcat(autoconf_addr, substr);
			  }
			  ipaddr = autoconf_addr;

		      }
		      ssystem("ifconfig %s down", tundev);


		      strcpy(&macs[9], &macs[15]);	// Elide two middle bytes.
		      ssystem("ifconfig %s hw ether %s", tundev, macs);
		      ssystem("ifconfig %s inet6 up", tundev);
		      if(ipaddr){
		      	ssystem("ifconfig %s inet6 add %s", tundev, ipaddr);
		      	ssystem("sysctl -w net.ipv6.conf.all.forwarding=1");
		      }
		      ssystem("ifconfig %s\n", tundev);

		  }
#define DEBUG_LINE_MARKER '\r'
	      }
	      else if (uip.inbuf[0] == '?') {
		  if (uip.inbuf[1] == 'M'
		      && memcmp(&eth_addr, &eth_addr_null, 6) != 0) {
		      /* Send our MAC address and other configuration options. */		      
		      send_mac = 1;
		      set_sniffer_mode = 1;
		      set_channel = 1;
		  }
	      }
	      else if (uip.inbuf[0] == DEBUG_LINE_MARKER) {
		  fwrite(uip.inbuf + 1, inbufptr - 1, 1, stdout);
	      }
	      else if (is_sensible_string(uip.inbuf, inbufptr)) {
		  fwrite(uip.inbuf, inbufptr, 1, stdout);
	      }
	      else {
		  PRINTF("Writing to tun  len: %d\n", inbufptr);
		  /*      print_packet(uip.inbuf, inbufptr); */
		  if (write(outfd, uip.inbuf, inbufptr) != inbufptr) {
		      err(1, "serial_to_tun: write");
		  }
	      }
	      inbufptr = 0;
	  }
	  break;

      case SLIP_ESC:
	  if (fread(&c, 1, 1, inslip) != 1) {
	      clearerr(inslip);
	      /* Put ESC back and give up! */
	      ungetc(SLIP_ESC, inslip);
	      return;
	  }

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

    goto read_more;
}

unsigned char slip_buf[2000];
int slip_end, slip_begin;


void slip_send(unsigned char c)
{
    if (slip_end >= sizeof(slip_buf))
	err(1, "slip_send overflow");
    slip_buf[slip_end] = c;
    slip_end++;
}

int slip_empty()
{
    return slip_end == 0;
}

void slip_flushbuf(int fd)
{
    int n;

    if (slip_empty())
	return;

    n = write(fd, slip_buf + slip_begin, (slip_end - slip_begin));

    if (n == -1 && errno != EAGAIN) {
	err(1, "slip_flushbuf write failed");
    }
    else if (n == -1) {
	PROGRESS("Q");		/* Outqueueis full! */
    }
    else {
	slip_begin += n;
	if (slip_begin == slip_end) {
	    slip_begin = slip_end = 0;
	}
    }
}

void write_to_serial(void *inbuf, int len)
{
    u_int8_t *p = inbuf;
    int i;

    /*  printf("Got packet of length %d - write SLIP\n", len); */
    /*  print_packet(p, len); */

    /* It would be ``nice'' to send a SLIP_END here but it's not
     * really necessary.
     */
    /* slip_send(outfd, SLIP_END); */
    PRINTF("Writing to serial  len: %d\n", len);
    for (i = 0; i < len; i++) {
	switch (p[i]) {
	  case SLIP_END:
	      slip_send(SLIP_ESC);
	      slip_send(SLIP_ESC_END);
	      break;
	  case SLIP_ESC:
	      slip_send(SLIP_ESC);
	      slip_send(SLIP_ESC_ESC);
	      break;
	  default:
	      slip_send(p[i]);
	      break;
	}

    }
    slip_send(SLIP_END);
    PROGRESS("t");
}


/*
 * Read from tun, write to slip.
 */
void tun_to_serial(int infd, int outfd)
{
    struct {
	unsigned char inbuf[2000];
    } uip;
    int size;

    if ((size = read(infd, uip.inbuf, 2000)) == -1)
	err(1, "tun_to_serial: read");

    write_to_serial(uip.inbuf, size);
}

#ifndef BAUDRATE
#define BAUDRATE B115200
#endif
speed_t b_rate = BAUDRATE;

void stty_telos(int fd)
{
    struct termios tty;
    speed_t speed = b_rate;
    int i;

    if (tcflush(fd, TCIOFLUSH) == -1)
	err(1, "tcflush");

    if (tcgetattr(fd, &tty) == -1)
	err(1, "tcgetattr");

    cfmakeraw(&tty);

    /* Nonblocking read. */
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag &= ~HUPCL;
    tty.c_cflag &= ~CLOCAL;

    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    if (tcsetattr(fd, TCSAFLUSH, &tty) == -1)
	err(1, "tcsetattr");

#if 1
    /* Nonblocking read and write. */
    /* if(fcntl(fd, F_SETFL, O_NONBLOCK) == -1) err(1, "fcntl"); */

    tty.c_cflag |= CLOCAL;
    if (tcsetattr(fd, TCSAFLUSH, &tty) == -1)
	err(1, "tcsetattr");

    i = TIOCM_DTR;
    if (ioctl(fd, TIOCMBIS, &i) == -1)
	err(1, "ioctl");
#endif

    usleep(10 * 1000);		/* Wait for hardware 10ms. */

    /* Flush input and output buffers. */
    if (tcflush(fd, TCIOFLUSH) == -1)
	err(1, "tcflush");
}

int devopen(const char *dev, int flags)
{
    char t[32];
    strcpy(t, "/dev/");
    strcat(t, dev);
    return open(t, flags);
}

#ifdef linux
#include <linux/if.h>
#include <linux/if_tun.h>

int tun_alloc(char *dev)
{
    struct ifreq ifr;
    int fd, err;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
	return -1;

    memset(&ifr, 0, sizeof(ifr));

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (*dev != 0)
	strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
	close(fd);
	return err;
    }
    strcpy(dev, ifr.ifr_name);
    return fd;
}
#else
int tun_alloc(char *dev)
{
    return devopen(dev, O_RDWR);
}
#endif


void cleanup(void)
{
    ssystem("ifconfig %s down", tundev);
    if(ipaddr){
    	ssystem("sysctl -w net.ipv6.conf.all.forwarding=0");
    }
}

void sigcleanup(int signo)
{
    fprintf(stderr, "signal %d\n", signo);
    exit(0);			/* exit(0) will call cleanup() */
}


static int request_mac;
static int send_mac;

void send_commands(void)
{
	char buf[3];

	if (request_mac) {
		slip_send('?');
		slip_send('M');
		slip_send(SLIP_END);

		request_mac = 0;
	}
	/* Send our mac to the device. If it knows our address, it is not needed to change
	the MAC address of our local interface (this can be also unsupported, especially under
	Windows).
	*/
	else if(send_mac && memcmp(&eth_addr, &eth_addr_null, 6) != 0 
		&& slip_empty()){
		short i;
		PRINTF("Sending our MAC.\n");

		slip_send('!');
		slip_send('M');

		for(i=0; i < 6; i++){
			sprintf(buf,"%02X",eth_addr.addr[i]);
			slip_send(buf[0]);
			slip_send(buf[1]);
		}
		slip_send(SLIP_END);

		send_mac = 0;
	}
	else if(set_sniffer_mode && slip_empty()){

		PRINTF("Setting sniffer mode to %d.\n", sniffer_mode);

		slip_send('!');
		slip_send('O');
		slip_send('S');

		if(sniffer_mode){
			slip_send('1');
		}
		else {
			slip_send('0');
		}
		slip_send(SLIP_END);

		set_sniffer_mode = 0;

	}
	else if(set_channel && slip_empty()){

		if(channel != 0){
			PRINTF("Setting channel %02d.\n", channel);

			slip_send('!');
			slip_send('O');
			slip_send('C');

			sprintf(buf,"%02d",channel);
			slip_send(buf[0]);
			slip_send(buf[1]);
				
			slip_send(SLIP_END);
		}

		set_channel = 0;

	}
}


int main(int argc, char **argv)
{
    int c;
    int tunfd, slipfd, maxfd;
    int ret;
    fd_set rset, wset;
    FILE *inslip;
    const char *siodev = NULL;
    int baudrate = -2;

    setvbuf(stdout, NULL, _IOLBF, 0);	/* Line buffered output. */

    while ((c = getopt(argc, argv, "B:D:hs:c:rt:a:p:v")) != -1) {
	switch (c) {
	  case 'B':
	      baudrate = atoi(optarg);
	      break;

	  case 's':
	      if (strncmp("/dev/", optarg, 5) == 0) {
		  siodev = optarg + 5;
	      }
	      else {
		  siodev = optarg;
	      }
	      break;

	  case 't':
	      if (strncmp("/dev/", optarg, 5) == 0) {
		  strcpy(tundev, optarg + 5);
	      }
	      else {
		  strcpy(tundev, optarg);
	      }
	      break;
	      
	  case 'c':
	  	channel = atoi(optarg);
		set_channel = 1;
		break;
	  case 'r':
		sniffer_mode = 1;
		break;

	  case 'a':
	      if (autoconf == true) {
		  errx(1, USAGE_STRING);
	      }
	      ipaddr = optarg;
	      break;

	  case 'p':
	      if (ipaddr != NULL) {
		  errx(1, USAGE_STRING);
	      }
	      autoconf = true;
	      ipprefix = optarg;
	      break;

	  case 'v':
	      verbose = true;
	      break;

	  case '?':
	  case 'h':
	  default:
	      errx(1, HELP_STRING);
	      break;
	}
    }
#if 0
    argc -= (optind - 1);
    argv += (optind - 1);

    //if(argc != 3 && argc != 4) {
    //if (autoconf == false && ipaddr == NULL ) {
	errx(1, USAGE_STRING);
    }
#endif


    switch (baudrate) {
      case -2:
	  break;		/* Use default. */
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


    if (siodev != NULL) {
	slipfd = devopen(siodev, O_RDWR | O_NONBLOCK);
	if (slipfd == -1) {
	    err(1, "can't open siodev ``/dev/%s''", siodev);
	}
    }
    else {
	static const char *siodevs[] = {
	    "ttyUSB0", "cuaU0", "ucom0"	/* linux, fbsd6, fbsd5 */
	};
	int i;
	for (i = 0; i < 3; i++) {
	    siodev = siodevs[i];
	    slipfd = devopen(siodev, O_RDWR | O_NONBLOCK);
	    if (slipfd != -1)
		break;
	}
	if (slipfd == -1) {
	    err(1, "can't open siodev");
	}
    }
    fprintf(stderr, "slip started on ``/dev/%s''\n", siodev);
    stty_telos(slipfd);
    slip_send(SLIP_END);
    inslip = fdopen(slipfd, "r");
    if (inslip == NULL)
	err(1, "main: fdopen");

    tunfd = tun_alloc(tundev);
    printf("opening: %s\n", tundev);
    if (tunfd == -1)
	err(1, "main: open");
    fprintf(stderr, "opened device ``/dev/%s''\n", tundev);

    atexit(cleanup);
    signal(SIGHUP, sigcleanup);
    signal(SIGTERM, sigcleanup);
    signal(SIGINT, sigcleanup);

  

    while (1) {
	maxfd = 0;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	
	send_commands();

	if (!slip_empty()) {	/* Anything to flush? */
	    FD_SET(slipfd, &wset);
	}

	FD_SET(slipfd, &rset);	/* Read from slip ASAP! */
	if (slipfd > maxfd)
	    maxfd = slipfd;

	/* We only have one packet at a time queued for slip output. */
	if (slip_empty()) {
	    FD_SET(tunfd, &rset);
	    if (tunfd > maxfd)
		maxfd = tunfd;
	}

	ret = select(maxfd + 1, &rset, &wset, NULL, NULL);
	if (ret == -1 && errno != EINTR) {
	    err(1, "select");
	}
	else if (ret > 0) {
	    if (FD_ISSET(slipfd, &rset)) {
		serial_to_tun(inslip, tunfd);
	    }

	    if (FD_ISSET(slipfd, &wset)) {
		slip_flushbuf(slipfd);
	    }

	    if (slip_empty() && FD_ISSET(tunfd, &rset)) {
		tun_to_serial(tunfd, slipfd);
		slip_flushbuf(slipfd);
	    }
	}
    }
}
