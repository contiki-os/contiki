/*
* Copyright (c) 2015 NXP B.V.
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
* 3. Neither the name of NXP B.V. nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
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
* Author: Theo van Daele <theo.van.daele@nxp.com>
*
*/
#include "net/ip/uip.h"
#include "net/linkaddr.h"
#include "rpl-tools.h"
#include "rest-engine.h"
#include "sys/ctimer.h"
#include "dev/uart-driver.h"
#include "uart1.h"
#include <AppHardwareApi.h>

static void get_coap_rx_uart1_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void event_coap_rx_uart1_handler(void);
static void put_post_tx_uart1_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void string2uart1(uint8_t *c);
static int handleRxChar(uint8_t c);
static int get_ringbuf(uint8_t *c);
static int put_ringbuf(uint8_t c);

/* COAP helpers */
static char content[REST_MAX_CHUNK_SIZE];
static int content_len = 0;
#define CONTENT_PRINTF(...) { if(content_len < sizeof(content)) content_len += snprintf(content+content_len, sizeof(content)-content_len, __VA_ARGS__); }

/* Ringbuf: written to by uart isr. Size must be power of 2 */
#define RINGBUF_SIZE 32
#define RINGBUF_MAX_INDEX (RINGBUF_SIZE-1)
static char ringbuf[RINGBUF_SIZE];
static int head_index = 1;  /* index to write next data */
static int tail_index = 0;  /* index where last read took place */

/* String aligned buffer */
#define RX_BUFFER_SIZE  RINGBUF_SIZE
static uint8_t rx_buf[RX_BUFFER_SIZE+1]; 
static uint8_t rx_buf_index = 0; /* index for rx_buf */

/*---------------------------------------------------------------------------*/
PROCESS(start_app, "START_APP");
PROCESS(rx_data_process, "RX_DATA");
AUTOSTART_PROCESSES(&start_app, &rx_data_process);
/*---------------------------------------------------------------------------*/

/*********** COAP resources *************************************************/
/*****************************************************************************/
/* Observable resource and event handler to obtain terminal input from UART1 */ 
/*****************************************************************************/
EVENT_RESOURCE(resource_coap_rx_uart1,                /* name */
               "obs;title=\"rx_uart1\"",              /* attributes */
               get_coap_rx_uart1_handler,             /* GET handler */
               NULL,                                  /* POST handler */
               NULL,                                  /* PUT handler */
               NULL,                                  /* DELETE handler */
               event_coap_rx_uart1_handler);          /* event handler */
static void
get_coap_rx_uart1_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    CONTENT_PRINTF("%s", rx_buf);
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }
} 

static void
event_coap_rx_uart1_handler(void)
{
  /* Registered observers are notified and will trigger the GET handler to create the response. */
  REST.notify_subscribers(&resource_coap_rx_uart1);
}

/*****************************************************************************/
/* GET/PUT resource to send data to terminal on UART1                        */ 
/*****************************************************************************/
RESOURCE(resource_coap_tx_uart1,                /* name */
         "obs;title=\"tx_uart1\"",              /* attributes */
         NULL,                                  /* GET handler */
         put_post_tx_uart1_handler,             /* POST handler */
         put_post_tx_uart1_handler,             /* PUT handler */
         NULL);                                 /* DELETE handler */

static void
put_post_tx_uart1_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content = NULL;
  unsigned int accept = -1;

  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.get_request_payload(request, &request_content);
    string2uart1((uint8_t *)request_content);
  }
}


/*********** End COAP resources *********************************************/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(start_app, ev, data)
{
  PROCESS_BEGIN();
  static int is_coordinator = 0;

  memset(rx_buf, '\0', sizeof(rx_buf));
  /* Define process that handles data */
  process_start(&rx_data_process ,NULL);
  /* Initialise UART1 */
  uart1_init(UART1_BAUD_RATE); 
  /* Callback received byte */
  uart1_set_input(handleRxChar);

  /* Start network stack */
  if(is_coordinator) {
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_tools_init(&prefix);
  } else {
    rpl_tools_init(NULL);
  }
  printf("Starting RPL node\n");
  
  rest_init_engine();
  rest_activate_resource(&resource_coap_rx_uart1, "UART1-RX");
  rest_activate_resource(&resource_coap_tx_uart1, "UART1-TX");

  PROCESS_END();
}


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rx_data_process, ev, data)
{
  PROCESS_BEGIN();
  
  /* Process is polled whenever data is available from uart isr */
  uint8_t c;

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    /* Read RX ringbuffer. ASCII chars Output when LF is seen. 
       If overflowed, strings are skipped */ 
    do {
      if (get_ringbuf(&c) == -1) {
        break;    /* No more rx char's in ringbuffer */
      } else {
        if (rx_buf_index == RX_BUFFER_SIZE) {   /* Skip current content if buffer full */
          rx_buf_index = 0;
        } 
        rx_buf[rx_buf_index++] = c;
        if ((c == '\n')||(c == '\r')) {
          rx_buf[rx_buf_index] = '\0';
          printf("RX on UART1: %s", rx_buf);   
          /* Signal event to coap clients.
             Demo assumes data is consumed before new data comes in */
          event_coap_rx_uart1_handler();     
          rx_buf_index = 0;      
        }
      }
    } while (1);
  }
  PROCESS_END();
}

/*************************************************************************/
/* Local test functions                                                  */
/*************************************************************************/
/* TX function for UART1 */
static void 
string2uart1(uint8_t *c)
{
  while (*c!= '\0') {
    uart1_writeb(*c);
    c++;
  }
}

/* handleRxChar runs on uart isr */
static int 
handleRxChar(uint8_t c)
{
  if (put_ringbuf(c) == -1) {
    printf("Ringbuf full. Skip char\n");
  } else {
    /* Send event to rx data handler */
    process_poll(&rx_data_process);
  }
  return 0;
}

/* Simple ringbuffer
   if tail==head, no data has been written yet on that position. So, empty buffer
   is also initial state */
static int 
get_ringbuf(uint8_t *c)
{
  int return_val = 0;

  uart1_disable_interrupts();
  if (head_index != ((tail_index + 1)&RINGBUF_MAX_INDEX)) {
    tail_index = ((tail_index + 1)& RINGBUF_MAX_INDEX);
    *c = ringbuf[tail_index];
  } else {
    return_val = -1;
  }
  uart1_enable_interrupts();
  return return_val;
}

static int 
put_ringbuf(uint8_t c)
{
  int return_val = 0;

  uart1_disable_interrupts();
  if (head_index != tail_index) {
    ringbuf[head_index] = c;
    head_index = ((head_index+1)&RINGBUF_MAX_INDEX); 
  } else {
    return_val = -1;
  }
  uart1_enable_interrupts();
  return return_val;                
}

