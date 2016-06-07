/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-AT-master-test
 * @{
 *
 * Test the Zoul hardware using AT commands
 * @{
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "cpu.h"
#include "at-master.h"
#include "sys/ctimer.h"
#include "sys/process.h"
#include "dev/adc.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "dev/sys-ctrl.h"
#include "dev/gpio.h"
#include "dev/ioc.h"
#include "net/rime/rime.h"
#include "lib/list.h"
#include "dev/sha256.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
PROCESS(at_test_process, "AT test process");
AUTOSTART_PROCESSES(&at_test_process);
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static struct at_cmd at_cmd_test;
static struct at_cmd at_cmd_board;
static struct at_cmd at_cmd_led;
static struct at_cmd at_cmd_addr;
static struct at_cmd at_cmd_gpio;
static struct at_cmd at_cmd_read;
static struct at_cmd at_cmd_flop;
static struct at_cmd at_cmd_reset;
static struct at_cmd at_cmd_sha256;
static struct at_cmd at_cmd_adc;
/*---------------------------------------------------------------------------*/
#define HWTEST_GPIO_INPUT            0
#define HWTEST_GPIO_OUTPUT           1
#define HWTEST_GPIO_OUTPUT_ODD       3
#define HWTEST_GPIO_OUTPUT_LIST      4
#define HWTEST_GPIO_OUTPUT_MASK      0x55
#define HWTEST_GPIO_OUTPUT_ODD_MASK  0xAA
/*---------------------------------------------------------------------------*/
typedef struct {
  char *name;
  uint8_t port;
  uint8_t pin;
} gpio_list_t;
/*---------------------------------------------------------------------------*/
static struct ctimer ct;
/*---------------------------------------------------------------------------*/
static void
floppin(uint8_t port, uint8_t pin)
{
  uint8_t i;
  GPIO_CLR_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  clock_delay_usec(500);
  for(i = 0; i < 50; i++) {
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
    clock_delay_usec(500);
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
    clock_delay_usec(500);
  }
}
/*---------------------------------------------------------------------------*/
#if DEBUG
static char *
pname(uint8_t num)
{
  if(num == GPIO_A_NUM) {
    return "PA";
  }
  if(num == GPIO_B_NUM) {
    return "PB";
  }
  if(num == GPIO_C_NUM) {
    return "PC";
  }
  if(num == GPIO_D_NUM) {
    return "PD";
  }
  return "INVALID";
}
#endif
/*---------------------------------------------------------------------------*/
static void
config_gpio(uint8_t port, uint8_t pin, uint8_t type)
{
  GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  if(type == HWTEST_GPIO_OUTPUT) {
    GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  } else if(type == HWTEST_GPIO_INPUT) {
    GPIO_SET_INPUT(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  }
}
/*---------------------------------------------------------------------------*/
static void
at_cmd_test_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  AT_RESPONSE("Hello!");
  AT_RESPONSE(AT_DEFAULT_RESPONSE_OK);
}
/*---------------------------------------------------------------------------*/
static void
at_cmd_board_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  AT_RESPONSE(BOARD_STRING);
  AT_RESPONSE(AT_DEFAULT_RESPONSE_OK);
}
/*---------------------------------------------------------------------------*/
static void
at_cmd_flop_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  /* Format: AT&FLOP=PN where P(ort)N(number) */
  uint8_t port;
  uint8_t pin = atoi(&data[9]);

  if((pin < 0) || (pin > 9)) {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  if(strncmp(&data[8], "A", 1) == 0) {
    port = GPIO_A_NUM;
  } else if(strncmp(&data[8], "B", 1) == 0) {
    port = GPIO_B_NUM;
  } else if(strncmp(&data[8], "C", 1) == 0) {
    port = GPIO_C_NUM;
  } else if(strncmp(&data[8], "D", 1) == 0) {
    port = GPIO_D_NUM;
  } else {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  config_gpio(port, pin, HWTEST_GPIO_OUTPUT);
  floppin(port, pin);

  AT_RESPONSE(AT_DEFAULT_RESPONSE_OK);
}
/*---------------------------------------------------------------------------*/
static void
at_cmd_address_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  static char _lladdr[17];
  snprintf(_lladdr, 17, "%02x%02x%02x%02x%02x%02x%02x%02x",
           linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
           linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[3],
           linkaddr_node_addr.u8[4], linkaddr_node_addr.u8[5],
           linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  AT_RESPONSE(_lladdr);
  AT_RESPONSE(AT_DEFAULT_RESPONSE_OK);
}
/*---------------------------------------------------------------------------*/
static void
at_cmd_reset_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  uint8_t reset_val = atoi(&data[9]);
  /* AT&RESET=n, where n:
   * 0 : CC2538 soft reset
   */
  if((reset_val != 0) && (reset_val != 1)) {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  /* Send the response and wait a second until executing the command */
  AT_RESPONSE(AT_DEFAULT_RESPONSE_OK);

  if(reset_val == 0) {
    ctimer_set(&ct, CLOCK_SECOND, sys_ctrl_reset, NULL);
  }
}
/*---------------------------------------------------------------------------*/
static void
at_cmd_leds_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  /* Format: AT&LED=L,s where L(ed)=R/G/B, s(tate)=1/0*/
  uint8_t led;
  uint8_t state = strncmp(&data[9], "1", 1) ? 0 : 1;

  if(strncmp(&data[8], ",", 1) != 0) {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  if(strncmp(&data[7], "R", 1) == 0) {
    led = LEDS_RED;
  } else if(strncmp(&data[7], "G", 1) == 0) {
    led = LEDS_GREEN;
  } else if(strncmp(&data[7], "B", 1) == 0) {
    led = LEDS_BLUE;
  } else {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  if(state) {
    leds_on(led);
  } else {
    leds_off(led);
  }
  AT_RESPONSE(AT_DEFAULT_RESPONSE_OK);
}
/*---------------------------------------------------------------------------*/
static void
at_cmd_gpio_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  /* Format: AT&GPIO=PN,s where P(ort)N(number), s(tate)=1/0 */
  uint8_t port;
  uint8_t state = strncmp(&data[11], "1", 1) ? 0 : 1;
  uint8_t pin = atoi(&data[9]);

  if(strncmp(&data[10], ",", 1) != 0) {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  if((pin < 0) || (pin > 7)) {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  if((state < 0) || (state > 1)) {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  if(strncmp(&data[8], "A", 1) == 0) {
    port = GPIO_A_NUM;
  } else if(strncmp(&data[8], "B", 1) == 0) {
    port = GPIO_B_NUM;
  } else if(strncmp(&data[8], "C", 1) == 0) {
    port = GPIO_C_NUM;
  } else if(strncmp(&data[8], "D", 1) == 0) {
    port = GPIO_D_NUM;
  } else {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  config_gpio(port, pin, HWTEST_GPIO_OUTPUT);

  if(state) {
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  } else {
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  }

  AT_RESPONSE(AT_DEFAULT_RESPONSE_OK);
}
/*---------------------------------------------------------------------------*/
static void
at_cmd_adc_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  /* Format: AT&ADC=N where N is 4-7, it can be "*" to read all */
  uint8_t i, pin;
  uint16_t res[4];
  char read_result[24];

  if(strncmp(&data[7], "*", 1) == 0) {
    pin = 8;
  } else {
    pin = atoi(&data[7]);
  }

  if((pin < 4) || (pin > 8)) {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  if(pin < 8) {
    config_gpio(GPIO_A_NUM, pin, HWTEST_GPIO_INPUT);
    ioc_set_over(GPIO_A_NUM, pin, IOC_OVERRIDE_ANA);
    res[pin - 4] = adc_get((SOC_ADC_ADCCON_CH_AIN0 + pin),
                           SOC_ADC_ADCCON_REF_AVDD5,
                           SOC_ADC_ADCCON_DIV_512);
    res[pin - 4] = res[pin - 4] / 10;
    PRINTF("ADC%u: %04d\n", pin, res[pin - 4]);
    snprintf(read_result, 5, "%04d", res[pin - 4]);
  } else {
    for(i = 4; i < 8; i++) {
      config_gpio(GPIO_A_NUM, i, HWTEST_GPIO_INPUT);
      ioc_set_over(GPIO_A_NUM, i, IOC_OVERRIDE_ANA);
      res[i - 4] = adc_get((SOC_ADC_ADCCON_CH_AIN0 + i),
                           SOC_ADC_ADCCON_REF_AVDD5,
                           SOC_ADC_ADCCON_DIV_512);
      res[i - 4] = res[i - 4] / 10;
    }
    snprintf(read_result, 24, "%04d %04d %04d %04d", res[0], res[1],
             res[2], res[3]);
  }

  AT_RESPONSE(read_result);
  AT_RESPONSE(AT_DEFAULT_RESPONSE_OK);
}
/*---------------------------------------------------------------------------*/
static void
at_cmd_read_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  /* Format: AT&READ=PN where P(ort)N(number), N can be "*" to read all */
  uint8_t port, pin;
  char read_result[5];

  if(strncmp(&data[9], "*", 1) == 0) {
    pin = 0xFF;
  } else {
    pin = atoi(&data[9]);
  }

  if((pin < 0) || (pin > 7)) {
    if(pin != 0xFF) {
      AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
      return;
    }
  }

  if(pin < 8) {
    pin = GPIO_PIN_MASK(pin);
  }

  /* Exclude PA0-PA3  */
  if(strncmp(&data[8], "A", 1) == 0) {
    port = GPIO_A_NUM;
    if(pin < 0x1F) {
      AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
      return;
    } else {
      if(pin == 0xFF) {
        pin = 0xF0;
      }
    }
  } else if(strncmp(&data[8], "B", 1) == 0) {
    port = GPIO_B_NUM;
  } else if(strncmp(&data[8], "C", 1) == 0) {
    port = GPIO_C_NUM;
  } else if(strncmp(&data[8], "D", 1) == 0) {
    port = GPIO_D_NUM;
  } else {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  config_gpio(port, pin, HWTEST_GPIO_INPUT);
  snprintf(read_result, 5, "0x%02X",
           (uint16_t)GPIO_READ_PIN(GPIO_PORT_TO_BASE(port), pin));
  AT_RESPONSE(read_result);
  AT_RESPONSE(AT_DEFAULT_RESPONSE_OK);
}
/*---------------------------------------------------------------------------*/
static void
at_cmd_sha256_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  /* Format: AT&SHA256=s, where s is a string up to 64 bytes */
  uint8_t i;
  char tmp[4], sha256[32], sha256_res[64];
  static sha256_state_t state;

  crypto_init();
  if(sha256_init(&state) != CRYPTO_SUCCESS) {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  if(sha256_process(&state, &data[10],
                    len - (cmd->cmd_hdr_len)) != CRYPTO_SUCCESS) {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  if(sha256_done(&state, sha256) != CRYPTO_SUCCESS) {
    AT_RESPONSE(AT_DEFAULT_RESPONSE_ERROR);
    return;
  }

  crypto_disable();

  PRINTF("Input: %s:\n", &data[10]);
  snprintf(tmp, 3, "%02X", sha256[0]);
  strncpy(sha256_res, tmp, 3);
  for(i = 1; i < 32; i++) {
    PRINTF("0x%02X ", sha256[i]);
    snprintf(tmp, 3, "%02X", sha256[i]);
    strcat(sha256_res, tmp);
  }
  PRINTF("\nSHA256: %s\n", sha256_res);
  AT_RESPONSE(sha256_res);
  AT_RESPONSE(AT_DEFAULT_RESPONSE_OK);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(at_test_process, ev, data)
{
  PROCESS_BEGIN();
  struct at_cmd *a;

  /* Initialize the driver, default is UART0 */
  at_init(0);

  /* Register a list of commands, is mandatory to start with "AT" */
  at_register(&at_cmd_test, &at_test_process, "AT", 2, 2,
              at_cmd_test_callback);
  at_register(&at_cmd_board, &at_test_process, "AT&V", 4, 4,
              at_cmd_board_callback);
  at_register(&at_cmd_led, &at_test_process, "AT&LED", 6, 10,
              at_cmd_leds_callback);
  at_register(&at_cmd_addr, &at_test_process, "AT&A", 4, 4,
              at_cmd_address_callback);
  at_register(&at_cmd_gpio, &at_test_process, "AT&GPIO=", 8, 12,
              at_cmd_gpio_callback);
  at_register(&at_cmd_read, &at_test_process, "AT&READ=", 8, 10,
              at_cmd_read_callback);
  at_register(&at_cmd_adc, &at_test_process, "AT&ADC=", 7, 8,
              at_cmd_adc_callback);
  at_register(&at_cmd_flop, &at_test_process, "AT&FLOP=", 8, 10,
              at_cmd_flop_callback);
  at_register(&at_cmd_reset, &at_test_process, "AT&RESET=", 9, 10,
              at_cmd_reset_callback);
  at_register(&at_cmd_sha256, &at_test_process, "AT&SHA256=", 10, 64,
              at_cmd_sha256_callback);

  /* Print the command list */
  PRINTF("AT command list:\n");
  for(a = at_list(); a != NULL; a = list_item_next(a)) {
    PRINTF("* HDR %s LEN %u MAX %u\n", a->cmd_header, a->cmd_hdr_len,
           a->cmd_max_len);
  }

  /*
   * When an AT command is received over the serial line, the registered
   * callbacks will be invoked, let the process spin until then
   */
  while(1) {
    PROCESS_YIELD();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */
