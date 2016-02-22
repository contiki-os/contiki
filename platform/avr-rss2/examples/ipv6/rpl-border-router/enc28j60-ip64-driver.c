/*
 * Copyright (c) 2012-2013, Thingsquare, http://www.thingsquare.com/.
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "contiki.h"
#include "enc28j60.h"
#include "enc28j60-ip64-driver.h"
#include "net/linkaddr.h"

#include "ip64.h"
#include "ip64-eth.h"

#include <string.h>
#include <stdio.h>

PROCESS(enc28j60_ip64_driver_process, "ENC28J60 IP64 driver");

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  uint8_t eui64[8];
  uint8_t macaddr[6];

  /* Assume that linkaddr_node_addr holds the EUI64 of this device. */
  memcpy(eui64, &linkaddr_node_addr, sizeof(eui64));

  /* Mangle the EUI64 into a 48-bit Ethernet address. */
  memcpy(&macaddr[0], &eui64[0], 3);
  memcpy(&macaddr[3], &eui64[5], 3);

  /* In case the OUI happens to contain a broadcast bit, we mask that
     out here. */
  macaddr[0] = (macaddr[0] & 0xfe);

  /* Set the U/L bit, in order to create a locally administered MAC address */
  macaddr[0] = (macaddr[0] | 0x02);

  memcpy(ip64_eth_addr.addr, macaddr, sizeof(macaddr));

  printf("MAC addr %02x:%02x:%02x:%02x:%02x:%02x\n",
         macaddr[0], macaddr[1], macaddr[2],
         macaddr[3], macaddr[4], macaddr[5]);
  enc28j60_init(macaddr);
  process_start(&enc28j60_ip64_driver_process, NULL);
}
/*---------------------------------------------------------------------------*/
static int
output(uint8_t *packet, uint16_t len)
{
  enc28j60_send(packet, len);
  return len;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(enc28j60_ip64_driver_process, ev, data)
{
  static int len;
  static struct etimer e;
  PROCESS_BEGIN();

  while(1) {
    etimer_set(&e, 1);
    PROCESS_WAIT_EVENT();
    len = enc28j60_read(ip64_packet_buffer, ip64_packet_buffer_maxlen);
    if(len > 0) {
      IP64_INPUT(ip64_packet_buffer, len);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
const struct ip64_driver enc28j60_ip64_driver = {
  init,
  output
};
/*---------------------------------------------------------------------------*/
