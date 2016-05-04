/*
 * Copyright (c) 2015, Yanzi Networks AB.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \addtogroup oma-lwm2m
 * @{
 */

/**
 * \file
 *         Implementation of the Contiki OMA LWM2M engine
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include "contiki.h"
#include "lwm2m-engine.h"
#include "lwm2m-object.h"
#include "lwm2m-device.h"
#include "lwm2m-plain-text.h"
#include "rest-engine.h"
#include "er-coap-constants.h"
#include "er-coap-engine.h"
#include "oma-tlv.h"
#include "oma-tlv-reader.h"
#include "oma-tlv-writer.h"
#include "net/ipv6/uip-ds6.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#if UIP_CONF_IPV6_RPL
#include "net/rpl/rpl.h"
#endif /* UIP_CONF_IPV6_RPL */

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

#ifndef LWM2M_ENGINE_CLIENT_ENDPOINT_PREFIX
#ifdef LWM2M_DEVICE_MODEL_NUMBER
#define LWM2M_ENGINE_CLIENT_ENDPOINT_PREFIX LWM2M_DEVICE_MODEL_NUMBER
#else /* LWM2M_DEVICE_MODEL_NUMBER */
#define LWM2M_ENGINE_CLIENT_ENDPOINT_PREFIX "Contiki-"
#endif /* LWM2M_DEVICE_MODEL_NUMBER */
#endif /* LWM2M_ENGINE_CLIENT_ENDPOINT_PREFIX */

#ifdef LWM2M_ENGINE_CONF_MAX_OBJECTS
#define MAX_OBJECTS LWM2M_ENGINE_CONF_MAX_OBJECTS
#else /* LWM2M_ENGINE_CONF_MAX_OBJECTS */
#define MAX_OBJECTS 10
#endif /* LWM2M_ENGINE_CONF_MAX_OBJECTS */

#define REMOTE_PORT        UIP_HTONS(COAP_DEFAULT_PORT)
#define BS_REMOTE_PORT     UIP_HTONS(5685)

static const lwm2m_object_t *objects[MAX_OBJECTS];
static char endpoint[32];
static char rd_data[128]; /* allocate some data for the RD */

PROCESS(lwm2m_rd_client, "LWM2M Engine");

static uip_ipaddr_t server_ipaddr;
static uint16_t server_port = REMOTE_PORT;
static uip_ipaddr_t bs_server_ipaddr;
static uint16_t bs_server_port = BS_REMOTE_PORT;

static uint8_t use_bootstrap = 0;
static uint8_t has_bootstrap_server_info = 0;
static uint8_t use_registration = 0;
static uint8_t has_registration_server_info = 0;
static uint8_t registered = 0;
static uint8_t bootstrapped = 0; /* bootstrap made... */

void lwm2m_device_init(void);
void lwm2m_security_init(void);
void lwm2m_server_init(void);

static const lwm2m_instance_t *get_first_instance_of_object(uint16_t id, lwm2m_context_t *context);
static const lwm2m_instance_t *get_instance(const lwm2m_object_t *object, lwm2m_context_t *context, int depth);
static const lwm2m_resource_t *get_resource(const lwm2m_instance_t *instance, lwm2m_context_t *context);
/*---------------------------------------------------------------------------*/
static void
client_chunk_handler(void *response)
{
#if (DEBUG) & DEBUG_PRINT
  const uint8_t *chunk;

  int len = coap_get_payload(response, &chunk);

  PRINTF("|%.*s\n", len, (char *)chunk);
#endif /* (DEBUG) & DEBUG_PRINT */
}
/*---------------------------------------------------------------------------*/
static int
index_of(const uint8_t *data, int offset, int len, uint8_t c)
{
  if(offset < 0) {
    return offset;
  }
  for(; offset < len; offset++) {
    if(data[offset] == c) {
      return offset;
    }
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int
has_network_access(void)
{
#if UIP_CONF_IPV6_RPL
  if(rpl_get_any_dag() == NULL) {
    return 0;
  }
#endif /* UIP_CONF_IPV6_RPL */
  return 1;
}
/*---------------------------------------------------------------------------*/
void
lwm2m_engine_use_bootstrap_server(int use)
{
  use_bootstrap = use != 0;
  if(use_bootstrap) {
    process_poll(&lwm2m_rd_client);
  }
}
/*---------------------------------------------------------------------------*/
void
lwm2m_engine_use_registration_server(int use)
{
  use_registration = use != 0;
  if(use_registration) {
    process_poll(&lwm2m_rd_client);
  }
}
/*---------------------------------------------------------------------------*/
void
lwm2m_engine_register_with_server(const uip_ipaddr_t *server, uint16_t port)
{
  uip_ipaddr_copy(&server_ipaddr, server);
  if(port != 0) {
    server_port = port;
  } else {
    server_port = REMOTE_PORT;
  }
  has_registration_server_info = 1;
  registered = 0;
  if(use_registration) {
    process_poll(&lwm2m_rd_client);
  }
}
/*---------------------------------------------------------------------------*/
static int
update_registration_server(void)
{
  if(has_registration_server_info) {
    return 1;
  }

#if UIP_CONF_IPV6_RPL
  {
    rpl_dag_t *dag;

    /* Use the DAG id as server address if no other has been specified */
    dag = rpl_get_any_dag();
    if(dag != NULL) {
      uip_ipaddr_copy(&server_ipaddr, &dag->dag_id);
      server_port = REMOTE_PORT;
      return 1;
    }
  }
#endif /* UIP_CONF_IPV6_RPL */

  return 0;
}
/*---------------------------------------------------------------------------*/
void
lwm2m_engine_register_with_bootstrap_server(const uip_ipaddr_t *server,
                                            uint16_t port)
{
  uip_ipaddr_copy(&bs_server_ipaddr, server);
  if(port != 0) {
    bs_server_port = port;
  } else {
    bs_server_port = BS_REMOTE_PORT;
  }
  has_bootstrap_server_info = 1;
  bootstrapped = 0;
  registered = 0;
  if(use_bootstrap) {
    process_poll(&lwm2m_rd_client);
  }
}
/*---------------------------------------------------------------------------*/
static int
update_bootstrap_server(void)
{
  if(has_bootstrap_server_info) {
    return 1;
  }

#if UIP_CONF_IPV6_RPL
  {
    rpl_dag_t *dag;

    /* Use the DAG id as server address if no other has been specified */
    dag = rpl_get_any_dag();
    if(dag != NULL) {
      uip_ipaddr_copy(&bs_server_ipaddr, &dag->dag_id);
      bs_server_port = REMOTE_PORT;
      return 1;
    }
  }
#endif /* UIP_CONF_IPV6_RPL */

  return 0;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(lwm2m_rd_client, ev, data)
{
  static coap_packet_t request[1];      /* This way the packet can be treated as pointer as usual. */
  static struct etimer et;

  PROCESS_BEGIN();

  printf("RD Client started with endpoint '%s'\n", endpoint);

  etimer_set(&et, 15 * CLOCK_SECOND);

  while(1) {
    PROCESS_YIELD();

    if(etimer_expired(&et)) {
      if(!has_network_access()) {
        /* Wait until for a network to join */
      } else if(use_bootstrap && bootstrapped == 0) {
        if(update_bootstrap_server()) {
          /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
          coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
          coap_set_header_uri_path(request, "/bs");
          coap_set_header_uri_query(request, endpoint);

          printf("Registering ID with bootstrap server [");
          uip_debug_ipaddr_print(&bs_server_ipaddr);
          printf("]:%u as '%s'\n", uip_ntohs(bs_server_port), endpoint);

          COAP_BLOCKING_REQUEST(&bs_server_ipaddr, bs_server_port, request,
                                client_chunk_handler);
          bootstrapped++;
        }
      } else if(use_bootstrap && bootstrapped == 1) {
        lwm2m_context_t context;
        const lwm2m_instance_t *instance = NULL;
        const lwm2m_resource_t *rsc;
        const uint8_t *first;
        int len;

        PRINTF("*** Bootstrap - checking for server info...\n");

        /* get the security object */
        instance = get_first_instance_of_object(LWM2M_OBJECT_SECURITY_ID, &context);
        if(instance != NULL) {
          /* get the server URI */
          context.resource_id = LWM2M_SECURITY_SERVER_URI;
          rsc = get_resource(instance, &context);
          first = lwm2m_object_get_resource_string(rsc, &context);
          len = lwm2m_object_get_resource_strlen(rsc, &context);
          if(first != NULL && len > 0) {
            int start, end;
            uip_ipaddr_t addr;
            int32_t port;
            uint8_t secure = 0;

            PRINTF("**** Found security instance using: %.*s\n", len, first);
            /* TODO Should verify it is a URI */

            /* Check if secure */
            secure = strncmp((const char *)first, "coaps:", 6) == 0;

            /* Only IPv6 supported */
            start = index_of(first, 0, len, '[');
            end = index_of(first, start, len, ']');
            if(start > 0 && end > start &&
               uiplib_ipaddrconv((const char *)&first[start], &addr)) {
              if(first[end + 1] == ':' &&
                 lwm2m_plain_text_read_int(first + end + 2, len - end - 2, &port)) {
              } else if(secure) {
                /**
                 * Secure CoAP should use a different port but for now
                 * the same port is used.
                 */
                port = COAP_DEFAULT_PORT;
              } else {
                port = COAP_DEFAULT_PORT;
              }
              PRINTF("Server address ");
              PRINT6ADDR(&addr);
              PRINTF(" port %" PRId32 "%s\n", port, secure ? " (secure)" : "");
              if(secure) {
                printf("Secure CoAP requested but not supported - can not bootstrap\n");
              } else {
                lwm2m_engine_register_with_server(&addr,
                                                  UIP_HTONS((uint16_t)port));
                bootstrapped++;
              }
            } else {
              printf("** failed to parse URI %.*s\n", len, first);
            }
          }
        }

        if(bootstrapped == 1) {
          /* Not ready. Lets retry with the bootstrap server again */
          bootstrapped = 0;
        }

      } else if(use_registration && !registered &&
                update_registration_server()) {
        int pos;
        int len, i, j;
        registered = 1;

        /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
        coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
        coap_set_header_uri_path(request, "/rd");
        coap_set_header_uri_query(request, endpoint);

        /* generate the rd data */
        pos = 0;
        for(i = 0; i < MAX_OBJECTS; i++) {
          if(objects[i] != NULL) {
            for(j = 0; j < objects[i]->count; j++) {
              if(objects[i]->instances[j].flag & LWM2M_INSTANCE_FLAG_USED) {
                len = snprintf(&rd_data[pos], sizeof(rd_data) - pos,
                               "%s<%d/%d>", pos > 0 ? "," : "",
                               objects[i]->id, objects[i]->instances[j].id);
                if(len > 0 && len < sizeof(rd_data) - pos) {
                  pos += len;
                }
              }
            }
          }
        }

        coap_set_payload(request, (uint8_t *)rd_data, pos);

        printf("Registering with [");
        uip_debug_ipaddr_print(&server_ipaddr);
        printf("]:%u lwm2m endpoint '%s': '%.*s'\n", uip_ntohs(server_port),
               endpoint, pos, rd_data);
        COAP_BLOCKING_REQUEST(&server_ipaddr, server_port, request,
                              client_chunk_handler);
      }
      /* for now only register once...   registered = 0; */
      etimer_set(&et, 15 * CLOCK_SECOND);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
lwm2m_engine_init(void)
{
#ifdef LWM2M_ENGINE_CLIENT_ENDPOINT_NAME

  snprintf(endpoint, sizeof(endpoint) - 1,
           "?ep=" LWM2M_ENGINE_CLIENT_ENDPOINT_NAME);

#else /* LWM2M_ENGINE_CLIENT_ENDPOINT_NAME */

  int len, i;
  uint8_t state;
  uip_ipaddr_t *ipaddr;
  char client[sizeof(endpoint)];

  len = strlen(LWM2M_ENGINE_CLIENT_ENDPOINT_PREFIX);
  /* ensure that this fits with the hex-nums */
  if(len > sizeof(client) - 13) {
    len = sizeof(client) - 13;
  }
  memcpy(client, LWM2M_ENGINE_CLIENT_ENDPOINT_PREFIX, len);

  /* pick an IP address that is PREFERRED or TENTATIVE */
  ipaddr = NULL;
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      ipaddr = &(uip_ds6_if.addr_list[i]).ipaddr;
      break;
    }
  }

  if(ipaddr != NULL) {
    for(i = 0; i < 6; i++) {
      /* assume IPv6 for now */
      uint8_t b = ipaddr->u8[10 + i];
      client[len++] = (b >> 4) > 9 ? 'A' - 10 + (b >> 4) : '0' + (b >> 4);
      client[len++] = (b & 0xf) > 9 ? 'A' - 10 + (b & 0xf) : '0' + (b & 0xf);
    }
  }

  /* a zero at end of string */
  client[len] = 0;
  /* create endpoint */
  snprintf(endpoint, sizeof(endpoint) - 1, "?ep=%s", client);

#endif /* LWM2M_ENGINE_CLIENT_ENDPOINT_NAME */

  rest_init_engine();
  process_start(&lwm2m_rd_client, NULL);
}
/*---------------------------------------------------------------------------*/
void
lwm2m_engine_register_default_objects(void)
{
  lwm2m_security_init();
  lwm2m_server_init();
  lwm2m_device_init();
}
/*---------------------------------------------------------------------------*/
static int
parse_next(const char **path, int *path_len, uint16_t *value)
{
  char c;
  *value = 0;
  /* printf("parse_next: %p %d\n", *path, *path_len); */
  if(*path_len == 0) {
    return 0;
  }
  while(*path_len > 0) {
    c = **path;
    (*path)++;
    *path_len = *path_len - 1;
    if(c >= '0' && c <= '9') {
      *value = *value * 10 + (c - '0');
    } else if(c == '/') {
      return 1;
    } else {
      /* error */
      return -4;
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
int
lwm2m_engine_parse_context(const lwm2m_object_t *object,
                           const char *path, int path_len,
                           lwm2m_context_t *context)
{
  int ret;
  if(context == NULL || object == NULL || path == NULL) {
    return 0;
  }
  memset(context, 0, sizeof(lwm2m_context_t));
  /* get object id */
  ret = 0;
  ret += parse_next(&path, &path_len, &context->object_id);
  ret += parse_next(&path, &path_len, &context->object_instance_id);
  ret += parse_next(&path, &path_len, &context->resource_id);

  /* Set default reader/writer */
  context->reader = &lwm2m_plain_text_reader;
  context->writer = &oma_tlv_writer;

  return ret;
}
/*---------------------------------------------------------------------------*/
const lwm2m_object_t *
lwm2m_engine_get_object(uint16_t id)
{
  int i;
  for(i = 0; i < MAX_OBJECTS; i++) {
    if(objects[i] != NULL && objects[i]->id == id) {
      return objects[i];
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
int
lwm2m_engine_register_object(const lwm2m_object_t *object)
{
  int i;
  int found = 0;
  for(i = 0; i < MAX_OBJECTS; i++) {
    if(objects[i] == NULL) {
      objects[i] = object;
      found = 1;
      break;
    }
  }
  rest_activate_resource(lwm2m_object_get_coap_resource(object),
                         (char *)object->path);
  return found;
}
/*---------------------------------------------------------------------------*/
static const lwm2m_instance_t *
get_first_instance_of_object(uint16_t id, lwm2m_context_t *context)
{
  const lwm2m_object_t *object;
  int i;

  object = lwm2m_engine_get_object(id);
  if(object == NULL) {
    /* No object with the specified id found */
    return NULL;
  }

  /* Initialize the context */
  memset(context, 0, sizeof(lwm2m_context_t));
  context->object_id = id;

  for(i = 0; i < object->count; i++) {
    if(object->instances[i].flag & LWM2M_INSTANCE_FLAG_USED) {
      context->object_instance_id = object->instances[i].id;
      context->object_instance_index = i;
      return &object->instances[i];
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static const lwm2m_instance_t *
get_instance(const lwm2m_object_t *object, lwm2m_context_t *context, int depth)
{
  int i;
  if(depth > 1) {
    PRINTF("lwm2m: searching for instance %u\n", context->object_instance_id);
    for(i = 0; i < object->count; i++) {
      PRINTF("  Instance %d -> %u (used: %d)\n", i, object->instances[i].id,
             (object->instances[i].flag & LWM2M_INSTANCE_FLAG_USED) != 0);
      if(object->instances[i].id == context->object_instance_id &&
         object->instances[i].flag & LWM2M_INSTANCE_FLAG_USED) {
        context->object_instance_index = i;
        return &object->instances[i];
      }
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static const lwm2m_resource_t *
get_resource(const lwm2m_instance_t *instance, lwm2m_context_t *context)
{
  int i;
  if(instance != NULL) {
    PRINTF("lwm2m: searching for resource %u\n", context->resource_id);
    for(i = 0; i < instance->count; i++) {
      PRINTF("  Resource %d -> %u\n", i, instance->resources[i].id);
      if(instance->resources[i].id == context->resource_id) {
        context->resource_index = i;
        return &instance->resources[i];
      }
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static int
write_rd_link_data(const lwm2m_object_t *object,
                   const lwm2m_instance_t *instance,
                   char *buffer, size_t size)
{
  const lwm2m_resource_t *resource;
  int len, rdlen, i;

  PRINTF("<%d/%d>", object->id, instance->id);
  rdlen = snprintf(buffer, size, "<%d/%d>",
                   object->id, instance->id);
  if(rdlen < 0 || rdlen >= size) {
    return -1;
  }

  for(i = 0; i < instance->count; i++) {
    resource = &instance->resources[i];
    PRINTF(",<%d/%d/%d>", object->id, instance->id, resource->id);

    len = snprintf(&buffer[rdlen], size - rdlen,
                   ",<%d/%d/%d>", object->id, instance->id, resource->id);
    rdlen += len;
    if(len < 0 || rdlen >= size) {
      return -1;
    }
  }
  return rdlen;
}
/*---------------------------------------------------------------------------*/
static int
write_rd_json_data(const lwm2m_context_t *context,
                   const lwm2m_object_t *object,
                   const lwm2m_instance_t *instance,
                   char *buffer, size_t size)
{
  const lwm2m_resource_t *resource;
  const char *s = "";
  int len, rdlen, i;

  PRINTF("{\"e\":[");
  rdlen = snprintf(buffer, size, "{\"e\":[");
  if(rdlen < 0 || rdlen >= size) {
    return -1;
  }

  for(i = 0, len = 0; i < instance->count; i++) {
    resource = &instance->resources[i];
    len = 0;
    if(lwm2m_object_is_resource_string(resource)) {
      const uint8_t *value;
      uint16_t slen;
      value = lwm2m_object_get_resource_string(resource, context);
      slen = lwm2m_object_get_resource_strlen(resource, context);
      if(value != NULL) {
        PRINTF("%s{\"n\":\"%u\",\"sv\":\"%.*s\"}", s,
               resource->id, slen, value);
        len = snprintf(&buffer[rdlen], size - rdlen,
                       "%s{\"n\":\"%u\",\"sv\":\"%.*s\"}", s,
                       resource->id, slen, value);
      }
    } else if(lwm2m_object_is_resource_int(resource)) {
      int32_t value;
      if(lwm2m_object_get_resource_int(resource, context, &value)) {
        PRINTF("%s{\"n\":\"%u\",\"v\":%" PRId32 "}", s,
               resource->id, value);
        len = snprintf(&buffer[rdlen], size - rdlen,
                       "%s{\"n\":\"%u\",\"v\":%" PRId32 "}", s,
                       resource->id, value);
      }
    } else if(lwm2m_object_is_resource_floatfix(resource)) {
      int32_t value;
      if(lwm2m_object_get_resource_floatfix(resource, context, &value)) {
        PRINTF("%s{\"n\":\"%u\",\"v\":%" PRId32 "}", s, resource->id,
               value / LWM2M_FLOAT32_FRAC);
        len = snprintf(&buffer[rdlen], size - rdlen,
                       "%s{\"n\":\"%u\",\"v\":", s, resource->id);
        rdlen += len;
        if(len < 0 || rdlen >= size) {
          return -1;
        }

        len = lwm2m_plain_text_write_float32fix((uint8_t *)&buffer[rdlen],
                                                size - rdlen,
                                                value, LWM2M_FLOAT32_BITS);
        if(len == 0) {
          return -1;
        }
        rdlen += len;

        if(rdlen < size) {
          buffer[rdlen] = '}';
        }
        len = 1;
      }
    } else if(lwm2m_object_is_resource_boolean(resource)) {
      int value;
      if(lwm2m_object_get_resource_boolean(resource, context, &value)) {
        PRINTF("%s{\"n\":\"%u\",\"bv\":%s}", s, resource->id,
               value ? "true" : "false");
        len = snprintf(&buffer[rdlen], size - rdlen,
                       "%s{\"n\":\"%u\",\"bv\":%s}", s, resource->id,
                       value ? "true" : "false");
      }
    }
    rdlen += len;
    if(len < 0 || rdlen >= size) {
      return -1;
    }
    if(len > 0) {
      s = ",";
    }
  }
  PRINTF("]}\n");
  len = snprintf(&buffer[rdlen], size - rdlen, "]}");
  rdlen += len;
  if(len < 0 || rdlen >= size) {
    return -1;
  }

  return rdlen;
}
/*---------------------------------------------------------------------------*/
/**
 * @brief  Set the writer pointer to the proper writer based on the Accept: header
 *
 * @param[in] context  LWM2M context to operate on
 * @param[in] accept   Accept type number from CoAP headers
 *
 * @return The content type of the response if the selected writer is used
 */
static unsigned int
lwm2m_engine_select_writer(lwm2m_context_t *context, unsigned int accept)
{
  switch(accept) {
    case LWM2M_TLV:
      context->writer = &oma_tlv_writer;
      break;
    case LWM2M_TEXT_PLAIN:
    case TEXT_PLAIN:
      context->writer = &lwm2m_plain_text_writer;
      break;
    default:
      PRINTF("Unknown Accept type %u, using LWM2M plain text\n", accept);
      context->writer = &lwm2m_plain_text_writer;
      /* Set the response type to plain text */
      accept = LWM2M_TEXT_PLAIN;
      break;
  }
  return accept;
}
/*---------------------------------------------------------------------------*/
/**
 * @brief  Set the reader pointer to the proper reader based on the Content-format: header
 *
 * @param[in] context        LWM2M context to operate on
 * @param[in] content_format Content-type type number from CoAP headers
 */
static void
lwm2m_engine_select_reader(lwm2m_context_t *context, unsigned int content_format)
{
  switch(content_format) {
    case LWM2M_TLV:
      context->reader = &oma_tlv_reader;
      break;
    case LWM2M_TEXT_PLAIN:
    case TEXT_PLAIN:
      context->reader = &lwm2m_plain_text_reader;
      break;
    default:
      PRINTF("Unknown content type %u, using LWM2M plain text\n", accept);
      context->reader = &lwm2m_plain_text_reader;
      break;
  }
}
/*---------------------------------------------------------------------------*/
void
lwm2m_engine_handler(const lwm2m_object_t *object,
                     void *request, void *response,
                     uint8_t *buffer, uint16_t preferred_size,
                     int32_t *offset)
{
  int len;
  const char *url;
  unsigned int format;
  unsigned int accept;
  unsigned int content_type;
  int depth;
  lwm2m_context_t context;
  rest_resource_flags_t method;
  const lwm2m_instance_t *instance;
#if (DEBUG) & DEBUG_PRINT
  const char *method_str;
#endif /* (DEBUG) & DEBUG_PRINT */

  method = REST.get_method_type(request);

  len = REST.get_url(request, &url);
  if(!REST.get_header_content_type(request, &format)) {
    PRINTF("No format given. Assume text plain...\n");
    format = LWM2M_TEXT_PLAIN;
  } else if(format == TEXT_PLAIN) {
    /* CoAP content format text plain - assume LWM2M text plain */
    format = LWM2M_TEXT_PLAIN;
  }
  if(!REST.get_header_accept(request, &accept)) {
    PRINTF("No Accept header, using same as Content-format...\n");
    accept = format;
  }

  depth = lwm2m_engine_parse_context(object, url, len, &context);
  PRINTF("Context: %u/%u/%u  found: %d\n", context.object_id,
         context.object_instance_id, context.resource_id, depth);

  /* Select reader and writer based on provided Content type and Accept headers */
  lwm2m_engine_select_reader(&context, format);
  content_type = lwm2m_engine_select_writer(&context, accept);

#if (DEBUG) & DEBUG_PRINT
  /* for debugging */
  if(method == METHOD_GET) {
    method_str = "GET";
  } else if(method == METHOD_POST) {
    method_str = "POST";
  } else if(method == METHOD_PUT) {
    method_str = "PUT";
  } else if(method == METHOD_DELETE) {
    method_str = "DELETE";
  } else {
    method_str = "UNKNOWN";
  }
  PRINTF("%s Called Path:%.*s Format:%d ID:%d bsize:%u\n", method_str, len,
         url, format, object->id, preferred_size);
  if(format == LWM2M_TEXT_PLAIN) {
    /* a string */
    const uint8_t *data;
    int plen = REST.get_request_payload(request, &data);
    if(plen > 0) {
      PRINTF("Data: '%.*s'\n", plen, (char *)data);
    }
  }
#endif /* (DEBUG) & DEBUG_PRINT */

  instance = get_instance(object, &context, depth);

  /* from POST */
  if(depth > 1 && instance == NULL) {
    if(method != METHOD_PUT && method != METHOD_POST) {
      PRINTF("Error - do not have instance %d\n", context.object_instance_id);
      REST.set_response_status(response, NOT_FOUND_4_04);
      return;
    } else {
      const uint8_t *data;
      int i, len, plen, pos;
      oma_tlv_t tlv;
      PRINTF(">>> CREATE ? %d/%d\n", context.object_id,
             context.object_instance_id);

      for(i = 0; i < object->count; i++) {
        if((object->instances[i].flag & LWM2M_INSTANCE_FLAG_USED) == 0) {
          /* allocate this instance */
          object->instances[i].flag |= LWM2M_INSTANCE_FLAG_USED;
          object->instances[i].id = context.object_instance_id;
          context.object_instance_index = i;
          PRINTF("Created instance: %d\n", context.object_instance_id);
          REST.set_response_status(response, CREATED_2_01);
          instance = &object->instances[i];
          break;
        }
      }

      if(instance == NULL) {
        /* could for some reason not create the instance */
        REST.set_response_status(response, NOT_ACCEPTABLE_4_06);
        return;
      }

      plen = REST.get_request_payload(request, &data);
      if(plen == 0) {
        /* do nothing more */
        return;
      }
      PRINTF("Payload: ");
      for(i = 0; i < plen; i++) {
        PRINTF("%02x", data[i]);
      }
      PRINTF("\n");

      pos = 0;
      do {
        len = oma_tlv_read(&tlv, (uint8_t *)&data[pos], plen - pos);
        PRINTF("Found TLV type=%u id=%u len=%lu\n",
               tlv.type, tlv.id, (unsigned long)tlv.length);
        /* here we need to do callbacks or write value */
        if(tlv.type == OMA_TLV_TYPE_RESOURCE) {
          context.resource_id = tlv.id;
          const lwm2m_resource_t *rsc = get_resource(instance, &context);
          if(rsc != NULL) {
            /* write the value to the resource */
            if(lwm2m_object_is_resource_string(rsc)) {
              PRINTF("  new string value for /%d/%d/%d = %.*s\n",
                     context.object_id, context.object_instance_id,
                     context.resource_id, (int)tlv.length, tlv.value);
              lwm2m_object_set_resource_string(rsc, &context,
                                               tlv.length, tlv.value);
            } else if(lwm2m_object_is_resource_int(rsc)) {
              PRINTF("  new int value for /%d/%d/%d = %" PRId32 "\n",
                     context.object_id, context.object_instance_id,
                     context.resource_id, oma_tlv_get_int32(&tlv));
              lwm2m_object_set_resource_int(rsc, &context,
                                            oma_tlv_get_int32(&tlv));
            } else if(lwm2m_object_is_resource_floatfix(rsc)) {
              int32_t value;
              if(oma_tlv_float32_to_fix(&tlv, &value, LWM2M_FLOAT32_BITS)) {
                PRINTF("  new float value for /%d/%d/%d = %" PRId32 "\n",
                     context.object_id, context.object_instance_id,
                     context.resource_id, value >> LWM2M_FLOAT32_BITS);
                lwm2m_object_set_resource_floatfix(rsc, &context, value);
              } else {
                PRINTF("  new float value for /%d/%d/%d: FAILED\n",
                     context.object_id, context.object_instance_id,
                     context.resource_id);
              }
            } else if(lwm2m_object_is_resource_boolean(rsc)) {
              PRINTF("  new boolean value for /%d/%d/%d = %" PRId32 "\n",
                     context.object_id, context.object_instance_id,
                     context.resource_id, oma_tlv_get_int32(&tlv));
              lwm2m_object_set_resource_boolean(rsc, &context,
                                                oma_tlv_get_int32(&tlv) != 0);
            }
          }
        }
        pos = pos + len;
      } while(len > 0 && pos < plen);
    }
    return;
  }

  if(depth == 3) {
    const lwm2m_resource_t *resource = get_resource(instance, &context);
    size_t content_len = 0;
    if(resource == NULL) {
      PRINTF("Error - do not have resource %d\n", context.resource_id);
      REST.set_response_status(response, NOT_FOUND_4_04);
      return;
    }
    /* HANDLE PUT */
    if(method == METHOD_PUT) {
      if(lwm2m_object_is_resource_callback(resource)) {
        if(resource->value.callback.write != NULL) {
          /* pick a reader ??? */
          if(format == LWM2M_TEXT_PLAIN) {
            /* a string */
            const uint8_t *data;
            int plen = REST.get_request_payload(request, &data);
            context.reader = &lwm2m_plain_text_reader;
            PRINTF("PUT Callback with data: '%.*s'\n", plen, data);
            /* no specific reader for plain text */
            content_len = resource->value.callback.write(&context, data, plen,
                                                    buffer, preferred_size);
            PRINTF("content_len:%u\n", (unsigned int)content_len);
            REST.set_response_status(response, CHANGED_2_04);
          } else {
            PRINTF("PUT callback with format %d\n", format);
            REST.set_response_status(response, NOT_ACCEPTABLE_4_06);
          }
        } else {
          PRINTF("PUT - no write callback\n");
          REST.set_response_status(response, METHOD_NOT_ALLOWED_4_05);
        }
      } else {
        PRINTF("PUT on non-callback resource!\n");
        REST.set_response_status(response, METHOD_NOT_ALLOWED_4_05);
      }
      /* HANDLE GET */
    } else if(method == METHOD_GET) {
      if(lwm2m_object_is_resource_string(resource)) {
        const uint8_t *value;
        value = lwm2m_object_get_resource_string(resource, &context);
        if(value != NULL) {
          uint16_t len = lwm2m_object_get_resource_strlen(resource, &context);
          PRINTF("Get string value: %.*s\n", (int)len, (char *)value);
          content_len = context.writer->write_string(&context, buffer,
            preferred_size, (const char *)value, len);
        }
      } else if(lwm2m_object_is_resource_int(resource)) {
        int32_t value;
        if(lwm2m_object_get_resource_int(resource, &context, &value)) {
          content_len = context.writer->write_int(&context, buffer, preferred_size, value);
        }
      } else if(lwm2m_object_is_resource_floatfix(resource)) {
        int32_t value;
        if(lwm2m_object_get_resource_floatfix(resource, &context, &value)) {
          /* export FLOATFIX */
          PRINTF("Exporting %d-bit fix as float: %" PRId32 "\n",
                 LWM2M_FLOAT32_BITS, value);
          content_len = context.writer->write_float32fix(&context, buffer,
            preferred_size, value, LWM2M_FLOAT32_BITS);
        }
      } else if(lwm2m_object_is_resource_callback(resource)) {
        if(resource->value.callback.read != NULL) {
          content_len = resource->value.callback.read(&context,
                                                 buffer, preferred_size);
        } else {
          REST.set_response_status(response, METHOD_NOT_ALLOWED_4_05);
          return;
        }
      }
      if(content_len > 0) {
        REST.set_response_payload(response, buffer, content_len);
        REST.set_header_content_type(response, content_type);
      } else {
        /* failed to produce output - it is an internal error */
        REST.set_response_status(response, INTERNAL_SERVER_ERROR_5_00);
      }
      /* Handle POST */
    } else if(method == METHOD_POST) {
      if(lwm2m_object_is_resource_callback(resource)) {
        if(resource->value.callback.exec != NULL) {
          const uint8_t *data;
          int plen = REST.get_request_payload(request, &data);
          PRINTF("Execute Callback with data: '%.*s'\n", plen, data);
          content_len = resource->value.callback.exec(&context,
                                                 data, plen,
                                                 buffer, preferred_size);
          REST.set_response_status(response, CHANGED_2_04);
        } else {
          PRINTF("Execute callback - no exec callback\n");
          REST.set_response_status(response, METHOD_NOT_ALLOWED_4_05);
        }
      } else {
        PRINTF("Resource post but no callback resource\n");
        REST.set_response_status(response, METHOD_NOT_ALLOWED_4_05);
      }
    }
  } else if(depth == 2) {
    /* produce an instance response */
    if(method != METHOD_GET) {
      REST.set_response_status(response, METHOD_NOT_ALLOWED_4_05);
    } else if(instance == NULL) {
      REST.set_response_status(response, NOT_FOUND_4_04);
    } else {
      int rdlen;
      if(accept == APPLICATION_LINK_FORMAT) {
        rdlen = write_rd_link_data(object, instance,
                                   (char *)buffer, preferred_size);
      } else {
        rdlen = write_rd_json_data(&context, object, instance,
                                   (char *)buffer, preferred_size);
      }
      if(rdlen < 0) {
        PRINTF("Failed to generate instance response\n");
        REST.set_response_status(response, SERVICE_UNAVAILABLE_5_03);
        return;
      }
      REST.set_response_payload(response, buffer, rdlen);
      if(accept == APPLICATION_LINK_FORMAT) {
        REST.set_header_content_type(response, REST.type.APPLICATION_LINK_FORMAT);
      } else {
        REST.set_header_content_type(response, LWM2M_JSON);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void
lwm2m_engine_delete_handler(const lwm2m_object_t *object, void *request,
                            void *response, uint8_t *buffer,
                            uint16_t preferred_size, int32_t *offset)
{
  int len;
  const char *url;
  lwm2m_context_t context;

  len = REST.get_url(request, &url);
  PRINTF("*** DELETE URI:'%.*s' called... - responding with DELETED.\n",
         len, url);
  len = lwm2m_engine_parse_context(object, url, len, &context);
  PRINTF("Context: %u/%u/%u  found: %d\n", context.object_id,
         context.object_instance_id, context.resource_id, len);

  REST.set_response_status(response, DELETED_2_02);
}
/*---------------------------------------------------------------------------*/
/** @} */
