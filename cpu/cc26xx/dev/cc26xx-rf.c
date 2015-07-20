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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-rf
 * @{
 *
 * \file
 * Implementation of the CC26xx RF driver
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/radio.h"
#include "dev/cc26xx-rf.h"
#include "dev/oscillators.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/linkaddr.h"
#include "net/netstack.h"
#include "sys/energest.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "sys/cc.h"
#include "lpm.h"
#include "ti-lib.h"
/*---------------------------------------------------------------------------*/
/* RF core and RF HAL API */
#include "hw_rfc_dbell.h"
#include "hw_rfc_pwr.h"
/*---------------------------------------------------------------------------*/
/* RF Core Mailbox API */
#include "mailbox.h"
#include "common_cmd.h"
#include "common_cmd_field.h"
#include "ble_cmd.h"
#include "ble_cmd_field.h"
#include "ieee_cmd.h"
#include "ieee_cmd_field.h"
#include "data_entry.h"
#include "ble_mailbox.h"
#include "ieee_mailbox.h"
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#define BUSYWAIT_UNTIL(cond, max_time)                                \
  do {                                                                \
    rtimer_clock_t t0;                                                \
    t0 = RTIMER_NOW();                                                \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))); \
  } while(0)
/*---------------------------------------------------------------------------*/
#ifdef __GNUC__
#define CC_ALIGN_ATTR(n) __attribute__ ((aligned(n)))
#else
#define CC_ALIGN_ATTR(n)
#endif
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#ifdef CC26XX_CONF_DEBUG_CRC
#define CC26XX_DEBUG_CRC CC26XX_CONF_DEBUG_CRC
#else /* CC26XX_CONF_DEBUG_CRC */
#define CC26XX_DEBUG_CRC DEBUG
#endif /* CC26XX_CONF_DEBUG_CRC */
/*---------------------------------------------------------------------------*/
/* Data entry status field constants */
#define DATA_ENTRY_STATUS_PENDING    0x00 /* Not in use by the Radio CPU */
#define DATA_ENTRY_STATUS_ACTIVE     0x01 /* Open for r/w by the radio CPU */
#define DATA_ENTRY_STATUS_BUSY       0x02 /* Ongoing r/w */
#define DATA_ENTRY_STATUS_FINISHED   0x03 /* Free to use and to free */
#define DATA_ENTRY_STATUS_UNFINISHED 0x04 /* Partial RX entry */
/*---------------------------------------------------------------------------*/
/* RF stats data structure */
static uint8_t rf_stats[16] = { 0 };
/*---------------------------------------------------------------------------*/
/* RF Command status constants - Correspond to values in the CMDSTA register */
#define RF_CMD_STATUS_PENDING         0x00
#define RF_CMD_STATUS_DONE            0x01
#define RF_CMD_STATUS_ILLEGAL_PTR     0x81
#define RF_CMD_STATUS_UNKNOWN_CMD     0x82
#define RF_CMD_STATUS_UNKNOWN_DIR_CMD 0x83
#define RF_CMD_STATUS_CONTEXT_ERR     0x85
#define RF_CMD_STATUS_SCHEDULING_ERR  0x86
#define RF_CMD_STATUS_PAR_ERR         0x87
#define RF_CMD_STATUS_QUEUE_ERR       0x88
#define RF_CMD_STATUS_QUEUE_BUSY      0x89

/* Status values starting with 0x8 correspond to errors */
#define RF_CMD_STATUS_ERR_MASK        0x80

/* Return values for rf_send_cmd_ok */
#define RF_CMD_ERROR                     0
#define RF_CMD_OK                        1

/* The size of the RF commands buffer */
#define RF_CMD_BUFFER_SIZE             128
/*---------------------------------------------------------------------------*/
/* RF Radio Op status constants. Field 'status' in Radio Op command struct */
#define RF_RADIO_OP_STATUS_IDLE             0x0000
#define RF_RADIO_OP_STATUS_PENDING          0x0001
#define RF_RADIO_OP_STATUS_ACTIVE           0x0002
#define RF_RADIO_OP_STATUS_SKIPPED          0x0003
#define RF_RADIO_OP_STATUS_DONE_OK          0x0400
#define RF_RADIO_OP_STATUS_DONE_COUNTDOWN   0x0401
#define RF_RADIO_OP_STATUS_DONE_RXERR       0x0402
#define RF_RADIO_OP_STATUS_DONE_TIMEOUT     0x0403
#define RF_RADIO_OP_STATUS_DONE_STOPPED     0x0404
#define RF_RADIO_OP_STATUS_DONE_ABORT       0x0405
#define RF_RADIO_OP_STATUS_ERROR_PAST_START 0x0800
#define RF_RADIO_OP_STATUS_ERROR_START_TRIG 0x0801
#define RF_RADIO_OP_STATUS_ERROR_CONDITION  0x0802
#define RF_RADIO_OP_STATUS_ERROR_PAR        0x0803
#define RF_RADIO_OP_STATUS_ERROR_POINTER    0x0804
#define RF_RADIO_OP_STATUS_ERROR_CMDID      0x0805
#define RF_RADIO_OP_STATUS_ERROR_NO_SETUP   0x0807
#define RF_RADIO_OP_STATUS_ERROR_NO_FS      0x0808
#define RF_RADIO_OP_STATUS_ERROR_SYNTH_PROG 0x0809

/* Additional Op status values for IEEE mode */
#define RF_RADIO_OP_STATUS_IEEE_SUSPENDED           0x2001
#define RF_RADIO_OP_STATUS_IEEE_DONE_OK             0x2400
#define RF_RADIO_OP_STATUS_IEEE_DONE_BUSY           0x2401
#define RF_RADIO_OP_STATUS_IEEE_DONE_STOPPED        0x2402
#define RF_RADIO_OP_STATUS_IEEE_DONE_ACK            0x2403
#define RF_RADIO_OP_STATUS_IEEE_DONE_ACKPEND        0x2404
#define RF_RADIO_OP_STATUS_IEEE_DONE_TIMEOUT        0x2405
#define RF_RADIO_OP_STATUS_IEEE_DONE_BGEND          0x2406
#define RF_RADIO_OP_STATUS_IEEE_DONE_ABORT          0x2407
#define RF_RADIO_OP_STATUS_ERROR_WRONG_BG           0x0806
#define RF_RADIO_OP_STATUS_IEEE_ERROR_PAR           0x2800
#define RF_RADIO_OP_STATUS_IEEE_ERROR_NO_SETUP      0x2801
#define RF_RADIO_OP_STATUS_IEEE_ERROR_NO_FS         0x2802
#define RF_RADIO_OP_STATUS_IEEE_ERROR_SYNTH_PROG    0x2803
#define RF_RADIO_OP_STATUS_IEEE_ERROR_RXOVF         0x2804
#define RF_RADIO_OP_STATUS_IEEE_ERROR_TXUNF         0x2805

/* Op status values for BLE mode */
#define RF_RADIO_OP_STATUS_BLE_DONE_OK              0x1400
#define RF_RADIO_OP_STATUS_BLE_DONE_RXTIMEOUT       0x1401
#define RF_RADIO_OP_STATUS_BLE_DONE_NOSYNC          0x1402
#define RF_RADIO_OP_STATUS_BLE_DONE_RXERR           0x1403
#define RF_RADIO_OP_STATUS_BLE_DONE_CONNECT         0x1404
#define RF_RADIO_OP_STATUS_BLE_DONE_MAXNACK         0x1405
#define RF_RADIO_OP_STATUS_BLE_DONE_ENDED           0x1406
#define RF_RADIO_OP_STATUS_BLE_DONE_ABORT           0x1407
#define RF_RADIO_OP_STATUS_BLE_DONE_STOPPED         0x1408
#define RF_RADIO_OP_STATUS_BLE_ERROR_PAR            0x1800
#define RF_RADIO_OP_STATUS_BLE_ERROR_RXBUF          0x1801
#define RF_RADIO_OP_STATUS_BLE_ERROR_NO_SETUP       0x1802
#define RF_RADIO_OP_STATUS_BLE_ERROR_NO_FS          0x1803
#define RF_RADIO_OP_STATUS_BLE_ERROR_SYNTH_PROG     0x1804
#define RF_RADIO_OP_STATUS_BLE_ERROR_RXOVF          0x1805
#define RF_RADIO_OP_STATUS_BLE_ERROR_TXUNF          0x1806

/* Bits 15:12 signify the protocol */
#define RF_RADIO_OP_STATUS_PROTO_MASK               0xF000
#define RF_RADIO_OP_STATUS_PROTO_GENERIC            0x0000
#define RF_RADIO_OP_STATUS_PROTO_BLE                0x1000
#define RF_RADIO_OP_STATUS_PROTO_IEEE               0x2000
#define RF_RADIO_OP_STATUS_PROTO_PROP               0x3000

/* Bits 11:10 signify Running / Done OK / Done with error */
#define RF_RADIO_OP_MASKED_STATUS           0x0C00
#define RF_RADIO_OP_MASKED_STATUS_RUNNING   0x0000
#define RF_RADIO_OP_MASKED_STATUS_DONE      0x0400
#define RF_RADIO_OP_MASKED_STATUS_ERROR     0x0800
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns the current status of a running Radio Op command
 * \param a A pointer with the buffer used to initiate the command
 * \return The value of the Radio Op buffer's status field
 *
 * This macro can be used to e.g. return the status of a previously
 * initiated background operation, or of an immediate command
 */
#define RF_RADIO_OP_GET_STATUS(a) GET_FIELD_V(a, radioOp, status)
/*---------------------------------------------------------------------------*/
/* Special value returned by CMD_IEEE_CCA_REQ when an RSSI is not available */
#define RF_CMD_CCA_REQ_RSSI_UNKNOWN     -128

/* Used for the return value of channel_clear */
#define RF_CCA_CLEAR                       1
#define RF_CCA_BUSY                        0

/* Used as an error return value for get_cca_info */
#define RF_GET_CCA_INFO_ERROR           0xFF

/*
 * Values of the individual bits of the ccaInfo field in CMD_IEEE_CCA_REQ's
 * status struct
 */
#define RF_CMD_CCA_REQ_CCA_STATE_IDLE      0 /* 00 */
#define RF_CMD_CCA_REQ_CCA_STATE_BUSY      1 /* 01 */
#define RF_CMD_CCA_REQ_CCA_STATE_INVALID   2 /* 10 */
/*---------------------------------------------------------------------------*/
#define RF_MODE_BLE  0
#define RF_MODE_IEEE 1
/*---------------------------------------------------------------------------*/
/* How long to wait for an ongoing ACK TX to finish before starting frame TX */
#define TX_WAIT_TIMEOUT       (RTIMER_SECOND >> 11)

/* How long to wait for the RF to enter RX in rf_cmd_ieee_rx */
#define ENTER_RX_WAIT_TIMEOUT (RTIMER_SECOND >> 10)
/*---------------------------------------------------------------------------*/
/* TX Power dBm lookup table - values from SmartRF Studio */
typedef struct output_config {
  radio_value_t dbm;
  uint8_t register_ib;
  uint8_t register_gc;
} output_config_t;

static const output_config_t output_power[] = {
  { 5, 0x29, 0x00 },
  { 4, 0x20, 0x00 },
  { 3, 0x19, 0x00 },
  { 2, 0x25, 0x01 },
  { 1, 0x21, 0x01 },
  { 0, 0x1D, 0x01 },
  { -3, 0x19, 0x03 },
  { -6, 0x13, 0x03 },
  { -9, 0x0F, 0x03 },
};

#define OUTPUT_CONFIG_COUNT (sizeof(output_power) / sizeof(output_config_t))

/* Max and Min Output Power in dBm */
#define OUTPUT_POWER_MIN     (output_power[OUTPUT_CONFIG_COUNT - 1].dbm)
#define OUTPUT_POWER_MAX     (output_power[0].dbm)
#define OUTPUT_POWER_UNKNOWN 0xFFFF

/* Default TX Power - position in output_power[] */
#define CC26XX_RF_TX_POWER 0
const output_config_t *tx_power_current = &output_power[0];
/*---------------------------------------------------------------------------*/
#define RF_CORE_CLOCKS_MASK (RFC_PWR_PWMCLKEN_RFC_M | RFC_PWR_PWMCLKEN_CPE_M \
                             | RFC_PWR_PWMCLKEN_CPERAM_M)
/*---------------------------------------------------------------------------*/
/* RF interrupts */
#define RX_IRQ       IRQ_IEEE_RX_ENTRY_DONE
#define RX_NOK_IRQ   IRQ_IEEE_RX_NOK
#define TX_ACK_IRQ   IRQ_IEEE_TX_ACK
#define ERROR_IRQ    IRQ_INTERNAL_ERROR

/* Those IRQs are enabled all the time */
#if CC26XX_DEBUG_CRC
#define ENABLED_IRQS (RX_IRQ | ERROR_IRQ | RX_NOK_IRQ)
#else
#define ENABLED_IRQS (RX_IRQ | ERROR_IRQ)
#endif

/*
 * We only enable this right before starting frame TX, so we can sleep while
 * the TX is ongoing
 */
#define LAST_FG_CMD_DONE  IRQ_LAST_FG_COMMAND_DONE

#define cc26xx_rf_cpe0_isr RFCCPE0IntHandler
#define cc26xx_rf_cpe1_isr RFCCPE1IntHandler
/*---------------------------------------------------------------------------*/
/*
 * Buffers used to send commands to the RF core (generic and IEEE commands).
 * Some of those buffers are re-usable, some are not.
 *
 * If you are uncertain, declare a new buffer.
 */
/*
 * A buffer to send a CMD_IEEE_RX and to subsequently monitor its status
 * Do not use this buffer for any commands other than CMD_IEEE_RX
 */
static uint8_t cmd_ieee_rx_buf[RF_CMD_BUFFER_SIZE] CC_ALIGN_ATTR(4);

/*
 * A buffer used to send immediate and foreground Radio Op (e.g. CMD_IEEE_TX)
 * commands.
 *
 * Do not re-use this buffer to send a command before the previous command
 * has been completed.
 *
 * Do not intermingle preparation of this buffer to send a command with calls
 * that might lead to a different command, since the latter will overwrite what
 * you have written in preparation for the former.
 */
static uint8_t cmd_immediate_buf[RF_CMD_BUFFER_SIZE] CC_ALIGN_ATTR(4);
/*---------------------------------------------------------------------------*/
/* BLE macros, variables and buffers */

/* BLE Intervals: Send a burst of advertisements every BLE_ADV_INTERVAL secs */
#define BLE_ADV_INTERVAL      (CLOCK_SECOND * 5)
#define BLE_ADV_DUTY_CYCLE    (CLOCK_SECOND / 10)
#define BLE_ADV_MESSAGES      10

/* BLE Advertisement-related macros */
#define BLE_ADV_TYPE_DEVINFO      0x01
#define BLE_ADV_TYPE_NAME         0x09
#define BLE_ADV_TYPE_MANUFACTURER 0xFF
#define BLE_ADV_NAME_BUF_LEN        32
#define BLE_ADV_PAYLOAD_BUF_LEN     64
#define BLE_UUID_SIZE               16

#if CC26XX_RF_BLE_SUPPORT
/* BLE buffers / variables */
static unsigned char ble_cmd_buf[32] CC_ALIGN_ATTR(4) = { 0 };
static unsigned char ble_tx_rx_buf[128] CC_ALIGN_ATTR(4);
static uint8_t ble_mode_on;

/* BLE beacond config */
static struct ble_beacond_config {
  clock_time_t interval;
  char adv_name[BLE_ADV_NAME_BUF_LEN];
} beacond_config;

/* BLE overrides */
static uint32_t ble_overrides[] = {
  0x00364038, /* Synth: Set RTRIM (POTAILRESTRIM) to 6 */
  0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */
  0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */
  0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */
  0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */
  0x00456088, /* Adjust AGC reference level */
  0xFFFFFFFF, /* End of override list */
};

PROCESS(cc26xx_rf_ble_beacon_process, "CC26xx RF BLE Beacon Process");

static void init_ble(void);
#else
#define init_ble(...)
#endif /* CC26XX_RF_BLE_SUPPORT */
/*---------------------------------------------------------------------------*/
#define RX_BUF_SIZE 140
/* Four receive buffers entries with room for 1 IEEE802.15.4 frame in each */
static uint8_t rx_buf_0[RX_BUF_SIZE] CC_ALIGN_ATTR(4);
static uint8_t rx_buf_1[RX_BUF_SIZE] CC_ALIGN_ATTR(4);
static uint8_t rx_buf_2[RX_BUF_SIZE] CC_ALIGN_ATTR(4);
static uint8_t rx_buf_3[RX_BUF_SIZE] CC_ALIGN_ATTR(4);

/* The RX Data Queue */
static dataQueue_t rx_data_queue = { 0 };

/* Receive entry pointer to keep track of read items */
volatile static uint8_t *rx_read_entry;
/*---------------------------------------------------------------------------*/
/* The outgoing frame buffer */
#define TX_BUF_SIZE 180

static uint8_t tx_buf[TX_BUF_SIZE];
/*---------------------------------------------------------------------------*/
/* Overrides for IEEE 802.15.4, differential mode */
static uint32_t ieee_overrides[] = {
  0x00354038, /* Synth: Set RTRIM (POTAILRESTRIM) to 5 */
  0x4001402D, /* Synth: Correct CKVD latency setting (address) */
  0x00608402, /* Synth: Correct CKVD latency setting (value) */
  0x4001405D, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address) */
  0x1801F800, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value) */
  0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */
  0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */
  0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */
  0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */
  0x002B50DC, /* Adjust AGC DC filter */
  0x05000243, /* Increase synth programming timeout */
  0x002082C3, /* Increase synth programming timeout */
  0xFFFFFFFF, /* End of override list */
};
/*---------------------------------------------------------------------------*/
PROCESS(cc26xx_rf_process, "CC26xx RF driver");
/*---------------------------------------------------------------------------*/
static int on(void);
static int off(void);
static void setup_interrupts(void);
/*---------------------------------------------------------------------------*/
static uint8_t
rf_is_accessible(void)
{
  if(ti_lib_prcm_rf_ready() &&
     ti_lib_prcm_power_domain_status(PRCM_DOMAIN_RFCORE) ==
     PRCM_DOMAIN_POWER_ON) {
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Sends a command to the RF core.
 *
 * \param cmd The command value or a pointer to a command buffer
 * \param status A pointer to a variable which will hold the status
 * \return RF_CMD_OK or RF_CMD_ERROR
 *
 * This function supports all three types of command (Radio OP, immediate and
 * direct)
 *
 * For immediate and Radio OPs, cmd is a pointer to the data structure
 * containing the command and its parameters. This data structure must be
 * 4-byte aligned.
 *
 * For direct commands, cmd contains the value of the command alongside its
 * parameters
 *
 * The caller is responsible of allocating and populating cmd for Radio OP and
 * immediate commands
 *
 * The caller is responsible for allocating status
 */
static uint_fast8_t
rf_send_cmd(uint32_t cmd, uint32_t *status)
{
  uint32_t timeout_count = 0;
  bool interrupts_disabled;

  /*
   * Make sure ContikiMAC doesn't turn us off from within an interrupt while
   * we are accessing RF Core registers
   */
  interrupts_disabled = ti_lib_int_master_disable();

  if(!rf_is_accessible()) {
    PRINTF("rf_send_cmd: RF was off\n");
    if(!interrupts_disabled) {
      ti_lib_int_master_enable();
    }
    return RF_CMD_ERROR;
  }

  HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR) = cmd;
  do {
    *status = HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDSTA);
    if(++timeout_count > 50000) {
      PRINTF("rf_send_cmd: Timeout\n");
      if(!interrupts_disabled) {
        ti_lib_int_master_enable();
      }
      return RF_CMD_ERROR;
    }
  } while(*status == RF_CMD_STATUS_PENDING);

  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }

  /*
   * If we reach here the command is no longer pending. It is either completed
   * successfully or with error
   */
  return *status == RF_CMD_STATUS_DONE;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Checks whether the RFC domain is accessible and the RFC is in IEEE RX
 * \return 1: RFC in RX mode (and therefore accessible too). 0 otherwise
 */
static uint8_t
rf_is_on(void)
{
  if(!rf_is_accessible()) {
    return 0;
  }

  return RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf) == RF_RADIO_OP_STATUS_ACTIVE;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Check the RF's TX status
 * \return 1 RF is transmitting
 * \return 0 RF is not transmitting
 *
 * TX mode may be triggered either by a CMD_IEEE_TX or by the automatic
 * transmission of an ACK frame.
 */
static uint8_t
transmitting(void)
{
  uint32_t cmd_status;

  /* If we are off, we are not in TX */
  if(!rf_is_accessible()) {
    return 0;
  }

  memset(cmd_immediate_buf, 0x00, SIZEOF_STRUCT(CMD_IEEE_CCA_REQ));
  GET_FIELD(cmd_immediate_buf, command, commandNo) = CMD_IEEE_CCA_REQ;

  if(rf_send_cmd((uint32_t)cmd_immediate_buf, &cmd_status) == RF_CMD_ERROR) {
    PRINTF("transmitting: CMDSTA=0x%08lx, status=0x%04x\n",
           cmd_status, RF_RADIO_OP_GET_STATUS(cmd_immediate_buf));
    return 0;
  }

  if((GET_FIELD(cmd_immediate_buf, CMD_IEEE_CCA_REQ, currentRssi)
      == RF_CMD_CCA_REQ_RSSI_UNKNOWN) &&
     (GET_BITFIELD(cmd_immediate_buf, CMD_IEEE_CCA_REQ, ccaInfo, ccaEnergy)
      == RF_CMD_CCA_REQ_CCA_STATE_BUSY)) {
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns CCA information
 * \return RF_GET_CCA_INFO_ERROR if the RF was not on
 * \return On success, the return value is formatted as per the ccaInfo field
 *         of CMD_IEEE_CCA_REQ
 *
 * It is the caller's responsibility to make sure the RF is on. This function
 * will return RF_GET_CCA_INFO_ERROR if the RF is off
 *
 * This function will in fact wait for a valid RSSI signal
 */
static uint8_t
get_cca_info(void)
{
  uint32_t cmd_status;
  int8_t rssi;

  if(!rf_is_on()) {
    PRINTF("get_cca_info: Not on\n");
    return RF_GET_CCA_INFO_ERROR;
  }

  rssi = RF_CMD_CCA_REQ_RSSI_UNKNOWN;

  while(rssi == RF_CMD_CCA_REQ_RSSI_UNKNOWN || rssi == 0) {
    memset(cmd_immediate_buf, 0x00, SIZEOF_STRUCT(CMD_IEEE_CCA_REQ));
    GET_FIELD(cmd_immediate_buf, command, commandNo) = CMD_IEEE_CCA_REQ;

    if(rf_send_cmd((uint32_t)cmd_immediate_buf, &cmd_status) == RF_CMD_ERROR) {
      PRINTF("get_cca_info: CMDSTA=0x%08lx, status=0x%04x\n",
             cmd_status, RF_RADIO_OP_GET_STATUS(cmd_immediate_buf));

      return RF_GET_CCA_INFO_ERROR;
    }

    rssi = GET_FIELD(cmd_immediate_buf, CMD_IEEE_CCA_REQ, currentRssi);
  }

  /* We have a valid RSSI signal. Return the CCA Info */
  return GET_FIELD(cmd_immediate_buf, CMD_IEEE_CCA_REQ, ccaInfo);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Reads the current signal strength (RSSI)
 * \return The current RSSI in dBm or CMD_GET_RSSI_UNKNOWN
 *
 * This function reads the current RSSI on the currently configured
 * channel.
 */
static radio_value_t
get_rssi(void)
{
  uint32_t cmd_status;
  int8_t rssi;
  uint8_t was_off = 0;

  /* If we are off, turn on first */
  if(!rf_is_on()) {
    was_off = 1;
    if(on() != RF_CMD_OK) {
      PRINTF("get_rssi: on() failed\n");
      return RF_CMD_CCA_REQ_RSSI_UNKNOWN;
    }
  }

  memset(cmd_immediate_buf, 0x00, SIZEOF_STRUCT(CMD_GET_RSSI));
  GET_FIELD(cmd_immediate_buf, command, commandNo) = CMD_GET_RSSI;

  if(rf_send_cmd((uint32_t)cmd_immediate_buf, &cmd_status) == RF_CMD_ERROR) {
    rssi = RF_CMD_CCA_REQ_RSSI_UNKNOWN;
  }

  /* Current RSSI in bits 23:16 of cmd_status */
  rssi = (cmd_status >> 16) & 0xFF;

  /* If we were off, turn back off */
  if(was_off) {
    off();
  }

  return rssi;
}
/*---------------------------------------------------------------------------*/
/* Returns the current TX power in dBm */
static radio_value_t
get_tx_power(void)
{
  return tx_power_current->dbm;
}
/*---------------------------------------------------------------------------*/
/*
 * Set TX power to 'at least' power dBm
 * This works with a lookup table. If the value of 'power' does not exist in
 * the lookup table, TXPOWER will be set to the immediately higher available
 * value
 */
static void
set_tx_power(radio_value_t power)
{
  uint32_t cmd_status;
  int i;

  /* Send a CMD_SET_TX_POWER command to the RF */
  memset(cmd_immediate_buf, 0x00, SIZEOF_STRUCT(CMD_SET_TX_POWER));

  GET_FIELD(cmd_immediate_buf, command, commandNo) = CMD_SET_TX_POWER;

  for(i = OUTPUT_CONFIG_COUNT - 1; i >= 0; --i) {
    if(power <= output_power[i].dbm) {
      GET_FIELD(cmd_immediate_buf, CMD_SET_TX_POWER, txPower) =
        BITVALUE(CMD_SET_TX_POWER, txPower, IB, output_power[i].register_ib) |
        BITVALUE(CMD_SET_TX_POWER, txPower, GC, output_power[i].register_gc) |
        BITVALUE(CMD_SET_TX_POWER, txPower, tempCoeff, 0);

      if(rf_send_cmd((uint32_t)cmd_immediate_buf, &cmd_status) == RF_CMD_OK) {
        /* Success: Remember the new setting */
        tx_power_current = &output_power[i];
      } else {
        PRINTF("set_tx_power: CMDSTA=0x%08lx\n", cmd_status);
      }
      return;
    }
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Wait till running radio Op command completes
 *
 * \return RF_CMD_ERROR or RF_CMD_OK
 *
 * RF_CMD_OK will be returned if the Radio Op returned with
 * RF_RADIO_OP_STATUS_DONE_OK
 *
 * RF_CMD_ERROR will be returned in the radio op returned with any other
 * RF_RADIO_OP_STATUS_DONE_xyz
 */
static uint_fast8_t
rf_wait_cmd_completed_ok(uint8_t *cmd)
{
  _TYPE_radioOp_status tmp_status;
  uint32_t timeoutCount = 0;

  /*
   * 0x04XX=DONE, 0x0400=DONE_OK while all other "DONE" values means done
   * but with some kind of error (ref. "Common radio operation status codes")
   */
  do {
    tmp_status = GET_FIELD_V(cmd, radioOp, status);
    if(++timeoutCount > 500000) {
      return RF_CMD_ERROR;
    }
  } while((tmp_status & RF_RADIO_OP_MASKED_STATUS) != RF_RADIO_OP_MASKED_STATUS_DONE);

  return tmp_status == RF_RADIO_OP_STATUS_DONE_OK;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief    Builds common radio parameters for radio operations
 *
 * \param    *cmd     Pointer to buffer to add parameters to
 * \param    command   Radio command number (e.g. COMMAND_RADIO_SETUP)
 *
 * \note     The buffer must be emptied with memset() before calling this function
 *
 * \return   None
 */
static void
rf_build_radio_op_cmd(uint8_t *cmd, uint16_t command)
{
  GET_FIELD(cmd, radioOp, commandNo) = command;
  GET_FIELD(cmd, radioOp, status) = IDLE;
  GET_FIELD(cmd, radioOp, pNextOp) = NULL;
  GET_FIELD(cmd, radioOp, startTime) = 0;
  GET_FIELD(cmd, radioOp, startTrigger) = TRIG_NOW;
  GET_FIELD(cmd, radioOp, condition) = COND_NEVER;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Sends a CMD_RADIO_SETUP for the selected mode (IEEE or BLE)
 * \param mode RF_MODE_BLE or RF_MODE_IEEE
 * \return RF_CMD_OK or RF_CMD_ERROR
 *
 * ToDo: Likely to need one more argument to set bNoAdi<foo> on first startup
 * vs when coming back from sleep
 */
static uint8_t
rf_radio_setup(uint8_t mode)
{
  uint32_t cmd_status;

  /* Create radio setup command */
  memset(cmd_immediate_buf, 0x00, SIZEOF_RADIO_OP(CMD_RADIO_SETUP));
  rf_build_radio_op_cmd(cmd_immediate_buf, CMD_RADIO_SETUP);

  /* Set output power to the current (or default) value */
  GET_FIELD(cmd_immediate_buf, CMD_RADIO_SETUP, txPower) =
    BITVALUE(CMD_RADIO_SETUP, txPower, IB, tx_power_current->register_ib) |
    BITVALUE(CMD_RADIO_SETUP, txPower, GC, tx_power_current->register_gc) |
    BITVALUE(CMD_RADIO_SETUP, txPower, tempCoeff, 0);

  /* Do mode-dependent things (e.g. apply overrides) */
  if(mode == RF_MODE_IEEE) {
    /* Add override control pointer */
    GET_FIELD(cmd_immediate_buf, CMD_RADIO_SETUP, pRegOverride) = ieee_overrides;
#if CC26XX_RF_BLE_SUPPORT
  } else if(mode == RF_MODE_BLE) {
    /* Add override control pointer */
    GET_FIELD(cmd_immediate_buf, CMD_RADIO_SETUP, pRegOverride) = ble_overrides;
#endif
  } else {
    PRINTF("rf_radio_setup: Unknown mode %u\n", mode);
    return RF_CMD_ERROR;
  }

  /* Lastly, set the mode */
  GET_FIELD(cmd_immediate_buf, CMD_RADIO_SETUP, mode) = mode;

  /* Send Radio setup to RF Core */
  if(rf_send_cmd((uint32_t)cmd_immediate_buf, &cmd_status) != RF_CMD_OK) {
    PRINTF("rf_radio_setup: CMD_RADIO_SETUP, CMDSTA=0x%08lx, status=0x%04x\n",
           cmd_status, RF_RADIO_OP_GET_STATUS(cmd_immediate_buf));
    return RF_CMD_ERROR;
  }

  /* Wait until radio setup is done */
  if(rf_wait_cmd_completed_ok(cmd_immediate_buf) != RF_CMD_OK) {
    PRINTF("rf_radio_setup: CMD_RADIO_SETUP wait, CMDSTA=0x%08lx, status=0x%04x\n",
           cmd_status, RF_RADIO_OP_GET_STATUS(cmd_immediate_buf));
    return RF_CMD_ERROR;
  }

  return RF_CMD_OK;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Applies patches (if any)
 * \return RF_CMD_OK or RF_CMD_ERROR
 *
 * Currently patches are not required.
 */
static uint8_t
apply_patches()
{
  uint32_t cmd_status;

  /* Patch of uninitialized pointer */
  *((uint32_t *)0x21000028) = 0x00000000;

  /* Start radio timer (RAT) */
  if(rf_send_cmd(CMDR_DIR_CMD(CMD_START_RAT), &cmd_status) != RF_CMD_OK) {
    PRINTF("apply_patches: START_RAT fail, CMDSTA=0x%08lx\n", cmd_status);
    return RF_CMD_ERROR;
  }

  return RF_CMD_OK;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief    Set up radio in IEEE802.15.4 RX mode
 *
 * \return   RF_CMD_OK   Succeeded
 * \return   RF_CMD_ERROR   Failed
 *
 * This function assumes that cmd_ieee_rx_buf has been previously populated
 * with correct values. This can be done through init_rf_params (sets defaults)
 * or through Contiki's extended RF API (set_value, set_object)
 */
static uint8_t
rf_cmd_ieee_rx()
{
  uint32_t cmd_status;
  rtimer_clock_t t0;
  int ret;

  ret = rf_send_cmd((uint32_t)cmd_ieee_rx_buf, &cmd_status);

  if(ret != RF_CMD_OK) {
    PRINTF("rf_cmd_ieee_rx: ret=%d, CMDSTA=0x%08lx, status=0x%04x\n",
           ret, cmd_status, RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf));
  }

  t0 = RTIMER_NOW();

  while(RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf) != RF_RADIO_OP_STATUS_ACTIVE &&
        (RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + ENTER_RX_WAIT_TIMEOUT)));

  /* Wait to enter RX */
  if(RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf) != RF_RADIO_OP_STATUS_ACTIVE) {
    PRINTF("rf_cmd_ieee_rx: CMDSTA=0x%08lx, status=0x%04x\n",
           cmd_status, RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf));
    return RF_CMD_ERROR;
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static void
init_rx_buffers(void)
{
  /* Two-element circular buffer, hardcoded for now.. */
  GET_FIELD(rx_buf_0, dataEntry, pNextEntry) = rx_buf_1;
  GET_FIELD(rx_buf_0, dataEntry, config) = 0x04;
  GET_FIELD(rx_buf_0, dataEntry, length) = sizeof(rx_buf_0) - 8;

  GET_FIELD(rx_buf_1, dataEntry, pNextEntry) = rx_buf_2;
  GET_FIELD(rx_buf_1, dataEntry, config) = 0x04;
  GET_FIELD(rx_buf_1, dataEntry, length) = sizeof(rx_buf_1) - 8;

  GET_FIELD(rx_buf_2, dataEntry, pNextEntry) = rx_buf_3;
  GET_FIELD(rx_buf_2, dataEntry, config) = 0x04;
  GET_FIELD(rx_buf_2, dataEntry, length) = sizeof(rx_buf_2) - 8;

  /* Point to first element again */
  GET_FIELD(rx_buf_3, dataEntry, pNextEntry) = rx_buf_0;
  GET_FIELD(rx_buf_3, dataEntry, config) = 0x04;
  GET_FIELD(rx_buf_3, dataEntry, length) = sizeof(rx_buf_3) - 8;
}
/*---------------------------------------------------------------------------*/
static void
init_rf_params(void)
{
  memset(cmd_ieee_rx_buf, 0x00, SIZEOF_RADIO_OP(CMD_IEEE_RX));

  GET_FIELD(cmd_ieee_rx_buf, radioOp, commandNo) = CMD_IEEE_RX;
  GET_FIELD(cmd_ieee_rx_buf, radioOp, status) = IDLE;
  GET_FIELD(cmd_ieee_rx_buf, radioOp, pNextOp) = NULL;
  GET_FIELD(cmd_ieee_rx_buf, radioOp, startTime) = 0x00000000;
  GET_FIELD(cmd_ieee_rx_buf, radioOp, startTrigger) = TRIG_NOW;
  GET_FIELD(cmd_ieee_rx_buf, radioOp, condition) = COND_NEVER;
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, channel) = CC26XX_RF_CHANNEL;
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, rxConfig) =
    BITVALUE(CMD_IEEE_RX, rxConfig, bAutoFlushCrc, 1) |
    BITVALUE(CMD_IEEE_RX, rxConfig, bAutoFlushIgn, 0) |
    BITVALUE(CMD_IEEE_RX, rxConfig, bIncludePhyHdr, 0) |
    BITVALUE(CMD_IEEE_RX, rxConfig, bIncludeCrc, 1) |
    BITVALUE(CMD_IEEE_RX, rxConfig, bAppendRssi, 1) |
    BITVALUE(CMD_IEEE_RX, rxConfig, bAppendCorrCrc, 1) |
    BITVALUE(CMD_IEEE_RX, rxConfig, bAppendSrcInd, 0) |
    BITVALUE(CMD_IEEE_RX, rxConfig, bAppendTimestamp, 0);
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, pRxQ) = &rx_data_queue;
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, pOutput) = rf_stats;
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, frameFiltOpt) =
#if CC26XX_RF_CONF_PROMISCOUS
    BITVALUE(CMD_IEEE_RX, frameFiltOpt, frameFiltEn, 0) |
#else
    BITVALUE(CMD_IEEE_RX, frameFiltOpt, frameFiltEn, 1) |
#endif
    BITVALUE(CMD_IEEE_RX, frameFiltOpt, frameFiltStop, 1) |
#if CC26XX_RF_CONF_AUTOACK
    BITVALUE(CMD_IEEE_RX, frameFiltOpt, autoAckEn, 1) |
#else
    BITVALUE(CMD_IEEE_RX, frameFiltOpt, autoAckEn, 0) |
#endif
    BITVALUE(CMD_IEEE_RX, frameFiltOpt, slottedAckEn, 0) |
    BITVALUE(CMD_IEEE_RX, frameFiltOpt, autoPendEn, 0) |
    BITVALUE(CMD_IEEE_RX, frameFiltOpt, defaultPend, 0) |
    BITVALUE(CMD_IEEE_RX, frameFiltOpt, bPendDataReqOnly, 0) |
    BITVALUE(CMD_IEEE_RX, frameFiltOpt, bPanCoord, 0) |
    BITVALUE(CMD_IEEE_RX, frameFiltOpt, maxFrameVersion, 1) |
    BITVALUE(CMD_IEEE_RX, frameFiltOpt, bStrictLenFilter, 0);
  /* Receive all frame types */
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, frameTypes) =
    BITVALUE(CMD_IEEE_RX, frameTypes, bAcceptFt0Beacon, 1) |
    BITVALUE(CMD_IEEE_RX, frameTypes, bAcceptFt1Data, 1) |
    BITVALUE(CMD_IEEE_RX, frameTypes, bAcceptFt2Ack, 1) |
    BITVALUE(CMD_IEEE_RX, frameTypes, bAcceptFt3MacCmd, 1) |
    BITVALUE(CMD_IEEE_RX, frameTypes, bAcceptFt4Reserved, 1) |
    BITVALUE(CMD_IEEE_RX, frameTypes, bAcceptFt5Reserved, 1) |
    BITVALUE(CMD_IEEE_RX, frameTypes, bAcceptFt6Reserved, 1) |
    BITVALUE(CMD_IEEE_RX, frameTypes, bAcceptFt7Reserved, 1);
  /* Configure CCA settings */
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, ccaOpt) =
    BITVALUE(CMD_IEEE_RX, ccaOpt, ccaEnEnergy, 1) |
    BITVALUE(CMD_IEEE_RX, ccaOpt, ccaEnCorr, 1) |
    BITVALUE(CMD_IEEE_RX, ccaOpt, ccaEnSync, 0) |
    BITVALUE(CMD_IEEE_RX, ccaOpt, ccaCorrOp, 1) |
    BITVALUE(CMD_IEEE_RX, ccaOpt, ccaSyncOp, 1) |
    BITVALUE(CMD_IEEE_RX, ccaOpt, ccaCorrThr, 3);
  /* Set CCA RSSI Threshold, 0xA6 corresponds to -90dBm (two's comp.)*/
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, ccaRssiThr) = 0xA6;
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, numExtEntries) = 0x00;
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, numShortEntries) = 0x00;
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, pExtEntryList) = 0;
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, pShortEntryList) = 0;

  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, endTrigger) = TRIG_NEVER;
  GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, endTime) = 0x00000000;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Turn on power to the RFC and boot it.
 *
 * \return RF_CMD_OK or RF_CMD_ERROR
 */
static int
power_up(void)
{
  uint32_t cmd_status;
  bool interrupts_disabled = ti_lib_int_master_disable();

  ti_lib_int_pend_clear(INT_RF_CPE0);
  ti_lib_int_pend_clear(INT_RF_CPE1);
  ti_lib_int_disable(INT_RF_CPE0);
  ti_lib_int_disable(INT_RF_CPE1);

  /* Enable RF Core power domain */
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_RFCORE);
  while(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_RFCORE)
        != PRCM_DOMAIN_POWER_ON);

  ti_lib_prcm_domain_enable(PRCM_DOMAIN_RFCORE);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  while(!rf_is_accessible()) {
    PRINTF("power_up: Not ready\n");
  }

  HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIFG) = 0x0;
  HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIEN) = 0x0;
  ti_lib_int_enable(INT_RF_CPE0);
  ti_lib_int_enable(INT_RF_CPE1);

  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }

  /* Let CPE boot */
  HWREG(RFC_PWR_NONBUF_BASE + RFC_PWR_O_PWMCLKEN) = RF_CORE_CLOCKS_MASK;

  /* Send ping (to verify RFCore is ready and alive) */
  if(rf_send_cmd(CMDR_DIR_CMD(CMD_PING), &cmd_status) != RF_CMD_OK) {
    PRINTF("power_up: CMD_PING fail, CMDSTA=0x%08lx\n", cmd_status);
    return RF_CMD_ERROR;
  }

  return RF_CMD_OK;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Disable RFCORE clock domain in the MCU VD and turn off the RFCORE PD
 */
static void
power_down(void)
{
  bool interrupts_disabled = ti_lib_int_master_disable();
  ti_lib_int_disable(INT_RF_CPE0);
  ti_lib_int_disable(INT_RF_CPE1);

  if(rf_is_accessible()) {
    HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIFG) = 0x0;
    HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIEN) = 0x0;
  }

  /* Shut down the RFCORE clock domain in the MCU VD */
  ti_lib_prcm_domain_disable(PRCM_DOMAIN_RFCORE);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Turn off RFCORE PD */
  ti_lib_prcm_power_domain_off(PRCM_DOMAIN_RFCORE);
  while(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_RFCORE)
        != PRCM_DOMAIN_POWER_OFF);

  ti_lib_int_pend_clear(INT_RF_CPE0);
  ti_lib_int_pend_clear(INT_RF_CPE1);
  ti_lib_int_enable(INT_RF_CPE0);
  ti_lib_int_enable(INT_RF_CPE1);
  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }
}
/*---------------------------------------------------------------------------*/
static int
rx_on(void)
{
  int ret;

  /* Get status of running IEEE_RX (if any) */
  if(rf_is_on()) {
    PRINTF("rx_on: We were on. PD=%u, RX=0x%04x \n", rf_is_accessible(),
           RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf));
    return RF_CMD_OK;
  }

  /* Put CPE in RX using the currently configured parameters */
  ret = rf_cmd_ieee_rx();

  if(ret) {
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
rx_off(void)
{
  uint32_t cmd_status;
  int ret;

  /* If we are off, do nothing */
  if(!rf_is_on()) {
    return RF_CMD_OK;
  }

  /* Wait for ongoing ACK TX to finish */
  while(transmitting());

  /* Send a CMD_STOP command to RF Core */
  if(rf_send_cmd(CMDR_DIR_CMD(CMD_ABORT), &cmd_status) != RF_CMD_OK) {
    PRINTF("RX off: CMD_ABORT status=0x%08lx\n", cmd_status);
    /* Continue nonetheless */
  }

  while(rf_is_on());

  if(RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf) == IEEE_DONE_STOPPED ||
     RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf) == IEEE_DONE_ABORT) {
    /* Stopped gracefully */
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
    ret = RF_CMD_OK;
  } else {
    PRINTF("RX off: BG status=0x%04x\n", RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf));
    ret = RF_CMD_ERROR;
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static void
rx_isr(void)
{
  process_poll(&cc26xx_rf_process);
}
/*---------------------------------------------------------------------------*/
static void
rx_nok_isr(void)
{
  RIMESTATS_ADD(badcrc);
  PRINTF("RF: Bad CRC\n");
}
/*---------------------------------------------------------------------------*/
void
cc26xx_rf_cpe1_isr(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  ti_lib_int_master_disable();
  PRINTF("RF Error\n");

  if(!rf_is_accessible()) {
    if(power_up() != RF_CMD_OK) {
      return;
    }
  }

  /* Clear interrupt flags */
  HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIFG) = 0x0;
  ti_lib_int_master_enable();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
cc26xx_rf_cpe0_isr(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(!rf_is_accessible()) {
    printf("RF ISR called but RF not ready... PANIC!!\n");
    if(power_up() != RF_CMD_OK) {
      PRINTF("power_up() failed\n");
      return;
    }
  }

  ti_lib_int_master_disable();
  if(HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIFG) & RX_IRQ) {
    rx_isr();
  }
  if(CC26XX_DEBUG_CRC) {
    if(HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIFG) & RX_NOK_IRQ) {
      rx_nok_isr();
    }
  }
  /* Clear interrupt flags */
  HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIFG) = 0x0;
  ti_lib_int_master_enable();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
static void
setup_interrupts(void)
{
  bool interrupts_disabled;

  /* We are already turned on by the caller, so this should not happen */
  if(!rf_is_accessible()) {
    PRINTF("setup_interrupts: No access\n");
    return;
  }

  /* Disable interrupts */
  interrupts_disabled = ti_lib_int_master_disable();

  /* Set all interrupt channels to CPE0 channel, error to CPE1 */
  HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEISL) = ERROR_IRQ;

  /* Acknowledge TX_Frame, Rx_Entry_Done and ERROR */
  HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIEN) = ENABLED_IRQS;

  /* Clear interrupt flags, active low clear(?) */
  HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIFG) = 0x0;

  ti_lib_int_pend_clear(INT_RF_CPE0);
  ti_lib_int_pend_clear(INT_RF_CPE1);
  ti_lib_int_enable(INT_RF_CPE0);
  ti_lib_int_enable(INT_RF_CPE1);

  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }
}
/*---------------------------------------------------------------------------*/
static uint8_t
request(void)
{
  /*
   * We rely on the RDC layer to turn us on and off. Thus, if we are on we
   * will only allow sleep, standby otherwise
   */
  if(rf_is_on()) {
    return LPM_MODE_SLEEP;
  }

  return LPM_MODE_MAX_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
LPM_MODULE(cc26xx_rf_lpm_module, request, NULL, NULL, LPM_DOMAIN_NONE);
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  lpm_register_module(&cc26xx_rf_lpm_module);

  /* Enable IEEE, BLE and Common-CMD APIs */
  HWREG(PRCM_BASE + PRCM_O_RFCMODESEL) = PRCM_RFCMODESEL_CURR_MODE5;

  /* Wipe out the BLE adv buffer */
  init_ble();

  /* Initialise RX buffers */
  memset(rx_buf_0, 0, RX_BUF_SIZE);
  memset(rx_buf_1, 0, RX_BUF_SIZE);
  memset(rx_buf_2, 0, RX_BUF_SIZE);
  memset(rx_buf_3, 0, RX_BUF_SIZE);

  /* Set of RF Core data queue. Circular buffer, no last entry */
  rx_data_queue.pCurrEntry = rx_buf_0;

  rx_data_queue.pLastEntry = NULL;

  /* Initialize current read pointer to first element (used in ISR) */
  rx_read_entry = rx_buf_0;

  /* Populate the RF parameters data structure with default values */
  init_rf_params();

  if(on() != RF_CMD_OK) {
    PRINTF("init: on() failed\n");
    return RF_CMD_ERROR;
  }

  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

  process_start(&cc26xx_rf_process, NULL);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  int len = MIN(payload_len, sizeof(tx_buf));

  memcpy(tx_buf, payload, len);
  return RF_CMD_OK;
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short transmit_len)
{
  int ret;
  uint8_t was_off = 0;
  uint32_t cmd_status;
  uint16_t stat;
  uint8_t tx_active = 0;
  rtimer_clock_t t0;

  if(!rf_is_on()) {
    was_off = 1;
    if(on() != RF_CMD_OK) {
      PRINTF("transmit: on() failed\n");
      return RF_CMD_ERROR;
    }
  }

  /*
   * We are certainly not TXing a frame as a result of CMD_IEEE_TX, but we may
   * be in the process of TXing an ACK. In that case, wait for the TX to finish
   * or return after approx TX_WAIT_TIMEOUT
   */
  t0 = RTIMER_NOW();

  do {
    tx_active = transmitting();
  } while(tx_active == 1 &&
          (RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + TX_WAIT_TIMEOUT)));

  if(tx_active) {
    PRINTF("transmit: Already TXing and wait timed out\n");

    if(was_off) {
      off();
    }

    return RADIO_TX_COLLISION;
  }

  /* Send the CMD_IEEE_TX command */
  memset(cmd_immediate_buf, 0, SIZEOF_RADIO_OP(CMD_IEEE_TX));

  rf_build_radio_op_cmd(cmd_immediate_buf, CMD_IEEE_TX);

  GET_FIELD(cmd_immediate_buf, CMD_IEEE_TX, payloadLen) = transmit_len;
  GET_FIELD(cmd_immediate_buf, CMD_IEEE_TX, pPayload) = tx_buf;

  /* Enable the LAST_FG_COMMAND_DONE interrupt, which will wake us up */
  HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIEN) = ENABLED_IRQS +
    LAST_FG_CMD_DONE;

  ret = rf_send_cmd((uint32_t)cmd_immediate_buf, &cmd_status);

  if(ret) {
    /* If we enter here, TX actually started */
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
    ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

    /* Idle away while the command is running */
    while((RF_RADIO_OP_GET_STATUS(cmd_immediate_buf) & RF_RADIO_OP_MASKED_STATUS)
          == RF_RADIO_OP_MASKED_STATUS_RUNNING) {
      lpm_sleep();
    }

    stat = RF_RADIO_OP_GET_STATUS(cmd_immediate_buf);

    if(stat == RF_RADIO_OP_STATUS_IEEE_DONE_OK) {
      /* Sent OK */
      RIMESTATS_ADD(lltx);
      ret = RADIO_TX_OK;
    } else {
      /* Operation completed, but frame was not sent */
      PRINTF("transmit: ret=%d, CMDSTA=0x%08lx, status=0x%04x\n", ret,
             cmd_status, stat);
      ret = RADIO_TX_ERR;
    }
  } else {
    /* Failure sending the CMD_IEEE_TX command */
    PRINTF("transmit: ret=%d, CMDSTA=0x%08lx, status=0x%04x\n",
           ret, cmd_status, RF_RADIO_OP_GET_STATUS(cmd_immediate_buf));

    ret = RADIO_TX_ERR;
  }

  /*
   * Update ENERGEST state here, before a potential call to off(), which
   * will correctly update it if required.
   */
  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

  /*
   * Disable LAST_FG_COMMAND_DONE interrupt. We don't really care about it
   * except when we are transmitting
   */
  HWREG(RFC_DBELL_NONBUF_BASE + RFC_DBELL_O_RFCPEIEN) = ENABLED_IRQS;

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
send(const void *payload, unsigned short payload_len)
{
  prepare(payload, payload_len);
  return transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
static void
release_data_entry(void)
{
  /* Clear the length byte */
  rx_read_entry[8] = 0;
  /* Set status to 0 "Pending" in element */
  GET_FIELD_V(rx_read_entry, dataEntry, status) = DATA_ENTRY_STATUS_PENDING;
  rx_read_entry = GET_FIELD_V(rx_read_entry, dataEntry, pNextEntry);
}
/*---------------------------------------------------------------------------*/
static int
read_frame(void *buf, unsigned short buf_len)
{
  int8_t rssi;
  int len = 0;
  uint8_t status = GET_FIELD_V(rx_read_entry, dataEntry, status);

  if(status != DATA_ENTRY_STATUS_FINISHED) {
    /* No available data */
    return 0;
  }


  if(rx_read_entry[8] < 4) {
    PRINTF("RF: too short\n");
    RIMESTATS_ADD(tooshort);

    release_data_entry();
    return 0;
  }

  len = rx_read_entry[8] - 4;

  if(len > buf_len) {
    PRINTF("RF: too long\n");
    RIMESTATS_ADD(toolong);

    release_data_entry();
    return 0;
  }

  memcpy(buf, (char *)&rx_read_entry[9], len);

  rssi = (int8_t)rx_read_entry[9 + len + 2];

  packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi);
  RIMESTATS_ADD(llrx);

  release_data_entry();

  return len;
}
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  uint8_t was_off = 0;
  uint8_t cca_info;
  int ret = RF_CCA_CLEAR;

  /*
   * If we are in the middle of a BLE operation, we got called by ContikiMAC
   * from within an interrupt context. Indicate a clear channel
   */
#if CC26XX_RF_BLE_SUPPORT
  if(ble_mode_on) {
    PRINTF("channel_clear: Interrupt context but BLE in progress\n");
    return RF_CCA_CLEAR;
  }
#endif

  if(rf_is_on()) {
    /*
     * Wait for potential leftover ACK still being sent.
     * Strictly speaking, if we are TXing an ACK then the channel is not clear.
     * However, channel_clear is only ever called to determine whether there is
     * someone else's packet in the air, not ours.
     *
     * We could probably even simply return that the channel is clear
     */
    while(transmitting());
  } else {
    was_off = 1;
    if(on() != RF_CMD_OK) {
      PRINTF("channel_clear: on() failed\n");
      if(was_off) {
        off();
      }
      return RF_CCA_CLEAR;
    }
  }

  cca_info = get_cca_info();

  if(cca_info == RF_GET_CCA_INFO_ERROR) {
    PRINTF("channel_clear: CCA error\n");
    ret = RF_CCA_CLEAR;
  } else {
    /*
     * cca_info bits 1:0 - ccaStatus
     * Return 1 (clear) if idle or invalid.
     */
    ret = (cca_info & 0x03) != RF_CMD_CCA_REQ_CCA_STATE_BUSY;
  }

  if(was_off) {
    off();
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  int ret = 0;
  uint8_t cca_info;
  uint8_t was_off = 0;

  if(!rf_is_on()) {
    was_off = 1;
    if(on() != RF_CMD_OK) {
      PRINTF("receiving_packet: on() failed\n");
      return RF_CMD_ERROR;
    }
  }

  /*
   * If we are in the middle of a BLE operation, we got called by ContikiMAC
   * from within an interrupt context. We are not receiving
   */
#if CC26XX_RF_BLE_SUPPORT
  if(ble_mode_on) {
    PRINTF("receiving_packet: Interrupt context but BLE in progress\n");
    return 0;
  }
#endif

  /* If we are off, we are not receiving */
  if(!rf_is_on()) {
    PRINTF("receiving_packet: We were off\n");
    return 0;
  }

  /* If we are transmitting (can only be an ACK here), we are not receiving */
  if(transmitting()) {
    PRINTF("receiving_packet: We were TXing\n");
    return 0;
  }

  cca_info = get_cca_info();

  if(cca_info == RF_GET_CCA_INFO_ERROR) {
    /* If we can't read CCA info, return "not receiving" */
    ret = 0;
  } else {
    /* Return 1 (receiving) if ccaState is busy */
    ret = (cca_info & 0x03) == RF_CMD_CCA_REQ_CCA_STATE_BUSY;
  }

  if(was_off) {
    off();
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  volatile uint8_t *current = rx_data_queue.pCurrEntry;
  int rv = 0;

  /* Go through all RX buffers and check their status */
  do {
    if(GET_FIELD_V(current, dataEntry, status) ==
       DATA_ENTRY_STATUS_FINISHED) {
      rv = 1;
      process_poll(&cc26xx_rf_process);
    }

    current = GET_FIELD_V(current, dataEntry, pNextEntry);
  } while(current != rx_data_queue.pCurrEntry);

  /* If we didn't find an entry at status finished, no frames are pending */
  return rv;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  /*
   * Request the HF XOSC as the source for the HF clock. Needed before we can
   * use the FS. This will only request, it will _not_ perform the switch.
   */
  oscillators_request_hf_xosc();

  /*
   * If we are in the middle of a BLE operation, we got called by ContikiMAC
   * from within an interrupt context. Abort, but pretend everything is OK.
   */
#if CC26XX_RF_BLE_SUPPORT
  if(ble_mode_on) {
    PRINTF("on: Interrupt context but BLE in progress\n");
    return RF_CMD_OK;
  }
#endif

  if(rf_is_on()) {
    PRINTF("on: We were on. PD=%u, RX=0x%04x \n", rf_is_accessible(),
           RF_RADIO_OP_GET_STATUS(cmd_ieee_rx_buf));
    return RF_CMD_OK;
  }

  if(power_up() != RF_CMD_OK) {
    PRINTF("on: power_up() failed\n");
    return RF_CMD_ERROR;
  }

  if(apply_patches() != RF_CMD_OK) {
    PRINTF("on: apply_patches() failed\n");
    return RF_CMD_ERROR;
  }

  init_rx_buffers();

  setup_interrupts();

  /*
   * Trigger a switch to the XOSC, so that we can subsequently use the RF FS
   * This will block until the XOSC is actually ready, but give how we
   * requested it early on, this won't be too long a wait/
   */
  oscillators_switch_to_hf_xosc();

  if(rf_radio_setup(RF_MODE_IEEE) != RF_CMD_OK) {
    PRINTF("on: radio_setup() failed\n");
    return RF_CMD_ERROR;
  }

  return rx_on();
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  /*
   * If we are in the middle of a BLE operation, we got called by ContikiMAC
   * from within an interrupt context. Abort, but pretend everything is OK.
   */
#if CC26XX_RF_BLE_SUPPORT
  if(ble_mode_on) {
    PRINTF("off: Interrupt context but BLE in progress\n");
    return RF_CMD_OK;
  }
#endif

  while(transmitting());

  power_down();

  /* Switch HF clock source to the RCOSC to preserve power */
  oscillators_switch_to_hf_rc();

  /* We pulled the plug, so we need to restore the status manually */
  GET_FIELD(cmd_ieee_rx_buf, radioOp, status) = IDLE;

  /*
   * Just in case there was an ongoing RX (which started after we begun the
   * shutdown sequence), we don't want to leave the buffer in state == ongoing
   */
  GET_FIELD_V(rx_buf_0, dataEntry, status) = DATA_ENTRY_STATUS_PENDING;
  GET_FIELD_V(rx_buf_1, dataEntry, status) = DATA_ENTRY_STATUS_PENDING;
  GET_FIELD_V(rx_buf_2, dataEntry, status) = DATA_ENTRY_STATUS_PENDING;
  GET_FIELD_V(rx_buf_3, dataEntry, status) = DATA_ENTRY_STATUS_PENDING;

  return RF_CMD_OK;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }

  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    /* On / off */
    *value = rf_is_on() ? RADIO_POWER_MODE_ON : RADIO_POWER_MODE_OFF;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CHANNEL:
    *value = (radio_value_t)GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, channel);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_PAN_ID:
    *value = (radio_value_t)GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, localPanID);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_16BIT_ADDR:
    *value = (radio_value_t)GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, localShortAddr);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RX_MODE:
    *value = 0;
    if(GET_BITFIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, frameFiltOpt, frameFiltEn)) {
      *value |= RADIO_RX_MODE_ADDRESS_FILTER;
    }
    if(GET_BITFIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, frameFiltOpt, autoAckEn)) {
      *value |= RADIO_RX_MODE_AUTOACK;
    }

    return RADIO_RESULT_OK;
  case RADIO_PARAM_TXPOWER:
    *value = get_tx_power();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    *value = GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, ccaRssiThr);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RSSI:
    *value = get_rssi();

    if(*value == RF_CMD_CCA_REQ_RSSI_UNKNOWN) {
      return RADIO_RESULT_ERROR;
    } else {
      return RADIO_RESULT_OK;
    }
  case RADIO_CONST_CHANNEL_MIN:
    *value = CC26XX_RF_CHANNEL_MIN;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MAX:
    *value = CC26XX_RF_CHANNEL_MAX;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MIN:
    *value = OUTPUT_POWER_MIN;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MAX:
    *value = OUTPUT_POWER_MAX;
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  uint8_t was_off = 0;
  radio_result_t rv;

  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    if(value == RADIO_POWER_MODE_ON) {
      if(on() != RF_CMD_OK) {
        PRINTF("set_value: on() failed (1)\n");
        return RADIO_RESULT_ERROR;
      }
      return RADIO_RESULT_OK;
    }
    if(value == RADIO_POWER_MODE_OFF) {
      off();
      return RADIO_RESULT_OK;
    }
    return RADIO_RESULT_INVALID_VALUE;
  case RADIO_PARAM_CHANNEL:
    if(value < CC26XX_RF_CHANNEL_MIN ||
       value > CC26XX_RF_CHANNEL_MAX) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, channel) = (uint8_t)value;
    break;
  case RADIO_PARAM_PAN_ID:
    GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, localPanID) = (uint16_t)value;
    break;
  case RADIO_PARAM_16BIT_ADDR:
    GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, localShortAddr) = (uint16_t)value;
    break;
  case RADIO_PARAM_RX_MODE:
  {
    if(value & ~(RADIO_RX_MODE_ADDRESS_FILTER |
                 RADIO_RX_MODE_AUTOACK)) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, frameFiltOpt) =
      BITVALUE(CMD_IEEE_RX, frameFiltOpt, frameFiltEn,
               (value & RADIO_RX_MODE_ADDRESS_FILTER) != 0) |
      BITVALUE(CMD_IEEE_RX, frameFiltOpt, frameFiltStop, 1) |
      BITVALUE(CMD_IEEE_RX, frameFiltOpt, autoAckEn,
               (value & RADIO_RX_MODE_AUTOACK) != 0) |
      BITVALUE(CMD_IEEE_RX, frameFiltOpt, slottedAckEn, 0) |
      BITVALUE(CMD_IEEE_RX, frameFiltOpt, autoPendEn, 0) |
      BITVALUE(CMD_IEEE_RX, frameFiltOpt, defaultPend, 0) |
      BITVALUE(CMD_IEEE_RX, frameFiltOpt, bPendDataReqOnly, 0) |
      BITVALUE(CMD_IEEE_RX, frameFiltOpt, bPanCoord, 0) |
      BITVALUE(CMD_IEEE_RX, frameFiltOpt, bStrictLenFilter, 0);

    break;
  }
  case RADIO_PARAM_TXPOWER:
    if(value < OUTPUT_POWER_MIN || value > OUTPUT_POWER_MAX) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    set_tx_power(value);

    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    GET_FIELD(cmd_ieee_rx_buf, CMD_IEEE_RX, ccaRssiThr) = (int8_t)value;
    break;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }

  /* If we reach here we had no errors. Apply new settings */
  if(!rf_is_on()) {
    was_off = 1;
    if(on() != RF_CMD_OK) {
      PRINTF("set_value: on() failed (2)\n");
      return RADIO_RESULT_ERROR;
    }
  }

  if(rx_off() != RF_CMD_OK) {
    PRINTF("set_value: rx_off() failed\n");
    rv = RADIO_RESULT_ERROR;
  }

  if(rx_on() != RF_CMD_OK) {
    PRINTF("set_value: rx_on() failed\n");
    rv = RADIO_RESULT_ERROR;
  }

  /* If we were off, turn back off */
  if(was_off) {
    off();
  }

  return rv;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  uint8_t *target;
  uint8_t *src;
  int i;

  if(param == RADIO_PARAM_64BIT_ADDR) {
    if(size != 8 || !dest) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    target = dest;
    src = (uint8_t *)(GET_FIELD_PTR(cmd_ieee_rx_buf, CMD_IEEE_RX, localExtAddr));

    for(i = 0; i < 8; i++) {
      target[i] = src[7 - i];
    }

    return RADIO_RESULT_OK;
  }
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  uint8_t was_off = 0;
  radio_result_t rv;
  int i;
  uint8_t *dst;

  if(param == RADIO_PARAM_64BIT_ADDR) {
    if(size != 8 || !src) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    dst = (uint8_t *)(GET_FIELD_PTR(cmd_ieee_rx_buf, CMD_IEEE_RX,
                                    localExtAddr));

    for(i = 0; i < 8; i++) {
      dst[i] = ((uint8_t *)src)[7 - i];
    }

    if(!rf_is_on()) {
      was_off = 1;
      if(on() != RF_CMD_OK) {
        PRINTF("set_object: on() failed\n");
        return RADIO_RESULT_ERROR;
      }
    }

    if(rx_off() != RF_CMD_OK) {
      PRINTF("set_object: rx_off() failed\n");
      rv = RADIO_RESULT_ERROR;
    }

    if(rx_on() != RF_CMD_OK) {
      PRINTF("set_object: rx_on() failed\n");
      rv = RADIO_RESULT_ERROR;
    }

    /* If we were off, turn back off */
    if(was_off) {
      off();
    }

    return rv;
  }
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver cc26xx_rf_driver = {
  init,
  prepare,
  transmit,
  send,
  read_frame,
  channel_clear,
  receiving_packet,
  pending_packet,
  on,
  off,
  get_value,
  set_value,
  get_object,
  set_object,
};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc26xx_rf_process, ev, data)
{
  int len;

  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT();
    do {
      packetbuf_clear();
      len = read_frame(packetbuf_dataptr(), PACKETBUF_SIZE);

      if(len > 0) {
        packetbuf_set_datalen(len);

        NETSTACK_RDC.input();
      }
    } while(len > 0);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#if CC26XX_RF_BLE_SUPPORT
/*---------------------------------------------------------------------------*/
/**
 * \brief    Builds common radio parameters for radio operations
 *
 * \param    *cmd     Pointer to buffer to add parameters to
 * \param    command   Radio command number (e.g. COMMAND_RADIO_SETUP)
 *
 * \note     The buffer must be emptied with memset() before calling this function
 *
 * \return   None
 */
static void
rf_build_ble_radio_op_cmd(uint8_t *cmd, uint16_t command)
{
  GET_FIELD(cmd, radioOp, commandNo) = command;
  GET_FIELD(cmd, radioOp, status) = IDLE;
  GET_FIELD(cmd, radioOp, pNextOp) = NULL;
  GET_FIELD(cmd, radioOp, startTime) = 0;
  GET_FIELD(cmd, radioOp, startTrigger) = TRIG_NOW;
  GET_FIELD(cmd, radioOp, condition) = COND_NEVER;
}
/*---------------------------------------------------------------------------*/
static void
init_ble()
{
  ble_mode_on = 0;

  memset(beacond_config.adv_name, 0, BLE_ADV_NAME_BUF_LEN);
  beacond_config.interval = BLE_ADV_INTERVAL;
}
/*---------------------------------------------------------------------------*/
static int
send_ble_adv_nc(int channel, uint8_t *output, uint8_t *adv_payload,
                int adv_payload_len, uint16_t *dev_address)
{
  uint32_t cmd_status;
  int ret;

  /* Erase ble_tx_rx_buf array */
  memset(ble_tx_rx_buf, 0x00, SIZEOF_RADIO_OP(CMD_BLE_ADV_NC));
  rf_build_ble_radio_op_cmd(ble_tx_rx_buf, CMD_BLE_ADV_NC);

  GET_FIELD(ble_tx_rx_buf, bleRadioOp, channel) = channel;
  GET_FIELD(ble_tx_rx_buf, bleRadioOp, whitening) = 0;

  memset(ble_cmd_buf, 0x00, SIZEOF_STRUCT(bleAdvPar));
  GET_FIELD(ble_tx_rx_buf, bleRadioOp, pParams) = (uint8_t *)ble_cmd_buf;
  GET_FIELD(ble_tx_rx_buf, bleRadioOp, pOutput) = output;

  /* Set up BLE Advertisement parameters */
  GET_FIELD(ble_cmd_buf, bleAdvPar, pRxQ) = NULL;
  GET_FIELD(ble_cmd_buf, bleAdvPar, rxConfig) = 0;
  GET_FIELD(ble_cmd_buf, bleAdvPar, advConfig) = 0;
  GET_FIELD(ble_cmd_buf, bleAdvPar, advLen) = adv_payload_len;
  GET_FIELD(ble_cmd_buf, bleAdvPar, scanRspLen) = 0;
  GET_FIELD(ble_cmd_buf, bleAdvPar, pAdvData) = adv_payload;
  GET_FIELD(ble_cmd_buf, bleAdvPar, pScanRspData) = NULL;
  GET_FIELD(ble_cmd_buf, bleAdvPar, pDeviceAddress) = dev_address;
  GET_FIELD(ble_cmd_buf, bleAdvPar, pWhiteList) = NULL;
  GET_FIELD(ble_cmd_buf, bleAdvPar, endTrigger) = TRIG_NEVER;
  GET_FIELD(ble_cmd_buf, bleAdvPar, endTime) = TRIG_NEVER;

  if(rf_send_cmd((uint32_t)ble_tx_rx_buf, &cmd_status) == RF_CMD_ERROR) {
    PRINTF("send_ble_adv_nc: Chan=%d CMDSTA=0x%08lx, status=0x%04x\n",
           channel, cmd_status, RF_RADIO_OP_GET_STATUS(ble_tx_rx_buf));
    return RF_CMD_ERROR;
  }

  /* Wait for the ADV_NC to go out */
  while((RF_RADIO_OP_GET_STATUS(ble_tx_rx_buf) & RF_RADIO_OP_MASKED_STATUS)
        == RF_RADIO_OP_MASKED_STATUS_RUNNING);

  if(RF_RADIO_OP_GET_STATUS(ble_tx_rx_buf) == RF_RADIO_OP_STATUS_BLE_DONE_OK) {
    /* Sent OK */
    ret = RF_CMD_OK;
  } else {
    /* Radio Op completed, but ADV NC was not sent */
    PRINTF("send_ble_adv_nc: Chan=%d CMDSTA=0x%08lx, status=0x%04x\n",
           channel, cmd_status, RF_RADIO_OP_GET_STATUS(ble_tx_rx_buf));
    ret = RF_CMD_ERROR;
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
send_ble_adv(int channel, uint8_t *adv_payload, int adv_payload_len)
{
  if(send_ble_adv_nc(channel, rf_stats, adv_payload, adv_payload_len,
                     (uint16_t *)&linkaddr_node_addr.u8[2]) != RF_CMD_OK) {
    PRINTF("send_ble_adv: Channel=%d, Error advertising\n", channel);
    /* Break the loop, but don't return just yet */
    return RF_CMD_ERROR;
  }

  return RF_CMD_OK;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc26xx_rf_ble_beacon_process, ev, data)
{
  static struct etimer ble_adv_et;
  static uint8_t payload[BLE_ADV_PAYLOAD_BUF_LEN];
  static int p = 0;
  static int i;
  uint8_t was_on;
  int j;
  uint32_t cmd_status;
  bool interrupts_disabled;

  PROCESS_BEGIN();

  while(1) {
    etimer_set(&ble_adv_et, beacond_config.interval);

    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_EXIT) {
      PROCESS_EXIT();
    }

    /* Set the adv payload each pass: The device name may have changed */
    p = 0;

    /* device info */
    payload[p++] = 0x02;          /* 2 bytes */
    payload[p++] = BLE_ADV_TYPE_DEVINFO;
    payload[p++] = 0x1a;          /* LE general discoverable + BR/EDR */
    payload[p++] = 1 + strlen(beacond_config.adv_name);
    payload[p++] = BLE_ADV_TYPE_NAME;
    memcpy(&payload[p], beacond_config.adv_name,
           strlen(beacond_config.adv_name));
    p += strlen(beacond_config.adv_name);

    for(i = 0; i < BLE_ADV_MESSAGES; i++) {
      /*
       * Under ContikiMAC, some IEEE-related operations will be called from an
       * interrupt context. We need those to see that we are in BLE mode.
       */
      interrupts_disabled = ti_lib_int_master_disable();
      ble_mode_on = 1;
      if(!interrupts_disabled) {
        ti_lib_int_master_enable();
      }

      /*
       * Send BLE_ADV_MESSAGES beacon bursts. Each burst on all three
       * channels, with a BLE_ADV_DUTY_CYCLE interval between bursts
       *
       * First, determine our state:
       *
       * If we are running NullRDC, we are likely in IEEE RX mode. We need to
       * abort the IEEE BG Op before entering BLE mode.
       * If we are ContikiMAC, we are likely off, in which case we need to
       * boot the CPE before entering BLE mode
       */
      was_on = rf_is_accessible();

      if(was_on) {
        /*
         * We were on: If we are in the process of receiving an IEEE frame,
         * abort the BLE beacon burst. Otherwise, terminate the IEEE BG Op
         * so we can switch to BLE mode
         */
        if(receiving_packet()) {
          PRINTF("cc26xx_rf_ble_beacon_process: We were receiving\n");

          /* Abort this pass */
          break;
        }

        if(rx_off() != RF_CMD_OK) {
          PRINTF("cc26xx_rf_ble_beacon_process: rx_off() failed\n");

          /* Abort this pass */
          break;
        }
      } else {
        /* Request the HF XOSC to source the HF clock. */
        oscillators_request_hf_xosc();

        /* We were off: Boot the CPE */
        if(power_up() != RF_CMD_OK) {
          PRINTF("cc26xx_rf_ble_beacon_process: power_up() failed\n");

          /* Abort this pass */
          break;
        }

        if(apply_patches() != RF_CMD_OK) {
          PRINTF("cc26xx_rf_ble_beacon_process: apply_patches() failed\n");

          /* Abort this pass */
          break;
        }

        /* Trigger a switch to the XOSC, so that we can use the FS */
        oscillators_switch_to_hf_xosc();
      }

      /* Enter BLE mode */
      if(rf_radio_setup(RF_MODE_BLE) != RF_CMD_OK) {
        PRINTF("cc26xx_rf_ble_beacon_process: Error entering BLE mode\n");
        /* Continue so we can at least try to restore our previous state */
      } else {
        /* Send advertising packets on all 3 advertising channels */
        for(j = 37; j <= 39; j++) {
          if(send_ble_adv(j, payload, p) != RF_CMD_OK) {
            PRINTF("cc26xx_rf_ble_beacon_process: Channel=%d,"
                   "Error advertising\n", j);
            /* Break the loop, but don't return just yet */
            break;
          }
        }
      }

      /* Send a CMD_STOP command to RF Core */
      if(rf_send_cmd(CMDR_DIR_CMD(CMD_STOP), &cmd_status) != RF_CMD_OK) {
        PRINTF("cc26xx_rf_ble_beacon_process: status=0x%08lx\n", cmd_status);
        /* Continue... */
      }

      if(was_on) {
        /* We were on, go back to IEEE mode */
        if(rf_radio_setup(RF_MODE_IEEE) != RF_CMD_OK) {
          PRINTF("cc26xx_rf_ble_beacon_process: radio_setup() failed\n");
        }

        /* Enter IEEE RX mode */
        if(rx_on() != RF_CMD_OK) {
          PRINTF("cc26xx_rf_ble_beacon_process: rx_on() failed\n");
        }
      } else {
        power_down();

        /* Switch HF clock source to the RCOSC to preserve power */
        oscillators_switch_to_hf_rc();
      }
      etimer_set(&ble_adv_et, BLE_ADV_DUTY_CYCLE);

      interrupts_disabled = ti_lib_int_master_disable();

      ble_mode_on = 0;

      if(!interrupts_disabled) {
        ti_lib_int_master_enable();
      }

      /* Wait unless this is the last burst */
      if(i < BLE_ADV_MESSAGES - 1) {
        PROCESS_WAIT_EVENT();
      }
    }

    interrupts_disabled = ti_lib_int_master_disable();

    ble_mode_on = 0;

    if(!interrupts_disabled) {
      ti_lib_int_master_enable();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#endif /* CC26XX_RF_BLE_SUPPORT */
/*---------------------------------------------------------------------------*/
void
cc26xx_rf_ble_beacond_config(clock_time_t interval, const char *name)
{
#if CC26XX_RF_BLE_SUPPORT
  if(name != NULL) {
    memset(beacond_config.adv_name, 0, BLE_ADV_NAME_BUF_LEN);

    if(strlen(name) == 0 || strlen(name) >= BLE_ADV_NAME_BUF_LEN) {
      return;
    }

    memcpy(beacond_config.adv_name, name, strlen(name));
  }

  if(interval != 0) {
    beacond_config.interval = interval;
  }
#endif
}
/*---------------------------------------------------------------------------*/
uint8_t
cc26xx_rf_ble_beacond_start()
{
#if CC26XX_RF_BLE_SUPPORT
  if(beacond_config.adv_name[0] == 0) {
    return RF_CMD_ERROR;
  }

  process_start(&cc26xx_rf_ble_beacon_process, NULL);

  return RF_CMD_OK;
#else
  return RF_CMD_ERROR;
#endif
}
/*---------------------------------------------------------------------------*/
void
cc26xx_rf_ble_beacond_stop()
{
#if CC26XX_RF_BLE_SUPPORT
  process_exit(&cc26xx_rf_ble_beacon_process);
#endif
}
/*---------------------------------------------------------------------------*/
/** @} */
