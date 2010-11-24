/*
 * coap.h
 *
 *  Created on: Aug 25, 2010
 *      Author: dogan
 */

#ifndef COAP_COMMON_H_
#define COAP_COMMON_H_

#include "contiki-net.h"

/*COAP method types*/
typedef enum {
  COAP_GET = 1,
  COAP_POST,
  COAP_PUT,
  COAP_DELETE
} coap_method_t;

typedef enum {
  MESSAGE_TYPE_CON,
  MESSAGE_TYPE_NON,
  MESSAGE_TYPE_ACK,
  MESSAGE_TYPE_RST
} message_type;

typedef enum {
  OK_200 = 80,
  CREATED_201 = 81,
  NOT_MODIFIED_304 = 124,
  BAD_REQUEST_400 = 160,
  NOT_FOUND_404 = 164,
  METHOD_NOT_ALLOWED_405 = 165,
  UNSUPPORTED_MADIA_TYPE_415 = 175,
  INTERNAL_SERVER_ERROR_500 = 200,
  BAD_GATEWAY_502 = 202,
  GATEWAY_TIMEOUT_504 = 204
} status_code_t;

typedef enum {
  Option_Type_Content_Type = 1,
  Option_Type_Max_Age = 2,
  Option_Type_Etag = 4,
  Option_Type_Uri_Authority = 5,
  Option_Type_Location = 6,
  Option_Type_Uri_Path = 9,
  Option_Type_Subscription_Lifetime = 10,
  Option_Type_Token = 11,
  Option_Type_Block = 13,
  Option_Type_Uri_Query = 15
} option_type;

typedef enum {
  TEXT_PLAIN = 0,
  TEXT_XML = 1,
  TEXT_CSV = 2,
  TEXT_HTML = 3,
  IMAGE_GIF = 21,
  IMAGE_JPEG = 22,
  IMAGE_PNG = 23,
  IMAGE_TIFF = 24,
  AUDIO_RAW = 25,
  VIDEO_RAW = 26,
  APPLICATION_LINK_FORMAT = 40,
  APPLICATION_XML = 41,
  APPLICATION_OCTET_STREAM = 42,
  APPLICATION_RDF_XML = 43,
  APPLICATION_SOAP_XML = 44,
  APPLICATION_ATOM_XML = 45,
  APPLICATION_XMPP_XML = 46,
  APPLICATION_EXI = 47,
  APPLICATION_X_BXML = 48,
  APPLICATION_FASTINFOSET = 49,
  APPLICATION_SOAP_FASTINFOSET = 50,
  APPLICATION_JSON = 51
} content_type_t;

#define COAP_HEADER_VERSION_MASK 0xC0
#define COAP_HEADER_TYPE_MASK 0x30
#define COAP_HEADER_OPTION_COUNT_MASK 0x0F
#define COAP_HEADER_OPTION_DELTA_MASK 0xF0
#define COAP_HEADER_OPTION_SHORT_LENGTH_MASK 0x0F

#define COAP_HEADER_VERSION_POSITION 6
#define COAP_HEADER_TYPE_POSITION 4
#define COAP_HEADER_OPTION_DELTA_POSITION 4

#define REQUEST_BUFFER_SIZE 200

#define DEFAULT_CONTENT_TYPE 0
#define DEFAULT_MAX_AGE 60
#define DEFAULT_URI_AUTHORITY ""
#define DEFAULT_URI_PATH ""

//keep open requests and their xactid

struct header_option_t
{
  struct header_option_t* next;
  uint16_t option;
  uint16_t len;
  uint8_t* value;
};
typedef struct header_option_t header_option_t;

struct block_option_t {
  uint32_t number;
  uint8_t more;
  uint8_t size;
};
typedef struct block_option_t block_option_t;

typedef struct
{
  uint8_t ver; //2-bits currently set to 1.
  uint8_t type; //2-bits Confirmable (0), Non-Confirmable (1), Acknowledgment (2) or Reset (3)
  uint8_t option_count; //4-bits
  uint8_t code; //8-bits Method or response code
  uint16_t tid; //16-bit unsigned integer
  header_option_t* options;
  char* url; //put it just as a shortcut or else need to parse options everytime to access it.
  uint16_t url_len;
  char* query;
  uint16_t query_len;
  uint16_t payload_len;
  uint8_t* payload;
  uip_ipaddr_t addr;
} coap_packet_t;

/*error definitions*/
typedef enum
{
  NO_ERROR,

  /*Memory errors*/
  MEMORY_ALLOC_ERR,
  MEMORY_BOUNDARY_EXCEEDED
} error_t;

int serialize_packet(coap_packet_t* request, uint8_t* buffer);
void init_packet(coap_packet_t* packet);

#endif /* COAP_COMMON_H_ */
