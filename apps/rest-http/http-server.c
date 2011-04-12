#include <stdio.h>
#include <stdlib.h> /*for atoi*/
#include <string.h>
#include "contiki.h"

#include "http-server.h"
#include "buffer.h"
#include "rest-util.h"

#if !UIP_CONF_IPV6_RPL && !defined (CONTIKI_TARGET_MINIMAL_NET)
#include "static-routing.h"
#endif

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

static void
init_response(http_response_t* response)
{
  response->status_code = OK_200;
  response->status_string = NULL;
  response->headers = NULL;
  response->payload = NULL;
  response->payload_len = 0;
}

static void
init_request(http_request_t* request)
{
  request->request_type = 0;
  request->url = NULL;
  request->url_len = 0;
  request->query = NULL;
  request->query_len = 0;
  request->headers = NULL;
  request->payload = NULL;
  request->payload_len = 0;
}

/**
 * Initializes the connection state by clearing out the data structures
 */
static void
init_connection(connection_state_t* conn_state)
{
  conn_state->state = STATE_WAITING;

  init_request(&conn_state->request);
  init_response(&conn_state->response);
}

void
http_set_status(http_response_t* response, status_code_t status)
{
  response->status_code = status;
}

static http_header_t*
allocate_header(uint16_t variable_len)
{
  PRINTF("sizeof http_header_t %u variable size %u\n", sizeof(http_header_t), variable_len);
  uint8_t* buffer = allocate_buffer(sizeof(http_header_t) + variable_len);
  if (buffer) {
    http_header_t* option = (http_header_t*) buffer;
    option->next = NULL;
    option->name = NULL;
    option->value = buffer + sizeof(http_header_t);
    return option;
  }

  return NULL;
}

int
http_set_res_header(http_response_t* response, const char* name, const char* value, int copy)
{
  PRINTF("http_set_res_header (copy:%d) %s:%s\n", copy, name, value);
  uint16_t size = 0;
  http_header_t* current_header = NULL;
  http_header_t* head = NULL;

  if (copy) {
    size += strlen(value) + 1;
  }

  current_header = allocate_header(size);

  if (current_header) {
    current_header->name = (char*)name;
    if (copy) {
      strcpy(current_header->value, value);
    } else {
      current_header->value = (char*)value;
    }

    head = response->headers;
    response->headers = current_header;
    if (head) {
      current_header->next = head;
    }

    return 1;
  }

  return 0;
}

static const char* is_request_hdr_needed(const char* header_name)
{
  const char* header = NULL;
  /*FIXME add needed headers here*/
  if (strcmp(header_name, HTTP_HEADER_NAME_CONTENT_LENGTH) == 0) {
    header = HTTP_HEADER_NAME_CONTENT_LENGTH;
  } else if (strcmp(header_name, HTTP_HEADER_NAME_CONTENT_TYPE) == 0) {
    header = HTTP_HEADER_NAME_CONTENT_TYPE;
  }

  return header;
}

static service_callback service_cbk = NULL;

void
http_set_service_callback(service_callback callback)
{
  service_cbk = callback;
}

const char* content_types[] = {
  "text/plain",
  "text/xml",
  "text/csv",
  "text/html",
  "application/xml",
  "application/exi",
  "application/json",
  "application/link-format",
  "application/x-www-form-urlencoded",
};

const char*
http_get_content_type_string(content_type_t content_type)
{
  return content_types[content_type];
}

char*
get_default_status_string(status_code_t status_code)
{
  char* value = NULL;
  switch(status_code) {
    case 200:
      value = "OK";
      break;
    case 201:
      value = "Created";
      break;
    case 202:
      value = "Accepted";
      break;
    case 204:
      value = "No Content";
      break;
    case 304:
      value = "Not Modified";
      break;
    case 400:
      value = "Bad Request" ;
      break;
    case 404:
      value = "Not Found" ;
      break;
    case 405:
      value = "Method Not Allowed" ;
      break;
    case 406:
      value = "Not Acceptable" ;
      break;
    case 414:
      value = "Request-URI Too Long" ;
      break;
    case 415:
      value = "Unsupported Media Type" ;
      break;
    case 500:
      value = "Internal Server Error" ;
      break;
    case 501:
      value = "Not Implemented" ;
      break;
    case 503:
      value = "Service Unavailable" ;
      break;
    /*FIXME : will be removed later, put to catch the unhandled statuses.*/
    default:
      value = "$$BUG$$";
      break;
  }

  return value;
}

int
http_get_query_variable(http_request_t* request, const char *name, char* output, uint16_t output_size)
{
  if (request->query) {
    return get_variable(name, request->query, request->query_len, output, output_size, 0);
  }

  return 0;
}

int
http_get_post_variable(http_request_t* request, const char *name, char* output, uint16_t output_size)
{
  if (request->payload) {
    return get_variable(name, request->payload, request->payload_len, output, output_size, 1);
  }

  return 0;
}

static int
is_method_handled(connection_state_t* conn_state, const char* method)
{
  /*other method types can be added here if needed*/
  if(strncmp(method, http_get_string, 3) == 0) {
    conn_state->request.request_type = HTTP_METHOD_GET;
  } else if (strncmp(method, http_post_string, 4) == 0) {
    conn_state->request.request_type = HTTP_METHOD_POST;
  } else if (strncmp(method, http_put_string, 3) == 0) {
    conn_state->request.request_type = HTTP_METHOD_PUT;
  } else if (strncmp(method, http_delete_string, 3) == 0) {
    conn_state->request.request_type = HTTP_METHOD_DELETE;
  } else {
    PRINTF("No Method supported : %s\nstate : %d\n", conn_state->inputbuf, conn_state->state);
    return 0;
  }

  return 1;
}

static int
parse_url(connection_state_t* conn_state, char* url)
{
  int error = HTTP_NO_ERROR;
  int full_url_path = 0;
  /*even for default index.html there is / Ex: GET / HTTP/1.1*/
  if (url[0] != '/') {
    /*if url is complete (http://...) rather than relative*/
    if (strncmp(url, http_string, 4) != 0 ) {
      PRINTF("Url not valid : %s \n",url);
      error = HTTP_URL_INVALID;
    } else {
      full_url_path = 1;
    }
  }

  if (error == HTTP_NO_ERROR) {
    char* url_buffer = url;
    if (full_url_path) {
      unsigned char num_of_slash = 0;
      do {
        url_buffer = strchr( ++url_buffer, '/' );

        PRINTF("Buffer : %s %d\n", url_buffer, num_of_slash);

      } while (url_buffer && ++num_of_slash < 3);
    }

    PRINTF("Url found :%s\n", url_buffer);

    /*Get rid of the first slash*/
    if (url_buffer && ++url_buffer) {
      conn_state->request.url = (char*) copy_text_to_buffer(url_buffer);
      conn_state->request.url_len = strlen(url_buffer);

      if ((conn_state->request.query = strchr(conn_state->request.url, '?'))) {
        *(conn_state->request.query++) = 0;
        /*update url len - decrease the size of query*/
        conn_state->request.url_len = strlen(conn_state->request.url);
        conn_state->request.query_len = strlen(conn_state->request.query);
      }

      PRINTF("url %s, url_len %u, query %s, query_len %u\n", conn_state->request.url, conn_state->request.url_len, conn_state->request.query, conn_state->request.query_len);

      /*FIXME url is not decoded - should be done here*/
    } else {
      error = HTTP_URL_INVALID;
    }
  }

  return error;
}

static int
parse_header(connection_state_t* conn_state, char* inputbuf)
{
  PRINTF("parse_header --->\n");
  const char* header_name = NULL;

  char* delimiter = strchr(inputbuf, ':');
  if (delimiter) {
    *delimiter++ = 0; /*after increment delimiter will point space char*/

    header_name = is_request_hdr_needed(inputbuf);
    if (header_name && delimiter) {
      char* buffer = delimiter;

      if (buffer[0] == ' ') {
        buffer++;
      }

      http_header_t* current_header = NULL;
      http_header_t* head = NULL;

      current_header = allocate_header(strlen(buffer));

      if (current_header) {
        current_header->name = (char*)header_name;
        strcpy(current_header->value, buffer);
      }

      head = conn_state->request.headers;
      conn_state->request.headers = current_header;
      if (head) {
        current_header->next = head;
      }

      return 1;
    }
  }

  return 0;
}

int
http_set_res_payload(http_response_t* response, uint8_t* payload, uint16_t size)
{
  response->payload = copy_to_buffer(payload, size);
  if (response->payload) {
    response->payload_len = size;
    return 1;
  }

  return 0;
}

static const char*
get_header(http_header_t* headers, const char* hdr_name)
{
  for (;headers; headers = headers->next) {
    if (strcmp(headers->name, hdr_name) == 0) {
      return headers->value;
    }
  }

  return NULL;
}

const char* http_get_req_header(http_request_t* request, const char* name)
{
  return get_header(request->headers, name);
}

content_type_t http_get_header_content_type(http_request_t* request)
{
  const char* content_type_string = http_get_req_header(request, HTTP_HEADER_NAME_CONTENT_TYPE);
  if (content_type_string) {
    int i = 0;
    for(; i < sizeof(content_types)/sizeof(const char*) ; i++) {
      if (strcmp(content_types[i], content_type_string)) {
        return (content_type_t)i;
      }
    }
  }

  return UNKNOWN_CONTENT_TYPE;
}

static
PT_THREAD(handle_request(connection_state_t* conn_state))
{
  static int error;
  const char* content_len;

  PSOCK_BEGIN(&(conn_state->sin));

  content_len = NULL;

  error = HTTP_NO_ERROR; /*always reinit static variables due to protothreads*/

  PRINTF("Request--->\n");

  //read method
  PSOCK_READTO(&(conn_state->sin), ' ');

  if (!is_method_handled(conn_state, conn_state->inputbuf)) {
    /*method not handled*/
    http_set_status(&conn_state->response, SERVICE_UNAVAILABLE_503);
    conn_state->state = STATE_OUTPUT;
  } else {
    /*read until the end of url*/
    PSOCK_READTO(&(conn_state->sin), ' ');

    /*-1 is needed since it also includes space char*/
    if (conn_state->inputbuf[PSOCK_DATALEN(&(conn_state->sin)) - 1] != ' ' ) {
      error = HTTP_URL_TOO_LONG;
    }

    conn_state->inputbuf[PSOCK_DATALEN(&(conn_state->sin)) - 1] = 0;

    PRINTF("Read URL:%s\n", conn_state->inputbuf);

    if (error == HTTP_NO_ERROR) {
      error = parse_url(conn_state, conn_state->inputbuf);
    }

    if (error != HTTP_NO_ERROR) {
      if (error == HTTP_URL_TOO_LONG) {
        http_set_status(&conn_state->response, REQUEST_URI_TOO_LONG_414);
      } else {
        http_set_status(&conn_state->response, BAD_REQUEST_400);
      }

      conn_state->state = STATE_OUTPUT;
    } else {
      /*read until the end of HTTP version - not used yet*/
      PSOCK_READTO(&(conn_state->sin), LINE_FEED_CHAR);

      PRINTF("After URL:%s\n", conn_state->inputbuf);

      /*FIXME : PSOCK_READTO takes just a single delimiter so I read till the end of line
      but now it may not fit in the buffer. If PSOCK_READTO would take two delimiters,
      we would have read until : and <CR> so it would not be blocked.*/

      /*Read the headers and store the necessary ones*/
      do {
        /*read the next line*/
        PSOCK_READTO(&(conn_state->sin), LINE_FEED_CHAR);
        conn_state->inputbuf[ PSOCK_DATALEN(&(conn_state->sin)) - 1] = 0;

        /*if headers finished then stop the infinite loop*/
        if (conn_state->inputbuf[0] == CARRIAGE_RETURN_CHAR || conn_state->inputbuf[0] == 0) {
          PRINTF("Finished Headers!\n\n");
          break;
        }

        parse_header(conn_state, conn_state->inputbuf);
      }
      while(1);

      content_len = get_header(conn_state->request.headers, HTTP_HEADER_NAME_CONTENT_LENGTH);
      if (content_len) {
        conn_state->request.payload_len = atoi(content_len);

        PRINTF("Post Data Size string: %s int: %d\n", content_len, conn_state->request.payload_len);
      }

      if (conn_state->request.payload_len) {
        static uint16_t read_bytes = 0;
        /*init the static variable again*/
        read_bytes = 0;

        conn_state->request.payload = allocate_buffer(conn_state->request.payload_len + 1);

        if (conn_state->request.payload) {
          do {
            PSOCK_READBUF(&(conn_state->sin));
            /*null terminate the buffer in case it is a string.*/
            conn_state->inputbuf[PSOCK_DATALEN(&(conn_state->sin))] = 0;

            memcpy(conn_state->request.payload + read_bytes, conn_state->inputbuf, PSOCK_DATALEN(&(conn_state->sin)));

            read_bytes += PSOCK_DATALEN(&(conn_state->sin));

          } while (read_bytes < conn_state->request.payload_len);

          conn_state->request.payload[read_bytes++] = 0;

          PRINTF("PostData => %s \n", conn_state->request.payload);
        } else {
          error = HTTP_MEMORY_ALLOC_ERR;
        }
      }

      if (error == HTTP_NO_ERROR) {
        if (service_cbk) {
          service_cbk(&conn_state->request, &conn_state->response);
        }
      } else {
        PRINTF("Error:%d\n",error);
        http_set_status(&conn_state->response, INTERNAL_SERVER_ERROR_500);
      }

      conn_state->state = STATE_OUTPUT;
    }
  }

  PSOCK_END(&(conn_state->sin));
}

static
PT_THREAD(send_data(connection_state_t* conn_state))
{
  uint16_t index;
  http_response_t* response;
  http_header_t* header;
  uint8_t* buffer;

  PSOCK_BEGIN(&(conn_state->sout));

  PRINTF("send_data -> \n");

  index = 0;
  response = &conn_state->response;
  header = response->headers;
  buffer = allocate_buffer(200);

  /*FIXME: what is the best solution here to send the data. Right now, if buffer is not allocated, no data is sent!*/
  if (buffer) {
    index += sprintf(buffer + index, "%s %d %s%s", httpv1_1, response->status_code, response->status_string, line_end);
    for (;header;header = header->next) {
      PRINTF("header %u \n", (uint16_t)header);
      index += sprintf(buffer + index, "%s%s%s%s", header->name, header_delimiter, header->value, line_end);
    }
    index += sprintf(buffer + index, "%s", line_end);

    memcpy(buffer + index, response->payload, response->payload_len);
    index += response->payload_len;

    PRINTF("Sending Data(size %d): %s \n", index, buffer);

    PSOCK_SEND(&(conn_state->sout), buffer, index);
  } else {
    PRINTF("BUFF ERROR: send_data!\n");
  }

  PSOCK_END(&(conn_state->sout));
}

static
PT_THREAD(handle_response(connection_state_t* conn_state))
{
  PT_BEGIN(&(conn_state->outputpt));

  PRINTF("handle_response ->\n");

  http_set_res_header(&conn_state->response, HTTP_HEADER_NAME_CONNECTION, close, 0);
  http_set_res_header(&conn_state->response, HTTP_HEADER_NAME_SERVER, contiki, 0);

  if (!(conn_state->response.status_string)) {
    conn_state->response.status_string =
        get_default_status_string(conn_state->response.status_code);
  }

  PT_WAIT_THREAD(&(conn_state->outputpt), send_data(conn_state));

  PRINTF("<-- handle_response\n\n\n");

  PSOCK_CLOSE(&(conn_state->sout));

  PT_END(&(conn_state->outputpt));
}

static void
handle_connection(connection_state_t* conn_state)
{
  if (conn_state->state == STATE_WAITING) {
    handle_request(conn_state);
  }

  if (conn_state->state == STATE_OUTPUT) {
    handle_response(conn_state);
  }
}

PROCESS(http_server, "Httpd Process");

PROCESS_THREAD(http_server, ev, data)
{
  connection_state_t *conn_state;

  PROCESS_BEGIN();

  /* if static routes are used rather than RPL */
#if !UIP_CONF_IPV6_RPL && !defined (CONTIKI_TARGET_MINIMAL_NET)
  set_global_address();
  configure_routing();
#endif /*!UIP_CONF_IPV6_RPL*/

  #ifdef CONTIKI_TARGET_SKY
    PRINTF("##RF CHANNEL : %d##\n",RF_CHANNEL);
  #endif //CONTIKI_TARGET_SKY

  tcp_listen(uip_htons(HTTP_PORT));

  /*
   * We loop for ever, accepting new connections.
   */
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

    conn_state = (connection_state_t *)data;

    if(uip_connected()) {
      PRINTF("##Connected##\n");

      if(init_buffer(HTTP_DATA_BUFF_SIZE)) {
        conn_state = (connection_state_t*)allocate_buffer(sizeof(connection_state_t));

        if (conn_state) {
          tcp_markconn(uip_conn, conn_state);

          /*initialize connection state*/
          init_connection(conn_state);

          /*-1 is needed to be able to null terminate the strings in the buffer, especially good for debugging (to have null terminated strings)*/
          PSOCK_INIT(&(conn_state->sin), (uint8_t*)conn_state->inputbuf, sizeof(conn_state->inputbuf) - 1);
          PSOCK_INIT(&(conn_state->sout), (uint8_t*)conn_state->inputbuf, sizeof(conn_state->inputbuf) - 1);
          PT_INIT(&(conn_state->outputpt));

          handle_connection(conn_state);
        } else {
          PRINTF("Memory Alloc Error. Aborting!\n");
          uip_abort();
        }
      }
    } else if (uip_aborted() || uip_closed() || uip_timedout()) {
      if (conn_state) {
        delete_buffer();

        /*Following 2 lines are needed since this part of code is somehow executed twice so it tries to free the same region twice.
        Potential bug in uip*/
        conn_state = NULL;
        tcp_markconn(uip_conn, conn_state);
      }
    } else {
      handle_connection(conn_state);
    }
  }

  PROCESS_END();
}
