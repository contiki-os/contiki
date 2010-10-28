#include "contiki.h"
#include <string.h> /*for string operations in match_addresses*/
#include "rest.h"
#include "buffer.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*FIXME it is possible to define some of the rest functions as MACROs rather than functions full of ifdefs.*/

LIST(restful_services);

void
rest_init(void)
{
  list_init(restful_services);

#ifdef WITH_COAP
  coap_set_service_callback(rest_invoke_restful_service);
#else /*WITH_COAP*/
  http_set_service_callback(rest_invoke_restful_service);
#endif /*WITH_COAP*/

  //start the coap server
  process_start(SERVER_PROCESS, NULL);
}

void
rest_activate_resource(Resource_t* resource)
{
  /*add it to the restful web service link list*/
  list_add(restful_services, resource);
}

int
rest_invoke_restful_service(REQUEST* request, RESPONSE* response)
{
  int found = 0;
  const char* url = request->url;
  uint16_t url_len = request->url_len;

  PRINTF("rest_invoke_restful_service url %s url_len %d -->\n", url, url_len);

  Resource_t* resource = NULL;

  for ( resource = (Resource_t*)list_head(restful_services); resource ; resource = resource->next ){
    /*if the web service handles that kind of requests and urls matches*/
    /*FIXME Need to make case insensitive?*/
    if (url && strlen(resource->url) == url_len && strncmp(resource->url, url, url_len) == 0){
      found = 1;
      method_t method = rest_get_method_type(request);

      PRINTF("method %u, resource->methods_to_handle %u\n", (uint16_t)method, resource->methods_to_handle);

      //DY FIX_ME resources can only handle 1 method currently anyway, fix this
      if ( resource->methods_to_handle & method ) {

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

void
rest_set_user_data(Resource_t* resource, void* user_data)
{
  resource->user_data = user_data;
}

void*
rest_get_user_data(Resource_t* resource)
{
  return resource->user_data;
}

void
rest_set_pre_handler(Resource_t* resource, restful_pre_handler pre_handler)
{
  resource->pre_handler = pre_handler;
}

void
rest_set_post_handler(Resource_t* resource, restful_post_handler post_handler)
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
  coap_set_code(response, (uint8_t)status);
#else /*WITH_COAP*/
  http_set_status(response, status);
#endif /*WITH_COAP*/
}


method_t
rest_get_method_type(REQUEST* request)
{
#ifdef WITH_COAP
  return (method_t)((request->code) << (request->code - 1));
#else
  return (method_t)(request->request_type);
#endif
}

void
rest_set_payload(RESPONSE* response, uint8_t* payload, uint16_t size)
{
#ifdef WITH_COAP
  coap_set_payload(response, payload, size);
#else
  http_set_payload(response, payload, size);
#endif /*WITH_COAP*/
}

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
