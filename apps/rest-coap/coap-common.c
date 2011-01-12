/*
 * coap-common.c
 *
 *  Created on: Aug 30, 2010
 *      Author: dogan
 */

#ifdef CONTIKI_TARGET_NETSIM
  #include <stdio.h>
  #include <iostream>
  #include <cstring>
  #include <cstdlib>
  #include <unistd.h>
  #include <errno.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
#else
  #include "contiki.h"
  #include "contiki-net.h"
  #include <string.h>
#endif

#include "coap-common.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

void init_packet(coap_packet_t* packet)
{
  packet->ver = 1;
  packet->type = 0;
  packet->option_count = 0;
  packet->code = 0;
  packet->tid = 0;
  packet->options = NULL;
  packet->url = NULL;
  packet->url_len = 0;
  packet->query = NULL;
  packet->query_len = 0;
  packet->payload = NULL;
  packet->payload_len = 0;
}

int serialize_packet(coap_packet_t* packet, uint8_t* buffer)
{
  int index = 0;
  header_option_t* option = NULL;
  uint16_t option_delta = 0;

  buffer[0] = (packet->ver) << COAP_HEADER_VERSION_POSITION;
  buffer[0] |= (packet->type) << COAP_HEADER_TYPE_POSITION;
  buffer[0] |= packet->option_count;
  buffer[1] = packet->code;
  uint16_t temp = uip_htons(packet->tid);
  memcpy(
    (void*)&buffer[2],
    (void*)(&temp),
    sizeof(packet->tid));

  index += 4;

  PRINTF("serialize option_count %u\n", packet->option_count);

  /*Options should be sorted beforehand*/
  for (option = packet->options ; option ; option = option->next){
    uint16_t delta = option->option - option_delta;
    if ( !delta ){
      PRINTF("WARNING: Delta==Zero\n");
    }
    buffer[index] = (delta) << COAP_HEADER_OPTION_DELTA_POSITION;

    PRINTF("option %u len %u option diff %u option_value addr %x option addr %x next option addr %x", option->option, option->len, option->option - option_delta, (unsigned int) option->value, (unsigned int)option, (unsigned int)option->next);

    int i = 0;
    for ( ; i < option->len ; i++ ){
      PRINTF(" (%u)", option->value[i]);
    }
    PRINTF("\n");

    if (option->len < 0xF){
      buffer[index] |= option->len;
      index++;
    } else{
      buffer[index] |= (0xF); //1111
      buffer[index + 1] = option->len - (0xF);
      index += 2;
    }

    memcpy((char*)&buffer[index], option->value, option->len);
    index += option->len;
    option_delta += option->option;
  }

  if(packet->payload){
    memcpy(&buffer[index], packet->payload, packet->payload_len);
    index += packet->payload_len;
  }

  return index;
}
