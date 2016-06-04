/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 */
#include "contiki.h"
#include "contiki-lib.h"
#include "at-master.h"
#include "cpu.h"
#include "dev/uart.h"
#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include "lib/list.h"
#include "sys/cc.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
LIST(at_cmd_list);
process_event_t at_cmd_received_event;
/*---------------------------------------------------------------------------*/
static uint8_t at_uart = 0;
/*---------------------------------------------------------------------------*/
PROCESS(at_process, "AT process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(at_process, ev, data)
{
  uint8_t plen;
  char *pch, *buf;
  struct at_cmd *a;
  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message && data != NULL);
    buf = (char *)data;
    plen = strlen(buf);
    for(a = list_head(at_cmd_list); a != NULL; a = list_item_next(a)) {
      pch = strstr(buf, a->cmd_header);
      if((plen <= a->cmd_max_len) && (pch != NULL)) {
        if(strncmp(a->cmd_header, pch, a->cmd_hdr_len) == 0) {
          if((a->cmd_hdr_len == plen) || (a->cmd_max_len > a->cmd_hdr_len)) {
            a->event_callback(a, plen, (char *)pch);
            process_post(a->app_process, at_cmd_received_event, NULL);
            break;
          }
        }
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
struct at_cmd *
at_list(void)
{
  return list_head(at_cmd_list);
}
/*---------------------------------------------------------------------------*/
uint8_t
at_send(char *s, uint8_t len)
{
  uint8_t i = 0;
  while(s && *s != 0) {
    if(i >= len) {
      break;
    }
    uart_write_byte(at_uart, *s++);
    i++;
  }
  return i;
}
/*---------------------------------------------------------------------------*/
void
at_init(uint8_t uart_sel)
{
  static uint8_t inited = 0;
  if(!inited) {
    list_init(at_cmd_list);
    at_cmd_received_event = process_alloc_event();
    inited = 1;

    at_uart = uart_sel;
    uart_init(at_uart);
    uart_set_input(at_uart, serial_line_input_byte);
    serial_line_init();

    process_start(&at_process, NULL);
    PRINTF("AT: Started (%u)\n", at_uart);
  }
}
/*---------------------------------------------------------------------------*/
at_status_t
at_register(struct at_cmd *cmd, struct process *app_process,
            const char *cmd_hdr, const uint8_t hdr_len,
            const uint8_t cmd_max_len, at_event_callback_t event_callback)
{
  if((hdr_len < 1) || (cmd_max_len < 1) || (!strncmp(cmd_hdr, "AT", 2) == 0) ||
     (event_callback == NULL)) {
    PRINTF("AT: Invalid argument\n");
    return AT_STATUS_INVALID_ARGS_ERROR;
  }

  memset(cmd, 0, sizeof(struct at_cmd));
  cmd->event_callback = event_callback;
  cmd->cmd_header = cmd_hdr;
  cmd->cmd_hdr_len = hdr_len;
  cmd->cmd_max_len = cmd_max_len;
  cmd->app_process = app_process;
  list_add(at_cmd_list, cmd);
  PRINTF("AT: registered HDR %s LEN %u MAX %u\n", cmd->cmd_header,
                                                  cmd->cmd_hdr_len,
                                                  cmd->cmd_max_len);
  return AT_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
