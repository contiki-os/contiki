/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "contiki-net.h"
#include "lib/petsciiconv.h"

#include "websocket.h"

PROCESS(websocket_process, "Websockets process");

#define MAX_HOSTLEN 64
#define MAX_PATHLEN 100

LIST(websocketlist);

#define WEBSOCKET_FIN_BIT       0x80

#define WEBSOCKET_OPCODE_MASK   0x0f
#define WEBSOCKET_OPCODE_CONT   0x00
#define WEBSOCKET_OPCODE_TEXT   0x01
#define WEBSOCKET_OPCODE_BIN    0x02
#define WEBSOCKET_OPCODE_CLOSE  0x08
#define WEBSOCKET_OPCODE_PING   0x09
#define WEBSOCKET_OPCODE_PONG   0x0a

#define WEBSOCKET_MASK_BIT      0x80
#define WEBSOCKET_LEN_MASK      0x7f
struct websocket_frame_hdr {
  uint8_t opcode;
  uint8_t len;
  uint8_t extlen[4];
};

struct websocket_frame_mask {
  uint8_t mask[4];
};

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

/*---------------------------------------------------------------------------*/
static int
parse_url(const char *url, char *host, uint16_t *portptr, char *path)
{
  const char *urlptr;
  int i;
  const char *file;
  uint16_t port;

  if(url == NULL) {
    return 0;
  }

  /* Don't even try to go further if the URL is empty. */
  if(strlen(url) == 0) {
    return 0;
  }

  /* See if the URL starts with http:// or ws:// and remove it. */
  if(strncmp(url, "http://", strlen("http://")) == 0) {
    urlptr = url + strlen("http://");
  } else if(strncmp(url, "ws://", strlen("ws://")) == 0) {
    urlptr = url + strlen("ws://");
  } else {
    urlptr = url;
  }

  /* Find host part of the URL. */
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

  /* Find the port. Default is 0, which lets the underlying transport
     select its default port. */
  port = 0;
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
static int
start_get(struct websocket *s)
{
  if(websocket_http_client_get(&(s->s)) == 0) {
    PRINTF("Out of memory error\n");
    s->state = WEBSOCKET_STATE_CLOSED;
    return WEBSOCKET_ERR;
  } else {
    PRINTF("Connecting...\n");
    s->state = WEBSOCKET_STATE_HTTP_REQUEST_SENT;
    return WEBSOCKET_OK;
  }
  return WEBSOCKET_ERR;
}
/*---------------------------------------------------------------------------*/
void
call(struct websocket *s, websocket_result_t r,
     const uint8_t *data, uint16_t datalen)
{
  if(s != NULL && s->callback != NULL) {
    s->callback(s, r, data, datalen);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(websocket_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {

    PROCESS_WAIT_EVENT();

    if(ev == resolv_event_found && data != NULL) {
      int ret;
      struct websocket *s;
      const char *name = data;
      /* Either found a hostname, or not. We need to go through the
	 list of websocketsand figure out to which connection this
	 reply corresponds, then either restart the HTTP get, or kill
	 it (if no hostname was found). */
      for(s = list_head(websocketlist);
	  s != NULL;
	  s = list_item_next(s)) {
        if(strcmp(name, websocket_http_client_hostname(&s->s)) == 0) {
          ret = resolv_lookup(name, NULL);
          if(ret == RESOLV_STATUS_CACHED) {
	    /* Hostname found, restart get. */
            if(s->state == WEBSOCKET_STATE_DNS_REQUEST_SENT) {
              PRINTF("Restarting get\n");
              start_get(s);
            }
	  } else {
            if(s->state == WEBSOCKET_STATE_DNS_REQUEST_SENT) {
              /* Hostname not found, kill connection. */
              /*	    PRINTF("XXX killing connection\n");*/
              call(s, WEBSOCKET_HOSTNAME_NOT_FOUND, NULL, 0);
            }
	  }
	}
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Callback function. Called from the webclient when the HTTP
 * connection was abruptly aborted.
 */
void
websocket_http_client_aborted(struct websocket_http_client_state *client_state)
{
  if(client_state != NULL) {
    struct websocket *s = (struct websocket *)
      ((char *)client_state - offsetof(struct websocket, s));
    PRINTF("Websocket reset\n");
    s->state = WEBSOCKET_STATE_CLOSED;
    call(s, WEBSOCKET_RESET, NULL, 0);
  }
}
/*---------------------------------------------------------------------------*/
/* Callback function. Called from the webclient when the HTTP
 * connection timed out.
 */
void
websocket_http_client_timedout(struct websocket_http_client_state *client_state)
{
  if(client_state != NULL) {
    struct websocket *s = (struct websocket *)
      ((char *)client_state - offsetof(struct websocket, s));
    PRINTF("Websocket timed out\n");
    s->state = WEBSOCKET_STATE_CLOSED;
    call(s, WEBSOCKET_TIMEDOUT, NULL, 0);
  }
}
/*---------------------------------------------------------------------------*/
/* Callback function. Called from the webclient when the HTTP
 * connection was closed after a request from the "websocket_http_client_close()"
 * function. .
 */
void
websocket_http_client_closed(struct websocket_http_client_state *client_state)
{
  if(client_state != NULL) {
    struct websocket *s = (struct websocket *)
      ((char *)client_state - offsetof(struct websocket, s));
    PRINTF("Websocket closed.\n");
    s->state = WEBSOCKET_STATE_CLOSED;
    call(s, WEBSOCKET_CLOSED, NULL, 0);
  }
}
/*---------------------------------------------------------------------------*/
/* Callback function. Called from the webclient when the HTTP
 * connection is connected.
 */
void
websocket_http_client_connected(struct websocket_http_client_state *client_state)
{
  struct websocket *s = (struct websocket *)
    ((char *)client_state - offsetof(struct websocket, s));

  PRINTF("Websocket connected\n");
  s->state = WEBSOCKET_STATE_WAITING_FOR_HEADER;
  call(s, WEBSOCKET_CONNECTED, NULL, 0);
}
/*---------------------------------------------------------------------------*/
/* The websocket header may potentially be split into multiple TCP
   segments. This function eats one byte each, puts it into
   s->headercache, and checks whether or not the full header has been
   received. */
static int
receive_header_byte(struct websocket *s, uint8_t byte)
{
  int len;
  int expected_len;
  struct websocket_frame_hdr *hdr;

  /* Take the next byte of data and place it in the header cache. */
  if(s->state == WEBSOCKET_STATE_RECEIVING_HEADER) {
    s->headercache[s->headercacheptr] = byte;
    s->headercacheptr++;
    if(s->headercacheptr >= sizeof(s->headercache)) {
      /* Something bad happened: we ad read 10 bytes and had not yet
         found a reasonable header, so we close the socket. */
      websocket_close(s);
    }
  }

  len = s->headercacheptr;
  hdr = (struct websocket_frame_hdr *)s->headercache;

  /* Check the header that we have received to see if it is long
     enough. */

  /* We start with expecting a length of at least two bytes (opcode +
     1 length byte). */
  expected_len = 2;

  if(len >= expected_len) {

    /* We check how many more bytes we should expect to see. The
       length byte determines how many length bytes are included in
       the header. */
    if((hdr->len & WEBSOCKET_LEN_MASK) == 126) {
      expected_len += 2;
    } else if((hdr->len & WEBSOCKET_LEN_MASK) == 127) {
      expected_len += 4;
    }

    /* If the option has the mask bit set, we should expect to see 4
       mask bytes at the end of the header. */
    if((hdr->len & WEBSOCKET_MASK_BIT ) != 0) {
      expected_len += 4;
    }

    /* Now we know how long our header if expected to be. If it is
       this long, we are done and we set the state to reflect this. */
    if(len == expected_len) {
      s->state = WEBSOCKET_STATE_HEADER_RECEIVED;
      return 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/* Callback function. Called from the webclient module when HTTP data
 * has arrived.
 */
void
websocket_http_client_datahandler(struct websocket_http_client_state *client_state,
				  const uint8_t *data, uint16_t datalen)
{
  struct websocket *s = (struct websocket *)
    ((char *)client_state - offsetof(struct websocket, s));
  struct websocket_frame_hdr *hdr;
  struct websocket_frame_mask *maskptr;

  if(data == NULL) {
    call(s, WEBSOCKET_CLOSED, NULL, 0);
  } else {
    /* This function is a state machine that does different things
       depending on the state. If we are waiting for header (the
       default state), we change to the RECEIVING_HEADER state when we
       get the first byte. If we are receiving header, we put all
       bytes we have into a header buffer until the full header has
       been received. If we have received the header, we parse it. If
       we have received and parsed the header, we are ready to receive
       data. Finally, if there is data left in the incoming packet, we
       repeat the process. */

    if(s->state == WEBSOCKET_STATE_WAITING_FOR_HEADER) {
      s->state = WEBSOCKET_STATE_RECEIVING_HEADER;
      s->headercacheptr = 0;
    }

    if(s->state == WEBSOCKET_STATE_RECEIVING_HEADER) {
      while(datalen > 0 && s->state == WEBSOCKET_STATE_RECEIVING_HEADER) {
        receive_header_byte(s, data[0]);
        data++;
        datalen--;
      }
    }

    if(s->state == WEBSOCKET_STATE_HEADER_RECEIVED) {
      /* If this is the start of an incoming websocket data frame, we
         decode the header and check if we should act on in. If not, we
         pipe the data to the application through a callback handler. If
         data arrives in multiple packets, it is up to the application to
         put it back together again. */

      /* The websocket header is at the start of the incoming data. */
      hdr = (struct websocket_frame_hdr *)s->headercache;

      /* The s->left field holds the length of the application data
       * chunk that we are about to receive. */
      s->len = s->left = 0;

      /* The s->mask field holds the bitmask of the data chunk, if
       * any. */
      memset(s->mask, 0, sizeof(s->mask));

      /* We first read out the length of the application data
         chunk. The length may be encoded over multiple bytes. If the
         length is >= 126 bytes, it is encoded as two or more
         bytes. The first length field determines if it is in 2 or 4
         bytes. We also keep track of where the bitmask is held - its
         place also differs depending on how the length is encoded.  */
      maskptr = (struct websocket_frame_mask *)hdr->extlen;
      if((hdr->len & WEBSOCKET_LEN_MASK) < 126) {
        s->len = s->left = hdr->len & WEBSOCKET_LEN_MASK;
      } else if(hdr->len == 126) {
        s->len = s->left = (hdr->extlen[0] << 8) + hdr->extlen[1];
        maskptr = (struct websocket_frame_mask *)&hdr->extlen[2];
      } else if(hdr->len == 127) {
        s->len = s->left = ((uint32_t)hdr->extlen[0] << 24) +
          ((uint32_t)hdr->extlen[1] << 16) +
          ((uint32_t)hdr->extlen[2] << 8) +
          hdr->extlen[3];
        maskptr = (struct websocket_frame_mask *)&hdr->extlen[4];
      }

      /* Set user_data to point to the first byte of application data.
         See if the application data chunk is masked or not. If it is,
         we copy the bitmask into the s->mask field. */
      if((hdr->len & WEBSOCKET_MASK_BIT) == 0) {
        /*        PRINTF("No mask\n");*/
      } else {
        memcpy(s->mask, &maskptr->mask, sizeof(s->mask));
        /*        PRINTF("There was a mask, %02x %02x %02x %02x\n",
                  s->mask[0], s->mask[1], s->mask[2], s->mask[3]);*/
      }

      /* Remember the opcode of the application chunk, put it in the
       * s->opcode field. */
      s->opcode = hdr->opcode & WEBSOCKET_OPCODE_MASK;

      if(s->opcode == WEBSOCKET_OPCODE_PING) {
        /* If the opcode is ping, we change the opcode to a pong, and
         * send the data back. */
        hdr->opcode = (hdr->opcode & (~WEBSOCKET_OPCODE_MASK)) |
          WEBSOCKET_OPCODE_PONG;
        websocket_http_client_send(&s->s, (const uint8_t*)hdr, 2);
        if(s->left > 0) {
          websocket_http_client_send(&s->s, (const uint8_t*)data, s->left);
        }
        PRINTF("Got ping\n");
        call(s, WEBSOCKET_PINGED, NULL, 0);
        s->state = WEBSOCKET_STATE_WAITING_FOR_HEADER;
      } else if(s->opcode == WEBSOCKET_OPCODE_PONG) {
          /* If the opcode is pong, we call the application to let it
           know we got a pong. */
        PRINTF("Got pong\n");
        call(s, WEBSOCKET_PONG_RECEIVED, NULL, 0);
        s->state = WEBSOCKET_STATE_WAITING_FOR_HEADER;
      } else if(s->opcode == WEBSOCKET_OPCODE_CLOSE) {
        /* If the opcode is a close, we send a close frame back. */
        hdr->opcode = (hdr->opcode & (~WEBSOCKET_OPCODE_MASK)) |
          WEBSOCKET_OPCODE_CLOSE;
        websocket_http_client_send(&s->s, (const uint8_t*)hdr, 2);
        if(s->left > 0) {
          websocket_http_client_send(&s->s, (const uint8_t*)data, s->left);
        }
        PRINTF("websocket: got close, sending close\n");
        s->state = WEBSOCKET_STATE_WAITING_FOR_HEADER;
        websocket_http_client_close(&s->s);
      } else if(s->opcode == WEBSOCKET_OPCODE_BIN ||
                s->opcode == WEBSOCKET_OPCODE_TEXT) {

        /* If the opcode is bin or text, and there is application
         * layer data in the packet, we call the application to
         * process it. */
        if(s->left > 0) {
          s->state = WEBSOCKET_STATE_RECEIVING_DATA;
          if(datalen > 0) {
            int len;

            len = MIN(s->left, datalen);
            /* XXX todo: mask if needed. */
            call(s, WEBSOCKET_DATA, data, len);
            data += len;
            s->left -= len;
            datalen -= len;
          }
        }
      }

      if(s->left == 0) {
        call(s, WEBSOCKET_DATA_RECEIVED, NULL, s->len);
        s->state = WEBSOCKET_STATE_WAITING_FOR_HEADER;

        /* Need to keep parsing the incoming data to check for more
           frames, if the incoming datalen is > than s->left. */
        if(datalen > 0) {
          PRINTF("XXX 1 again\n");
          websocket_http_client_datahandler(client_state,
                                            data, datalen);
        }
      }
    } else if(s->state == WEBSOCKET_STATE_RECEIVING_DATA) {
      /* XXX todo: mask if needed. */
      /*      PRINTF("Calling with s->left %d datalen %d\n",
              s->left, datalen);*/
      if(datalen > 0) {
        if(datalen < s->left) {
          call(s, WEBSOCKET_DATA, data, datalen);
          s->left -= datalen;
          data += datalen;
          datalen = 0;
        } else {
          call(s, WEBSOCKET_DATA, data, s->left);
          data += s->left;
          datalen -= s->left;
          s->left = 0;
        }
      }
      if(s->left == 0) {
        call(s, WEBSOCKET_DATA_RECEIVED, NULL, s->len);
        s->state = WEBSOCKET_STATE_WAITING_FOR_HEADER;
        /* Need to keep parsing the incoming data to check for more
           frames, if the incoming datalen is > than len. */
        if(datalen > 0) {
          PRINTF("XXX 2 again (datalen %d s->left %d)\n", datalen, (int)s->left);
          websocket_http_client_datahandler(client_state,
                                            data, datalen);

        }
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  static uint8_t inited = 0;
  if(!inited) {
    process_start(&websocket_process, NULL);
    list_init(websocketlist);
    inited = 1;
  }
}
/*---------------------------------------------------------------------------*/
void
websocket_init(struct websocket *s)
{
  init();
  websocket_http_client_init(&s->s);
}
/*---------------------------------------------------------------------------*/
void
websocket_set_proxy(struct websocket *s,
                    const uip_ipaddr_t *addr, uint16_t port)
{
  websocket_http_client_set_proxy(&s->s, addr, port);
}
/*---------------------------------------------------------------------------*/
websocket_result_t
websocket_open(struct websocket *s, const char *url,
               const char *subprotocol, const char *hdr,
	       websocket_callback c)
{
  int ret;
  char host[MAX_HOSTLEN];
  char path[MAX_PATHLEN];
  uint16_t port;
  uip_ipaddr_t addr;

  init();

  if(s == NULL) {
    return WEBSOCKET_ERR;
  }

  if(s->state != WEBSOCKET_STATE_CLOSED) {
    PRINTF("websocket_open: closing websocket before opening it again.\n");
    websocket_close(s);
  }
  s->callback = c;

  if(parse_url(url, host, &port, path)) {
    list_add(websocketlist, s);
    websocket_http_client_register(&s->s, host, port, path, subprotocol, hdr);

    /* First check if the host is an IP address. */
    if(uiplib_ip4addrconv(host, (uip_ip4addr_t *)&addr) == 0 &&
       uiplib_ip6addrconv(host, (uip_ip6addr_t *)&addr) == 0) {
      /* Try to lookup the hostname. If it fails, we initiate a hostname
	 lookup and print out an informative message on the
	 statusbar. */
      ret = resolv_lookup(host, NULL);
      if(ret != RESOLV_STATUS_CACHED) {
	resolv_query(host);
	s->state = WEBSOCKET_STATE_DNS_REQUEST_SENT;
	PRINTF("Resolving host...\n");
	return WEBSOCKET_OK;
      }
    }

    PROCESS_CONTEXT_BEGIN(&websocket_process);
    ret = start_get(s);
    PROCESS_CONTEXT_END();
    return ret;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
void
websocket_close(struct websocket *s)
{
  websocket_http_client_close(&s->s);
  s->state = WEBSOCKET_STATE_CLOSED;
}
/*---------------------------------------------------------------------------*/
static int
send_data(struct websocket *s, const void *data,
          uint16_t datalen, uint8_t data_type_opcode)
{
  uint8_t buf[WEBSOCKET_MAX_MSGLEN + 4 + 4]; /* The extra + 4 + 4 here
                                                comes from the size of
                                                the websocket framing
                                                header. */
  struct websocket_frame_hdr *hdr;
  struct websocket_frame_mask *mask;

  PRINTF("websocket send data len %d %.*s\n", datalen, datalen, (char *)data);
  if(s->state == WEBSOCKET_STATE_CLOSED ||
     s->state == WEBSOCKET_STATE_DNS_REQUEST_SENT ||
     s->state == WEBSOCKET_STATE_HTTP_REQUEST_SENT) {
    /* Trying to send data on a non-connected websocket. */
    PRINTF("websocket send fail: not connected\n");
    return -1;
  }

  /* We need to have 4 + 4 additional bytes for the websocket framing
     header. */
  if(4 + 4 + datalen > websocket_http_client_sendbuflen(&s->s)) {
    PRINTF("websocket: too few bytes left (%d left, %d needed)\n",
           websocket_http_client_sendbuflen(&s->s),
           4 + 4 + datalen);
    return -1;
  }

  if(datalen > sizeof(buf) - 4 - 4) {
    PRINTF("websocket: trying to send too large data chunk %d > %d\n",
           datalen, sizeof(buf) - 4 - 4);
    return -1;
  }

  hdr = (struct websocket_frame_hdr *)&buf[0];
  hdr->opcode = WEBSOCKET_FIN_BIT | data_type_opcode;

  /* If the datalen is larger than 125 bytes, we need to send the data
     length as two bytes. If the data length would be larger than 64k,
     we should send the length as 4 bytes, but since we specify the
     datalen as an unsigned 16-bit int, we do not handle the 64k case
     here. */
  if(datalen > 125) {
    /* Data from client must always have the mask bit set, and a data
       mask sent right after the header. */
    hdr->len = 126 | WEBSOCKET_MASK_BIT;
    hdr->extlen[0] = datalen >> 8;
    hdr->extlen[1] = datalen & 0xff;

    mask = (struct websocket_frame_mask *)&buf[4];
    mask->mask[0] =
      mask->mask[1] =
      mask->mask[2] =
      mask->mask[3] = 0;
    memcpy(&buf[8], data, datalen);
    return websocket_http_client_send(&s->s, buf, 8 + datalen);
  } else {
    /* Data from client must always have the mask bit set, and a data
       mask sent right after the header. */
    hdr->len = datalen | WEBSOCKET_MASK_BIT;

    mask = (struct websocket_frame_mask *)&buf[2];
    mask->mask[0] =
      mask->mask[1] =
      mask->mask[2] =
      mask->mask[3] = 0;
    memcpy(&buf[6], data, datalen);
    return websocket_http_client_send(&s->s, buf, 6 + datalen);
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
int
websocket_send_str(struct websocket *s, const char *str)
{
  return send_data(s, str, strlen(str), WEBSOCKET_OPCODE_TEXT);
}
/*---------------------------------------------------------------------------*/
int
websocket_send(struct websocket *s, const uint8_t *data,
	       uint16_t datalen)
{
  return send_data(s, data, datalen, WEBSOCKET_OPCODE_BIN);
}
/*---------------------------------------------------------------------------*/
int
websocket_ping(struct websocket *s)
{
  uint8_t buf[sizeof(struct websocket_frame_hdr) +
              sizeof(struct websocket_frame_mask)];
  struct websocket_frame_hdr *hdr;
  struct websocket_frame_mask *mask;

  /* We need 2 + 4 additional bytes for the websocket framing
     header. */
  if(2 + 4 > websocket_http_client_sendbuflen(&s->s)) {
    return -1;
  }

  hdr = (struct websocket_frame_hdr *)&buf[0];
  mask = (struct websocket_frame_mask *)&buf[2];
  hdr->opcode = WEBSOCKET_FIN_BIT | WEBSOCKET_OPCODE_PING;

  /* Data from client must always have the mask bit set, and a data
     mask sent right after the header. */
  hdr->len = 0 | WEBSOCKET_MASK_BIT;

  /* XXX: We just set a dummy mask of 0 for now and hope that this
     works. */
  mask->mask[0] =
    mask->mask[1] =
    mask->mask[2] =
    mask->mask[3] = 0;
  websocket_http_client_send(&s->s, buf, 2 + 4);
  return 1;
}
/*---------------------------------------------------------------------------*/
int
websocket_queuelen(struct websocket *s)
{
  return websocket_http_client_queuelen(&s->s);
}
/*---------------------------------------------------------------------------*/
