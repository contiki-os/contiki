/*
 * Copyright (c) 2011, Institute for Pervasive Computing, ETH Zurich
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
 *      CoAP module for reliable transport
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include "contiki.h"
#include "contiki-net.h"

#include "er-coap-03-transactions.h"
#include "er-coap-03-observing.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif


MEMB(transactions_memb, coap_transaction_t, COAP_MAX_OPEN_TRANSACTIONS);
LIST(transactions_list);


static struct process *transaction_handler_process = NULL;

void
coap_register_as_transaction_handler()
{
  transaction_handler_process = PROCESS_CURRENT();
}

coap_transaction_t *
coap_new_transaction(uint16_t tid, uip_ipaddr_t *addr, uint16_t port)
{
  coap_transaction_t *t = memb_alloc(&transactions_memb);

  if (t)
  {
    t->tid = tid;
    t->retrans_counter = 0;

    /* save client address */
    uip_ipaddr_copy(&t->addr, addr);
    t->port = port;
  }

  return t;
}

void
coap_send_transaction(coap_transaction_t *t)
{
  PRINTF("Sending transaction %u\n", t->tid);

  coap_send_message(&t->addr, t->port, t->packet, t->packet_len);

  if (COAP_TYPE_CON==((COAP_HEADER_TYPE_MASK & t->packet[0])>>COAP_HEADER_TYPE_POSITION))
  {
    if (t->retrans_counter<COAP_MAX_RETRANSMIT)
    {
      PRINTF("Keeping transaction %u\n", t->tid);

      /*FIXME hack, maybe there is a better way, but avoid posting everything to the process */
      struct process *process_actual = PROCESS_CURRENT();
      process_current = transaction_handler_process;
      etimer_set(&t->retrans_timer, CLOCK_SECOND * COAP_RESPONSE_TIMEOUT * (1<<(t->retrans_counter)));
      process_current = process_actual;

      list_add(transactions_list, t); /* list itself makes sure same element is not added twice */

      t = NULL;
    }
    else
    {
      /* timeout */
      PRINTF("Timeout\n");
      restful_response_handler callback = t->callback;
      void *callback_data = t->callback_data;

      /* handle observers */
      coap_remove_observer_by_client(&t->addr, t->port);

      coap_clear_transaction(t);

      if (callback) {
        callback(callback_data, NULL);
      }
    }
  }
  else
  {
    coap_clear_transaction(t);
  }
}

void
coap_clear_transaction(coap_transaction_t *t)
{
  if (t)
  {
    PRINTF("Freeing transaction %u: %p\n", t->tid, t);

    etimer_stop(&t->retrans_timer);
    list_remove(transactions_list, t);
    memb_free(&transactions_memb, t);
  }
}

coap_transaction_t *
coap_get_transaction_by_tid(uint16_t tid)
{
  coap_transaction_t *t = NULL;

  for (t = (coap_transaction_t*)list_head(transactions_list); t; t = t->next)
  {
    if (t->tid==tid)
    {
      PRINTF("Found transaction for TID %u: %p\n", t->tid, t);
      return t;
    }
  }
  return NULL;
}

void
coap_check_transactions()
{
  coap_transaction_t *t = NULL;

  for (t = (coap_transaction_t*)list_head(transactions_list); t; t = t->next)
  {
    if (etimer_expired(&t->retrans_timer))
    {
      ++(t->retrans_counter);
      PRINTF("Retransmitting %u (%u)\n", t->tid, t->retrans_counter);
      coap_send_transaction(t);
    }
  }
}
