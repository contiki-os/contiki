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
 *         Sets up some commands for the border router
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#include "contiki.h"
#include "cmd.h"
#include "border-router.h"
#include "border-router-cmds.h"
#include "dev/serial-line.h"
#include "net/rpl/rpl.h"
#include "net/ip/uiplib.h"
#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"


uint8_t command_context;

void packet_sent(uint8_t sessionid, uint8_t status, uint8_t tx);
void nbr_print_stat(void);

/*---------------------------------------------------------------------------*/
PROCESS(border_router_cmd_process, "Border router cmd process");
/*---------------------------------------------------------------------------*/
/* TODO: the below code needs some way of identifying from where the command */
/* comes. In this case it can be from stdin or from SLIP.                    */
/*---------------------------------------------------------------------------*/
int
border_router_cmd_handler(const uint8_t *data, int len)
{
  /* handle global repair, etc here */
  if(data[0] == '!') {
    PRINTF("Got configuration message of type %c\n", data[1]);
    if(data[1] == 'G' && command_context == CMD_CONTEXT_STDIO) {
      /* This is supposed to be from stdin */
      printf("Performing Global Repair...\n");
      rpl_repair_root(RPL_DEFAULT_INSTANCE);
      return 1;
    } else if(data[1] == 'M' && command_context == CMD_CONTEXT_RADIO) {
      /* We need to know that this is from the slip-radio here. */
      PRINTF("Setting MAC address\n");
      border_router_set_mac(&data[2]);
      return 1;
    } else if(data[1] == 'C' && command_context == CMD_CONTEXT_RADIO) {
      /* We need to know that this is from the slip-radio here. */
      printf("Channel is:%d\n", data[2]);
      return 1;
    } else if(data[1] == 'R' && command_context == CMD_CONTEXT_RADIO) {
      /* We need to know that this is from the slip-radio here. */
      PRINTF("Packet data report for sid:%d st:%d tx:%d\n",
	     data[2], data[3], data[4]);
      packet_sent(data[2], data[3], data[4]);
      return 1;
    } else if(data[1] == 'D' && command_context == CMD_CONTEXT_RADIO) {
      /* We need to know that this is from the slip-radio here... */
      PRINTF("Sensor data received\n");
      border_router_set_sensors((const char *)&data[2], len - 2);
      return 1;
    }
  } else if(data[0] == '?') {
    PRINTF("Got request message of type %c\n", data[1]);
    if(data[1] == 'M' && command_context == CMD_CONTEXT_STDIO) {
      uint8_t buf[20];
      char* hexchar = "0123456789abcdef";
      int j;
      /* this is just a test so far... just to see if it works */
      buf[0] = '!';
      buf[1] = 'M';
      for(j = 0; j < 8; j++) {
        buf[2 + j * 2] = hexchar[uip_lladdr.addr[j] >> 4];
        buf[3 + j * 2] = hexchar[uip_lladdr.addr[j] & 15];
      }
      cmd_send(buf, 18);
      return 1;
    } else if(data[1] == 'C' && command_context == CMD_CONTEXT_STDIO) {
      /* send on! */
      write_to_slip(data, len);
      return 1;
    } else if(data[1] == 'S') {
      border_router_print_stat();
      return 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
border_router_cmd_output(const uint8_t *data, int data_len)
{
  int i;
  printf("CMD output: ");
  for(i = 0; i < data_len; i++) {
    printf("%c", data[i]);
  }
  printf("\n");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(border_router_cmd_process, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("Started br-cmd process\n");
  while(1) {
    PROCESS_YIELD();
    if(ev == serial_line_event_message && data != NULL) {
      PRINTF("Got serial data!!! %s of len: %d\n",
	     (char *)data, strlen((char *)data));
      command_context = CMD_CONTEXT_STDIO;
      cmd_input(data, strlen((char *)data));
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
