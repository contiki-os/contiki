/*
 * Copyright (c) 2015, Yanzi Networks AB.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
 * \file
 *         Simple serial protocol to list and interact with devices
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include "contiki.h"
#include "net/ip/uip.h"
#include "net/ip/uiplib.h"
#include <stdio.h>

void print_node_list(void);
void set_value(const uip_ipaddr_t *addr, char *uri, char *value);
void get_value(const uip_ipaddr_t *addr, char *uri);
/*---------------------------------------------------------------------------*/
int
find_next_sep(const char *str, char sep, int pos)
{
  char c;
  while((c = str[pos]) != 0) {
    if(c == sep) {
      return pos + 1;
    }
    pos++;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
/*
 * l - list all discovered devices
 * s - set <IP> <URI> <value>
 * d - get <IP> <URI>
 */
void
serial_protocol_input(char *data)
{
  /* We assume that we have a string here */
  char cmd = data[0];
  int pos = 0;

  switch(cmd) {
  case 'l':
    /* list devices */
    print_node_list();
    break;
  case 's': {
    uip_ip6addr_t ipaddr;
    char *uri;
    char *value;
    pos = find_next_sep(data, ' ', pos);
    if(pos > 0) {
      /* start of IP */
      int start = pos;
      pos = find_next_sep(data, ' ', pos);
      if(pos == -1) {
        return;
      }
      data[pos - 1] = 0;
      if(uiplib_ip6addrconv(&data[start], &ipaddr) == 0) {
        printf("* Error not valid IP\n");
      }
      uri = &data[pos];
      pos = find_next_sep(data, ' ', pos);
      if(pos == -1) return;
      data[pos - 1] = 0;
      value = &data[pos];
      /* set the value at the specified node */
      set_value(&ipaddr, uri, value);
    }
    break;
  }
  case 'g': {
    uip_ip6addr_t ipaddr;
    char *uri;
    pos = find_next_sep(data, ' ', pos);
    if(pos > 0) {
      /* start of IP */
      int start = pos;
      pos = find_next_sep(data, ' ', pos);
      if(pos == -1) return;
      data[pos - 1] = 0;
      if(uiplib_ip6addrconv((const char *) &data[start], &ipaddr) == 0) {
        printf("* Error not valid IP\n");
      }
      uri = &data[pos];
      /* get the value at the specified node */
      get_value(&ipaddr, uri);
    }
    break;
  }
  default:
    printf("Unknown command\n");
  }
}
/*---------------------------------------------------------------------------*/
