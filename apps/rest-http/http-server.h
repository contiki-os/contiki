#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include "http-common.h"
#include "rest.h"

/*Declare process*/
PROCESS_NAME(http_server);

/*Type definition of the service callback*/
typedef int (*service_callback) (http_request_t* request, http_response_t* response);

/*
 *Setter of the service callback, this callback will be called in case of HTTP request.
 */
void http_set_service_callback(service_callback callback);

/*
 * Setter for the status code (200, 201, etc) of the response.
 */
void http_set_status(http_response_t* response, status_code_t status);

/*
 * Adds the header name and value provided to the response.
 * Name of the header should be hardcoded since it is accessed from code segment
 * (not copied to buffer) whereas value of the header can be copied
 * depending on the relevant parameter. This is needed since some values may be
 * generated dynamically (ex: e-tag value)
 */
int http_set_res_header(http_response_t* response, const char* name, const char* value, int copy);

/*
 * Returns the value of the header name provided. Return NULL if header does not exist.
 */
const char* http_get_req_header(http_request_t* request, const char* name);

int http_set_res_payload(http_response_t* response, uint8_t* payload, uint16_t size);

/*
 * Returns query variable in the URL.
 * Returns true if the variable found, false otherwise.
 * Variable is put in the buffer provided.
 */
int http_get_query_variable(http_request_t* request, const char *name, char* output, uint16_t output_size);

/*
 * Returns variable in the Post Data.
 * Returns true if the variable found, false otherwise.
 * Variable is put in the buffer provided.
 */
int http_get_post_variable(http_request_t* request, const char *name, char* output, uint16_t output_size);

/*
 * Get the header "Content-Type".
 */
const char* http_get_content_type_string(content_type_t content_type);
content_type_t http_get_header_content_type(http_request_t* request);

#endif /*HTTPSERVER_H_*/
