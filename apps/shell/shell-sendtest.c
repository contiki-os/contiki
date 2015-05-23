/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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

/**
 * \file
 *         A small pogram to measure the communication performance between two nodes
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell-ps.h"
#include "net/rime/rime.h"

#include <stdio.h>
#include <string.h>

#if CONTIKI_TARGET_NETSIM
#include "ether.h"
#endif /* CONTIKI_TARGET_NETSIM */
#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t size, const char *format, ...);
#endif /* HAVE_SNPRINTF */

/*---------------------------------------------------------------------------*/
PROCESS(shell_sendtest_process, "sendtest");
SHELL_COMMAND(sendtest_command,
	      "sendtest",
	      "sendtest: measure single-hop throughput",
	      &shell_sendtest_process);
/*---------------------------------------------------------------------------*/
static clock_time_t start_time_rucb, end_time_rucb;
static unsigned long filesize, bytecount, packetsize;
static int download_complete;
static void
write_chunk(struct rucb_conn *c, int offset, int flag,
	    char *data, int datalen)
{
#if CONTIKI_TARGET_NETSIM
  {
    char buf[100];
    printf("received %d; %d\n", offset, datalen);
    sprintf(buf, "%lu%%", (100 * (offset + datalen)) / filesize);
    ether_set_text(buf);
  }
#endif /* CONTIKI_TARGET_NETSIM */
  /*  printf("+");*/
}
static int
read_chunk(struct rucb_conn *c, int offset, char *to, int maxsize)
{
  int size;
  /*  printf("-");*/
  size = maxsize;
  if(bytecount + maxsize >= filesize) {
    size = filesize - bytecount;
  }
  if(size > packetsize) {
    size = packetsize;
  }
  bytecount += size;
  if(bytecount == filesize) {
    end_time_rucb = clock_time();
    download_complete = 1;
    process_post(&shell_sendtest_process, PROCESS_EVENT_CONTINUE, NULL);
    /*     profile_aggregates_print(); */
/*     profile_print_stats(); */
//    print_stats();
  }

  /*  printf("bytecount %lu\n", bytecount);*/
  return size;
}
const static struct rucb_callbacks rucb_callbacks = {write_chunk,
						     read_chunk,
						     NULL};
static struct rucb_conn rucb;
/*---------------------------------------------------------------------------*/
static void
print_usage(void)
{
  shell_output_str(&sendtest_command,
		   "sendtest <receiver> <size> [packetsize]: recevier must be specified", "");

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_sendtest_process, ev, data)
{
  static linkaddr_t receiver;
  static unsigned long cpu, lpm, rx, tx;
  const char *nextptr;
  const char *args;
  char buf[40];
  unsigned long cpu2, lpm2, rx2, tx2;
  
  PROCESS_BEGIN();

  args = data;
  receiver.u8[0] = shell_strtolong(args, &nextptr);
  if(nextptr == data || *nextptr != '.') {
    print_usage();
    PROCESS_EXIT();
  }
  args = nextptr + 1;
  receiver.u8[1] = shell_strtolong(args, &nextptr);

  
  args = nextptr;
  while(*args == ' ') {
    ++args;
  }
  filesize = shell_strtolong(args, &nextptr);  
  if(nextptr == data || filesize == 0) {
    print_usage();
    PROCESS_EXIT();
  }

  args = nextptr;
  while(*args == ' ') {
    ++args;
  }
  packetsize = 64;
  packetsize = shell_strtolong(args, &nextptr);  
  if(packetsize == 0) {
    print_usage();
    PROCESS_EXIT();
  }

  snprintf(buf, sizeof(buf), "%d.%d, %lu bytes, packetsize %lu",
	   receiver.u8[0], receiver.u8[1], filesize, packetsize);
  shell_output_str(&sendtest_command, "Sending data to ", buf);

  bytecount = 0;
  download_complete = 0;

  start_time_rucb = clock_time();
  rucb_send(&rucb, &receiver);

  energest_flush();
  lpm = energest_type_time(ENERGEST_TYPE_LPM);
  cpu = energest_type_time(ENERGEST_TYPE_CPU);
  rx = energest_type_time(ENERGEST_TYPE_LISTEN);
  tx = energest_type_time(ENERGEST_TYPE_TRANSMIT);

  PROCESS_WAIT_UNTIL(download_complete);

  energest_flush();
  lpm2 = energest_type_time(ENERGEST_TYPE_LPM);
  cpu2 = energest_type_time(ENERGEST_TYPE_CPU);
  rx2 = energest_type_time(ENERGEST_TYPE_LISTEN);
  tx2 = energest_type_time(ENERGEST_TYPE_TRANSMIT);

  sprintf(buf, "%d seconds, %lu bytes/second",
	  (int)((end_time_rucb - start_time_rucb) / CLOCK_SECOND),
	  CLOCK_SECOND * filesize / (end_time_rucb - start_time_rucb));
  shell_output_str(&sendtest_command, "Completed in ", buf);

  sprintf(buf, "%lu/%d rx %lu/%d tx (seconds)",
	  (rx2 - rx), RTIMER_ARCH_SECOND,
	  (tx2 - tx), RTIMER_ARCH_SECOND);
  shell_output_str(&sendtest_command, "Radio total on time ", buf);

  sprintf(buf, "%lu/%lu = %lu%%",
	  (rx2 - rx),
	  (cpu2 + lpm2 - cpu - lpm),
	  100 * (rx2 - rx)/(cpu2 + lpm2 - cpu - lpm));
  shell_output_str(&sendtest_command, "Radio rx duty cycle ", buf);

  sprintf(buf, "%lu/%lu = %lu%%",
	  (tx2 - tx),
	  (cpu2 + lpm2 - cpu - lpm),
	  100 * (tx2 - tx)/(cpu2 + lpm2 - cpu - lpm));
  shell_output_str(&sendtest_command, "Radio tx duty cycle ", buf);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_sendtest_init(void)
{
  rucb_open(&rucb, SHELL_RIME_CHANNEL_SENDTEST, &rucb_callbacks);
  shell_register_command(&sendtest_command);
}
/*---------------------------------------------------------------------------*/
