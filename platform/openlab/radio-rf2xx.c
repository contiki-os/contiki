/*
 * This file is part of HiKoB Openlab.
 *
 * HiKoB Openlab is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, version 3.
 *
 * HiKoB Openlab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with HiKoB Openlab. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2011-2014 HiKoB.
 */

/**
 * \file radio-rf2xx.c
 *         This file contains wrappers around the OpenLab rf2Xx periph layer
 *
 * \author
 *         Antoine Fraboulet <antoine.fraboulet.at.hikob.com>
 *         Damien Hedde <damien.hedde.at.hikob.com>
 *         Gaëtan Harter <gaetan.harter.at.inria.fr>
 */

#include <stdlib.h>
#include <string.h>

#include "platform.h"
#define NO_DEBUG_HEADER
#define LOG_LEVEL LOG_LEVEL_ERROR
#include "debug.h"
#include "periph/rf2xx.h"

#include "contiki.h"
#include "contiki-net.h"
#include "sys/rtimer.h"
#include "dev/leds.h"

/*---------------------------------------------------------------------------*/

#ifndef RF2XX_DEVICE
#error "RF2XX_DEVICE is not defined"
#endif
extern rf2xx_t RF2XX_DEVICE;
#ifndef RF2XX_CHANNEL
#define RF2XX_CHANNEL   11
#endif
#ifndef RF2XX_TX_POWER
#define RF2XX_TX_POWER  PHY_POWER_3dBm
#endif
#ifndef RF2XX_RX_RSSI_THRESHOLD
#define RF2XX_RX_RSSI_THRESHOLD  RF2XX_PHY_RX_THRESHOLD__m101dBm
#endif
#ifndef RF2XX_SOFT_PREPARE
/* The RF2xx has a single FIFO for Tx and Rx.
 * - When RF2XX_SOFT_PREPARE is set, rf2xx_wr_prepare merely copies the data to be sent to a soft tx_buf.
 * rf2xx_wr_transmit will copy from tx_buf to the FIFO and then send.
 * - When RF2XX_SOFT_PREPARE is unset, rf2xx_wr_prepare actually copies to the FIFO, and rf2xx_wr_transmit
 * only will only trigger the transmission. */
#define RF2XX_SOFT_PREPARE 1
#endif

/* TSCH requires sending and receiving from interrupt, which requires not to rely on the interrupt-driven state only.
 * Instead, we use rf2xx_reg_write and rf2xx_reg_read in the sending and receiving routines. This, however breaks
 * should the driver be interrupted by an ISR. In TSCH, this never happens as transmissions and receptions are
 * done from rtimer interrupt. Keep this disabled for ContikiMAC and NullRDC. */
#ifndef RF2XX_WITH_TSCH
#define RF2XX_WITH_TSCH 0
#endif

#define RF2XX_MAX_PAYLOAD 125
#if RF2XX_SOFT_PREPARE
static uint8_t tx_buf[RF2XX_MAX_PAYLOAD];
#endif /* RF2XX_SOFT_PREPARE */
static uint8_t tx_len;

enum rf2xx_state
{
    RF_IDLE = 0,
    RF_BUSY,
    RF_TX,
    RF_TX_DONE,
    RF_LISTEN,
    RF_RX,
    RF_RX_DONE,
    RF_RX_READ,
};
static volatile enum rf2xx_state rf2xx_state;
static volatile int rf2xx_on;
static volatile int cca_pending;
static volatile int rf2xx_current_channel;

/* Are we currently in poll mode? */
static uint8_t volatile poll_mode = 0;
/* SFD timestamp of last incoming packet */
static rtimer_clock_t sfd_start_time;

static int read(uint8_t *buf, uint8_t buf_len);
static void listen(void);
static void idle(void);
static void reset(void);
static void restart(void);
static void irq_handler(handler_arg_t arg);

PROCESS(rf2xx_process, "rf2xx driver");

static int rf2xx_wr_on(void);
static int rf2xx_wr_off(void);
static int rf2xx_wr_prepare(const void *, unsigned short);
static int rf2xx_wr_transmit(unsigned short);
static int rf2xx_wr_send(const void *, unsigned short);
static int rf2xx_wr_read(void *, unsigned short);
static int rf2xx_wr_channel_clear(void);
static int rf2xx_wr_receiving_packet(void);
static int rf2xx_wr_pending_packet(void);

/*---------------------------------------------------------------------------*/

static int
rf2xx_wr_init(void)
{
    log_info("radio-rf2xx: rf2xx_wr_init (channel %u)", RF2XX_CHANNEL);

    rf2xx_on = 0;
    cca_pending = 0;
    tx_len = 0;
    rf2xx_state = RF_IDLE;

    reset();
    idle();
    process_start(&rf2xx_process, NULL);

    return 1;
}

/*---------------------------------------------------------------------------*/

/** Copy packet to be sent to the fifo */
static int
rf2xx_wr_hard_prepare(const void *payload, unsigned short payload_le, int async)
{
	uint8_t reg;
	int flag;
    rtimer_clock_t time;

	// Check state
	platform_enter_critical();
	// critical section ensures
	// no packet reception will be started
	flag = 0;
	switch (rf2xx_state)
	{
		case RF_LISTEN:
			flag = 1;
		case RF_IDLE:
			rf2xx_state = RF_TX;
			break;
		default:
			platform_exit_critical();
			return RADIO_TX_COLLISION;
	}
	platform_exit_critical();

	if (flag)
	{
		idle();
	}

	// Read IRQ to clear it
	rf2xx_reg_read(RF2XX_DEVICE, RF2XX_REG__IRQ_STATUS);

	// If radio has external PA, enable DIG3/4
	if (rf2xx_has_pa(RF2XX_DEVICE))
	{
	  // Enable the PA
	  rf2xx_pa_enable(RF2XX_DEVICE);

	  // Activate DIG3/4 pins
	  reg = rf2xx_reg_read(RF2XX_DEVICE, RF2XX_REG__TRX_CTRL_1);
	  reg |= RF2XX_TRX_CTRL_1_MASK__PA_EXT_EN;
	  rf2xx_reg_write(RF2XX_DEVICE, RF2XX_REG__TRX_CTRL_1, reg);
	}

	// Wait until PLL ON state
	time = RTIMER_NOW() + RTIMER_SECOND / 1000;
	do
	{
	  reg = rf2xx_get_status(RF2XX_DEVICE);

	  // Check for block
	  if (RTIMER_CLOCK_LT(time, RTIMER_NOW()))
	  {
	    log_error("radio-rf2xx: Failed to enter tx");
	    restart();
	    return RADIO_TX_ERR;
	  }
	} while (reg != RF2XX_TRX_STATUS__PLL_ON);

	// Copy the packet to the radio FIFO
	rf2xx_fifo_write_first(RF2XX_DEVICE, tx_len + 2);
	if(async) {
	  rf2xx_fifo_write_remaining_async(RF2XX_DEVICE, payload,
	      tx_len, NULL, NULL);
	} else {
	  rf2xx_fifo_write_remaining(RF2XX_DEVICE, payload,
	      tx_len);
	}
	return 0;
}

/** Prepare the radio with a packet to be sent. */
static int
rf2xx_wr_prepare(const void *payload, unsigned short payload_len)
{
    log_debug("radio-rf2xx: rf2xx_wr_prepare %d",payload_len);

    if (payload_len > RF2XX_MAX_PAYLOAD)
    {
        log_error("radio-rf2xx: payload is too big");
        tx_len = 0;
        return 1;
    }

    tx_len = payload_len;

#if RF2XX_SOFT_PREPARE
    memcpy(tx_buf, payload, tx_len);
    return 0;
#else /* RF2XX_SOFT_PREPARE */
    /* Synchronous copy to the FIFO, return */
    return rf2xx_wr_hard_prepare(payload, tx_len, 0);
#endif /* RF2XX_SOFT_PREPARE */
}

/*---------------------------------------------------------------------------*/

/** Send the packet that has previously been prepared. */
static int
rf2xx_wr_transmit(unsigned short transmit_len)
{
    int ret;

    log_info("radio-rf2xx: rf2xx_wr_transmit %d", transmit_len);

    if (tx_len != transmit_len)
    {
        log_error("radio-rf2xx: Length is has changed (was %u now %u)",
                tx_len, transmit_len);
        return RADIO_TX_ERR;
    }

#if RF2XX_SOFT_PREPARE
    /* Asynchronous copy to the FIFO and before starting to transmit */
    ret = rf2xx_wr_hard_prepare(tx_buf, tx_len, 1);
    if(ret != 0) {
      return ret;
    }
#endif /* RF2XX_SOFT_PREPARE */

#ifdef RF2XX_LEDS_ON
    if (transmit_len > 10)
    {
        leds_on(LEDS_RED);
    }
#endif

    // Enable IRQ interrupt
    rf2xx_irq_enable(RF2XX_DEVICE);
    // Start TX
    rf2xx_slp_tr_set(RF2XX_DEVICE);

    if(!RF2XX_WITH_TSCH) {
      // Wait until the end of the packet
      while (rf2xx_state == RF_TX);
      ret = (rf2xx_state == RF_TX_DONE) ? RADIO_TX_OK : RADIO_TX_ERR;
    } else {
      // Wait until the transmission starts and ends
      while(!(rf2xx_get_status(RF2XX_DEVICE) == RF2XX_TRX_STATUS__BUSY_TX));
      while(!((rf2xx_get_status(RF2XX_DEVICE) != RF2XX_TRX_STATUS__BUSY_TX) || (rf2xx_state != RF_TX)));
      ret = RADIO_TX_OK;
    }


#ifdef RF2XX_LEDS_ON
    leds_off(LEDS_RED);
#endif

    restart();
    return ret;
}

/*---------------------------------------------------------------------------*/

/** Prepare & transmit a packet. */
static int
rf2xx_wr_send(const void *payload, unsigned short payload_len)
{
    log_debug("radio-rf2xx: rf2xx_wr_send %d", payload_len);
    if (rf2xx_wr_prepare(payload, payload_len))
    {
        return RADIO_TX_ERR;
    }
    return rf2xx_wr_transmit(payload_len);
}

/*---------------------------------------------------------------------------*/

/** Read a received packet into a buffer. */
static int
rf2xx_wr_read(void *buf, unsigned short buf_len)
{
    int len;
    log_info("radio-rf2xx: rf2xx_wr_read %d", buf_len);

    // Is there a packet pending
    platform_enter_critical();
    if (rf2xx_wr_pending_packet() == 0)
    {
        platform_exit_critical();
        return 0;
    }
    rf2xx_state = RF_RX_READ;
    platform_exit_critical();

    // Get the packet
    len = read(buf, buf_len);

    restart();
    return len;
}

/*---------------------------------------------------------------------------*/

/** Perform a Clear-Channel Assessment (CCA) to find out if there is
    a packet in the air or not. */
static int
rf2xx_wr_channel_clear(void)
{
    int clear = 1;
    log_debug("radio-rf2xx: rf2xx_wr_channel_clear");

    // critical section is necessary
    // to avoid spi access conflicts
    // with irq_handler
    switch (rf2xx_state)
    {
        uint8_t reg;
        case RF_LISTEN:
            //initiate a cca request
            platform_enter_critical();
            reg = RF2XX_PHY_CC_CCA_DEFAULT__CCA_MODE |
                (rf2xx_current_channel & RF2XX_PHY_CC_CCA_MASK__CHANNEL) |
                RF2XX_PHY_CC_CCA_MASK__CCA_REQUEST;
            rf2xx_reg_write(RF2XX_DEVICE, RF2XX_REG__PHY_CC_CCA, reg);
            platform_exit_critical();

            // wait cca to be done
            do
            {
                platform_enter_critical();
                reg = rf2xx_reg_read(RF2XX_DEVICE, RF2XX_REG__TRX_STATUS);
                platform_exit_critical();
            }
            while (rf2xx_state == RF_LISTEN && !(reg & RF2XX_TRX_STATUS_MASK__CCA_DONE));

            // get result
            if (!(reg & RF2XX_TRX_STATUS_MASK__CCA_STATUS))
            {
                clear = 0;
            }
            break;

        case RF_RX:
            clear = 0;
            break;

        default:
            break;
    }

    return clear;
}

/*---------------------------------------------------------------------------*/

/** Check if the radio driver is currently receiving a packet */
static int
rf2xx_wr_receiving_packet(void)
{
  if(!RF2XX_WITH_TSCH) {
    return (rf2xx_state == RF_RX) ? 1 : 0;
  } else {
    return (rf2xx_state == RF_RX) || rf2xx_get_status(RF2XX_DEVICE) == RF2XX_TRX_STATUS__BUSY_RX;
  }
}

/*---------------------------------------------------------------------------*/

/** Check if the radio driver has just received a packet */
static int
rf2xx_wr_pending_packet(void)
{
  if(!RF2XX_WITH_TSCH) {
    return (rf2xx_state == RF_RX_DONE) ? 1 : 0;
  } else {
    if(rf2xx_reg_read(RF2XX_DEVICE, RF2XX_REG__IRQ_STATUS) & RF2XX_IRQ_STATUS_MASK__TRX_END) {
      rf2xx_state = RF_RX_DONE;
    }
    return rf2xx_state == RF_RX_DONE;
  }
}

/*---------------------------------------------------------------------------*/

/** Turn the radio on. */
static int
rf2xx_wr_on(void)
{
    int flag = 0;
    log_debug("radio-rf2xx: rf2xx_wr_on");

    platform_enter_critical();
    if (!rf2xx_on)
    {
        rf2xx_on = 1;
        if (rf2xx_state == RF_IDLE)
        {
            flag = 1;
            rf2xx_state = RF_BUSY;
        }
    }
    platform_exit_critical();

    if (flag)
    {
        listen();
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

/** Turn the radio off. */
static int
rf2xx_wr_off(void)
{
    int flag = 0;
    log_debug("radio-rf2xx: rf2xx_wr_off");

    platform_enter_critical();
    if (rf2xx_on)
    {
        rf2xx_on = 0;
        if (rf2xx_state == RF_LISTEN)
        {
            flag = 1;
            rf2xx_state = RF_BUSY;
        }
    }
    platform_exit_critical();

    if (flag)
    {
        idle();
        rf2xx_state = RF_IDLE;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/
/* Enable or disable poll mode */
static void
set_poll_mode(uint8_t enable)
{
  poll_mode = enable;
}

static void
set_channel(uint8_t channel)
{
  uint8_t reg = RF2XX_PHY_CC_CCA_DEFAULT__CCA_MODE |
        (channel & RF2XX_PHY_CC_CCA_MASK__CHANNEL);
  platform_enter_critical();
  rf2xx_current_channel = channel;
  rf2xx_reg_write(RF2XX_DEVICE, RF2XX_REG__PHY_CC_CCA, reg);
  platform_exit_critical();
}

static uint8_t
get_channel()
{
  platform_enter_critical();
  uint8_t reg = rf2xx_reg_read(RF2XX_DEVICE, RF2XX_REG__PHY_CC_CCA);
  platform_exit_critical();
  return reg & RF2XX_PHY_CC_CCA_MASK__CHANNEL;
}

static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  int v;

  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }
  switch(param) {
  case RADIO_PARAM_RX_MODE:
    *value = 0;
    /* No frame filtering, no autoack */
    if(!poll_mode) {
      *value |= RADIO_RX_MODE_POLL_MODE;
    }
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TX_MODE:
      *value = 0; /* Mode is always 0 (send-on-cca not supported yet) */
      return RADIO_RESULT_OK;
  case RADIO_PARAM_CHANNEL:
    *value = get_channel();
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}

static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  switch(param) {
  case RADIO_PARAM_RX_MODE:
    if(value & ~(RADIO_RX_MODE_ADDRESS_FILTER |
        RADIO_RX_MODE_AUTOACK | RADIO_RX_MODE_POLL_MODE)) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    if(value & RADIO_RX_MODE_ADDRESS_FILTER) {
      /* Frame filtering not supported yet */
      return RADIO_RESULT_INVALID_VALUE;
    }
    if(value & RADIO_RX_MODE_AUTOACK) {
      /* Autoack not supported yet */
      return RADIO_RESULT_INVALID_VALUE;
    }
    set_poll_mode((value & RADIO_RX_MODE_POLL_MODE) != 0);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TX_MODE:
    if(value != 0) { /* We support only mode 0 (send-on-cca not supported yet) */
      return RADIO_RESULT_INVALID_VALUE;
    }
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CHANNEL:
    if(value < 11 || value > 26) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    set_channel(value);
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}

static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  if(param == RADIO_PARAM_LAST_PACKET_TIMESTAMP) {
    *(rtimer_clock_t*)dest = sfd_start_time;
    return RADIO_RESULT_OK;
  }
  return RADIO_RESULT_NOT_SUPPORTED;
}

static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

const struct radio_driver rf2xx_driver =
  {
    .init             = rf2xx_wr_init,
    .prepare          = rf2xx_wr_prepare,
    .transmit         = rf2xx_wr_transmit,
    .send             = rf2xx_wr_send,
    .read             = rf2xx_wr_read,
    .channel_clear    = rf2xx_wr_channel_clear,
    .receiving_packet = rf2xx_wr_receiving_packet,
    .pending_packet   = rf2xx_wr_pending_packet,
    .on               = rf2xx_wr_on,
    .off              = rf2xx_wr_off,
    .get_value        = get_value,
    .set_value        = set_value,
    .get_object       = get_object,
    .set_object       = set_object,
 };

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(rf2xx_process, ev, data)
{
    PROCESS_BEGIN();

    while(1)
    {
        static int len;
        static int flag;
        PROCESS_YIELD_UNTIL(!poll_mode && ev == PROCESS_EVENT_POLL);

        /*
         * at this point, we may be in any state
         *
         * this process can be 'interrupted' by rtimer tasks
         * such as contikimac rdc listening task
         * which may call on/off/read/receiving/pending
         */

        len = 0;
        flag = 0;
        platform_enter_critical();
        if (rf2xx_state == RF_RX_DONE)
        {
            // the process will do the read
            rf2xx_state = RF_RX_READ;
            flag = 1;
        }
        platform_exit_critical();

        if (flag)
        {
            // get data
            packetbuf_clear();
            len = read(packetbuf_dataptr(), PACKETBUF_SIZE);

            restart();

            // eventually call upper layer
            if (len > 0)
            {
                packetbuf_set_datalen(len);
                NETSTACK_RDC.input();
            }
        }
    }

    PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static int convert_power(phy_power_t power)
{
    // Convert power value to register value
    switch (power)
    {
        case PHY_POWER_m30dBm:
        case PHY_POWER_m29dBm:
        case PHY_POWER_m28dBm:
        case PHY_POWER_m27dBm:
        case PHY_POWER_m26dBm:
        case PHY_POWER_m25dBm:
        case PHY_POWER_m24dBm:
        case PHY_POWER_m23dBm:
        case PHY_POWER_m22dBm:
        case PHY_POWER_m21dBm:
        case PHY_POWER_m20dBm:
        case PHY_POWER_m19dBm:
        case PHY_POWER_m18dBm:
        case PHY_POWER_m17dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__m17dBm;
            break;
        case PHY_POWER_m16dBm:
        case PHY_POWER_m15dBm:
        case PHY_POWER_m14dBm:
        case PHY_POWER_m13dBm:
        case PHY_POWER_m12dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__m12dBm;
            break;
        case PHY_POWER_m11dBm:
        case PHY_POWER_m10dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__m9dBm;
            break;
        case PHY_POWER_m9dBm:
        case PHY_POWER_m8dBm:
        case PHY_POWER_m7dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__m7dBm;
            break;
        case PHY_POWER_m6dBm:
        case PHY_POWER_m5dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__m5dBm;
            break;
        case PHY_POWER_m4dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__m4dBm;
            break;
        case PHY_POWER_m3dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__m3dBm;
            break;
        case PHY_POWER_m2dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__m2dBm;
            break;
        case PHY_POWER_m1dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__m1dBm;
            break;
        case PHY_POWER_0dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__0dBm;
            break;
        case PHY_POWER_0_7dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__0_7dBm;
            break;
        case PHY_POWER_1dBm:
        case PHY_POWER_1_3dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__1_3dBm;
            break;
        case PHY_POWER_1_8dBm:
        case PHY_POWER_2dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__1_8dBm;
            break;
        case PHY_POWER_2_3dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__2_3dBm;
            break;
        case PHY_POWER_2_8dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__2_8dBm;
            break;
        case PHY_POWER_3dBm:
        case PHY_POWER_4dBm:
        case PHY_POWER_5dBm:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__3dBm;
            break;
        default:
            power = RF2XX_PHY_TX_PWR_TX_PWR_VALUE__0dBm;
            break;
    }
    return power;
}

static void reset(void)
{
    uint8_t reg;
    int rf_tx_power = convert_power(RF2XX_TX_POWER);

    // Stop any Asynchronous access
    rf2xx_fifo_access_cancel(RF2XX_DEVICE);

    // Configure the radio interrupts
    rf2xx_irq_disable(RF2XX_DEVICE);
    rf2xx_irq_configure(RF2XX_DEVICE, irq_handler, NULL);

    // Disable DIG2 pin
    if (rf2xx_has_dig2(RF2XX_DEVICE))
    {
        rf2xx_dig2_disable(RF2XX_DEVICE);
    }

    // Reset the SLP_TR output
    rf2xx_slp_tr_clear(RF2XX_DEVICE);

    // Reset the radio chip
    rf2xx_reset(RF2XX_DEVICE);

    // Enable Dynamic Frame Buffer Protection, standard data rate (250kbps)
    reg = RF2XX_TRX_CTRL_2_MASK__RX_SAFE_MODE;
    rf2xx_reg_write(RF2XX_DEVICE, RF2XX_REG__TRX_CTRL_2, reg);

    // Set max TX power
    reg = RF2XX_PHY_TX_PWR_DEFAULT__PA_BUF_LT
            | RF2XX_PHY_TX_PWR_DEFAULT__PA_LT
            | rf_tx_power;
    rf2xx_reg_write(RF2XX_DEVICE, RF2XX_REG__PHY_TX_PWR, reg);

    // Disable CLKM signal
    reg = RF2XX_TRX_CTRL_0_DEFAULT__PAD_IO
            | RF2XX_TRX_CTRL_0_DEFAULT__PAD_IO_CLKM
            | RF2XX_TRX_CTRL_0_DEFAULT__CLKM_SHA_SEL
            | RF2XX_TRX_CTRL_0_CLKM_CTRL__OFF;
    rf2xx_reg_write(RF2XX_DEVICE, RF2XX_REG__TRX_CTRL_0, reg);

    /** Set XCLK TRIM
     * \todo this highly depends on the board
     */
    reg = RF2XX_XOSC_CTRL__XTAL_MODE_CRYSTAL | 0x0;
    rf2xx_reg_write(RF2XX_DEVICE, RF2XX_REG__XOSC_CTRL, reg);

    // Set channel
    reg = RF2XX_PHY_CC_CCA_DEFAULT__CCA_MODE |
        (RF2XX_CHANNEL & RF2XX_PHY_CC_CCA_MASK__CHANNEL);
    rf2xx_reg_write(RF2XX_DEVICE, RF2XX_REG__PHY_CC_CCA, reg);
    set_channel(RF2XX_CHANNEL);

    // Set IRQ to TRX END/RX_START/CCA_DONE
    rf2xx_reg_write(RF2XX_DEVICE, RF2XX_REG__IRQ_MASK,
            RF2XX_IRQ_STATUS_MASK__TRX_END |
            RF2XX_IRQ_STATUS_MASK__RX_START);
    set_poll_mode(poll_mode);

    reg = rf2xx_reg_read(RF2XX_DEVICE, RF2XX_REG__RX_SYN);
    reg &= 0xF0;
    reg |= (0x0F & RF2XX_RX_RSSI_THRESHOLD);
    rf2xx_reg_write(RF2XX_DEVICE, RF2XX_REG__RX_SYN, reg);
}


/*---------------------------------------------------------------------------*/

static void idle(void)
{
    // Disable the interrupts
    rf2xx_irq_disable(RF2XX_DEVICE);

    // Cancel any ongoing transfer
    rf2xx_fifo_access_cancel(RF2XX_DEVICE);

    // Clear slp/tr
    rf2xx_slp_tr_clear(RF2XX_DEVICE);

    // Force IDLE
    rf2xx_set_state(RF2XX_DEVICE, RF2XX_TRX_STATE__FORCE_PLL_ON);

    // If radio has external PA, disable DIG3/4
    if (rf2xx_has_pa(RF2XX_DEVICE))
    {
        // Enable the PA
        rf2xx_pa_disable(RF2XX_DEVICE);

        // De-activate DIG3/4 pins
        uint8_t reg = rf2xx_reg_read(RF2XX_DEVICE, RF2XX_REG__TRX_CTRL_1);
        reg &= ~RF2XX_TRX_CTRL_1_MASK__PA_EXT_EN;
        rf2xx_reg_write(RF2XX_DEVICE, RF2XX_REG__TRX_CTRL_1, reg);
    }
}

/*---------------------------------------------------------------------------*/

static void listen(void)
{
    uint8_t reg;

    // Read IRQ to clear it
    rf2xx_reg_read(RF2XX_DEVICE, RF2XX_REG__IRQ_STATUS);

    // If radio has external PA, enable DIG3/4
    if (rf2xx_has_pa(RF2XX_DEVICE))
    {
        // Enable the PA
        rf2xx_pa_enable(RF2XX_DEVICE);

        // Activate DIG3/4 pins
        reg = rf2xx_reg_read(RF2XX_DEVICE, RF2XX_REG__TRX_CTRL_1);
        reg |= RF2XX_TRX_CTRL_1_MASK__PA_EXT_EN;
        rf2xx_reg_write(RF2XX_DEVICE, RF2XX_REG__TRX_CTRL_1, reg);
    }

    // Enable IRQ interrupt
    rf2xx_irq_enable(RF2XX_DEVICE);

    // Start RX
    platform_enter_critical();
    rf2xx_state = RF_LISTEN;
    rf2xx_set_state(RF2XX_DEVICE, RF2XX_TRX_STATE__RX_ON);
    platform_exit_critical();
}

/*---------------------------------------------------------------------------*/

static void restart(void)
{
    idle();

    if (rf2xx_on)
    {
        listen();
    }
    else
    {
        rf2xx_state = RF_IDLE;
    }
}

/*---------------------------------------------------------------------------*/

static int read(uint8_t *buf, uint8_t buf_len)
{
    uint8_t len;
    // Check the CRC is good
    if (!(rf2xx_reg_read(RF2XX_DEVICE, RF2XX_REG__PHY_RSSI)
            & RF2XX_PHY_RSSI_MASK__RX_CRC_VALID))
    {
        log_warning("radio-rf2xx: Received packet with bad crc");
        return 0;
    }

#ifdef RF2XX_LEDS_ON
        leds_on(LEDS_GREEN);
#endif

    // Get payload length
    len = rf2xx_fifo_read_first(RF2XX_DEVICE) - 2;
    log_info("radio-rf2xx: Received packet of length: %u", len);

    // Check valid length (not zero and enough space to store it)
    if (len > buf_len)
    {
        log_warning("radio-rf2xx: Received packet is too big (%u)", len);
        // Error length, end transfer
        rf2xx_fifo_read_remaining(RF2XX_DEVICE, buf, 0);
        return 0;
    }

    // Read payload
    rf2xx_fifo_read_remaining(RF2XX_DEVICE, buf, len);

#ifdef RF2XX_LEDS_ON
        leds_off(LEDS_GREEN);
#endif

    return len;
}

/*---------------------------------------------------------------------------*/

static void irq_handler(handler_arg_t arg)
{
    (void) arg;
    uint8_t reg;
    int state = rf2xx_state;
    switch (state)
    {
        case RF_TX:
        case RF_LISTEN:
        case RF_RX:
            break;
        default:
            log_warning("radio-rf2xx: unexpected irq while state %d", state);
            // may eventually happen when transitioning
            // from listen to idle for example
            return;
    }

    // only read irq_status in somes states to avoid any
    // concurrency problem on spi access
    reg = rf2xx_reg_read(RF2XX_DEVICE, RF2XX_REG__IRQ_STATUS);

    // rx start detection
    if (reg & RF2XX_IRQ_STATUS_MASK__RX_START && state == RF_LISTEN)
    {
        rf2xx_state = state = RF_RX;
        sfd_start_time = RTIMER_NOW();
    }

    // rx/tx end
    if (reg & RF2XX_IRQ_STATUS_MASK__TRX_END)
    {
        switch (state)
        {
            case RF_TX:
                rf2xx_state = state = RF_TX_DONE;
                break;
            case RF_RX:
            case RF_LISTEN:
                rf2xx_state = state = RF_RX_DONE;
                // we do not want to start a 2nd RX
                rf2xx_set_state(RF2XX_DEVICE, RF2XX_TRX_STATE__PLL_ON);
                if(!poll_mode) {
                  /* In poll mode, do not wakeup the process. Rather, the upper layer will poll
                   * for incoming packets */
                  process_poll(&rf2xx_process);
                }
                break;
        }
    }
}
