/*
 * Copyright (c) 2015, Nordic Semiconductor
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
 */
/**
 * \addtogroup nrf52832-ble
 * @{
 *
 * \file
 *         A MAC protocol implementation that uses nRF52 IPSP implementation
 *         as a link layer.
 * \author
 *         Wojciech Bober <wojciech.bober@nordicsemi.no>
 */
#include <stdint.h>
#include <ble-core.h>
#include "app_error.h"
#include "ble_ipsp.h"
#include "nrf_soc.h"
#include "iot_defines.h"

#include "net/mac/nullmac.h"
#include "net/netstack.h"
#include "net/ip/uip.h"
#include "net/ip/tcpip.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "net/linkaddr.h"

#include "dev/watchdog.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef BLE_MAC_MAX_INTERFACE_NUM
#define BLE_MAC_MAX_INTERFACE_NUM 1 /**< Maximum number of interfaces, i.e., connection to master devices */
#endif

/*---------------------------------------------------------------------------*/
process_event_t ble_event_interface_added; /**< This event is broadcast when BLE connection is established */
process_event_t ble_event_interface_deleted; /**< This event is broadcast when BLE connection is destroyed */

/*---------------------------------------------------------------------------*/
PROCESS(ble_ipsp_process, "BLE IPSP process");

/*---------------------------------------------------------------------------*/
/**
 * \brief A structure that binds IPSP connection with a peer address.
 */
typedef struct {
  eui64_t peer_addr;
  ble_ipsp_handle_t handle;
} ble_mac_interface_t;

static ble_mac_interface_t interfaces[BLE_MAC_MAX_INTERFACE_NUM];

static volatile int busy_tx; /**< Flag is set to 1 when the driver is busy transmitting a packet. */
static volatile int busy_rx; /**< Flag is set to 1 when there is a received packet pending. */

struct {
  eui64_t src;
  uint8_t payload[PACKETBUF_SIZE];
  uint16_t len;
  int8_t rssi;
} input_packet;

static mac_callback_t mac_sent_cb;
static void *mac_sent_ptr;

/*---------------------------------------------------------------------------*/
/**
 * \brief Lookup interface by IPSP connection.
 *
 * \param handle a pointer to IPSP handle.
 * \retval a pointer to interface structure
 * \retval NULL if no interface has been found for a given handle
 */
static ble_mac_interface_t *
ble_mac_interface_lookup(ble_ipsp_handle_t *handle)
{
  int i;
  for(i = 0; i < BLE_MAC_MAX_INTERFACE_NUM; i++) {
    if(interfaces[i].handle.conn_handle == handle->conn_handle &&
        interfaces[i].handle.cid == handle->cid) {
      return &interfaces[i];
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Add IPSP connection to the interface table.
 *
 * This function binds IPSP connection with peer address.
 *
 * \param peer a pointer to eui64 address
 * \param handle a pointer to IPSP handle
 *
 * \retval a pointer to an interface structure on success
 * \retval NULL if interface table is full
 */
static ble_mac_interface_t *
ble_mac_interface_add(eui64_t *peer, ble_ipsp_handle_t *handle)
{
  int i;
  for(i = 0; i < BLE_MAC_MAX_INTERFACE_NUM; i++) {
    if(interfaces[i].handle.conn_handle == 0 && interfaces[i].handle.cid == 0) {
      memcpy(&interfaces[i].handle, handle, sizeof(ble_ipsp_handle_t));
      memcpy(&interfaces[i].peer_addr, peer, sizeof(eui64_t));
      process_post(PROCESS_BROADCAST, ble_event_interface_added, NULL);
      return &interfaces[i];
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Remove interface from the interface table.
 * \param interface a pointer to interface
 */
static void
ble_mac_interface_delete(ble_mac_interface_t *interface)
{
  memset(interface, 0, sizeof(ble_mac_interface_t));
  process_post(PROCESS_BROADCAST, ble_event_interface_deleted, NULL);
}

/*---------------------------------------------------------------------------*/
/**
 * \brief Callback registered with IPSP to receive asynchronous events from the module.
 * \note This function is called from SoftDevice interrupt context.
 *
 * \param[in]   p_handle  Pointer to IPSP handle.
 * \param[in]   p_evt     Pointer to specific event, generated by IPSP module.
 *
 * \return      NRF_SUCCESS on success, otherwise NRF_ERROR_NO_MEM error.
 */
static uint32_t
ble_mac_ipsp_evt_handler_irq(ble_ipsp_handle_t *p_handle, ble_ipsp_evt_t *p_evt)
{
  uint32_t retval = NRF_SUCCESS;

  ble_mac_interface_t *p_instance = NULL;
  p_instance = ble_mac_interface_lookup(p_handle);

  if(p_handle) {
    PRINTF("ble-mac: IPSP event [handle:%d CID 0x%04X]\n", p_handle->conn_handle, p_handle->cid);
  }

  switch(p_evt->evt_id) {
    case BLE_IPSP_EVT_CHANNEL_CONNECTED: {
      eui64_t peer_addr;

      PRINTF("ble-mac: channel connected\n");

      IPV6_EUI64_CREATE_FROM_EUI48(
          peer_addr.identifier,
          p_evt->evt_param->params.ch_conn_request.peer_addr.addr,
          p_evt->evt_param->params.ch_conn_request.peer_addr.addr_type);

      p_instance = ble_mac_interface_add(&peer_addr, p_handle);

      if(p_instance != NULL) {
        PRINTF("ble-mac: added new IPSP interface\n");
      } else {
        PRINTF("ble-mac: cannot add new interface. Table is full\n");
        ble_ipsp_disconnect(p_handle);
      }
      break;
    }

    case BLE_IPSP_EVT_CHANNEL_DISCONNECTED: {
      PRINTF("ble-mac: channel disconnected\n");
      if(p_instance != NULL) {
        PRINTF("ble-mac: removed IPSP interface\n");
        ble_mac_interface_delete(p_instance);
      }
      break;
    }

    case BLE_IPSP_EVT_CHANNEL_DATA_RX: {
      PRINTF("ble-mac: data received\n");
      if(p_instance != NULL) {
        if(busy_rx) {
          PRINTF("ble-mac: packet dropped as input buffer is busy\n");
          break;
        }

        if(p_evt->evt_param->params.ch_rx.len > PACKETBUF_SIZE) {
          PRINTF("ble-mac: packet buffer is too small!\n");
          break;
        }

        busy_rx = 1;

        input_packet.len = p_evt->evt_param->params.ch_rx.len;
        memcpy(input_packet.payload, p_evt->evt_param->params.ch_rx.p_data, input_packet.len);
        memcpy(input_packet.src.identifier, p_instance->peer_addr.identifier, sizeof(eui64_t));
        sd_ble_gap_rssi_get(p_handle->conn_handle, &input_packet.rssi);

        process_poll(&ble_ipsp_process);
      } else {
        PRINTF("ble-mac: got data to unknown interface!\n");
      }
      break;
    }

    case BLE_IPSP_EVT_CHANNEL_DATA_TX_COMPLETE: {
      PRINTF("ble-mac: data transmitted\n");
      busy_tx = 0;
      break;
    }
  }

  return retval;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ble_ipsp_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_POLL) {
      packetbuf_copyfrom(input_packet.payload, input_packet.len);
      packetbuf_set_attr(PACKETBUF_ATTR_RSSI, input_packet.rssi);
      packetbuf_set_addr(PACKETBUF_ADDR_SENDER, (const linkaddr_t *)input_packet.src.identifier);
      packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_node_addr);
      busy_rx = 0;
      NETSTACK_LLSEC.input();
    }
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/**
 * \brief Lookup IPSP handle by peer address.
 *
 * \param addr a pointer to eui64 address.
 * \retval a pointer to IPSP handle on success
 * \retval NULL if an IPSP handle for given address haven't been found
 */
static ble_ipsp_handle_t *
find_handle(const linkaddr_t *addr)
{
  int i;
  for(i = 0; i < BLE_MAC_MAX_INTERFACE_NUM; i++) {
    if(linkaddr_cmp((const linkaddr_t *)&interfaces[i].peer_addr, addr)) {
      return &interfaces[i].handle;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Send packet on a given IPSP handle.
 *
 * \param handle a pointer to IPSP handle.
 * \return 1 on success, 0 otherwise
 */
static int
send_to_peer(ble_ipsp_handle_t *handle)
{
  PRINTF("ble-mac: sending packet[GAP handle:%d CID:0x%04X]\n", handle->conn_handle, handle->cid);
  return (ble_ipsp_send(handle, packetbuf_dataptr(), packetbuf_datalen()) == NRF_SUCCESS);
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  int i;
  const linkaddr_t *dest;
  ble_ipsp_handle_t *handle;
  int ret = 0;

  mac_sent_cb = sent;
  mac_sent_ptr = ptr;

  dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);

  if(linkaddr_cmp(dest, &linkaddr_null)) {
    for(i = 0; i < BLE_MAC_MAX_INTERFACE_NUM; i++) {
      if(interfaces[i].handle.cid != 0 && interfaces[i].handle.conn_handle != 0) {
        ret = send_to_peer(&interfaces[i].handle);
        watchdog_periodic();
      }
    }
  } else if((handle = find_handle(dest)) != NULL) {
    ret = send_to_peer(handle);
  } else {
    PRINTF("ble-mac: no connection found for peer");
  }

  if(ret) {
    busy_tx = 1;
    while(busy_tx) {
      watchdog_periodic();
      sd_app_evt_wait();
    }
    mac_call_sent_callback(sent, ptr, MAC_TX_OK, 1);
  } else {
    mac_call_sent_callback(sent, ptr, MAC_TX_ERR, 1);
  }
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
// Initialize IPSP service
  uint32_t err_code;
  ble_ipsp_init_t ipsp_init_params;

  memset(&ipsp_init_params, 0, sizeof(ipsp_init_params));
  ipsp_init_params.evt_handler = ble_mac_ipsp_evt_handler_irq;
  err_code = ble_ipsp_init(&ipsp_init_params);
  APP_ERROR_CHECK(err_code);

  ble_event_interface_added = process_alloc_event();
  ble_event_interface_deleted = process_alloc_event();

  process_start(&ble_ipsp_process, NULL);
}
/*---------------------------------------------------------------------------*/
const struct mac_driver ble_ipsp_mac_driver = {
  "nRF52 IPSP driver",
  init,
  send_packet,
  NULL,
  on,
  off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
