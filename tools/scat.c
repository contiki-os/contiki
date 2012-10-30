/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
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

#define SLIP_END     0300
#define SLIP_ESC     0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335

#ifndef BAUDRATE
#define BAUDRATE B115200
#endif
speed_t b_rate = BAUDRATE;

void
stty_telos(int fd)
{
  struct termios tty;
  int i;

  if(tcflush(fd, TCIOFLUSH) == -1) err(1, "tcflush");

  if(tcgetattr(fd, &tty) == -1) err(1, "tcgetattr");

  cfmakeraw(&tty);

  /* Blocking read. */
  tty.c_cc[VTIME] = 0;
  tty.c_cc[VMIN] = 1;
  tty.c_cflag &= ~CRTSCTS;
  tty.c_cflag &= ~HUPCL;
  tty.c_cflag &= ~CLOCAL;

  cfsetispeed(&tty, b_rate);
  cfsetospeed(&tty, b_rate);

  if(tcsetattr(fd, TCSAFLUSH, &tty) == -1) err(1, "tcsetattr");

  tty.c_cflag |= CLOCAL;
  if(tcsetattr(fd, TCSAFLUSH, &tty) == -1) err(1, "tcsetattr");

  i = TIOCM_DTR;
  if(ioctl(fd, TIOCMBIS, &i) == -1) err(1, "ioctl");

  usleep(10*1000);		/* Wait for hardware 10ms. */

  /* Flush input and output buffers. */
  if(tcflush(fd, TCIOFLUSH) == -1) err(1, "tcflush");
}

int
main(int argc, char **argv)
{
  int c;
  int slipfd;
  FILE *inslip;
  const char *siodev;
  int baudrate = -2;
  
  while ((c = getopt(argc, argv, "B:")) != -1) {
    switch (c) {
    case 'B':
      baudrate = atoi(optarg);
      break;

    case '?':
    case 'h':
    default:
      err(1, "usage: scat [-B baudrate] device-file");
      break;
    }
  }
  argc -= (optind - 1);
  argv += (optind - 1);

  switch (baudrate) {
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

  if (argc != 2)
    err(1, "usage: scat device-file");
  siodev = argv[1];

  setvbuf(stdout, NULL, _IOLBF, 0); /* Line buffered output. */

  slipfd = open(siodev, O_RDWR);
  if (slipfd == -1) err(1, "can't open '%s'", siodev);
  stty_telos(slipfd);
  inslip = fdopen(slipfd, "r");
  if(inslip == NULL) err(1, "main: fdopen");

  while (1) {
    int c = getc(inslip);
    while (c == SLIP_END)
      c = getc(inslip);
    do {
      if (c == SLIP_ESC) {
	c = getc(inslip);
	if (c == SLIP_ESC_ESC)
	  c = SLIP_ESC;
	else if (c == SLIP_ESC_END)
	  c = SLIP_END;
      }
      switch (c) {
      case EOF:
	err(1, "getc(inslip)");
	break;

      case '\007':
      case '\b':
      case '\f':
      case '\n':
      case '\r':
      case '\t':
      case '\v':
	putchar(c);
	break;

      default:
	if (isprint(c))
	  putchar(c);
	else
	  printf("%02x ", c);
	break;
      }
      c = getc(inslip);
    } while (c != SLIP_END);
  }

  return 0;    
}
