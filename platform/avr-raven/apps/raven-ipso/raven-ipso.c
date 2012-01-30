/*   Copyright (c) 2008, Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of the copyright holders nor the names of
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 *  \brief This module contains code to interface a Contiki-based
 *  project on the AVR Raven platform's ATMega1284P chip to the LCD
 *  driver chip (ATMega3290P) on the Raven.
 *  
 *  \author Durvy Mathilde <mdurvy@cisco.com>
 *
 */

/**  \addtogroup raven
 * @{ 
 */

/**
 *  \defgroup ravenserial Serial interface between Raven processors
 * @{
 */

/**
 *  \file
 *  This file contains code to connect the two AVR Raven processors via
 *  a serial connection for the IPSO interop application
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "mac.h"

#include "raven-lcd.h"

#include <string.h>
#include <stdio.h>

#define cmd_len 8
#define data_len 20

static uint8_t seqno;
uip_ipaddr_t ping_addr;
uip_ipaddr_t udp_addr;
static struct uip_udp_conn *udp_conn;
char udp_data[data_len];

static struct{
  uint8_t frame[cmd_len];
  uint8_t ndx;
  uint8_t len;
  uint8_t cmd;
  uint8_t done;
} cmd;

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

void rs232_send(uint8_t port, unsigned char c);

/*------------------------------------------------------------------*/
/* Sends a ping packet out the radio */
void
raven_ping6(void)
{
   
  /* ping ipv6.google.com*/
  uip_ip6addr(&ping_addr,0x2001,0x420,0x5FFF,0x7D,0x2D0,0xB7FF,0xFE23,0xE6DB);
  //uip_ip6addr(&ping_addr, 0x2001, 0x4860, 0, 0x2001, 0, 0, 0, 0x68);
  //uip_ip6addr(&ping_addr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
  
  UIP_IP_BUF->vtc = 0x60;
  UIP_IP_BUF->tcflow = 1;
  UIP_IP_BUF->flow = 0;
  UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
//UIP_IP_BUF->ttl = uip_netif_physical_if.cur_hop_limit;
  UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;
  uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &ping_addr);
//uip_netif_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
  uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr); 
  UIP_ICMP_BUF->type = ICMP6_ECHO_REQUEST;
  UIP_ICMP_BUF->icode = 0;
  /* set identifier and sequence number to 0 */
  memset((void *)UIP_ICMP_BUF + UIP_ICMPH_LEN, 0, 4);
    
  uip_len = UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN + UIP_IPH_LEN;
  UIP_IP_BUF->len[0] = (uint8_t)((uip_len - 40) >> 8);
  UIP_IP_BUF->len[1] = (uint8_t)((uip_len - 40) & 0x00FF);
    
  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();
    
  tcpip_ipv6_output();
}

/*------------------------------------------------------------------*/
/* Send a serial command frame to the ATMega3290 Processsor on Raven
   via serial port */
static void
send_frame(uint8_t cmd, uint8_t len, uint8_t *payload)
{
  uint8_t i;

  rs232_send(0, SOF_CHAR);    /* Start of Frame */
  rs232_send(0, len);
  rs232_send(0, cmd);
  for (i=0;i<len;i++)
    rs232_send(0,*payload++);
  rs232_send(0, EOF_CHAR);
}

/*------------------------------------------------------------------*/
static uint8_t
raven_gui_loop(process_event_t ev, process_data_t data)
{
  if(ev == tcpip_icmp6_event) {
    switch(*((uint8_t *)data)){
    case ICMP6_ECHO_REQUEST:
      /* We have received a ping request over the air. Tell 3290 */
      send_frame(REPORT_PING_BEEP, 0, 0);
      break;
    case ICMP6_ECHO_REPLY:
      /* We have received a ping reply over the air. Pass seqno to 3290 */
      send_frame(REPORT_PING, 1, &seqno);
      break;
    }
  } else {
    switch(ev){
    case SERIAL_CMD:        
      /* Check for command from serial port, execute it. */
      if (cmd.done){
        /* Execute the waiting command */
        switch (cmd.cmd){
          case SEND_PING:
            /* Send ping request over the air */
            seqno = cmd.frame[0];
            raven_ping6();
            break;
          case SEND_TEMP:
            /* Set temperature string in web server */
            sprintf(udp_data, "T%s\r\n", (char *)cmd.frame);
            uip_udp_packet_send(udp_conn, udp_data, data_len);
            break;
          default:
            break;
        }
        /* Reset command done flag. */
        cmd.done = 0;
      }
      break;
    default:
      break;
    }
  }
  return 0;
}

/*--------------------------------------------------------------------*/
/* Process an input character from serial port.  
 * ** This is called from an ISR!!
 */
int raven_lcd_serial_input(unsigned char ch)
{
  /* Parse frame,  */
  switch (cmd.ndx){
    case 0:
      /* first byte, must be 0x01 */
      cmd.done = 0;
      if (ch != 0x01){
        return 0;
      }
      break;
    case 1: 
      /* Second byte, length of payload */
      cmd.len = ch;
      break;
    case 2:
      /* Third byte, command byte */
      cmd.cmd = ch;
      break;
    default:
      /* Payload and ETX */
      if (cmd.ndx >= cmd.len+3){
        /* all done, check ETX */
        if (ch == 0x04){
          cmd.done = 1;
          process_post(&raven_lcd_process, SERIAL_CMD, 0);
        } else {
          /* Failed ETX */
          cmd.ndx = 0;
        }
      } else {
        /* Just grab and store payload */
        cmd.frame[cmd.ndx - 3] = ch;
      }
      break;
  }

  cmd.ndx++;

  return 0;
}

/*---------------------------------------------------------------------------*/
PROCESS(raven_lcd_process, "Raven LCD interface process");
PROCESS_THREAD(raven_lcd_process, ev, data)
{
  uint8_t error;

  PROCESS_BEGIN();

  /*Create a udp connection to the IPSOserver*/

  //swisscom uip_ip6addr(&udp_addr,0x2001,918,0xfff9,0,0,0,0,1); 
  //HE uip_ip6addr(&udp_addr,0x2001,0x470,0x1f12,0x5ec,0x12,0x13ff,0xfe14,0x1516);
  uip_ip6addr(&udp_addr,0x2001,0x420,0x5FFF,0x7D,0x2D0,0xB7FF,0xFE23,0xE6DB);
  
  /* set destination parameters*/
  udp_conn = udp_new(&udp_addr, UIP_HTONS(0xF0B0), NULL);
  /*set local port */
  udp_bind(udp_conn, UIP_HTONS(0xF0B0+1));
  
  if((error = icmp6_new(NULL)) == 0) {
    while(1) {
      PROCESS_YIELD();
      raven_gui_loop(ev, data);
    } 
  }
  PROCESS_END();
}
/** @} */
/** @} */
