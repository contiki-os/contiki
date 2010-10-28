/*
 * coap-server.h
 *
 *  Created on: Oct 4, 2010
 *      Author: dogan
 */

#ifndef COAPSERVER_H_
#define COAPSERVER_H_

#define COAP_DATA_BUFF_SIZE 300

#include "contiki.h"

/*Declare process*/
PROCESS_NAME(coap_server);

#define MOTE_PORT 61616

int coap_set_payload(coap_packet_t* packet, uint8_t* payload, uint16_t size);

content_type_t coap_get_header_content_type(coap_packet_t* packet);
int coap_set_header_content_type(coap_packet_t* packet, content_type_t content_type);

int coap_set_header_uri(coap_packet_t* packet, char* uri);
int coap_set_header_etag(coap_packet_t* packet, uint8_t* etag, uint8_t size);
void coap_set_code(coap_packet_t* packet, uint8_t code);
int coap_get_query_variable(coap_packet_t* packet, const char *name, char* output, uint16_t output_size);
int coap_get_post_variable(coap_packet_t* packet, const char *name, char* output, uint16_t output_size);

/*Type definition of the service callback*/
typedef int (*service_callback) (coap_packet_t* request, coap_packet_t* response);

/*
 *Setter of the service callback, this callback will be called in case of HTTP request.
 */
void coap_set_service_callback(service_callback callback);

#endif /* COAPSERVER_H_ */
