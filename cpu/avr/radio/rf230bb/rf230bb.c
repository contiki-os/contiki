/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
 * All rights reserved.
 *
 *  Additional fixes for AVR contributed by:
 *
 *  David Kopf dak664@embarqmail.com
 *  Ivan Delamer delamer@ieee.com
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
/*
 * This code is almost device independent and should be easy to port.
 * Ported to Atmel RF230 21Feb2010 by dak
 */

#include <stdio.h>
#include <string.h>

#include "contiki.h"

#if defined(__AVR__)
#include <avr/io.h>

//_delay_us has the potential to use floating point which brings the 256 byte clz table into RAM
//#include <util/delay.h>
//#define delay_us( us )   ( _delay_us( ( us ) ) )
//_delay_loop_2(uint16_t count) is 4 CPU cycles per iteration, up to 32 milliseconds at 8MHz
#include <util/delay_basic.h>
#define delay_us( us )   ( _delay_loop_2(1+((unsigned long long)us*F_CPU)/4000000UL) ) 

#include <avr/pgmspace.h>
#elif defined(__MSP430__)
#include <io.h>
#endif

#include "dev/leds.h"
#include "dev/spi.h"
#include "rf230bb.h"

#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"

#include "sys/timetable.h"

#define WITH_SEND_CCA 0

/* Timestamps have not been tested */
#if RF230_CONF_TIMESTAMPS
#include "net/rime/timesynch.h"
#define TIMESTAMP_LEN 3
#else /* RF230_CONF_TIMESTAMPS */
#define TIMESTAMP_LEN 0
#endif /* RF230_CONF_TIMESTAMPS */
/* Nonzero FOOTER_LEN has not been tested */
#define FOOTER_LEN 0

/* RF230_CONF_CHECKSUM=0 for automatic hardware checksum */
#ifndef RF230_CONF_CHECKSUM
#define RF230_CONF_CHECKSUM 0
#endif

/* Autoack setting ignored in non-extended mode */
#ifndef RF230_CONF_AUTOACK
#define RF230_CONF_AUTOACK 1
#endif

/* We need to turn off autoack in promiscuous mode */
#if RF230_CONF_AUTOACK
static bool is_promiscuous;
#endif

/* RF230_CONF_FRAME_RETRIES is 1 plus the number written to the hardware. */
/* Valid range 1-16, zero disables extended mode. */
#ifndef RF230_CONF_FRAME_RETRIES
#ifdef RF230_CONF_AUTORETRIES /* Support legacy definition. */
#define RF230_CONF_FRAME_RETRIES RF230_CONF_AUTORETRIES
#else
#define RF230_CONF_FRAME_RETRIES 0 /* Extended mode disabled by default. */
#endif
#endif

/* In extended mode (FRAME_RETRIES>0) the tx routine waits for hardware
 * processing of an expected ACK and returns RADIO_TX_OK/NOACK result.
 * In non-extended mode the ACK is treated as a normal rx packet.
 * If the caller needs the ACK to be returned as an rx packet,
 * RF230_INSERTACK will generate one based on the hardware result.
 * This is triggered when the read routine is called with a buffer
 * length of three (the ack length).
 * In extended nmode it can be enabled by default to support either
 * method. In nonextended mode it would pass an extra ACK to RDCs
 * that use the TX_OK result to signal a successful ACK.
 * Adds 100 bytes of program flash and two bytes of RAM. 
 */
#if RF320_CONF_INSERTACK && RF230_CONF_FRAME_RETRIES
#define RF230_INSERTACK 1
uint8_t ack_pending,ack_seqnum;
#endif

/* RF230_CONF_CSMA_RETRIES is number of random-backoff/CCA retries. */
/* The hardware will accept 0-7, but 802.15.4-2003 only allows 5 maximum */
/* In RF231/128RFA1, a value of 7 means no CSMA bebofe the Tx. */
/* CSMA backoffs are long and can block radio duty cycling
 * over several channel check periods! */
/* Used only if RF230_CONF_FRAME_RETRIES > 0. */
#ifndef RF230_CONF_CSMA_RETRIES
#define RF230_CONF_CSMA_RETRIES 5
#endif

//Automatic and manual CRC both append 2 bytes to packets 
#if RF230_CONF_CHECKSUM || defined(RF230BB_HOOK_TX_PACKET)
#include "lib/crc16.h"
#endif
#define CHECKSUM_LEN 2

/* Note the AUX_LEN is equal to the CHECKSUM_LEN in any tested configurations to date! */
#define AUX_LEN (CHECKSUM_LEN + TIMESTAMP_LEN + FOOTER_LEN)
#if AUX_LEN != CHECKSUM_LEN
#warning RF230 Untested Configuration!
#endif

struct timestamp {
  uint16_t time;
  uint8_t authority_level;
};

#define FOOTER1_CRC_OK      0x80
#define FOOTER1_CORRELATION 0x7f

/* Leave radio on when USB powered or for testing low power protocols */
/* This allows DEBUGFLOW indication of packets received when the radio is "off" */
#if JACKDAW
#define RADIOALWAYSON 1
#else
#define RADIOALWAYSON 0
#define RADIOSLEEPSWHENOFF 1
#endif

/* RS232 delays will cause 6lowpan fragment overruns! Use DEBUGFLOW instead. */
#define DEBUG 0
#if DEBUG
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#define PRINTSHORT(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF(...)
#define PRINTSHORT(...)
#endif
#if DEBUG>1
/* Output format is suitable for text2pcap to convert to wireshark pcap file.
 * Use $text2pcap -e 0x809a (these_outputs) capture.pcap
 * Since the hardware calculates and appends the two byte checksum to Tx packets,
 * we just add two zero bytes to the packet dump. Don't forget to enable wireshark
 * 802.15.4 dissection even when the checksum is wrong!
 */
#endif

/* See clock.c and httpd-cgi.c for RADIOSTATS code */
#if AVR_WEBSERVER
#define RADIOSTATS 1
#endif
#if RADIOSTATS
uint16_t RF230_sendpackets,RF230_receivepackets,RF230_sendfail,RF230_receivefail;
#endif

#if RADIO_CONF_CALIBRATE_INTERVAL
/* Set in clock.c every 256 seconds */
/* The calibration is automatic when the radio turns on, so not needed when duty cycling */
uint8_t rf230_calibrate;
uint8_t rf230_calibrated; //for debugging, prints from main loop when calibration occurs
#endif

/* Track flow through mac, rdc and radio drivers, see contiki-raven-main.c for example of use */
#if DEBUGFLOWSIZE
extern uint8_t debugflowsize,debugflow[DEBUGFLOWSIZE];
#define DEBUGFLOW(c) if (debugflowsize<(DEBUGFLOWSIZE-1)) debugflow[debugflowsize++]=c
#else
#define DEBUGFLOW(c)
#endif

/* XXX hack: these will be made as Chameleon packet attributes */
#if RF230_CONF_TIMESTAMPS
rtimer_clock_t rf230_time_of_arrival, rf230_time_of_departure;

int rf230_authority_level_of_sender;

static rtimer_clock_t setup_time_for_transmission;
static unsigned long total_time_for_transmission, total_transmission_len;
static int num_transmissions;
#endif

#if defined(__AVR_ATmega128RFA1__)
volatile uint8_t rf230_wakewait, rf230_txendwait, rf230_ccawait;
#endif

uint8_t volatile rf230_pending;

/* RF230 hardware delay times, from datasheet */
typedef enum{
    TIME_TO_ENTER_P_ON               = 510, /**<  Transition time from VCC is applied to P_ON - most favorable case! */
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
PROCESS(rf230_process, "RF230 driver");
/*---------------------------------------------------------------------------*/

static int rf230_on(void);
static int rf230_off(void);

static int rf230_read(void *buf, unsigned short bufsize);

static int rf230_prepare(const void *data, unsigned short len);
static int rf230_transmit(unsigned short len);
static int rf230_send(const void *data, unsigned short len);

static int rf230_receiving_packet(void);
static int rf230_pending_packet(void);
static int rf230_cca(void);

uint8_t rf230_last_correlation,rf230_last_rssi,rf230_smallest_rssi;

const struct radio_driver rf230_driver =
  {
    rf230_init,
    rf230_prepare,
    rf230_transmit,
    rf230_send,
    rf230_read,
    rf230_cca,
    rf230_receiving_packet,
    rf230_pending_packet,
    rf230_on,
    rf230_off
  };

uint8_t RF230_receive_on;
static uint8_t channel;

/* Received frames are buffered to rxframe in the interrupt routine in hal.c */
uint8_t rxframe_head,rxframe_tail;
hal_rx_frame_t rxframe[RF230_CONF_RX_BUFFERS];

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
//static uint8_t
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
#if 0
static bool radio_is_sleeping(void)
{
    bool sleeping = false;

    /* The radio transceiver will be at SLEEP or one of the *_NOCLK states only if */
    /* the SLP_TR pin is high. */
    if (hal_get_slptr() != 0){
        sleeping = true;
    }

    return sleeping;
}
#endif
/*----------------------------------------------------------------------------*/
/** \brief  This function will reset the state machine (to TRX_OFF) from any of
 *          its states, except for the SLEEP state.
 */
static void
radio_reset_state_machine(void)
{
    /* The data sheet is not clear on what happens when slptr is raised in RX on
     * states, it "remains in the new state and returns to the preceding state
     * when slptr is lowered". Possibly that is why there is an undocumented
     * TIME_NOCLK_TO_WAKE delay here?
     */
    if (hal_get_slptr()) {
        DEBUGFLOW('V');
        hal_set_slptr_low();
        delay_us(TIME_NOCLK_TO_WAKE);
    }

    hal_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
    delay_us(TIME_CMD_FORCE_TRX_OFF);
}
/*---------------------------------------------------------------------------*/
static char
rf230_isidle(void)
{
  uint8_t radio_state;
  /* Contikimac can turn the radio off during an interrupt, so we always check
   * slptr before doing the SPI transfer. The caller must also make this test
   * if it could otherwise hang waiting for idle! */
  if (hal_get_slptr()) {
    if (RF230_receive_on) DEBUGFLOW('-');
	  return 1;
  }
  else {
    radio_state = hal_subregister_read(SR_TRX_STATUS);
    if (radio_state != BUSY_TX_ARET &&
      radio_state != BUSY_RX_AACK &&
      radio_state != STATE_TRANSITION &&
      radio_state != BUSY_RX && 
      radio_state != BUSY_TX) {
      return(1);
    }
    else {
      return(0);
    }
  }
}
  
static void
rf230_waitidle(void)
{
  /* TX_ARET with multiple csma retries can take a very long time to finish */
  while (1) {
    if (hal_get_slptr()) return;
    if (rf230_isidle()) break;
  }
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
static radio_status_t
radio_set_trx_state(uint8_t new_state)
{
    uint8_t current_state;

    /*Check function parameter and current state of the radio transceiver.*/
    if (!((new_state == TRX_OFF)    ||
          (new_state == RX_ON)      ||
          (new_state == PLL_ON)     ||
          (new_state == RX_AACK_ON) ||
          (new_state == TX_ARET_ON))){
        return RADIO_INVALID_ARGUMENT;
    }

	  if (hal_get_slptr()) {
	      DEBUGFLOW('W');
        return RADIO_WRONG_STATE;
    }

    /* Wait for radio to finish previous operation */
    rf230_waitidle();
    current_state = radio_get_trx_state();

    if (new_state == current_state){
        return RADIO_SUCCESS;
    }


    /* At this point it is clear that the requested new_state is: */
    /* TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON or TX_ARET_ON. */

    /* The radio transceiver can be in one of the following states: */
    /* TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON, TX_ARET_ON. */
    if(new_state == TRX_OFF){
        if (hal_get_slptr()) DEBUGFLOW('K');DEBUGFLOW('K');DEBUGFLOW('A'+hal_subregister_read(SR_TRX_STATUS));
        radio_reset_state_machine(); /* Go to TRX_OFF from any state. */
    } else {
        /* It is not allowed to go from RX_AACK_ON or TX_AACK_ON and directly to */
        /* TX_AACK_ON or RX_AACK_ON respectively. Need to go via PLL_ON. */
        /* (Old datasheets allowed other transitions, but this code complies with */
        /* the current specification for RF230, RF231 and 128RFA1.) */
        if (((new_state == TX_ARET_ON) && (current_state == RX_AACK_ON)) ||
            ((new_state == RX_AACK_ON) && (current_state == TX_ARET_ON))){
            /* First do intermediate state transition to PLL_ON. */
            /* The final state transition is handled after the if-else if. */
            hal_subregister_write(SR_TRX_CMD, PLL_ON);
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        }

        /* Any other state transition can be done directly. */
        hal_subregister_write(SR_TRX_CMD, new_state);

        /* When the PLL is active most states can be reached in 1us. However, from */
        /* TRX_OFF the PLL needs time to activate. */
        if (current_state == TRX_OFF){
            delay_us(TIME_TRX_OFF_TO_PLL_ACTIVE);
        } else {
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        }
    } /*  end: if(new_state == TRX_OFF) ... */

    /* Verify state transition.
     * Radio could have already switched to an RX_BUSY state, at least in cooja.
     * Don't know what the hardware does but this would not be an error.*/
    current_state = radio_get_trx_state();
    if (current_state != new_state) {
        if (((new_state == RX_ON) && (current_state == BUSY_RX)) ||
            ((new_state == RX_AACK_ON) && (current_state == BUSY_RX_AACK))) {
           /* This is OK. */
        } else {
            DEBUGFLOW('N');DEBUGFLOW('A'+new_state);DEBUGFLOW('A'+radio_get_trx_state());DEBUGFLOW('N');
            return RADIO_TIMED_OUT;
        }
    }

    return RADIO_SUCCESS;
}

void
rf230_set_promiscuous_mode(bool isPromiscuous) {
#if RF230_CONF_AUTOACK
    is_promiscuous = isPromiscuous;
/* TODO: Figure out when to pass promisc state to 802.15.4 */
//    radio_set_trx_state(is_promiscuous?RX_ON:RX_AACK_ON);
#endif
}

bool
rf230_is_ready_to_send() {
	switch(radio_get_trx_state()) {
		case BUSY_TX:
		case BUSY_TX_ARET:
			return false;
	}
	
	return true;
}


static void
flushrx(void)
{
  rxframe[rxframe_head].length=0;
}
/*---------------------------------------------------------------------------*/
static void
radio_on(void)
{
//   ENERGEST_OFF(ENERGEST_TYPE_LISTEN);//testing
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  RF230_receive_on = 1;
#ifdef RF230BB_HOOK_RADIO_ON
  RF230BB_HOOK_RADIO_ON();
#endif

/* If radio is off (slptr high), turn it on */
  if (hal_get_slptr()) {
    ENERGEST_ON(ENERGEST_TYPE_LED_RED);
#if RF230BB_CONF_LEDONPORTE1
    PORTE|=(1<<PE1); //ledon
#endif
#if defined(__AVR_ATmega128RFA1__)
    /* Use the poweron interrupt for delay */
    rf230_wakewait=1;
    {
      uint8_t sreg = SREG;
      sei();
      if (hal_get_slptr() == 0) DEBUGFLOW('$');
      hal_set_slptr_low();
      {
        int i;
        for (i=0;i<10000;i++) {
          if (!rf230_wakewait) break;
        }
        if (i>=10000) {DEBUGFLOW('G');DEBUGFLOW('g');DEBUGFLOW('A'+hal_subregister_read(SR_TRX_STATUS));}
      }
      SREG = sreg;
    }
#else
/* SPI based radios. The wake time depends on board capacitance.
 * Make sure the delay is long enough, as using SPI too soon will reset the MCU!
 * Use 2x the nominal value for safety. 1.5x is not long enough for Raven!
 */
//  uint8_t sreg = SREG;cli();
    hal_set_slptr_low();
    delay_us(2*TIME_SLEEP_TO_TRX_OFF);
//  SREG=sreg;
#endif
  }

#if RF230_CONF_AUTOACK
 // radio_set_trx_state(is_promiscuous?RX_ON:RX_AACK_ON);
  radio_set_trx_state(RX_AACK_ON);
#else
  radio_set_trx_state(RX_ON);
#endif
  rf230_waitidle();
}
static void
radio_off(void)
{
  RF230_receive_on = 0;
  if (hal_get_slptr()) {
    DEBUGFLOW('F');
    return;
  }

#if RF230BB_CONF_LEDONPORTE1
  PORTE&=~(1<<PE1); //ledoff
#endif
#ifdef RF230BB_HOOK_RADIO_OFF
  RF230BB_HOOK_RADIO_OFF();
#endif

  /* Wait for any transmission to end */
  rf230_waitidle(); 

#if RADIOALWAYSON
/* Do not transmit autoacks when stack thinks radio is off */
  radio_set_trx_state(RX_ON);
#else 
  /* Force the device into TRX_OFF.
   * First make sure an interrupt did not initiate a sleep. */
  if (hal_get_slptr()) {
    DEBUGFLOW('?');
    return;
  }
  radio_reset_state_machine();
#if RADIOSLEEPSWHENOFF
  /* Sleep Radio */
  hal_set_slptr_high();
  ENERGEST_OFF(ENERGEST_TYPE_LED_RED);
#endif
#endif /* RADIOALWAYSON */

   ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
}
/*---------------------------------------------------------------------------*/
static void
set_txpower(uint8_t power)
{
  if (power > TX_PWR_17_2DBM){
    power=TX_PWR_17_2DBM;
  }
  if (hal_get_slptr()) {
    DEBUGFLOW('f');
    PRINTF("rf230_set_txpower:Sleeping");  //happens with cxmac
  } else {
    //DEBUGFLOW('g');
    hal_subregister_write(SR_TX_PWR, power);
  }
}
void rf230_setpendingbit(uint8_t value)
{
  hal_subregister_write(SR_AACK_SET_PD, value);
}
#if 0
/*----------------------------------------------------------------------------*/
/**
    \brief Calibrate the internal RC oscillator

    This function calibrates the internal RC oscillator, based
    on an external 32KHz crystal connected to TIMER2. In order to
    verify the calibration result you can program the CKOUT fuse
    and monitor the CPU clock on an I/O pin.
*/
#define AVR_ENTER_CRITICAL_REGION( ) {uint8_t volatile saved_sreg = SREG; cli( )
#define AVR_LEAVE_CRITICAL_REGION( ) SREG = saved_sreg;}
    uint8_t osccal_original,osccal_calibrated;
void
calibrate_rc_osc_32k(void)
{

    /* Calibrate RC Oscillator: The calibration routine is done by clocking TIMER2
     * from the external 32kHz crystal while running an internal timer simultaneously.
     * The internal timer will be clocked at the same speed as the internal RC
     * oscillator, while TIMER2 is running at 32768 Hz. This way it is not necessary
     * to use a timed loop, and keep track cycles in timed loop vs. optimization
     * and compiler.
     */
    uint8_t osccal_original = OSCCAL;
    volatile uint16_t temp;
     
    /* Start with current value, which for some MCUs could be in upper or lower range */

//  PRR0 &= ~((1 << PRTIM2)|(1 << PRTIM1)); /*  Enable Timer 1 and 2 */

    TIMSK2 = 0x00; /*  Disable Timer/Counter 2 interrupts. */
    TIMSK1 = 0x00; /*  Disable Timer/Counter 1 interrupts. */

    /* Enable TIMER/COUNTER 2 to be clocked from the external 32kHz clock crystal.
     * Then wait for the timer to become stable before doing any calibration.
     */
    ASSR |= (1 << AS2);
 // while (ASSR & ((1 << TCN2UB)|(1 << OCR2AUB)|(1 << TCR2AUB)|(1 << TCR2BUB))) { ; }
    TCCR2B = 1 << CS20;   /* run timer 2 at divide by 1 (32KHz) */

    delay_us(50000UL);  //crystal takes significant time to stabilize
    AVR_ENTER_CRITICAL_REGION();

    uint8_t counter = 128;
    bool cal_ok = false;
    do{
        /* wait for timer to be ready for updated config */
        TCCR1B = 1 << CS10;

        while (ASSR & ((1 << TCN2UB)|(1 << OCR2AUB)|(1 << TCR2AUB)|(1 << TCR2BUB))) { ; }

        TCNT2 = 0x80;
        TCNT1 = 0;

        TIFR2 = 0xFF; /* Clear TIFR2 flags (Yes, really) */

        /* Wait for TIMER/COUNTER 2 to overflow. Stop TIMER/COUNTER 1 and 2, and
         * read the counter value of TIMER/COUNTER 1. It will now contain the
         * number of cpu cycles elapsed within the 3906.25 microsecond period.
         */
        while (!(TIFR2 & (1 << TOV2))){
            ;
            }
        temp = TCNT1;

        TCCR1B = 0;
/* Defining these as floating point introduces a lot of code and the 256 byte .clz table to RAM */
/* At 8 MHz we would expect 8*3906.25 = 31250 CPU clocks */
#define cal_upper 32812 //(31250*1.05) // 32812 = 0x802c
#define cal_lower 29687 //(31250*0.95) // 29687 = 0x73f7
        /* Iteratively reduce the error to be within limits */
        if (temp < cal_lower) {
            /* Too slow. Put the hammer down. */
            if (OSCCAL==0x7e) break; //stay in lowest range
            if (OSCCAL==0xff) break;
            OSCCAL++;
        } else if (temp > cal_upper) {
            /* Too fast, retard. */
            if (OSCCAL==0x81) break; //stay in highest range
            if (OSCCAL==0x00) break;
            OSCCAL--;
        } else {
            /* The CPU clock frequency is now within +/- 0.5% of the target value. */
            cal_ok = true;
        }

        counter--;
    } while ((counter != 0) && (false == cal_ok));

     osccal_calibrated=OSCCAL;   
    if (true != cal_ok) {
        /* We failed, therefore restore previous OSCCAL value. */
        OSCCAL = osccal_original;
    }

    OSCCAL = osccal_original;
    TCCR2B = 0;

    ASSR &= ~(1 << AS2);

    /* Disable both timers again to save power. */
    //    PRR0 |= (1 << PRTIM2);/* |(1 << PRTIM1); */

    AVR_LEAVE_CRITICAL_REGION();
}
#endif
/*---------------------------------------------------------------------------*/
int
rf230_init(void)
{
  uint8_t i;
  DEBUGFLOW('i');
  /* Wait in case VCC just applied */
  delay_us(TIME_TO_ENTER_P_ON);
  /* Initialize Hardware Abstraction Layer */
  hal_init();
 
  /* Calibrate oscillator */
 // printf_P(PSTR("\nBefore calibration OSCCAL=%x\n"),OSCCAL);
 // calibrate_rc_osc_32k();
 // printf_P(PSTR("After calibration OSCCAL=%x\n"),OSCCAL); 

  /* Set receive buffers empty and point to the first */
  for (i=0;i<RF230_CONF_RX_BUFFERS;i++) {
      rxframe[i].length=0;
  }
  rxframe_head=0;rxframe_tail=0;
  
  /* Do full rf230 Reset */
  hal_set_rst_low();
  hal_set_slptr_low();
  /* On powerup a TIME_RESET delay is needed here, however on some other MCU reset
   * (JTAG, WDT, Brownout) the radio may be sleeping. It can enter an uncertain
   * state (sending wrong hardware FCS for example) unless the full wakeup delay
   * is done.
   * Wake time depends on board capacitance; use 2x the nominal delay for safety.
   * See www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=78725
   */
  delay_us(2*TIME_SLEEP_TO_TRX_OFF);
  //delay_us(TIME_RESET); /* Old impl. */
  hal_set_rst_high();

  /* Force transition to TRX_OFF */
  hal_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
  delay_us(TIME_P_ON_TO_TRX_OFF);
  
  /* Verify that it is a supported version */
  /* Note gcc optimizes this away if DEBUG is not set! */
  //ATMEGA128RFA1 - version 4, ID 31
  uint8_t tvers = hal_register_read(RG_VERSION_NUM);
  uint8_t tmanu = hal_register_read(RG_MAN_ID_0);

  if ((tvers != RF230_REVA) && (tvers != RF230_REVB))
    PRINTF("rf230: Unsupported version %u\n",tvers);
  if (tmanu != SUPPORTED_MANUFACTURER_ID) 
    PRINTF("rf230: Unsupported manufacturer ID %u\n",tmanu);

  PRINTF("rf230: Version %u, ID %u\n",tvers,tmanu);
  
  rf230_warm_reset();
 
 /* Start the packet receive process */
  process_start(&rf230_process, NULL);
 
 /* Leave radio in on state (?)*/
  radio_on();

  return 1;
}
/*---------------------------------------------------------------------------*/
/* Used to reinitialize radio parameters without losing pan and mac address, channel, power, etc. */
void rf230_warm_reset(void) {
#if RF230_CONF_SNEEZER && JACKDAW
  /* Take jackdaw radio out of test mode */
#warning Manipulating PORTB pins for RF230 Sneezer mode!
  PORTB &= ~(1<<7);
  DDRB  &= ~(1<<7);
#endif
  
  hal_register_write(RG_IRQ_MASK, RF230_SUPPORTED_INTERRUPT_MASK);

  /* Set up number of automatic retries 0-15
   * (0 implies PLL_ON sends instead of the extended TX_ARET mode */
  hal_subregister_write(SR_MAX_FRAME_RETRIES,
      (RF230_CONF_FRAME_RETRIES > 0) ? (RF230_CONF_FRAME_RETRIES - 1) : 0 );
 
 /* Set up carrier sense/clear channel assesment parameters for extended operating mode */
  hal_subregister_write(SR_MAX_CSMA_RETRIES, RF230_CONF_CSMA_RETRIES );//highest allowed retries
  hal_register_write(RG_CSMA_BE, 0x80);       //min backoff exponent 0, max 8 (highest allowed)
  hal_register_write(RG_CSMA_SEED_0,hal_register_read(RG_PHY_RSSI) );//upper two RSSI reg bits RND_VALUE are random in rf231
 // hal_register_write(CSMA_SEED_1,42 );

  /* CCA Mode Mode 1=Energy above threshold  2=Carrier sense only  3=Both 0=Either (RF231 only) */
//hal_subregister_write(SR_CCA_MODE,1);  //1 is the power-on default

  /* Carrier sense threshold (not implemented in RF230 or RF231) */
// hal_subregister_write(SR_CCA_CS_THRES,1);

  /* Receiver sensitivity. If nonzero rf231/128rfa1 saves 0.5ma in rx mode */
  /* Not implemented on rf230 but does not hurt to write to it */
#ifdef RF230_MIN_RX_POWER
#if RF230_MIN_RX_POWER > 84
#warning rf231 power threshold clipped to -48dBm by hardware register
 hal_register_write(RG_RX_SYN, 0xf);
#elif RF230_MIN_RX_POWER < 0
#error RF230_MIN_RX_POWER can not be negative!
#endif
  hal_register_write(RG_RX_SYN, RF230_MIN_RX_POWER/6 + 1); //1-15 -> -90 to -48dBm
#endif

  /* CCA energy threshold = -91dB + 2*SR_CCA_ED_THRESH. Reset defaults to -77dB */
  /* Use RF230 base of -91;  RF231 base is -90 according to datasheet */
#ifdef RF230_CONF_CCA_THRES
#if RF230_CONF_CCA_THRES < -91
#warning
#warning RF230_CONF_CCA_THRES below hardware limit, setting to -91dBm
#warning
  hal_subregister_write(SR_CCA_ED_THRES,0);  
#elif RF230_CONF_CCA_THRES > -61
#warning
#warning RF230_CONF_CCA_THRES above hardware limit, setting to -61dBm
#warning
  hal_subregister_write(SR_CCA_ED_THRES,15);  
#else
  hal_subregister_write(SR_CCA_ED_THRES,(RF230_CONF_CCA_THRES+91)/2);  
#endif
#endif

  /* Use automatic CRC unless manual is specified */
#if RF230_CONF_CHECKSUM
  hal_subregister_write(SR_TX_AUTO_CRC_ON, 0);
#else
  hal_subregister_write(SR_TX_AUTO_CRC_ON, 1);
#endif

/* Limit tx power for testing miniature Raven mesh */
#ifdef RF230_MAX_TX_POWER
  set_txpower(RF230_MAX_TX_POWER);  //0=3dbm 15=-17.2dbm
#endif
}
/*---------------------------------------------------------------------------*/
static uint8_t buffer[RF230_MAX_TX_FRAME_LENGTH+AUX_LEN];

static int
rf230_transmit(unsigned short payload_len)
{
  int txpower;
  uint8_t total_len;
  uint8_t tx_result;
#if RF230_CONF_TIMESTAMPS
  struct timestamp timestamp;
#endif /* RF230_CONF_TIMESTAMPS */

  /* If radio is sleeping we have to turn it on first */
  /* This automatically does the PLL calibrations */
  if (hal_get_slptr()) {
#if defined(__AVR_ATmega128RFA1__)
	ENERGEST_ON(ENERGEST_TYPE_LED_RED);
#if RF230BB_CONF_LEDONPORTE1
    PORTE|=(1<<PE1); //ledon
#endif
    rf230_wakewait=1;
	  hal_set_slptr_low();
    {
      int i;
      for (i=0;i<10000;i++) {
        if (!rf230_wakewait) break;
      }
      if (i>=10000) {DEBUGFLOW('G');DEBUGFLOW('G');DEBUGFLOW('A'+hal_subregister_read(SR_TRX_STATUS));}
    }
#else
    hal_set_slptr_low();
    DEBUGFLOW('j');
    delay_us(2*TIME_SLEEP_TO_TRX_OFF); //extra delay (2x) depends on board capacitance
#endif

  } else {
#if RADIO_CONF_CALIBRATE_INTERVAL
  /* If nonzero, do periodic calibration. See clock.c */
    if (rf230_calibrate) {
      DEBUGFLOW('k');
      hal_subregister_write(SR_PLL_CF_START,1);   //takes 80us max
      hal_subregister_write(SR_PLL_DCU_START,1); //takes 6us, concurrently
      rf230_calibrate=0;
      rf230_calibrated=1;
      delay_us(80); //?
    }
#endif
  }
 
  /* Wait for any previous operation or state transition to finish */
  rf230_waitidle();
  if(RF230_receive_on) {
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  }
  /* Prepare to transmit */
#if RF230_CONF_FRAME_RETRIES
  radio_set_trx_state(TX_ARET_ON);
  DEBUGFLOW('t');
#else
  radio_set_trx_state(PLL_ON);
  DEBUGFLOW('T');
#endif

  txpower = 0;
  
  if(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) > 0) {
    /* Remember the current transmission power */
    txpower = rf230_get_txpower();
    /* Set the specified transmission power */
    set_txpower(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) - 1);
  }

  total_len = payload_len + AUX_LEN;

#if RF230_CONF_TIMESTAMPS
  rtimer_clock_t txtime = timesynch_time();
#endif /* RF230_CONF_TIMESTAMPS */

  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);
  
/* No interrupts across frame download! */
  HAL_ENTER_CRITICAL_REGION();

  /* Toggle the SLP_TR pin to initiate the frame transmission, then transfer
   * the frame. We have about 16 us + the on-air transmission time of 40 bits
   * (for the synchronization header) before the transceiver sends the PHR. */
  hal_set_slptr_high();
  hal_set_slptr_low();
  hal_frame_write(buffer, total_len);

  HAL_LEAVE_CRITICAL_REGION();
  PRINTF("rf230_transmit: %d\n", (int)total_len);

#if DEBUG>1
/* Note the dumped packet will have a zero checksum unless compiled with RF230_CONF_CHECKSUM
 * since we don't know what it will be if calculated by the hardware.
 */
  {
    uint8_t i;
    PRINTF("0000");       //Start a new wireshark packet
    for (i=0;i<total_len;i++) PRINTF(" %02x",buffer[i]);
    PRINTF("\n");
  }
#endif

#if RADIOSTATS
  RF230_sendpackets++;
#endif
 
 /* We wait until transmission has ended so that we get an
     accurate measurement of the transmission time.*/
  rf230_waitidle();

 /* Get the transmission result */  
#if RF230_CONF_FRAME_RETRIES
  tx_result = hal_subregister_read(SR_TRAC_STATUS);
#else
  tx_result=RADIO_TX_OK;
#endif

#ifdef ENERGEST_CONF_LEVELDEVICE_LEVELS
  ENERGEST_OFF_LEVEL(ENERGEST_TYPE_TRANSMIT,rf230_get_txpower());
#endif

 /* Restore the transmission power */
 if(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) > 0) {
    set_txpower(txpower & 0xff);
  }
 
#if RF230_CONF_TIMESTAMPS
  setup_time_for_transmission = txtime - timestamp.time;

  if(num_transmissions < 10000) {
    total_time_for_transmission += timesynch_time() - txtime;
    total_transmission_len += total_len;
    num_transmissions++;
  }

#endif /* RF230_CONF_TIMESTAMPS */

  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  if(RF230_receive_on) {
    DEBUGFLOW('l');
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
    radio_on();
  } else {
#if RADIOALWAYSON
    /* Enable reception */
    radio_on();
#else
    radio_off();
    PRINTF("rf230_transmit: turning radio off\n");
#endif
  }

#if RF230_INSERTACK
   ack_pending = 0;
#endif

  if (tx_result==1) {        //success, data pending from addressee
    tx_result=RADIO_TX_OK;           //handle as ordinary success
  }

  if (tx_result==RADIO_TX_OK) {
    RIMESTATS_ADD(lltx);
    if(packetbuf_attr(PACKETBUF_ATTR_RELIABLE))
      RIMESTATS_ADD(ackrx);		//ack was requested and received
#if RF230_INSERTACK
  /* Not PAN broadcast to FFFF, and ACK was requested and received */
  if (!((buffer[5]==0xff) && (buffer[6]==0xff)) && (buffer[0]&(1<<6)))
    ack_pending=1;
#endif

  } else if (tx_result==3) {        //CSMA channel access failure
    DEBUGFLOW('m');
    RIMESTATS_ADD(contentiondrop);
    PRINTF("rf230_transmit: Transmission never started\n");
    tx_result = RADIO_TX_COLLISION;
  } else if (tx_result==5) {        //Expected ACK, none received
    DEBUGFLOW('n');
    tx_result = RADIO_TX_NOACK;
    PRINTF("rf230_transmit: ACK not received\n");
    RIMESTATS_ADD(badackrx);		//ack was requested but not received
  } else if (tx_result==7) {        //Invalid (Can't happen since waited for idle above?)
    DEBUGFLOW('o');
    tx_result = RADIO_TX_ERR;
  }

  return tx_result;
}
/*---------------------------------------------------------------------------*/
static int
rf230_prepare(const void *payload, unsigned short payload_len)
{
  int ret = 0;
  uint8_t total_len,*pbuf;
#if RF230_CONF_TIMESTAMPS
  struct timestamp timestamp;
#endif
#if RF230_CONF_CHECKSUM
  uint16_t checksum;
#endif
#if RF230_INSERTACK
/* The sequence number is needed to construct the ack packet */
  ack_seqnum=*(((uint8_t *)payload)+2);
#endif

  DEBUGFLOW('p');

//  PRINTF("rf230: sending %d bytes\n", payload_len);
//  PRINTSHORT("s%d ",payload_len);

  RIMESTATS_ADD(tx);

#if RF230_CONF_CHECKSUM
  checksum = crc16_data(payload, payload_len, 0);
#endif
 
  /* Copy payload to RAM buffer */
  total_len = payload_len + AUX_LEN;
  if (total_len > RF230_MAX_TX_FRAME_LENGTH){
#if RADIOSTATS
    RF230_sendfail++;
#endif
    PRINTF("rf230_prepare: packet too large (%d, max: %d)\n",total_len,RF230_MAX_TX_FRAME_LENGTH);
    ret = -1;
    goto bail;
  }
  pbuf=&buffer[0];
  memcpy(pbuf,payload,payload_len);
  pbuf+=payload_len;

#if RF230_CONF_CHECKSUM
  memcpy(pbuf,&checksum,CHECKSUM_LEN);
  pbuf+=CHECKSUM_LEN;
#endif

#if RF230_CONF_TIMESTAMPS
  timestamp.authority_level = timesynch_authority_level();
  timestamp.time = timesynch_time();
  memcpy(pbuf,&timestamp,TIMESTAMP_LEN);
  pbuf+=TIMESTAMP_LEN;
#endif
/*------------------------------------------------------------*/  

#ifdef RF230BB_HOOK_TX_PACKET
#if !RF230_CONF_CHECKSUM
  { // Add a checksum before we log the packet out
    uint16_t checksum;
    checksum = crc16_data(payload, payload_len, 0);
    memcpy(buffer+total_len-CHECKSUM_LEN,&checksum,CHECKSUM_LEN);
  }
#endif /* RF230_CONF_CHECKSUM */
  RF230BB_HOOK_TX_PACKET(buffer,total_len);
#endif
  

bail:
  return ret;
}
/*---------------------------------------------------------------------------*/
static int
rf230_send(const void *payload, unsigned short payload_len)
{
	int ret = 0;

#ifdef RF230BB_HOOK_IS_SEND_ENABLED
	if(!RF230BB_HOOK_IS_SEND_ENABLED()) {
		goto bail;
	}
#endif
	
	if((ret=rf230_prepare(payload, payload_len))) {
	    PRINTF("rf230_send: Unable to send, prep failed (%d)\n",ret);
		goto bail;
	}

	ret = rf230_transmit(payload_len);
	
bail:
#if RADIOSTATS
    if (ret) RF230_sendfail++;
#endif
	return ret;
}
/*---------------------------------------------------------------------------*/
static int
rf230_off(void)
{
  /* Don't do anything if we are already turned off. */
  if(RF230_receive_on == 0) {
    //if (!hal_get_slptr()) DEBUGFLOW('5');
    return 0;
  }
  //if (hal_get_slptr()) DEBUGFLOW('6');

  /* If we are currently receiving a packet, we still call radio_off(),
     as that routine waits until Rx is complete (packet uploaded in ISR
     so no worries about losing it). The transmit routine may also turn
+     the radio off on a return to sleep. rf230_isidle checks for that. */
  if (!rf230_isidle()) {
  //DEBUGFLOW('X');DEBUGFLOW('X');DEBUGFLOW('A'+hal_subregister_read(SR_TRX_STATUS));
    PRINTF("rf230_off: busy receiving\r\n");
    //return 1;
  }

  radio_off();
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
rf230_on(void)
{
  if(RF230_receive_on) {
    //if (hal_get_slptr()) DEBUGFLOW('Q');//Cooja TODO: shows sleeping occasionally
    return 1;
  }

  radio_on();
  return 1;
}
/*---------------------------------------------------------------------------*/
uint8_t
rf230_get_channel(void)
{
//jackdaw reads zero channel, raven reads correct channel?
//return hal_subregister_read(SR_CHANNEL);
  return channel;
}
/*---------------------------------------------------------------------------*/
void
rf230_set_channel(uint8_t c)
{
 /* Wait for any transmission to end. */
  PRINTF("rf230: Set Channel %u\n",c);
  rf230_waitidle();
  channel=c;
  hal_subregister_write(SR_CHANNEL, c);
}
/*---------------------------------------------------------------------------*/
void
rf230_listen_channel(uint8_t c)
{
 /* Same as set channel but forces RX_ON state for sniffer or energy scan */
//  PRINTF("rf230: Listen Channel %u\n",c);
  rf230_set_channel(c);
  radio_set_trx_state(RX_ON);
}
/*---------------------------------------------------------------------------*/
void
rf230_set_pan_addr(unsigned pan,
                    unsigned addr,
                    const uint8_t ieee_addr[8])
//rf230_set_pan_addr(uint16_t pan,uint16_t addr,uint8_t *ieee_addr)
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
/*
 * Interrupt leaves frame intact in FIFO.
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
int
rf230_interrupt(void)
{
  /* Poll the receive process, unless the stack thinks the radio is off */
#if RADIOALWAYSON
if (RF230_receive_on) {
  DEBUGFLOW('+');
#endif
#if RF230_CONF_TIMESTAMPS
  interrupt_time = timesynch_time();
  interrupt_time_set = 1;
#endif /* RF230_CONF_TIMESTAMPS */

  process_poll(&rf230_process);
  
#if RF230_TIMETABLE_PROFILING
  timetable_clear(&rf230_timetable);
  TIMETABLE_TIMESTAMP(rf230_timetable, "interrupt");
#endif /* RF230_TIMETABLE_PROFILING */

  rf230_pending = 1;
  
#if RADIOSTATS //TODO:This will double count buffered packets
  RF230_receivepackets++;
#endif
  RIMESTATS_ADD(llrx);

#if RADIOALWAYSON
} else {
  DEBUGFLOW('-');
  rxframe[rxframe_head].length=0;
}
#endif
  return 1;
}
/*---------------------------------------------------------------------------*/
/* Process to handle input packets
 * Receive interrupts cause this process to be polled
 * It calls the core MAC layer which calls rf230_read to get the packet
 * rf230processflag can be printed in the main idle loop for debugging
 */
#if 0
uint8_t rf230processflag;
#define RF230PROCESSFLAG(arg) rf230processflag=arg
#else
#define RF230PROCESSFLAG(arg)
#endif

PROCESS_THREAD(rf230_process, ev, data)
{
  int len;
  PROCESS_BEGIN();
  RF230PROCESSFLAG(99);

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    RF230PROCESSFLAG(42);
#if RF230_TIMETABLE_PROFILING
    TIMETABLE_TIMESTAMP(rf230_timetable, "poll");
#endif /* RF230_TIMETABLE_PROFILING */

    packetbuf_clear();

    /* Turn off interrupts to avoid ISR writing to the same buffers we are reading. */
    HAL_ENTER_CRITICAL_REGION();

    len = rf230_read(packetbuf_dataptr(), PACKETBUF_SIZE);

    /* Restore interrupts. */
    HAL_LEAVE_CRITICAL_REGION();
    PRINTF("rf230_read: %u bytes lqi %u\n",len,rf230_last_correlation);
#if DEBUG>1
     {
        uint8_t i;
        unsigned const char * rxdata = packetbuf_dataptr();
        PRINTF("0000");
        for (i=0;i<len+AUX_LEN;i++) PRINTF(" %02x",rxdata[i]);
        PRINTF("\n");
      }
#endif


    RF230PROCESSFLAG(1);
    if(len > 0) {
      packetbuf_set_datalen(len);
      RF230PROCESSFLAG(2);
      NETSTACK_RDC.input();
#if RF230_TIMETABLE_PROFILING
      TIMETABLE_TIMESTAMP(rf230_timetable, "end");
      timetable_aggregate_compute_detailed(&aggregate_time,
                                           &rf230_timetable);
      timetable_clear(&rf230_timetable);
#endif /* RF230_TIMETABLE_PROFILING */
    } else {
#if RADIOSTATS
       RF230_receivefail++;
#endif
    }
  }

  PROCESS_END();
}
/* Read packet that was uploaded from Radio in ISR, else return zero.
 * The two-byte checksum is appended but the returned length does not include it.
 * Frames are buffered in the interrupt routine so this routine
 * does not access the hardware or change its status.
 * However, this routine must be called with interrupts disabled to avoid ISR
 * writing to the same buffer we are reading.
 * As a result, PRINTF cannot be used in here.
 */
/*---------------------------------------------------------------------------*/
static int
rf230_read(void *buf, unsigned short bufsize)
{
  uint8_t len,*framep;
#if FOOTER_LEN
  uint8_t footer[FOOTER_LEN];
#endif
#if RF230_CONF_CHECKSUM
  uint16_t checksum;
#endif
#if RF230_CONF_TIMESTAMPS
  struct timestamp t;
#endif
#if RF230_INSERTACK
/* Return an ACK to the mac layer */
  if(ack_pending && bufsize == 3){	
    ack_pending=0;
    uint8_t *buff=(uint8_t *)buf;
    buff[0]=2;
    buff[1]=0;
    buff[2]=ack_seqnum;
    return bufsize;
 }
#endif

  /* The length includes the twp-byte checksum but not the LQI byte */
  len=rxframe[rxframe_head].length;
  if (len==0) {
#if RADIOALWAYSON && DEBUGFLOWSIZE
   if (RF230_receive_on==0) {if (debugflow[debugflowsize-1]!='z') DEBUGFLOW('z');} //cxmac calls with radio off?
#endif
    return 0;
  }

#if RF230_CONF_TIMESTAMPS
  if(interrupt_time_set) {
    rf230_time_of_arrival = interrupt_time;
    interrupt_time_set = 0;
  } else {
    rf230_time_of_arrival = 0;
  }
  rf230_time_of_departure = 0;
#endif /* RF230_CONF_TIMESTAMPS */

  if(len > RF230_MAX_TX_FRAME_LENGTH) {
    /* Oops, we must be out of sync. */
    DEBUGFLOW('u');
    flushrx();
    RIMESTATS_ADD(badsynch);
    return 0;
  }

  if(len <= AUX_LEN) {
    DEBUGFLOW('s');
    //PRINTF("len <= AUX_LEN\n");
    flushrx();
    RIMESTATS_ADD(tooshort);
    return 0;
  }

  if(len - AUX_LEN > bufsize) {
    DEBUGFLOW('v');
    //PRINTF("len - AUX_LEN > bufsize\n");
    flushrx();
    RIMESTATS_ADD(toolong);
    return 0;
  }

 /* Transfer the frame, stripping the footer, but copying the checksum */
  framep=&(rxframe[rxframe_head].data[0]);
  memcpy(buf,framep,len-AUX_LEN+CHECKSUM_LEN);
  rf230_last_correlation = rxframe[rxframe_head].lqi;

  /* Clear the length field to allow buffering of the next packet */
  rxframe[rxframe_head].length=0;
  rxframe_head++;
  if (rxframe_head >= RF230_CONF_RX_BUFFERS) {
    rxframe_head=0;
  }
  /* If another packet has been buffered, schedule another receive poll */
  if (rxframe[rxframe_head].length) {
    rf230_interrupt();
  }
  else {
    rf230_pending = 0;
  }
  
 /* Point to the checksum */
  framep+=len-AUX_LEN; 
#if RF230_CONF_CHECKSUM
  memcpy(&checksum,framep,CHECKSUM_LEN);
#endif /* RF230_CONF_CHECKSUM */
  framep+=CHECKSUM_LEN;
#if RF230_CONF_TIMESTAMPS
  memcpy(&t,framep,TIMESTAMP_LEN);
#endif /* RF230_CONF_TIMESTAMPS */
  framep+=TIMESTAMP_LEN;
#if FOOTER_LEN
  memcpy(footer,framep,FOOTER_LEN);
#endif
#if RF230_CONF_CHECKSUM
  if(checksum != crc16_data(buf, len - AUX_LEN, 0)) {
    DEBUGFLOW('w');
    //PRINTF("checksum failed 0x%04x != 0x%04x\n",
    //  checksum, crc16_data(buf, len - AUX_LEN, 0));
  }
#if FOOTER_LEN
  if(footer[1] & FOOTER1_CRC_OK &&
     checksum == crc16_data(buf, len - AUX_LEN, 0)) {
#endif
#endif /* RF230_CONF_CHECKSUM */

/* Get the received signal strength for the packet, 0-84 dB above rx threshold */
#if 0   //more general
    rf230_last_rssi = rf230_get_raw_rssi();
#else   //faster
#if RF230_CONF_AUTOACK
 //   rf230_last_rssi = hal_subregister_read(SR_ED_LEVEL);  //0-84 resolution 1 dB
    rf230_last_rssi = hal_register_read(RG_PHY_ED_LEVEL);  //0-84, resolution 1 dB
#else
/* last_rssi will have been set at RX_START interrupt */
//  rf230_last_rssi = 3*hal_subregister_read(SR_RSSI);    //0-28 resolution 3 dB
#endif
#endif /* speed vs. generality */

  /* Save the smallest rssi. The display routine can reset by setting it to zero */
  if ((rf230_smallest_rssi==0) || (rf230_last_rssi<rf230_smallest_rssi))
     rf230_smallest_rssi=rf230_last_rssi;

 //   rf230_last_correlation = rxframe[rxframe_head].lqi;
    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rf230_last_rssi);
    packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, rf230_last_correlation);

    RIMESTATS_ADD(rx);

#if RF230_CONF_TIMESTAMPS
    rf230_time_of_departure =
      t.time +
      setup_time_for_transmission +
      (total_time_for_transmission * (len - 2)) / total_transmission_len;

    rf230_authority_level_of_sender = t.authority_level;

    packetbuf_set_attr(PACKETBUF_ATTR_TIMESTAMP, t.time);
#endif /* RF230_CONF_TIMESTAMPS */

#if RF230_CONF_CHECKSUM
#if FOOTER_LEN
  } else {
    DEBUGFLOW('x');
    //PRINTF("bad crc");
    RIMESTATS_ADD(badcrc);
    len = AUX_LEN;
  }
#endif
#endif

#ifdef RF230BB_HOOK_RX_PACKET
  RF230BB_HOOK_RX_PACKET(buf,len);
#endif

  /* Here return just the data length. The checksum is however still in the buffer for packet sniffing */
  return len - AUX_LEN;
}
/*---------------------------------------------------------------------------*/
void
rf230_set_txpower(uint8_t power)
{
  set_txpower(power);
}
/*---------------------------------------------------------------------------*/
uint8_t
rf230_get_txpower(void)
{
	uint8_t power = TX_PWR_UNDEFINED;
	if (hal_get_slptr()) {
		PRINTF("rf230_get_txpower:Sleeping");
	} else {
		power = hal_subregister_read(SR_TX_PWR);
	}
	return power;
}

/*---------------------------------------------------------------------------*/
uint8_t
rf230_get_raw_rssi(void)
{
  uint8_t rssi,state;
  bool radio_was_off = 0;

  /*The RSSI measurement should only be done in RX_ON or BUSY_RX.*/
  if(!RF230_receive_on) {
    radio_was_off = 1;
    rf230_on();
  }

/* The energy detect register is used in extended mode (since RSSI will read 0) */
/* The rssi register is multiplied by 3 to a consistent value from either register */
  state=radio_get_trx_state();
  if ((state==RX_AACK_ON) || (state==BUSY_RX_AACK)) {
 //  rssi = hal_subregister_read(SR_ED_LEVEL);  //0-84, resolution 1 dB
     rssi = hal_register_read(RG_PHY_ED_LEVEL);  //0-84, resolution 1 dB
  } else {
#if 0   // 3-clock shift and add is faster on machines with no hardware multiply
/* avr-gcc may have an -Os bug that uses the general subroutine for multiplying by 3 */
     rssi = hal_subregister_read(SR_RSSI);      //0-28, resolution 3 dB
     rssi = (rssi << 1)  + rssi;                //*3
#else  // 1 or 2 clock multiply, or compiler with correct optimization
     rssi = 3 * hal_subregister_read(SR_RSSI);
#endif

  }

  if(radio_was_off) {
    rf230_off();
  }
  return rssi;
}

/*---------------------------------------------------------------------------*/
static int
rf230_cca(void)
{
  uint8_t cca=0;
  uint8_t radio_was_off = 0;

  /* Turn radio on if necessary. If radio is currently busy return busy channel */
  /* This may happen when testing radio duty cycling with RADIOALWAYSON,
   * or because a packet just started. */
  if(RF230_receive_on) {
    if (hal_get_slptr()) {  //should not be sleeping!
	  DEBUGFLOW('<');
	  goto busyexit;
	} else {
      if (!rf230_isidle()) {
        //DEBUGFLOW('2');
        goto busyexit;
      }
	}
  } else {
    radio_was_off = 1;
    rf230_on();
  }

  ENERGEST_ON(ENERGEST_TYPE_LED_YELLOW);
  /* CCA Mode Mode 1=Energy above threshold  2=Carrier sense only  3=Both 0=Either (RF231 only) */
  /* Use the current mode. Note triggering a manual CCA is not recommended in extended mode */
//hal_subregister_write(SR_CCA_MODE,1);

  /* Start the CCA, wait till done, return result */
  /* Note reading the TRX_STATUS register clears both CCA_STATUS and CCA_DONE bits */
#if defined(__AVR_ATmega128RFA1__)
#if 1  //interrupt method
    /* Disable rx transitions to busy (RX_PDT_BIT) */
    /* Note: for speed this resets rx threshold to the compiled default */
#ifdef RF230_MIN_RX_POWER
    hal_register_write(RG_RX_SYN, RF230_MIN_RX_POWER/6 + 0x81);
#else
    hal_register_write(RG_RX_SYN, 0x80);
#endif
   /* Switch to RX_ON for measurement. This will wait if a packet is being received */
   radio_set_trx_state(RX_ON);

    rf230_ccawait=1;
//CCA_REQUEST is supposed to trigger the interrupt but it doesn't
//  hal_subregister_write(SR_CCA_REQUEST,1);

    /* Write to ED_LEVEL register to start CCA */
    {
      uint8_t volatile saved_sreg = SREG;
      sei( );
      hal_register_write(PHY_ED_LEVEL,0);
      while (rf230_ccawait) {}
      SREG = saved_sreg;
    }

    /* Use ED register to determine result. 77dBm is poweron csma default.*/
#ifdef RF230_CONF_CCA_THRES
    if (hal_register_read(RG_PHY_ED_LEVEL)<(91+RF230_CONF_CCA_THRES) cca=0xff;
#else
    if (hal_register_read(RG_PHY_ED_LEVEL)<(91-77)) cca=0xff;
#endif
//TODO:see if the status register works!
//   cca=hal_register_read(RG_TRX_STATUS);
#if RF230_CONF_AUTOACK
    radio_set_trx_state(RX_AACK_ON);
#endif

    /* Enable packet reception */
#ifdef RF230_MIN_RX_POWER
    hal_register_write(RG_RX_SYN, RF230_MIN_RX_POWER/6 + 0x01);
#else
    hal_register_write(RG_RX_SYN, 0x00);
#endif


#else
  /* If already in receive mode can read the current ED register without delay */
  /* CCA energy threshold = -91dB + 2*SR_CCA_ED_THRESH. Reset defaults to -77dB */
#ifdef RF230_CONF_CCA_THRES
    if (hal_register_read(RG_PHY_ED_LEVEL)<(91+RF230_CONF_CCA_THRES) cca=0xff;
#else
	if (hal_register_read(RG_PHY_ED_LEVEL)<(91-77)) cca=0xff;
#endif
#endif


#else /* RF230, RF231 */
  /* Don't allow interrupts! */
  /* Start the CCA, wait till done, return result */
  /* Note reading the TRX_STATUS register clears both CCA_STATUS and CCA_DONE bits */
{ uint8_t volatile saved_sreg = SREG;
  cli();
  rf230_waitidle();
  hal_subregister_write(SR_CCA_REQUEST,1);
  delay_us(TIME_CCA);
  while ((cca & 0x80) == 0 ) {
    if (hal_get_slptr()) {
        DEBUGFLOW('S');
        break;
    }
    cca=hal_register_read(RG_TRX_STATUS);
  }
  SREG=saved_sreg;
}
#endif
  ENERGEST_OFF(ENERGEST_TYPE_LED_YELLOW); 
  if(radio_was_off) {
    rf230_off();
  }
// if (cca & 0x40) {/*DEBUGFLOW('3')*/;} else {rf230_pending=1;DEBUGFLOW('4');}  
   if (cca & 0x40) {
//   DEBUGFLOW('5');
	 return 1;
   } else {
//  DEBUGFLOW('6');
 busyexit:
	 return 0;
   }
}
/*---------------------------------------------------------------------------*/
int
rf230_receiving_packet(void)
{
  uint8_t radio_state;
  if (hal_get_slptr()) {
    DEBUGFLOW('=');
  } else {  
    radio_state = hal_subregister_read(SR_TRX_STATUS);
    if ((radio_state==BUSY_RX) || (radio_state==BUSY_RX_AACK)) {
//      DEBUGFLOW('8');
      return 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
rf230_pending_packet(void)
{
#if RF230_INSERTACK
    if(ack_pending == 1) return 1;
#endif
  return rf230_pending;
}
/*---------------------------------------------------------------------------*/
#if RF230_CONF_SNEEZER && JACKDAW
/* See A.2 in the datasheet for the sequence needed.
 * This version for RF230 only, hence Jackdaw.
 * A full reset seems not necessary and allows keeping the pan address, etc.
 * for an easy reset back to network mode.
 */
void rf230_start_sneeze(void) {
//write buffer with random data for uniform spectral noise

//uint8_t txpower = hal_register_read(0x05);  //save auto_crc bit and power
//  hal_set_rst_low();
//  hal_set_slptr_low();
//  delay_us(TIME_RESET);
//  hal_set_rst_high();
    hal_register_write(0x0E, 0x01);
    hal_register_write(0x02, 0x03);
    hal_register_write(0x03, 0x10);
 // hal_register_write(0x08, 0x20+26);    //channel 26
    hal_subregister_write(SR_CCA_MODE,1); //leave channel unchanged

 // hal_register_write(0x05, 0x00);       //output power maximum
    hal_subregister_write(SR_TX_AUTO_CRC_ON, 0);  //clear AUTO_CRC, leave output power unchanged
 
    hal_register_read(0x01);             //should be trx-off state=0x08  
    hal_frame_write(buffer, 127);        //maximum length, random for spectral noise 

    hal_register_write(0x36,0x0F);       //configure continuous TX
    hal_register_write(0x3D,0x00);       //Modulated frame, other options are:
//  hal_register_write(RG_TX_2,0x10);    //CW -2MHz
//  hal_register_write(RG_TX_2,0x80);    //CW -500KHz
//  hal_register_write(RG_TX_2,0xC0);    //CW +500KHz

    DDRB  |= 1<<7;                       //Raven USB stick has PB7 connected to the RF230 TST pin.   
    PORTB |= 1<<7;                       //Raise it to enable continuous TX Test Mode.

    hal_register_write(0x02,0x09);       //Set TRX_STATE to PLL_ON
    delay_us(TIME_TRX_OFF_TO_PLL_ACTIVE);
    delay_us(TIME_PLL_LOCK);
    delay_us(TIME_PLL_LOCK);
 // while (hal_register_read(0x0f)!=1) {continue;}  //wait for pll lock-hangs
    hal_register_write(0x02,0x02);       //Set TRX_STATE to TX_START
}
#endif
