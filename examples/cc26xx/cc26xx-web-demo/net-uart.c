/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 */
/**
 * \addtogroup cc26xx-web-demo
 * @{
 *
 * \file
 *     A process which receives data over UART and transmits them over UDP
 *     to a pre-defined IPv6 address and port. It also listens on the same UDP
 *     port for messages, which it prints out over UART.
 *
 *     For this example to work, you will have to modify the destination IPv6
 *     address by adjusting the set_dest_addr() macro below.
 *
 *     To listen on your linux or OS X box:
 *     nc -6ulkw 1 REMOTE_PORT
 *
 *     (REMOTE_PORT should be the actual value of the define below, e.g. 7777)
 *
 *     Once netcat is up and listening, type something to the CC26xx's terminal
 *     Bear in mind that the datagram will only be sent after a 0x0a (LF) char
 *     has been received. Therefore, if you are on Win, do NOT use PuTTY for
 *     this purpose, since it does not send 0x0a as part of the line end. On
 *     Win XP use hyperterm. On Win 7 use some other software (e.g. Tera Term,
 *     which can be configured to send CRLF on enter keystrokes).
 *
 *     To send data in the other direction from your linux or OS X box:
 *
 *        nc -6u \<node IPv6 address\> REMOTE_PORT
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "sys/process.h"
#include "dev/serial-line.h"
#include "dev/cc26xx-uart.h"
#include "net/ip/uip.h"
#include "net/ip/uip-udp-packet.h"
#include "net/ip/uiplib.h"
#include "net-uart.h"
#include "httpd-simple.h"
#include "sys/cc.h"

#include "ti-lib.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"
/*---------------------------------------------------------------------------*/
#define REMOTE_PORT  7777
#define MAX_MSG_SIZE  100

#define set_dest_addr() uip_ip6addr(&remote_addr, \
                                    0xBBBB, 0x0000, 0x0000, 0x0000, \
                                    0x3E07, 0x54FF, 0xFE74, 0x4885);
/*---------------------------------------------------------------------------*/
#define ADDRESS_CONVERSION_OK       1
#define ADDRESS_CONVERSION_ERROR    0
/*---------------------------------------------------------------------------*/
static struct uip_udp_conn *udp_conn = NULL;

static uint8_t buffer[MAX_MSG_SIZE];
static uint8_t msg_len;
static uip_ip6addr_t remote_addr;
/*---------------------------------------------------------------------------*/
#define IPV6_ADDR_STR_LEN       64
/*---------------------------------------------------------------------------*/
PROCESS(net_uart_process, "Net UART Process");
/*---------------------------------------------------------------------------*/
/*
 * \brief Attempts to convert a string representation of an IPv6 address to a
 * numeric one.
 * \param buf The buffer with the string to be converted.
 * \return ADDRESS_CONVERSION_OK or ADDRESS_CONVERSION_ERROR
 *
 * ToDo: Add support for NAT64 conversion in case the incoming address is a v4
 * This is now supported in the current master, so when we pull it in this will
 * be very straightforward.
 */
static int
set_new_ip_address(char *buf)
{
  /*
   * uiplib_ip6addrconv will immediately start writing into the supplied buffer
   * even if it subsequently fails. Thus, pass an intermediate buffer
   */
  uip_ip6addr_t tmp_addr;

  int rv = uiplib_ip6addrconv(buf, &tmp_addr);

  if(rv == ADDRESS_CONVERSION_OK) {
    /* Conversion OK, copy to our main buffer */
    memcpy(&remote_addr, &tmp_addr, sizeof(remote_addr));

    PRINTF("Updated remote address ");
    PRINT6ADDR(&remote_addr);
    PRINTF("\n");
  }

  return rv;
}
/*---------------------------------------------------------------------------*/
static void
net_input(void)
{
  if(uip_newdata()) {
    memset(buffer, 0, MAX_MSG_SIZE);
    msg_len = MIN(uip_datalen(), MAX_MSG_SIZE - 1);

    /* Copy data */
    memcpy(buffer, uip_appdata, msg_len);
    printf("%s", (char *)buffer);
  }

  return;
}
/*---------------------------------------------------------------------------*/
static void
release_uart(void)
{
  cc26xx_uart_set_input(NULL);
}
/*---------------------------------------------------------------------------*/
static void
keep_uart_on(void)
{
  cc26xx_uart_set_input(serial_line_input_byte);
}
/*---------------------------------------------------------------------------*/
static int
remote_port_post_handler(char *key, int key_len, char *val, int val_len)
{
  int rv;

  if(key_len != strlen("net_uart_port") ||
     strncasecmp(key, "net_uart_port", strlen("net_uart_port")) != 0) {
    /* Not ours */
    return HTTPD_SIMPLE_POST_HANDLER_UNKNOWN;
  }

  rv = atoi(val);

  if(rv <= 65535 && rv > 0) {
    cc26xx_web_demo_config.net_uart.remote_port = (uint16_t)rv;
  } else {
    return HTTPD_SIMPLE_POST_HANDLER_ERROR;
  }

  return HTTPD_SIMPLE_POST_HANDLER_OK;
}
/*---------------------------------------------------------------------------*/
static int
remote_ipv6_post_handler(char *key, int key_len, char *val, int val_len)
{
  int rv = HTTPD_SIMPLE_POST_HANDLER_UNKNOWN;

  if(key_len != strlen("net_uart_ip") ||
     strncasecmp(key, "net_uart_ip", strlen("net_uart_ip")) != 0) {
    /* Not ours */
    return HTTPD_SIMPLE_POST_HANDLER_UNKNOWN;
  }

  if(val_len > IPV6_ADDR_STR_LEN) {
    /* Ours but bad value */
    rv = HTTPD_SIMPLE_POST_HANDLER_ERROR;
  } else {
    if(set_new_ip_address(val)) {
      memset(cc26xx_web_demo_config.net_uart.remote_address, 0,
             NET_UART_IP_ADDR_STRLEN);
      memcpy(cc26xx_web_demo_config.net_uart.remote_address, val, val_len);
      rv = HTTPD_SIMPLE_POST_HANDLER_OK;
    }
  }

  return rv;
}
/*---------------------------------------------------------------------------*/
static int
on_off_post_handler(char *key, int key_len, char *val, int val_len)
{
  int rv;

  if(key_len != strlen("net_uart_on") ||
     strncasecmp(key, "net_uart_on", strlen("net_uart_on")) != 0) {
    /* Not ours */
    return HTTPD_SIMPLE_POST_HANDLER_UNKNOWN;
  }

  rv = atoi(val);

  /* Be pedantic: only accept 0 and 1, not just any non-zero value */
  if(rv == 0) {
    cc26xx_web_demo_config.net_uart.enable = 0;
    release_uart();
  } else if(rv == 1) {
    cc26xx_web_demo_config.net_uart.enable = 1;
    keep_uart_on();
  } else {
    return HTTPD_SIMPLE_POST_HANDLER_ERROR;
  }

  return HTTPD_SIMPLE_POST_HANDLER_OK;
}
/*---------------------------------------------------------------------------*/
HTTPD_SIMPLE_POST_HANDLER(remote_port, remote_port_post_handler);
HTTPD_SIMPLE_POST_HANDLER(remote_ipv6, remote_ipv6_post_handler);
HTTPD_SIMPLE_POST_HANDLER(on_off, on_off_post_handler);
/*---------------------------------------------------------------------------*/
static void
set_config_defaults(void)
{
  /* Set a hard-coded destination address to start with */
  set_dest_addr();

  /* Set config defaults */
  cc26xx_web_demo_ipaddr_sprintf(cc26xx_web_demo_config.net_uart.remote_address,
                                 NET_UART_IP_ADDR_STRLEN, &remote_addr);
  cc26xx_web_demo_config.net_uart.remote_port = REMOTE_PORT;
  cc26xx_web_demo_config.net_uart.enable = 1;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(net_uart_process, ev, data)
{
  PROCESS_BEGIN();

  printf("CC26XX Net UART Process\n");

  set_config_defaults();

  udp_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(udp_conn, UIP_HTONS(REMOTE_PORT));

  if(udp_conn == NULL) {
    printf("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }

  httpd_simple_register_post_handler(&remote_port_handler);
  httpd_simple_register_post_handler(&remote_ipv6_handler);
  httpd_simple_register_post_handler(&on_off_handler);

  while(1) {

    PROCESS_YIELD();

    if(ev == serial_line_event_message) {
      /*
       * If the message contains a new IP address, save it and go back to
       * waiting.
       */
      if(set_new_ip_address((char *)data) == ADDRESS_CONVERSION_ERROR) {
        /* Not an IP address in the message. Send to current destination */
        memset(buffer, 0, MAX_MSG_SIZE);

        /* We need to add a line feed, thus never fill the entire buffer */
        msg_len = MIN(strlen(data), MAX_MSG_SIZE - 1);
        memcpy(buffer, data, msg_len);

        /* Add a line feed */
        buffer[msg_len] = 0x0A;
        msg_len++;

        uip_udp_packet_sendto(
          udp_conn, buffer, msg_len, &remote_addr,
          UIP_HTONS(cc26xx_web_demo_config.net_uart.remote_port));
      }
    } else if(ev == tcpip_event) {
      net_input();
    } else if(ev == cc26xx_web_demo_config_loaded_event) {
      /*
       * New config. Check if it's possible to update the remote address.
       * The port will have been updated already
       */
      set_new_ip_address(cc26xx_web_demo_config.net_uart.remote_address);

      if(cc26xx_web_demo_config.net_uart.enable == 1) {
        keep_uart_on();
      }
    } else if(ev == cc26xx_web_demo_load_config_defaults) {
      set_config_defaults();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
