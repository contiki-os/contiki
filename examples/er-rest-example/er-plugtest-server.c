/*
 * Copyright (c) 2013, Matthias Kovatsch
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
 *      Server for the ETSI IoT CoAP Plugtests, Paris, France, 24 - 25 March 2012
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"

#define MAX_PLUGFEST_PAYLOAD 64+1 /* +1 for the terminating zero, which is not transmitted */
#define MAX_PLUGFEST_BODY    2048
#define CHUNKS_TOTAL         2012

/* Define which resources to include to meet memory constraints. */
#define REST_RES_TEST 1
#define REST_RES_LONG 1
#define REST_RES_QUERY 1
#define REST_RES_LOC_QUERY 1
#define REST_RES_MULTI 1
#define REST_RES_LINKS 1
#define REST_RES_PATH 1
#define REST_RES_SEPARATE 1
#define REST_RES_LARGE 1
#define REST_RES_LARGE_UPDATE 1
#define REST_RES_LARGE_CREATE 1
#define REST_RES_OBS 1

#define REST_RES_MIRROR 1



#if !defined (CONTIKI_TARGET_MINIMAL_NET)
#warning "Should only be compiled for minimal-net!"
#endif



#include "erbium.h"

/* For CoAP-specific example: not required for normal RESTful Web service. */
#if WITH_COAP==7
#include "er-coap-07.h"
#elif WITH_COAP == 12
#include "er-coap-12.h"
#elif WITH_COAP == 13
#include "er-coap-13.h"
#else
#error "Plugtests server without CoAP"
#endif /* CoAP-specific example */

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif


#if REST_RES_TEST
/*
 * Default test resource
 */
RESOURCE(test, METHOD_GET|METHOD_POST|METHOD_PUT|METHOD_DELETE, "test", "title=\"Default test resource\"");

static uint8_t test_etag[8] = {0};
static uint8_t test_etag_len = 1;
static uint8_t test_change = 1;
static uint8_t test_none_match_okay = 1;

static
void
test_update_etag()
{
    int i;
    test_etag_len = (random_rand() % 8) + 1;
    for (i=0; i<test_etag_len; ++i)
    {
      test_etag[i] = random_rand();
    }
    test_change = 0;
    
    PRINTF("### SERVER ACTION ### Changed ETag %u [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n", test_etag_len,
            test_etag[0],
            test_etag[1],
            test_etag[2],
            test_etag[3],
            test_etag[4],
            test_etag[5],
            test_etag[6],
            test_etag[7]
          );
}

void
test_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_packet_t *const coap_req = (coap_packet_t *) request;

  uint8_t method = REST.get_method_type(request);
  const uint8_t *bytes = NULL;
  size_t len = 0;
  
  if (test_change)
  {
    test_update_etag();
  }

  PRINTF("/test           ");
  
  if (method & METHOD_GET)
  {
    PRINTF("GET ");
    
    if ((len = coap_get_header_etag(request, &bytes))>0 && len==test_etag_len && memcmp(test_etag, bytes, len)==0)
    {
      PRINTF("validate ");
      REST.set_response_status(response, REST.status.NOT_MODIFIED);
      REST.set_header_etag(response, test_etag, test_etag_len);
      
      test_change = 1;
      PRINTF("### SERVER ACTION ### Resouce will change\n");
	}
    else
    {
      /* Code 2.05 CONTENT is default. */
      REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
      REST.set_header_etag(response, test_etag, test_etag_len);
      REST.set_header_max_age(response, 30);
      REST.set_response_payload(response, buffer, snprintf((char *)buffer, MAX_PLUGFEST_PAYLOAD, "Type: %u\nCode: %u\nMID: %u", coap_req->type, coap_req->code, coap_req->mid));
    }
  }
  else if (method & METHOD_POST)
  {
    PRINTF("POST ");
    REST.set_response_status(response, REST.status.CREATED);
    REST.set_header_location(response, "/location1/location2/location3");
  }
  else if (method & METHOD_PUT)
  {
    PRINTF("PUT ");
    
    if (coap_get_header_if_none_match(request))
    {
      if (test_none_match_okay)
      {
        REST.set_response_status(response, REST.status.CREATED);
        
        test_none_match_okay = 0;
        PRINTF("### SERVER ACTION ### If-None-Match will FAIL\n");
      }
      else
      {
        REST.set_response_status(response, PRECONDITION_FAILED_4_12);
        
	    test_none_match_okay = 1;
        PRINTF("### SERVER ACTION ### If-None-Match will SUCCEED\n");
	  }
	}
    else if (((len = coap_get_header_if_match(request, &bytes))>0 && (len==test_etag_len && memcmp(test_etag, bytes, len)==0)) || len==0)
    {
      test_update_etag();
      REST.set_header_etag(response, test_etag, test_etag_len);
	  
      REST.set_response_status(response, REST.status.CHANGED);
      
      if (len>0)
      {
        test_change = 1;
        PRINTF("### SERVER ACTION ### Resouce will change\n");
	  }
    }
    else
    {
    
      PRINTF("Check %u/%u\n  [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n  [0x%02X%02X%02X%02X%02X%02X%02X%02X] ", len, test_etag_len,
            bytes[0],
            bytes[1],
            bytes[2],
            bytes[3],
            bytes[4],
            bytes[5],
            bytes[6],
            bytes[7],
            test_etag[0],
            test_etag[1],
            test_etag[2],
            test_etag[3],
            test_etag[4],
            test_etag[5],
            test_etag[6],
            test_etag[7] );
    
	  REST.set_response_status(response, PRECONDITION_FAILED_4_12);
	}
  }
  else if (method & METHOD_DELETE)
  {
    PRINTF("DELETE ");
    REST.set_response_status(response, REST.status.DELETED);
  }

  PRINTF("(%s %u)\n", coap_req->type==COAP_TYPE_CON?"CON":"NON", coap_req->mid);
}


RESOURCE(create1, METHOD_PUT|METHOD_DELETE, "create1", "title=\"Default test resource\"");

static uint8_t create1_exists = 0;

void
create1_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  uint8_t method = REST.get_method_type(request);

  if (test_change)
  {
    test_update_etag();
  }

  PRINTF("/create1       ");

  if (method & METHOD_PUT)
  {
    PRINTF("PUT ");

    if (coap_get_header_if_none_match(request))
    {
      if (!create1_exists)
      {
        REST.set_response_status(response, REST.status.CREATED);

        create1_exists = 1;
      }
      else
      {
        REST.set_response_status(response, PRECONDITION_FAILED_4_12);
      }
    }
    else
    {
      REST.set_response_status(response, REST.status.CHANGED);
    }
  }
  else if (method & METHOD_DELETE)
  {
    PRINTF("DELETE ");
    REST.set_response_status(response, REST.status.DELETED);

    create1_exists = 0;
  }
}

RESOURCE(create2, METHOD_POST, "create2", "title=\"Creates on POST\"");

void
create2_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  if (test_change)
  {
    test_update_etag();
  }

  PRINTF("/create2       ");

  REST.set_response_status(response, REST.status.CREATED);
  REST.set_header_location(response, "/location1/location2/location3");
}

RESOURCE(create3, METHOD_PUT|METHOD_DELETE, "create3", "title=\"Default test resource\"");

static uint8_t create3_exists = 0;

void
create3_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  uint8_t method = REST.get_method_type(request);

  if (test_change)
  {
    test_update_etag();
  }

  PRINTF("/create3       ");

  if (method & METHOD_PUT)
  {
    PRINTF("PUT ");

    if (coap_get_header_if_none_match(request))
    {
      if (!create3_exists)
      {
        REST.set_response_status(response, REST.status.CREATED);

        create3_exists = 1;
      }
      else
      {
        REST.set_response_status(response, PRECONDITION_FAILED_4_12);
      }
    }
    else
    {
      REST.set_response_status(response, REST.status.CHANGED);
    }
  }
  else if (method & METHOD_DELETE)
  {
    PRINTF("DELETE ");
    REST.set_response_status(response, REST.status.DELETED);

    create3_exists = 0;
  }
}





RESOURCE(validate, METHOD_GET|METHOD_PUT, "validate", "title=\"Default test resource\"");

static uint8_t validate_etag[8] = {0};
static uint8_t validate_etag_len = 1;
static uint8_t validate_change = 1;

static
void
validate_update_etag()
{
    int i;
    validate_etag_len = (random_rand() % 8) + 1;
    for (i=0; i<validate_etag_len; ++i)
    {
      validate_etag[i] = random_rand();
    }
    validate_change = 0;

    PRINTF("### SERVER ACTION ### Changed ETag %u [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n", validate_etag_len,
            validate_etag[0],
            validate_etag[1],
            validate_etag[2],
            validate_etag[3],
            validate_etag[4],
            validate_etag[5],
            validate_etag[6],
            validate_etag[7]
          );
}

void
validate_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_packet_t *const coap_req = (coap_packet_t *) request;

  uint8_t method = REST.get_method_type(request);
  const uint8_t *bytes = NULL;
  size_t len = 0;

  if (validate_change)
  {
    validate_update_etag();
  }

  PRINTF("/validate       ");

  if (method & METHOD_GET)
  {
    PRINTF("GET ");

    if ((len = coap_get_header_etag(request, &bytes))>0 && len==validate_etag_len && memcmp(validate_etag, bytes, len)==0)
    {
      PRINTF("validate ");
      REST.set_response_status(response, REST.status.NOT_MODIFIED);
      REST.set_header_etag(response, validate_etag, validate_etag_len);

      validate_change = 1;
      PRINTF("### SERVER ACTION ### Resouce will change\n");
        }
    else
    {
      /* Code 2.05 CONTENT is default. */
      REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
      REST.set_header_etag(response, validate_etag, validate_etag_len);
      REST.set_header_max_age(response, 30);
      REST.set_response_payload(response, buffer, snprintf((char *)buffer, MAX_PLUGFEST_PAYLOAD, "Type: %u\nCode: %u\nMID: %u", coap_req->type, coap_req->code, coap_req->mid));
    }
  }
  else if (method & METHOD_PUT)
  {
    PRINTF("PUT ");

    if (((len = coap_get_header_if_match(request, &bytes))>0 && (len==validate_etag_len && memcmp(validate_etag, bytes, len)==0)) || len==0)
    {
      validate_update_etag();
      REST.set_header_etag(response, validate_etag, validate_etag_len);

      REST.set_response_status(response, REST.status.CHANGED);

      if (len>0)
      {
        validate_change = 1;
        PRINTF("### SERVER ACTION ### Resouce will change\n");
          }
    }
    else
    {
      PRINTF("Check %u/%u\n  [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n  [0x%02X%02X%02X%02X%02X%02X%02X%02X] ", len, validate_etag_len,
          bytes[0],
          bytes[1],
          bytes[2],
          bytes[3],
          bytes[4],
          bytes[5],
          bytes[6],
          bytes[7],
          validate_etag[0],
          validate_etag[1],
          validate_etag[2],
          validate_etag[3],
          validate_etag[4],
          validate_etag[5],
          validate_etag[6],
          validate_etag[7] );

        REST.set_response_status(response, PRECONDITION_FAILED_4_12);
      }
  }

  PRINTF("(%s %u)\n", coap_req->type==COAP_TYPE_CON?"CON":"NON", coap_req->mid);
}
#endif

#if REST_RES_LONG
/*
 * Long path resource
 */
RESOURCE(longpath, METHOD_GET, "seg1/seg2/seg3", "title=\"Long path resource\"");

void
longpath_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_packet_t *const coap_req = (coap_packet_t *) request;

  uint8_t method = REST.get_method_type(request);

  PRINTF("/seg1/seg2/seg3 ");
  if (method & METHOD_GET)
  {
    PRINTF("GET ");
    /* Code 2.05 CONTENT is default. */
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, buffer, snprintf((char *)buffer, MAX_PLUGFEST_PAYLOAD, "Type: %u\nCode: %u\nMID: %u", coap_req->type, coap_req->code, coap_req->mid));
  }
  PRINTF("(%s %u)\n", coap_req->type==COAP_TYPE_CON?"CON":"NON", coap_req->mid);
}
#endif

#if REST_RES_QUERY
/*
 * Resource accepting query parameters
 */
RESOURCE(query, METHOD_GET, "query", "title=\"Resource accepting query parameters\"");

void
query_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_packet_t *const coap_req = (coap_packet_t *) request;
  int len = 0;
  const char *query = NULL;

  PRINTF("/query          GET (%s %u)\n", coap_req->type==COAP_TYPE_CON?"CON":"NON", coap_req->mid);

  if ((len = REST.get_query(request, &query)))
  {
    PRINTF("Query: %.*s\n", len, query);
  }

  /* Code 2.05 CONTENT is default. */
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, snprintf((char *)buffer, MAX_PLUGFEST_PAYLOAD, "Type: %u\nCode: %u\nMID: %u\nQuery: %.*s", coap_req->type, coap_req->code, coap_req->mid, len, query));
}
#endif

#if REST_RES_LOC_QUERY
/*
 * Resource accepting query parameters
 */
RESOURCE(locquery, METHOD_POST, "location-query", "title=\"Resource accepting query parameters\"");

void
locquery_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_packet_t *const coap_req = (coap_packet_t *) request;

  PRINTF("/location-query POST (%s %u)\n", coap_req->type==COAP_TYPE_CON?"CON":"NON", coap_req->mid);
  
  REST.set_response_status(response, REST.status.CREATED);
  REST.set_header_location(response, "?first=1&second=2");
}
#endif

#if REST_RES_MULTI
/*
 * Resource providing text/plain and application/xml
 */
RESOURCE(multi, METHOD_GET, "multi-format", "title=\"Resource providing text/plain and application/xml\";ct=\"0 41\"");
void
multi_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_packet_t *const coap_req = (coap_packet_t *) request;

  const uint16_t *accept = NULL;
  int num = REST.get_header_accept(request, &accept);

  PRINTF("/multi-format   GET (%s %u) %d\n", coap_req->type==COAP_TYPE_CON?"CON":"NON", coap_req->mid, num);

  if (num==0 || (num && accept[0]==REST.type.TEXT_PLAIN))
  {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, buffer, snprintf((char *)buffer, MAX_PLUGFEST_PAYLOAD, "Type: %u\nCode: %u\nMID: %u%s", coap_req->type, coap_req->code, coap_req->mid, num ? "\nAccept: 0" : ""));
PRINTF("PLAIN\n");
  }
  else if (num && (accept[0]==REST.type.APPLICATION_XML))
  {
    REST.set_header_content_type(response, REST.type.APPLICATION_XML);
    REST.set_response_payload(response, buffer, snprintf((char *)buffer, MAX_PLUGFEST_PAYLOAD, "<status type=\"%u\" code=\"%u\" mid=\"%u\" accept=\"%u\"/>", coap_req->type, coap_req->code, coap_req->mid, accept[0]));
PRINTF("XML\n");
  }
  else
  {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    const char *msg = "Supporting content-types text/plain and application/xml";
    REST.set_response_payload(response, msg, strlen(msg));
    PRINTF("ERROR\n");
  }
}
#endif

#if REST_RES_LINKS
/*
 * Resources providing text/plain and application/xml
 */
RESOURCE(link1, METHOD_GET, "link1", "rt=\"Type1 Type2\";if=\"If1\"");
SUB_RESOURCE(link2, METHOD_GET, "link2", "rt=\"Type2 Type3\";if=\"If2\"", link1);
SUB_RESOURCE(link3, METHOD_GET, "link3", "rt=\"Type1 Type3\";if=\"foo\"", link1);

void
link1_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const char *msg = "Dummy link";
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, msg, strlen(msg));
}
#endif

#if REST_RES_PATH
/*
 * Resources providing text/plain and application/xml
 */
RESOURCE(path, METHOD_GET | HAS_SUB_RESOURCES, "path", "ct=\"40\"");

void
path_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

  const char *uri_path = NULL;
  int len = REST.get_url(request, &uri_path);
  int base_len = strlen(resource_path.url);

  if (len==base_len)
  {
    REST.set_header_content_type(response, REST.type.APPLICATION_LINK_FORMAT);
	snprintf((char *)buffer, MAX_PLUGFEST_PAYLOAD, "</path/sub1>,</path/sub2>,</path/sub3>");
  }
  else
  {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, MAX_PLUGFEST_PAYLOAD, "/%.*s", len, uri_path);
  }

  REST.set_response_payload(response, buffer, strlen((char *)buffer));
}
#endif

#if REST_RES_SEPARATE
/* Required to manually (=not by the engine) handle the response transaction. */
#if WITH_COAP == 7
#include "er-coap-07-separate.h"
#include "er-coap-07-transactions.h"
#elif WITH_COAP == 12
#include "er-coap-12-separate.h"
#include "er-coap-12-transactions.h"
#elif WITH_COAP == 13
#include "er-coap-13-separate.h"
#include "er-coap-13-transactions.h"
#endif
/*
 * Resource which cannot be served immediately and which cannot be acknowledged in a piggy-backed way
 */
PERIODIC_RESOURCE(separate, METHOD_GET, "separate", "title=\"Resource which cannot be served immediately and which cannot be acknowledged in a piggy-backed way\"", 3*CLOCK_SECOND);

/* A structure to store the required information */
typedef struct application_separate_store {
  /* Provided by Erbium to store generic request information such as remote address and token. */
  coap_separate_t request_metadata;
  /* Add fields for addition information to be stored for finalizing, e.g.: */
  char buffer[MAX_PLUGFEST_PAYLOAD];
} application_separate_store_t;

static uint8_t separate_active = 0;
static application_separate_store_t separate_store[1];

void
separate_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_packet_t *const coap_req = (coap_packet_t *) request;

  PRINTF("/separate       ");
  if (separate_active)
  {
    PRINTF("REJECTED ");
    coap_separate_reject();
  }
  else
  {
    PRINTF("STORED ");
    separate_active = 1;

    /* Take over and skip response by engine. */
    coap_separate_accept(request, &separate_store->request_metadata);
    /* Be aware to respect the Block2 option, which is also stored in the coap_separate_t. */

    snprintf(separate_store->buffer, MAX_PLUGFEST_PAYLOAD, "Type: %u\nCode: %u\nMID: %u", coap_req->type, coap_req->code, coap_req->mid);
  }

  PRINTF("(%s %u)\n", coap_req->type==COAP_TYPE_CON?"CON":"NON", coap_req->mid);
}

void
separate_periodic_handler(resource_t *resource)
{
  if (separate_active)
  {
    PRINTF("/separate       ");
    coap_transaction_t *transaction = NULL;
    if ( (transaction = coap_new_transaction(separate_store->request_metadata.mid, &separate_store->request_metadata.addr, separate_store->request_metadata.port)) )
    {
      PRINTF("RESPONSE (%s %u)\n", separate_store->request_metadata.type==COAP_TYPE_CON?"CON":"NON", separate_store->request_metadata.mid);

      coap_packet_t response[1]; /* This way the packet can be treated as pointer as usual. */

      /* Restore the request information for the response. */
      coap_separate_resume(response, &separate_store->request_metadata, CONTENT_2_05);

      REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
      coap_set_payload(response, separate_store->buffer, strlen(separate_store->buffer));

      /*
       * Be aware to respect the Block2 option, which is also stored in the coap_separate_t.
       * As it is a critical option, this example resource pretends to handle it for compliance.
       */
      coap_set_header_block2(response, separate_store->request_metadata.block2_num, 0, separate_store->request_metadata.block2_size);

      /* Warning: No check for serialization error. */
      transaction->packet_len = coap_serialize_message(response, transaction->packet);
      coap_send_transaction(transaction);
      /* The engine will clear the transaction (right after send for NON, after acked for CON). */

      separate_active = 0;
    } else {
      PRINTF("ERROR (transaction)\n");
    }
  } /* if (separate_active) */
}
#endif

#if REST_RES_LARGE

/* double expansion */
#define TO_STRING2(x)  #x
#define TO_STRING(x)  TO_STRING2(x)

/*
 * Large resource
 */
RESOURCE(large, METHOD_GET, "large", "title=\"Large resource\";rt=\"block\";sz=\"" TO_STRING(CHUNKS_TOTAL) "\"");

void
large_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  int32_t strpos = 0;

  /* Check the offset for boundaries of the resource data. */
  if (*offset>=CHUNKS_TOTAL)
  {
    REST.set_response_status(response, REST.status.BAD_OPTION);
    /* A block error message should not exceed the minimum block size (16). */

    const char *error_msg = "BlockOutOfScope";
    REST.set_response_payload(response, error_msg, strlen(error_msg));
    return;
  }

  /* Generate data until reaching CHUNKS_TOTAL. */
  while (strpos<preferred_size)
  {
    strpos += snprintf((char *)buffer+strpos, preferred_size-strpos+1, "|%ld|", *offset);
  }

  /* snprintf() does not adjust return value if truncated by size. */
  if (strpos > preferred_size)
  {
    strpos = preferred_size;
  }

  /* Truncate if above CHUNKS_TOTAL bytes. */
  if (*offset+(int32_t)strpos > CHUNKS_TOTAL)
  {
    strpos = CHUNKS_TOTAL - *offset;
  }

  REST.set_response_payload(response, buffer, strpos);
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);

  /* IMPORTANT for chunk-wise resources: Signal chunk awareness to REST engine. */
  *offset += strpos;

  /* Signal end of resource representation. */
  if (*offset>=CHUNKS_TOTAL)
  {
    *offset = -1;
  }
}
#endif

#if REST_RES_LARGE_UPDATE
/*
 * Large resource that can be updated using PUT method
 */
RESOURCE(large_update, METHOD_GET|METHOD_PUT, "large-update", "title=\"Large resource that can be updated using PUT method\";rt=\"block\";sz=\"" TO_STRING(MAX_PLUGFEST_BODY) "\"");

static int32_t large_update_size = 0;
static uint8_t large_update_store[MAX_PLUGFEST_BODY] = {0};
static unsigned int large_update_ct = -1;

void
large_update_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_packet_t *const coap_req = (coap_packet_t *) request;
  uint8_t method = REST.get_method_type(request);

  if (method & METHOD_GET)
  {
    /* Check the offset for boundaries of the resource data. */
    if (*offset>=large_update_size)
    {
      REST.set_response_status(response, REST.status.BAD_OPTION);
      /* A block error message should not exceed the minimum block size (16). */

      const char *error_msg = "BlockOutOfScope";
      REST.set_response_payload(response, error_msg, strlen(error_msg));
      return;
    }

    REST.set_response_payload(response, large_update_store+*offset, MIN(large_update_size - *offset, preferred_size));
    REST.set_header_content_type(response, large_update_ct);

    /* IMPORTANT for chunk-wise resources: Signal chunk awareness to REST engine. */
    *offset += preferred_size;

    /* Signal end of resource representation. */
    if (*offset>=large_update_size)
    {
      *offset = -1;
    }
  } else {
    uint8_t *incoming = NULL;
    size_t len = 0;

    unsigned int ct = REST.get_header_content_type(request);
    if (ct==-1)
    {
      REST.set_response_status(response, REST.status.BAD_REQUEST);
      const char *error_msg = "NoContentType";
      REST.set_response_payload(response, error_msg, strlen(error_msg));
      return;
    }

    if ((len = REST.get_request_payload(request, (const uint8_t **) &incoming)))
    {
      if (coap_req->block1_num*coap_req->block1_size+len <= sizeof(large_update_store))
      {
        memcpy(large_update_store+coap_req->block1_num*coap_req->block1_size, incoming, len);
        large_update_size = coap_req->block1_num*coap_req->block1_size+len;
        large_update_ct = REST.get_header_content_type(request);

        REST.set_response_status(response, REST.status.CHANGED);
        coap_set_header_block1(response, coap_req->block1_num, 0, coap_req->block1_size);
      }
      else
      {
        REST.set_response_status(response, REST.status.REQUEST_ENTITY_TOO_LARGE);
        REST.set_response_payload(response, buffer, snprintf((char *)buffer, MAX_PLUGFEST_PAYLOAD, "%uB max.", sizeof(large_update_store)));
        return;
      }
    }
    else
    {
      REST.set_response_status(response, REST.status.BAD_REQUEST);
      const char *error_msg = "NoPayload";
      REST.set_response_payload(response, error_msg, strlen(error_msg));
      return;
    }
  }
}
#endif

#if REST_RES_LARGE_CREATE
/*
 * Large resource that can be created using POST method
 */
RESOURCE(large_create, METHOD_POST, "large-create", "title=\"Large resource that can be created using POST method\";rt=\"block\"");

void
large_create_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_packet_t *const coap_req = (coap_packet_t *) request;

  uint8_t *incoming = NULL;
  size_t len = 0;

  unsigned int ct = REST.get_header_content_type(request);
  if (ct==-1)
  {
    REST.set_response_status(response, REST.status.BAD_REQUEST);
    const char *error_msg = "NoContentType";
    REST.set_response_payload(response, error_msg, strlen(error_msg));
    return;
  }

  if ((len = REST.get_request_payload(request, (const uint8_t **) &incoming)))
  {
    if (coap_req->block1_num*coap_req->block1_size+len <= 2048)
    {
      REST.set_response_status(response, REST.status.CREATED);
      REST.set_header_location(response, "/nirvana");
      coap_set_header_block1(response, coap_req->block1_num, 0, coap_req->block1_size);
    }
    else
    {
      REST.set_response_status(response, REST.status.REQUEST_ENTITY_TOO_LARGE);
      const char *error_msg = "2048B max.";
      REST.set_response_payload(response, error_msg, strlen(error_msg));
      return;
    }
  }
  else
  {
    REST.set_response_status(response, REST.status.BAD_REQUEST);
    const char *error_msg = "NoPayload";
    REST.set_response_payload(response, error_msg, strlen(error_msg));
    return;
  }
}
#endif

#if REST_RES_OBS

#if WITH_COAP == 12
#include "er-coap-12-observing.h"
#elif WITH_COAP == 13
#include "er-coap-13-observing.h"
#endif
/*
 * Observable resource which changes every 5 seconds
 */
PERIODIC_RESOURCE(obs, METHOD_GET|METHOD_PUT|METHOD_DELETE, "obs", "title=\"Observable resource which changes every 5 seconds\";obs", 5*CLOCK_SECOND);

static uint16_t obs_counter = 0;
static char obs_content[MAX_PLUGFEST_BODY];
static size_t obs_content_len = 0;
static unsigned int obs_format = 0;

static char obs_status = 0;

static
void
obs_purge_list()
{
  PRINTF("### SERVER ACTION ### Purging obs list");
  coap_remove_observer_by_url(NULL, 0, resource_obs.url);
}

void
obs_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  uint8_t method = request==NULL ? METHOD_GET : REST.get_method_type(request);

  /* Keep server log clean from ticking events */
  if (request!=NULL)
  {
    PRINTF("/obs            ");
  }
  
  if (method & METHOD_GET)
  {
    /* Keep server log clean from ticking events */
    if (request!=NULL)
    {
      PRINTF("GET ");
    }
    
    REST.set_header_content_type(response, obs_format);
    REST.set_header_max_age(response, 5);
    
    if (obs_content_len)
    {
      REST.set_header_content_type(response, obs_format);
	  REST.set_response_payload(response, obs_content, obs_content_len);
	}
	else
	{
      REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
	  REST.set_response_payload(response, obs_content, snprintf(obs_content, MAX_PLUGFEST_PAYLOAD, "TICK %lu", obs_counter));
	}
    /* A post_handler that handles subscriptions will be called for periodic resources by the REST framework. */
  }
  else if (method & METHOD_PUT)
  {
    uint8_t *incoming = NULL;
    unsigned int ct = REST.get_header_content_type(request);
    
    PRINTF("PUT ");
    
    if (ct!=obs_format)
    {
        obs_status = 1;

      obs_format = ct;
    } else {
    
      obs_format = ct;
      obs_content_len = REST.get_request_payload(request, (const uint8_t **) &incoming);
      memcpy(obs_content, incoming, obs_content_len);
      obs_periodic_handler(&resource_obs);
    }
    
    REST.set_response_status(response, REST.status.CHANGED);
  }
  else if (method & METHOD_DELETE)
  {
    PRINTF("DELETE ");

    obs_status = 2;
    
    REST.set_response_status(response, REST.status.DELETED);
  }
  
  /* Keep server log clean from ticking events */
  if (request!=NULL)
  {
    PRINTF("\n");
  }
}

/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the REST manager process with the defined period.
 */
void
obs_periodic_handler(resource_t *r)
{
  ++obs_counter;

  //PRINTF("TICK %u for /%s\n", obs_counter, r->url);

  if (obs_status==1)
    {
      coap_packet_t notification[1]; /* This way the packet can be treated as pointer as usual. */
            coap_init_message(notification, COAP_TYPE_NON, INTERNAL_SERVER_ERROR_5_00, 0 );

            /* Notify the registered observers with the given message type, observe option, and payload. */
            REST.notify_subscribers(&resource_obs, -1, notification);

            PRINTF("######### sending 5.00\n");

            obs_purge_list();
    }
  else if (obs_status==2)
    {

      coap_packet_t notification[1]; /* This way the packet can be treated as pointer as usual. */
      coap_init_message(notification, COAP_TYPE_NON, NOT_FOUND_4_04, 0 );


      /* Notify the registered observers with the given message type, observe option, and payload. */
      REST.notify_subscribers(&resource_obs, -1, notification);

      obs_purge_list();

      obs_counter = 0;
      obs_content_len = 0;
    }
  else
    {
  /* Build notification. */
  /*TODO: REST.new_response() */
  coap_packet_t notification[1]; /* This way the packet can be treated as pointer as usual. */
  coap_init_message(notification, COAP_TYPE_NON, CONTENT_2_05, 0 );

  /* Better use a generator function for both handlers that only takes *resonse. */
  obs_handler(NULL, notification, NULL, 0, NULL);

  /* Notify the registered observers with the given message type, observe option, and payload. */
  REST.notify_subscribers(r, obs_counter, notification);
    }
  obs_status = 0;
}
#endif

#if REST_RES_MIRROR
/* This resource mirrors the incoming request. It shows how to access the options and how to set them for the response. */
RESOURCE(mirror, METHOD_GET | METHOD_POST | METHOD_PUT | METHOD_DELETE, "debug/mirror", "title=\"Returns your decoded message\";rt=\"Debug\"");

void
mirror_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /* The ETag and Token is copied to the header. */
  uint8_t opaque[] = {0x0A, 0xBC, 0xDE};

  /* Strings are not copied, so use static string buffers or strings in .text memory (char *str = "string in .text";). */
  static char location[] = {'/','f','/','a','?','k','&','e', 0};

  /* Getter for the header option Content-Type. If the option is not set, text/plain is returned by default. */
  unsigned int content_type = REST.get_header_content_type(request);

  /* The other getters copy the value (or string/array pointer) to the given pointers and return 1 for success or the length of strings/arrays. */
  uint32_t max_age_and_size = 0;
  const char *str = NULL;
  uint32_t observe = 0;
  const uint8_t *bytes = NULL;
  const uint16_t *words = NULL;
  uint32_t block_num = 0;
  uint8_t block_more = 0;
  uint16_t block_size = 0;
  const char *query = "";
  int len = 0;

  /* Mirror the received header options in the response payload. Unsupported getters (e.g., rest_get_header_observe() with HTTP) will return 0. */

  int strpos = 0;
  /* snprintf() counts the terminating '\0' to the size parameter.
   * The additional byte is taken care of by allocating REST_MAX_CHUNK_SIZE+1 bytes in the REST framework.
   * Add +1 to fill the complete buffer, as the payload does not need a terminating '\0'. */
  
  
  if (strpos<=REST_MAX_CHUNK_SIZE && (len = REST.get_header_if_match(request, &bytes)))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "If-Match 0x");
    int index = 0;
    for (index = 0; index<len; ++index) {
        strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "%02X", bytes[index]);
    }
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "\n");
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && (len = REST.get_header_host(request, &str)))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Uri-Host %.*s\n", len, str);
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && (len = coap_get_header_etag(request, &bytes)))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "ETag 0x");
    int index = 0;
    for (index = 0; index<len; ++index) {
        strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "%02X", bytes[index]);
    }
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "\n");
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && REST.get_header_if_none_match(request))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "If-None-Match\n");
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && coap_get_header_observe(request, &observe))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Observe %lu\n", observe);
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && (len = coap_get_header_location_path(request, &str)))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Location-Path %.*s\n", len, str);
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && (len = coap_get_header_uri_path(request, &str)))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Uri-Path %.*s\n", len, str);
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && content_type!=-1)
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Content-Format %u\n", content_type);
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && REST.get_header_max_age(request, &max_age_and_size))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Max-Age %lu\n", max_age_and_size);
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && (len = REST.get_query(request, &query)))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Uri-Query %.*s\n", len, query);
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && (len = REST.get_header_accept(request, &words)))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Accept ");
    int index = 0;
    for (index = 0; index<len; ++index) {
        strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "%u", words[index]);
    }
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "\n");
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && (len = coap_get_header_token(request, &bytes)))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Token 0x");
    int index = 0;
    for (index = 0; index<len; ++index) {
        strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "%02X", bytes[index]);
    }
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "\n");
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && (len = coap_get_header_location_query(request, &str)))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Location-Query %.*s\n", len, str);
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && coap_get_header_block2(request, &block_num, &block_more, &block_size, NULL)) /* This getter allows NULL pointers to get only a subset of the block parameters. */
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Block2 %lu%s (%u)\n", block_num, block_more ? "+" : "", block_size);
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && coap_get_header_block1(request, &block_num, &block_more, &block_size, NULL)) /* This getter allows NULL pointers to get only a subset of the block parameters. */
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Block1 %lu%s (%u)\n", block_num, block_more ? "+" : "", block_size);
  }
  if (strpos<=REST_MAX_CHUNK_SIZE && REST.get_header_length(request, &max_age_and_size))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Size %lu\n", max_age_and_size);
  }

  if (strpos<=REST_MAX_CHUNK_SIZE && (len = REST.get_request_payload(request, &bytes)))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "\n%.*s", len, bytes);
  }

  if (strpos >= REST_MAX_CHUNK_SIZE)
  {
      buffer[REST_MAX_CHUNK_SIZE-1] = 0xBB; /* '»' to indicate truncation */
  }

  REST.set_response_payload(response, buffer, strpos);

  PRINTF("/mirror options received: %s\n", buffer);

  /* Set dummy header options for response. Like getters, some setters are not implemented for HTTP and have no effect. */
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_header_max_age(response, 17); /* For HTTP, browsers will not re-request the page for 17 seconds. */
  REST.set_header_etag(response, opaque, 2);
  REST.set_header_location(response, location); /* Initial slash is omitted by framework */
  REST.set_header_length(response, strpos); /* For HTTP, browsers will not re-request the page for 10 seconds. CoAP action depends on the client. */

/* CoAP-specific example: actions not required for normal RESTful Web service. */
  coap_set_header_uri_host(response, "Contiki");
  coap_set_header_observe(response, 10);
  coap_set_header_proxy_uri(response, "ftp://x");
  //coap_set_header_block2(response, 42, 0, 64);
  //coap_set_header_block1(response, 23, 0, 16);
  coap_set_header_accept(response, APPLICATION_XML);
  coap_set_header_accept(response, APPLICATION_ATOM_XML);
  coap_set_header_if_none_match(response);
}
#endif /* REST_RES_MIRROR */





PROCESS(plugtest_server, "PlugtestServer");
AUTOSTART_PROCESSES(&plugtest_server);

PROCESS_THREAD(plugtest_server, ev, data)
{
  PROCESS_BEGIN();

  PRINTF("ETSI IoT CoAP Plugtests Server\n");

#ifdef RF_CHANNEL
  PRINTF("RF channel: %u\n", RF_CHANNEL);
#endif
#ifdef IEEE802154_PANID
  PRINTF("PAN ID: 0x%04X\n", IEEE802154_PANID);
#endif

  PRINTF("uIP buffer: %u\n", UIP_BUFSIZE);
  PRINTF("LL header: %u\n", UIP_LLH_LEN);
  PRINTF("IP+UDP header: %u\n", UIP_IPUDPH_LEN);
  PRINTF("REST max chunk: %u\n", REST_MAX_CHUNK_SIZE);

  /* Initialize the REST engine. */
  rest_init_engine();

  /* Activate the application-specific resources. */
#if REST_RES_TEST
  rest_activate_resource(&resource_test);
  rest_activate_resource(&resource_validate);
  rest_activate_resource(&resource_create1);
  rest_activate_resource(&resource_create2);
  rest_activate_resource(&resource_create3);
#endif
#if REST_RES_LONG
  rest_activate_resource(&resource_longpath);
#endif
#if REST_RES_QUERY
  rest_activate_resource(&resource_query);
#endif
#if REST_RES_LOC_QUERY
  rest_activate_resource(&resource_locquery);
#endif
#if REST_RES_MULTI
  rest_activate_resource(&resource_multi);
#endif
#if REST_RES_LINKS
  rest_activate_resource(&resource_link1);
  rest_activate_resource(&resource_link2);
  rest_activate_resource(&resource_link3);
#endif
#if REST_RES_PATH
  rest_activate_resource(&resource_path);
#endif
#if REST_RES_SEPARATE
  rest_activate_periodic_resource(&periodic_resource_separate);
#endif
#if REST_RES_LARGE
  rest_activate_resource(&resource_large);
#endif
#if REST_RES_LARGE_UPDATE
  large_update_ct = REST.type.APPLICATION_OCTET_STREAM;
  rest_activate_resource(&resource_large_update);
#endif
#if REST_RES_LARGE_CREATE
  rest_activate_resource(&resource_large_create);
#endif
#if REST_RES_OBS
  rest_activate_periodic_resource(&periodic_resource_obs);
#endif

#if REST_RES_MIRROR
  rest_activate_resource(&resource_mirror);
#endif

  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();

  } /* while (1) */

  PROCESS_END();
}
