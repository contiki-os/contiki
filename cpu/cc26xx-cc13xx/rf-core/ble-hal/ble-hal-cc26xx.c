/*
 * Copyright (c) 2016, Michael Spoerk
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
 *
 * Author: Michael Spoerk <mi.spoerk@gmail.com>
 *
 */
/*---------------------------------------------------------------------------*/

#include "ble-hal.h"
#include "rf-core/ble-hal/rf-ble-cmd.h"
#include "lpm.h"

#include "dev/oscillators.h"

#include "ble-addr.h"

#include "net/netstack.h"
#include "net/packetbuf.h"

#include "rf_data_entry.h"
#include "rf-core/rf-core.h"
#include "rf_ble_cmd.h"
#include "lib/memb.h"
#include "lib/list.h"

#include <string.h>

/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns the current status of a running Radio Op command
 * \param a A pointer with the buffer used to initiate the command
 * \return The value of the Radio Op buffer's status field
 *
 * This macro can be used to e.g. return the status of a previously
 * initiated background operation, or of an immediate command
 */
#define CMD_GET_STATUS(a) (((rfc_radioOp_t *)a)->status)
/*---------------------------------------------------------------------------*/
/**
 * The location of the primary BLE device address (public BLE address)
 */
#define BLE_ADDR_LOCATION   0x500012E8
/*---------------------------------------------------------------------------*/
typedef uint32_t rf_ticks_t;
/*---------------------------------------------------------------------------*/
/* ADVERTISING                                                               */
typedef struct {
  rf_ticks_t interval;
  ble_adv_type_t type;
  ble_addr_type_t own_addr_type;
  uint8_t channel_map;
} ble_adv_param_t;

/* advertising parameter */
static ble_adv_param_t adv_param;

/* advertising data */
static uint8_t adv_data_len;
static uint8_t adv_data[BLE_ADV_DATA_LEN];

/* scan response data */
static uint8_t scan_resp_data_len;
static uint8_t scan_resp_data[BLE_SCAN_RESP_DATA_LEN];

/* timing values */
static rf_ticks_t adv_event_next;
/*---------------------------------------------------------------------------*/
/* CONNECTION                                                                */
#define CONN_EVENT_NUM_DATA_CHANNELS       37
#define CONN_EVENT_WINDOW_WIDENING       3000   /* 0.75 ms */
#define CONN_EVENT_DELAY       5000   /* 1.25 ms */

typedef struct {
  uint8_t initiator_address[6];
  uint32_t access_address;
  uint8_t crc_init_0;
  uint8_t crc_init_1;
  uint8_t crc_init_2;
  rf_ticks_t win_size;
  rf_ticks_t win_offset;
  rf_ticks_t interval;
  uint16_t latency;
  rf_ticks_t timeout;
  uint64_t channel_map;
  uint8_t num_used_channels;
  uint8_t hop;
  uint8_t sca;
  rf_ticks_t window_widening;
  rf_ticks_t timestamp;
} ble_conn_param_t;

/* connection parameter */
static ble_conn_param_t conn_param;

/* connection timeout timer */
static struct timer conn_timeout_timer;

typedef struct {
  uint64_t channel_map;
  uint16_t counter;
  uint8_t num_used_channels;
} ble_conn_channel_update_t;

static ble_conn_channel_update_t conn_channel_update;

typedef struct {
  /* connection event counter */
  uint16_t counter;

  /* result code of the last connection event */
  uint16_t last_status;

  /* unmapped data mapped_channel */
  uint8_t unmapped_channel;

  /* used data mapped_channel */
  uint8_t mapped_channel;

  /* start of the connection event */
  rf_ticks_t start;

  /* start of the next connection event */
  rf_ticks_t next_start;
} ble_conn_event_t;

/* connection event data */
static ble_conn_event_t conn_event;
static rf_ticks_t first_conn_event_anchor;
/* event that notify upper layer of disconnection */
process_event_t ll_disconnect_event;
/*---------------------------------------------------------------------------*/
/* RX data queue (all received packets are stored in the same queue)         */
#define BLE_RX_BUF_DATA_LEN 60
#define BLE_RX_BUF_OVERHEAD 8
#define BLE_RX_BUF_LEN      (BLE_RX_BUF_DATA_LEN + BLE_RX_BUF_OVERHEAD)
#define BLE_RX_NUM_BUF      20

static uint8_t rx_bufs[BLE_RX_NUM_BUF][BLE_RX_BUF_LEN] CC_ALIGN(4);

static dataQueue_t rx_data_queue = { 0 };
static uint8_t *current_rx_entry;
/*---------------------------------------------------------------------------*/
/* TX data queue (data channel packets are stored in the same queue)         */
#define BLE_TX_BUF_DATA_LEN 27
#define BLE_TX_BUF_OVERHEAD  9
#define BLE_TX_BUF_LEN      (BLE_TX_BUF_OVERHEAD + BLE_TX_BUF_DATA_LEN)
#define BLE_TX_NUM_BUF      60

#define BLE_BUFFER_SIZE    255  /* maximum size of the data buffer */

typedef struct tx_buf_s {
  /* pointer to the next element, needed for using LIST */
  struct tx_buf_s *next;
  /* data of the tx_buffer (including the 9 byte header) */
  uint8_t data[BLE_TX_BUF_LEN];
  /* flag indicating if the entry was already queued to the radio core */
  uint8_t queued;
} tx_buf_t;

static dataQueue_t tx_data_queue = { 0 };

/* list af all dynamically allocated tx buffers, which are used */
LIST(tx_buffers_queued);
/* memory block for allocation of tx buffers */
MEMB(tx_buffers, tx_buf_t, BLE_TX_NUM_BUF);
/*---------------------------------------------------------------------------*/
typedef enum {
  BLE_CONTROLLER_STATE_STANDBY,
  BLE_CONTROLLER_STATE_ADVERTISING,
  BLE_CONTROLLER_STATE_SCANNING,
  BLE_CONTROLLER_STATE_INITIATING,
  BLE_CONTROLLER_STATE_CONN_MASTER,
  BLE_CONTROLLER_STATE_CONN_SLAVE
} ble_controller_state_t;

/* The link layer state of the ble controller */
static ble_controller_state_t state = BLE_CONTROLLER_STATE_STANDBY;
/*---------------------------------------------------------------------------*/
/* RF core buffers                                                           */
#define RF_CMD_BUFFER_SIZE    128
#define RF_PARAM_BUFFER_SIZE   80
/* buffer for all commands to the RF core */
static uint8_t cmd_buf[RF_CMD_BUFFER_SIZE];
/* buffer for all command parameters to the RF core */
static uint8_t param_buf[RF_PARAM_BUFFER_SIZE];
/* buffer for all command output */
static uint8_t output_buf[RF_PARAM_BUFFER_SIZE];
/* Errors codes */
#define BLE_ECODE_CONNECTION_TIMEOUT         0x08
/*---------------------------------------------------------------------------*/
/* Types of LL control PDUs                                                  */
#define BLE_LL_CONN_UPDATE_REQ              0x00
#define BLE_LL_CHANNEL_MAP_REQ              0x01
#define BLE_LL_TERMINATE_IND                0x02
#define BLE_LL_ENC_REQ                      0x03
#define BLE_LL_ENC_RSP                      0x04
#define BLE_LL_START_ENC_REQ                0x05
#define BLE_LL_START_ENC_RSP                0x06
#define BLE_LL_UNKNOWN_RSP                  0x07
#define BLE_LL_FEATURE_REQ                  0x08
#define BLE_LL_FEATURE_RSP                  0x09
#define BLE_LL_PAUSE_ENC_REQ                0x0A
#define BLE_LL_PAUSE_ENC_RSP                0x0B
#define BLE_LL_VERSION_IND                  0x0C
#define BLE_LL_REJECT_IND                   0x0D
#define BLE_LL_SLAVE_FEATURE_REQ            0x0E
#define BLE_LL_CONN_PARAM_REQ               0x0F
#define BLE_LL_CONN_PARAM_RSP               0x10
#define BLE_LL_REJECT_IND_EXT               0x11
#define BLE_LL_PING_REQ                     0x12
#define BLE_LL_PING_RSP                     0x13
/*---------------------------------------------------------------------------*/
/* controller specific LL fields                                             */
#define BLE_VERSION_4_0                     6
#define BLE_VERSION_4_1                     7
#define BLE_VERSION_4_2                     8
#define BLE_VERSION_NR        BLE_VERSION_4_1

#define BLE_COMPANY_ID                 0xFFFF
#define BLE_SUB_VERSION_NR             0xBEEF
/*---------------------------------------------------------------------------*/
/* LPM                                                                       */
/*---------------------------------------------------------------------------*/
static uint8_t
request(void)
{
  if(rf_core_is_accessible()) {
    return LPM_MODE_SLEEP;
  }

  return LPM_MODE_MAX_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
LPM_MODULE(cc26xx_ble_lpm_module, request, NULL, NULL, LPM_DOMAIN_NONE);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS(ble_hal_process, "BLE/CC26xx process");
/*---------------------------------------------------------------------------*/
static void
setup_buffers(void)
{
  rfc_dataEntry_t *entry;
  uint8_t i;

  /* setup circular RX buffer */
  rx_data_queue.pCurrEntry = rx_bufs[0];
  rx_data_queue.pLastEntry = NULL;
  current_rx_entry = rx_bufs[0];

  /* initialize each individual rx buffer entry */
  for(i = 0; i < BLE_RX_NUM_BUF; i++) {
    memset(rx_bufs[i], 0x00, BLE_RX_BUF_LEN);
    entry = (rfc_dataEntry_t *)rx_bufs[i];
    entry->pNextEntry = rx_bufs[(i + 1) % BLE_RX_NUM_BUF];
    entry->config.lenSz = 1;
    entry->length = BLE_RX_BUF_LEN - BLE_RX_BUF_OVERHEAD;
  }

  /* TX buffers are allocated on demand */
  memb_init(&tx_buffers);
  list_init(tx_buffers_queued);
}
/*---------------------------------------------------------------------------*/
static ble_result_t
reset(void)
{
  /* register low power module */
  lpm_register_module(&cc26xx_ble_lpm_module);

  /* set the mode in the rf core */
  rf_core_set_modesel();

  setup_buffers();

  oscillators_request_hf_xosc();

  if(!rf_core_is_accessible()) {
    /* boot the rf core */
    if(rf_core_boot() != RF_CORE_CMD_OK) {
      PRINTF("ble_controller_reset() could not boot rf-core\n");
      return BLE_RESULT_ERROR;
    }

    rf_core_setup_interrupts(0);
    oscillators_switch_to_hf_xosc();

    if(rf_ble_cmd_setup_ble_mode() != RF_BLE_CMD_OK) {
      PRINTF("could not setup rf-core to BLE mode\n");
      return BLE_RESULT_ERROR;
    }
  }

  state = BLE_CONTROLLER_STATE_STANDBY;

  /* start the BLE controller process, if it is not currently running */
  if(!process_is_running(&ble_hal_process)) {
    process_start(&ble_hal_process, NULL);
  }
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
read_bd_addr(uint8_t *addr)
{
  ble_addr_cpy_to(addr);
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
read_buffer_size(unsigned int *buf_len,
                 unsigned int *num_buf)
{
  uint16_t ll_bufs = memb_numfree(&tx_buffers);
  uint16_t buffers = (uint8_t)ll_bufs / 10;
  uint16_t buffer_size = BLE_BUFFER_SIZE;

  memcpy(buf_len, &buffer_size, 2);
  memcpy(num_buf, &buffers, 2);
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
set_adv_param(unsigned int adv_int, ble_adv_type_t type,
              ble_addr_type_t own_type, unsigned short adv_map)
{
  /* convert the adv_int according to BLE standard to rf core ticks */
  adv_param.interval = adv_int * 2500;
  adv_param.type = type;
  adv_param.own_addr_type = own_type;
  adv_param.channel_map = adv_map;

  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
read_adv_channel_tx_power(short *power)
{
  return BLE_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
set_adv_data(unsigned short data_len, char *data)
{
  if(data_len > BLE_ADV_DATA_LEN) {
    return BLE_RESULT_INVALID_PARAM;
  }
  adv_data_len = data_len;
  memcpy(adv_data, data, data_len);
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
set_scan_resp_data(unsigned short data_len, char *data)
{
  if(data_len > BLE_SCAN_RESP_DATA_LEN) {
    return BLE_RESULT_INVALID_PARAM;
  }
  scan_resp_data_len = data_len;
  memcpy(scan_resp_data, data, data_len);
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
set_adv_enable(unsigned short enable)
{
  if((enable == 1) && (state == BLE_CONTROLLER_STATE_STANDBY)) {
    state = BLE_CONTROLLER_STATE_ADVERTISING;

    /* start the timer for advertising events */
    adv_event_next = rf_core_read_current_rf_ticks() + adv_param.interval;
    rf_core_start_timer_comp(adv_event_next);

    return BLE_RESULT_OK;
  } else if((enable != 1) && (state == BLE_CONTROLLER_STATE_ADVERTISING)) {
    state = BLE_CONTROLLER_STATE_STANDBY;

    return BLE_RESULT_OK;
  }
  return BLE_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
set_scan_param(ble_scan_type_t type,
               unsigned int scan_interval,
               unsigned int scan_window,
               ble_addr_type_t own_addr_type)
{
  return BLE_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
set_scan_enable(unsigned short enable,
                unsigned short filter_duplicates)
{
  return BLE_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
create_connection(unsigned int scan_interval,
                  unsigned int scan_window,
                  ble_addr_type_t peer_addr_type,
                  uint8_t *peer_addr,
                  ble_addr_type_t own_addr_type,
                  unsigned int conn_interval,
                  unsigned int conn_latency,
                  unsigned int supervision_timeout)
{
  return BLE_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
create_connection_cancel(void)
{
  return BLE_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
connection_update(unsigned int connection_handle,
                  unsigned int conn_interval,
                  unsigned int conn_latency,
                  unsigned int supervision_timeout)
{
  return BLE_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
disconnect(unsigned int connection_handle, unsigned short reason)
{
  PRINTF("reason : 0x%X\n", reason);
  state = BLE_CONTROLLER_STATE_ADVERTISING;
  adv_event_next = rf_core_read_current_rf_ticks() + adv_param.interval;
  rf_core_start_timer_comp(adv_event_next);
  process_post(PROCESS_BROADCAST, ll_disconnect_event, NULL);
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static tx_buf_t *
prepare_tx_buf()
{
  tx_buf_t *tx_buf;

  /* allocate a TX buffer */
  tx_buf = memb_alloc(&tx_buffers);
  if(tx_buf != NULL) {
    memset(tx_buf, 0, sizeof(tx_buf_t));
    list_add(tx_buffers_queued, tx_buf);
  }
  return tx_buf;
}
/*---------------------------------------------------------------------------*/
static void
prepare_tx_buf_payload(tx_buf_t *tx_buf, void *buf,
                       unsigned short buf_len, unsigned short frame_type)
{

  uint8_t length = MIN(buf_len, 27);

  rfc_dataEntryGeneral_t *e = (rfc_dataEntryGeneral_t *)tx_buf->data;

  /* the buffer length does not contain the frame type byte */
  e->length = length + 1;
  e->config.lenSz = 1;
  e->pNextEntry = NULL;
  e->status = DATA_ENTRY_PENDING;

  tx_buf->queued = 0;

  /* set the frame type */
  memset(&tx_buf->data[8], frame_type, 1);

  /* set frame payload */
  memcpy(&tx_buf->data[9], buf, length);
}
/*---------------------------------------------------------------------------*/
static ble_result_t
send(void *buf, unsigned short buf_len)
{
  tx_buf_t *tx_buf;
  uint16_t i;
  uint8_t frame_type;
  uint8_t *data = (uint8_t *)buf;

  if((state != BLE_CONTROLLER_STATE_CONN_SLAVE) && (BLE_CONTROLLER_STATE_CONN_MASTER)) {
    /* init not finished */
    return BLE_RESULT_ERROR;
  }

  for(i = 0; i < buf_len; i += 27) {
    tx_buf = prepare_tx_buf();
    if(tx_buf == NULL) {
      PRINTF("send() could not allocate TX buffer\n");
      return BLE_RESULT_ERROR;
    }

    if(i == 0) {
      frame_type = BLE_DATA_PDU_LLID_DATA_MESSAGE;
    } else {
      frame_type = BLE_DATA_PDU_LLID_DATA_FRAGMENT;
    }

    prepare_tx_buf_payload(tx_buf, &data[i], (buf_len - i), frame_type);
  }
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
send_list(struct ble_buf_list *list)
{
  return BLE_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
const struct ble_hal_driver ble_hal =
{
  reset,
  read_bd_addr,
  read_buffer_size,
  set_adv_param,
  read_adv_channel_tx_power,
  set_adv_data,
  set_scan_resp_data,
  set_adv_enable,
  set_scan_param,
  set_scan_enable,
  create_connection,
  create_connection_cancel,
  connection_update,
  disconnect,
  send,
  send_list
};
/*---------------------------------------------------------------------------*/
/* The parameter are parsed according to Bluetooth Specification v4 (page 2206) v4.2 (page 2587)*/
static void
parse_connect_request_data(ble_conn_param_t *p, uint8_t *entry)
{
  int offset = 12;
  int i;
  for(i = 0; i < BLE_ADDR_SIZE; i++) {
    p->initiator_address[i] = entry[BLE_ADDR_SIZE - 1 - i];
  }
  memcpy(&p->access_address, &entry[offset], 4);
  p->crc_init_0 = entry[offset + 4];
  p->crc_init_1 = entry[offset + 5];
  p->crc_init_2 = entry[offset + 6];
  p->win_size = entry[offset + 7];
  p->win_offset = (entry[offset + 9] << 8) + entry[offset + 8];
  p->interval = (entry[offset + 11] << 8) + entry[offset + 10];
  p->latency = (entry[offset + 13] << 8) + entry[offset + 12];
  p->timeout = (entry[offset + 15] << 8) + entry[offset + 14];
  memcpy(&p->channel_map, &entry[offset + 16], 5);
  p->hop = entry[offset + 21] & 0x1F;
  p->sca = (entry[offset + 21] >> 5) & 0x07;
  p->timestamp = (entry[offset + 27] << 24) +
    (entry[offset + 26] << 16) +
    (entry[offset + 25] << 8) +
    entry[offset + 24];
}
/*---------------------------------------------------------------------------*/
static void
free_finished_rx_buf(void)
{
  rfc_dataEntryGeneral_t *entry;

  /* free finished RX entries */
  entry = (rfc_dataEntryGeneral_t *)current_rx_entry;
  /* clear the length field */
  current_rx_entry[8] = 0;
  /* set status to pending */
  entry->status = DATA_ENTRY_PENDING;
  /* set next data queue entry */
  current_rx_entry = entry->pNextEntry;
  entry = (rfc_dataEntryGeneral_t *)current_rx_entry;
}
/*---------------------------------------------------------------------------*/
static void
free_finished_tx_bufs(void)
{
  tx_buf_t *buf = list_head(tx_buffers_queued);
  rfc_dataEntryGeneral_t *e;

  while(buf != NULL) {
    e = (rfc_dataEntryGeneral_t *)buf->data;
    if((e != NULL) && (e->status == DATA_ENTRY_FINISHED)) {
      /* free memory block */
      memb_free(&tx_buffers, buf);

      /* remove from queued list */
      list_pop(tx_buffers_queued);
    }
    buf = list_item_next(buf);
  }
}
/*---------------------------------------------------------------------------*/
static void
append_new_tx_bufs(void)
{
  tx_buf_t *buf = list_head(tx_buffers_queued);

  while(buf != NULL) {
    if(buf->queued == 0) {
      /* add tx entry to tx queue */
      if(rf_ble_cmd_add_data_queue_entry(&tx_data_queue, buf->data)
         != RF_BLE_CMD_OK) {
        PRINTF("append_new_tx_bufs() could not add buffer to tx data queue\n");
      }
      buf->queued = 1;
    }
    buf = list_item_next(buf);
  }
}
/*---------------------------------------------------------------------------*/
static void
update_data_channel()
{
  uint8_t i;
  uint8_t j;
  uint8_t remap_index;
  /* perform the data channel selection according to BLE standard */

  /* calculate unmapped channel*/
  conn_event.unmapped_channel = (conn_event.unmapped_channel + conn_param.hop)
    % CONN_EVENT_NUM_DATA_CHANNELS;

  /* map the calculated channel */
  if(conn_param.channel_map & (1ULL << conn_event.unmapped_channel)) {
    /* channel is marked as used */
    conn_event.mapped_channel = conn_event.unmapped_channel;
  } else {
    remap_index = conn_event.unmapped_channel % conn_param.num_used_channels;
    j = 0;
    for(i = 0; i < CONN_EVENT_NUM_DATA_CHANNELS; i++) {
      if(conn_param.channel_map & (1ULL << i)) {
        if(j == remap_index) {
          conn_event.mapped_channel = i;
        }
        j++;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
state_advertising(process_event_t ev, process_data_t data,
                  uint8_t *cmd, uint8_t *param, uint8_t *output)
{
  rfc_dataEntryGeneral_t *entry;

  if(ev == rf_core_timer_event) {
    /* advertising event */
    rf_ble_cmd_create_adv_params(param, &rx_data_queue, adv_data_len,
                                 adv_data, scan_resp_data_len, scan_resp_data,
                                 adv_param.own_addr_type,
                                 (uint8_t *)BLE_ADDR_LOCATION);

    if(adv_param.channel_map & BLE_ADV_CHANNEL_1_MASK) {
      rf_ble_cmd_create_adv_cmd(cmd, BLE_ADV_CHANNEL_1, param, output);
      rf_ble_cmd_send(cmd);
      rf_ble_cmd_wait(cmd);
      PRINTF("ADV CHANNEL 1\n");
    }
    if(adv_param.channel_map & BLE_ADV_CHANNEL_2_MASK) {

      rf_ble_cmd_create_adv_cmd(cmd, BLE_ADV_CHANNEL_2, param, output);
      rf_ble_cmd_send(cmd);
      rf_ble_cmd_wait(cmd);
      PRINTF("ADV CHANNEL 2\n");
    }
    if(adv_param.channel_map & BLE_ADV_CHANNEL_3_MASK) {

      rf_ble_cmd_create_adv_cmd(cmd, BLE_ADV_CHANNEL_3, param, output);
      rf_ble_cmd_send(cmd);
      rf_ble_cmd_wait(cmd);
      PRINTF("ADV CHANNEL 3\n");
    }
    /* set timer interrupt for next advertising event */
    adv_event_next = adv_event_next + adv_param.interval;
    rf_core_start_timer_comp(adv_event_next);
  } else if(ev == rf_core_data_rx_event) {
    /* data received */
    entry = (rfc_dataEntryGeneral_t *)current_rx_entry;
    if(entry->status != DATA_ENTRY_FINISHED) {
      return;
    }

    //test
    /*for(int i=0;i<45;i++){
      PRINTF("Data %d : 0x%x\n",i,current_rx_entry[i]);
    }*/

    if((current_rx_entry[9] & 0x0F) == 5) {
      /* CONN REQ received */

      /* switch to the standby state, until connection data is parsed */
      state = BLE_CONTROLLER_STATE_STANDBY;

      /* parse connection data*/
      parse_connect_request_data(&conn_param,
                                 (uint8_t *)&current_rx_entry[11]);

      /* convert the timing values into rf core ticks */
      conn_param.win_size = conn_param.win_size * 5000;
      conn_param.win_offset = conn_param.win_offset * 5000;
      conn_param.interval = conn_param.interval * 5000;
      conn_param.window_widening = CONN_EVENT_WINDOW_WIDENING;
      /* set connection timeout timer see coreV5 p2639*/
      timer_set(&conn_timeout_timer, (conn_param.timeout*CLOCK_SECOND)/100);

      conn_event.counter = 0;
      conn_event.unmapped_channel = 0;
      update_data_channel();
      first_conn_event_anchor = conn_param.timestamp + conn_param.win_offset + CONN_EVENT_DELAY;
      timer_set(&conn_timeout_timer, (conn_param.timeout*CLOCK_SECOND)/100);
      if(conn_param.win_offset <= 60000) {
        //printf("too early");
        /* in this case the first anchor point starts too early,
         * ignore the first conn event and start with the 2nd */
        conn_event.counter++;
        update_data_channel();
        first_conn_event_anchor += conn_param.interval;
      }
      conn_event.start = first_conn_event_anchor;

      rf_ble_cmd_create_slave_params(param, &rx_data_queue, &tx_data_queue,
                                     conn_param.access_address,
                                     conn_param.crc_init_0,
                                     conn_param.crc_init_1,
                                     conn_param.crc_init_2,
                                     conn_param.win_size,
                                     conn_param.window_widening, 1);

      rf_ble_cmd_create_slave_cmd(cmd, conn_event.mapped_channel, param,
                                  output,
                                  (conn_event.start - conn_param.window_widening));

      if(rf_ble_cmd_send(cmd) != RF_BLE_CMD_OK) {
        PRINTF("could not establish connection\n");
        state = BLE_CONTROLLER_STATE_STANDBY;
        set_adv_enable(1);
        return;
      }
      //PRINTF("init connection\n");
      /* setup timer interrupt for first connection event */
      conn_event.next_start = first_conn_event_anchor + conn_param.interval;

      state = BLE_CONTROLLER_STATE_CONN_SLAVE;

      /* clear the output buffer, so that the counters are actually correct*/
      memset(output_buf, 0x00, sizeof(output_buf));
    }
    free_finished_rx_buf();
  }
}
/*---------------------------------------------------------------------------*/
void
process_ll_ctrl_msg(uint8_t *data)
{
  uint8_t resp_len = 0;
  uint8_t resp_data[26];
  /* the first 2 bytes in the packet buffer are the header */
/*    uint8_t *data = packetbuf_dataptr() + 2; */
  uint8_t op_code = data[0];

  uint64_t channel_map = 0;
  uint16_t instant;
  uint16_t i;

  if(op_code == BLE_LL_CHANNEL_MAP_REQ) {
    PRINTF("Map request\n");
    memcpy(&channel_map, &data[1], 5);
    memcpy(&instant, &data[6], 2);

    conn_channel_update.channel_map = channel_map;
    conn_channel_update.counter = instant;
    conn_channel_update.num_used_channels = 0;

    for(i = 0; i < CONN_EVENT_NUM_DATA_CHANNELS; i++) {
      if(channel_map & (1ULL << i)) {
        conn_channel_update.num_used_channels++;
      }
    }
  } else if(op_code == BLE_LL_FEATURE_REQ) {
    PRINTF("feature request\n");
    resp_data[0] = BLE_LL_FEATURE_RSP;
    memset(&resp_data[1], 0x00, 8);
    resp_len = 9;
  } else if(op_code == BLE_LL_VERSION_IND) {
    PRINTF("Version ind\n");
    resp_data[0] = BLE_LL_VERSION_IND;
    resp_data[1] = BLE_VERSION_NR;
    resp_data[2] = (BLE_COMPANY_ID >> 8) & 0xFF;
    resp_data[3] = BLE_COMPANY_ID & 0xFF;
    resp_data[4] = (BLE_SUB_VERSION_NR >> 8) & 0xFF;
    resp_data[5] = BLE_SUB_VERSION_NR & 0xFF;
    resp_len = 6;
  } else if(op_code == BLE_LL_TERMINATE_IND) {
    PRINTF("Terminate Connexion\n");
    disconnect(0, data[1]);
  } else {
    PRINTF("parse_ll_ctrl_msg() opcode: 0x%02X received\n", op_code);
  }

  if(resp_len > 0) {
    tx_buf_t *tx_buf = prepare_tx_buf();
    if(tx_buf == NULL) {
      PRINTF("process_ll_ctrl_msg() could not allocate TX buffer\n");
      return;
    }

    prepare_tx_buf_payload(tx_buf, resp_data, resp_len, BLE_DATA_PDU_LLID_CONTROL);

    if(rf_ble_cmd_add_data_queue_entry(&tx_data_queue, tx_buf->data)
       != RF_BLE_CMD_OK) {
      PRINTF("process_ll_ctrl_msg() could not add buffer to tx data queue\n");
      return;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
process_rx_entry_data_channel(void)
{
  uint8_t data_offset = 9;                       /* start index of BLE data */
  uint8_t data_len;
  uint8_t rssi;
  uint8_t frame_type;
  uint8_t more_data;
  linkaddr_t sender_addr;

  rfc_dataEntryGeneral_t *entry = (rfc_dataEntryGeneral_t *)current_rx_entry;
  if(entry->status != DATA_ENTRY_FINISHED) {
    return;
  }

  PRINTF("Data de type : %x\n",entry->config.type);

  while(entry->status == DATA_ENTRY_FINISHED) {
    /* the last 6 bytes of the data are status and timestamp bytes */
    data_len = current_rx_entry[8] - 6 - 2;

    if(data_len > 0) {
      frame_type = (current_rx_entry[data_offset] & 0x03);
      more_data = (current_rx_entry[data_offset] & 0x10) >> 4;

      /* process the received data */
      if(frame_type == BLE_DATA_PDU_LLID_CONTROL) {
        PRINTF("received control LL\n");
        /* received frame is a LL control frame */
        /* exclude the header (first 2 bytes) */
        process_ll_ctrl_msg(&current_rx_entry[data_offset + 2]);
      } else if(frame_type == BLE_DATA_PDU_LLID_DATA_MESSAGE) {
        PRINTF("received DATA LL\n");
        /* message start or complete message */
        packetbuf_clear();
        memcpy(packetbuf_dataptr(), &current_rx_entry[data_offset + 2], data_len);
        packetbuf_set_datalen(data_len);
        /* set the controller dependent attributes */
        rssi = current_rx_entry[data_offset + data_len];
        ble_addr_to_eui64(sender_addr.u8, conn_param.initiator_address);
        packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi);
        packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_node_addr);
        packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &sender_addr);

        /* notify upper layers, if complete message was received */
        if(!more_data || (data_len < 27)) {
          NETSTACK_RDC.input();
        }
      } else if(frame_type == BLE_DATA_PDU_LLID_DATA_FRAGMENT) {
        /* message fragment */
        PRINTF("received fragment LL\n");
        memcpy((packetbuf_dataptr() + packetbuf_datalen()),
               &current_rx_entry[data_offset + 2], data_len);
        packetbuf_set_datalen(packetbuf_datalen() + data_len);

        /* notify upper layers, if complete message was received */
        if(!more_data || (data_len < 27)) {
          NETSTACK_RDC.input();
        }
      }
    }
    free_finished_rx_buf();
  }
}
/*---------------------------------------------------------------------------*/
static void
state_conn_slave(process_event_t ev, process_data_t data,
                 uint8_t *cmd, uint8_t *param, uint8_t *output)
{
  rfc_bleMasterSlaveOutput_t *o = (rfc_bleMasterSlaveOutput_t *)output;

  if(ev == rf_core_command_done_event) {

    free_finished_tx_bufs();
    append_new_tx_bufs();

    conn_event.last_status = CMD_GET_STATUS(cmd);
    /* check if the last connection event was executed properly */
    if(conn_event.last_status != RF_CORE_RADIO_OP_STATUS_BLE_DONE_OK) {
      PRINTF("command status: 0x%04X; connection event counter: %d\n",
             CMD_GET_STATUS(cmd), conn_event.counter);
      PRINTF("command_status_flags: crc_err: %d, ignored: %d, md: %d, ack: %d\n",
             o->pktStatus.bLastCrcErr, o->pktStatus.bLastIgnored,
             o->pktStatus.bLastMd, o->pktStatus.bLastAck);
      if(timer_expired(&conn_timeout_timer)){
        disconnect(0, BLE_ECODE_CONNECTION_TIMEOUT);
      }
    }else{
      timer_restart(&conn_timeout_timer);
    }

    /* calculate parameters for upcoming connection event */
    if(o->pktStatus.bTimeStampValid) {
      /* get the actual value of the anchor point */
      conn_event.start = (o->timeStamp + conn_param.interval);
    } else {
      /* use the estimated start */
      conn_event.start = conn_event.next_start;
    }
    conn_event.counter++;
    if(conn_event.counter == conn_channel_update.counter) {
      /* use the new channel map from this event on */
      conn_param.channel_map = conn_channel_update.channel_map;
      conn_param.num_used_channels = conn_channel_update.num_used_channels;
    }
    update_data_channel();

    /* create & send slave command for upcoming connection event */
    rf_ble_cmd_create_slave_params(param, &rx_data_queue, &tx_data_queue,
                                   conn_param.access_address,
                                   conn_param.crc_init_0,
                                   conn_param.crc_init_1,
                                   conn_param.crc_init_2,
                                   conn_param.win_size,
                                   conn_param.window_widening,
                                   0);

    rf_ble_cmd_create_slave_cmd(cmd, conn_event.mapped_channel, param,
                                output,
                                (conn_event.start - conn_param.window_widening));


    if(rf_ble_cmd_send(cmd) != RF_BLE_CMD_OK) {
      PRINTF("connection error; event counter: %u\n", conn_event.counter);
    }
    PRINTF("Slave command\n");
    /* calculate next anchor point*/
    conn_event.next_start = conn_event.start + conn_param.interval;
  } else
  if(ev == rf_core_data_rx_event) {
    process_rx_entry_data_channel();
   }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ble_hal_process, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("ble_hal_process started\n");

  while(1) {
    PROCESS_WAIT_EVENT();
    switch(state) {
    case BLE_CONTROLLER_STATE_STANDBY:
      /* nothing to do here */
      break;
    case BLE_CONTROLLER_STATE_ADVERTISING:
      state_advertising(ev, data, (uint8_t *)cmd_buf,
                        (uint8_t *)param_buf, (uint8_t *)output_buf);
      break;
    case BLE_CONTROLLER_STATE_SCANNING:
      /* currently not supported */
      break;
    case BLE_CONTROLLER_STATE_INITIATING:
      /* currently not supported */
      break;
    case BLE_CONTROLLER_STATE_CONN_MASTER:
      /* currently not supported */
      break;
    case BLE_CONTROLLER_STATE_CONN_SLAVE:
      state_conn_slave(ev, data, (uint8_t *)cmd_buf,
                       (uint8_t *)param_buf, (uint8_t *)output_buf);
      break;
    }
  }

  PRINTF("ble_hal_process stopped\n");
  PROCESS_END();
}
