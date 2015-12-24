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
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "rich.h"
#include "uart1.h"
#include "dev/uart-driver.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch-rpl.h"
#include "net/mac/tsch/tsch.h"

#include <stdio.h>
#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

static uip_ip6addr_t ip6addr_coordinator;
static linkaddr_t coord_address;
static struct uip_udp_conn *server_conn;
static uint8_t udp_buf[MAX_PAYLOAD_LEN];

/* UART1 data */
/* Ringbuf: written by uart1 isr. Size must be power of 2 */
#define RINGBUF_SIZE 32
#define RINGBUF_MAX_INDEX (RINGBUF_SIZE-1)
static uint8_t ringbuf[RINGBUF_SIZE];
static int head_index = 1;  /* index to write next data */
static int tail_index = 0;  /* index where last read took place */

/* String aligned buffer. Contains '\0' terminated rx string for server process */
#define RX_BUFFER_SIZE  RINGBUF_SIZE
static uint8_t rx_buf[RX_BUFFER_SIZE+1]; 
static uint8_t rx_buf_index = 0; /* index for rx_buf */

/* XY table data */
/* Initialisation strings XY table */
static const char * XY_Startup_Strings[] = { "$1 X8 Y12 Z21",
                                             "$2 X11 Y13 Z21", /* Avoid conflict pin 9; used to be Z9. Solve if servo is used */
                                             "$3 X22 Y9 Z22", 
                                             "$4 X23 Y10 Z23",
/*                                           "$6 X357.0 Y357.0 Z40.0",  */ /* gain for 1/16 step mode */
                                             "$6 X89.0 Y89.0 Z40.0",     /* gain for 1/8 step mode */
                                             "G90",        // Absolute
                                             "G21",        // [mm]
                                             "G100"};      // Go to origin and use as reference
static int ok_received = 0;
static int wait_for_ok = 0;
static int associated = 0;
static struct etimer et;

/* Process data */
PROCESS(udp_server_process, "UDP server process");
PROCESS(rx_uart1_process, "RX UART1 process");
AUTOSTART_PROCESSES(&udp_server_process, &rx_uart1_process);

/* Local functions */
static int xy_connected(void);
static void udp_handler(void);
static int XY_table_init(void);
static void string2uart1(uint8_t *c);
static int handleRxChar(uint8_t c);
static int get_ringbuf(uint8_t *c);
static int put_ringbuf(uint8_t c);

/* Callback function called when device is associated via TSCH_CALLBACK_JOINING_NETWORK
   definition in configuration.Use to trigger set-up of schedule */
extern void
xy_server_associated(void)
{
  associated = 1;
  tsch_rpl_callback_joining_network();   
  process_post(&udp_server_process, PROCESS_EVENT_CONTINUE, NULL);
} 

static int 
xy_connected(void)
{
  const uip_lladdr_t *client_lladdr = uip_ds6_nbr_lladdr_from_ipaddr(&ip6addr_coordinator);
  if (client_lladdr != NULL) {
    PRINTF("XY Connected\n");
    etimer_stop(&et);
  } else {
    etimer_restart(&et);
  }
  return (client_lladdr != NULL);
}

static void
udp_handler(void)
{
  uint8_t buf[10]; 

  if(uip_newdata()) {
    ((uint8_t *)uip_appdata)[uip_datalen()] = '\0';
    if (!strncmp(uip_appdata, XY_LOCATION, strlen(XY_LOCATION))) {
      /* New location data received from coordinator/client.
         String after XY_LOCATION to be transferred to XY device */
      uint8_t * xy_gcode = uip_appdata + strlen(XY_LOCATION) + 1;
      PRINTF("UDP-RX: %s\n", xy_gcode);
      string2uart1(xy_gcode);
      wait_for_ok = 1;
    } else if (!strcmp(uip_appdata, XY_RESET)) {
      sprintf((char *)buf, "G100");
      PRINTF("%s", buf);
      string2uart1(buf);
      wait_for_ok = 1;
    } else {
      printf("XY Received unknown data\n"); 
    }
  }
}

static void 
rx_uart1_handler(void)
{
  /* String is present in rx_buf. Consume it. Flags only updated if cleared. 
     Used to prevent over run */
  if (!ok_received) {
      ok_received = !strncmp((char *)rx_buf, "ok", strlen("ok"));
  }
  process_post(&udp_server_process, PROCESS_EVENT_CONTINUE, NULL);
}

static int 
XY_table_init(void)
{
  /* Send startup strings to XY table over UART1. Wait for "ok" being returned before
     sending next string
     If all string are sent, '1' is returned */ 
  static int string_cnt =  0;
  int return_value = 0;
  if (ok_received){
    ok_received = 0; 
    if (string_cnt < sizeof(XY_Startup_Strings)/sizeof(char *)) {
      PRINTF("%s\n",XY_Startup_Strings[string_cnt]);
      string2uart1((uint8_t *)XY_Startup_Strings[string_cnt]); 
      string_cnt++;
    } else {
      return_value = 1;
    }
  }
  return return_value;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  struct tsch_slotframe *sf;
  static struct etimer et;

  PROCESS_BEGIN();
  PRINTF("XY server started\n");

  /* Get IPv6 address and link address of coordinator */
  uiplib_ipaddrconv(QUOTEME(UDP_ADDR_COORDINATOR), &ip6addr_coordinator);
  coord_address.u8[0] = 0;
  memcpy(&coord_address.u8[1],&ip6addr_coordinator.u8[9],7);

 /* Initialise UART1 */
  uart1_init(UART1_BAUD_RATE); 
  /* Callback received byte */
  uart1_set_input(handleRxChar);

   /* Initialise and calibrate XY table */
  PRINTF("XY Table initialisation\n");
  while (!XY_table_init()){
    PROCESS_WAIT_EVENT_UNTIL(ev = PROCESS_EVENT_CONTINUE);
  }
  PRINTF("Exit XY Table initialisation\n");

  /* Start rich stack */
  rich_init(NULL);
  /* Wait till xy_server_associated() indicates association */
  PRINTF("Wait for association\n");
  while (!associated) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
  }
  PRINTF("Associated\n");

  /* Set schedule */
  /* TS0: EB slot. Already configured with tsch_schedule_create_minimal() 
     after association in tsch_associate()*/  
  sf = tsch_schedule_get_slotframe_by_handle(0);
   /* TS1: C->XY */
	tsch_schedule_add_link(sf, LINK_OPTION_RX, LINK_TYPE_NORMAL, &coord_address, 1, 0);
	/* TX4: XY->C */
	tsch_schedule_add_link(sf, LINK_OPTION_TX, LINK_TYPE_NORMAL, &coord_address, 18, 0);
  tsch_schedule_print();

  /* Server socket only connects with coordinator-client */
  server_conn = udp_new(&ip6addr_coordinator, UIP_HTONS(3001), NULL);
  udp_bind(server_conn, UIP_HTONS(3000));

  PRINTF("Wait for coordinator client to connect .....\n");
  etimer_set(&et, CHECK_CONNECT_INTERVAL);
  while(!xy_connected()) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)); 
  }

  /* Next loop will wait for xy-commands from coordinator/client and return ok when handled */
  while(1) {
    /* Wait for XY location data or XY reset */
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    udp_handler();
    if (wait_for_ok) {   
      /* Valid location data received and sent to XY device. Now wait till ok received from XY device */
      while (!ok_received) {
        PROCESS_WAIT_EVENT_UNTIL(ev = PROCESS_EVENT_CONTINUE);
      }
      PRINTF("OK from XY device\n");
      ok_received = 0;
      /* ok received from XY device. Echo back to client/server */
      sprintf((char *)udp_buf, "%s", XY_OK);
      uip_udp_packet_send(server_conn, udp_buf, strlen((char *)udp_buf));
      wait_for_ok = 0;
      PRINTF("UDP-TX: OK\n\n");
    }
  }  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rx_uart1_process, ev, data)
{
  PROCESS_BEGIN();
  /* Process is polled whenever data is available from uart1 isr. 
     Whenever full string is available ('\0' detected), rx_uart1_handler is called
     to consume rx buffer */
  uint8_t c;

  while(1) {
     PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
    /* Read RX ringbuffer. ASCII chars Output when '\0' is seen. 
       If overflowed, strings are skipped */ 
    do {
      if (get_ringbuf(&c) == -1) {
        break;    /* No more rx char's in ringbuffer */
      } else {
        if (rx_buf_index == RX_BUFFER_SIZE) {   /* Skip current content if buffer full */
          rx_buf_index = 0;
        } 
        rx_buf[rx_buf_index++] = c;
        if ((c == '\n')||(c == '\r')||(c == '\0')) {
          rx_buf[rx_buf_index] = '\0';
          /* Signal event to coap clients.
             Demo assumes data is consumed before new data comes in ! */
          rx_uart1_handler();     
          rx_buf_index = 0;      
        }
      }
    } while (1);
  }
  PROCESS_END();
}

/*****************************************************************************/
/* UART1 helper functions                                                    */
/*****************************************************************************/
/* TX function for UART0 */
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
    process_poll(&rx_uart1_process);
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

