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
 * $Id: raven-lcd.c,v 1.7 2010/03/17 20:19:33 dak664 Exp $
*/

/**
 *  \brief This module contains code to interface a Contiki-based
 *  project on the AVR Raven platform's ATMega1284P chip to the LCD
 *  driver chip (ATMega3290P) on the Raven.
 *  
 *  \author Blake Leverett <bleverett@gmail.com>
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
 *  This file contains code to connect the two AVR Raven processors via a serial connection.
 *
 */



#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "webserver-nogui.h"
#include "httpd-cgi.h"

//#include "frame.h"
#include "mac.h"

#include "raven-lcd.h"

#include <string.h>
#include <stdio.h>


static u8_t count = 0;
static u8_t seqno;
uip_ipaddr_t dest_addr;

#define cmd_len 8
static struct{
    u8_t frame[cmd_len];
    u8_t ndx;
    u8_t len;
    u8_t cmd;
    u8_t done;
} cmd;

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define PING6_DATALEN 16

#define CMD_TEMP 0x80
#define CMD_PING 0x81
#define CMD_ADC2 0x82

#define SOF_CHAR 1
#define EOF_CHAR 4

void rs232_send(uint8_t port, unsigned char c);


/*---------------------------------------------------------------------------*/
/* Sends a ping packet out the radio */
/* Useful for debugging so allow external calls */
void
raven_ping6(void)
{
    /* ping the router */

#if THEOLDWAY
    // Setup destination address.
	static u16_t addr[8];
    struct uip_nd6_defrouter *defrouter;
    uint8_t i,tmp;

    defrouter = uip_nd6_choose_defrouter();
    /* Get address from defrouter struct */
    memcpy(addr, defrouter->nb->ipaddr.u8, 16);
    /* Swap the bytes in the address array */
    for (i=0;i<8;i++)
    {
        tmp = addr[i] & 0xff;
        addr[i] >>= 8;
        addr[i] |= tmp << 8;
    }
    uip_ip6addr(&dest_addr, addr[0], addr[1],addr[2],
                addr[3],addr[4],addr[5],addr[6],addr[7]);
  
    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 1;
    UIP_IP_BUF->flow = 0;
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    UIP_IP_BUF->ttl = uip_netif_physical_if.cur_hop_limit;
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &dest_addr);
    uip_netif_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);

 #else
    /* Get address from default router */
    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 1;
    UIP_IP_BUF->flow = 0;
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
	UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;
	uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, uip_ds6_defrt_choose());
	uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
#endif  
 
    UIP_ICMP_BUF->type = ICMP6_ECHO_REQUEST;
    UIP_ICMP_BUF->icode = 0;
    /* set identifier and sequence number to 0 */
    memset((void *)UIP_ICMP_BUF + UIP_ICMPH_LEN, 0, 4);
    /* put one byte of data */
    memset((void *)UIP_ICMP_BUF + UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN,
           count, PING6_DATALEN);
     
    
    uip_len = UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN + UIP_IPH_LEN + PING6_DATALEN;
    UIP_IP_BUF->len[0] = (u8_t)((uip_len - 40) >> 8);
    UIP_IP_BUF->len[1] = (u8_t)((uip_len - 40) & 0x00FF);
    
    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();
   
    
    tcpip_ipv6_output();
}

/*---------------------------------------------------------------------------*/
/* Send a serial command frame to the ATMega3290 Processsor on Raven via serial port */
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

/*---------------------------------------------------------------------------*/
static u8_t
raven_gui_loop(process_event_t ev, process_data_t data)
{
  if(ev == tcpip_icmp6_event) {
    switch(*((uint8_t *)data)) {
    case ICMP6_ECHO_REQUEST:
        /* We have received a ping request over the air. Send frame back to 3290 */
        send_frame(PING_REQUEST, 0, 0);
        break;
    case ICMP6_ECHO_REPLY:
        /* We have received a ping reply over the air.  Send frame back to 3290 */
        send_frame(PING_REPLY, 1, &seqno);
        break;
    }
  } else {
    switch(ev){
    case SERIAL_CMD:        
        /* Check for command from serial port, execute it. */
        if (cmd.done){
            /* Execute the waiting command */
            switch (cmd.cmd){
            case CMD_PING:
                /* Send ping request over the air */
                seqno = cmd.frame[0];
                raven_ping6();
                break;
            case CMD_TEMP:
                /* Set temperature string in web server */
                web_set_temp((char *)cmd.frame);
                break;
            case CMD_ADC2:
                /* Set ext voltage string in web server */
                web_set_voltage((char *)cmd.frame);
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

/*---------------------------------------------------------------------------*/
/* Process an input character from serial port.  
 *  ** This is called from an ISR!!
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
  u8_t error;

  PROCESS_BEGIN();
  
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
