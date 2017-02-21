/*
 * Copyright (c) 2017, Arthur Courtel
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
 * Author: Arthur Courtel <arthurcourtel@gmail.com>
 *
 */
#include "notify.h"
#include "net/netstack.h"
#include "net/packetbuf.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static att_buffer_t g_tx_buffer_notify;

#define ATT_CID       4

/*---------------------------------------------------------------------------*/
void
prepare_notification(uint16_t handle_to_notify, bt_size_t *sensor_value)
{
  g_tx_buffer_notify.sdu[0] = ATT_HANDLE_VALUE_NOTIFICATION;
  memcpy(&g_tx_buffer_notify.sdu[1], &handle_to_notify, sizeof(handle_to_notify));
  memcpy(&g_tx_buffer_notify.sdu[3], &sensor_value->value, sensor_value->type);
  /* set sdu length */
  g_tx_buffer_notify.sdu_length = LENGHT_ATT_HEADER_NOTIFICATION + sensor_value->type;
}
/*---------------------------------------------------------------------------*/
void
prepare_error_resp_notif(uint16_t error_handle, uint8_t error)
{
  /* Response code */
  g_tx_buffer_notify.sdu[0] = ATT_ERROR_RESPONSE;
  /* Operation asked */
  g_tx_buffer_notify.sdu[1] = ATT_HANDLE_VALUE_NOTIFICATION;
  /* Attribute handle that generate an error */
  memcpy(&g_tx_buffer_notify.sdu[2], &error_handle, sizeof(error_handle));
  /* Error code */
  g_tx_buffer_notify.sdu[4] = error;
  /* set sdu length */
  g_tx_buffer_notify.sdu_length = 5;
}
/*---------------------------------------------------------------------------*/
void
send_notify()
{
  PRINTF("SEND NOTIFY\n");
  memcpy(packetbuf_dataptr(), g_tx_buffer_notify.sdu, g_tx_buffer_notify.sdu_length);
  packetbuf_set_datalen(g_tx_buffer_notify.sdu_length);
  packetbuf_set_attr(PACKETBUF_ATTR_CHANNEL, ATT_CID);
  NETSTACK_MAC.send(NULL, NULL);
}
/*---------------------------------------------------------------------------*/
int
is_values_equals(bt_size_t *v1, bt_size_t *v2)
{
  PRINTF("comparaison : %d", memcmp(v1, v2, sizeof(bt_size_t)));
  return memcmp(v1, v2, sizeof(bt_size_t));
}
/*---------------------------------------------------------------------------*/
