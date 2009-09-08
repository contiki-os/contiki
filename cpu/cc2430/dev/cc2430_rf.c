/**
 * \file
 *         CC2430 RF driver
 * \author
 *         Zach Shelby <zach@sensinode.com>
 */

#include <stdio.h>

#include "contiki.h"
#include "dev/radio.h"
#include "dev/cc2430_rf.h"
#include "cc2430_sfr.h"
#include "dev/leds.h"
#include "sys/clock.h"

#include "net/rime/packetbuf.h"
#include "net/rime/rimestats.h"

static void (* receiver_callback)(const struct radio_driver *);

void cc2430_rf_set_receiver(void (* recv)(const struct radio_driver *));
int cc2430_rf_on(void);
int cc2430_rf_off(void);
int cc2430_rf_read(void *buf, unsigned short bufsize);
int cc2430_rf_send(const void *data, unsigned short len);

void cc2430_rf_init(void);

#ifndef RF_DEFAULT_POWER
#define RF_DEFAULT_POWER 100
#endif

#ifndef RF_DEFAULT_CHANNEL
#define RF_DEFAULT_CHANNEL 18
#endif

#ifndef CC2430_CONF_CHECKSUM
#define CC2430_CONF_CHECKSUM 0
#endif /* CC2420_CONF_CHECKSUM */

#if CC2430_CONF_CHECKSUM
#include "lib/crc16.h"
#define CHECKSUM_LEN 2
#else
#define CHECKSUM_LEN 0
#endif /* CC2430_CONF_CHECKSUM */

#define RF_RX_LED_ON()		leds_on(LEDS_RED);
#define RF_RX_LED_OFF()		leds_off(LEDS_RED);
#define RF_TX_LED_ON()		leds_on(LEDS_GREEN);
#define RF_TX_LED_OFF()		leds_off(LEDS_GREEN);

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#define RX_ACTIVE 0x80
#define TX_ACK 0x40
#define TX_ON_AIR 0x20
#define RX_NO_DMA

#ifdef HAVE_RF_ERROR
uint8_t rf_error = 0;
#endif

uint8_t rf_initialized = 0;
uint8_t rf_tx_power;
uint8_t rf_flags;
uint8_t rf_channel = 0;
rf_address_mode_t rf_addr_mode;
uint16_t rf_manfid;
uint8_t rf_softack;

/*---------------------------------------------------------------------------*/
PROCESS(cc2430_rf_process, "CC2430 RF driver");
/*---------------------------------------------------------------------------*/

const struct radio_driver cc2430_rf_driver =
  {
    cc2430_rf_send,
    cc2430_rf_read,
    cc2430_rf_set_receiver,
    cc2430_rf_on,
    cc2430_rf_off,
  };

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc2430_rf_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

    if(receiver_callback != NULL) {
      PRINTF("cc2430_rf_process: calling receiver callback\n");
      receiver_callback(&cc2430_rf_driver);
    } else {
      PRINTF("cc2430_rf_process: no receiver callback\n");
      cc2430_rf_command(ISFLUSHRX);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
cc2430_rf_init(void)
{
  if(rf_initialized) {
    return;
  }
  
  printf("cc2430_rf_init called\n");
  
  RFPWR &= ~RREG_RADIO_PD;	/*make sure it's powered*/
  while((RFPWR & ADI_RADIO_PD) == 1);
  while((RFIF & IRQ_RREG_ON) == 0);	/*wait for power up*/
  SLEEP &= ~OSC_PD; /*Osc on*/
  while((SLEEP & XOSC_STB) == 0);	/*wait for power up*/
  
  rf_flags = 0;
  rf_softack = 0;
  rf_addr_mode = RF_DECODER_NONE;
  
  FSMTC1 = 1;	/*don't abort reception, if enable called, accept ack, auto rx after tx*/
  
  MDMCTRL0H = 0x02;	 /* Generic client, standard hysteresis, decoder on 0x0a */
  MDMCTRL0L = 0xE2;	 /* automatic ACK and CRC, standard CCA and preamble 0xf2 */
  
  MDMCTRL1H = 0x30;			/* Defaults */
  MDMCTRL1L = 0x0;

  RXCTRL0H = 0x32;			/* RX tuning optimized */
  RXCTRL0L = 0xf5;
  
  /* get ID for MAC */
  rf_manfid = CHVER;
  rf_manfid <<= 8;
  rf_manfid += CHIPID;
  
  cc2430_rf_channel_set(RF_DEFAULT_CHANNEL);
  cc2430_rf_command(ISFLUSHTX);
  cc2430_rf_command(ISFLUSHRX);
  
  RFIM = IRQ_FIFOP;
  RFIF &= ~(IRQ_FIFOP);
  
  S1CON &= ~(RFIF_0 | RFIF_1);
  IEN2 |= RFIE;
  
  RF_TX_LED_OFF();
  RF_RX_LED_OFF();
  rf_initialized = 1;
  
  process_start(&cc2430_rf_process, NULL);
}
/*---------------------------------------------------------------------------*/
void
cc2430_rf_set_receiver(void (* recv)(const struct radio_driver *))
{
  receiver_callback = recv;
}
/*---------------------------------------------------------------------------*/
int
cc2430_rf_send(const void *payload, unsigned short payload_len)
{
  uint8_t i, counter;
  
  /*PRINTF("cc2430_rf_send\n");*/
  
  if(rf_flags & TX_ACK) {
    return -1;
  }
  if((rf_flags & RX_ACTIVE) == 0) {
    cc2430_rf_rx_enable();
  }

  /* Check packet attributes */
  /*printf("packetbuf_attr: txpower = %d\n", packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER));*/
  /* Should set TX power according to this if > 0 */
  
  PRINTF("cc2430_rf: sending %d byte payload\n", payload_len);
  
  RIMESTATS_ADD(lltx);
  
  /* Add checksum to end of packet as in cc2420 driver? */
  
  /* Send */

  cc2430_rf_command(ISFLUSHTX);
  RFD = (payload_len + 2);
  for(i = 0 ; i < payload_len; i++) {
    RFD = ((unsigned char*)(payload))[i];
  }
  RFD = (0);
  RFD = (0);
  
  if(cc2430_rf_cca_check(0,0) == -1) {
    return -1;
  }
  
  /* Start the transmission */
  
  RFIF &= ~IRQ_TXDONE;
  cc2430_rf_command(ISTXON);
  counter = 0;
  while(!(RFSTATUS & TX_ACTIVE) && (counter++ < 3)) {
    clock_delay(10);
  }
  
  if(!(RFSTATUS & TX_ACTIVE)) {
    printf("cc2430_rf: TX never active.\n");
    cc2430_rf_command(ISFLUSHTX);
    return -1;
  } else {
    RF_RX_LED_OFF();
    RF_TX_LED_ON();
    // rf_flags |= TX_ON_AIR;
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
int
cc2430_rf_read(void *buf, unsigned short bufsize)
{
  uint8_t i, len, type;
#if CC2420_CONF_CHECKSUM
  uint16_t checksum;
#endif /* CC2420_CONF_CHECKSUM */
  
  /* RX interrupt polled the cc2430_rf_process, now read the RX FIFO */
  
  /* Check the length */
  len = RFD;
  len &= 0x7f;
  printf("cc2430_rf: received %d bytes\n", len);
  
  /* Check for validity */
  if(len > CC2430_MAX_PACKET_LEN) {
    /* Oops, we must be out of sync. */
    printf("error: bad sync\n");
    cc2430_rf_command(ISFLUSHRX);
    RIMESTATS_ADD(badsynch);
    return 0;
  }
  
  if(len <= CC2430_MIN_PACKET_LEN) {
    printf("error: too short\n");
    cc2430_rf_command(ISFLUSHRX);
    RIMESTATS_ADD(tooshort);
    return 0;
  }
  
  if(len - CHECKSUM_LEN > bufsize) {
    printf("error: too long\n");
    cc2430_rf_command(ISFLUSHRX);
    RIMESTATS_ADD(toolong);
    return 0;
  }
  
  /* Check the type */
  type = RFD;
  type &= 0x07;
  if(typ e== 0x02) {
    printf("cc2430_rf: ack\n");
  } else {
    /* Read the buffer */
    printf("cc2430_rf: data = ");
    for(i = 0; i < (len - 2 - CHECKSUM_LEN); i++) {
      ((unsigned char*)(buf))[i] = RFD;
      printf("%c", ((unsigned char*)(buf))[i]);
    }
    printf("\n");
#if CC2430_CONF_CHECKSUM
    /* Deal with the checksum */
#endif /* CC2430_CONF_CHECKSUM */
  }

  packetbuf_set_attr(PACKETBUF_ATTR_RSSI, ((int8_t) RFD) - 45);
  packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, RFD);
  
  cc2430_rf_command(ISFLUSHRX);
  
  RIMESTATS_ADD(llrx);
  
  RF_RX_LED_OFF();
  
  return (len - 2 - CHECKSUM_LEN);
}
/*---------------------------------------------------------------------------*/
int
cc2430_rf_off(void)
{
  return cc2430_rf_rx_disable();
}
/*---------------------------------------------------------------------------*/
int
cc2430_rf_on(void)
{
  return cc2430_rf_rx_enable();
}
/*---------------------------------------------------------------------------*/

/**
 * Execute a single CSP command.
 *
 * \param command command to execute
 *
 */
void cc2430_rf_command(uint8_t command)
{
  if(command >= 0xE0) {	/*immediate strobe*/
    uint8_t fifo_count;
    switch(command) {	/*hardware bug workaround*/
    case ISRFOFF:
    case ISRXON:
    case ISTXON:
      fifo_count = RXFIFOCNT;
      RFST = command;
      clock_delay(2);
      if(fifo_count != RXFIFOCNT) {
	RFST = ISFLUSHRX;
	RFST = ISFLUSHRX;
      }
      break;
      
    default:
      RFST = command;
    }
  } else if(command == SSTART) {
    RFIF &= ~IRQ_CSP_STOP;	/*clear IRQ flag*/
    RFST = SSTOP;	/*make sure there is a stop in the end*/
    RFST = ISSTART;	/*start execution*/
    while((RFIF & IRQ_CSP_STOP) == 0);
  } else {
    RFST = command;	/*write command*/
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Select RF channel.
 *
 * \param channel channel number to select
 *
 * \return channel value or negative (invalid channel number)
 */

 /* channel freqdiv = (2048 + FSCTRL(9:0)) / 4
            freq = (2048 + FSCTRL(9:0)) MHz */

int8_t
cc2430_rf_channel_set(uint8_t channel)
{
  uint16_t freq;
  
  if((channel < 11) || (channel > 26)) {
    return -1;
  }
  
  cc2430_rf_command(ISSTOP);	/*make sure CSP is not running*/
  cc2430_rf_command(ISRFOFF);
  /* Channel values: 11-26 */
  freq = (uint16_t) channel - 11;
  freq *= 5;	/*channel spacing*/
  freq += 357; /*correct channel range*/
  freq |= 0x4000; /*LOCK_THR = 1*/
  FSCTRLH = (freq >> 8);
  FSCTRLL = (uint8_t)freq;
  
  cc2430_rf_command(ISRXON);
  
  rf_channel = channel;
  
  return (int8_t) channel;
}
/*---------------------------------------------------------------------------*/
/*PA_LEVEL TXCTRL register Output Power [dBm] Current Consumption [mA]
	31 0xA0FF 0 17.4
	27 0xA0FB -1 16.5
	23 0xA0F7 -3 15.2
	19 0xA0F3 -5 13.9
	15 0xA0EF -7 12.5
	11 0xA0EB -10 11.2
	 7 0xA0E7 -15 9.9
	 3 0xA0E3 -25 8.5*/

/**
 * Select RF transmit power.
 *
 * \param new_power new power level (in per cent)
 *
 * \return new level or negative (value out of range)
 */

int8_t
cc2430_rf_power_set(uint8_t new_power)
{
  uint16_t power;
  
  if(new_power > 100) {
    return -1;
  }

  power = 31 * new_power;
  power /= 100;
  power += 0xA160;
  
  /* Set transmitter power */
  TXCTRLH = (power >> 8);
  TXCTRLL = (uint8_t)power;
  
  rf_tx_power = (int8_t) new_power;
  return rf_tx_power;
}
/*---------------------------------------------------------------------------*/
/**
 * Enable RF receiver.
 *
 *
 * \return pdTRUE
 * \return pdFALSE	bus not free
 */
int8_t
cc2430_rf_rx_enable(void)
{
  printf("cc2430_rf_rx_enable called\n");
  if(!(rf_flags & RX_ACTIVE)) {
    IOCFG0 = 0x7f;   // Set the FIFOP threshold 127
    RSSIH = 0xd2; /* -84dbm = 0xd2 default, 0xe0 -70 dbm */
    rf_flags |= RX_ACTIVE;
    
    RFPWR &= ~RREG_RADIO_PD;	/*make sure it's powered*/
    while((RFIF & IRQ_RREG_ON) == 0);	/*wait for power up*/
    SLEEP &= ~OSC_PD; /*Osc on*/
    while((SLEEP & XOSC_STB) == 0);	/*wait for power up*/
    
    cc2430_rf_command(ISRXON);
    cc2430_rf_command(ISFLUSHRX);
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * Disable RF receiver.
 *
 *
 * \return pdTRUE
 * \return pdFALSE	bus not free
 */
int8_t cc2430_rf_rx_disable(void)
{
  cc2430_rf_command(ISSTOP);	/*make sure CSP is not running*/
  cc2430_rf_command(ISRFOFF);
  
  RFPWR |= RREG_RADIO_PD;		/*RF powerdown*/
  
  rf_flags = 0;
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * Enable RF transmitter.
 *
 *
 * \return pdTRUE
 * \return pdFALSE	bus not free
 */
int8_t
cc2430_rf_tx_enable(void)
{
  DMAARM = 0x80 + (1 << 0);	/*ABORT + channel bit*/
  
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * Set address decoder on/off.
 *
 * \param param 1=on 0=off.
 * \return pdTRUE operation successful
 */
int8_t
cc2430_rf_address_decoder_mode(rf_address_mode_t mode)
{
  int8_t retval = -1;
  
  rf_softack = 0;
  /* set oscillator on*/
  switch(mode) {
  case RF_SOFTACK_MONITOR:
    rf_softack = 1;
  case RF_MONITOR:
    MDMCTRL0H |= 0x10;	 /*Address-decode off , coordinator*/
    MDMCTRL0L &= ~0x10;	 /*no automatic ACK */
    break;
    
  case RF_DECODER_COORDINATOR:
    MDMCTRL0H |= 0x18;	 /*Address-decode on , coordinator*/
    MDMCTRL0L |= 0x10;	 /*automatic ACK */
    break;
    
  case RF_DECODER_ON:
    MDMCTRL0H |= 0x08;	 /*Address-decode on */
    MDMCTRL0L &= ~0x10;	 /* no automatic ACK */
    break;
    
  default:
    MDMCTRL0H &= ~0x18;	 /* Generic client */
    MDMCTRL0L &= ~0x10;	 /* no automatic ACK */
    break;
  }
  rf_addr_mode = mode;
  
  retval = 1;
  return retval;
}
/*---------------------------------------------------------------------------*/
/**
 * Channel energy detect.
 *
 * Coordinator use this function detect best channel for PAN-network.
 * \return RSSI-energy level dBm.
 * \return 0	operation failed.
 */

int8_t
cc2430_rf_analyze_rssi(void)
{
  int8_t retval = -128;
  /*pause_us(128);*/
  
  retval = (int8_t)RSSIL;
  retval -= 45;
  return retval;
}
/*---------------------------------------------------------------------------*/
/**
 * Clear channel assesment check.
 *
 * \return pdTRUE	CCA clear
 * \return pdFALSE	CCA reserved
 */
int8_t
cc2430_rf_cca_check(uint8_t backoff_count, uint8_t slotted)
{
  uint8_t counter, cca = 1;
  int8_t retval = 1;
  backoff_count;
  cc2430_rf_command(ISRXON);
  
  clock_delay(64);
  switch(slotted) {
  case 1:
    
    if(RFSTATUS & CCA) {
      counter = 0;
      cca = 1;
      while(cca != 0) {
	if(counter > 1) {
	  cca = 0;
	}
	clock_delay(256);
	if(!(RFSTATUS & CCA)) {
	  cca = 0;
	  retval = -1;
	}
	counter++;
      }
    } else {
      retval = -1;
    }
    break;
    
  case 0:
    if(!(RFSTATUS & CCA)) {
      retval = -1;
    } else {
      
    }
    break;
  }
  return retval;
}
/*---------------------------------------------------------------------------*/
/**
 * Send ACK.
 *
 *\param pending set up pending flag if pending > 0.
 */
void
cc2430_rf_send_ack(uint8_t pending)
{
  if(pending) {
    cc2430_rf_command(ISACKPEND);
  } else {
    cc2430_rf_command(ISACK);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * RF interrupt service routine.
 *
 */
void
cc2430_rf_ISR( void ) __interrupt (RF_VECTOR)
{
  EA = 0;
  RF_TX_LED_ON();
  if(RFIF & IRQ_TXDONE) {
    RF_TX_LED_OFF();
    RFIF &= ~IRQ_TXDONE;
    cc2430_rf_command(ISFLUSHTX);
  }

  if(RFIF & IRQ_FIFOP) {
    RF_TX_LED_OFF();
    if(RFSTATUS & FIFO) {
      RF_RX_LED_ON();
      /* Poll the RF process which calls cc2430_rf_read() */
      process_poll(&cc2430_rf_process);
    } else {
      cc2430_rf_command(ISFLUSHRX);
      cc2430_rf_command(ISFLUSHRX);
    }
    RFIF &= ~IRQ_FIFOP;
  }
  
  S1CON &= ~(RFIF_0 | RFIF_1);
  RFIM |= IRQ_FIFOP;
  EA = 1;
}
/*---------------------------------------------------------------------------*/
/**
 * RF error interrupt service routine.
 *
 */
void
cc2430_rf_error_ISR( void ) __interrupt (RFERR_VECTOR)
{
  EA = 0;
  TCON_RFERRIF = 0;
#ifdef HAVE_RF_ERROR
  rf_error = 254;
#endif
  cc2430_rf_command(ISRFOFF);
  cc2430_rf_command(ISFLUSHRX);
  cc2430_rf_command(ISFLUSHRX);
  cc2430_rf_command(ISRXON);
  RF_RX_LED_OFF();
  RF_TX_LED_OFF();
  EA = 1;
}
/*---------------------------------------------------------------------------*/
