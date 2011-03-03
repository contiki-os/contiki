#ifndef REST_H_
#define REST_H_

/*includes*/
#include "contiki.h"
#include "contiki-lib.h"

#ifdef WITH_COAP
  #include "coap-common.h"
#include "coap-server.h"
  #define REQUEST coap_packet_t
  #define RESPONSE coap_packet_t
  #define SERVER_PROCESS (&coap_server)
#else /*WITH_COAP*/
  /*WITH_HTTP*/
  #include "http-common.h"
  #include "http-server.h"
  #define REQUEST http_request_t
  #define RESPONSE http_response_t
  #define SERVER_PROCESS (&http_server)
#endif /*WITH_COAP*/

struct resource_t;

/*REST method types*/
typedef enum {
  METHOD_GET = (1 << 0),
  METHOD_POST = (1 << 1),
  METHOD_PUT = (1 << 2),
  METHOD_DELETE = (1 << 3)
} method_t;

/*Signature of handler functions*/
typedef void (*restful_handler) (REQUEST* request, RESPONSE* response);
typedef int (*restful_pre_handler) (REQUEST* request, RESPONSE* response);
typedef void (*restful_post_handler) (REQUEST* request, RESPONSE* response);

typedef int (*restful_periodic_handler) (struct resource_t* resource);
typedef void (*restful_periodic_request_generator) (REQUEST* request);

/*
 * Data structure representing a resource in REST.
 */
struct resource_t {
  struct resource_t *next; /*points to next resource defined*/
  method_t methods_to_handle; /*handled HTTP methods*/
  const char* url; /*handled URL*/
  restful_handler handler; /*handler function*/
  restful_pre_handler pre_handler; /*to be called before handler, may perform initializations*/
  restful_post_handler post_handler; /*to be called after handler, may perform finalizations (cleanup, etc)*/
  void* user_data; /*pointer to user specific data*/
};
typedef struct resource_t resource_t;

struct periodic_resource_t {
  struct periodic_resource_t *next;
  resource_t *resource;
  uint32_t period;
  struct etimer* handler_cb_timer;
  struct stimer* lifetime_timer;
  restful_periodic_handler periodic_handler;
  restful_periodic_request_generator periodic_request_generator;
  uint32_t lifetime;
  uip_ipaddr_t addr;
  struct uip_udp_conn *client_conn;
};
typedef struct periodic_resource_t periodic_resource_t;

/*
 * Macro to define a Resource
 * Resources are statically defined for the sake of efficiency and better memory management.
 */
#define RESOURCE(name, methods_to_handle, url) \
void name##_handler(REQUEST*, RESPONSE*); \
resource_t resource_##name = {NULL, methods_to_handle, url, name##_handler, NULL, NULL, NULL}

/*
 * Macro to define a Periodic Resource
 */
#define PERIODIC_RESOURCE(name, methods_to_handle, url, period) \
RESOURCE(name, methods_to_handle, url); \
int name##_periodic_handler(resource_t*); \
void name##_periodic_request_generator(REQUEST*); \
struct etimer handler_cb_timer_##name; \
struct stimer lifetime_timer_##name; \
periodic_resource_t periodic_resource_##name = {NULL, &resource_##name, period, &handler_cb_timer_##name, &lifetime_timer_##name, name##_periodic_handler, name##_periodic_request_generator, 0}


/*
 * Initializes REST framework and starts HTTP or COAP process
 */
void rest_init(void);

/*
 * Resources wanted to be accessible should be activated with the following code.
 */
void rest_activate_resource(resource_t* resource);

void rest_activate_periodic_resource(periodic_resource_t* periodic_resource);

/*
 * To be called by HTTP/COAP server as a callback function when a new service request appears.
 * This function dispatches the corresponding RESTful service.
 */
int rest_invoke_restful_service(REQUEST* request, RESPONSE* response);

/*
 * Returns the resource list
 */
list_t rest_get_resources(void);

/*
 * Returns query variable in the URL.
 * Returns true if the variable found, false otherwise.
 * Variable is put in the buffer provided.
 */
int rest_get_query_variable(REQUEST* request, const char *name, char* output, uint16_t output_size);

/*
 * Returns variable in the Post Data/Payload.
 * Returns true if the variable found, false otherwise.
 * Variable is put in the buffer provided.
 */
int rest_get_post_variable(REQUEST* request, const char *name, char* output, uint16_t output_size);

method_t rest_get_method_type(REQUEST* request);
void rest_set_method_type(REQUEST* request, method_t method);

/*
 * Getter for the request content type
 */
content_type_t rest_get_header_content_type(REQUEST* request);

/*
 * Setter for the response content type
 */
int rest_set_header_content_type(RESPONSE* response, content_type_t content_type);

/*
 * Setter for the response etag header
 */
int rest_set_header_etag(RESPONSE* response, uint8_t* etag, uint8_t size);

/*
 * Setter for the status code (200, 201, etc) of the response.
 */
void rest_set_response_status(RESPONSE* response, status_code_t status);

/*
 * Setter for the payload of the request and response
 */
void rest_set_request_payload(RESPONSE* response, uint8_t* payload, uint16_t size);
void rest_set_response_payload(RESPONSE* response, uint8_t* payload, uint16_t size);

/*
 * Getter method for user specific data.
 */
void* rest_get_user_data(resource_t* resource);

/*
 * Setter method for user specific data.
 */
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

#endif /*REST_H_*/
