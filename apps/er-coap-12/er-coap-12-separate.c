/*
 * Copyright (c) 2012, Institute for Pervasive Computing, ETH Zurich
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
 *      CoAP module for separate responses
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <stdio.h>
#include <string.h>

#include "er-coap-12-separate.h"
#include "er-coap-12-transactions.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

/*----------------------------------------------------------------------------*/
void
coap_separate_reject()
{
  coap_error_code = SERVICE_UNAVAILABLE_5_03;
  coap_error_message = "AlreadyInUse";
}
/*----------------------------------------------------------------------------*/
int
coap_separate_accept(void *request, coap_separate_t *separate_store)
{
  coap_packet_t *const coap_req = (coap_packet_t *) request;
  coap_transaction_t *const t = coap_get_transaction_by_mid(coap_req->mid);

  PRINTF("Separate ACCEPT: /%.*s MID %u\n", coap_req->uri_path_len, coap_req->uri_path, coap_req->mid);
  if (t)
  {
    /* Send separate ACK for CON. */
    if (coap_req->type==COAP_TYPE_CON)
    {
      coap_packet_t ack[1];
      /* ACK with empty code (0) */
      coap_init_message(ack, COAP_TYPE_ACK, 0, coap_req->mid);
      /* Serializing into IPBUF: Only overwrites header parts that are already parsed into the request struct. */
      coap_send_message(&UIP_IP_BUF->srcipaddr, UIP_UDP_BUF->srcport, (uip_appdata), coap_serialize_message(ack, uip_appdata));
    }

    /* Store remote address. */
    uip_ipaddr_copy(&separate_store->addr, &t->addr);
    separate_store->port = t->port;

    /* Store correct response type. */
    separate_store->type = coap_req->type==COAP_TYPE_CON ? COAP_TYPE_CON : COAP_TYPE_NON;
    separate_store->mid = coap_get_mid(); /* if it was a NON, we burned one MID in the engine... */

    memcpy(separate_store->token, coap_req->token, coap_req->token_len);
    separate_store->token_len = coap_req->token_len;

    separate_store->block2_num = coap_req->block2_num;
    separate_store->block2_size = coap_req->block2_size;

    /* Signal the engine to skip automatic response and clear transaction by engine. */
    coap_error_code = MANUAL_RESPONSE;

    return 1;
  }
  else
  {
    PRINTF("ERROR: Response transaction for separate request not found!\n");
    return 0;
  }
}
/*----------------------------------------------------------------------------*/
void
coap_separate_resume(void *response, coap_separate_t *separate_store, uint8_t code)
{
  coap_init_message(response, separate_store->type, code, separate_store->mid);
  if (separate_store->token_len)
  {
    coap_set_header_token(response, separate_store->token, separate_store->token_len);
  }
}
