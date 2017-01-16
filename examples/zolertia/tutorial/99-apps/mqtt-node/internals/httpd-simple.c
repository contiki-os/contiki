/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2016, Antonio Lignan - antonio.lignan@gmail.com
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "httpd-simple.h"
#include "net/ipv6/uip-ds6-route.h"
#include "lib/list.h"

#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <strformat.h>
#include <ctype.h>

#include "mqtt-client.h"
/*---------------------------------------------------------------------------*/
#define SEND_STRING(s, str) PSOCK_SEND(s, (uint8_t *)str, strlen(str))
/*---------------------------------------------------------------------------*/
#define CONNS                   2
#define CONTENT_LENGTH_MAX      256
#define STATE_WAITING           0
#define STATE_OUTPUT            1
#define IPADDR_BUF_LEN          64
/*---------------------------------------------------------------------------*/
#define RETURN_CODE_OK          0
#define RETURN_CODE_NF          1 /* Not Found */
#define RETURN_CODE_SU          2 /* Service Unavailable */
#define RETURN_CODE_BR          3 /* Bad Request */
#define RETURN_CODE_LR          4 /* Length Required */
#define RETURN_CODE_TL          5 /* Content Length too Large */
/*---------------------------------------------------------------------------*/
/* POST request machine states */
#define PARSE_POST_STATE_INIT            0
#define PARSE_POST_STATE_MORE            1
#define PARSE_POST_STATE_READING_KEY     2
#define PARSE_POST_STATE_READING_VAL     3
#define PARSE_POST_STATE_ERROR  0xFFFFFFFF
/*---------------------------------------------------------------------------*/
#define PARSE_POST_BUF_SIZES            64

/* Last byte always used to null terminate */
#define PARSE_POST_MAX_POS        (PARSE_POST_BUF_SIZES - 2)

static char key[PARSE_POST_BUF_SIZES];
static char val_escaped[PARSE_POST_BUF_SIZES];
static char val[PARSE_POST_BUF_SIZES];
static int key_len;
static int val_len;
static int state;
/*---------------------------------------------------------------------------*/
/* Stringified min/max intervals */
#define STRINGIFY(x) XSTR(x)
#define XSTR(x)      #x
/*---------------------------------------------------------------------------*/
/*
 * We can only handle a single POST request at a time. Since a second POST
 * request cannot interrupt us while obtaining a lock, we don't really need
 * this lock to be atomic.
 *
 * An HTTP connection will first request a lock before it starts processing
 * a POST request. We maintain a global lock which is either NULL or points
 * to the http conn which currently has the lock
 */
static struct httpd_state *lock;
/*---------------------------------------------------------------------------*/
PROCESS(httpd_simple_process, "MQTT client Web Server");
PROCESS_NAME(mqtt_client_process);
/*---------------------------------------------------------------------------*/
#define ISO_nl        0x0A
#define ISO_space     0x20
#define ISO_slash     0x2F
#define ISO_amp       0x26
#define ISO_column    0x3A
#define ISO_equal     0x3D
/*---------------------------------------------------------------------------*/
#define HTTP_200_OK "HTTP/1.0 200 OK\r\n"
#define HTTP_302_FO "HTTP/1.0 302 Found\r\n"
#define HTTP_400_BR "HTTP/1.0 400 Bad Request\r\n"
#define HTTP_404_NF "HTTP/1.0 404 Not Found\r\n"
#define HTTP_411_LR "HTTP/1.0 411 Length Required\r\n"
#define HTTP_413_TL "HTTP/1.0 413 Request Entity Too Large\r\n"
#define HTTP_503_SU "HTTP/1.0 503 Service Unavailable\r\n"
#define CONN_CLOSE  "Connection: close\r\n"
/*---------------------------------------------------------------------------*/
#define SECTION_TAG   "div"
#define SECTION_OPEN  "<" SECTION_TAG ">"
#define SECTION_CLOSE "</" SECTION_TAG ">"

#define CONTENT_OPEN  "<pre>"
#define CONTENT_CLOSE "</pre>"
/*---------------------------------------------------------------------------*/
#define REQUEST_TYPE_GET  1
#define REQUEST_TYPE_POST 2
/*---------------------------------------------------------------------------*/
static const char *NOT_FOUND = "<html><body bgcolor=\"white\">"
                               "<center>"
                               "<h1>404 - file not found</h1>"
                               "</center>"
                               "</body>"
                               "</html>";
/*---------------------------------------------------------------------------*/
/* Page template */
static const char http_doctype[] = "<!DOCTYPE html>";
static const char http_header_200[] = HTTP_200_OK;
static const char http_header_302[] = HTTP_302_FO;
static const char http_header_400[] = HTTP_400_BR;
static const char http_header_404[] = HTTP_404_NF;
static const char http_header_411[] = HTTP_411_LR;
static const char http_header_413[] = HTTP_413_TL;
static const char http_header_503[] = HTTP_503_SU;
static const char http_get[] = "GET ";
static const char http_post[] = "POST ";
static const char http_index_html[] = "/index.html";
static const char http_html_start[] = "<html><head>";
static const char *http_header_srv_str[] = {
  "Server: Contiki, ",
  BOARD_STRING "\r\n",
  NULL
};

static const char *http_header_con_close[] = {
  CONN_CLOSE,
  NULL
};

static const char *http_config_css[] = {
  "<style>",
  ".left{float:left;text-align:right;}",
  ".right{margin-left:150px;}",
  "input[type=\"radio\"]",
  "{display: inline-block;}",
  "</style>",
  NULL
};
static const char http_head_charset[] = "<meta charset=\"UTF-8\">";
static const char http_title_start[] = "<title>";
static const char http_title_end[] = "</title>";
static const char http_head_end[] = "</head>";
static const char http_body_start[] = "<body>";
static const char http_bottom[] = "</body></html>";
/*---------------------------------------------------------------------------*/
static const char http_content_type_html[] = "text/html";
static const char http_content_type_plain[] = "text/plain";
/*---------------------------------------------------------------------------*/
/* For the config page */
static const char config_div_left[] = "<div class=\"left\">";
static const char config_div_right[] = "<div class=\"right\">";
static const char config_div_close[] = "</div>";
/*---------------------------------------------------------------------------*/
typedef struct page {
  struct page *next;
  char *filename;
  char *title;
  char (*script)(struct httpd_state *s);
} page_t;

static char generate_ws_config(struct httpd_state *s);

static page_t http_ws_cfg_page = {
  NULL,
  "config.html",
  "MQTT web provisioning",
  generate_ws_config,
};
/*---------------------------------------------------------------------------*/
static const httpd_simple_post_handler_t *handler;
/*---------------------------------------------------------------------------*/
static uint8_t config_ok;
process_event_t httpd_simple_event_new_config;
/*---------------------------------------------------------------------------*/
struct httpd_state;
typedef char (*httpd_simple_script_t)(struct httpd_state *s);

struct httpd_state {
  char buf[HTTPD_SIMPLE_MAIN_BUF_SIZE];
  char tmp_buf[TMP_BUF_SIZE];
  struct timer timer;
  struct psock sin, sout;
  int blen;
  const char **ptr;
  const page_t *page;
  uip_ds6_route_t *r;
  uip_ds6_nbr_t *nbr;
  httpd_simple_script_t script;
  int content_length;
  int tmp_buf_len;
  int tmp_buf_copied;
  char filename[HTTPD_PATHLEN];
  char inputbuf[HTTPD_INBUF_LEN];
  struct pt outputpt;
  struct pt generate_pt;
  struct pt top_matter_pt;
  char state;
  char request_type;
  char return_code;
};
/*---------------------------------------------------------------------------*/
LIST(post_handlers);
LIST(pages_list);
MEMB(conns, struct httpd_state, CONNS);
/*---------------------------------------------------------------------------*/
#define HEX_TO_INT(x)  (isdigit(x) ? x - '0' : x - 'W')
static size_t
url_unescape(const char *src, size_t srclen, char *dst, size_t dstlen)
{
  size_t i, j;
  int a, b;

  for(i = j = 0; i < srclen && j < dstlen - 1; i++, j++) {
    if(src[i] == '%' && isxdigit(*(unsigned char *)(src + i + 1))
       && isxdigit(*(unsigned char *)(src + i + 2))) {
      a = tolower(*(unsigned char *)(src + i + 1));
      b = tolower(*(unsigned char *)(src + i + 2));
      dst[j] = ((HEX_TO_INT(a) << 4) | HEX_TO_INT(b)) & 0xff;
      i += 2;
    } else if(src[i] == '+') {
      dst[j] = ' ';
    } else {
      dst[j] = src[i];
    }
  }

  dst[j] = '\0';

  return i == srclen;
}
/*---------------------------------------------------------------------------*/
void
httpd_simple_register_post_handler(httpd_simple_post_handler_t *h)
{
  list_add(post_handlers, h);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(enqueue_chunk(struct httpd_state *s, uint8_t immediate,
                        const char *format, ...))
{
  va_list ap;

  PSOCK_BEGIN(&s->sout);

  va_start(ap, format);

  s->tmp_buf_len = vsnprintf(s->tmp_buf, TMP_BUF_SIZE, format, ap);

  va_end(ap);

  if(s->blen + s->tmp_buf_len < HTTPD_SIMPLE_MAIN_BUF_SIZE) {
    /* Enough space for the entire chunk. Copy over */
    memcpy(&s->buf[s->blen], s->tmp_buf, s->tmp_buf_len);
    s->blen += s->tmp_buf_len;
  } else {
    memcpy(&s->buf[s->blen], s->tmp_buf, HTTPD_SIMPLE_MAIN_BUF_SIZE - s->blen);
    s->tmp_buf_copied = HTTPD_SIMPLE_MAIN_BUF_SIZE - s->blen;
    s->blen = HTTPD_SIMPLE_MAIN_BUF_SIZE;
    PSOCK_SEND(&s->sout, (uint8_t *)s->buf, s->blen);
    s->blen = 0;
    if(s->tmp_buf_copied < s->tmp_buf_len) {
      memcpy(s->buf, &s->tmp_buf[s->tmp_buf_copied],
             s->tmp_buf_len - s->tmp_buf_copied);
      s->blen += s->tmp_buf_len - s->tmp_buf_copied;
    }
  }

  if(immediate != 0 && s->blen > 0) {
    PSOCK_SEND(&s->sout, (uint8_t *)s->buf, s->blen);
    s->blen = 0;
  }

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_top_matter(struct httpd_state *s, const char *title,
                              const char **css))
{
  PT_BEGIN(&s->top_matter_pt);

  PT_WAIT_THREAD(&s->top_matter_pt, enqueue_chunk(s, 0, http_doctype));
  PT_WAIT_THREAD(&s->top_matter_pt, enqueue_chunk(s, 0, http_html_start));
  PT_WAIT_THREAD(&s->top_matter_pt, enqueue_chunk(s, 0, http_title_start));

  PT_WAIT_THREAD(&s->top_matter_pt, enqueue_chunk(s, 0, title));
  PT_WAIT_THREAD(&s->top_matter_pt, enqueue_chunk(s, 0, http_title_end));

  if(css != NULL) {
    for(s->ptr = css; *(s->ptr) != NULL; s->ptr++) {
      PT_WAIT_THREAD(&s->top_matter_pt, enqueue_chunk(s, 0, *(s->ptr)));
    }
  }

  PT_WAIT_THREAD(&s->top_matter_pt, enqueue_chunk(s, 0, http_head_charset));
  PT_WAIT_THREAD(&s->top_matter_pt, enqueue_chunk(s, 0, http_head_end));
  PT_WAIT_THREAD(&s->top_matter_pt, enqueue_chunk(s, 0, http_body_start));

  /* Links */
  PT_WAIT_THREAD(&s->top_matter_pt,
                 enqueue_chunk(s, 0, SECTION_OPEN "<p>"));

  s->page = list_head(pages_list);
  PT_WAIT_THREAD(&s->top_matter_pt,
                 enqueue_chunk(s, 0, "[ <a href=\"%s\">%s</a> ]",
                               s->page->filename, s->page->title));

  for(s->page = s->page->next; s->page != NULL; s->page = s->page->next) {
    PT_WAIT_THREAD(&s->top_matter_pt,
                   enqueue_chunk(s, 0, " | [ <a href=\"%s\">%s</a> ]",
                                 s->page->filename, s->page->title));
  }

  PT_WAIT_THREAD(&s->top_matter_pt,
                 enqueue_chunk(s, 0, "</p>" SECTION_CLOSE));

  PT_END(&s->top_matter_pt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_ws_config(struct httpd_state *s))
{
  PT_BEGIN(&s->generate_pt);

  /* Generate top matter (doctype, title, nav links etc) */
  PT_WAIT_THREAD(&s->generate_pt,
                 generate_top_matter(s, http_ws_cfg_page.title,
                                     http_config_css));

  /* MQTT webserver configuration */
  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "<h1>%s</h1>", http_ws_cfg_page.title));

  /* This is the User Auth form */
  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0,
                               "<form name=\"input\" action=\"%s\" ",
                               http_ws_cfg_page.filename));
  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "method=\"post\" enctype=\""));
  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "application/x-www-form-urlencoded\" "));
  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "accept-charset=\"UTF-8\">"));

  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "%sAuth User:%s", config_div_left,
                               config_div_close));

  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "%s<input type=\"text\" ",
                               config_div_right));
  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "value=\"%s\" ", conf.auth_user));

  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "name=\"auth_user\">"));

  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "<input type=\"submit\" value=\"Submit\">%s",
                               config_div_close));

  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "</form>"));

  /* This is the User Token form */
  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0,
                               "<form name=\"input\" action=\"%s\" ",
                               http_ws_cfg_page.filename));
  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "method=\"post\" enctype=\""));
  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "application/x-www-form-urlencoded\" "));
  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "accept-charset=\"UTF-8\">"));

  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "%sAuth Token:%s", config_div_left,
                               config_div_close));

  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "%s<input type=\"text\" ",
                               config_div_right));
  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "value=\"%s\" ", conf.auth_token));

  PT_WAIT_THREAD(&s->generate_pt, enqueue_chunk(s, 0, "name=\"auth_token\">"));

  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "<input type=\"submit\" value=\"Submit\">%s",
                               config_div_close));

  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "</form>"));

  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "<br>%sZolertia SL, Antonio Lignan, 2016%s",
                               config_div_left, config_div_close));

  PT_WAIT_THREAD(&s->generate_pt, enqueue_chunk(s, 1, http_bottom));

  PT_END(&s->generate_pt);
}
/*---------------------------------------------------------------------------*/
static void
lock_obtain(struct httpd_state *s)
{
  if(lock == NULL) {
    lock = s;
  }
}
/*---------------------------------------------------------------------------*/
static void
lock_release(struct httpd_state *s)
{
  if(lock == s) {
    lock = NULL;
  }
}
/*---------------------------------------------------------------------------*/
static void
parse_post_request_chunk(char *buf, int buf_len, int last_chunk)
{
  int i;
  int finish;

  for(i = 0; i < buf_len; i++) {
    switch(state) {
    case PARSE_POST_STATE_INIT:
      state = PARSE_POST_STATE_MORE;
    /* continue */
    case PARSE_POST_STATE_MORE:
      memset(key, 0, PARSE_POST_BUF_SIZES);
      memset(val, 0, PARSE_POST_BUF_SIZES);
      memset(val_escaped, 0, PARSE_POST_BUF_SIZES);
      key_len = 0;
      val_len = 0;
      state = PARSE_POST_STATE_READING_KEY;
    /* continue */
    case PARSE_POST_STATE_READING_KEY:
      if(buf[i] == ISO_equal) {
        state = PARSE_POST_STATE_READING_VAL;
      } else if(buf[i] == ISO_amp) {
        /* Don't accept an amp while reading a key */
        state = PARSE_POST_STATE_ERROR;
      } else {
        /* Make sure we don't overshoot key's boundary */
        if(key_len <= PARSE_POST_MAX_POS) {
          key[key_len] = buf[i];
          key_len++;
        } else {
          /* Not enough space for the key. Abort */
          state = PARSE_POST_STATE_ERROR;
        }
      }
      break;
    case PARSE_POST_STATE_READING_VAL:
      finish = 0;
      if(buf[i] == ISO_amp) {
        finish = 1;
      } else if(buf[i] == ISO_equal) {
        /* Don't accept an '=' while reading a val */
        state = PARSE_POST_STATE_ERROR;
      } else {
        /* Make sure we don't overshoot key's boundary */
        if(val_len <= PARSE_POST_MAX_POS) {
          val[val_len] = buf[i];
          val_len++;
          /* Last character of the last chunk */
          if((i == buf_len - 1) && (last_chunk == 1)) {
            finish = 1;
          }
        } else {
          /* Not enough space for the value. Abort */
          state = PARSE_POST_STATE_ERROR;
        }
      }

      if(finish == 1) {
        /*
         * Done reading a key=value pair, either because we encountered an amp
         * or because we reached the end of the message body.
         *
         * First, unescape the value.
         *
         * Then invoke handlers. We will bail out with PARSE_POST_STATE_ERROR,
         * unless the key-val gets correctly processed
         */
        url_unescape(val, val_len, val_escaped, PARSE_POST_BUF_SIZES);
        val_len = strlen(val_escaped);

        for(handler = list_head(post_handlers); handler != NULL;
            handler = list_item_next((void *)handler)) {
          if(handler->handler != NULL) {
            finish = handler->handler(key, key_len, val_escaped, val_len);
          }
          if(finish == HTTPD_SIMPLE_POST_HANDLER_ERROR) {
            state = PARSE_POST_STATE_ERROR;
            break;
          } else if(finish == HTTPD_SIMPLE_POST_HANDLER_OK) {
            /* Restart the state machine to expect the next pair */
            state = PARSE_POST_STATE_MORE;

            /*
             * At least one handler returned OK, therefore we must generate a
             * new config event when we're done.
             */
            config_ok = 1;
            break;
          }
          /* Else, continue */
        }
      }
      break;
    case PARSE_POST_STATE_ERROR:
      /* If we entered the error state earlier, do nothing */
      return;
    default:
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
static httpd_simple_script_t
get_script(const char *name)
{
  page_t *page;

  for(page = list_head(pages_list); page != NULL;
      page = list_item_next(page)) {
    if(strncmp(name, page->filename, strlen(page->filename)) == 0) {
      return page->script;
    }
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_string(struct httpd_state *s, const char *str))
{
  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, str);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_headers(struct httpd_state *s, const char *statushdr,
                       const char *content_type, const char *redir,
                       const char **additional))
{
  PT_BEGIN(&s->generate_pt);

  PT_WAIT_THREAD(&s->generate_pt, enqueue_chunk(s, 0, statushdr));

  for(s->ptr = http_header_srv_str; *(s->ptr) != NULL; s->ptr++) {
    PT_WAIT_THREAD(&s->generate_pt, enqueue_chunk(s, 0, *(s->ptr)));
  }

  if(redir) {
    PT_WAIT_THREAD(&s->generate_pt,
                   enqueue_chunk(s, 0, "Location: %s\r\n", redir));
  }

  if(additional) {
    for(s->ptr = additional; *(s->ptr) != NULL; s->ptr++) {
      PT_WAIT_THREAD(&s->generate_pt, enqueue_chunk(s, 0, *(s->ptr)));
    }
  }

  PT_WAIT_THREAD(&s->generate_pt,
                 enqueue_chunk(s, 0, "Content-type: %s; ", content_type));

  PT_WAIT_THREAD(&s->generate_pt, enqueue_chunk(s, 1, "charset=UTF-8\r\n\r\n"));

  PT_END(&s->generate_pt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_output(struct httpd_state *s))
{
  PT_BEGIN(&s->outputpt);

  s->script = NULL;

  PT_INIT(&s->generate_pt);
  PT_INIT(&s->top_matter_pt);

  if(s->request_type == REQUEST_TYPE_POST) {
    if(s->return_code == RETURN_CODE_OK) {
      PT_WAIT_THREAD(&s->outputpt, send_headers(s, http_header_302,
                                                http_content_type_plain,
                                                s->filename,
                                                NULL));
    } else if(s->return_code == RETURN_CODE_LR) {
      PT_WAIT_THREAD(&s->outputpt, send_headers(s, http_header_411,
                                                http_content_type_plain,
                                                NULL,
                                                http_header_con_close));
      PT_WAIT_THREAD(&s->outputpt, send_string(s, "Content-Length Required\n"));
    } else if(s->return_code == RETURN_CODE_TL) {
      PT_WAIT_THREAD(&s->outputpt, send_headers(s, http_header_413,
                                                http_content_type_plain,
                                                NULL,
                                                http_header_con_close));
      PT_WAIT_THREAD(&s->outputpt, send_string(s, "Content-Length too Large\n"));
    } else if(s->return_code == RETURN_CODE_SU) {
      PT_WAIT_THREAD(&s->outputpt, send_headers(s, http_header_503,
                                                http_content_type_plain,
                                                NULL,
                                                http_header_con_close));
      PT_WAIT_THREAD(&s->outputpt, send_string(s, "Service Unavailable\n"));
    } else {
      PT_WAIT_THREAD(&s->outputpt, send_headers(s, http_header_400,
                                                http_content_type_plain,
                                                NULL,
                                                http_header_con_close));
      PT_WAIT_THREAD(&s->outputpt, send_string(s, "Bad Request\n"));
    }
  } else if(s->request_type == REQUEST_TYPE_GET) {
    s->script = get_script(&s->filename[1]);
    if(s->script == NULL) {
      strncpy(s->filename, "/notfound.html", sizeof(s->filename));
      PT_WAIT_THREAD(&s->outputpt, send_headers(s, http_header_404,
                                                http_content_type_html,
                                                NULL,
                                                http_header_con_close));
      PT_WAIT_THREAD(&s->outputpt,
                     send_string(s, NOT_FOUND));
      uip_close();
      PT_EXIT(&s->outputpt);
    } else {
      PT_WAIT_THREAD(&s->outputpt, send_headers(s, http_header_200,
                                                http_content_type_html,
                                                NULL,
                                                http_header_con_close));
      PT_WAIT_THREAD(&s->outputpt, s->script(s));
    }
  }
  s->script = NULL;
  PSOCK_CLOSE(&s->sout);
  PT_END(&s->outputpt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_input(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sin);

  PSOCK_READTO(&s->sin, ISO_space);

  if(strncasecmp(s->inputbuf, http_get, 4) == 0) {
    s->request_type = REQUEST_TYPE_GET;
    PSOCK_READTO(&s->sin, ISO_space);

    if(s->inputbuf[0] != ISO_slash) {
      PSOCK_CLOSE_EXIT(&s->sin);
    }

    if(s->inputbuf[1] == ISO_space) {
      strncpy(s->filename, http_index_html, sizeof(s->filename));
    } else {
      s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
      strncpy(s->filename, s->inputbuf, sizeof(s->filename));
    }
  } else if(strncasecmp(s->inputbuf, http_post, 5) == 0) {
    s->request_type = REQUEST_TYPE_POST;
    PSOCK_READTO(&s->sin, ISO_space);

    if(s->inputbuf[0] != ISO_slash) {
      PSOCK_CLOSE_EXIT(&s->sin);
    }

    s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
    strncpy(s->filename, s->inputbuf, sizeof(s->filename));

    /* POST: Read out the rest of the line and ignore it */
    PSOCK_READTO(&s->sin, ISO_nl);

    /*
     * Start parsing headers. We look for Content-Length and ignore everything
     * else until we hit the start of the message body.
     *
     * We will return 411 if the client doesn't send Content-Length and 413
     * if Content-Length is too high
     */
    s->content_length = 0;
    s->return_code = RETURN_CODE_LR;
    do {
      s->inputbuf[PSOCK_DATALEN(&s->sin)] = 0;
      /* We anticipate a content length */
      if((PSOCK_DATALEN(&s->sin) > 14) &&
         strncasecmp(s->inputbuf, "Content-Length:", 15) == 0) {
        char *val_start = &s->inputbuf[15];
        s->content_length = atoi(val_start);

        /* So far so good */
        s->return_code = RETURN_CODE_OK;
      }
      PSOCK_READTO(&s->sin, ISO_nl);
    } while(PSOCK_DATALEN(&s->sin) != 2);

    /*
     * Done reading headers.
     * Reject content length greater than CONTENT_LENGTH_MAX bytes
     */
    if(s->content_length > CONTENT_LENGTH_MAX) {
      s->content_length = 0;
      s->return_code = RETURN_CODE_TL;
    }

    if(s->return_code == RETURN_CODE_OK) {
      /* Acceptable Content Length. Try to obtain a lock */
      lock_obtain(s);

      if(lock == s) {
        state = PARSE_POST_STATE_INIT;
      } else {
        s->return_code = RETURN_CODE_SU;
      }
    }

    /* Parse the message body, unless we have detected an error. */
    while(s->content_length > 0 && lock == s &&
          s->return_code == RETURN_CODE_OK) {
      PSOCK_READBUF_LEN(&s->sin, s->content_length);
      s->content_length -= PSOCK_DATALEN(&s->sin);

      /* Parse the message body */
      parse_post_request_chunk(s->inputbuf, PSOCK_DATALEN(&s->sin),
                               (s->content_length == 0));
      if(state == PARSE_POST_STATE_ERROR) {
        /* Could not parse: Bad Request and stop parsing */
        s->return_code = RETURN_CODE_BR;
      }
    }

    /* If the flag is set, we had at least 1 configuration value accepted */
    if(config_ok) {
      process_post(&mqtt_client_process, httpd_simple_event_new_config, NULL);
    }
    config_ok = 0;

    lock_release(s);
  } else {
    PSOCK_CLOSE_EXIT(&s->sin);
  }

  s->state = STATE_OUTPUT;

  while(1) {
    PSOCK_READTO(&s->sin, ISO_nl);
  }

  PSOCK_END(&s->sin);
}
/*---------------------------------------------------------------------------*/
static void
handle_connection(struct httpd_state *s)
{
  handle_input(s);
  if(s->state == STATE_OUTPUT) {
    handle_output(s);
  }
}
/*---------------------------------------------------------------------------*/
static void
appcall(void *state)
{
  struct httpd_state *s = (struct httpd_state *)state;

  if(uip_closed() || uip_aborted() || uip_timedout()) {
    if(s != NULL) {
      s->script = NULL;
      s->blen = 0;
      s->tmp_buf_len = 0;
      memb_free(&conns, s);
    }
  } else if(uip_connected()) {
    s = (struct httpd_state *)memb_alloc(&conns);
    if(s == NULL) {
      uip_abort();
      return;
    }
    tcp_markconn(uip_conn, s);
    PSOCK_INIT(&s->sin, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
    PSOCK_INIT(&s->sout, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
    PT_INIT(&s->outputpt);
    s->script = NULL;
    s->state = STATE_WAITING;
    timer_set(&s->timer, CLOCK_SECOND * 10);
    handle_connection(s);
  } else if(s != NULL) {
    if(uip_poll()) {
      if(timer_expired(&s->timer)) {
        uip_abort();
        s->script = NULL;
        memb_free(&conns, s);
      }
    } else {
      timer_restart(&s->timer);
    }
    handle_connection(s);
  } else {
    uip_abort();
  }
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  tcp_listen(UIP_HTONS(80));
  memb_init(&conns);
  list_add(pages_list, &http_ws_cfg_page);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(httpd_simple_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Started MQTT webserver for provisioning\n");

  httpd_simple_event_new_config = process_alloc_event();

  init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    appcall(data);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
