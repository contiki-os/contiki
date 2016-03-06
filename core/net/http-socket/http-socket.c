/*
 * Copyright (c) 2013, Thingsquare, http://www.thingsquare.com/.
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "contiki-net.h"
#include "ip64-addr.h"
#include "http-socket.h"

#include <ctype.h>
#include <stdio.h>

#define MAX_PATHLEN 80
#define MAX_HOSTLEN 40
PROCESS(http_socket_process, "HTTP socket process");
LIST(socketlist);

static void removesocket(struct http_socket *s);
/*---------------------------------------------------------------------------*/
static void
call_callback(struct http_socket *s, http_socket_event_t e,
              const uint8_t *data, uint16_t datalen)
{
  if(s->callback != NULL) {
    s->callback(s, s->callbackptr, e,
                data, datalen);
  }
}
/*---------------------------------------------------------------------------*/
static void
parse_header_init(struct http_socket *s)
{
  PT_INIT(&s->headerpt);
}
/*---------------------------------------------------------------------------*/
static int
parse_header_byte(struct http_socket *s, char c)
{
  PT_BEGIN(&s->headerpt);

  memset(&s->header, -1, sizeof(s->header));

  /* Skip the HTTP response */
  while(c != ' ') {
    PT_YIELD(&s->headerpt);
  }

  /* Skip the space */
  PT_YIELD(&s->headerpt);
  /* Read three characters of HTTP status and convert to BCD */
  s->header.status_code = 0;
  for(s->header_chars = 0; s->header_chars < 3; s->header_chars++) {
    s->header.status_code = s->header.status_code << 4 | (c - '0');
    PT_YIELD(&s->headerpt);
  }

  if(s->header.status_code == 0x200 || s->header.status_code == 0x206) {
    /* Read headers until data */

    while(1) {
      /* Skip characters until end of line */
      do {
        while(c != '\r') {
          s->header_chars++;
          PT_YIELD(&s->headerpt);
        }
        s->header_chars++;
        PT_YIELD(&s->headerpt);
      } while(c != '\n');
      s->header_chars--;
      PT_YIELD(&s->headerpt);

      if(s->header_chars == 0) {
        /* This was an empty line, i.e. the end of headers */
        break;
      }

      /* Start of line */
      s->header_chars = 0;

      /* Read header field */
      while(c != ' ' && c != '\t' && c != ':' && c != '\r' &&
            s->header_chars < sizeof(s->header_field) - 1) {
        s->header_field[s->header_chars++] = c;
        PT_YIELD(&s->headerpt);
      }
      s->header_field[s->header_chars] = '\0';
      /* Skip linear white spaces */
      while(c == ' ' || c == '\t') {
        s->header_chars++;
        PT_YIELD(&s->headerpt);
      }
      if(c == ':') {
        /* Skip the colon */
        s->header_chars++;
        PT_YIELD(&s->headerpt);
        /* Skip linear white spaces */
        while(c == ' ' || c == '\t') {
          s->header_chars++;
          PT_YIELD(&s->headerpt);
        }
        if(!strcmp(s->header_field, "Content-Length")) {
          s->header.content_length = 0;
          while(isdigit((int)c)) {
            s->header.content_length = s->header.content_length * 10 + c - '0';
            s->header_chars++;
            PT_YIELD(&s->headerpt);
          }
        } else if(!strcmp(s->header_field, "Content-Range")) {
          /* Skip the bytes-unit token */
          while(c != ' ' && c != '\t') {
            s->header_chars++;
            PT_YIELD(&s->headerpt);
          }
          /* Skip linear white spaces */
          while(c == ' ' || c == '\t') {
            s->header_chars++;
            PT_YIELD(&s->headerpt);
          }
          s->header.content_range.first_byte_pos = 0;
          while(isdigit((int)c)) {
            s->header.content_range.first_byte_pos =
              s->header.content_range.first_byte_pos * 10 + c - '0';
            s->header_chars++;
            PT_YIELD(&s->headerpt);
          }
          /* Skip linear white spaces */
          while(c == ' ' || c == '\t') {
            s->header_chars++;
            PT_YIELD(&s->headerpt);
          }
          if(c == '-') {
            /* Skip the dash */
            s->header_chars++;
            PT_YIELD(&s->headerpt);
            /* Skip linear white spaces */
            while(c == ' ' || c == '\t') {
              s->header_chars++;
              PT_YIELD(&s->headerpt);
            }
            s->header.content_range.last_byte_pos = 0;
            while(isdigit((int)c)) {
              s->header.content_range.last_byte_pos =
                s->header.content_range.last_byte_pos * 10 + c - '0';
              s->header_chars++;
              PT_YIELD(&s->headerpt);
            }
            /* Skip linear white spaces */
            while(c == ' ' || c == '\t') {
              s->header_chars++;
              PT_YIELD(&s->headerpt);
            }
            if(c == '/') {
              /* Skip the slash */
              s->header_chars++;
              PT_YIELD(&s->headerpt);
              /* Skip linear white spaces */
              while(c == ' ' || c == '\t') {
                s->header_chars++;
                PT_YIELD(&s->headerpt);
              }
              if(c != '*') {
                s->header.content_range.instance_length = 0;
                while(isdigit((int)c)) {
                  s->header.content_range.instance_length =
                    s->header.content_range.instance_length * 10 + c - '0';
                  s->header_chars++;
                  PT_YIELD(&s->headerpt);
                }
              }
            }
          }
        }
      }
    }

    /* All headers read, now read data */
    call_callback(s, HTTP_SOCKET_HEADER, (void *)&s->header, sizeof(s->header));

    /* Should exit the pt here to indicate that all headers have been
       read */
    PT_EXIT(&s->headerpt);
  } else {
    if(s->header.status_code == 0x404) {
      printf("File not found\n");
    } else if(s->header.status_code == 0x301 || s->header.status_code == 0x302) {
      printf("File moved (not handled)\n");
    }

    call_callback(s, HTTP_SOCKET_ERR, (void *)&s->header, sizeof(s->header));
    tcp_socket_close(&s->s);
    removesocket(s);
    PT_EXIT(&s->headerpt);
  }


  PT_END(&s->headerpt);
}
/*---------------------------------------------------------------------------*/
static int
input_pt(struct http_socket *s,
         const uint8_t *inputptr, int inputdatalen)
{
  int i;
  PT_BEGIN(&s->pt);

  /* Parse the header */
  s->header_received = 0;
  do {
    for(i = 0; i < inputdatalen; i++) {
      if(!PT_SCHEDULE(parse_header_byte(s, inputptr[i]))) {
        s->header_received = 1;
        break;
      }
    }
    inputdatalen -= i;
    inputptr += i;

    if(s->header_received == 0) {
      /* If we have not yet received the full header, we wait for the
         next packet to arrive. */
      PT_YIELD(&s->pt);
    }
  } while(s->header_received == 0);

  s->bodylen = 0;
  do {
    /* Receive the data */
    call_callback(s, HTTP_SOCKET_DATA, inputptr, inputdatalen);

    /* Close the connection if the expected content length has been received */
    if(s->header.content_length >= 0 && s->bodylen < s->header.content_length) {
      s->bodylen += inputdatalen;
      if(s->bodylen >= s->header.content_length) {
        tcp_socket_close(&s->s);
      }
    }

    PT_YIELD(&s->pt);
  } while(inputdatalen > 0);

  PT_END(&s->pt);
}
/*---------------------------------------------------------------------------*/
static void
start_timeout_timer(struct http_socket *s)
{
  PROCESS_CONTEXT_BEGIN(&http_socket_process);
  etimer_set(&s->timeout_timer, HTTP_SOCKET_TIMEOUT);
  PROCESS_CONTEXT_END(&http_socket_process);
  s->timeout_timer_started = 1;
}
/*---------------------------------------------------------------------------*/
static int
input(struct tcp_socket *tcps, void *ptr,
      const uint8_t *inputptr, int inputdatalen)
{
  struct http_socket *s = ptr;

  input_pt(s, inputptr, inputdatalen);
  start_timeout_timer(s);

  return 0; /* all data consumed */
}
/*---------------------------------------------------------------------------*/
static int
parse_url(const char *url, char *host, uint16_t *portptr, char *path)
{
  const char *urlptr;
  int i;
  const char *file;
  uint16_t port;

  if(url == NULL) {
    printf("null url\n");
    return 0;
  }

  /* Don't even try to go further if the URL is empty. */
  if(strlen(url) == 0) {
    printf("empty url\n");
    return 0;
  }

  /* See if the URL starts with http:// and remove it. Otherwise, we
     assume it is an implicit http://. */
  if(strncmp(url, "http://", strlen("http://")) == 0) {
    urlptr = url + strlen("http://");
  } else {
    urlptr = url;
  }

  /* Find host part of the URL. */
  if(*urlptr == '[') {
    /* Handle IPv6 addresses - scan for matching ']' */
    urlptr++;
    for(i = 0; i < MAX_HOSTLEN; ++i) {
      if(*urlptr == ']') {
        if(host != NULL) {
          host[i] = 0;
        }
        break;
      }
      if(host != NULL) {
        host[i] = *urlptr;
      }
      ++urlptr;
    }
  } else {
    for(i = 0; i < MAX_HOSTLEN; ++i) {
      if(*urlptr == 0 ||
         *urlptr == '/' ||
         *urlptr == ' ' ||
         *urlptr == ':') {
        if(host != NULL) {
          host[i] = 0;
      }
        break;
      }
      if(host != NULL) {
        host[i] = *urlptr;
      }
      ++urlptr;
    }
  }

  /* Find the port. Default is 80. */
  port = 80;
  if(*urlptr == ':') {
    port = 0;
    do {
      ++urlptr;
      if(*urlptr >= '0' && *urlptr <= '9') {
	port = (10 * port) + (*urlptr - '0');
      }
    } while(*urlptr >= '0' &&
	    *urlptr <= '9');
  }
  if(portptr != NULL) {
    *portptr = port;
  }
  /* Find file part of the URL. */
  while(*urlptr != '/' && *urlptr != 0) {
    ++urlptr;
  }
  if(*urlptr == '/') {
    file = urlptr;
  } else {
    file = "/";
  }
  if(path != NULL) {
    strncpy(path, file, MAX_PATHLEN);
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
removesocket(struct http_socket *s)
{
  etimer_stop(&s->timeout_timer);
  s->timeout_timer_started = 0;
  list_remove(socketlist, s);
}
/*---------------------------------------------------------------------------*/
static void
event(struct tcp_socket *tcps, void *ptr,
      tcp_socket_event_t e)
{
  struct http_socket *s = ptr;
  char host[MAX_HOSTLEN];
  char path[MAX_PATHLEN];
  uint16_t port;
  char str[42];
  int len;

  if(e == TCP_SOCKET_CONNECTED) {
    printf("Connected\n");
    if(parse_url(s->url, host, &port, path)) {
      tcp_socket_send_str(tcps, s->postdata != NULL ? "POST " : "GET ");
      if(s->proxy_port != 0) {
        /* If we are configured to route through a proxy, we should
           provide the full URL as the path. */
        tcp_socket_send_str(tcps, s->url);
      } else {
        tcp_socket_send_str(tcps, path);
      }
      tcp_socket_send_str(tcps, " HTTP/1.1\r\n");
      tcp_socket_send_str(tcps, "Connection: close\r\n");
      tcp_socket_send_str(tcps, "Host: ");
      tcp_socket_send_str(tcps, host);
      tcp_socket_send_str(tcps, "\r\n");
      if(s->postdata != NULL) {
        if(s->content_type) {
          tcp_socket_send_str(tcps, "Content-Type: ");
          tcp_socket_send_str(tcps, s->content_type);
          tcp_socket_send_str(tcps, "\r\n");
        }
        tcp_socket_send_str(tcps, "Content-Length: ");
        sprintf(str, "%u", s->postdatalen);
        tcp_socket_send_str(tcps, str);
        tcp_socket_send_str(tcps, "\r\n");
      } else if(s->length || s->pos > 0) {
        tcp_socket_send_str(tcps, "Range: bytes=");
        if(s->length) {
          if(s->pos >= 0) {
            sprintf(str, "%llu-%llu", s->pos, s->pos + s->length - 1);
          } else {
            sprintf(str, "-%llu", s->length);
          }
        } else {
          sprintf(str, "%llu-", s->pos);
        }
        tcp_socket_send_str(tcps, str);
        tcp_socket_send_str(tcps, "\r\n");
      }
      tcp_socket_send_str(tcps, "\r\n");
      if(s->postdata != NULL && s->postdatalen) {
        len = tcp_socket_send(tcps, s->postdata, s->postdatalen);
        s->postdata += len;
        s->postdatalen -= len;
      }
    }
    parse_header_init(s);
  } else if(e == TCP_SOCKET_CLOSED) {
    call_callback(s, HTTP_SOCKET_CLOSED, NULL, 0);
    removesocket(s);
    printf("Closed\n");
  } else if(e == TCP_SOCKET_TIMEDOUT) {
    call_callback(s, HTTP_SOCKET_TIMEDOUT, NULL, 0);
    removesocket(s);
    printf("Timedout\n");
  } else if(e == TCP_SOCKET_ABORTED) {
    call_callback(s, HTTP_SOCKET_ABORTED, NULL, 0);
    removesocket(s);
    printf("Aborted\n");
  } else if(e == TCP_SOCKET_DATA_SENT) {
    if(s->postdata != NULL && s->postdatalen) {
      len = tcp_socket_send(tcps, s->postdata, s->postdatalen);
      s->postdata += len;
      s->postdatalen -= len;
    } else {
      start_timeout_timer(s);
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
start_request(struct http_socket *s)
{
  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;
  uip_ip6addr_t *addr;
  char host[MAX_HOSTLEN];
  char path[MAX_PATHLEN];
  uint16_t port;
  int ret;

  if(parse_url(s->url, host, &port, path)) {

    printf("url %s host %s port %d path %s\n",
           s->url, host, port, path);

    /* Check if we are to route the request through a proxy. */
    if(s->proxy_port != 0) {
      /* The proxy address should be an IPv6 address. */
      uip_ip6addr_copy(&ip6addr, &s->proxy_addr);
      port = s->proxy_port;
    } else if(uiplib_ip6addrconv(host, &ip6addr) == 0) {
      /* First check if the host is an IP address. */
      if(uiplib_ip4addrconv(host, &ip4addr) != 0) {
        ip64_addr_4to6(&ip4addr, &ip6addr);
      } else {
        /* Try to lookup the hostname. If it fails, we initiate a hostname
           lookup. */
        ret = resolv_lookup(host, &addr);
        if(ret == RESOLV_STATUS_UNCACHED ||
           ret == RESOLV_STATUS_EXPIRED) {
          resolv_query(host);
          puts("Resolving host...");
          return HTTP_SOCKET_OK;
        }
        if(addr != NULL) {
          s->did_tcp_connect = 1;
          tcp_socket_connect(&s->s, addr, port);
          return HTTP_SOCKET_OK;
        } else {
          return HTTP_SOCKET_ERR;
        }
      }
    }
    tcp_socket_connect(&s->s, &ip6addr, port);
    return HTTP_SOCKET_OK;
  } else {
    return HTTP_SOCKET_ERR;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(http_socket_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {

    PROCESS_WAIT_EVENT();

    if(ev == resolv_event_found && data != NULL) {
      struct http_socket *s;
      const char *name = data;
      /* Either found a hostname, or not. We need to go through the
	 list of http sockets and figure out to which connection this
	 reply corresponds, then either restart the HTTP get, or kill
	 it (if no hostname was found). */
      for(s = list_head(socketlist);
          s != NULL;
          s = list_item_next(s)) {
        char host[MAX_HOSTLEN];
        if(s->did_tcp_connect) {
          /* We already connected, ignored */
        } else if(parse_url(s->url, host, NULL, NULL) &&
            strcmp(name, host) == 0) {
          if(resolv_lookup(name, NULL) == RESOLV_STATUS_CACHED) {
            /* Hostname found, restart get. */
            start_request(s);
          } else {
            /* Hostname not found, kill connection. */
            call_callback(s, HTTP_SOCKET_HOSTNAME_NOT_FOUND, NULL, 0);
            removesocket(s);
          }
        }
      }
    } else if(ev == PROCESS_EVENT_TIMER) {
      struct http_socket *s;
      struct etimer *timeout_timer = data;
      /*
       * A socket time-out has occurred. We need to go through the list of HTTP
       * sockets and figure out to which socket this timer event corresponds,
       * then close this socket.
       */
      for(s = list_head(socketlist);
          s != NULL;
          s = list_item_next(s)) {
        if(timeout_timer == &s->timeout_timer && s->timeout_timer_started) {
          tcp_socket_close(&s->s);
          break;
        }
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  static uint8_t inited = 0;
  if(inited == 0) {
    process_start(&http_socket_process, NULL);
    list_init(socketlist);
    inited = 1;
  }
}
/*---------------------------------------------------------------------------*/
void
http_socket_init(struct http_socket *s)
{
  init();
  uip_create_unspecified(&s->proxy_addr);
  s->proxy_port = 0;
}
/*---------------------------------------------------------------------------*/
static void
initialize_socket(struct http_socket *s)
{
  s->pos = 0;
  s->length = 0;
  s->postdata = NULL;
  s->postdatalen = 0;
  s->timeout_timer_started = 0;
  PT_INIT(&s->pt);
  tcp_socket_register(&s->s, s,
                      s->inputbuf, sizeof(s->inputbuf),
                      s->outputbuf, sizeof(s->outputbuf),
                      input, event);
}
/*---------------------------------------------------------------------------*/
int
http_socket_get(struct http_socket *s,
                const char *url,
                int64_t pos,
                uint64_t length,
                http_socket_callback_t callback,
                void *callbackptr)
{
  initialize_socket(s);
  strncpy(s->url, url, sizeof(s->url));
  s->pos = pos;
  s->length = length;
  s->callback = callback;
  s->callbackptr = callbackptr;

  s->did_tcp_connect = 0;

  list_add(socketlist, s);

  return start_request(s);
}
/*---------------------------------------------------------------------------*/
int
http_socket_post(struct http_socket *s,
                 const char *url,
                 const void *postdata,
                 uint16_t postdatalen,
                 const char *content_type,
                 http_socket_callback_t callback,
                 void *callbackptr)
{
  initialize_socket(s);
  strncpy(s->url, url, sizeof(s->url));
  s->postdata = postdata;
  s->postdatalen = postdatalen;
  s->content_type = content_type;

  s->callback = callback;
  s->callbackptr = callbackptr;

  s->did_tcp_connect = 0;

  list_add(socketlist, s);

  return start_request(s);
}
/*---------------------------------------------------------------------------*/
int
http_socket_close(struct http_socket *socket)
{
  struct http_socket *s;
  for(s = list_head(socketlist);
      s != NULL;
      s = list_item_next(s)) {
    if(s == socket) {
      tcp_socket_close(&s->s);
      removesocket(s);
      return 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
http_socket_set_proxy(struct http_socket *s,
                      const uip_ipaddr_t *addr, uint16_t port)
{
  uip_ipaddr_copy(&s->proxy_addr, addr);
  s->proxy_port = port;
}
/*---------------------------------------------------------------------------*/
