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
 *      An abstraction layer for RESTful Web services
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#ifndef ERBIUM_H_
#define ERBIUM_H_

/*includes*/
#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"

/*
 * The maximum buffer size that is provided for resource responses and must be respected due to the limited IP buffer.
 * Larger data must be handled by the resource and will be sent chunk-wise through a TCP stream or CoAP blocks.
 */
#ifndef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE     128
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */

/* REST method types */
typedef enum {
  /* methods to handle */
  METHOD_GET = (1 << 0),
  METHOD_POST = (1 << 1),
  METHOD_PUT = (1 << 2),
  METHOD_DELETE = (1 << 3),

  /* special flags */
  HAS_SUB_RESOURCES = (1<<7)
} rest_resource_flags_t;

struct resource_s;
struct periodic_resource_s;

/* Signatures of handler functions. */
typedef void (*restful_handler) (void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
typedef int (*restful_pre_handler) (struct resource_s *resource, void* request, void* response);
typedef void (*restful_post_handler) (struct resource_s *resource, void* request, void* response);
typedef void (*restful_periodic_handler) (struct resource_s* resource);
typedef void (*restful_response_handler) (void *data, void* response);

/* Signature of the rest-engine service function. */
typedef int (* service_callback_t)(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/**
 * The structure of a MAC protocol driver in Contiki.
 */
struct rest_implementation_status
{
  const unsigned int OK;                        /* CONTENT_2_05,                  OK_200 */
  const unsigned int CREATED;                   /* CREATED_2_01,                  CREATED_201 */
  const unsigned int CHANGED;                   /* CHANGED_2_04,                  NO_CONTENT_204 */
  const unsigned int DELETED;                   /* DELETED_2_02,                  NO_CONTENT_204 */
  const unsigned int NOT_MODIFIED;              /* VALID_2_03,                    NOT_MODIFIED_304 */

  const unsigned int BAD_REQUEST;               /* BAD_REQUEST_4_00,              BAD_REQUEST_400 */
  const unsigned int UNAUTHORIZED;              /* UNAUTHORIZED_4_01,             UNAUTHORIZED_401 */
  const unsigned int BAD_OPTION;                /* BAD_OPTION_4_02,               BAD_REQUEST_400 */
  const unsigned int FORBIDDEN;                 /* FORBIDDEN_4_03,                FORBIDDEN_403 */
  const unsigned int NOT_FOUND;                 /* NOT_FOUND_4_04,                NOT_FOUND_404 */
  const unsigned int METHOD_NOT_ALLOWED;        /* METHOD_NOT_ALLOWED_4_05,       METHOD_NOT_ALLOWED_405 */
  const unsigned int REQUEST_ENTITY_TOO_LARGE;  /* REQUEST_ENTITY_TOO_LARGE_4_13, REQUEST_ENTITY_TOO_LARGE_413 */
  const unsigned int UNSUPPORTED_MADIA_TYPE;    /* UNSUPPORTED_MADIA_TYPE_4_15,   UNSUPPORTED_MADIA_TYPE_415 */

  const unsigned int INTERNAL_SERVER_ERROR;     /* INTERNAL_SERVER_ERROR_5_00,    INTERNAL_SERVER_ERROR_500 */
  const unsigned int NOT_IMPLEMENTED;           /* NOT_IMPLEMENTED_5_01,          NOT_IMPLEMENTED_501 */
  const unsigned int BAD_GATEWAY;               /* BAD_GATEWAY_5_02,              BAD_GATEWAY_502 */
  const unsigned int SERVICE_UNAVAILABLE;       /* SERVICE_UNAVAILABLE_5_03,      SERVICE_UNAVAILABLE_503 */
  const unsigned int GATEWAY_TIMEOUT;           /* GATEWAY_TIMEOUT_5_04,          GATEWAY_TIMEOUT_504 */
  const unsigned int PROXYING_NOT_SUPPORTED;    /* PROXYING_NOT_SUPPORTED_5_05,   INTERNAL_SERVER_ERROR_500 */
};
struct rest_implementation_type
{
  unsigned int TEXT_PLAIN;
  unsigned int TEXT_XML;
  unsigned int TEXT_CSV;
  unsigned int TEXT_HTML;
  unsigned int IMAGE_GIF;
  unsigned int IMAGE_JPEG;
  unsigned int IMAGE_PNG;
  unsigned int IMAGE_TIFF;
  unsigned int AUDIO_RAW;
  unsigned int VIDEO_RAW;
  unsigned int APPLICATION_LINK_FORMAT;
  unsigned int APPLICATION_XML;
  unsigned int APPLICATION_OCTET_STREAM;
  unsigned int APPLICATION_RDF_XML;
  unsigned int APPLICATION_SOAP_XML;
  unsigned int APPLICATION_ATOM_XML;
  unsigned int APPLICATION_XMPP_XML;
  unsigned int APPLICATION_EXI;
  unsigned int APPLICATION_FASTINFOSET;
  unsigned int APPLICATION_SOAP_FASTINFOSET;
  unsigned int APPLICATION_JSON;
  unsigned int APPLICATION_X_OBIX_BINARY;
};

/*
 * Data structure representing a resource in REST.
 */
struct resource_s {
  struct resource_s *next; /* for LIST, points to next resource defined */
  rest_resource_flags_t flags; /* handled RESTful methods */
  const char* url; /*handled URL*/
  const char* attributes; /* link-format attributes */
  restful_handler handler; /* handler function */
  restful_pre_handler pre_handler; /* to be called before handler, may perform initializations */
  restful_post_handler post_handler; /* to be called after handler, may perform finalizations (cleanup, etc) */
  void* user_data; /* pointer to user specific data */
  unsigned int benchmark; /* to benchmark resource handler, used for separate response */
};
typedef struct resource_s resource_t;

struct periodic_resource_s {
  struct periodic_resource_s *next; /* for LIST, points to next resource defined */
  resource_t *resource;
  uint32_t period;
  struct etimer periodic_timer;
  restful_periodic_handler periodic_handler;
};
typedef struct periodic_resource_s periodic_resource_t;

struct rest_implementation {
  char *name;

  /** Initialize the REST implementation. */
  void (* init)(void);

  /** Register the RESTful service callback at implementation */
  void (* set_service_callback)(service_callback_t callback);

  /** Get request URI path */
  int (* get_url)(void *request, const char **url);

  int (* set_url)(void *request, const char *url);

  /** Get the method of a request. */
  rest_resource_flags_t (* get_method_type)(void *request);

  /** Set the status code of a response. */
  int (* set_response_status)(void *response, unsigned int code);

  /** Get the content-type of a request. */
  unsigned int (* get_header_content_type)(void *request);

  /** Set the content-type of a response. */
  int (* set_header_content_type)(void *response, unsigned int content_type);

  int (* get_header_accept)(void *request, const uint16_t **accept);

  /** Get the Max-Age option of a request. */
  int (* get_header_max_age)(void *request, uint32_t *age);

  /** Set the Max-Age option of a response. */
  int (* set_header_max_age)(void *response, uint32_t age);

  /** Set the ETag option of a response. */
  int (* set_header_etag)(void *response, const uint8_t *etag, size_t length);

  /** Get the If-Match option of a request. */
  int (* get_header_if_match)(void *request, const uint8_t **etag);

  /** Get the If-Match option of a request. */
  int (* get_header_if_none_match)(void *request);

  /** Get the Host option of a request. */
  int (* get_header_host)(void *request, const char **host);

  /** Set the location option of a response. */
  int (* set_header_location)(void *response, const char *location);

  /** Get the payload option of a request. */
  int (* get_request_payload)(void *request, uint8_t **payload);

  /** Set the payload option of a response. */
  int (* set_response_payload)(void *response, const void *payload, size_t length);

  /** Get the query string of a request. */
  int (* get_query)(void *request, const char **value);

  /** Get the value of a request query key-value pair. */
  int (* get_query_variable)(void *request, const char *name, const char **value);

  /** Get the value of a request POST key-value pair. */
  int (* get_post_variable)(void *request, const char *name, const char **value);

  /** Send the payload to all subscribers of the resource at url. */
  void (* notify_subscribers)(resource_t *resource, uint16_t counter, void *notification);

  /** The handler for resource subscriptions. */
  restful_post_handler subscription_handler;

  /** A default pre-handler that is assigned with the RESOURCE macro. */
  restful_pre_handler default_pre_handler;

  /** A default post-handler that is assigned with the RESOURCE macro. */
  restful_post_handler default_post_handler;

  /* REST status codes. */
  const struct rest_implementation_status status;

  /* REST content-types. */
  const struct rest_implementation_type type;
};

/*
 * Instance of REST implementation
 */
extern const struct rest_implementation REST;

/*
 * Macro to define a Resource
 * Resources are statically defined for the sake of efficiency and better memory management.
 */
#define RESOURCE(name, flags, url, attributes) \
void name##_handler(void *, void *, uint8_t *, uint16_t, int32_t *); \
resource_t resource_##name = {NULL, flags, url, attributes, name##_handler, NULL, NULL, NULL}

/*
 * Macro to define a sub-resource
 * Make sure to define its parent resource beforehand and set 'parent' to that name.
 */
#define SUB_RESOURCE(name, flags, url, attributes, parent) \
resource_t resource_##name = {NULL, flags, url, attributes, parent##_handler, NULL, NULL, NULL}

/*
 * Macro to define an event resource
 * Like periodic resources, event resources have a post_handler that manages a subscriber list.
 * Instead of a periodic_handler, an event_callback must be provided.
 */
#define EVENT_RESOURCE(name, flags, url, attributes) \
void name##_handler(void *, void *, uint8_t *, uint16_t, int32_t *); \
void name##_event_handler(resource_t*); \
resource_t resource_##name = {NULL, flags, url, attributes, name##_handler, NULL, NULL, NULL}

/*
 * Macro to define a periodic resource
 * The corresponding [name]_periodic_handler() function will be called every period.
 * For instance polling a sensor and publishing a changed value to subscribed clients would be done there.
 * The subscriber list will be maintained by the post_handler rest_subscription_handler() (see rest-mapping header file).
 */
#define PERIODIC_RESOURCE(name, flags, url, attributes, period) \
void name##_handler(void *, void *, uint8_t *, uint16_t, int32_t *); \
resource_t resource_##name = {NULL, flags, url, attributes, name##_handler, NULL, NULL, NULL}; \
void name##_periodic_handler(resource_t*); \
periodic_resource_t periodic_resource_##name = {NULL, &resource_##name, period, {{0}}, name##_periodic_handler}


/*
 * Initializes REST framework and starts HTTP or COAP process
 */
void rest_init_engine(void);

/*
 * Resources wanted to be accessible should be activated with the following code.
 */
void rest_activate_resource(resource_t* resource);
void rest_activate_periodic_resource(periodic_resource_t* periodic_resource);
void rest_activate_event_resource(resource_t* resource);


/*
 * To be called by HTTP/COAP server as a callback function when a new service request appears.
 * This function dispatches the corresponding RESTful service.
 */
int rest_invoke_restful_service(void* request, void* response, uint8_t *buffer, uint16_t buffer_size, int32_t *offset);

/*
 * Returns the resource list
 */
list_t rest_get_resources(void);

/*
 * Getter and setter methods for user specific data.
 */
void* rest_get_user_data(resource_t* resource);
void rest_set_user_data(resource_t* resource, void* user_data);

/*
 * Sets the pre handler function of the Resource.
 * If set, this function will be called just before the original handler function.
 * Can be used to setup work before resource handling.
 */
void rest_set_pre_handler(resource_t* resource, restful_pre_handler pre_handler);

/*
 * Sets the post handler function of the Resource.
 * If set, this function will be called just after the original handler function.
 * Can be used to do cleanup (deallocate memory, etc) after resource handling.
 */
void rest_set_post_handler(resource_t* resource, restful_post_handler post_handler);

/*
 * Sets resource flags for special properties, e.g., handling of sub-resources of URI-path.
 */
void rest_set_special_flags(resource_t* resource, rest_resource_flags_t flags);

#endif /*ERBIUM_H_*/
