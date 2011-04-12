#ifndef HTTPCOMMON_H_
#define HTTPCOMMON_H_

/*includes*/
#include "contiki.h"
#include "contiki-net.h"

/*current state of the request, waiting: handling request, output: sending response*/
#define STATE_WAITING 0
#define STATE_OUTPUT  1

/*definitions of the line ending characters*/
#define LINE_FEED_CHAR '\n'
#define CARRIAGE_RETURN_CHAR '\r'

/*needed for web services giving all path (http://172.16.79.0/services/light1)
 * instead relative (/services/light1) in HTTP request. Ex: Restlet lib. does it*/
extern const char* http_string;

/*HTTP method strings*/
extern const char* http_get_string;
extern const char* http_head_string;
extern const char* http_post_string;
extern const char* http_put_string;
extern const char* http_delete_string;

extern const char* httpv1_1;
extern const char* line_end;
extern const char* contiki;
extern const char* close;

/*header names*/
extern const char* HTTP_HEADER_NAME_CONTENT_TYPE;
extern const char* HTTP_HEADER_NAME_CONTENT_LENGTH;
extern const char* HTTP_HEADER_NAME_LOCATION;
extern const char* HTTP_HEADER_NAME_CONNECTION;
extern const char* HTTP_HEADER_NAME_SERVER;
extern const char* HTTP_HEADER_NAME_HOST;
extern const char* HTTP_HEADER_NAME_IF_NONE_MATCH;
extern const char* HTTP_HEADER_NAME_ETAG;

extern const char* header_delimiter;


/*Configuration parameters*/
#define HTTP_PORT 8080
#define HTTP_DATA_BUFF_SIZE 600
#define INCOMING_DATA_BUFF_SIZE 102 /*100+2, 100 = max url len, 2 = space char+'\0'*/

/*HTTP method types*/
typedef enum {
  HTTP_METHOD_GET = (1 << 0),
  HTTP_METHOD_POST = (1 << 1),
  HTTP_METHOD_PUT = (1 << 2),
  HTTP_METHOD_DELETE = (1 << 3)
} http_method_t;

//DY : FIXME right now same enum names with COAP with different values. Will this work fine?
typedef enum {
  OK_200 = 200,
  CREATED_201 = 201,
  NOT_MODIFIED_304 = 304,
  BAD_REQUEST_400 = 400,
  NOT_FOUND_404 = 404,
  METHOD_NOT_ALLOWED_405 = 405,
  REQUEST_URI_TOO_LONG_414 = 414,
  UNSUPPORTED_MADIA_TYPE_415 = 415,
  INTERNAL_SERVER_ERROR_500 = 500,
  BAD_GATEWAY_502 = 502,
  SERVICE_UNAVAILABLE_503 = 503,
  GATEWAY_TIMEOUT_504 = 504
} status_code_t;

typedef enum {
  TEXT_PLAIN,
  TEXT_XML,
  TEXT_CSV,
  TEXT_HTML,
  APPLICATION_XML,
  APPLICATION_EXI,
  APPLICATION_JSON,
  APPLICATION_LINK_FORMAT,
  APPLICATION_WWW_FORM,
  UNKNOWN_CONTENT_TYPE
} content_type_t;

/*Header type*/
struct http_header_t {
  struct http_header_t* next;
  char* name;
  char* value;
};
typedef struct http_header_t http_header_t;

/*This structure contains information about the HTTP request.*/
struct http_request_t {
  char* url;
  uint16_t url_len;
  http_method_t request_type; /* GET, POST, etc */
  char* query;
  uint16_t query_len;
  http_header_t* headers;
  uint16_t payload_len;
  uint8_t* payload;
};
typedef struct http_request_t http_request_t;

/*This structure contains information about the HTTP response.*/
struct http_response_t {
  status_code_t status_code;
  char* status_string;
  http_header_t* headers;
  uint16_t payload_len;
  uint8_t* payload;
};
typedef struct http_response_t http_response_t;

/*This structure contains information about the TCP Connection.*/
typedef struct {
  struct psock sin, sout; /*Protosockets for incoming and outgoing communication*/
  struct pt outputpt;
  char inputbuf[INCOMING_DATA_BUFF_SIZE]; /*to put incoming data in*/
  uint8_t state;
  http_request_t request;
  http_response_t response;
} connection_state_t;

/*error definitions*/
typedef enum {
  HTTP_NO_ERROR,

  /*Memory errors*/
  HTTP_MEMORY_ALLOC_ERR,
  HTTP_MEMORY_BOUNDARY_EXCEEDED,

  /*specific errors*/
  HTTP_XML_NOT_VALID,
  HTTP_SOAP_MESSAGE_NOT_VALID,
  HTTP_URL_TOO_LONG,
  HTTP_URL_INVALID
} http_error_t;

#endif /*HTTPCOMMON_H_*/
