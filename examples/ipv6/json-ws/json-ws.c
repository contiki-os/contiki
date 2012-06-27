/*
 * Copyright (c) 2011-2012, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         JSON webservice util
 * \author
 *         Niclas Finne    <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Joel Hoglund    <joel@sics.se>
 */

#include "contiki.h"
#if PLATFORM_HAS_LEDS
#include "dev/leds.h"
#endif
#include "httpd-ws.h"
#include "jsontree.h"
#include "jsonparse.h"
#include "json-ws.h"
#include <stdio.h>
#include <string.h>

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifdef JSON_WS_CONF_CALLBACK_PROTO
#define CALLBACK_PROTO JSON_WS_CONF_CALLBACK_PROTO
#else
#define CALLBACK_PROTO "http"
#endif /* JSON_WS_CONF_CALLBACK_PROTO */

#ifdef JSON_WS_CONF_CALLBACK_PORT
#define CALLBACK_PORT JSON_WS_CONF_CALLBACK_PORT
#else
#define CALLBACK_PORT 8080;
#endif /* JSON_WS_CONF_CALLBACK_PORT */

/* Predefined startup-send interval */
#ifdef JSON_WS_CONF_CALLBACK_INTERVAL
#define SEND_INTERVAL JSON_WS_CONF_CALLBACK_INTERVAL
#else
#define SEND_INTERVAL 120
#endif

static const char http_content_type_json[] = "application/json";

/* Maximum 40 chars in host name?: 5 x 8 */
static char callback_host[40] = "[aaaa::1]";
static uint16_t callback_port = CALLBACK_PORT;
static uint16_t callback_interval = SEND_INTERVAL;
static char callback_path[80] = "/debug/";
static char callback_appdata[80] = "";
static char callback_proto[8] = CALLBACK_PROTO;
static const char *callback_json_path = NULL;
static struct jsontree_object *tree;
static struct ctimer periodic_timer;
long json_time_offset = 0;

/* support for submitting to cosm */
#if WITH_COSM
extern struct jsontree_callback cosm_value_callback;

JSONTREE_OBJECT_EXT(cosm_tree,
                    JSONTREE_PAIR("current_value", &cosm_value_callback));
#endif /* WITH_COSM */

static void periodic(void *ptr);

/*---------------------------------------------------------------------------*/
static void
json_copy_string(struct jsonparse_state *parser, char *string, int len)
{
  jsonparse_next(parser);
  jsonparse_next(parser);
  jsonparse_copy_value(parser, string, len);
}
/*---------------------------------------------------------------------------*/
static int
cfg_get(struct jsontree_context *js_ctx)
{
  const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);

  if(strncmp(path, "host", 4) == 0) {
    jsontree_write_string(js_ctx, callback_host);
  } else if(strncmp(path, "port", 4) == 0) {
    jsontree_write_int(js_ctx, callback_port);
  } else if(strncmp(path, "interval", 8) == 0) {
    jsontree_write_int(js_ctx, callback_interval);
  } else if(strncmp(path, "path", 4) == 0) {
    jsontree_write_string(js_ctx, callback_path);
  } else if(strncmp(path, "appdata", 7) == 0) {
    jsontree_write_string(js_ctx, callback_appdata[0] == '\0' ? "" : "***");
  } else if(strncmp(path, "proto", 5) == 0) {
    jsontree_write_string(js_ctx, callback_proto);
  }
  return 0;
}
static int
cfg_set(struct jsontree_context *js_ctx, struct jsonparse_state *parser)
{
  int type;
  int update = 0;

  while((type = jsonparse_next(parser)) != 0) {
    if(type == JSON_TYPE_PAIR_NAME) {
      if(jsonparse_strcmp_value(parser, "host") == 0) {
        json_copy_string(parser, callback_host, sizeof(callback_host));
        update++;
      } else if(jsonparse_strcmp_value(parser, "path") == 0) {
        json_copy_string(parser, callback_path, sizeof(callback_path));
        update++;
      } else if(jsonparse_strcmp_value(parser, "appdata") == 0) {
        json_copy_string(parser, callback_appdata, sizeof(callback_appdata));
        update++;
      } else if(jsonparse_strcmp_value(parser, "proto") == 0) {
        json_copy_string(parser, callback_proto, sizeof(callback_proto));
        update++;
      } else if(jsonparse_strcmp_value(parser, "port") == 0) {
        jsonparse_next(parser);
        jsonparse_next(parser);
        callback_port = jsonparse_get_value_as_int(parser);
        if(callback_port == 0) {
          callback_port = CALLBACK_PORT;
        }
        update++;
      } else if(jsonparse_strcmp_value(parser, "interval") == 0) {
        jsonparse_next(parser);
        jsonparse_next(parser);
        callback_interval = jsonparse_get_value_as_int(parser);
        if(callback_interval == 0) {
          callback_interval = SEND_INTERVAL;
        }
        update++;
      }
    }
  }
  if(update && callback_json_path != NULL) {
#if WITH_UDP
    if(strncmp(callback_proto, "udp", 3) == 0) {
      json_ws_udp_setup(callback_host, callback_port);
    }
#endif
    ctimer_set(&periodic_timer, CLOCK_SECOND * callback_interval,
               periodic, NULL);
  }
  return 0;
}
static struct jsontree_callback cfg_callback =
  JSONTREE_CALLBACK(cfg_get, cfg_set);

JSONTREE_OBJECT_EXT(json_subscribe_callback,
                    JSONTREE_PAIR("host", &cfg_callback),
                    JSONTREE_PAIR("port", &cfg_callback),
                    JSONTREE_PAIR("path", &cfg_callback),
                    JSONTREE_PAIR("appdata", &cfg_callback),
                    JSONTREE_PAIR("proto", &cfg_callback),
                    JSONTREE_PAIR("interval", &cfg_callback));
/*---------------------------------------------------------------------------*/
static int
time_get(struct jsontree_context *js_ctx)
{
  /* unix time */
  char buf[20];
  unsigned long time = json_time_offset + clock_seconds();

  snprintf(buf, 20, "%lu", time);
  jsontree_write_atom(js_ctx, buf);
  return 0;
}

static int
time_set(struct jsontree_context *js_ctx, struct jsonparse_state *parser)
{
  int type;
  unsigned long time;

  while((type = jsonparse_next(parser)) != 0) {
    if(type == JSON_TYPE_PAIR_NAME) {
      if(jsonparse_strcmp_value(parser, "time") == 0) {
        jsonparse_next(parser);
        jsonparse_next(parser);
        time = jsonparse_get_value_as_long(parser);
        json_time_offset = time - clock_seconds();
      }
    }
  }
  return 0;
}

struct jsontree_callback json_time_callback =
  JSONTREE_CALLBACK(time_get, time_set);
/*---------------------------------------------------------------------------*/
#if PLATFORM_HAS_LEDS
#include "dev/leds.h"

static int
ws_leds_get(struct jsontree_context *js_ctx)
{
  char buf[4];
  unsigned char leds = leds_get();

  snprintf(buf, 4, "%u", leds);
  jsontree_write_atom(js_ctx, buf);
  return 0;
}

static int
ws_leds_set(struct jsontree_context *js_ctx, struct jsonparse_state *parser)
{
  int type, old_leds, new_leds;

  while((type = jsonparse_next(parser)) != 0) {
    if(type == JSON_TYPE_PAIR_NAME) {
      if(jsonparse_strcmp_value(parser, "leds") == 0) {
        jsonparse_next(parser);
        jsonparse_next(parser);
        new_leds = jsonparse_get_value_as_int(parser);
        old_leds = leds_get();
        leds_on(~old_leds & new_leds);
        leds_off(old_leds & ~new_leds);
      }
    }
  }
  return 0;
}

struct jsontree_callback json_leds_callback =
  JSONTREE_CALLBACK(ws_leds_get, ws_leds_set);

#endif /* PLATFORM_HAS_LEDS */
/*---------------------------------------------------------------------------*/
static struct httpd_ws_state *json_putchar_context;
static int
json_putchar(int c)
{
  if(json_putchar_context != NULL &&
     json_putchar_context->outbuf_pos < HTTPD_OUTBUF_SIZE) {
    json_putchar_context->outbuf[json_putchar_context->outbuf_pos++] = c;
    return c;
  }
  return 0;
}
static int putchar_size = 0;
static int
json_putchar_count(int c)
{
  putchar_size++;
  return c;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_values(struct httpd_ws_state *s))
{
  json_putchar_context = s;

  PSOCK_BEGIN(&s->sout);

  s->json.putchar = json_putchar;
  s->outbuf_pos = 0;

  if(s->json.values[0] == NULL) {
    /* Nothing to do */

  } else if(s->request_type == HTTPD_WS_POST &&
            s->state == HTTPD_WS_STATE_OUTPUT) {
    /* Set value */
    struct jsontree_value *v;
    struct jsontree_callback *c;

    while((v = jsontree_find_next(&s->json, JSON_TYPE_CALLBACK)) != NULL) {
      c = (struct jsontree_callback *)v;
      if(c->set != NULL) {
        struct jsonparse_state js;

        jsonparse_setup(&js, s->inputbuf, s->content_len);
        c->set(&s->json, &js);
      }
    }
    memcpy(s->outbuf, "{\"Status\":\"OK\"}", 15);
    s->outbuf_pos = 15;

  } else {
    /* Get value */
    while(jsontree_print_next(&s->json) && s->json.path <= s->json.depth) {
      if(s->outbuf_pos >= UIP_TCP_MSS) {
        SEND_STRING(&s->sout, s->outbuf, UIP_TCP_MSS);
        s->outbuf_pos -= UIP_TCP_MSS;
        if(s->outbuf_pos > 0) {
          memcpy(s->outbuf, &s->outbuf[UIP_TCP_MSS], s->outbuf_pos);
        }
      }
    }
  }

  if(s->outbuf_pos > 0) {
    SEND_STRING(&s->sout, s->outbuf, s->outbuf_pos);
    s->outbuf_pos = 0;
  }
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
struct jsontree_value *
find_json_path(struct jsontree_context *json, const char *path)
{
  struct jsontree_value *v;
  const char *start;
  const char *end;
  int len;

  v = json->values[0];
  start = path;
  do {
    end = strchr(start, '/');
    if(end == start) {
      break;
    }
    if(end != NULL) {
      len = end - start;
      end++;
    } else {
      len = strlen(start);
    }
    if(v->type != JSON_TYPE_OBJECT) {
      v = NULL;
    } else {
      struct jsontree_object *o;
      int i;

      o = (struct jsontree_object *)v;
      v = NULL;
      for(i = 0; i < o->count; i++) {
        if(strncmp(start, o->pairs[i].name, len) == 0) {
          v = o->pairs[i].value;
          json->index[json->depth] = i;
          json->depth++;
          json->values[json->depth] = v;
          json->index[json->depth] = 0;
          break;
        }
      }
    }
    start = end;
  } while(end != NULL && *end != '\0' && v != NULL);
  json->callback_state = 0;
  return v;
}
/*---------------------------------------------------------------------------*/
static int
calculate_json_size(const char *path, struct jsontree_value *v)
{
  /* check size of JSON expression */
  struct jsontree_context json;

  json.values[0] = (v == NULL) ? (struct jsontree_value *)tree : v;
  jsontree_reset(&json);

  if(path != NULL) {
    find_json_path(&json, path);
  }

  json.path = json.depth;
  json.putchar = json_putchar_count;
  putchar_size = 0;
  while(jsontree_print_next(&json) && json.path <= json.depth);

  return putchar_size;
}
/*---------------------------------------------------------------------------*/
httpd_ws_script_t
httpd_ws_get_script(struct httpd_ws_state *s)
{
  struct jsontree_value *v;

  s->json.values[0] = v = (struct jsontree_value *)tree;
  jsontree_reset(&s->json);

  if(s->filename[1] == '\0') {
    /* Default page: show full JSON tree. */
  } else {
    v = find_json_path(&s->json, &s->filename[1]);
  }
  if(v != NULL) {
    s->json.path = s->json.depth;
    s->content_type = http_content_type_json;
    return send_values;
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
#if JSON_POST_EXTRA_HEADER || WITH_COSM
static int
output_headers(struct httpd_ws_state *s, char *buffer, int buffer_size,
               int index)
{
  if(index == 0) {
#ifdef JSON_POST_EXTRA_HEADER
    return snprintf(buffer, buffer_size, "%s\r\n", JSON_POST_EXTRA_HEADER);
  } else if(index == 1) {
#endif
#if WITH_COSM
    if(strncmp(callback_proto, "cosm", 4) == 0 && callback_appdata[0] != '\0') {
      return snprintf(buffer, buffer_size, "X-PachubeApiKey:%s\r\n",
                      callback_appdata);
    }
#endif
  }
  return 0;
}
#endif /* JSON_POST_EXTRA_HEADER || WITH_COSM */
/*---------------------------------------------------------------------------*/
static void
periodic(void *ptr)
{
  struct httpd_ws_state *s;
  int callback_size;

  if(callback_json_path != NULL && strlen(callback_host) > 2) {
    ctimer_restart(&periodic_timer);

    if(strncmp(callback_proto, "http", 4) == 0) {
      callback_size = calculate_json_size(callback_json_path, NULL);
      s = httpd_ws_request(HTTPD_WS_POST, callback_host, NULL, callback_port,
                           callback_path, http_content_type_json,
                           callback_size, send_values);
      if(s != NULL) {
        PRINTF("PERIODIC POST %s\n", callback_json_path);
#if JSON_POST_EXTRA_HEADER
        s->output_extra_headers = output_headers;
#endif
        s->json.values[0] = (struct jsontree_value *)tree;
        jsontree_reset(&s->json);
        find_json_path(&s->json, callback_json_path);
        s->json.path = s->json.depth;
      } else {
        PRINTF("PERIODIC CALLBACK FAILED\n");
      }
#if WITH_COSM
    } else if(strncmp(callback_proto, "cosm", 4) == 0) {
      callback_size = calculate_json_size(NULL, (struct jsontree_value *)
                                          &cosm_tree);
      /* printf("JSON Size:%d\n", callback_size); */
      s = httpd_ws_request(HTTPD_WS_PUT, callback_host, "api.pachube.com",
                           callback_port, callback_path,
                           http_content_type_json, callback_size, send_values);
      /* host = cosm host */
      /* path => path to datastream / data point */
      s->output_extra_headers = output_headers;
      s->json.values[0] = (struct jsontree_value *)&cosm_tree;
      jsontree_reset(&s->json);
      s->json.path = 0;

      PRINTF("PERIODIC cosm callback: %d\n", callback_size);
#endif /* WITH_COSM */
    }
#if WITH_UDP
    else {
      callback_size = calculate_json_size(callback_json_path, NULL);
      PRINTF("PERIODIC UDP size: %d\n", callback_size);
      json_ws_udp_send(tree, callback_json_path);
    }
#endif /* WITH_UDP */
  } else {
    printf("PERIODIC CALLBACK - nothing todo\n");
  }
}
/*---------------------------------------------------------------------------*/
void
json_ws_init(struct jsontree_object *json)
{
  PRINTF("JSON INIT (callback %s every %u seconds)\n",
         CALLBACK_PROTO, SEND_INTERVAL);
  tree = json;
  ctimer_set(&periodic_timer, CLOCK_SECOND * SEND_INTERVAL, periodic, NULL);
  process_start(&httpd_ws_process, NULL);
#if WITH_UDP
  if(strncmp(callback_proto, "udp", 3) == 0) {
    json_ws_udp_setup(callback_host, callback_port);
  }
#endif /* WITH_UDP */
}
/*---------------------------------------------------------------------------*/
void
json_ws_set_callback(const char *path)
{
  callback_json_path = path;
  ctimer_restart(&periodic_timer);
}
/*---------------------------------------------------------------------------*/
