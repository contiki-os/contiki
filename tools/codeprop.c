/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Should be included from codeprop.h, but the include paths in the makefiles
   isn't set up for that. */
#define HDR_SIZE 4

int
main(int argc, char **argv) {
  struct sockaddr_in sa;
  int s, port, fd;
  char *ip_addr;
  int total = 0;

  if(argc != 3) {
    printf("usage: %s ipaddress filename\n", argv[0]);
    exit(1);
  }
  ip_addr = argv[1];
  port    = 6510;

  /* Create socket. */
  if((s = socket(AF_INET,SOCK_STREAM,0)) < 0){
    perror("Can't create socket");
    exit(1);
  }

  /* Set the destination address of the socket. */
  bzero((char *) &sa, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr(ip_addr);
  sa.sin_port = uip_htons(port);

  /* Connect the socket to the remote host. */
  if(connect(s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    perror("connect failed");
    exit(1);
  }
  socklen_t slen = 576 - 40;
  if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, &slen, sizeof(slen)) < 0) {
    perror("setsockopt failed");
    exit(1);
  }

  if((fd = open(argv[2], O_RDONLY)) < 0) {
    perror("Could not open file");
    exit(1);
  }
  
  
  while(1) {
    char buf[64000];
    int len;
    
    len = read(fd, &buf[HDR_SIZE], sizeof(buf) - HDR_SIZE);
    if(len == 0) {
      printf("File successfully sent (%d bytes)\n", total);
      len = read(s, buf, sizeof(buf));
      buf[len] = 0;
      printf("Reply: %s", buf);
      if(buf[0] != 'o' || buf[1] != 'k') {
	/* Cut and pasted from core/loader/elfloader.h */
	printf("OK                  0\n"
	       "BAD_HEADER          1\n"
	       "NO_SYMTAB           2\n"
	       "NO_STRTAB           3\n"
	       "NO_TEXT             4\n"
	       "UNDEFINED           5\n"
	       "UNKNOWN_SEGMENT     6\n"
	       "NO_STARTPOINT       7\n"
	       "TEXT_TO_LARGE       8\n"
	       "DATA_TO_LARGE       9\n"
	       "UNKNOWN_RELOC      10\n"
	       "MULTIPLY_DEFINED   11\n");
      }
      exit(0);
    }
    total += len;
    buf[0] = len >> 8;
    buf[1] = len & 0xff;
    if(write(s, buf, len + HDR_SIZE) == -1) {
      perror("network send failed");
      exit(1);
    }
  }
  
  return 0;
}
