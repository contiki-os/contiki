/*
 * coap-common.c
 *
 *  Created on: Aug 30, 2010
 *      Author: dogan
 */

#ifdef CONTIKI_TARGET_SKY
  #include "contiki.h"
  #include "contiki-net.h"
#else
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
#endif

#include "coap-common.h"

#define DEBUG 1
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

void initialize_packet(coap_packet_t* packet)
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
}

//void parse_message(coap_packet_t* packet, uint8_t* buf, uint16_t size)
//{
//  int processed=0;
//  int i=0;
//  PRINTF("parse_message size %d-->\n",size);
//
//  initialize_packet(packet);
//
//  packet->ver = (buf[0]&COAP_HEADER_VERSION_MASK)>>COAP_HEADER_VERSION_POSITION;
//  packet->type = (buf[0]&COAP_HEADER_TYPE_MASK)>>COAP_HEADER_TYPE_POSITION;
//  packet->option_count = buf[0]&COAP_HEADER_OPTION_COUNT_MASK;
//  packet->code = buf[1];
//  packet->tid = (buf[2] << 8) + buf[3];
//
//  processed += 4;
//
//  header_option_t options[5];
//
//  if(packet->option_count){
//    int option_index=0;
//    uint8_t option_delta;
//    uint16_t option_len;
////    uint8_t option_value[100];
//    uint8_t* option_buf = buf+processed;
//
//    while(option_index < packet->option_count){
//      //DY FIX_ME : put boundary controls
////      int j=0;
//      option_delta=(option_buf[i] & COAP_HEADER_OPTION_DELTA_MASK) >> COAP_HEADER_OPTION_DELTA_POSITION;
//      option_len=(option_buf[i] & COAP_HEADER_OPTION_SHORT_LENGTH_MASK);
//      i++;
//      if(option_len==0xf){
//        option_len+=option_buf[i];
//        i++;
//      }
//
//      options[option_index].option=option_delta;
//      options[option_index].len=option_len;
//      options[option_index].value=option_buf+i;
//      if (option_index){
//        options[option_index-1].next=&options[option_index];
//        /*This field defines the difference between the option Type of
//         * this option and the previous option (or zero for the first option)*/
//        options[option_index].option+=options[option_index-1].option;
//      }
//
//      if (options[option_index].option==Option_Type_Uri_Path){
//        packet->url = (char*)options[option_index].value;
//        packet->url_len = options[option_index].len;
//      }
//
//      PRINTF("OPTION %d %u %s \n", options[option_index].option, options[option_index].len, options[option_index].value);
//
//      i += option_len;
//      option_index++;
//    }
//  }
//  processed += i;
//
//  /**/
//  if (processed < size) {
//    packet->payload = &buf[processed];
//  }
//
//  PRINTF("PACKET ver:%d type:%d oc:%d \ncode:%d tid:%u url:%s len:%u payload:%s\n", (int)packet->ver, (int)packet->type, (int)packet->option_count, (int)packet->code, packet->tid, packet->url, packet->url_len, packet->payload);
//}

int serialize_packet(coap_packet_t* packet, uint8_t* buffer)
{
  int index = 0;
  header_option_t* option = NULL;
  uint16_t option_delta = 0;

  buffer[0] = (packet->ver) << COAP_HEADER_VERSION_POSITION;
  buffer[0] |= (packet->type) << COAP_HEADER_TYPE_POSITION;
  buffer[0] |= packet->option_count;
  buffer[1] = packet->code;
  uint16_t temp = htons(packet->tid);
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

    PRINTF("option %u len %u option diff %u option_value addr %x option addr %x next option addr %x", option->option, option->len, option->option - option_delta, (uint16_t) option->value, (uint16_t)option, (uint16_t)option->next);
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

//  //QUICK HACK TO SEND URL
//  if(packet->url){
//    buffer[index] = (Option_Type_Uri_Path) << COAP_HEADER_OPTION_DELTA_POSITION;
//    int uri_len = strlen(packet->url);
//    if(uri_len < 0xF)
//    {
//      buffer[index] |= uri_len;
//      strcpy((char*)&buffer[index + 1], packet->url);
//      index += 1 + uri_len;
//    }
//    else
//    {
//      buffer[index] |= (0xF); //1111
//      buffer[index + 1] = uri_len - (0xF);
//      strcpy((char*)&buffer[index + 2],packet->url);
//      index += 2 + uri_len;
//    }
//  }

  if(packet->payload){
    memcpy(&buffer[index], packet->payload, packet->payload_len);
    index += packet->payload_len;
  }

  return index;
}
