/*
 * Copyright (c) 2010, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
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
 * This file is part of the Contiki OS
 *
 * $Id: contiki-init-net.c,v 1.1 2010/10/25 09:03:39 salvopitru Exp $
 */
/*---------------------------------------------------------------------------*/
/**
* \file
*			Functions for net initialization.
* \author
*			Salvatore Pitrulli <salvopitru@users.sourceforge.net>
*/
/*---------------------------------------------------------------------------*/

#include "contiki-net.h"

#if UIP_CONF_IPV6

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x ",lladdr.u8[0], lladdr.u8[1], lladdr.u8[2], lladdr.u8[3],lladdr.u8[4], lladdr.u8[5], lladdr.u8[6], lladdr.u8[7])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

void print_address(uip_ds6_addr_t *lladdr)
{
  int i;
  
  for(i = 0; i < 7; ++i) {
    printf("%02x%02x:", lladdr->ipaddr.u8[i * 2], lladdr->ipaddr.u8[i * 2 + 1]);
  }
  printf("%02x%02x", lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);
}

/*---------------------------------------------------------------------------*/
void print_addresses(void)
{
  uip_ds6_addr_t *lladdr;


  printf("link-local IPv6 address: ");
  
  lladdr = uip_ds6_get_link_local(-1);
  if(lladdr != NULL){
    print_address(lladdr);  
    printf("\r\n");
  }
  else
    printf("None\r\n");
  
  printf("global IPv6 address: ");
  
  lladdr = uip_ds6_get_global(-1);
  if(lladdr != NULL){
    print_address(lladdr);  
    printf("\r\n");
  }
  else
    printf("None\r\n");

}

#if FIXED_NET_ADDRESS

#include "net/rpl/rpl.h"


void set_net_address(void)
{
  uip_ipaddr_t ipaddr;
#if RPL_BORDER_ROUTER
  rpl_dag_t *dag;
#endif

  uip_ip6addr(&ipaddr, NET_ADDR_A, NET_ADDR_B, NET_ADDR_C, NET_ADDR_D, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);  

 
//#if !UIP_CONF_ROUTER
//  uip_ds6_prefix_add(&ipaddr, 64, 0); // For on-link determination.
//#else
//  uip_ds6_prefix_add(&ipaddr, 64, 0, 0, 600, 600);
//#endif
  
  print_addresses();
  
#if RPL_BORDER_ROUTER
  dag = rpl_set_root(RPL_DEFAULT_INSTANCE,&ipaddr);
  if(dag != NULL) {
    PRINTF("This node is setted as root of a DAG.\r\n");
  }
  else {
    PRINTF("Error while setting this node as root of a DAG.\r\n");
  }
#endif
  
}
#endif /* FIXED_GLOBAL_ADDRESS */


#endif /* UIP_CONF_IPV6 */
