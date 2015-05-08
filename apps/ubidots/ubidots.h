/*
 * Copyright (c) 2015, George Oikonomou - <george@contiki-os.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup apps
 * @{
 *
 * \defgroup ubidots-engine A client for the Ubidots cloud service
 * @{
 *
 * This engine allows a device to connect to the Ubidots cloud service and
 * push data.
 *
 * The engine supports API version defined by #UBIDOTS_API_VER
 *
 * To keep code footprint / RAM requirements low, the engine only supports
 * a single connection with Ubidots and many of the connection's parameters
 * are selected at compile time. As a consequence of this design decision,
 * each Contiki image may only have one process using this engine. The rest
 * of the engine's documentation refers to this process as the 'consumer'
 * process.
 *
 * The engine supports writing multiple data points using a single request:
 * See the "Collections" section of the Ubidots API docs.
 *
 * The engine assumes that the user
 * - Has previously registered an account and generated a short auth token
 * - Has created the necessary data sources and variables by using the Ubidots
 *   web interface
 *
 * The user MUST satisfy both requirements above before attempting to use the
 * engine.
 *
 * This library's compilation will fail if the auth token has not been defined.
 *
 *     #define UBIDOTS_CONF_AUTH_TOKEN     "<the-token>"
 *
 * Include the opening and closing double-quotes.
 *
 * A normal workflow would be as follows:
 * - The application fires up the engine by placing a call to ubidots_init()
 *   If the client is interested in some of the reply's HTTP headers, it can
 *   specify them at this stage.
 * - Upon reception of the ubidots_event_established event, the application
 *   prepares a POST message by making a single call to ubidots_prepare_post(),
 *   following by one or more calls to ubidots_enqueue_value().
 * - The application triggers the POST message by calling ubidots_post()
 * - When the POST has been sent, the engine will generate
 *   ubidots_event_post_sent.
 * - As soon as the reply starts streaming, the library will generate a series
 *   of synchronous ubidots_event_post_reply_received events. Each event will
 *   pass to the consumer a part of the reply (HTTP status, HTTP headers,
 *   payload). The application will have to process it immediately, or copy it
 *   to separate RAM storage, since the memory will be overwritten as soon as
 *   the application has finished processing the event.
 *
 * The library will refuse to send a POST if a previous POST is in-flight.
 *
 * The resource to be updated must be specified in calls to
 * ubidots_enqueue_value() and ubidots_prepare_post() through the \e variable
 * argument.
 *
 * The optional 'timestamp' argument is not supported, due to it being
 * specified as msec since the epoch (POSIX), which is something not currently
 * supported by Contiki.
 *
 * For more information, on the Ubidots API, visit:
 * http://ubidots.com/docs/index.html
 *
 * Many thanks to Antonio Liñán Colina ([Zolertia](http://zolertia.io)) for
 * his ideas and contributions, and also to the Ubidots team for the support.
 */
/**
 * \file
 *    Header file for the Ubidots client engine
 *
 * \author
 *    George Oikonomou - <george@contiki-os.org>,
 */
/*---------------------------------------------------------------------------*/
#ifndef UBIDOTS_H_
#define UBIDOTS_H_
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "sys/process.h"
#include "net/ip/uipopt.h"
/*---------------------------------------------------------------------------*/
#if !UIP_TCP
#error "The Ubidots engine requires an image with TCP support"
#error "Check the value of UIP_CONF_TCP"
#endif
/*---------------------------------------------------------------------------*/
#define UBIDOTS_API_VER  "1.6" /**< The currently supported API version */
/*---------------------------------------------------------------------------*/
/**
 * \name Ubidots function return values
 * @{
 */
/** Generic success return value */
#define UBIDOTS_SUCCESS  1

/** Generic error return value */
#define UBIDOTS_ERROR   -1
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Ubidots HTTP reply processing
 * @{
 */
/** The ubidots_reply_part_t::content holds the HTTP status (long int) */
#define UBIDOTS_REPLY_TYPE_HTTP_STATUS 0

/** The ubidots_reply_part_t::content holds an HTTP header (char *) */
#define UBIDOTS_REPLY_TYPE_HTTP_HEADER 1

/** The ubidots_reply_part_t::content holds the HTTP (char *) */
#define UBIDOTS_REPLY_TYPE_PAYLOAD     2
/*---------------------------------------------------------------------------*/
/**
 * A structure holding information about the HTTP reply from the server.
 * This is passed to the consumer process as the data portion of the
 * ubidots_event_post_reply_received event
 */
typedef struct ubidots_reply_part {
  /**
   * An opaque pointer to the content of the HTTP reply that we are telling
   * the consumer about. This will be a pointer to a buffer for HTTP reply
   * headers and the payload, or a pointer to long int for the HTTP status.
   * The data type can be determined by looking at the type field
   */
  const void *content;

  /** Indicates the type of data pointed to by the content field */
  uint8_t type;
} ubidots_reply_part_t;
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Ubidots engine events
 * @{
 */
/** Generated when the client disconnects from Ubidots */
extern process_event_t ubidots_event_disconnected;

/**
 * \brief Generated to notify the consumer process that the engine is ready
 * to POST data
 *
 * When the consumer process receives this event, it should place calls to
 * ubidots_prepare_post(), ubidots_enqueue_value() and ubidots_post() in order
 * to send data ta Ubidots.
 */
extern process_event_t ubidots_event_established;

/**
 * Generated to notify the consumer process that the last post has been
 * completed
 */
extern process_event_t ubidots_event_post_sent;

/**
 * Generated multiple times as the reply streams. This is a synchronous event.
 * It is accompanied by a pointer to a ubidots_reply_part_t. The type field
 * indicates whether content hods an HTTP status, an HTTP header or the
 * response's payload
 */
extern process_event_t ubidots_event_post_reply_received;
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Ubidots engine functions
 * @{
 */
/**
 * \brief Initialise the Ubidots engine
 * \param consumer A pointer to the process that initialised the engine
 * \param headers A string array of HTTP reply headers the consumer is
 *        interested in being notified about
 *
 * Events generated by the Ubidots engine will be posted to the process
 * specified by the \e consumer argument.
 *
 * If the consumer is interested in any of the HTTP reply headers, they can
 * be specified as part of the \e headers argument. The \e headers string array
 * must end with a NULL.
 */
void ubidots_init(struct process *consumer, const char **headers);

/**
 * \brief Prepare an HTTP POST request to be used to send data
 * \param variable A string containing the variable to POST or NULL
 * \return UBIDOTS_SUCCESS or UBIDOTS_ERROR
 *
 * This function will prepare HTTP headers and will otherwise prepare the
 * engine to accept values.
 *
 * This function MUST be called each time the consumer wishes to post data.
 * This call MUST then be followed by one ore more calls to
 * ubidots_enqueue_value(), followed by a single call to ubidots_post()
 *
 * If the argument \e variable is NULL, then the engine will prepare to post
 * multiple values with different variables, by POST to this URL:
 * /api/v1.6/collections/values
 * In this case, ALL subsequent calls to ubidots_enqueue_value() MUST specify
 * a non-NULL value for the variable argument
 *
 * If the argument \e variable is NOT NULL, then the engine will prepare to
 * post one ore more values for the same variable, by posting to this URL:
 * /api/v1.6/variables/\<variable\>/values
 * where \<variable\> is replaced by the value of \e variable
 * By using this format, all \e value arguments in subsequent calls to
 * ubidots_enqueue_value() will be with reference to this same variable.
 * If \e variable is specified here, then ALL subsequent calls to
 * ubidots_enqueue_value() must specify NULL for the \e variable argument.
 * Failing to do so will result in HTTP POSTs that violate the Ubidots API
 */
int ubidots_prepare_post(const char *variable);

/**
 * \brief Enqueues a data point to a previously initialised request
 * \param variable A string containing the variable to POST or NULL
 * \param value The value to be POSTEd
 * \return UBIDOTS_SUCCESS or UBIDOTS_ERROR
 *
 * If \e value should be encoded as a JSON string, the surrounding double quotes
 * must be included in a call to this function. Skip them to encode as a JSON
 * number. To encode as JSON boolean, pass "true" or "false". Some examples:
 *
 * - ubidots_enqueue_value(NULL, "32"); will produce {"value":32}
 * - ubidots_enqueue_value(NULL, "\"32\\""); will produce {"value":"32"}
 * - ubidots_enqueue_value(NULL, "false"); will produce {"value":false}
 *
 * Regarding \e variable, it depends on whether it was specified or not in the
 * previous call to ubidots_prepare_post(). Read this function's documentation
 * for more details.
 */
int ubidots_enqueue_value(const char *variable, const char *value);

/**
 * \brief Post previously enqueued data
 * \return UBIDOTS_SUCCESS or UBIDOTS_ERROR
 *
 * This function MUST be preceded by a call to ubidots_prepare_post(),
 * followed by one or more calls to ubidots_enqueue_value(). The entire HTTP
 * POST request must fit in a single buffer.
 *
 * The caller does NOT need to allocate the buffer, but must make sure that
 * the outgoing buffer is large enough for the POST request by specifying large
 * enough a value for UBIDOTS_OUT_BUFFER_SIZE
 *
 * The function will return UBIDOTS_ERROR if a previous POST is still in
 * progress
 */
int ubidots_post(void);
/** @} */
/*---------------------------------------------------------------------------*/
#endif /* UBIDOTS_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
