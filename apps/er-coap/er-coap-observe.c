/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      CoAP module for observing resources (draft-ietf-core-observe-11).
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <stdio.h>
#include <string.h>
#include "er-coap-observe.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

/*---------------------------------------------------------------------------*/
MEMB(observers_memb, coap_observer_t, COAP_MAX_OBSERVERS);
LIST(observers_list);
/*---------------------------------------------------------------------------*/
/*- Internal API ------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
coap_observer_t *
coap_add_observer(uip_ipaddr_t *addr, uint16_t port, const uint8_t *token,
                  size_t token_len, const char *uri)
{
  /* Remove existing observe relationship, if any. */
  coap_remove_observer_by_uri(addr, port, uri);

  coap_observer_t *o = memb_alloc(&observers_memb);

  if(o) {
    o->url = uri;
    uip_ipaddr_copy(&o->addr, addr);
    o->port = port;
    o->token_len = token_len;
    memcpy(o->token, token, token_len);
    o->last_mid = 0;

    PRINTF("Adding observer (%u/%u) for /%s [0x%02X%02X]\n",
           list_length(observers_list) + 1, COAP_MAX_OBSERVERS,
           o->url, o->token[0], o->token[1]);
    list_add(observers_list, o);
  }

  return o;
}
/*---------------------------------------------------------------------------*/
/*- Removal -----------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void
coap_remove_observer(coap_observer_t *o)
{
  PRINTF("Removing observer for /%s [0x%02X%02X]\n", o->url, o->token[0],
         o->token[1]);

  memb_free(&observers_memb, o);
  list_remove(observers_list, o);
}
/*---------------------------------------------------------------------------*/
int
coap_remove_observer_by_client(uip_ipaddr_t *addr, uint16_t port)
{
  int removed = 0;
  coap_observer_t *obs = NULL;

  for(obs = (coap_observer_t *)list_head(observers_list); obs;
      obs = obs->next) {
    PRINTF("Remove check client ");
    PRINT6ADDR(addr);
    PRINTF(":%u\n", port);
    if(uip_ipaddr_cmp(&obs->addr, addr) && obs->port == port) {
      coap_remove_observer(obs);
      removed++;
    }
  }
  return removed;
}
/*---------------------------------------------------------------------------*/
int
coap_remove_observer_by_token(uip_ipaddr_t *addr, uint16_t port,
                              uint8_t *token, size_t token_len)
{
  int removed = 0;
  coap_observer_t *obs = NULL;

  for(obs = (coap_observer_t *)list_head(observers_list); obs;
      obs = obs->next) {
    PRINTF("Remove check Token 0x%02X%02X\n", token[0], token[1]);
    if(uip_ipaddr_cmp(&obs->addr, addr) && obs->port == port
       && obs->token_len == token_len
       && memcmp(obs->token, token, token_len) == 0) {
      coap_remove_observer(obs);
      removed++;
    }
  }
  return removed;
}
/*---------------------------------------------------------------------------*/
int
coap_remove_observer_by_uri(uip_ipaddr_t *addr, uint16_t port,
                            const char *uri)
{
  int removed = 0;
  coap_observer_t *obs = NULL;

  for(obs = (coap_observer_t *)list_head(observers_list); obs;
      obs = obs->next) {
    PRINTF("Remove check URL %p\n", uri);
    if((addr == NULL
        || (uip_ipaddr_cmp(&obs->addr, addr) && obs->port == port))
       && (obs->url == uri || memcmp(obs->url, uri, strlen(obs->url)) == 0)) {
      coap_remove_observer(obs);
      removed++;
    }
  }
  return removed;
}
/*---------------------------------------------------------------------------*/
int
coap_remove_observer_by_mid(uip_ipaddr_t *addr, uint16_t port, uint16_t mid)
{
  int removed = 0;
  coap_observer_t *obs = NULL;

  for(obs = (coap_observer_t *)list_head(observers_list); obs;
      obs = obs->next) {
    PRINTF("Remove check MID %u\n", mid);
    if(uip_ipaddr_cmp(&obs->addr, addr) && obs->port == port
       && obs->last_mid == mid) {
      coap_remove_observer(obs);
      removed++;
    }
  }
  return removed;
}
/*---------------------------------------------------------------------------*/
/*- Notification ------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void
coap_notify_observers(resource_t *resource)
{
  /* build notification */
  coap_packet_t notification[1]; /* this way the packet can be treated as pointer as usual */
  coap_init_message(notification, COAP_TYPE_NON, CONTENT_2_05, 0);
  coap_observer_t *obs = NULL;

  PRINTF("Observe: Notification from %s\n", resource->url);

  /* iterate over observers */
  for(obs = (coap_observer_t *)list_head(observers_list); obs;
      obs = obs->next) {
    if(obs->url == resource->url) {     /* using RESOURCE url pointer as handle */
      coap_transaction_t *transaction = NULL;

      /*TODO implement special transaction for CON, sharing the same buffer to allow for more observers */

      if((transaction = coap_new_transaction(coap_get_mid(), &obs->addr, obs->port))) {
        if(obs->obs_counter % COAP_OBSERVE_REFRESH_INTERVAL == 0) {
          PRINTF("           Force Confirmable for\n");
          notification->type = COAP_TYPE_CON;
        }

        PRINTF("           Observer ");
        PRINT6ADDR(&obs->addr);
        PRINTF(":%u\n", obs->port);

        /* update last MID for RST matching */
        obs->last_mid = transaction->mid;

        /* prepare response */
        notification->mid = transaction->mid;

        resource->get_handler(NULL, notification,
                              transaction->packet + COAP_MAX_HEADER_SIZE,
                              REST_MAX_CHUNK_SIZE, NULL);

        if(notification->code < BAD_REQUEST_4_00) {
          coap_set_header_observe(notification, (obs->obs_counter)++);
        }
        coap_set_token(notification, obs->token, obs->token_len);

        transaction->packet_len =
          coap_serialize_message(notification, transaction->packet);

        coap_send_transaction(transaction);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void
coap_observe_handler(resource_t *resource, void *request, void *response)
{
  coap_packet_t *const coap_req = (coap_packet_t *)request;
  coap_packet_t *const coap_res = (coap_packet_t *)response;
  coap_observer_t * obs;

  if(coap_req->code == COAP_GET && coap_res->code < 128) { /* GET request and response without error code */
    if(IS_OPTION(coap_req, COAP_OPTION_OBSERVE)) {
      if(coap_req->observe == 0) {
        obs = coap_add_observer(&UIP_IP_BUF->srcipaddr, UIP_UDP_BUF->srcport,
                                coap_req->token, coap_req->token_len,
                                resource->url);
       if(obs) {
          coap_set_header_observe(coap_res, (obs->obs_counter)++);
          /*
           * Following payload is for demonstration purposes only.
           * A subscription should return the same representation as a normal GET.
           * Uncomment if you want an information about the avaiable observers.
           */
#if 0
          static char content[16];
          coap_set_payload(coap_res,
                           content,
                           snprintf(content, sizeof(content), "Added %u/%u",
                                    list_length(observers_list),
                                    COAP_MAX_OBSERVERS));
#endif
        } else {
          coap_res->code = SERVICE_UNAVAILABLE_5_03;
          coap_set_payload(coap_res, "TooManyObservers", 16);
        }
      } else if(coap_req->observe == 1) {

        /* remove client if it is currently observe */
        coap_remove_observer_by_token(&UIP_IP_BUF->srcipaddr,
                                      UIP_UDP_BUF->srcport, coap_req->token,
                                      coap_req->token_len);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
