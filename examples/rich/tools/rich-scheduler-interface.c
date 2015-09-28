/*
 * Copyright (c) 2014, Swedish Institute of Computer Science.
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
 *         RICH CoAP scheduler interface
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */


#include "contiki.h"
#include "net/rpl/rpl.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/rime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "erbium.h"
#include "er-coap-13.h"
#include "er-coap-13-observing.h"
#include "jsontree.h"
#include "jsonparse.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define SM_UPDATE_INTERVAL (60 * CLOCK_SECOND)

static coap_method_t
coap_get_method(coap_packet_t* packet)
{
  return (coap_method_t)packet->code;
}

static char content[REST_MAX_CHUNK_SIZE];
static int content_len = 0;

#define CONTENT_PRINTF(...) { if(content_len < sizeof(content)) content_len += snprintf(content+content_len, sizeof(content)-content_len, __VA_ARGS__); }

/* Utility function for json parsing */
int
jsonparse_find_field(struct jsonparse_state *js,
    char *field_buf, int field_buf_len
  )
{
  int state;
  while((state=jsonparse_next(js))) {
    switch(state) {
      case JSON_TYPE_PAIR_NAME:
        jsonparse_copy_value(js, field_buf, field_buf_len);
        /* Move to ":" */
        jsonparse_next(js);
        /* Move to value and return its type */
        return jsonparse_next(js);
      default:
        return state;
    }
  }
  return 0;
}

/* Utility function. Converts na field (string containing the lower 64bit of the IPv6) to
 * 64-bit MAC. */
static int
na_to_rimeaddr(const char *na_inbuf, int bufsize, rimeaddr_t *rimeaddr) {
  int i;
  char next_end_char = ':';
  const char *na_inbuf_end = na_inbuf + bufsize - 1;
  char *end;
  unsigned val;
  for(i=0; i<4; i++) {
    if(na_inbuf >= na_inbuf_end) {
      return 0;
    }
    if(i == 3) {
      next_end_char = '\0';
    }
    val = (unsigned)strtoul(na_inbuf, &end, 16);
    /* Check convertion */
    if(end != na_inbuf && *end == next_end_char && errno != ERANGE) {
      rimeaddr->u8[2*i] = val >> 8;
      rimeaddr->u8[2*i+1] = val;
      na_inbuf = end+1;
    } else {
      return 0;
    }
  }
  /* We consider only links with IEEE EUI-64 identifier */
  rimeaddr->u8[0] ^= 0x02;
  return 1;
}

/* Statistic/metric data structure */
struct rich_sm {
  int so; /* slot offset */
  int co; /* channel offset */
  int fd; /* slotframeID (handle) */
  rimeaddr_t na; /* node address */
#define MT_PRR   1
#define MT_RSSI  2
#define MT_ETX   4
  int mt; /* metrics */
  int wi; /* window interval */
};
/* The currently installed statistic/metrics */
struct rich_sm current_sm;
int current_sm_id = -1;

/*********** RICH 6t/6/cl/ml/0 resource *************************************************/

EVENT_RESOURCE(sm0, METHOD_GET, "6t/6/ml/0", "title=\"Statistic/metric 0\";obs");

void
sm0_create_response(coap_packet_t *response)
{
  if(current_sm_id == 0) {
    uip_ds6_nbr_t *nbr;
    int first_item = 1;
    uip_ipaddr_t *last_next_hop = NULL;
    uip_ipaddr_t *curr_next_hop = NULL;

    content_len = 0;

    CONTENT_PRINTF("[");
    for(nbr = nbr_table_head(ds6_neighbors);
        nbr != NULL;
        nbr = nbr_table_next(ds6_neighbors, nbr)) {
      /* We rely on the fact that routes are ordered by next hop.
       * Loop over all loops and print every next hop exactly once. */
      curr_next_hop = uip_ds6_nbr_get_ipaddr(nbr);
      if(curr_next_hop != last_next_hop) {
        /* Matching the neighbor */
        if(rimeaddr_cmp(&current_sm.na, &rimeaddr_null)
            || rimeaddr_cmp(&current_sm.na, uip_ds6_nbr_get_ll(nbr))) {
          if(!first_item) {
            CONTENT_PRINTF(",");
          }
          first_item = 0;
          CONTENT_PRINTF("{\"%x:%x:%x:%x\":[",
              UIP_HTONS(curr_next_hop->u16[4]), UIP_HTONS(curr_next_hop->u16[5]),
              UIP_HTONS(curr_next_hop->u16[6]), UIP_HTONS(curr_next_hop->u16[7])
          );
          int first_mt = 1;
          if(current_sm.mt & MT_PRR) {
            if(!first_mt) {
              CONTENT_PRINTF(",");
            }
            first_mt = 0;
            CONTENT_PRINTF("{\"PRR\":%i}", nbr->prr);
          }
          if(current_sm.mt & MT_RSSI) {
            if(!first_mt) {
              CONTENT_PRINTF(",");
            }
            first_mt = 0;
            CONTENT_PRINTF("{\"RSSI\":%i}", nbr->rssi);
          }
          if(current_sm.mt & MT_ETX) {
            if(!first_mt) {
              CONTENT_PRINTF(",");
            }
            first_mt = 0;
            CONTENT_PRINTF("{\"ETX\":%i}", nbr->etx);
          }
          CONTENT_PRINTF("]}");
          last_next_hop = curr_next_hop;
        }
      }
    }
    CONTENT_PRINTF("]");

    coap_set_header_content_type(response, REST.type.APPLICATION_JSON);
    coap_set_payload(response, content, content_len);
  }
}

void
rich_sm0_notify_subscribers()
{
  if(current_sm_id == 0) {
    static uint16_t event_counter = 0;
    coap_packet_t notification;
    /* Build notification. */
    coap_init_message(&notification, COAP_TYPE_CON, REST.status.OK, 0 );
    sm0_create_response(&notification);
    /* Notify the registered observers with the given message type, observe option, and payload. */
    PRINTF("RICH: notify sm0 subscribers\n");
    coap_notify_observers(&resource_sm0, ++event_counter, &notification);
  }
}

void
sm0_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  if(current_sm_id == 0) {
    PRINTF("RICH: get sm0\n");
    sm0_create_response(response);
  } else {
    PRINTF("RICH: get sm0 -- not installed yet\n");
    const char *resp = "Statistic/metric 0 not installed yet";
    coap_init_message(response, COAP_TYPE_NON, REST.status.NOT_FOUND, 0 );
    coap_set_header_content_type(response, REST.type.TEXT_PLAIN);
    coap_set_payload(response, resp, strlen(resp));
  }
}

/*********** RICH 6t/6/sm resource *************************************************/
/* Stands for statistic metrics. Used to post a description of statistics one wants
 * to monitor later via observe */

RESOURCE(statisticsmetrics, METHOD_POST,
    "6t/6/sm", "title=\"StatisticMetrics\";");

void
statisticsmetrics_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content;
  int request_content_len;

  if(IS_OPTION((coap_packet_t *)request, COAP_OPTION_BLOCK1)) {
    return;
  }

  content_len = 0;

  if(coap_get_method(request) == COAP_POST) {
    char *content_ptr = content;
    struct jsonparse_state js;
    int state;

    request_content_len = coap_get_payload(request, &request_content);
    jsonparse_setup(&js, (const char *)request_content, request_content_len);

    /* Parse json input */
    struct rich_sm sm;
    char field_buf[32] = "";
    char value_buf[32] = "";

    /* Start creating response */
    int first_item = 1;
    int new_sm_count = 0;
    CONTENT_PRINTF("[");

    /* Parse json input */
    while((state=jsonparse_find_field(&js, field_buf, sizeof(field_buf)))) {

      switch(state) {
        case '{': /* New element */
          memset(&sm, 0, sizeof(sm));
          break;
        case '}': /* End of current element */
          current_sm_id = 0; /* We currently support only a single s/m installed */
          current_sm = sm;

          PRINTF("RICH: added statistics/metrics %u, so %u, co %u, fd %u, mt %u, wi %u\n",
              current_sm_id, sm.so, sm.co, sm.fd, sm.mt, sm.wi);
          PRINTF("RICH: link address: ");
          PRINTLLADDR(&sm.na);
          PRINTF("\n");

          /* Update response */
          new_sm_count++;
          if(!first_item) {
            CONTENT_PRINTF(",");
          }
          first_item = 0;
          CONTENT_PRINTF("{\"md\":%u}", current_sm_id);
          break;
        case JSON_TYPE_NUMBER:
          if(!strncmp(field_buf, "so", sizeof(field_buf))) {
            sm.so = jsonparse_get_value_as_int(&js);
          } else if(!strncmp(field_buf, "co", sizeof(field_buf))) {
            sm.co = jsonparse_get_value_as_int(&js);
          } else if(!strncmp(field_buf, "fd", sizeof(field_buf))) {
            sm.fd = jsonparse_get_value_as_int(&js);
          } else if(!strncmp(field_buf, "wi", sizeof(field_buf))) {
            sm.wi = jsonparse_get_value_as_int(&js);
          }
          break;
        case JSON_TYPE_STRING:
          if(!strncmp(field_buf, "na", sizeof(field_buf))) {
            jsonparse_copy_value(&js, value_buf, sizeof(value_buf));
            na_to_rimeaddr(value_buf, sizeof(value_buf), &sm.na);
          } else if(!strncmp(field_buf, "mt", sizeof(field_buf))) {
            jsonparse_copy_value(&js, value_buf, sizeof(value_buf));
            if(!strncmp(value_buf, "PRR", sizeof(value_buf))) {
              sm.mt |= MT_PRR;
            } else if(!strncmp(value_buf, "RSSI", sizeof(value_buf))) {
              sm.mt |= MT_RSSI;
            } else if(!strncmp(value_buf, "ETX", sizeof(value_buf))) {
              sm.mt |= MT_ETX;
            }
          }
          break;
      }
    }

    /* Check if json parsing succeeded */
    if(js.error == JSON_ERROR_OK) {
      if(new_sm_count > 1) {
        /* We have several s/m, we need to send an array */
        CONTENT_PRINTF("]");
        content_ptr = content;
      } else {
        /* We added only one s/m, don't send an array:
         * Start one byte farther and reduce content_len accordingly. */
        content_ptr = content + 1;
        content_len--;
      }

      coap_set_header_content_type(response, REST.type.APPLICATION_JSON);
      coap_set_payload(response, content_ptr, content_len);
    }
  }
}

/*********** RICH 6t/6/cl resource *************************************************/

RESOURCE(celllist, METHOD_POST | METHOD_DELETE | HAS_SUB_RESOURCES,
    "6t/6/cl", "title=\"Celllist\";");

void
celllist_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  int first_item = 1;
  const uint8_t *request_content;
  int request_content_len;

  if(IS_OPTION((coap_packet_t *)request, COAP_OPTION_BLOCK1)) {
    return;
  }

  content_len = 0;

  switch(coap_get_method(request)) {
    case COAP_POST: {
      char *content_ptr;
      int state;
      int new_link_count = 0; /* The number of newly added links */

      int so = 0; /* slot offset */
      int co = 0; /* channel offset */
      int fd = 0; /* slotframeID (handle) */
      int lo = 0; /* link options */
      int lt = 0; /* link type */
      rimeaddr_t na; /* node address */

      char field_buf[32] = "";
      char value_buf[32] = "";
      struct jsonparse_state js;
      /* Add new link */
      request_content_len = coap_get_payload(request, &request_content);

      jsonparse_setup(&js, (const char *)request_content, request_content_len);

      /* Start creating response */
      CONTENT_PRINTF("[");

      /* Parse json input */
      while((state=jsonparse_find_field(&js, field_buf, sizeof(field_buf)))) {

         switch(state) {
           case '{': /* New element */
             so = co = fd = lo = lt = 0;
             rimeaddr_copy(&na, &rimeaddr_null);
             break;
           case '}': { /* End of current element */
             struct tsch_slotframe *slotframe;
             struct tsch_link *link;
             slotframe = tsch_schedule_get_slotframe_by_handle(fd);
             if(slotframe) {
               /* Add link */
               if((link = tsch_schedule_add_link(slotframe, lo, lt, &na, so, co))) {
                 new_link_count++;
                 PRINTF("RICH: added link %u, so %u, co %u, fd %u, lo %u, lt %u\n",
                     link->handle, so, co, fd, lo, lt);
                 PRINTF("RICH: link address: ");
                 PRINTLLADDR(&na);
                 PRINTF("\n");
                 /* Update response */
                 if(!first_item) {
                   CONTENT_PRINTF(",");
                 }
                 first_item = 0;
                 CONTENT_PRINTF("{\"cd\":%u}", link->handle);
               }
             } else {
               PRINTF("RICH:! slotframe %u not found\n", fd);
             }
             break;
           }
           case JSON_TYPE_NUMBER:
             if(!strncmp(field_buf, "so", sizeof(field_buf))) {
               so = jsonparse_get_value_as_int(&js);
             } else if(!strncmp(field_buf, "co", sizeof(field_buf))) {
               co = jsonparse_get_value_as_int(&js);
             } else if(!strncmp(field_buf, "fd", sizeof(field_buf))) {
               fd = jsonparse_get_value_as_int(&js);
             } else if(!strncmp(field_buf, "lo", sizeof(field_buf))) {
               lo = jsonparse_get_value_as_int(&js);
             } else if(!strncmp(field_buf, "lt", sizeof(field_buf))) {
               lt = jsonparse_get_value_as_int(&js);
             }
             break;
           case JSON_TYPE_STRING:
             if(!strncmp(field_buf, "na", sizeof(field_buf))) {
               jsonparse_copy_value(&js, value_buf, sizeof(value_buf));
               na_to_rimeaddr(value_buf, sizeof(value_buf), &na);
             }
             break;
         }
       }

       /* Check if json parsing succeeded */
       if(js.error == JSON_ERROR_OK) {
         if(new_link_count > 1) {
           /* We have several links, we need to send an array */
           CONTENT_PRINTF("]");
           content_ptr = content;
         } else {
           /* We added only one link, don't send an array:
            * Start one byte farther and reduce content_len accordingly. */
           content_ptr = content + 1;
           content_len--;
         }

         coap_set_header_content_type(response, REST.type.APPLICATION_JSON);
         coap_set_payload(response, content_ptr, content_len);
         break;
       }
    }
    case COAP_DELETE: {
      /* A buffer where the subresource part of the URI will be stored */
      char uri_subresource[32];
      const char *uri_path = NULL;
      int uri_len = REST.get_url(request, &uri_path);
      int base_len = strlen(resource_celllist.url);
      /* Check that there is a subresource, and that we have enough space to store it */
      if(uri_len > base_len + 1 && uri_len - base_len - 1 < sizeof(uri_subresource)) {
        char *end;
        /* Extract the subresource path, have it null-terminated */
        strlcpy(uri_subresource, uri_path+base_len+1, uri_len-base_len);
        /* Convert subresource to unsigned */
        unsigned cd = (unsigned)strtoul(uri_subresource, &end, 10);
        /* Check convertion */
        if(end != uri_subresource && *end == '\0' && errno != ERANGE) {
          /* Actually remove the link */
          struct tsch_link *link = tsch_schedule_get_link_by_handle(cd);
          if(link) {
            struct tsch_slotframe *sf = tsch_schedule_get_slotframe_by_handle(link->slotframe_handle);
            if(sf && tsch_schedule_remove_link(sf, link)) {
              PRINTF("RICH: deleted link %u\n", cd);
              CONTENT_PRINTF("{\"cd\":%u}", cd);
              coap_set_header_content_type(response, REST.type.APPLICATION_JSON);
              coap_set_payload(response, content, content_len);
            }
          }
        }
      }
      break;
    default:
      break;
    }
  }
}

/*********** RICH 6t/6/sf resource *************************************************/

int
get_next_fd() {
  int i;
  for(i=0; i<65536; i++) {
    if(tsch_schedule_get_slotframe_by_handle(i) == NULL) {
      /* Return first unused slotframe handle */
      return i;
    }
  }
  return -1;
}

RESOURCE(slotframe, METHOD_POST | METHOD_DELETE | HAS_SUB_RESOURCES,
    "6t/6/sf", "title=\"Slotframe\";");

void
slotframe_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  int first_item = 1;
  const uint8_t *request_content;
  int request_content_len;

  if(IS_OPTION((coap_packet_t *)request, COAP_OPTION_BLOCK1)) {
    return;
  }

  content_len = 0;

  switch(coap_get_method(request)) {
    case COAP_POST: {
      int state;
      char *content_ptr;
      char field_buf[32] = "";
      int new_sf_count = 0; /* The number of newly added slotframes */
      int ns = 0; /* number of slots */
      /* Add new slotframe */
      request_content_len = coap_get_payload(request, &request_content);

      struct jsonparse_state js;
      jsonparse_setup(&js, (const char *)request_content, request_content_len);

      /* Start creating response */
      CONTENT_PRINTF("[");

      /* Parse json input */
      while((state=jsonparse_find_field(&js, field_buf, sizeof(field_buf)))) {

         switch(state) {
           case '{': /* New element */
             ns = 0;
             break;
           case '}': { /* End of current element */
             int fd; /* slotframeID (handle) */
             fd = get_next_fd();

             /* Add slotframe */
             if(fd != -1 && tsch_schedule_add_slotframe(fd, ns)) {
               new_sf_count++;
               PRINTF("RICH: added slotframe %u with length %u\n", fd, ns);

               /* Update response */
               if(!first_item) {
                 CONTENT_PRINTF(",");
               }
               first_item = 0;
               CONTENT_PRINTF("{\"fd\":%u}", fd);
             } else {
               PRINTF("RICH:! coult not add slotframe %u with length %u\n", fd, ns);
             }
             break;
           }
           case JSON_TYPE_NUMBER:
             if(!strncmp(field_buf, "ns", sizeof(field_buf))) {
               ns = jsonparse_get_value_as_int(&js);
             }
             break;
         }
       }

      /* Check if json parsing succeeded */
      if(js.error == JSON_ERROR_OK) {
        if(new_sf_count > 1) {
          /* We have several slotframes, we need to send an array */
          CONTENT_PRINTF("]");
          content_ptr = content;
        } else {
          /* We added only one slotframe, don't send an array:
           * Start one byte farther and reduce content_len accordingly. */
          content_ptr = content + 1;
          content_len--;
        }

        coap_set_header_content_type(response, REST.type.APPLICATION_JSON);
        coap_set_payload(response, content_ptr, content_len);
      }
      break;
    }
    case COAP_DELETE: {
      /* A buffer where the subresource part of the URI will be stored */
      char uri_subresource[32];
      const char *uri_path = NULL;
      int uri_len = REST.get_url(request, &uri_path);
      int base_len = strlen(resource_slotframe.url);
      /* Check that there is a subresource, and that we have enough space to store it */
      if(uri_len > base_len + 1 && uri_len - base_len - 1 < sizeof(uri_subresource)) {
        char *end;
        /* Extract the subresource path, have it null-terminated */
        strlcpy(uri_subresource, uri_path+base_len+1, uri_len-base_len);
        /* Convert subresource to unsigned */
        unsigned fd = (unsigned)strtoul(uri_subresource, &end, 10);
        /* Check convertion */
        if(end != uri_subresource && *end == '\0' && errno != ERANGE) {
          /* Actually remove the slotframe */
        }
        struct tsch_slotframe *sf = tsch_schedule_get_slotframe_by_handle(fd);
        if(sf && tsch_schedule_remove_slotframe(sf)) {
          PRINTF("RICH: deleted slotframe %u\n", fd);
          CONTENT_PRINTF("{\"fd\":%u}", fd);
          coap_set_header_content_type(response, REST.type.APPLICATION_JSON);
          coap_set_payload(response, content, content_len);
        }
      }
      break;
    default:
      break;
    }
  }
}

/*********** RICH rpl/c resource *************************************************/

EVENT_RESOURCE(childrenlist, METHOD_GET, "rpl/c", "title=\"RPL Chlidren List\";obs");

void
childrenlist_create_response(coap_packet_t *response)
{
  uip_ds6_route_t *r;
  int first_item = 1;
  uip_ipaddr_t *last_next_hop = NULL;
  uip_ipaddr_t *curr_next_hop = NULL;

  content_len = 0;

  CONTENT_PRINTF("[");
  for(r = uip_ds6_route_head();
      r != NULL;
      r = uip_ds6_route_next(r)) {

    /* We rely on the fact that routes are ordered by next hop.
     * Loop over all loops and print every next hop exactly once. */
    curr_next_hop = uip_ds6_route_nexthop(r);
    if(curr_next_hop != last_next_hop) {
      if(!first_item) {
        CONTENT_PRINTF(",");
      }
      first_item = 0;
      CONTENT_PRINTF("\"%x:%x:%x:%x\"",
          UIP_HTONS(curr_next_hop->u16[4]), UIP_HTONS(curr_next_hop->u16[5]),
          UIP_HTONS(curr_next_hop->u16[6]), UIP_HTONS(curr_next_hop->u16[7])
      );
      last_next_hop = curr_next_hop;
    }
  }
  CONTENT_PRINTF("]");

  coap_set_header_content_type(response, REST.type.APPLICATION_JSON);
  coap_set_payload(response, content, content_len);
}

void
rich_childrenlist_notify_subscribers()
{
  static uint16_t event_counter = 0;
  coap_packet_t notification;
  /* Build notification. */
  coap_init_message(&notification, COAP_TYPE_CON, REST.status.OK, 0 );
  childrenlist_create_response(&notification);
  /* Notify the registered observers with the given message type, observe option, and payload. */
  PRINTF("RICH: notify children list subscribers\n");
  coap_notify_observers(&resource_childrenlist, ++event_counter, &notification);
}

void
childrenlist_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  PRINTF("RICH: get children list\n");
  childrenlist_create_response(response);
}

/*********** RICH rpl/nd resource *************************************************/

EVENT_RESOURCE(nodelist, METHOD_GET, "rpl/nd", "title=\"RPL Node List\";obs");

void
nodelist_create_response(coap_packet_t *response)
{
  uip_ds6_route_t *r;
  int first_item = 1;

  content_len = 0;

  CONTENT_PRINTF("[");
  for(r = uip_ds6_route_head();
      r != NULL;
      r = uip_ds6_route_next(r)) {

    if(!first_item) {
      CONTENT_PRINTF(",");
    }
    first_item = 0;
    CONTENT_PRINTF("\"%x:%x:%x:%x\"",
        UIP_HTONS(r->ipaddr.u16[4]), UIP_HTONS(r->ipaddr.u16[5]),
        UIP_HTONS(r->ipaddr.u16[6]), UIP_HTONS(r->ipaddr.u16[7])
    );
  }
  CONTENT_PRINTF("]");

  coap_set_header_content_type(response, REST.type.APPLICATION_JSON);
  coap_set_payload(response, content, content_len);
}

void
rich_nodelist_notify_subscribers()
{
  static uint16_t event_counter = 0;
  coap_packet_t notification;
  /* Build notification. */
  coap_init_message(&notification, COAP_TYPE_CON, REST.status.OK, 0 );
  nodelist_create_response(&notification);
  /* Notify the registered observers with the given message type, observe option, and payload. */
  PRINTF("RICH: notify node list subscribers\n");
  coap_notify_observers(&resource_nodelist, ++event_counter, &notification);
}

void
nodelist_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  PRINTF("RICH: get node list\n");
  nodelist_create_response(response);
}

/* Wait for 30s without activity before notifying subscribers */
static struct ctimer route_changed_timer;

static void
route_changed_delayed_callback()
{
  rich_nodelist_notify_subscribers();
  rich_childrenlist_notify_subscribers();
}

static void
route_changed_callback(int event, uip_ipaddr_t *route, uip_ipaddr_t *ipaddr, int num_routes)
{
  /* We have added or removed a routing entry, notify subscribers */
  if(event == UIP_DS6_NOTIFICATION_ROUTE_ADD
      || event == UIP_DS6_NOTIFICATION_ROUTE_RM) {
    printf("RICH: setting route_changed callback with 30s delay\n");
    ctimer_set(&route_changed_timer, 30*CLOCK_SECOND,
        route_changed_delayed_callback, NULL);
  }
}

/*********** Init *************************************************/
PROCESS(rich_process, "RICH");

/* The logging process */
PROCESS_THREAD(rich_process, ev, data)
{
  static struct etimer periodic;
  PROCESS_BEGIN();
  etimer_set(&periodic, SM_UPDATE_INTERVAL);

  while(1) {
    PROCESS_WAIT_UNTIL(etimer_expired(&periodic));
    etimer_reset(&periodic);
    rich_sm0_notify_subscribers();
  }

  PROCESS_END();
}

/*********** Init *************************************************/

void
rich_scheduler_interface_init()
{
  static struct uip_ds6_notification n;
  rest_init_engine();
  rest_activate_resource(&resource_slotframe);
  rest_activate_resource(&resource_celllist);
  rest_activate_resource(&resource_statisticsmetrics);
  rest_activate_event_resource(&resource_nodelist);
  rest_activate_event_resource(&resource_childrenlist);
  rest_activate_event_resource(&resource_sm0);
  /* A callback for routing table changes */
  uip_ds6_notification_add(&n, route_changed_callback);
  process_start(&rich_process, NULL);
  PRINTF("RICH: initializing scheduler interface\n");
}

