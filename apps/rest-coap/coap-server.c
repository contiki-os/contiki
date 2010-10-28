#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> /*for isxdigit*/
#include "contiki.h"
#include "contiki-net.h"

#include "rest.h"
#include "buffer.h"
#include "rest-util.h"

#include "dev/leds.h"

#if !UIP_CONF_IPV6_RPL
#include "static-routing.h"
#endif

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

#define MAX_PAYLOAD_LEN 120
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
static struct uip_udp_conn *server_conn;

static service_callback service_cbk = NULL;

void
coap_set_service_callback(service_callback callback)
{
  service_cbk = callback;
}

static void
parse_message(coap_packet_t* packet, uint8_t* buf, uint16_t size)
{
  int processed=0;
  int i=0;
  PRINTF("parse_message size %d-->\n",size);

  initialize_packet(packet);

  packet->ver = (buf[0]&COAP_HEADER_VERSION_MASK)>>COAP_HEADER_VERSION_POSITION;
  packet->type = (buf[0]&COAP_HEADER_TYPE_MASK)>>COAP_HEADER_TYPE_POSITION;
  packet->option_count = buf[0]&COAP_HEADER_OPTION_COUNT_MASK;
  packet->code = buf[1];
  packet->tid = (buf[2] << 8) + buf[3];

  processed += 4;

  if (packet->option_count) {
    int option_index = 0;
    uint8_t option_delta;
    uint16_t option_len;
    uint8_t* option_buf = buf + processed;
    packet->options = (header_option_t*)allocate_buffer(sizeof(header_option_t) * packet->option_count);

    if (packet->options) {
      header_option_t* current_option = packet->options;
      header_option_t* prev_option = NULL;
      while(option_index < packet->option_count){
        /*FIXME : put boundary controls*/
        option_delta = (option_buf[i] & COAP_HEADER_OPTION_DELTA_MASK) >> COAP_HEADER_OPTION_DELTA_POSITION;
        option_len = (option_buf[i] & COAP_HEADER_OPTION_SHORT_LENGTH_MASK);
        i++;
        if(option_len == 0xf){
          option_len += option_buf[i];
          i++;
        }

        current_option->option = option_delta;
        current_option->len = option_len;
        current_option->value = option_buf + i;
        if (option_index){
          prev_option->next = current_option;
          /*This field defines the difference between the option Type of
           * this option and the previous option (or zero for the first option)*/
          current_option->option += prev_option->option;
        }

        if (current_option->option == Option_Type_Uri_Path){
          packet->url = (char*)current_option->value;
          packet->url_len = current_option->len;
        }

        PRINTF("OPTION %d %u %s \n", current_option->option, current_option->len, current_option->value);

        i += option_len;
        option_index++;
        prev_option = current_option++;
      }
      current_option->next = NULL;
    } else {
      PRINTF("MEMORY ERROR\n"); /*FIXME : add control here*/
      return;
    }
  }
  processed += i;

  /**/
  if (processed < size) {
    packet->payload = &buf[processed];
    packet->payload_len = size - processed;
  }

  /*FIXME url is not decoded - is necessary?*/
  if (packet->url) {
    if ((packet->query = strchr(packet->url, '?'))) {
      uint16_t total_url_len = packet->url_len;
      /*set query len and update url len so that it does not include query part now*/
      packet->url_len = packet->query - packet->url;
      packet->query++;
      packet->query_len = packet->url + total_url_len - packet->query;

      PRINTF("url %s, url_len %u, query %s, query_len %u\n", packet->url, packet->url_len, packet->query, packet->query_len);
    }
  }

  PRINTF("PACKET ver:%d type:%d oc:%d \ncode:%d tid:%u url:%s len:%u payload:%s pay_len %u\n", (int)packet->ver, (int)packet->type, (int)packet->option_count, (int)packet->code, packet->tid, packet->url, packet->url_len, packet->payload, packet->payload_len);
}

int
coap_get_query_variable(coap_packet_t* packet, const char *name, char* output, uint16_t output_size)
{
  if (packet->query) {
    return get_variable(name, packet->query, packet->query_len, output, output_size, 0);
  }

  return 0;
}

int
coap_get_post_variable(coap_packet_t* packet, const char *name, char* output, uint16_t output_size)
{
  if (packet->payload) {
    return get_variable(name, packet->payload, packet->payload_len, output, output_size, 1);
  }

  return 0;
}

static header_option_t*
allocate_header_option(uint16_t variable_len)
{
  PRINTF("sizeof header_option_t %u variable size %u\n", sizeof(header_option_t), variable_len);
  uint8_t* buffer = allocate_buffer(sizeof(header_option_t) + variable_len);
  if (buffer){
    header_option_t* option = (header_option_t*) buffer;
    option->next = NULL;
    option->len = 0;
    option->value = buffer + sizeof(header_option_t);
    return option;
  }

  return NULL;
}

/*FIXME : does not overwrite the same option yet.*/
static int
set_option(coap_packet_t* packet, option_type option_type, uint16_t len, uint8_t* value)
{
  PRINTF("set_option len %u\n", len);
  header_option_t* option = allocate_header_option(len);
  if (option){
    option->next = NULL;
    option->len = len;
    option->option = option_type;
    memcpy(option->value, value, len);
    header_option_t* option_current = packet->options;
    header_option_t* prev = NULL;
    while (option_current){
      if (option_current->option > option->option){
        break;
      }
      prev = option_current;
      option_current = option_current->next;
    }

    if (!prev){
      if (option_current){
        option->next = option_current;
      }
      packet->options = option;
    } else{
      option->next = option_current;
      prev->next = option;
    }

    packet->option_count++;

    PRINTF("option->len %u option->option %u option->value %x next %x\n", option->len, option->option, (uint16_t) option->value, (uint16_t)option->next);
    int i = 0;
    for ( ; i < option->len ; i++ ){
      PRINTF(" (%u)", option->value[i]);
    }
    PRINTF("\n");

    return 1;
  }

  return 0;
}

static uint8_t*
get_option(coap_packet_t* packet, option_type option_type)
{
  uint8_t* value = NULL;
  int i=0;

  header_option_t* current_option = packet->options;
  for (; packet->option_count; i++){
    if (current_option->option >= option_type){
      if (current_option->option == option_type){
        value = current_option->value;
      }
      break;
    }
  }

  return value;
}

static void
fill_error_packet(coap_packet_t* packet, int error, uint16_t tid)
{
  packet->ver=1;
  packet->option_count=0;
  packet->url=NULL;
  packet->options=NULL;
  switch (error){
    case MEMORY_ALLOC_ERR:
      packet->code=INTERNAL_SERVER_ERROR_500;
      packet->tid=tid;
      packet->type=MESSAGE_TYPE_ACK;
      break;
    default:
      break;
  }
}

static void
init_response(coap_packet_t* request, coap_packet_t* response)
{
  initialize_packet(response);
  if(request->type == MESSAGE_TYPE_CON)
  {
    response->code = OK_200;
    response->tid = request->tid;
    response->type = MESSAGE_TYPE_ACK;
  }
}

int
coap_set_payload(coap_packet_t* packet, uint8_t* payload, uint16_t size)
{
  packet->payload = copy_to_buffer(payload, size);
  if (packet->payload) {
    packet->payload_len = size;
    return 1;
  }

  return 0;
}

int
coap_set_header_content_type(coap_packet_t* packet, content_type_t content_type)
{
  uint16_t len = 1;

  return set_option(packet, Option_Type_Content_Type, len, (uint8_t*) &content_type);
}

content_type_t
coap_get_header_content_type(coap_packet_t* packet)
{
  uint8_t* value = get_option(packet, Option_Type_Content_Type);
  if(value){
    return (uint8_t)*value;
  }

  return DEFAULT_CONTENT_TYPE;
}

int
coap_set_header_uri(coap_packet_t* packet, char* uri)
{
  return set_option(packet, Option_Type_Uri_Path, strlen(uri), (uint8_t*) uri);
}

int
coap_set_header_etag(coap_packet_t* packet, uint8_t* etag, uint8_t size)
{
  return set_option(packet, Option_Type_Etag, size, etag);
}

void
coap_set_code(coap_packet_t* packet, uint8_t code)
{
  packet->code = code;
}

static int
handle_data(void)
{
  int error=NO_ERROR;
  char buf[MAX_PAYLOAD_LEN];

  printf("uip_datalen received %u \n",(u16_t)uip_datalen());

  char* data = uip_appdata + uip_ext_len;
  u16_t datalen = uip_datalen() - uip_ext_len;

  int data_size = 0;

  if(uip_newdata()) {
    ((char *)data)[datalen] = 0;
    PRINTF("Server received: '%s' (port:%u) from ", (char *)data, HTONS(UIP_UDP_BUF->srcport));
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("\n");

    if(init_buffer(COAP_DATA_BUFF_SIZE)) {
      /*FIXME need to get rid of Request here since now COAP depends on REST layer.*/
      coap_packet_t* request = (coap_packet_t*)allocate_buffer(sizeof(coap_packet_t));
      parse_message(request, (uint8_t*)data, datalen);

      if(request->type != MESSAGE_TYPE_ACK) {
        coap_packet_t* response = (coap_packet_t*)allocate_buffer(sizeof(coap_packet_t));
        init_response(request, response);

        if (service_cbk) {
          service_cbk(request, response);
        }

        data_size = serialize_packet(response, buf);
      }
      delete_buffer();
    } else {
      PRINTF("Memory Alloc Error\n");
      error = MEMORY_ALLOC_ERR;
      /*FIXME : Crappy way of accessing TID of the incoming packet, fix it!*/
      coap_packet_t error_packet;
      fill_error_packet(&error_packet,error, (data[2] << 8) + data[3]);
      data_size = serialize_packet(&error_packet, buf);
    }

    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    server_conn->rport = UIP_UDP_BUF->srcport;

    PRINTF("Responding with message size: %d",data_size);
    uip_udp_packet_send(server_conn, buf, data_size);
    /* Restore server connection to allow data from any node */
    memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
    server_conn->rport = 0;
  }

  return error;
}

/*---------------------------------------------------------------------------*/

PROCESS(coap_server, "Coap Server");
PROCESS_THREAD(coap_server, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("COAP SERVER\n");

/* if static routes are used rather than RPL */
#if !UIP_CONF_IPV6_RPL
  set_global_address();
  configure_routing();
#endif /*!UIP_CONF_IPV6_RPL*/

  /* new connection with remote host */
  server_conn = udp_new(NULL, HTONS(0), NULL);
  udp_bind(server_conn, HTONS(MOTE_PORT));
  PRINTF("Local/remote port %u/%u\n",
  HTONS(server_conn->lport), HTONS(server_conn->rport));

  while(1) {
    PROCESS_YIELD();

    if(ev == tcpip_event) {
      handle_data();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
