#include "http-common.h"

/*needed for web services giving all path (http://172.16.79.0/services/light1)
 * instead relative (/services/light1) in HTTP request. Ex: Restlet lib.*/
const char* http_string = "http";

/*HTTP method strings*/
const char* http_get_string = "GET";
const char* http_head_string = "HEAD";
const char* http_post_string = "POST";
const char* http_put_string = "PUT";
const char* http_delete_string = "DELETE";

const char* httpv1_1 = "HTTP/1.1";
const char* line_end = "\r\n";
const char* contiki = "Contiki";
const char* close = "close";

/*header names*/
const char* HTTP_HEADER_NAME_CONTENT_TYPE = "Content-Type";
const char* HTTP_HEADER_NAME_CONTENT_LENGTH = "Content-Length";
const char* HTTP_HEADER_NAME_LOCATION = "Location";
const char* HTTP_HEADER_NAME_CONNECTION = "Connection";
const char* HTTP_HEADER_NAME_SERVER = "Server";
const char* HTTP_HEADER_NAME_HOST = "Host";
const char* HTTP_HEADER_NAME_IF_NONE_MATCH = "If-None-Match";
const char* HTTP_HEADER_NAME_ETAG = "ETag";

const char* header_delimiter = ": ";
