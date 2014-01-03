/**
 * \defgroup cc112x CC112x
 * CC112x radio driver
 * @{
 *
 * \file
 * CC112x radio driver code
 *
 * \author Martin Chaplet <m.chaplet@kerlink.fr>
 */

/*
 * Copyright (c) 2012, Kerlink
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
 * This file is part of the Contiki operating system.
 *
 * $Id: $
 */
/*
 * This code is almost device independent and should be easy to port.
 */

#include <string.h>

#include "contiki.h"

#if defined(__AVR__)
#include <avr/io.h>
#endif

#include "watchdog.h"

#include "dev/leds.h"
#include "dev/spi.h"
#include "dev/cc112x.h"
#include "dev/cc112x_const.h"
#include "dev/cc112x_conf.h"

#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"

#include "sys/timetable.h"

//#define WITH_SEND_CCA 1

#define FOOTER_LEN 2
#define HEADER_LEN 0

//#define AUX_LEN (HEADER_LEN + FOOTER_LEN)
#define AUX_LEN 0

#define FOOTER1_CRC_OK      0x80
#define FOOTER1_CORRELATION 0x7f


#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#define DEBUG_LEDS DEBUG
#undef LEDS_ON
#undef LEDS_OFF
#if DEBUG_LEDS
#define LEDS_ON(x) leds_on(x)
#define LEDS_OFF(x) leds_off(x)
#else
#define LEDS_ON(x)
#define LEDS_OFF(x)
#endif

extern void cc112x_arch_init(void);

int cc112x_packets_seen, cc112x_packets_read;
static uint8_t sem_packet = 0;
static uint8_t receive_on, transmit_on;
static uint8_t volatile pending;

#define BUSYWAIT_UNTIL(cond, max_time)                                  \
  do {                                                                  \
    rtimer_clock_t t0;                                                  \
    t0 = RTIMER_NOW();                                                  \
    while(1) {                                                          \
      rtimer_clock_t tnow, tmax;                                        \
      if(cond) break;                                                   \
      tnow = RTIMER_NOW();                                              \
      tmax = t0 + (max_time);                                           \
      if(tmax >= t0 && (tnow >= tmax || tnow < t0)) break;              \
      if(tmax < t0 && tnow < t0 && tnow >= tmax) break;                 \
    }                                                                   \
  } while(0)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*---------------------------------------------------------------------------*/
PROCESS(cc112x_process, "CC112X driver");
/*---------------------------------------------------------------------------*/

int cc112x_on(void);
int cc112x_off(void);

static int cc112x_read(void *buf, unsigned short bufsize);

static int cc112x_prepare(const void *data, unsigned short len);
static int cc112x_transmit(unsigned short len);
static int cc112x_send(const void *data, unsigned short len);

static int cc112x_receiving_packet(void);
static int pending_packet(void);
static int cc112x_cca(void);
/*static int detected_energy(void);*/

int16_t cc112x_last_rssi;
uint8_t cc112x_last_lqi;

const struct radio_driver cc112x_driver =
  {
    cc112x_init,
    cc112x_prepare,
    cc112x_transmit,
    cc112x_send,
    cc112x_read,
    /* cc112x_set_channel, */
    /* detected_energy, */
    cc112x_cca,
    cc112x_receiving_packet,
    pending_packet,
    cc112x_on,
    cc112x_off,
  };

/*---------------------------------------------------------------------------*/

static void
getrxdata(void *buf, int len)
{
  CC112X_READ_FIFO_BUF(buf, len);
}
static void
getrxbyte(uint8_t *byte)
{
  CC112X_READ_FIFO_BYTE(*byte);
}
/*---------------------------------------------------------------------------*/
static void
strobe(enum cc112x_register regname)
{
  CC112X_STROBE(regname);
}
/*---------------------------------------------------------------------------*/
static unsigned int
status(void)
{
  uint8_t status;
  CC112X_GET_STATUS(status);
  return (status & CC112X_STATE_MASK);
}
/*---------------------------------------------------------------------------*/
static void
flushrx(void)
{
  uint8_t state = status();
  if(state == CC112X_STATE_IDLE || state == CC112X_STATE_RXFIFO_ERROR) {
    CC112X_STROBE(CC112X_SFRX);
  } else {
    PRINTF("CC112X Error: flushrx while not in IDLE or RXFIFO_OVERFLOW!\n");
  }
}
static void
flushtx(void)
{
  uint8_t state = status();
  if(state == CC112X_STATE_IDLE || state == CC112X_STATE_TXFIFO_ERROR) {
    CC112X_STROBE(CC112X_SFTX);
  } else {
    PRINTF("CC112X Error: flushtx while not in IDLE or TXFIFO_UNDERFLOW!\n");
  }
}

/*---------------------------------------------------------------------------*/
static unsigned
getreg(enum cc112x_register regname)
{
  unsigned reg;
  if( (unsigned)(regname >>8) == CC112X_EXTMEM )
  {
	  CC112X_READ_EXT_REG(regname, reg);
  }
  else  CC112X_READ_REG(regname, reg);
  return reg;
}
/*---------------------------------------------------------------------------*/
static void
setreg(enum cc112x_register regname, unsigned value)
{
	if( (unsigned)(regname >>8) == CC112X_EXTMEM )
	{
		  CC112X_WRITE_EXT_REG(regname, value);
	}
	else CC112X_WRITE_REG(regname, value);
}

/*---------------------------------------------------------------------------*/
static uint8_t locked, lock_on, lock_off;

static void
on(void)
{
  CC112X_ENABLE_GPIO0_INT();
  strobe(CC112X_SRX);

  BUSYWAIT_UNTIL(status() == CC112X_STATE_RX, RTIMER_SECOND / 100);
  if(status()!= CC112X_STATE_RX) {
    PRINTF("CC112X Error: could not get into RX! %02x - %d\n", status(), getreg(CC112X_MARCSTATE));
  }

  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  LEDS_ON(LEDS_YELLOW);
  receive_on = 1;
}
static void
off(void)
{
  receive_on = 0;

  /* Wait for transmission to end before turning radio off. */
  BUSYWAIT_UNTIL(status() != CC112X_STATE_TX, RTIMER_SECOND / 100);

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  strobe(CC112X_SIDLE);
  CC112X_DISABLE_GPIO0_INT();
  LEDS_OFF(LEDS_YELLOW);

  BUSYWAIT_UNTIL(status() == CC112X_STATE_IDLE, RTIMER_SECOND / 100);
  if(status() != CC112X_STATE_IDLE) {
    PRINTF("CC112X Error: could not get into IDLE! %02x - %d\n", status(), getreg(CC112X_MARCSTATE));
  }

  /* Flush FIFOs. */
  flushrx();
  flushtx();
}
/*---------------------------------------------------------------------------*/
#define GET_LOCK() locked++
static void RELEASE_LOCK(void) {
  if(locked == 1) {
    if(lock_on) {
      on();
      lock_on = 0;
    }
    if(lock_off) {
      off();
      lock_off = 0;
    }
  }
  locked--;
}


/*---------------------------------------------------------------------------*/
/**
 * \brief Set the transmission power.
 * \param power The transmission power in dBm
 */
static void
set_txpower(int8_t power)
{
  uint8_t pa_value;

  // Warning, RF studio seems to use a different formula (non-linear)
  // where: 15dBm -> 0x3F   ... 0 -> 0x1D ... -11dBm -> 0x03

  pa_value = (uint8_t) ( (power+18) * 2 ) - 1;

  if(pa_value > CC112X_POWER_MAX) pa_value = CC112X_POWER_MAX;
  if(pa_value < CC112X_POWER_MIN) pa_value = CC112X_POWER_MIN;

  setreg(CC112X_PA_CFG2, pa_value);
}

/*---------------------------------------------------------------------------*/
/**
 * \brief Calibrates Radio according to CC112x errata
 */
#define VCDAC_START_OFFSET 2
#define FS_VCO2_INDEX 0
#define FS_VCO4_INDEX 1
#define FS_CHP_INDEX 2
static void
cc112x_calib(void)
{
	uint8_t original_fs_cal2;
    uint8_t calResults_for_vcdac_start_high[3];
    uint8_t calResults_for_vcdac_start_mid[3];

    // 1) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
    setreg(CC112X_FS_VCO2,0);

    // 2) Start with high VCDAC (original VCDAC_START + 2):
    original_fs_cal2 = getreg(CC112X_FS_CAL2);
    setreg(CC112X_FS_CAL2, ( original_fs_cal2 + VCDAC_START_OFFSET) );

    // 3) Calibrate and wait for calibration to be done (radio back in IDLE state)
    strobe(CC112X_SCAL);
    while ( status() != CC112X_STATE_IDLE );

    // 4) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with high VCDAC_START value
    calResults_for_vcdac_start_high[FS_VCO2_INDEX] = getreg(CC112X_FS_VCO2);
    calResults_for_vcdac_start_high[FS_VCO4_INDEX] = getreg(CC112X_FS_VCO4);
    calResults_for_vcdac_start_high[FS_CHP_INDEX] = getreg(CC112X_FS_CHP);

    // 5) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
    setreg(CC112X_FS_VCO2, 0);

    // 6) Continue with mid VCDAC (original VCDAC_START):
    setreg(CC112X_FS_CAL2, original_fs_cal2);

    // 7) Calibrate and wait for calibration to be done (radio back in IDLE state)
    strobe(CC112X_SCAL);
    while ( status() != CC112X_STATE_IDLE );

    // 8) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with mid VCDAC_START value
    calResults_for_vcdac_start_mid[FS_VCO2_INDEX] = getreg(CC112X_FS_VCO2);
    calResults_for_vcdac_start_mid[FS_VCO4_INDEX] = getreg(CC112X_FS_VCO4);
    calResults_for_vcdac_start_mid[FS_CHP_INDEX] = getreg(CC112X_FS_CHP);

    // 9) Write back highest FS_VCO2 and corresponding FS_VCO and FS_CHP result
    if (calResults_for_vcdac_start_high[FS_VCO2_INDEX] > calResults_for_vcdac_start_mid[FS_VCO2_INDEX])
    {
        setreg(CC112X_FS_VCO2, calResults_for_vcdac_start_high[FS_VCO2_INDEX]);
        setreg(CC112X_FS_VCO4, calResults_for_vcdac_start_high[FS_VCO4_INDEX]);
        setreg(CC112X_FS_CHP, calResults_for_vcdac_start_high[FS_CHP_INDEX]);
    }
    else
    {
        setreg(CC112X_FS_VCO2, calResults_for_vcdac_start_mid[FS_VCO2_INDEX]);
        setreg(CC112X_FS_VCO4, calResults_for_vcdac_start_mid[FS_VCO4_INDEX]);
        setreg(CC112X_FS_CHP, calResults_for_vcdac_start_mid[FS_CHP_INDEX]);
    }
}


/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize the radio.
 */
int
cc112x_init(void)
{
  {
    int s = splhigh();
    cc112x_arch_init();		/* Initialize ports and SPI. */
    CC112X_DISABLE_GPIO0_INT();
    CC112X_GPIO0_INT_INIT();
    splx(s);
  }

  int i=0;

  /* Manual Reset Sequence. */
  SET_RESET_ACTIVE();
  clock_delay(127);
  SET_RESET_INACTIVE();
  clock_delay(125);

  PRINTF("Starting CC112X Driver on chip id 0x%X rev 0x%X\n\r",
		  getreg(CC112X_PARTNUMBER), getreg(CC112X_PARTVERSION) );

  /*
   * Apply default config from Smart RF Studio
   */

  for(i=0; i < (sizeof(preferredSettings)/sizeof(registerSetting_t)); i++)
  {
	  setreg(preferredSettings[i].addr, preferredSettings[i].data);
  }
  /*
  for(i=0; i < (sizeof(sniffSettings)/sizeof(registerSetting_t)); i++)
  {
	  setreg(sniffSettings[i].addr, sniffSettings[i].data);
  }*/


  /*
   * Driver Config :
   *
   * ** GPIOs :
   *   - GPIO0 as Sync TX / RX
   *   - GPIO1 as SPI SO
   *   - GPIO2 as CCA control
   *   - GPIO3 is free for use
   *
   *  **
   *  **
   */
  setreg(CC112X_IOCFG0, CC112X_GPIO_PKT_SYNC_RXTX);
  setreg(CC112X_IOCFG2, CC112X_GPIO_CCA);
  // Add CRC and RX Packet status bits (CRC, RSSI, LQI)
  setreg(CC112X_PKT_CFG1, 0x1);
  // Variable Packet length
  setreg(CC112X_PKT_CFG0, 0x20 );

  // GAIN Adjustement (cf. p34)
  //setreg(CC112X_AGC_GAIN_ADJUST, 0x5F);

  // Dump regs
  /*
  for(i=0;i<CC112X_EXTMEM;i++)
  {
	  PRINTF("Reg 0x%02X = 0x%02X\n\r",i,getreg(i));
  }
  for(i= CC112X_IF_MIX_CFG;i<=CC112X_PA_CFG3;i++)
  {
	  PRINTF("Reg 0x%X = 0x%02X\n\r",i,getreg(i));
  }
  */

  // Do an initial calibration.
  cc112x_calib();

#ifdef DEBUG
  // Clean up Fifo
  uint8_t packet[127];
  memset(packet,0xFF,sizeof(packet));
  CC112X_WRITE_FIFO_BUF(&packet,sizeof(packet));
  flushtx();
#endif

  flushrx();

  process_start(&cc112x_process, NULL);

//#define KLKTEST_RX
#ifdef KLKTEST_TX
  /* HACK : KLK-TEST */
  int ret = 0;
  do
  {
	  for(i=0; i <= 0xFF;i++)
	  {
		  memset(packet,i,sizeof(packet));
		  ret = cc112x_send(packet,sizeof(packet));
		  PRINTF("Send %02X frame: %d\n\r",i, ret);
		  clock_wait(10);
	  }
  }while(1);
#endif

#ifdef KLKTEST_RX
  cc112x_on();
#endif

  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Send the packet that has previously been prepared.
 * \param payload_len The packet length.
 * \return The transmission result status.
 * \return RADIO_TX_OK Transmission succeeded.
 * \return RADIO_TX_COLLISION Transmission failed: the channel wasn't clear.
 * \return RADIO_TX_ERR Transmission failed: FIFO error.
 */
static int
cc112x_transmit(unsigned short payload_len)
{
  int txpower;
  int ret = RADIO_TX_OK;

  GET_LOCK();

  if(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) > 0) {
	/* Remember the current transmission power */
	txpower = cc112x_get_txpower();
	/* Set the specified transmission power */
	set_txpower(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) - 1);
  }

  /* The TX FIFO can only hold one packet. Make sure to not overrun
   * FIFO by waiting for transmission to start here and synchronizing
   * with the CC112X_STATE_TX check in cc112x_send.
   */
#ifdef KLKTEST_TX
  /* KLK HACK : check FIFO */

  uint8_t buf[127];
  int i=0;
  CC112X_READ_TXFIFO_DIRECT(0,buf, sizeof(buf));
  PRINTF("FIFO Contents :\n\r");
  for(i=0; i<sizeof(buf); i++)
  {
	  PRINTF("%02X ",buf[i]);
	  if((i%16) == 15) PRINTF("\n\r");
  }
  PRINTF("\n\r");
#endif

  // Enable Sync Word interrupt
  sem_packet = 0;
  transmit_on = 1;
  CC112X_ENABLE_GPIO0_INT();

  LEDS_ON(LEDS_RED);

#if WITH_SEND_CCA
  if(!receive_on) {
      strobe(CC1100_SRX);
  }
  // Manually use GPIO config ... Internal CCA function can also be used, see User Guide p 39
  setreg(CC112X_IOCFG2, CC112X_GPIO_CCA);
  BUSYWAIT_UNTIL(CC112X_GPIO2_IS_1, RTIMER_SECOND / 100);
  strobe(CC112X_STX);
#else /* WITH_SEND_CCA */
  strobe(CC112X_STX);
#endif /* WITH_SEND_CCA */

  // Wait for real start of transmission (Sync word)
  //BUSYWAIT_UNTIL(CC112X_GPIO0_IS_1, RTIMER_SECOND / 10);
  BUSYWAIT_UNTIL((status() == CC112X_STATE_TX), RTIMER_SECOND / 10);

  if(status() != CC112X_STATE_TX)
  //if(!CC112X_GPIO0_IS_1)
  {
	  PRINTF("Transmission never started !\n\r");
	  goto exit;
  }

  if(receive_on) {
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  }
  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

  /*
   * We wait until transmission has ended so that we get an
   * accurate measurement of the transmission time.
   */
  BUSYWAIT_UNTIL(sem_packet!=0,RTIMER_SECOND);
  CC112X_DISABLE_GPIO0_INT();

  if(sem_packet==0)
  {
	  PRINTF("Transmission never ended !\n\r");
	  strobe(CC112X_SIDLE);
	  clock_delay(10);
	  flushtx();
  }

exit:

#ifdef ENERGEST_CONF_LEVELDEVICE_LEVELS
  ENERGEST_OFF_LEVEL(ENERGEST_TYPE_TRANSMIT,cc112x_get_txpower());
#endif
  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  transmit_on = 0;
  //flushtx();

  LEDS_OFF(LEDS_RED);

  if(receive_on) {
    /* We need to explicitly turn the radio on again, since TXOFF_MODE is IDLE. */
    on();
  } else {
    off();
  }

  if(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) > 0) {
	/* Restore the transmission power */
	set_txpower(txpower & 0xff);
  }

  RELEASE_LOCK();
  return ret;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Prepare the radio with a packet to be sent.
 * \param payload The packet.
 * \param payload_len The packet length.
 */
static int
cc112x_prepare(const void *payload, unsigned short payload_len)
{
	uint8_t total_len;
	int i;

	GET_LOCK();

	PRINTF("cc112x: sending %d bytes\n\r", payload_len);

	RIMESTATS_ADD(lltx);

	  PRINTF("payload Contents :\n\r");
	  for(i=0; i<payload_len; i++)
	  {
		  PRINTF("%02X ",((uint8_t *)payload)[i]);
		  if((i%16) == 15) PRINTF("\n\r");
	  }
	  PRINTF("\n\r");

	/* TODO : Wait for any previous transmission to finish ? */
/*	do
	{
		PRINTF("NumTXbytes = %d\n\r",getreg(CC112X_NUM_TXBYTES));
		if(getreg(CC112X_NUM_TXBYTES) == 0) break;
	}while(1);*/

	/* Flush TX Fifo (must be in idle mode) */
	//strobe(CC112X_SFTX);

	/* Write packet to TX FIFO. */
	//total_len = payload_len + AUX_LEN;
	total_len = payload_len;
	CC112X_WRITE_FIFO_BUF(&total_len, 1);
	CC112X_WRITE_FIFO_BUF(payload, payload_len);

	RELEASE_LOCK();
	return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Prepare & transmit a packet.
 * \param payload The packet.
 * \param payload_len The packet length.
 * \return The transmission result status.
 * \return RADIO_TX_OK Transmission succeeded.
 * \return RADIO_TX_COLLISION Transmission failed: the cahnnel wasn't clear.
 * \return RADIO_TX_ERR Transmission failed: FIFO error.
 */
static int
cc112x_send(const void *payload, unsigned short payload_len)
{
  cc112x_prepare(payload, payload_len);
  return cc112x_transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Turn the radio off.
 */
int
cc112x_off(void)
{
  /* Don't do anything if we are already turned off. */
  if(receive_on == 0) {
    return 1;
  }

  /*
   * If we are called when the driver is locked, we indicate that the
   * radio should be turned off when the lock is unlocked.
   */
  if(locked) {
    lock_off = 1;
    return 1;
  }

  GET_LOCK();
  /*
   * If we are currently receiving a packet (indicated by SFD == 1),
   * we don't actually switch the radio off now, but signal that the
   * driver should switch off the radio once the packet has been
   * received and processed, by setting the 'lock_off' variable.
   */
  if(CC112X_GPIO0_IS_1) {
    lock_off = 1;
  } else {
    off();
  }
  RELEASE_LOCK();
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Turn the radio on.
 */
int
cc112x_on(void)
{
  if(receive_on) {
    return 1;
  }
  if(locked) {
    lock_on = 1;
    return 1;
  }

  GET_LOCK();
  on();
  RELEASE_LOCK();
  return 1;
}

/*---------------------------------------------------------------------------*/
#if CC112X_TIMETABLE_PROFILING
#define cc112x_timetable_size 16
TIMETABLE(cc112x_timetable);
TIMETABLE_AGGREGATE(aggregate_time, 10);
#endif /* CC112X_TIMETABLE_PROFILING */
/**
 * \brief Packet interrupt handler.
 *
 * Triggered when the packet has been sent or is received (falling edge of synctxrx)
 *
 */
int
cc112x_interrupt(void)
{
	CC112X_CLEAR_GPIO0_INT();

	//PRINTF("Interrupt : TX = %d, RX = %d\n\r",transmit_on, receive_on );
	if(transmit_on)
	{
		// TX
		sem_packet = 1;
	}
	else if(receive_on)
	{
		// If RX packet, wake up thread
		process_poll(&cc112x_process);
		cc112x_packets_seen++;
	}

	#if CC112X_TIMETABLE_PROFILING
	timetable_clear(&cc112x_timetable);
	TIMETABLE_TIMESTAMP(cc112x_timetable, "interrupt");
	#endif /* CC112X_TIMETABLE_PROFILING */

	return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Forward received packets to upper layer.
 */
PROCESS_THREAD(cc112x_process, ev, data)
{
  int len;

  PROCESS_BEGIN();

  PRINTF("cc112x_process: started\n");

  while(1) {
	 PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

	 LEDS_ON(LEDS_GREEN);

  #if CC112X_TIMETABLE_PROFILING
      TIMETABLE_TIMESTAMP(cc112x_timetable, "poll");
  #endif /* CC112X_TIMETABLE_PROFILING */

      packetbuf_clear();
      len = cc112x_read(packetbuf_dataptr(), PACKETBUF_SIZE);

      packetbuf_set_datalen(len);

      NETSTACK_RDC.input();
  #if CC112X_TIMETABLE_PROFILING
      TIMETABLE_TIMESTAMP(cc112x_timetable, "end");
      timetable_aggregate_compute_detailed(&aggregate_time,
                                           &cc112x_timetable);
        timetable_clear(&cc112x_timetable);
  #endif /* CC112X_TIMETABLE_PROFILING */

      LEDS_OFF(LEDS_GREEN);

      if(receive_on)
      {
    	  // Strobe RX again
    	  strobe(CC112X_SRX);
      }
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/**
 * \brief Read a received packet into FIFO.
 * \param buf The destination buffer.
 * \param bufsize The size of the destination buffer.
 * \return The packet length.
 * \return 0 In case of error.
 */
static int
cc112x_read(void *buf, unsigned short bufsize)
{
  uint8_t footer[FOOTER_LEN];
  uint8_t len;

  GET_LOCK();

  cc112x_packets_read++;

  getrxbyte(&len);

  if(len > CC112X_MAX_PACKET_LEN) {
	/* Oops, we must be out of sync. */
	flushrx();
	RIMESTATS_ADD(badsynch);
	RELEASE_LOCK();
	return 0;
  }

  if(len <= AUX_LEN) {
	flushrx();
	RIMESTATS_ADD(tooshort);
	RELEASE_LOCK();
	return 0;
  }

  if(len - AUX_LEN > bufsize) {
	flushrx();
	RIMESTATS_ADD(toolong);
	RELEASE_LOCK();
	return 0;
  }

  /*
  uint8_t nrxbytes = getreg(CC112X_NUM_RXBYTES);
  do{
	  nrxbytes = getreg(CC112X_NUM_RXBYTES);
	  getreg(CC112X_NUM_RXBYTES);
	  //PRINTF("stat = %02X, fifoRX bytes = %d\n\r",status() & CC112X_STATE_MASK,nrxbytes);
	  clock_delay(1000);
  }while( nrxbytes < len);
  */

  /* Check RX Fifo Error */
  if( status() == CC112X_STATE_RXFIFO_ERROR )
  {
	  flushrx();
	  RIMESTATS_ADD(badsynch);
	  RELEASE_LOCK();
	  return 0;
  }

  getrxdata(buf, len - AUX_LEN);
  getrxdata(&footer, FOOTER_LEN);

//#ifdef KLKTEST_RX
#if 1
  int i=0;
  /* Dump Packet */
  PRINTF("Packet received (%d bytes) :\n\r",len);
  for(i=0; i<len; i++)
  {
	  PRINTF("%02X ",((uint8_t *)(buf))[i]);
	  if((i%16) == 15) PRINTF("\n\r");
  }
  PRINTF("\n\r");
  PRINTF("RSSI = %d\n\r", footer[0] );
#endif

  if(footer[1] & FOOTER1_CRC_OK) {
	cc112x_last_rssi = footer[0];
	cc112x_last_lqi = footer[1] & FOOTER1_CORRELATION;


	packetbuf_set_attr(PACKETBUF_ATTR_RSSI, cc112x_last_rssi);
	packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, cc112x_last_lqi);

	RIMESTATS_ADD(llrx);

  } else {
	PRINTF("Bad CRC");
	RIMESTATS_ADD(badcrc);
	len = AUX_LEN;
  }

  RELEASE_LOCK();

  if(len < AUX_LEN) {
	return 0;
  }

  return len - AUX_LEN;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set the transmission power.
 * \param power The transmission power in dB.
 */
void
cc112x_set_txpower(int8_t power)
{
  GET_LOCK();
  set_txpower(power);
  RELEASE_LOCK();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Get the current transmission power.
 * \return The transmission power in dB.
 */
int
cc112x_get_txpower(void)
{
	// TODO
#if 0
  uint8_t pa_value;
  int power = 0;
  int i;

  GET_LOCK();
  pa_value = getreg(CC112X_PATABLE);
  RELEASE_LOCK();

  for(i = 0; i < sizeof(CC112X_PA_LEVELS) / sizeof(CC112X_PA_LEVELS[0]); ++i) {
    if(CC112X_PA_LEVELS[i].value == pa_value) {
      power = CC112X_PA_LEVELS[i].level;
      break;
    }
  }

  return power;
#endif
  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Get the current RSSI.
 * \return The RSSI value in dB.
 */
int
cc112x_rssi(void)
{
  int rssi;
  int radio_was_off = 0;

  if(locked) {
    return 0;
  }

  GET_LOCK();

  if(!receive_on) {
    radio_was_off = 1;
    cc112x_on();
  }

  //BUSYWAIT_UNTIL(getreg(CC112X_PKTSTATUS) & (CC112X_PKTSTATUS_CS | CC112X_PKTSTATUS_CCA), RTIMER_SECOND / 100);

  // Don't compute with RSSI Offset .... assuming CC112X_AGC_GAIN_ADJUST is well programmed, see p34
  rssi = (int)(signed char)getreg(CC112X_RSSI1);

  if(radio_was_off) {
    cc112x_off();
  }
  RELEASE_LOCK();
  return rssi;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Get the current RSSI.
 * \return The RSSI value in dB.
 */
/*
static int
detected_energy(void)
{
  return cc112x_rssi();
}
*/
/*---------------------------------------------------------------------------*/
/**
 * \brief Check if the current CCA value is valid.
 * \return CCA validity status.
 * \return 1 CCA is valid.
 * \return 0 CCA is not valid (Radio was not on for long enough).
 */
int
cc112x_cca_valid(void)
{
  int valid;
  if(locked) {
    return 1;
  }
  GET_LOCK();
  // TODO
  //valid = !!(getreg(CC112X_PKTSTATUS) & (CC112X_PKTSTATUS_CS | CC112X_PKTSTATUS_CCA));
  valid = 1;
  RELEASE_LOCK();
  return valid;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Perform a Clear-Channel Assessment (CCA) to find out if there is a
 * packet in the air or not.
 * \return The CCA result status.
 * \return 1 The channel is clear.
 * \return 0 The channel is not clear.
 */
static int
cc112x_cca(void)
{
#if 0
  int cca;
  int radio_was_off = 0;

  /*
   * If the radio is locked by an underlying thread (because we are
   * being invoked through an interrupt), we preted that the coast is
   * clear (i.e., no packet is currently being transmitted by a
   * neighbor).
   */
  if(locked) {
    return 1;
  }

  GET_LOCK();
  if(!receive_on) {
    radio_was_off = 1;
    cc112x_on();
  }

  /* Make sure that the radio really got turned on. */
  if(!receive_on) {
    RELEASE_LOCK();
    return 1;
  }

  BUSYWAIT_UNTIL(getreg(CC112X_PKTSTATUS) & (CC112X_PKTSTATUS_CS | CC112X_PKTSTATUS_CCA), RTIMER_SECOND / 100);

  cca = !!(getreg(CC112X_PKTSTATUS) & CC112X_PKTSTATUS_CCA);

  if(radio_was_off) {
    cc112x_off();
  }
  RELEASE_LOCK();

  return cca;
#endif
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Check if the radio driver is currently receiving a packet.
 * \return Radio status.
 * \return 1 Radio is receiving a packet.
 * \return 0 Radio is not currently receiving.
 */
int
cc112x_receiving_packet(void)
{
  return CC112X_GPIO0_IS_1 && (status() == CC112X_STATE_RX);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Check if the radio driver has just received a packet.
 * \return Internal buffer status.
 * \return 1 A received packet is pending.
 * \return 0 No packet is pending.
 */
static int
pending_packet(void)
{
  return !!getreg(CC112X_NUM_RXBYTES);
}
/*---------------------------------------------------------------------------*/
/** @} */
