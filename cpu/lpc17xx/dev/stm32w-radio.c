/*
 * Copyright (c) 2010, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki OS
 *
 */
/*---------------------------------------------------------------------------*/
/**
* \file
*					Machine dependent STM32W radio code.
* \author
*					Salvatore Pitrulli
*					Chi-Anh La la@imag.fr
*         Simon Duquennoy <simonduq@sics.se>
*/
/*---------------------------------------------------------------------------*/

#include PLATFORM_HEADER
#include "hal/error.h"
#include "hal/hal.h"

#include "contiki.h"

#include "net/mac/frame802154.h"

#include "dev/stm32w-radio.h"
#include "net/netstack.h"

#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "sys/rtimer.h"

#define DEBUG 0

#include "dev/leds.h"
#define LED_ACTIVITY 0

#ifdef ST_CONF_RADIO_AUTOACK
#define ST_RADIO_AUTOACK ST_CONF_RADIO_AUTOACK
#else
#define ST_RADIO_AUTOACK 0
#endif /* ST_CONF_RADIO_AUTOACK */

#if RDC_CONF_DEBUG_LED
#define LED_RDC RDC_CONF_DEBUG_LED
#define LED_ACTIVITY 1
#else
#define LED_RDC 0
#endif


#if DEBUG > 0
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#if LED_ACTIVITY
#define LED_TX_ON() leds_on(LEDS_GREEN)
#define LED_TX_OFF() leds_off(LEDS_GREEN)
#define LED_RX_ON()     {                                       \
                                if(LED_RDC == 0){               \
                                  leds_on(LEDS_RED);            \
                                }                               \
                        }
#define LED_RX_OFF()    {                                       \
                                if(LED_RDC == 0){               \
                                  leds_off(LEDS_RED);            \
                                }                               \
                        }
#define LED_RDC_ON()    {                                       \
                                if(LED_RDC == 1){               \
                                  leds_on(LEDS_RED);            \
                                }                               \
                        }
#define LED_RDC_OFF()   {                                       \
                                if(LED_RDC == 1){               \
                                  leds_off(LEDS_RED);            \
                                }                               \
                        }
#else
#define LED_TX_ON()
#define LED_TX_OFF()
#define LED_RX_ON()
#define LED_RX_OFF()
#define LED_RDC_ON()
#define LED_RDC_OFF()
#endif

#if RDC_CONF_HARDWARE_CSMA
#define MAC_RETRIES 0
#endif

#ifndef MAC_RETRIES
#define MAC_RETRIES 1
#endif

#if MAC_RETRIES

 int8_t mac_retries_left;

 #define INIT_RETRY_CNT() (mac_retries_left = packetbuf_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS))
 #define DEC_RETRY_CNT() (mac_retries_left--)
 #define RETRY_CNT_GTZ() (mac_retries_left > 0)

#else

 #define INIT_RETRY_CNT()
 #define DEC_RETRY_CNT()
 #define RETRY_CNT_GTZ() 0

#endif


/* If set to 1, a send() returns only after the packet has been transmitted.
  This is necessary if you use the x-mac module, for example. */
#ifndef RADIO_WAIT_FOR_PACKET_SENT
#define RADIO_WAIT_FOR_PACKET_SENT 1
#endif

#define TO_PREV_STATE()       {                                       \
                                if(onoroff == OFF){                   \
                                  ST_RadioSleep();                    \
                                  ENERGEST_OFF(ENERGEST_TYPE_LISTEN); \
                                }                                     \
                              }
#if RDC_CONF_HARDWARE_CSMA
#define ST_RADIO_CHECK_CCA FALSE
#define ST_RADIO_CCA_ATTEMPT_MAX 0
#define ST_BACKOFF_EXP_MIN 0
#define ST_BACKOFF_EXP_MAX 0
#else
#define ST_RADIO_CHECK_CCA TRUE
#define ST_RADIO_CCA_ATTEMPT_MAX 4
#define ST_BACKOFF_EXP_MIN 2
#define ST_BACKOFF_EXP_MAX 6
#endif
const RadioTransmitConfig radioTransmitConfig = {
  TRUE,                       // waitForAck;
  ST_RADIO_CHECK_CCA,         // checkCca;     // Set to FALSE with low-power MACs.
  ST_RADIO_CCA_ATTEMPT_MAX,   // ccaAttemptMax;
  ST_BACKOFF_EXP_MIN,         // backoffExponentMin;
  ST_BACKOFF_EXP_MAX,         // backoffExponentMax;
  TRUE                        // appendCrc;
};

#define MAC_RETRIES 0

/*
 * The buffers which hold incoming data.
 */
#ifndef RADIO_RXBUFS
#define RADIO_RXBUFS 1
#endif

static uint8_t stm32w_rxbufs[RADIO_RXBUFS][STM32W_MAX_PACKET_LEN+1]; // +1 because of the first byte, which will contain the length of the packet.

#if RADIO_RXBUFS > 1
static volatile int8_t first = -1, last=0;
#else
static const int8_t first=0, last=0;
#endif

#if RADIO_RXBUFS > 1
 #define CLEAN_RXBUFS() do{first = -1; last = 0;}while(0)
 #define RXBUFS_EMPTY() (first == -1)

int RXBUFS_FULL(){

    int8_t first_tmp = first;
    return first_tmp == last;
}

#else /* RADIO_RXBUFS > 1 */
 #define CLEAN_RXBUFS() (stm32w_rxbufs[0][0] = 0)
 #define RXBUFS_EMPTY() (stm32w_rxbufs[0][0] == 0)
 #define RXBUFS_FULL() (stm32w_rxbufs[0][0] != 0)
#endif /* RADIO_RXBUFS > 1 */

static uint8_t __attribute__(( aligned(2) )) stm32w_txbuf[STM32W_MAX_PACKET_LEN+1];


#define CLEAN_TXBUF() (stm32w_txbuf[0] = 0)
#define TXBUF_EMPTY() (stm32w_txbuf[0] == 0)

#define CHECKSUM_LEN 2

/*
 * The transceiver state.
 */
#define ON     0
#define OFF    1

static volatile uint8_t onoroff = OFF;
static uint8_t receiving_packet = 0;
static s8 last_rssi;
static volatile StStatus last_tx_status;

#define BUSYWAIT_UNTIL(cond, max_time)                                  \
  do {                                                                  \
    rtimer_clock_t t0;                                                  \
    t0 = RTIMER_NOW();                                                  \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time)));   \
  } while(0)

static uint8_t locked;
#define GET_LOCK() locked++
static void RELEASE_LOCK(void) {
  if(locked>0)
       locked--;
}
static volatile uint8_t is_transmit_ack;
/*---------------------------------------------------------------------------*/
PROCESS(stm32w_radio_process, "STM32W radio driver");
/*---------------------------------------------------------------------------*/

static int stm32w_radio_init(void);
static int stm32w_radio_prepare(const void *payload, unsigned short payload_len);
static int stm32w_radio_transmit(unsigned short payload_len);
static int stm32w_radio_send(const void *data, unsigned short len);
static int stm32w_radio_read(void *buf, unsigned short bufsize);
static int stm32w_radio_channel_clear(void);
static int stm32w_radio_receiving_packet(void);
static int stm32w_radio_pending_packet(void);
static int stm32w_radio_on(void);
static int stm32w_radio_off(void);

static int add_to_rxbuf(uint8_t * src);
static int read_from_rxbuf(void * dest, unsigned short len);


const struct radio_driver stm32w_radio_driver =
 {
    stm32w_radio_init,
    stm32w_radio_prepare,
    stm32w_radio_transmit,
    stm32w_radio_send,
    stm32w_radio_read,
    stm32w_radio_channel_clear,
    stm32w_radio_receiving_packet,
    stm32w_radio_pending_packet,
    stm32w_radio_on,
    stm32w_radio_off,
  };
/*---------------------------------------------------------------------------*/
static int stm32w_radio_init(void)
{
  // A channel needs also to be setted.
  ST_RadioSetChannel(RF_CHANNEL);

  // Initialize radio (analog section, digital baseband and MAC).
  // Leave radio powered up in non-promiscuous rx mode.
  ST_RadioInit(ST_RADIO_POWER_MODE_OFF);

  onoroff = OFF;
  ST_RadioSetPanId(IEEE802154_PANID);

  CLEAN_RXBUFS();
  CLEAN_TXBUF();

#if ST_RADIO_AUTOACK && !(UIP_CONF_LL_802154 && RIMEADDR_CONF_SIZE==8)
#error "Autoack and address filtering can only be used with EUI 64"
#endif
  ST_RadioEnableAutoAck(ST_RADIO_AUTOACK);
  ST_RadioEnableAddressFiltering(ST_RADIO_AUTOACK);

  locked = 0;
  process_start(&stm32w_radio_process, NULL);

  return 0;
}
/*---------------------------------------------------------------------------*/
int stm32w_radio_set_channel(uint8_t channel)
{
  if (ST_RadioSetChannel(channel) == ST_SUCCESS)
    return 0;
  else
    return 1;
}
/*---------------------------------------------------------------------------*/
static int wait_for_tx(void){

  struct timer t;

  timer_set(&t, CLOCK_SECOND/10);
  while(!TXBUF_EMPTY()){
    if(timer_expired(&t)){
      PRINTF("stm32w: tx buffer full.\r\n");
      return 1;
    }
    /* Put CPU in sleep mode. */
    halSleepWithOptions(SLEEPMODE_IDLE,0);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int stm32w_radio_prepare(const void *payload, unsigned short payload_len)
{
    if(payload_len > STM32W_MAX_PACKET_LEN){
        PRINTF("stm32w: payload length=%d is too long.\r\n", payload_len);
        return RADIO_TX_ERR;
    }

#if !RADIO_WAIT_FOR_PACKET_SENT
    /* Check if the txbuf is empty.
     *  Wait for a finite time.
     * This sould not occur if we wait for the end of transmission in stm32w_radio_transmit().
    */
    if(wait_for_tx()){
      PRINTF("stm32w: tx buffer full.\r\n");
      return RADIO_TX_ERR;
    }
#endif /* RADIO_WAIT_FOR_PACKET_SENT */

    /* Copy to the txbuf.
     * The first byte must be the packet length.
     */
    CLEAN_TXBUF();
    memcpy(stm32w_txbuf + 1, payload, payload_len);

    return RADIO_TX_OK;

}
/*---------------------------------------------------------------------------*/
static int stm32w_radio_transmit(unsigned short payload_len)
{
    stm32w_txbuf[0] = payload_len + CHECKSUM_LEN;

    INIT_RETRY_CNT();

    if(onoroff == OFF){
      PRINTF("stm32w: Radio is off, turning it on.\r\n");
      ST_RadioWake();
      ENERGEST_ON(ENERGEST_TYPE_LISTEN);
    }

#if RADIO_WAIT_FOR_PACKET_SENT
    GET_LOCK();
#endif /* RADIO_WAIT_FOR_PACKET_SENT */
    last_tx_status = -1;
    LED_TX_ON();
    if(ST_RadioTransmit(stm32w_txbuf)==ST_SUCCESS){

      ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
      ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

      PRINTF("stm32w: sending %d bytes\r\n", payload_len);

#if DEBUG > 1
      for(uint8_t c=1; c <= stm32w_txbuf[0]-2; c++){
        PRINTF("%x:",stm32w_txbuf[c]);
      }
      PRINTF("\r\n");
#endif

#if RADIO_WAIT_FOR_PACKET_SENT

      if(wait_for_tx()){
        PRINTF("stm32w: unknown tx error.\r\n");
        TO_PREV_STATE();
        LED_TX_OFF();
        RELEASE_LOCK();
        return RADIO_TX_ERR;
      }
      TO_PREV_STATE();
      if(last_tx_status == ST_SUCCESS || last_tx_status == ST_PHY_ACK_RECEIVED || last_tx_status == ST_MAC_NO_ACK_RECEIVED){
        RELEASE_LOCK();
        if(last_tx_status == ST_PHY_ACK_RECEIVED){
          return RADIO_TX_OK; /* ACK status */
        }
        else if (last_tx_status == ST_MAC_NO_ACK_RECEIVED || last_tx_status == ST_SUCCESS){
          return RADIO_TX_NOACK;
        }
      }
      LED_TX_OFF();
      RELEASE_LOCK();
      return RADIO_TX_ERR;

#else /* RADIO_WAIT_FOR_PACKET_SENT */

      TO_PREV_STATE();
      LED_TX_OFF();
      return RADIO_TX_OK;

#endif /* RADIO_WAIT_FOR_PACKET_SENT */

    }

#if RADIO_WAIT_FOR_PACKET_SENT
    RELEASE_LOCK();
#endif /* RADIO_WAIT_FOR_PACKET_SENT */
    TO_PREV_STATE();

    PRINTF("stm32w: transmission never started.\r\n");
    /* TODO: Do we have to retransmit? */

    CLEAN_TXBUF();
    LED_TX_OFF();
    return RADIO_TX_ERR;

}
/*---------------------------------------------------------------------------*/
static int stm32w_radio_send(const void *payload, unsigned short payload_len)
{
  if(stm32w_radio_prepare(payload, payload_len) == RADIO_TX_ERR)
    return RADIO_TX_ERR;

  return stm32w_radio_transmit(payload_len);

}
/*---------------------------------------------------------------------------*/
static int stm32w_radio_channel_clear(void)
{
  return ST_RadioChannelIsClear();
}
/*---------------------------------------------------------------------------*/
static int stm32w_radio_receiving_packet(void)
{
  return receiving_packet;
}
/*---------------------------------------------------------------------------*/
static int stm32w_radio_pending_packet(void)
{
  return !RXBUFS_EMPTY();
}
/*---------------------------------------------------------------------------*/
static int stm32w_radio_off(void)
{
  /* Any transmit or receive packets in progress are aborted.
   * Waiting for end of transmission or reception have to be done.
   */
  if(locked)
  {
    PRINTF("stm32w: try to off while sending/receiving (lock=%u).\r\n", locked);
    return 0;
  }
  /* off only if there is no transmission or reception of packet. */
  if(onoroff == ON && TXBUF_EMPTY() && !receiving_packet){
    LED_RDC_OFF();
    ST_RadioSleep();
    onoroff = OFF;
    CLEAN_TXBUF();
    CLEAN_RXBUFS();

    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int stm32w_radio_on(void)
{
  PRINTF("stm32w: turn radio on\n");
  if(onoroff == OFF){
    LED_RDC_ON();
    ST_RadioWake();
    onoroff = ON;

    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
int stm32w_radio_is_on(void)
{
  return onoroff == ON;
}
/*---------------------------------------------------------------------------*/


void ST_RadioReceiveIsrCallback(u8 *packet,
                                  boolean ackFramePendingSet,
                                  u32 time,
                                  u16 errors,
                                  s8 rssi)
{
  LED_RX_ON();
  PRINTF("stm32w: incomming packet received\n");
  receiving_packet = 0;
  /* Copy packet into the buffer. It is better to do this here. */
  if(add_to_rxbuf(packet)){
    process_poll(&stm32w_radio_process);
    last_rssi = rssi;
  }
  LED_RX_OFF();
  GET_LOCK();
  is_transmit_ack = 1;
  /* Wait for sending ACK */
  BUSYWAIT_UNTIL(!is_transmit_ack, RTIMER_SECOND / 1500);
  RELEASE_LOCK();

}

void ST_RadioTxAckIsrCallback (void)
{
  /* This callback is for simplemac 1.1.0.
     Till now we block (RTIMER_SECOND / 1500)
     to prevent radio off during ACK transmission */
  is_transmit_ack = 0;
  //RELEASE_LOCK();
}


void ST_RadioTransmitCompleteIsrCallback(StStatus status,
                                           u32 txSyncTime,
                                           boolean framePending)
{

  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  LED_TX_OFF();

  last_tx_status = status;

  if(status == ST_SUCCESS || status == ST_PHY_ACK_RECEIVED){
      CLEAN_TXBUF();
  }
  else {

      if(RETRY_CNT_GTZ()){
          // Retransmission
          LED_TX_ON();
          if(ST_RadioTransmit(stm32w_txbuf)==ST_SUCCESS){

              ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
              ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

              PRINTF("stm32w: retransmission.\r\n");

              DEC_RETRY_CNT();
          }
          else {
              CLEAN_TXBUF();
              LED_TX_OFF();
              PRINTF("stm32w: retransmission failed.\r\n");
          }
      }
      else {
          CLEAN_TXBUF();
      }
  }

  /* Debug outputs. */
  if(status == ST_SUCCESS || status == ST_PHY_ACK_RECEIVED){
      PRINTF("stm32w: return status TX_END\r\n");
  }
  else if (status == ST_MAC_NO_ACK_RECEIVED){
      PRINTF("stm32w: return status TX_END_NOACK\r\n");
  }
  else if (status == ST_PHY_TX_CCA_FAIL){
      PRINTF("stm32w: return status TX_END_CCA_FAIL\r\n");
  }
  else if(status == ST_PHY_TX_UNDERFLOW){
      PRINTF("stm32w: return status TX_END_UNDERFLOW\r\n");
  }
  else {
      PRINTF("stm32w: return status TX_END_INCOMPLETE\r\n");
  }
}


boolean ST_RadioDataPendingShortIdIsrCallback(int16u shortId) {
  receiving_packet = 1;
  return FALSE;
}

boolean ST_RadioDataPendingLongIdIsrCallback(int8u* longId) {
  receiving_packet = 1;
  return FALSE;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(stm32w_radio_process, ev, data)
{
  int len;

  PROCESS_BEGIN();

  PRINTF("stm32w_radio_process: started\r\n");

  while(1) {

    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

    PRINTF("stm32w_radio_process: calling receiver callback\r\n");

#if DEBUG > 1
    for(uint8_t c=1; c <= RCVD_PACKET_LEN; c++){
      PRINTF("%x",stm32w_rxbuf[c]);
    }
    PRINTF("\r\n");
#endif

    packetbuf_clear();
    len = stm32w_radio_read(packetbuf_dataptr(), PACKETBUF_SIZE);
    if(len > 0) {
      packetbuf_set_datalen(len);
      NETSTACK_RDC.input();
    }
    if(!RXBUFS_EMPTY()){
      // Some data packet still in rx buffer (this happens because process_poll doesn't queue requests),
      // so stm32w_radio_process need to be called again.
      process_poll(&stm32w_radio_process);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static int stm32w_radio_read(void *buf, unsigned short bufsize)
{
  return read_from_rxbuf(buf,bufsize);
}

/*---------------------------------------------------------------------------*/
void ST_RadioOverflowIsrCallback(void)
{
  PRINTF("stm32w: radio overflow\r\n");
}
/*---------------------------------------------------------------------------*/
void ST_RadioSfdSentIsrCallback(u32 sfdSentTime)
{
}
/*---------------------------------------------------------------------------*/
void ST_RadioMacTimerCompareIsrCallback(void)
{
}
/*---------------------------------------------------------------------------*/
static int add_to_rxbuf(uint8_t * src)
{
    if(RXBUFS_FULL()){
        return 0;
    }

    memcpy(stm32w_rxbufs[last], src, src[0] + 1);
#if RADIO_RXBUFS > 1
    last = (last + 1) % RADIO_RXBUFS;
    if(first == -1){
        first = 0;
    }
#endif

    return 1;
}
/*---------------------------------------------------------------------------*/
static int read_from_rxbuf(void * dest, unsigned short len)
{

    if(RXBUFS_EMPTY()){ // Buffers are all empty
        return 0;
    }

    if(stm32w_rxbufs[first][0] > len){  // Too large packet for dest.
        len = 0;
    }
    else {
        len = stm32w_rxbufs[first][0];
        memcpy(dest,stm32w_rxbufs[first]+1,len);
        packetbuf_set_attr(PACKETBUF_ATTR_RSSI, last_rssi);
    }

#if RADIO_RXBUFS > 1
    ATOMIC(
           first = (first + 1) % RADIO_RXBUFS;
           int first_tmp = first;
           if(first_tmp == last){
               CLEAN_RXBUFS();
           }
    )
#else
    CLEAN_RXBUFS();
#endif

    return len;
}
/*---------------------------------------------------------------------------*/
short last_packet_rssi(){
    return last_rssi;
}

