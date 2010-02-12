/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 *
 * $Id: rf230bb.c,v 1.2 2010/02/12 16:41:02 dak664 Exp $
*/

/**
 *  \brief This module contains radio driver code for the Atmel
 *  AT86RF230. It is modified to use the contiki core MAC layer.
 *
 *  \author Blake Leverett <bleverett@gmail.com>
 *          Mike Vidales <mavida404@gmail.com>
 *          Eric Gnoske <egnoske@gmail.com>
 *          David Kopf <dak664@embarqmail.com>
 *
*/

/**  \addtogroup wireless
 * @{
 */

/**
 *  \defgroup radiorf230 RF230 interface
 * @{
 */
/**
 *  \file
 *  This file contains "barebones" radio driver code for use with the
 *  contiki core MAC layer.
 *
 */


#include <stdio.h>
#include <string.h>

#include "contiki.h"

//#if defined(__AVR__)
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
//#elif defined(__MSP430__)
//#include <io.h>
//#endif

#include "dev/leds.h"
#include "dev/spi.h"
#include "rf230bb.h"
#include "hal.h"
//#include "frame.h"
#include "radio.h"

#include "net/rime/packetbuf.h"
#include "net/rime/rimestats.h"

#include "sys/timetable.h"

#define WITH_SEND_CCA 0

/* See clock.c and httpd-cgi.c for RADIOSTATS code */
uint8_t RF230_radio_on;
#define RADIOSTATS 0
#if RADIOSTATS
uint8_t RF230_rsigsi;
uint16_t RF230_sendpackets,RF230_receivepackets,RF230_sendfail,RF230_receivefail;
#endif

#if RF230_CONF_TIMESTAMPS
#include "net/rime/timesynch.h"
#define TIMESTAMP_LEN 3
#else /* RF230_CONF_TIMESTAMPS */
#define TIMESTAMP_LEN 0
#endif /* RF230_CONF_TIMESTAMPS */
#define FOOTER_LEN 2

#ifndef RF230_CONF_CHECKSUM
#define RF230_CONF_CHECKSUM 0
#endif /* RF230_CONF_CHECKSUM */

#if RF230_CONF_CHECKSUM
#include "lib/crc16.h"
#define CHECKSUM_LEN 2
#else
#define CHECKSUM_LEN 0
#endif /* RF230_CONF_CHECKSUM */

#define AUX_LEN (CHECKSUM_LEN + TIMESTAMP_LEN + FOOTER_LEN)

struct timestamp {
  uint16_t time;
  uint8_t authority_level;
};


#define FOOTER1_CRC_OK      0x80
#define FOOTER1_CORRELATION 0x7f

#define DEBUG 0
#if DEBUG
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF(...) do {} while (0)
#endif

/* XXX hack: these will be made as Chameleon packet attributes */
rtimer_clock_t rf230_time_of_arrival, rf230_time_of_departure;

int rf230_authority_level_of_sender;

#if RF230_CONF_TIMESTAMPS
static rtimer_clock_t setup_time_for_transmission;
static unsigned long total_time_for_transmission, total_transmission_len;
static int num_transmissions;
#endif /* RF230_CONF_TIMESTAMPS */

/* RF230 hardware delay times, from datasheet */
typedef enum{
    TIME_TO_ENTER_P_ON               = 510, /**<  Transition time from VCC is applied to P_ON. */
    TIME_P_ON_TO_TRX_OFF             = 510, /**<  Transition time from P_ON to TRX_OFF. */
    TIME_SLEEP_TO_TRX_OFF            = 880, /**<  Transition time from SLEEP to TRX_OFF. */
    TIME_RESET                       = 6,   /**<  Time to hold the RST pin low during reset */
    TIME_ED_MEASUREMENT              = 140, /**<  Time it takes to do a ED measurement. */
    TIME_CCA                         = 140, /**<  Time it takes to do a CCA. */
    TIME_PLL_LOCK                    = 150, /**<  Maximum time it should take for the PLL to lock. */
    TIME_FTN_TUNING                  = 25,  /**<  Maximum time it should take to do the filter tuning. */
    TIME_NOCLK_TO_WAKE               = 6,   /**<  Transition time from *_NOCLK to being awake. */
    TIME_CMD_FORCE_TRX_OFF           = 1,   /**<  Time it takes to execute the FORCE_TRX_OFF command. */
    TIME_TRX_OFF_TO_PLL_ACTIVE       = 180, /**<  Transition time from TRX_OFF to: RX_ON, PLL_ON, TX_ARET_ON and RX_AACK_ON. */
    TIME_STATE_TRANSITION_PLL_ACTIVE = 1,   /**<  Transition time from PLL active state to another. */
}radio_trx_timing_t;

/*---------------------------------------------------------------------------*/
PROCESS(rf230_process, "RF230 receiver");
/*---------------------------------------------------------------------------*/

int rf230_send(const void *data, unsigned short len);
int rf230_read(void *buf, unsigned short bufsize);
void rf230_set_receiver(void (* recv)(const struct radio_driver *d));
int rf230_on(void);
int rf230_off(void);

const struct radio_driver rf230_driver =
  {
    rf230_send,
    rf230_read,
    rf230_set_receiver,
    rf230_on,
    rf230_off,
  };

static void (* receiver_callback)(const struct radio_driver *);

//signed char rf230_last_rssi;
//uint8_t rf230_last_correlation;
//static uint8_t rssi_val;
uint8_t rx_mode;
/* Radio stuff in network byte order. */
//static uint16_t pan_id;

//static int channel;


/*----------------------------------------------------------------------------*/
/** \brief  This function return the Radio Transceivers current state.
 *
 *  \retval     P_ON               When the external supply voltage (VDD) is
 *                                 first supplied to the transceiver IC, the
 *                                 system is in the P_ON (Poweron) mode.
 *  \retval     BUSY_RX            The radio transceiver is busy receiving a
 *                                 frame.
 *  \retval     BUSY_TX            The radio transceiver is busy transmitting a
 *                                 frame.
 *  \retval     RX_ON              The RX_ON mode enables the analog and digital
 *                                 receiver blocks and the PLL frequency
 *                                 synthesizer.
 *  \retval     TRX_OFF            In this mode, the SPI module and crystal
 *                                 oscillator are active.
 *  \retval     PLL_ON             Entering the PLL_ON mode from TRX_OFF will
 *                                 first enable the analog voltage regulator. The
 *                                 transceiver is ready to transmit a frame.
 *  \retval     BUSY_RX_AACK       The radio was in RX_AACK_ON mode and received
 *                                 the Start of Frame Delimiter (SFD). State
 *                                 transition to BUSY_RX_AACK is done if the SFD
 *                                 is valid.
 *  \retval     BUSY_TX_ARET       The radio transceiver is busy handling the
 *                                 auto retry mechanism.
 *  \retval     RX_AACK_ON         The auto acknowledge mode of the radio is
 *                                 enabled and it is waiting for an incomming
 *                                 frame.
 *  \retval     TX_ARET_ON         The auto retry mechanism is enabled and the
 *                                 radio transceiver is waiting for the user to
 *                                 send the TX_START command.
 *  \retval     RX_ON_NOCLK        The radio transceiver is listening for
 *                                 incomming frames, but the CLKM is disabled so
 *                                 that the controller could be sleeping.
 *                                 However, this is only true if the controller
 *                                 is run from the clock output of the radio.
 *  \retval     RX_AACK_ON_NOCLK   Same as the RX_ON_NOCLK state, but with the
 *                                 auto acknowledge module turned on.
 *  \retval     BUSY_RX_AACK_NOCLK Same as BUSY_RX_AACK, but the controller
 *                                 could be sleeping since the CLKM pin is
 *                                 disabled.
 *  \retval     STATE_TRANSITION   The radio transceiver's state machine is in
 *                                 transition between two states.
 */
uint8_t
radio_get_trx_state(void)
{
    return hal_subregister_read(SR_TRX_STATUS);
}

/*----------------------------------------------------------------------------*/
/** \brief  This function checks if the radio transceiver is sleeping.
 *
 *  \retval     true    The radio transceiver is in SLEEP or one of the *_NOCLK
 *                      states.
 *  \retval     false   The radio transceiver is not sleeping.
 */
bool radio_is_sleeping(void)
{
    bool sleeping = false;

    /* The radio transceiver will be at SLEEP or one of the *_NOCLK states only if */
    /* the SLP_TR pin is high. */
    if (hal_get_slptr() != 0){
        sleeping = true;
    }

    return sleeping;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will reset the state machine (to TRX_OFF) from any of
 *          its states, except for the SLEEP state.
 */
void
radio_reset_state_machine(void)
{
    hal_set_slptr_low();
    delay_us(TIME_NOCLK_TO_WAKE);
    hal_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
    delay_us(TIME_CMD_FORCE_TRX_OFF);
}
/*----------------------------------------------------------------------------*/
/** \brief  This function will change the current state of the radio
 *          transceiver's internal state machine.
 *
 *  \param     new_state        Here is a list of possible states:
 *             - RX_ON        Requested transition to RX_ON state.
 *             - TRX_OFF      Requested transition to TRX_OFF state.
 *             - PLL_ON       Requested transition to PLL_ON state.
 *             - RX_AACK_ON   Requested transition to RX_AACK_ON state.
 *             - TX_ARET_ON   Requested transition to TX_ARET_ON state.
 *
 *  \retval    RADIO_SUCCESS          Requested state transition completed
 *                                  successfully.
 *  \retval    RADIO_INVALID_ARGUMENT Supplied function parameter out of bounds.
 *  \retval    RADIO_WRONG_STATE      Illegal state to do transition from.
 *  \retval    RADIO_BUSY_STATE       The radio transceiver is busy.
 *  \retval    RADIO_TIMED_OUT        The state transition could not be completed
 *                                  within resonable time.
 */
radio_status_t
radio_set_trx_state(uint8_t new_state)
{
    uint8_t original_state;

    /*Check function paramter and current state of the radio transceiver.*/
    if (!((new_state == TRX_OFF)    ||
          (new_state == RX_ON)      ||
          (new_state == PLL_ON)     ||
          (new_state == RX_AACK_ON) ||
          (new_state == TX_ARET_ON))){
        return RADIO_INVALID_ARGUMENT;
    }

    if (radio_is_sleeping() == true){
        return RADIO_WRONG_STATE;
    }

    // Wait for radio to finish previous operation
    for(;;)
    {
        original_state = radio_get_trx_state();
        if (original_state != BUSY_TX_ARET &&
            original_state != BUSY_RX_AACK &&
            original_state != BUSY_RX && 
            original_state != BUSY_TX)
            break;
    }

    if (new_state == original_state){
        return RADIO_SUCCESS;
    }


    /* At this point it is clear that the requested new_state is: */
    /* TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON or TX_ARET_ON. */

    /* The radio transceiver can be in one of the following states: */
    /* TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON, TX_ARET_ON. */
    if(new_state == TRX_OFF){
        radio_reset_state_machine(); /* Go to TRX_OFF from any state. */
    } else {
        /* It is not allowed to go from RX_AACK_ON or TX_AACK_ON and directly to */
        /* TX_AACK_ON or RX_AACK_ON respectively. Need to go via RX_ON or PLL_ON. */
        if ((new_state == TX_ARET_ON) &&
            (original_state == RX_AACK_ON)){
            /* First do intermediate state transition to PLL_ON, then to TX_ARET_ON. */
            /* The final state transition to TX_ARET_ON is handled after the if-else if. */
            hal_subregister_write(SR_TRX_CMD, PLL_ON);
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        } else if ((new_state == RX_AACK_ON) &&
                 (original_state == TX_ARET_ON)){
            /* First do intermediate state transition to RX_ON, then to RX_AACK_ON. */
            /* The final state transition to RX_AACK_ON is handled after the if-else if. */
            hal_subregister_write(SR_TRX_CMD, RX_ON);
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        }

        /* Any other state transition can be done directly. */
        hal_subregister_write(SR_TRX_CMD, new_state);

        /* When the PLL is active most states can be reached in 1us. However, from */
        /* TRX_OFF the PLL needs time to activate. */
        if (original_state == TRX_OFF){
            delay_us(TIME_TRX_OFF_TO_PLL_ACTIVE);
        } else {
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        }
    } /*  end: if(new_state == TRX_OFF) ... */

    /*Verify state transition.*/
    radio_status_t set_state_status = RADIO_TIMED_OUT;

    if (radio_get_trx_state() == new_state){
        set_state_status = RADIO_SUCCESS;
        /*  set rx_mode flag based on mode we're changing to */
        if (new_state == RX_ON ||
            new_state == RX_AACK_ON){
            rx_mode = true;
        } else {
            rx_mode = false;
    }
    }

    return set_state_status;
}

/*---------------------------------------------------------------------------*/
void
rf230_waitidle(void)
{
//	PRINTF("rf230_waitidle");
	uint8_t radio_state;

    for(;;)
    {
        radio_state = hal_subregister_read(SR_TRX_STATUS);
        if (radio_state != BUSY_TX_ARET &&
            radio_state != BUSY_RX_AACK &&
            radio_state != BUSY_RX && 
            radio_state != BUSY_TX)
            break;
       PRINTF(".");
    }
}

/*---------------------------------------------------------------------------*/
static uint8_t locked, lock_on, lock_off;

static void
on(void)
{
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  PRINTF("rf230 internal on\n");
  RF230_radio_on = 1;

  hal_set_slptr_low();
//radio_is_waking=1;//can test this before tx instead of delaying
  delay_us(TIME_SLEEP_TO_TRX_OFF);
  delay_us(TIME_SLEEP_TO_TRX_OFF);//extra delay for now

    radio_set_trx_state(RX_AACK_ON);
// flushrx();
}
static void
off(void)
{
  PRINTF("rf230 internal off\n");
  RF230_radio_on = 0;
  
  /* Wait for transmission to end before turning radio off. */
  rf230_waitidle(); 

  /* Force the device into TRX_OFF. */   
  radio_reset_state_machine();
   
  /* Sleep Radio */
  hal_set_slptr_high();

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
}
/*---------------------------------------------------------------------------*/
#define GET_LOCK() locked = 1
static void RELEASE_LOCK(void) {
  if(lock_on) {
    on();
    lock_on = 0;
  }
  if(lock_off) {
    off();
    lock_off = 0;
  }
  locked = 0;
}
/*---------------------------------------------------------------------------*/
void
rf230_set_receiver(void (* recv)(const struct radio_driver *))
{
//PRINTF("rf230_set receiver\n");
  receiver_callback = recv;
}

/*---------------------------------------------------------------------------*/
int
rf230_off(void)
{
// PRINTF("rf230_off\n");	
  /* Don't do anything if we are already turned off. */
  if(RF230_radio_on == 0) {
    return 1;
  }

  /* If we are called when the driver is locked, we indicate that the
     radio should be turned off when the lock is unlocked. */
  if(locked) {
    lock_off = 1;
    return 1;
  }
  off();

  return 1;
}
/*---------------------------------------------------------------------------*/
int
rf230_on(void)
{
//PRINTF("rf230_on\n");
  if(RF230_radio_on) {
    return 1;
  }
  if(locked) {
    lock_on = 1;
    return 1;
  }
  on();

  return 1;
}
/*---------------------------------------------------------------------------*/
int
rf230_get_channel(void)
{
    return hal_subregister_read(SR_CHANNEL);
//	return channel;
}
/*---------------------------------------------------------------------------*/
void
rf230_set_channel(int c)
{
 /* Wait for any transmission to end. */
  rf230_waitidle();
	
//channel=c;
  hal_subregister_write(SR_CHANNEL, c);

}
/*---------------------------------------------------------------------------*/
void
rf230_set_pan_addr(uint16_t pan,uint16_t addr,uint8_t *ieee_addr)
{
  PRINTF("rf230: PAN=%x Short Addr=%x\n",pan,addr);
  
  uint8_t abyte;
  abyte = pan & 0xFF;
  hal_register_write(RG_PAN_ID_0,abyte);
  abyte = (pan >> 8*1) & 0xFF;
  hal_register_write(RG_PAN_ID_1, abyte);

  abyte = addr & 0xFF;
  hal_register_write(RG_SHORT_ADDR_0, abyte);
  abyte = (addr >> 8*1) & 0xFF;
  hal_register_write(RG_SHORT_ADDR_1, abyte);  

  if (ieee_addr != NULL) {
	PRINTF("MAC=%x",*ieee_addr);
	hal_register_write(RG_IEEE_ADDR_7, *ieee_addr++);
	PRINTF(":%x",*ieee_addr);
 	hal_register_write(RG_IEEE_ADDR_6, *ieee_addr++);
 	PRINTF(":%x",*ieee_addr);
 	hal_register_write(RG_IEEE_ADDR_5, *ieee_addr++);
 	PRINTF(":%x",*ieee_addr);
    hal_register_write(RG_IEEE_ADDR_4, *ieee_addr++);
 	PRINTF(":%x",*ieee_addr);
 	hal_register_write(RG_IEEE_ADDR_3, *ieee_addr++);
 	PRINTF(":%x",*ieee_addr);
 	hal_register_write(RG_IEEE_ADDR_2, *ieee_addr++);
 	PRINTF(":%x",*ieee_addr);
 	hal_register_write(RG_IEEE_ADDR_1, *ieee_addr++);
 	PRINTF(":%x",*ieee_addr);
 	hal_register_write(RG_IEEE_ADDR_0, *ieee_addr);
 	PRINTF("\n");
  }
 
}

/*---------------------------------------------------------------------------*/
/* Process to handle input packets
 * Receive interrupts cause this process to be polled
 * It calls the core MAC layer which calls rf230_read to get the packet
*/
PROCESS_THREAD(rf230_process, ev, data)
{
  PROCESS_BEGIN();
  
  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    
#if RF230_TIMETABLE_PROFILING
    TIMETABLE_TIMESTAMP(rf230_timetable, "poll");
#endif /* RF230_TIMETABLE_PROFILING */
        
    if(receiver_callback != NULL) {
      receiver_callback(&rf230_driver);
#if RF230_TIMETABLE_PROFILING
      TIMETABLE_TIMESTAMP(rf230_timetable, "end");
      timetable_aggregate_compute_detailed(&aggregate_time,
					   &rf230_timetable);
      timetable_clear(&rf230_timetable);
#endif /* RF230_TIMETABLE_PROFILING */
    } else {
      PRINTF("rf230_process not receiving function\n");
//    flushrx();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/*
 * This routine is called by the radio receive interrupt in hal.c
 * It just sets the poll flag for the rf230 process.
 */
#if RF230_CONF_TIMESTAMPS
static volatile rtimer_clock_t interrupt_time;
static volatile int interrupt_time_set;
#endif /* RF230_CONF_TIMESTAMPS */
#if RF230_TIMETABLE_PROFILING
#define rf230_timetable_size 16
TIMETABLE(rf230_timetable);
TIMETABLE_AGGREGATE(aggregate_time, 10);
#endif /* RF230_TIMETABLE_PROFILING */
void
rf230_interrupt(void)
{
#if RF230_CONF_TIMESTAMPS
  interrupt_time = timesynch_time();
  interrupt_time_set = 1;
#endif /* RF230_CONF_TIMESTAMPS */

  process_poll(&rf230_process);
#if RF230_TIMETABLE_PROFILING
  timetable_clear(&rf230_timetable);
  TIMETABLE_TIMESTAMP(rf230_timetable, "interrupt");
#endif /* RF230_TIMETABLE_PROFILING */
  return;
}
/* The frame is buffered to rxframe in the interrupt routine in hal.c */
  hal_rx_frame_t rxframe;
/*---------------------------------------------------------------------------*/
int
rf230_read(void *buf, unsigned short bufsize)
{

  uint8_t *framep;
 // uint8_t footer[2];
  uint8_t len;
#if RF230_CONF_CHECKSUM
  uint16_t checksum;
#endif /* RF230_CONF_CHECKSUM */
#if RF230_CONF_TIMESTAMPS
  struct timestamp t;
#endif /* RF230_CONF_TIMESTAMPS */

  PRINTF("rf230_read: %u bytes lqi %u crc %u\n",rxframe.length,rxframe.lqi,rxframe.crc);
#if DEBUG>1
  for (len=0;len<rxframe.length;len++) PRINTF(" %x",rxframe.data[len]);PRINTF("\n");
#endif
  if (rxframe.length==0) {
    return 0;
  }

#if RF230_CONF_TIMESTAMPS
bomb
  if(interrupt_time_set) {
    rf230_time_of_arrival = interrupt_time;
    interrupt_time_set = 0;
  } else {
    rf230_time_of_arrival = 0;
  }
  rf230_time_of_departure = 0;
#endif /* RF230_CONF_TIMESTAMPS */
  GET_LOCK();
//  if(rxframe.length > RF230_MAX_PACKET_LEN) {
//    // Oops, we must be out of sync.
//  flushrx();
//    RIMESTATS_ADD(badsynch);
//    RELEASE_LOCK();
//    return 0;
//  }

//hal returns two extra bytes containing the checksum
//below works because auxlen is 2
  len = rxframe.length;
  if(len <= AUX_LEN) {
 // flushrx();
    RIMESTATS_ADD(tooshort);
    RELEASE_LOCK();
    return 0;
  }
  
  if(len - AUX_LEN > bufsize) {
//  flushrx();
    RIMESTATS_ADD(toolong);
    RELEASE_LOCK();
    return 0;
  }
  /* Transfer the frame, stripping the checksum */
  framep=&(rxframe.data[0]);
  memcpy(buf,framep,len-2);
  /* Clear the length field to allow buffering of the next packet */
  rxframe.length=0;
 // framep+=len-AUX_LEN+2;

#if RADIOSTATS
  RF230_receivepackets++;
#endif

#if RF230_CONF_CHECKSUM
bomb
  memcpy(&checksum,framep,CHECKSUM_LEN);
  framep+=CHECKSUM_LEN;
#endif /* RF230_CONF_CHECKSUM */
#if RF230_CONF_TIMESTAMPS
bomb
  memcpy(&t,framep,TIMESTAMP_LEN);
  framep+=TIMESTAMP_LEN;
#endif /* RF230_CONF_TIMESTAMPS */
//  memcpy(&footer,framep,FOOTER_LEN);
 
#if RF230_CONF_CHECKSUM
bomb
  if(checksum != crc16_data(buf, len - AUX_LEN, 0)) {
    PRINTF("rf230: checksum failed 0x%04x != 0x%04x\n",
	   checksum, crc16_data(buf, len - AUX_LEN, 0));
  }
  
  if(footer[1] & FOOTER1_CRC_OK &&
     checksum == crc16_data(buf, len - AUX_LEN, 0)) {
#else
  if (rxframe.crc) {
#endif /* RF230_CONF_CHECKSUM */

/*
    packetbuf_copyfrom(parsed_frame->payload, parsed_frame->payload_length);
    packetbuf_set_datalen(parsed_frame->payload_length);
  
  	memcpy(dest_reversed, (uint8_t *)parsed_frame->dest_addr, UIP_LLADDR_LEN);
	memcpy(src_reversed, (uint8_t *)parsed_frame->src_addr, UIP_LLADDR_LEN);
  
	//Change addresses to expected byte order 
	byte_reverse((uint8_t *)dest_reversed, UIP_LLADDR_LEN);
	byte_reverse((uint8_t *)src_reversed, UIP_LLADDR_LEN);
  
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, (const rimeaddr_t *)dest_reversed);
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, (const rimeaddr_t *)src_reversed);

  */  
 #if RADIOSTATS
    RF230_rsigsi=hal_subregister_read( SR_RSSI );
#endif      
    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, hal_subregister_read( SR_RSSI ));
    packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, rxframe.lqi);
    
    RIMESTATS_ADD(llrx);
    
#if RF230_CONF_TIMESTAMPS
bomb
    rf230_time_of_departure =
      t.time +
      setup_time_for_transmission +
      (total_time_for_transmission * (len - 2)) / total_transmission_len;
  
    rf230_authority_level_of_sender = t.authority_level;

    packetbuf_set_attr(PACKETBUF_ATTR_TIMESTAMP, t.time);
#endif /* RF230_CONF_TIMESTAMPS */
  
  } else {
    PRINTF("rf230: Bad CRC\n");

#if RADIOSTATS
    RF230_receivefail++;
#endif

    RIMESTATS_ADD(badcrc);
    len = AUX_LEN;
  }
// if (?)
     /* Another packet has been received and needs attention. */
//    process_poll(&rf230_process);
//  }
  
  RELEASE_LOCK();
  
  if(len < AUX_LEN) {
    return 0;
  }

  return len - AUX_LEN;
}
/*---------------------------------------------------------------------------*/
void
rf230_set_txpower(uint8_t power)
{
  if (power > TX_PWR_17_2DBM){
    power=TX_PWR_17_2DBM;
  }
  if (radio_is_sleeping() ==true) {
	PRINTF("rf230_set_txpower:Sleeping");
  } else {
    hal_subregister_write(SR_TX_PWR, power);
  }

}
/*---------------------------------------------------------------------------*/
int
rf230_get_txpower(void)
{
  if (radio_is_sleeping() ==true) {
	PRINTF("rf230_get_txpower:Sleeping");
	return 0;
  } else {
    return hal_subregister_read(SR_TX_PWR);
  }
}
/*---------------------------------------------------------------------------*/
int
rf230_rssi(void)
{
  int rssi;
  int radio_was_off = 0;
  
  /*The RSSI measurement should only be done in RX_ON or BUSY_RX.*/
  if(!RF230_radio_on) {
    radio_was_off = 1;
    rf230_on();
  }

  rssi = (int)((signed char)hal_subregister_read(SR_RSSI));

  if(radio_was_off) {
    rf230_off();
  }
  return rssi;
}
/*---------------------------------------------------------------------------*/
int
rf230_send(const void *payload, unsigned short payload_len)
{
//  int i;
  uint8_t total_len,buffer[RF230_MAX_TX_FRAME_LENGTH],*pbuf;
#if RF230_CONF_TIMESTAMPS
  struct timestamp timestamp;
#endif /* RF230_CONF_TIMESTAMPS */
#if RF230_CONF_CHECKSUM
  uint16_t checksum;
#endif /* RF230_CONF_CHECKSUM */

#if RADIOSTATS
  RF230_sendpackets++;
#endif

  GET_LOCK();

  if(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) > 0) {
    rf230_set_txpower(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) - 1);
  } else {
    rf230_set_txpower(TX_PWR_17_2DBM);
  }
  
  RIMESTATS_ADD(lltx);

#if RF230_CONF_CHECKSUM
  checksum = crc16_data(payload, payload_len, 0);
#endif /* RF230_CONF_CHECKSUM */
  total_len = payload_len + AUX_LEN;
  /*Check function parameters and current state.*/
  if (total_len > RF230_MAX_TX_FRAME_LENGTH){
#if RADIOSTATS
    RF230_sendfail++;
#endif   
    return -1;
  }
  pbuf=&buffer[0];
  memcpy(pbuf,payload,payload_len);
  pbuf+=payload_len;
  
#if RF230_CONF_CHECKSUM
  memcpy(pbuf,&checksum,CHECKSUM_LEN);
  pbuf+=CHECKSUM_LEN;
#endif /* RF230_CONF_CHECKSUM */

#if RF230_CONF_TIMESTAMPS
  timestamp.authority_level = timesynch_authority_level();
  timestamp.time = timesynch_time();
  memcpy(pbuf,&timestamp,TIMESTAMP_LEN);
  pbuf+=TIMESTAMP_LEN;
#endif /* RF230_CONF_TIMESTAMPS */
 
/*Below comments were for cc240 radio, don't know how they apply to rf230 - DAK */
  /* The TX FIFO can only hold one packet. Make sure to not overrun
   * FIFO by waiting for transmission to start here and synchronizing
   * with the RF230_TX_ACTIVE check in rf230_send.
   *
   * Note that we may have to wait up to 320 us (20 symbols) before
   * transmission starts.
   */
//#ifdef TMOTE_SKY
//#define LOOP_20_SYMBOLS 400	/* 326us (msp430 @ 2.4576MHz) */
//#elif __AVR__
//#define LOOP_20_SYMBOLS 500	/* XXX */
//#endif
#define LOOP_20_SYMBOLS 500
 
 /* Wait for any previous transmission to finish. */
  rf230_waitidle();

  hal_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
  delay_us(TIME_P_ON_TO_TRX_OFF);

  radio_set_trx_state(TX_ARET_ON);	//enable auto ack
//#if WITH_SEND_CCA
//    radio_set_trx_state(TX_ARET_ON); //enable auto ack
//#endif /* WITH_SEND_CCA */

 /* Toggle the SLP_TR pin to initiate the frame transmission. */
 
  PRINTF("rf230: sending %d bytes\n", payload_len);
  hal_set_slptr_high();
  hal_set_slptr_low();
  hal_frame_write(buffer, total_len);
  
//  for(i = LOOP_20_SYMBOLS; i > 0; i--) {//dak was working with this

   if(1) {
#if RF230_CONF_TIMESTAMPS
      rtimer_clock_t txtime = timesynch_time();
#endif /* RF230_CONF_TIMESTAMPS */

      if(RF230_radio_on) {
	     ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
      }
      ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

      /* We wait until transmission has ended so that we get an
	  accurate measurement of the transmission time.*/
      rf230_waitidle();
      radio_set_trx_state(RX_AACK_ON);//Re-enable receive mode
#if RF230_CONF_TIMESTAMPS
      setup_time_for_transmission = txtime - timestamp.time;

      if(num_transmissions < 10000) {
	    total_time_for_transmission += timesynch_time() - txtime;
	    total_transmission_len += total_len;
	    num_transmissions++;
      }
#endif /* RF230_CONF_TIMESTAMPS */

#ifdef ENERGEST_CONF_LEVELDEVICE_LEVELS
      ENERGEST_OFF_LEVEL(ENERGEST_TYPE_TRANSMIT,rf230_get_txpower());
#endif
      ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
      if(RF230_radio_on) {
	    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
      }

      RELEASE_LOCK();
      return 0;
    }
//  }
  
  /* If we are using WITH_SEND_CCA, we get here if the packet wasn't
     transmitted because of other channel activity. */
  RIMESTATS_ADD(contentiondrop);
  PRINTF("rf230: do_send() transmission never started\n");
#if RADIOSTATS
  RF230_sendfail++;
#endif     
  RELEASE_LOCK();
  return -3;			/* Transmission never started! */
}/*---------------------------------------------------------------------------*/
void
rf230_init(void)
{
  /* Wait in case VCC just applied */
  delay_us(TIME_TO_ENTER_P_ON);

  /* Calibrate oscillator */
// calibrate_rc_osc_32k();

  /* Initialize Hardware Abstraction Layer. */
  hal_init();
  
  /* Do full rf230 Reset */
  hal_set_rst_low();
  hal_set_slptr_low();
  delay_us(TIME_RESET);
  hal_set_rst_high();

  /* Force transition to TRX_OFF. */
  hal_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
  delay_us(TIME_P_ON_TO_TRX_OFF);
  
  /* Verify that it is a supported version */
  uint8_t tvers = hal_register_read(RG_VERSION_NUM);
  uint8_t tmanu = hal_register_read(RG_MAN_ID_0);

  if ((tvers != RF230_REVA) && (tvers != RF230_REVB))
    PRINTF("rf230: Unsupported version %u\n",tvers);
  if (tmanu != SUPPORTED_MANUFACTURER_ID) 
    PRINTF("rf230: Unsupported manufacturer ID %u\n",tmanu);

  PRINTF("rf230: Version %u, ID %u\n",tvers,tmanu);
  hal_register_write(RG_IRQ_MASK, RF230_SUPPORTED_INTERRUPT_MASK);

    /* Turn off address decoding. */
//  reg = getreg(RF230_MDMCTRL0);
//  reg &= ~ADR_DECODE;
//  setreg(RF230_MDMCTRL0, reg);

  /* Change default values as recomended in the data sheet, */
  /* correlation threshold = 20, RX bandpass filter = 1.3uA. */
//  setreg(RF230_MDMCTRL1, CORR_THR(20));
//  reg = getreg(RF230_RXCTRL1);
//  reg |= RXBPF_LOCUR;
//  setreg(RF230_RXCTRL1, reg);
  
  /* Set the FIFOP threshold to maximum. */
 // setreg(RF230_IOCFG0, FIFOP_THR(127));

  /* Turn off "Security enable" (page 32). */
//  reg = getreg(RF230_SECCTRL0);
//  reg &= ~RXFIFO_PROTECTION;
//  setreg(RF230_SECCTRL0, reg);

 // rf230_set_pan_addr(0xffff, 0x0000, NULL);
 // rf230_set_channel(24);
  /* Set up the radio for auto mode operation. */
  hal_subregister_write(SR_MAX_FRAME_RETRIES, 2 );
  hal_subregister_write(SR_TX_AUTO_CRC_ON, 1);
  hal_subregister_write(SR_TRX_CMD, CMD_RX_AACK_ON);
  
  /* Start the packet receive process */
  process_start(&rf230_process, NULL);
}