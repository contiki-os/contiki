/*
 * Copyright (c) 2014, George Oikonomou (george@contiki-os.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
/**
 *   Example project demonstrating the extended RF API functionality
 */
#include "contiki.h"
#include "net/netstack.h"
#include "dev/radio.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
struct rf_consts {
  radio_value_t channel_min;
  radio_value_t channel_max;
  radio_value_t txpower_min;
  radio_value_t txpower_max;
};

static struct rf_consts consts;

static radio_value_t value;
static uint8_t ext_addr[8];
/*---------------------------------------------------------------------------*/
PROCESS(extended_rf_api_process, "Extended RF API demo process");
AUTOSTART_PROCESSES(&extended_rf_api_process);
/*---------------------------------------------------------------------------*/
static void
print_64bit_addr(const uint8_t *addr)
{
  unsigned int i;
  for(i = 0; i < 7; i++) {
    printf("%02x:", addr[i]);
  }
  printf("%02x (network order)\n", addr[7]);
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  radio_result_t rv;

  rv = NETSTACK_RADIO.get_object(param, dest, size);

  switch(rv) {
  case RADIO_RESULT_ERROR:
    printf("Radio returned an error\n");
    break;
  case RADIO_RESULT_INVALID_VALUE:
    printf("Value is invalid\n");
    break;
  case RADIO_RESULT_NOT_SUPPORTED:
    printf("Param %u not supported\n", param);
    break;
  case RADIO_RESULT_OK:
    break;
  default:
    printf("Unknown return value\n");
    break;
  }

  return rv;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, void *src, size_t size)
{
  radio_result_t rv;

  rv = NETSTACK_RADIO.set_object(param, src, size);

  switch(rv) {
  case RADIO_RESULT_ERROR:
    printf("Radio returned an error\n");
    break;
  case RADIO_RESULT_INVALID_VALUE:
    printf("Value is invalid\n");
    break;
  case RADIO_RESULT_NOT_SUPPORTED:
    printf("Param %u not supported\n", param);
    break;
  case RADIO_RESULT_OK:
    break;
  default:
    printf("Unknown return value\n");
    break;
  }

  return rv;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_param(radio_param_t param, radio_value_t *value)
{
  radio_result_t rv;

  rv = NETSTACK_RADIO.get_value(param, value);

  switch(rv) {
  case RADIO_RESULT_ERROR:
    printf("Radio returned an error\n");
    break;
  case RADIO_RESULT_INVALID_VALUE:
    printf("Value %d is invalid\n", *value);
    break;
  case RADIO_RESULT_NOT_SUPPORTED:
    printf("Param %u not supported\n", param);
    break;
  case RADIO_RESULT_OK:
    break;
  default:
    printf("Unknown return value\n");
    break;
  }

  return rv;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_param(radio_param_t param, radio_value_t value)
{
  radio_result_t rv;

  rv = NETSTACK_RADIO.set_value(param, value);

  switch(rv) {
  case RADIO_RESULT_ERROR:
    printf("Radio returned an error\n");
    break;
  case RADIO_RESULT_INVALID_VALUE:
    printf("Value %d is invalid\n", value);
    break;
  case RADIO_RESULT_NOT_SUPPORTED:
    printf("Param %u not supported\n", param);
    break;
  case RADIO_RESULT_OK:
    break;
  default:
    printf("Unknown return value\n");
    break;
  }

  return rv;
}
/*---------------------------------------------------------------------------*/
static void
get_rf_consts(void)
{
  printf("====================================\n");
  printf("RF Constants\n");
  printf("Min Channel : ");
  if(get_param(RADIO_CONST_CHANNEL_MIN, &consts.channel_min) == RADIO_RESULT_OK) {
    printf("%3d\n", consts.channel_min);
  }

  printf("Max Channel : ");
  if(get_param(RADIO_CONST_CHANNEL_MAX, &consts.channel_max) == RADIO_RESULT_OK) {
    printf("%3d\n", consts.channel_max);
  }

  printf("Min TX Power: ");
  if(get_param(RADIO_CONST_TXPOWER_MIN, &consts.txpower_min) == RADIO_RESULT_OK) {
    printf("%3d dBm\n", consts.txpower_min);
  }

  printf("Max TX Power: ");
  if(get_param(RADIO_CONST_TXPOWER_MAX, &consts.txpower_max) == RADIO_RESULT_OK) {
    printf("%3d dBm\n", consts.txpower_max);
  }
}
/*---------------------------------------------------------------------------*/
static void
test_off_on(void)
{
  printf("====================================\n");
  printf("Power mode Test: Off, then On\n");

  printf("Power mode is  : ");
  if(get_param(RADIO_PARAM_POWER_MODE, &value) == RADIO_RESULT_OK) {
    if(value == RADIO_POWER_MODE_ON) {
      printf("On\n");
    } else if(value == RADIO_POWER_MODE_OFF) {
      printf("Off\n");
    }
  }

  printf("Turning Off    : ");
  value = RADIO_POWER_MODE_OFF;
  set_param(RADIO_PARAM_POWER_MODE, value);
  if(get_param(RADIO_PARAM_POWER_MODE, &value) == RADIO_RESULT_OK) {
    if(value == RADIO_POWER_MODE_ON) {
      printf("On\n");
    } else if(value == RADIO_POWER_MODE_OFF) {
      printf("Off\n");
    }
  }

  printf("Turning On     : ");
  value = RADIO_POWER_MODE_ON;
  set_param(RADIO_PARAM_POWER_MODE, value);
  if(get_param(RADIO_PARAM_POWER_MODE, &value) == RADIO_RESULT_OK) {
    if(value == RADIO_POWER_MODE_ON) {
      printf("On\n");
    } else if(value == RADIO_POWER_MODE_OFF) {
      printf("Off\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
test_channels(void)
{
  int i;

  printf("====================================\n");
  printf("Channel Test: [%u , %u]\n", consts.channel_min, consts.channel_max);

  for(i = consts.channel_min; i <= consts.channel_max; i++) {
    value = i;
    printf("Switch to: %d, Now: ", value);
    set_param(RADIO_PARAM_CHANNEL, value);
    if(get_param(RADIO_PARAM_CHANNEL, &value) == RADIO_RESULT_OK) {
      printf("%d\n", value);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
test_rx_modes(void)
{
  int i;

  printf("====================================\n");
  printf("RX Modes Test: [0 , 3]\n");

  for(i = 0; i <= 3; i++) {
    value = i;
    printf("Switch to: %d, Now: ", value);
    set_param(RADIO_PARAM_RX_MODE, value);
    if(get_param(RADIO_PARAM_RX_MODE, &value) == RADIO_RESULT_OK) {
      printf("Address Filtering is ");
      if(value & RADIO_RX_MODE_ADDRESS_FILTER) {
        printf("On, ");
      } else {
        printf("Off, ");
      }
      printf("Auto ACK is ");
      if(value & RADIO_RX_MODE_AUTOACK) {
        printf("On, ");
      } else {
        printf("Off, ");
      }

      printf("(value=%d)\n", value);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
test_tx_powers(void)
{
  int i;

  printf("====================================\n");
  printf("TX Power Test: [%d , %d]\n", consts.txpower_min, consts.txpower_max);

  for(i = consts.txpower_min; i <= consts.txpower_max; i += 5) {
    value = i;
    printf("Switch to: %3d dBm, Now: ", value);
    set_param(RADIO_PARAM_TXPOWER, value);
    if(get_param(RADIO_PARAM_TXPOWER, &value) == RADIO_RESULT_OK) {
      printf("%3d dBm\n", value);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
test_cca_thresholds(void)
{
  printf("====================================\n");
  printf("CCA Thres. Test: -105, then -81\n");

  value = -105;
  printf("Switch to: %4d dBm, Now: ", value);
  set_param(RADIO_PARAM_CCA_THRESHOLD, value);
  if(get_param(RADIO_PARAM_CCA_THRESHOLD, &value) == RADIO_RESULT_OK) {
    printf("%4d dBm [0x%04x]\n", value, (uint16_t)value);
  }

  value = -81;
  printf("Switch to: %4d dBm, Now: ", value);
  set_param(RADIO_PARAM_CCA_THRESHOLD, value);
  if(get_param(RADIO_PARAM_CCA_THRESHOLD, &value) == RADIO_RESULT_OK) {
    printf("%4d dBm [0x%04x]\n", value, (uint16_t)value);
  }
}
/*---------------------------------------------------------------------------*/
static void
test_pan_id(void)
{
  radio_value_t new_val;

  printf("====================================\n");
  printf("PAN ID Test: Flip bytes and back\n");

  printf("PAN ID is: ");
  if(get_param(RADIO_PARAM_PAN_ID, &value) == RADIO_RESULT_OK) {
    printf("0x%02x%02x\n", (value >> 8) & 0xFF, value & 0xFF);
  }

  new_val = (value >> 8) & 0xFF;
  new_val |= (value & 0xFF) << 8;
  printf("Switch to: 0x%02x%02x, Now: ", (new_val >> 8) & 0xFF, new_val & 0xFF);
  set_param(RADIO_PARAM_PAN_ID, new_val);
  if(get_param(RADIO_PARAM_PAN_ID, &value) == RADIO_RESULT_OK) {
    printf("0x%02x%02x\n", (value >> 8) & 0xFF, value & 0xFF);
  }

  new_val = (value >> 8) & 0xFF;
  new_val |= (value & 0xFF) << 8;
  printf("Switch to: 0x%02x%02x, Now: ", (new_val >> 8) & 0xFF, new_val & 0xFF);
  set_param(RADIO_PARAM_PAN_ID, new_val);
  if(get_param(RADIO_PARAM_PAN_ID, &value) == RADIO_RESULT_OK) {
    printf("0x%02x%02x\n", (value >> 8) & 0xFF, value & 0xFF);
  }
}
/*---------------------------------------------------------------------------*/
static void
test_16bit_addr(void)
{
  radio_value_t new_val;

  printf("====================================\n");
  printf("16-bit Address Test: Flip bytes and back\n");

  printf("16-bit Address is: ");
  if(get_param(RADIO_PARAM_16BIT_ADDR, &value) == RADIO_RESULT_OK) {
    printf("0x%02x%02x\n", (value >> 8) & 0xFF, value & 0xFF);
  }

  new_val = (value >> 8) & 0xFF;
  new_val |= (value & 0xFF) << 8;
  printf("Switch to: 0x%02x%02x, Now: ", (new_val >> 8) & 0xFF, new_val & 0xFF);
  set_param(RADIO_PARAM_16BIT_ADDR, new_val);
  if(get_param(RADIO_PARAM_16BIT_ADDR, &value) == RADIO_RESULT_OK) {
    printf("0x%02x%02x\n", (value >> 8) & 0xFF, value & 0xFF);
  }

  new_val = (value >> 8) & 0xFF;
  new_val |= (value & 0xFF) << 8;
  printf("Switch to: 0x%02x%02x, Now: ", (new_val >> 8) & 0xFF, new_val & 0xFF);
  set_param(RADIO_PARAM_16BIT_ADDR, new_val);
  if(get_param(RADIO_PARAM_16BIT_ADDR, &value) == RADIO_RESULT_OK) {
    printf("0x%02x%02x\n", (value >> 8) & 0xFF, value & 0xFF);
  }
}
/*---------------------------------------------------------------------------*/
static void
test_64bit_addr(void)
{
  int i;
  uint8_t new_val[8];

  printf("====================================\n");
  printf("64-bit Address Test: Invert byte order\n");

  printf("64-bit Address is: ");
  if(get_object(RADIO_PARAM_64BIT_ADDR, ext_addr, 8) == RADIO_RESULT_OK) {
    print_64bit_addr(ext_addr);
  }

  for(i = 0; i <= 7; i++) {
    new_val[7 - i] = ext_addr[i];
  }

  printf("Setting to       : ");
  print_64bit_addr(new_val);

  printf("64-bit Address is: ");
  set_object(RADIO_PARAM_64BIT_ADDR, new_val, 8);
  if(get_object(RADIO_PARAM_64BIT_ADDR, ext_addr, 8) == RADIO_RESULT_OK) {
    print_64bit_addr(ext_addr);
  }
}
/*---------------------------------------------------------------------------*/
static void
print_rf_values(void)
{
  printf("====================================\n");
  printf("RF Values\n");

  printf("Power: ");
  if(get_param(RADIO_PARAM_POWER_MODE, &value) == RADIO_RESULT_OK) {
    if(value == RADIO_POWER_MODE_ON) {
      printf("On\n");
    } else if(value == RADIO_POWER_MODE_OFF) {
      printf("Off\n");
    }
  }

  printf("Channel: ");
  if(get_param(RADIO_PARAM_CHANNEL, &value) == RADIO_RESULT_OK) {
    printf("%d\n", value);
  }

  printf("PAN ID: ");
  if(get_param(RADIO_PARAM_PAN_ID, &value) == RADIO_RESULT_OK) {
    printf("0x%02x%02x\n", (value >> 8) & 0xFF, value & 0xFF);
  }

  printf("16-bit Address: ");
  if(get_param(RADIO_PARAM_16BIT_ADDR, &value) == RADIO_RESULT_OK) {
    printf("0x%02x%02x\n", (value >> 8) & 0xFF, value & 0xFF);
  }

  printf("64-bit Address: ");
  if(get_object(RADIO_PARAM_64BIT_ADDR, ext_addr, 8) == RADIO_RESULT_OK) {
    print_64bit_addr(ext_addr);
  }

  printf("RX Mode: ");
  if(get_param(RADIO_PARAM_RX_MODE, &value) == RADIO_RESULT_OK) {
    printf("Address Filtering is ");
    if(value & RADIO_RX_MODE_ADDRESS_FILTER) {
      printf("On, ");
    } else {
      printf("Off, ");
    }
    printf("Auto ACK is ");
    if(value & RADIO_RX_MODE_AUTOACK) {
      printf("On, ");
    } else {
      printf("Off, ");
    }

    printf("(value=%d)\n", value);
  }

  printf("TX Mode: ");
  if(get_param(RADIO_PARAM_TX_MODE, &value) == RADIO_RESULT_OK) {
    printf("%d\n", value);
  }

  printf("TX Power: ");
  if(get_param(RADIO_PARAM_TXPOWER, &value) == RADIO_RESULT_OK) {
    printf("%d dBm [0x%04x]\n", value, (uint16_t)value);
  }

  printf("CCA Threshold: ");
  if(get_param(RADIO_PARAM_CCA_THRESHOLD, &value) == RADIO_RESULT_OK) {
    printf("%d dBm [0x%04x]\n", value, (uint16_t)value);
  }

  printf("RSSI: ");
  if(get_param(RADIO_PARAM_RSSI, &value) == RADIO_RESULT_OK) {
    printf("%d dBm [0x%04x]\n", value, (uint16_t)value);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(extended_rf_api_process, ev, data)
{

  PROCESS_BEGIN();

  get_rf_consts();
  print_rf_values();

  test_off_on();
  test_channels();
  test_rx_modes();
  test_tx_powers();
  test_cca_thresholds();
  test_pan_id();
  test_16bit_addr();
  test_64bit_addr();

  printf("Done\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
