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

/**  \addtogroup raven
 * @{
 */

/**
 *  \defgroup ravenserial Serial interface between Raven processors
 *
 *  \brief This module contains code to interface a Contiki-based
 *  project on the AVR Raven platform's ATMega1284P chip to the LCD
 *  driver chip (ATMega3290P) on the Raven.
 *
 *  \author Blake Leverett <bleverett@gmail.com>
 *
 * @{
 */
/**
 *  \file
 *  This file contains code to connect the two AVR Raven processors via a serial connection.
 *
 */

#define DEBUG 0        //Making this 1 will slightly alter command timings
#if DEBUG
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF(...)
#endif
#define DEBUGSERIAL 0  //Making this 1 will significantly alter command timings

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "webserver-nogui.h"
#include "httpd-cgi.h"
#include "raven-lcd.h"

#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <dev/watchdog.h>

static uint8_t count = 0;
static uint8_t seqno;
uip_ipaddr_t dest_addr;

#define MAX_CMD_LEN 20
static struct{
    uint8_t frame[MAX_CMD_LEN];
    uint8_t ndx;
    uint8_t len;
    uint8_t cmd;
    uint8_t done;
} cmd;

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define PING6_DATALEN 16

void rs232_send(uint8_t port, unsigned char c);

/*---------------------------------------------------------------------------*/
/* Sends a ping packet out the radio */
/* Useful for debugging so allow external calls */
void
raven_ping6(void)
{
#if UIP_CONF_IPV6_RPL||1
/* No default router, so pick on someone else */
#define PING_GOOGLE 1
seqno++;
#endif

    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 1;
    UIP_IP_BUF->flow = 0;
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
	UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;
#if PING_GOOGLE
    if (seqno==1) {
	   uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, uip_ds6_defrt_choose());   //the default router
	} else if (seqno==2) {
	   uip_ip6addr(&UIP_IP_BUF->destipaddr,0x2001,0x4860,0x800f,0x0000,0x0000,0x0000,0x0000,0x0093);  //ipv6.google.com
	} else if (seqno==3) {
	   uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, uip_ds6_defrt_choose());   //the default router
	} else {
//	   uip_ip6addr(&UIP_IP_BUF->destipaddr,0x2001,0x0420,0x5FFF,0x007D,0x02D0,0xB7FF,0xFE23,0xE6DB);  //?.cisco.com
	   uip_ip6addr(&UIP_IP_BUF->destipaddr,0x2001,0x0420,0x0000,0x0010,0x0250,0x8bff,0xfee8,0xf800);  //six.cisco.com
	}
#else
	  uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, uip_ds6_defrt_choose());    //the default router
#endif

    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
    UIP_ICMP_BUF->type = ICMP6_ECHO_REQUEST;
    UIP_ICMP_BUF->icode = 0;
    /* set identifier and sequence number to 0 */
    memset((void *)UIP_ICMP_BUF + UIP_ICMPH_LEN, 0, 4);
    /* put one byte of data */
    memset((void *)UIP_ICMP_BUF + UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN,
           count, PING6_DATALEN);


    uip_len = UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN + UIP_IPH_LEN + PING6_DATALEN;
    UIP_IP_BUF->len[0] = (uint8_t)((uip_len - 40) >> 8);
    UIP_IP_BUF->len[1] = (uint8_t)((uip_len - 40) & 0x00FF);

    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();


    tcpip_ipv6_output();
}
#if defined(__AVR_ATmega128RFA1__)
/* No raven 3290p to talk to but the lcd process still needed for ping reporting */
/* Just dummy out send_frame */
static void
send_frame(uint8_t cmd, uint8_t len, uint8_t *payload)
{
}
#else
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
#endif
char serial_char_received;
/*---------------------------------------------------------------------------*/
/* Sleep for howlong seconds, or until UART interrupt if howlong==0.
 * Uses TIMER2 with external 32768 Hz crystal to sleep in 1 second multiples.
 * TIMER2 may have already been set up for 125 ticks/second in clock.c

 *
 * Until someone figures out how to get UART to wake from powerdown,
 * a three second powersave cycle is used with exit based on any character received.

 * The system clock is adjusted to reflect the sleep time.
 */

void micro_sleep(uint8_t howlong)
{
    uint8_t saved_sreg = SREG, saved_howlong = howlong;
#if AVR_CONF_USE32KCRYSTAL
/* Save TIMER2 configuration if clock.c is using it */
    uint8_t savedTCNT2=TCNT2, savedTCCR2A=TCCR2A, savedTCCR2B = TCCR2B, savedOCR2A = OCR2A;
#endif

//    if (howlong==0) {
//        set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // UART can't wake from powerdown
//     } else {
        set_sleep_mode(SLEEP_MODE_PWR_SAVE);    // Sleep for howlong seconds
        if (howlong==0) howlong=3;              // 3*32/(32768/1024) = 3 second sleep cycle if not specified
        cli();                                  // Disable interrupts for the present
        ASSR |= (1 << AS2);                     // Set TIMER2 asyncronous from external crystal
        TCCR2A =(1<<WGM21);                     // CTC mode
        TCCR2B =((1<<CS22)|(1<<CS21)|(1<<CS20));// Prescale by 1024 = 32 ticks/sec
//      while(ASSR & (1 << TCR2BUB));           // Wait for TCNT2 write to finish.
        OCR2A = howlong*32;                     // Set TIMER2 output compare register
//      while(ASSR & (1 << OCR2AUB));           // Wait for OCR2 write to finish.
        SREG = saved_sreg;                      // Restore interrupt state.
//      UCSR(USART,B)&= ~(1<<RXCIE(USART))      // Disable the RX Complete interrupt;
//      UCSR0B|=(1<<RXCIE0);                    // Enable UART0 RX complete interrupt
//      UCSR1B|=(1<<RXCIE1);                    // Enable UART1 RX complete interrupt
//      TCNT2 = 0;                              // Reset TIMER2 timer counter value.
//      while(ASSR & (1 << TCN2UB));            // Wait for TCNT2 write to finish before entering sleep.
//      TIMSK2 |= (1 << OCIE2A);                // Enable TIMER2 output compare interrupt.
//    }

    TCNT2 = 0;                                  // Reset timer
    watchdog_stop();                            // Silence annoying distractions
    while(ASSR & (1 << TCN2UB));                // Wait for TCNT2 write to (which assures TCCR2x and OCR2A are finished!)
    TIMSK2 |= (1 << OCIE2A);                    // Enable TIMER2 output compare interrupt
    SMCR |= (1 <<  SE);                         // Enable sleep mode.
    while (1) {
//    TCNT2 = 0;                                // Cleared automatically in CTC mode
//     while(ASSR & (1 << TCN2UB));             // Wait for TCNT2 write to finish before entering sleep.
       serial_char_received=0;                  // Set when chars received by UART
       sleep_mode();                            // Sleep

       /* Adjust clock.c for the time spent sleeping */
       extern void clock_adjust_ticks(uint16_t howmany);
       clock_adjust_ticks(howlong * CLOCK_SECOND);

//     if (TIMSK2&(1<<OCIE2A)) break;           // Exit sleep if not awakened by TIMER2
       PRINTF(".");
       if (saved_howlong) break;                // Exit sleep if nonzero time specified
       PRINTF("%d",serial_char_received);
       if (serial_char_received) break;
   }

    SMCR  &= ~(1 << SE);                        //Disable sleep mode after wakeup

#if AVR_CONF_USE32KCRYSTAL
/* Restore clock.c configuration */
//  OCRSetup();
    cli();
    TCCR2A = savedTCCR2A;
    TCCR2B = savedTCCR2B;
    OCR2A  = savedOCR2A;
    TCNT2  = savedTCNT2;
    sei();
#else
    TIMSK2 &= ~(1 << OCIE2A);                   //Disable TIMER2 interrupt
#endif

    watchdog_start();
}
#if !AVR_CONF_USE32KCRYSTAL
/*---------------------------------------------------------------------------*/
/* TIMER2 Interrupt service */

ISR(TIMER2_COMPA_vect)
{
//    TIMSK2 &= ~(1 << OCIE2A);       //Just one interrupt needed for waking
}
#endif /* !AVR_CONF_USE32KCRYSTAL */

#if DEBUGSERIAL
uint8_t serialcount;
char dbuf[30];
#endif

/*---------------------------------------------------------------------------*/
extern uint16_t ledtimer;
static uint8_t
raven_gui_loop(process_event_t ev, process_data_t data)
{
    uint8_t i,activeconnections,radio_state;

// PRINTF("\nevent %d ",ev);
#if DEBUGSERIAL
    printf_P(PSTR("Buffer [%d]="),serialcount);
    serialcount=0;
    for (i=0;i<30;i++) {
       printf_P(PSTR(" %d"),dbuf[i]);
       dbuf[i]=0;
    }
#endif
    if(ev == tcpip_icmp6_event) switch(*((uint8_t *)data)) {

//   case ICMP6_NS:
        /*Tell the 3290 we are being solicited. */
//       send_frame(REPORT_NS,...);
//       break;  //fall through for beep
//   case ICMP6_RA:
        /*Tell the 3290 we have a router. */
//       send_frame(REPORT_NA,...);
//       break;  //fall through for beep
    case ICMP6_ECHO_REQUEST:
        /* We have received a ping request over the air. Tell the 3290 */
 //       send_frame(REPORT_PING_BEEP, 0, 0);
#if RF230BB_CONF_LEDONPORTE1
          PORTE|=(1<<PE1);ledtimer=1000; //turn on led, set counter for turnoff
#endif
        break;
    case ICMP6_ECHO_REPLY:
        /* We have received a ping reply over the air.  Send frame back to 3290 */
 //       send_frame(REPORT_PING, 1, &seqno);
        break;

    } else switch (ev) {
     case SERIAL_CMD:
        /* Check for command from serial port, execute it. */
        /* Note cmd frame is written in an interrupt - delays here can cause overwriting by next command */
        PRINTF("\nCommand %d length %d done %d",cmd.cmd,cmd.len,cmd.done);
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
 //               web_set_temp((char *)cmd.frame);
                break;
            case SEND_ADC2:
                /* Set ext voltage string in web server */
  //              web_set_voltage((char *)cmd.frame);
                break;
            case SEND_SLEEP:
                /* Sleep radio and 1284p. */
                if (cmd.frame[0]==0) {  //Time to sleep in seconds
                /* Unconditional sleep. Don't wake until a serial interrupt. */
                } else {
                /* Sleep specified number of seconds (3290p "DOZE" mode) */
                /* It sleeps a bit longer so we will be always be awake for the next sleep command. */
                /* Only sleep this cycle if no active TCP/IP connections */
                   activeconnections=0;
                   for(i = 0; i < UIP_CONNS; ++i) {
                      if((uip_conns[i].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) activeconnections++;
                   }
                   if (activeconnections) {
                     PRINTF("\nWaiting for %d connections",activeconnections);
                     break;
                   }
                }
                radio_state = NETSTACK_RADIO.off();
                PRINTF ("\nsleep %d radio state %d...",cmd.frame[0],radio_state);

                /*Sleep for specified time*/
                PRINTF("\nSleeping...");
                micro_sleep(cmd.frame[0]);

                radio_state = NETSTACK_RADIO.on();
                if (radio_state > 0) {
                   PRINTF("Awake!");
                } else {
                    PRINTF("Radio wake error %d\n",radio_state);
                }
                break;
            case SEND_WAKE:
               /* 3290p requests return message showing awake status */
                send_frame(REPORT_WAKE, 0, 0);
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
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Process an input character from serial port.
 *  ** This is called from an ISR!!
*/

int raven_lcd_serial_input(unsigned char ch)
{
    /* Tell sleep routine if a  reception occurred */
    /* Random nulls occur for some reason, so ignore those */
    if (ch) serial_char_received++;
#if DEBUGSERIAL
    if (serialcount<25) dbuf[serialcount]=ch;
    serialcount++;
#endif
    /* Don't overwrite an unprocessed command */
//    if (cmd.done) return 0;

    /* Parse frame,  */
    switch (cmd.ndx){
    case 0:
        /* first byte, must be 0x01 */
        if (ch == 0x01){
//            cmd.done = false;
        } else {
#if DEBUGSERIAL
            dbuf[25]++;
#endif
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
        if (cmd.ndx >= (MAX_CMD_LEN+3)) {  //buffer overflow!
            cmd.ndx=0;
#if DEBUGSERIAL
            dbuf[26]++;
#endif
            return 0;
        }
        if (cmd.ndx >= cmd.len+3){
            /* all done, check ETX */
            if (ch == 0x04){
                cmd.done = 1;
#if DEBUGSERIAL
                dbuf[27]++;
#endif
                process_post(&raven_lcd_process, SERIAL_CMD, 0);
            } else {
                /* Failed ETX */
#if DEBUGSERIAL
                dbuf[28]++;
#endif
            }
            cmd.ndx=0;             //set up for next command
            return 0;
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
void
raven_lcd_show_text(char *text) {
    uint8_t textlen=strlen(text)+1;
    if (textlen > MAX_CMD_LEN) textlen=MAX_CMD_LEN;
    send_frame(REPORT_TEXT_MSG, textlen, (uint8_t *) text);
}

#if WEBSERVER
static void
lcd_show_servername(void) {

//extern uint8_t mac_address[8];     //These are defined in httpd-fsdata.c via makefsdata.h
extern uint8_t server_name[16];
//extern uint8_t domain_name[30];
char buf[sizeof(server_name)+1];
    eeprom_read_block (buf,server_name, sizeof(server_name));
    buf[sizeof(server_name)]=0;
    raven_lcd_show_text(buf);  //must fit in all the buffers or it will be truncated!
}
#endif
/*---------------------------------------------------------------------------*/
PROCESS(raven_lcd_process, "Raven LCD interface process");
PROCESS_THREAD(raven_lcd_process, ev, data)
{

  PROCESS_BEGIN();

#if WEBSERVER
  lcd_show_servername();
#endif

  /* Get ICMP6 callbacks from uip6 stack, perform 3290p action on pings, responses, etc. */
  if(icmp6_new(NULL) == 0) {

    while(1) {
      PROCESS_YIELD();
//      if (ev != ?)      //trap frequent strobes?
        raven_gui_loop(ev, data);
    }
  }
  PROCESS_END();
}

/** @} */
/** @} */
