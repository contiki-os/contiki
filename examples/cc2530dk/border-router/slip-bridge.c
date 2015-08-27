/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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

/**
 * \file
 *         Slip fallback interface
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Joel Hoglund <joel@sics.se>
 *         Nicolas Tsiftes <nvt@sics.se>
 */

#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"
#include "dev/slip.h"
#include <string.h>

#define UIP_IP_BUF        ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

void set_prefix_64(uip_ipaddr_t *);

static uip_ipaddr_t last_sender;
/*---------------------------------------------------------------------------*/
static void
slip_input_callback(void)
{
  PRINTF("SIN: %u\n", uip_len);
  if((char)uip_buf[0] == '!') {
    PRINTF("Got configuration message of type %c\n", uip_buf[1]);
    uip_clear_buf();
    if((char)uip_buf[1] == 'P') {
      uip_ipaddr_t prefix;
      /* Here we set a prefix !!! */
      memset(&prefix, 0, 16);
      memcpy(&prefix, &uip_buf[2], 8);
      PRINTF("Setting prefix ");
      PRINT6ADDR(&prefix);
      PRINTF("\n");
      set_prefix_64(&prefix);
    }
  }
  /* Save the last sender received over SLIP to avoid bouncing the
     packet back if no route is found */
  uip_ipaddr_copy(&last_sender, &UIP_IP_BUF->srcipaddr);
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  process_start(&slip_process, NULL);
  slip_set_input_callback(slip_input_callback);
}
/*---------------------------------------------------------------------------*/
static void
output(void)
{
  if(uip_ipaddr_cmp(&last_sender, &UIP_IP_BUF->srcipaddr)) {
    /* Do not bounce packets back over SLIP if the packet was received
       over SLIP */
    PRINTF("slip-bridge: Destination off-link but no route\n");
  } else {
    PRINTF("SUT: %u\n", uip_len);
    slip_send();
  }
}
/*---------------------------------------------------------------------------*/
struct uip_fallback_interface slip_interface = {
  init, output
};
/*---------------------------------------------------------------------------*/
