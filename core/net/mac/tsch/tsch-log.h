/*
 * Copyright (c) 2014, SICS Swedish ICT.
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
 *
 */

#ifndef __TSCH_LOG_H__
#define __TSCH_LOG_H__

/********** Includes **********/

#include "contiki.h"
#include "sys/rtimer.h"
#include "net/mac/tsch/tsch-private.h"

/******** Configuration *******/

/* The length of the log queue, i.e. maximum number postponed log messages */
#ifdef TSCH_LOG_CONF_QUEUE_LEN
#define TSCH_LOG_QUEUE_LEN TSCH_LOG_CONF_QUEUE_LEN
#else /* TSCH_LOG_CONF_QUEUE_LEN */
#define TSCH_LOG_QUEUE_LEN 8
#endif /* TSCH_LOG_CONF_QUEUE_LEN */

/* Returns an integer ID from a link-layer address */
#ifdef TSCH_LOG_CONF_ID_FROM_LINKADDR
#define TSCH_LOG_ID_FROM_LINKADDR(addr) TSCH_LOG_CONF_ID_FROM_LINKADDR(addr)
#else /* TSCH_LOG_ID_FROM_LINKADDR */
#define TSCH_LOG_ID_FROM_LINKADDR(addr) ((addr) ? (addr)->u8[LINKADDR_SIZE - 1] : 0)
#endif /* TSCH_LOG_ID_FROM_LINKADDR */

/* TSCH log levels:
 * 0: no log
 * 1: basic PRINTF enabled
 * 2: basic PRINTF enabled and tsch-log module enabled */
#ifdef TSCH_LOG_CONF_LEVEL
#define TSCH_LOG_LEVEL TSCH_LOG_CONF_LEVEL
#else /* TSCH_LOG_CONF_LEVEL */
#define TSCH_LOG_LEVEL 2
#endif /* TSCH_LOG_CONF_LEVEL */

#if TSCH_LOG_LEVEL < 2 /* For log level 0 or 1, the logging functions do nothing */

#define tsch_log_init()
#define tsch_log_process_pending()
#define TSCH_LOG_ADD(log_type, init_code)

#else /* TSCH_LOG_LEVEL */

/************ Types ***********/

/* Structure for a log. Union of different types of logs */
struct tsch_log_t {
  enum { tsch_log_tx,
         tsch_log_rx,
         tsch_log_message
  } type;
  struct tsch_asn_t asn;
  struct tsch_link *link;
  union {
    char message[48];
    struct {
      int mac_tx_status;
      int dest;
      int drift;
      uint8_t num_tx;
      uint8_t datalen;
      uint8_t is_data;
      uint8_t sec_level;
      uint8_t drift_used;
    } tx;
    struct {
      int src;
      int drift;
      int estimated_drift;
      uint8_t datalen;
      uint8_t is_unicast;
      uint8_t is_data;
      uint8_t sec_level;
      uint8_t drift_used;
    } rx;
  };
};

/********** Functions *********/

/* Prepare addition of a new log.
 * Returns pointer to log structure if success, NULL otherwise */
struct tsch_log_t *tsch_log_prepare_add(void);
/* Actually add the previously prepared log */
void tsch_log_commit(void);
/* Initialize log module */
void tsch_log_init(void);
/* Process pending log messages */
void tsch_log_process_pending(void);

/************ Macros **********/

/* Use this macro to add a log to the queue (will be printed out
 * later, after leaving interrupt context) */
#define TSCH_LOG_ADD(log_type, init_code) do { \
    struct tsch_log_t *log = tsch_log_prepare_add(); \
    if(log != NULL) { \
      log->type = (log_type); \
      init_code; \
      tsch_log_commit(); \
    } \
} while(0);

#endif /* TSCH_LOG_LEVEL */

#endif /* __TSCH_LOG_H__ */
