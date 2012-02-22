#include "contiki.h"
#include <string.h> /*for string operations in match_addresses*/
#include "rest.h"
#include "buffer.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

/*FIXME it is possible to define some of the rest functions as MACROs rather than functions full of ifdefs.*/

PROCESS_NAME(rest_manager_process);

LIST(restful_services);
LIST(restful_periodic_services);

void
rest_init(void)
{
  list_init(restful_services);

#ifdef WITH_COAP
  coap_set_service_callback(rest_invoke_restful_service);
#else /*WITH_COAP*/
  http_set_service_callback(rest_invoke_restful_service);
#endif /*WITH_COAP*/

  /*Start rest framework process*/
  process_start(&rest_manager_process, NULL);
}

void
rest_activate_resource(resource_t* resource)
{
  /*add it to the restful web service link list*/
  list_add(restful_services, resource);
}

void
rest_activate_periodic_resource(periodic_resource_t* periodic_resource)
{
  list_add(restful_periodic_services, periodic_resource);
  rest_activate_resource(periodic_resource->resource);
}

void
rest_set_user_data(resource_t* resource, void* user_data)
{
  resource->user_data = user_data;
}

void*
rest_get_user_data(resource_t* resource)
{
  return resource->user_data;
}

void
rest_set_pre_handler(resource_t* resource, restful_pre_handler pre_handler)
{
  resource->pre_handler = pre_handler;
}

void
rest_set_post_handler(resource_t* resource, restful_post_handler post_handler)
{
  resource->post_handler = post_handler;
}

list_t
rest_get_resources(void)
{
  return restful_services;
}

void
rest_set_response_status(RESPONSE* response, status_code_t status)
{
#ifdef WITH_COAP
  coap_set_code(response, status);
#else /*WITH_COAP*/
  http_set_status(response, status);
#endif /*WITH_COAP*/
}

#ifdef WITH_COAP
static method_t coap_to_rest_method(coap_method_t method)
{
  return (method_t)(1 << (method - 1));
}

static coap_method_t rest_to_coap_method(method_t method)
{
  coap_method_t coap_method = COAP_GET;
  switch (method) {
  case METHOD_GET:
    coap_method = COAP_GET;
    break;
  case METHOD_POST:
    coap_method = COAP_POST;
      break;
  case METHOD_PUT:
    coap_method = COAP_PUT;
      break;
  case METHOD_DELETE:
    coap_method = COAP_DELETE;
      break;
  default:
    break;
  }
  return coap_method;
}
#endif /*WITH_COAP*/

method_t
rest_get_method_type(REQUEST* request)
{
#ifdef WITH_COAP
  return coap_to_rest_method(coap_get_method(request));
#else
  return (method_t)(request->request_type);
#endif
}

/*Only defined for COAP for now.*/
#ifdef WITH_COAP
void
rest_set_method_type(REQUEST* request, method_t method)
{
  coap_set_method(request, rest_to_coap_method(method));
}
#endif /*WITH_COAP*/

void
rest_set_response_payload(RESPONSE* response, uint8_t* payload, uint16_t size)
{
#ifdef WITH_COAP
  coap_set_payload(response, payload, size);
#else
  http_set_res_payload(response, payload, size);
#endif /*WITH_COAP*/
}

/*Only defined for COAP for now.*/
#ifdef WITH_COAP
void
rest_set_request_payload(REQUEST* request, uint8_t* payload, uint16_t size)
{
  coap_set_payload(request, payload, size);
}
#endif /*WITH_COAP*/

int
rest_get_query_variable(REQUEST* request, const char *name, char* output, uint16_t output_size)
{
#ifdef WITH_COAP
  return coap_get_query_variable(request, name, output, output_size);
#else
  return http_get_query_variable(request, name, output, output_size);
#endif /*WITH_COAP*/
}

int
rest_get_post_variable(REQUEST* request, const char *name, char* output, uint16_t output_size)
{
#ifdef WITH_COAP
  return coap_get_post_variable(request, name, output, output_size);
#else
  return http_get_post_variable(request, name, output, output_size);
#endif /*WITH_COAP*/
}

content_type_t
rest_get_header_content_type(REQUEST* request)
{
#ifdef WITH_COAP
  return coap_get_header_content_type(request);
#else
  return http_get_header_content_type(request);
#endif /*WITH_COAP*/
}

int
rest_set_header_content_type(RESPONSE* response, content_type_t content_type)
{
#ifdef WITH_COAP
  return coap_set_header_content_type(response, content_type);
#else
  return http_set_res_header(response, HTTP_HEADER_NAME_CONTENT_TYPE, http_get_content_type_string(content_type), 1);
#endif /*WITH_COAP*/

}

int
rest_set_header_etag(RESPONSE* response, uint8_t* etag, uint8_t size)
{
#ifdef WITH_COAP
  return coap_set_header_etag(response, etag, size);
#else
  /*FIXME for now etag should be a "/0" ending string for http part*/
  char temp_etag[10];
  memcpy(temp_etag, etag, size);
  temp_etag[size] = 0;
  return http_set_res_header(response, HTTP_HEADER_NAME_ETAG, temp_etag, 1);
#endif /*WITH_COAP*/
}

int
rest_invoke_restful_service(REQUEST* request, RESPONSE* response)
{
  int found = 0;
  const char* url = request->url;
  uint16_t url_len = request->url_len;

  PRINTF("rest_invoke_restful_service url %s url_len %d -->\n", url, url_len);

  resource_t* resource = NULL;

  for (resource = (resource_t*)list_head(restful_services); resource; resource = resource->next) {
    /*if the web service handles that kind of requests and urls matches*/
    if (url && strlen(resource->url) == url_len && strncmp(resource->url, url, url_len) == 0){
      found = 1;
      method_t method = rest_get_method_type(request);

      PRINTF("method %u, resource->methods_to_handle %u\n", (uint16_t)method, resource->methods_to_handle);

      if (resource->methods_to_handle & method) {

        /*FIXME Need to move somewhere else*/
        #ifdef WITH_COAP
        uint32_t lifetime = 0;
        if (coap_get_header_subscription_lifetime(request, &lifetime)) {
          PRINTF("Lifetime %lu\n", lifetime);

          periodic_resource_t* periodic_resource = NULL;
          for (periodic_resource = (periodic_resource_t*)list_head(restful_periodic_services);
               periodic_resource;
               periodic_resource = periodic_resource->next) {
            if (periodic_resource->resource == resource) {
              PRINTF("Periodic Resource Found\n");
              PRINT6ADDR(&request->addr);
              periodic_resource->lifetime = lifetime;
              stimer_set(periodic_resource->lifetime_timer, lifetime);
              uip_ipaddr_copy(&periodic_resource->addr, &request->addr);
            }
          }
        }
        #endif /*WITH_COAP*/

        /*call pre handler if it exists*/
        if (!resource->pre_handler || resource->pre_handler(request, response)) {
          /* call handler function*/
          resource->handler(request, response);

          /*call post handler if it exists*/
          if (resource->post_handler) {
            resource->post_handler(request, response);
          }
        }
      } else {
        rest_set_response_status(response, METHOD_NOT_ALLOWED_405);
      }
      break;
    }
  }

  if (!found) {
    rest_set_response_status(response, NOT_FOUND_404);
  }

  return found;
}

PROCESS(rest_manager_process, "Rest Process");

PROCESS_THREAD(rest_manager_process, ev, data)
{
  PROCESS_BEGIN();

  /*start the coap or http server*/
  process_start(SERVER_PROCESS, NULL);

  PROCESS_PAUSE();

  /*Periodic resources are only available to COAP implementation*/
#if 0
#ifdef WITH_COAP
  periodic_resource_t* periodic_resource = NULL;
  for (periodic_resource = (periodic_resource_t*)list_head(restful_periodic_services); periodic_resource; periodic_resource = periodic_resource->next) {
    if (periodic_resource->period) {
      PRINTF("Set timer for Res: %s to %lu\n", periodic_resource->resource->url, periodic_resource->period);
      etimer_set(periodic_resource->handler_cb_timer, periodic_resource->period);
    }
  }

  while(1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_TIMER) {
      for (periodic_resource = (periodic_resource_t*)list_head(restful_periodic_services);periodic_resource;periodic_resource = periodic_resource->next) {
        if (periodic_resource->period && etimer_expired(periodic_resource->handler_cb_timer)) {
          PRINTF("Etimer expired for %s (period:%lu life:%lu)\n", periodic_resource->resource->url, periodic_resource->period, periodic_resource->lifetime);
          /*call the periodic handler function if exists*/
          if (periodic_resource->periodic_handler) {
            if ((periodic_resource->periodic_handler)(periodic_resource->resource)) {
              PRINTF("RES CHANGE\n");
              if (!stimer_expired(periodic_resource->lifetime_timer)) {
                PRINTF("TIMER NOT EXPIRED\n");
                resource_changed(periodic_resource);
                periodic_resource->lifetime = stimer_remaining(periodic_resource->lifetime_timer);
              } else {
                periodic_resource->lifetime = 0;
              }
            }

            PRINTF("%s lifetime %lu (%lu) expired %d\n", periodic_resource->resource->url, stimer_remaining(periodic_resource->lifetime_timer), periodic_resource->lifetime, stimer_expired(periodic_resource->lifetime_timer));
          }
          etimer_reset(periodic_resource->handler_cb_timer);
        }
      }
    }
  }
#endif /*WITH_COAP*/
#endif
  PROCESS_END();
}
