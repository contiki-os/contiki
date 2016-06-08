/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         Slip configuration
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <err.h>
#include "contiki.h"

int slip_config_verbose = 0;
const char *slip_config_ipaddr;
int slip_config_flowcontrol = 0;
int slip_config_timestamp = 0;
const char *slip_config_siodev = NULL;
const char *slip_config_host = NULL;
const char *slip_config_port = NULL;
char slip_config_tundev[32] = { "" };
uint16_t slip_config_basedelay = 0;

#ifndef BAUDRATE
#define BAUDRATE B115200
#endif
speed_t slip_config_b_rate = BAUDRATE;

/*---------------------------------------------------------------------------*/
int
slip_config_handle_arguments(int argc, char **argv)
{
  const char *prog;
  char c;
  int baudrate = 115200;

  slip_config_verbose = 0;

  prog = argv[0];
  while((c = getopt(argc, argv, "B:H:D:Lhs:t:v::d::a:p:T")) != -1) {
    switch(c) {
    case 'B':
      baudrate = atoi(optarg);
      break;

    case 'H':
      slip_config_flowcontrol = 1;
      break;

    case 'L':
      slip_config_timestamp = 1;
      break;

    case 's':
      if(strncmp("/dev/", optarg, 5) == 0) {
	slip_config_siodev = optarg + 5;
      } else {
	slip_config_siodev = optarg;
      }
      break;

    case 't':
      if(strncmp("/dev/", optarg, 5) == 0) {
	strncpy(slip_config_tundev, optarg + 5, sizeof(slip_config_tundev));
      } else {
	strncpy(slip_config_tundev, optarg, sizeof(slip_config_tundev));
      }
      break;

    case 'a':
      slip_config_host = optarg;
      break;

    case 'p':
      slip_config_port = optarg;
      break;

    case 'd':
      slip_config_basedelay = 10;
      if(optarg) slip_config_basedelay = atoi(optarg);
      break;

    case 'v':
      slip_config_verbose = 2;
      if(optarg) slip_config_verbose = atoi(optarg);
      break;

    case '?':
    case 'h':
    default:
fprintf(stderr,"usage:  %s [options] ipaddress\n", prog);
fprintf(stderr,"example: border-router.native -L -v2 -s ttyUSB1 aaaa::1/64\n");
fprintf(stderr,"Options are:\n");
#ifdef linux
fprintf(stderr," -B baudrate    9600,19200,38400,57600,115200,921600 (default 115200)\n");
#else
fprintf(stderr," -B baudrate    9600,19200,38400,57600,115200 (default 115200)\n");
#endif
fprintf(stderr," -H             Hardware CTS/RTS flow control (default disabled)\n");
fprintf(stderr," -L             Log output format (adds time stamps)\n");
fprintf(stderr," -s siodev      Serial device (default /dev/ttyUSB0)\n");
fprintf(stderr," -a host        Connect via TCP to server at <host>\n");
fprintf(stderr," -p port        Connect via TCP to server at <host>:<port>\n");
fprintf(stderr," -t tundev      Name of interface (default tun0)\n");
fprintf(stderr," -v[level]      Verbosity level\n");
fprintf(stderr,"    -v0         No messages\n");
fprintf(stderr,"    -v1         Encapsulated SLIP debug messages (default)\n");
fprintf(stderr,"    -v2         Printable strings after they are received\n");
fprintf(stderr,"    -v3         Printable strings and SLIP packet notifications\n");
fprintf(stderr,"    -v4         All printable characters as they are received\n");
fprintf(stderr,"    -v5         All SLIP packets in hex\n");
fprintf(stderr,"    -v          Equivalent to -v3\n");
fprintf(stderr," -d[basedelay]  Minimum delay between outgoing SLIP packets.\n");
fprintf(stderr,"                Actual delay is basedelay*(#6LowPAN fragments) milliseconds.\n");
fprintf(stderr,"                -d is equivalent to -d10.\n");
exit(1);
      break;
    }
  }
  argc -= optind - 1;
  argv += optind - 1;

  if(argc != 2 && argc != 3) {
    err(1, "usage: %s [-B baudrate] [-H] [-L] [-s siodev] [-t tundev] [-T] [-v verbosity] [-d delay] [-a serveraddress] [-p serverport] ipaddress", prog);
  }
  slip_config_ipaddr = argv[1];

  switch(baudrate) {
  case -2:
    break;			/* Use default. */
  case 9600:
    slip_config_b_rate = B9600;
    break;
  case 19200:
    slip_config_b_rate = B19200;
    break;
  case 38400:
    slip_config_b_rate = B38400;
    break;
  case 57600:
    slip_config_b_rate = B57600;
    break;
  case 115200:
    slip_config_b_rate = B115200;
    break;
#ifdef linux
  case 921600:
    slip_config_b_rate = B921600;
    break;
#endif
  default:
    err(1, "unknown baudrate %d", baudrate);
    break;
  }

  if(*slip_config_tundev == '\0') {
    /* Use default. */
    strcpy(slip_config_tundev, "tun0");
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
