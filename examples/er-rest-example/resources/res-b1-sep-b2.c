/*
 * Copyright (c) 2014, Lars Schmertmann <SmallLars@t-online.de>.
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
 *      Example resource
 * \author
 *      Lars Schmertmann <SmallLars@t-online.de>
 */

#include <string.h>
#include "rest-engine.h"
#include "er-coap-block1.h"
#include "er-coap-separate.h"
#include "er-coap-transactions.h"

static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
SEPARATE_RESOURCE(res_b1_sep_b2, "title=\"Block1 + Separate + Block2 demo\"", NULL, res_post_handler, NULL, NULL, NULL);

#define MAX_DATA_LEN 256

static uint8_t big_msg[MAX_DATA_LEN];
static size_t big_msg_len = 0;
static coap_separate_t request_metadata;

static void
res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /* Example allows only one request on time. There are no checks for multiply access !!! */
  if(*offset == 0) {
    /* Incoming Data */
    if(coap_block1_handler(request, response, big_msg, &big_msg_len, MAX_DATA_LEN)) {
      /* More Blocks will follow. Example waits for
       * the last block and stores data into big_msg.
       */
      return;
    }
    /* Last block was received. */
    coap_separate_accept(request, &request_metadata);

    /* Need Time for calculation now */
    unsigned i;
    for(i = 0; i <= 4096; i++) {
      printf("\r%4u\r", i);
    }
    printf("\n");

    /* Send first block */
    coap_transaction_t *transaction = NULL;
    if((transaction = coap_new_transaction(request_metadata.mid, &request_metadata.addr, request_metadata.port))) {
      coap_packet_t resp[1]; /* This way the packet can be treated as pointer as usual. */

      /* Restore the request information for the response. */
      coap_separate_resume(resp, &request_metadata, CONTENT_2_05);

      /* Set payload and block info */
      coap_set_payload(resp, big_msg, big_msg_len > request_metadata.block2_size ? request_metadata.block2_size : big_msg_len);
      if(big_msg_len > request_metadata.block2_size) {
        coap_set_header_block2(resp, 0, 1, request_metadata.block2_size);
      }

      /* Warning: No check for serialization error. */
      transaction->packet_len = coap_serialize_message(resp, transaction->packet);
      coap_send_transaction(transaction);
    }
  } else {
    /* request for more blocks */
    if(*offset >= big_msg_len) {
      coap_set_status_code(response, BAD_OPTION_4_02);
      coap_set_payload(response, "BlockOutOfScope", 15);
      return;
    }

    memcpy(buffer, big_msg + *offset, 32);
    if(big_msg_len - *offset < preferred_size) {
      preferred_size = big_msg_len - *offset;
      *offset = -1;
    } else {
      *offset += preferred_size;
    }
    coap_set_payload(response, buffer, preferred_size);
  }
}
