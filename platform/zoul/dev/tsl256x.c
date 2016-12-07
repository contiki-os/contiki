/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 * \addtogroup zoul-tsl256x-sensor
 * @{
 *
 * \file
 *  Driver for the external TSL256X light sensor
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 *         Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/i2c.h"
#include "dev/gpio.h"
#include "dev/zoul-sensors.h"
#include "lib/sensors.h"
#include "tsl256x.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define TSL256X_INT_PORT_BASE  GPIO_PORT_TO_BASE(I2C_INT_PORT)
#define TSL256X_INT_PIN_MASK   GPIO_PIN_MASK(I2C_INT_PIN)
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
static uint8_t gain;
static uint8_t timming;
/*---------------------------------------------------------------------------*/
void (*tsl256x_int_callback)(uint8_t value);
/*---------------------------------------------------------------------------*/
static uint16_t
calculate_lux(uint8_t *buf)
{
  uint32_t ch0, ch1, chscale = 0;
  uint32_t ratio = 0;
  uint32_t lratio, tmp = 0;
  uint16_t buffer[2];

  /* The calculations below assume the integration time is 402ms and the gain
   * is 16x (nominal), if not then it is required to normalize the reading
   * before converting to lux
   */

  buffer[0] = (buf[1] << 8 | (buf[0]));
  buffer[1] = (buf[3] << 8 | (buf[2]));

  switch(timming) {
  case TSL256X_TIMMING_INTEG_402MS:
    chscale = (1 << CH_SCALE);
    break;
  case TSL256X_TIMMING_INTEG_101MS:
    chscale = CHSCALE_TINT1;
    break;
  case TSL256X_TIMMING_INTEG_13_7MS:
    chscale = CHSCALE_TINT0;
    break;
  }

  if(!gain) {
    chscale = chscale << 4;
  }

  ch0 = (buffer[0] * chscale) >> CH_SCALE;
  ch1 = (buffer[1] * chscale) >> CH_SCALE;

  if(ch0 > 0) {
    ratio = (ch1 << CH_SCALE);
    ratio = ratio / ch0;
  }

  lratio = (ratio + 1) >> 1;

  if((lratio >= 0) && (lratio <= K1T)) {
    tmp = (ch0 * B1T) - (ch1 * M1T);
  } else if(lratio <= K2T) {
    tmp = (ch0 * B2T) - (ch1 * M2T);
  } else if(lratio <= K3T) {
    tmp = (ch0 * B3T) - (ch1 * M3T);
  } else if(lratio <= K4T) {
    tmp = (ch0 * B4T) - (ch1 * M4T);
  } else if(lratio <= K5T) {
    tmp = (ch0 * B5T) - (ch1 * M5T);
  } else if(lratio <= K6T) {
    tmp = (ch0 * B6T) - (ch1 * M6T);
  } else if(lratio <= K7T) {
    tmp = (ch0 * B7T) - (ch1 * M7T);
  } else if(lratio > K8T) {
    tmp = (ch0 * B8T) - (ch1 * M8T);
  }

  if(tmp < 0) {
    tmp = 0;
  }

  tmp += (1 << (LUX_SCALE - 1));
  return tmp >> LUX_SCALE;
}
/*---------------------------------------------------------------------------*/
static int
tsl256x_read_reg(uint8_t reg, uint8_t *buf, uint8_t regNum)
{
  i2c_master_enable();
  if(i2c_single_send(TSL256X_ADDR, reg) == I2C_MASTER_ERR_NONE) {
    while(i2c_master_busy());
    if(i2c_burst_receive(TSL256X_ADDR, buf, regNum) == I2C_MASTER_ERR_NONE) {
      return TSL256X_SUCCESS;
    }
  }
  return TSL256X_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
tsl256x_write_reg(uint8_t *buf, uint8_t num)
{
  if((buf == NULL) || (num <= 0)) {
    PRINTF("TSL256X: invalid write values\n");
    return TSL256X_ERROR;
  }

  i2c_master_enable();
  if(i2c_burst_send(TSL256X_ADDR, buf, num) == I2C_MASTER_ERR_NONE) {
    return TSL256X_SUCCESS;
  }
  return TSL256X_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
tsl256x_on(void)
{
  uint8_t buf[2];
  buf[0] = (TSL256X_COMMAND + TSL256X_CONTROL);
  buf[1] = TSL256X_CONTROL_POWER_ON;

  if(tsl256x_write_reg(buf, 2) == I2C_MASTER_ERR_NONE) {
    if(i2c_single_receive(TSL256X_ADDR, &buf[0]) == I2C_MASTER_ERR_NONE) {
      if((buf[0] & 0x0F) == TSL256X_CONTROL_POWER_ON) {
        PRINTF("TSL256X: powered on\n");
        return TSL256X_SUCCESS;
      }
    }
  }

  PRINTF("TSL256X: failed to power on\n");
  return TSL256X_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
tsl256x_id_register(uint8_t *buf)
{
  if(tsl256x_read_reg((TSL256X_COMMAND + TSL256X_ID_REG),
                      buf, 1) == TSL256X_SUCCESS) {
    PRINTF("TSL256X: partnum/revnum 0x%02X\n", *buf);
    return TSL256X_SUCCESS;
  }

  return TSL256X_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
tsl256x_off(void)
{
  uint8_t buf[2];
  buf[0] = (TSL256X_COMMAND + TSL256X_CONTROL);
  buf[1] = TSL256X_CONTROL_POWER_OFF;

  if(tsl256x_write_reg(buf, 2) == I2C_MASTER_ERR_NONE) {
    PRINTF("TSL256X: powered off\n");
    return TSL256X_SUCCESS;
  }

  PRINTF("TSL256X: failed to power off\n");
  return TSL256X_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
tsl256x_clear_interrupt(void)
{
  uint8_t buf = (TSL256X_COMMAND + TSL256X_CLEAR_INTERRUPT);
  if(tsl256x_write_reg(&buf, 1) != I2C_MASTER_ERR_NONE) {
    PRINTF("TSL256X: failed to clear the interrupt\n");
    return TSL256X_ERROR;
  }
  return TSL256X_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int
tsl256x_read_sensor(uint16_t *lux)
{
  uint8_t buf[4];

  /* This is hardcoded to use word write/read operations */
  if(tsl256x_read_reg((TSL256X_COMMAND + TSL256X_D0LOW),
                      &buf[0], 2) == TSL256X_SUCCESS) {
    if(tsl256x_read_reg((TSL256X_COMMAND + TSL256X_D1LOW),
                        &buf[2], 2) == TSL256X_SUCCESS) {

      PRINTF("TSL256X: CH0 0x%02X%02X CH1 0x%02X%02X\n", buf[1], buf[0],
             buf[3], buf[2]);
      *lux = calculate_lux(buf);
      return TSL256X_SUCCESS;
    }
  }
  PRINTF("TSL256X: failed to read\n");
  return TSL256X_ERROR;
}
/*---------------------------------------------------------------------------*/
PROCESS(tsl256x_int_process, "TSL256X interrupt process handler");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tsl256x_int_process, ev, data)
{
  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    tsl256x_clear_interrupt();
    tsl256x_int_callback(0);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
tsl256x_interrupt_handler(uint8_t port, uint8_t pin)
{
  /* There's no alert/interruption flag to check, clear the interruption by
   * writting to the CLEAR bit in the COMMAND register
   */
  process_poll(&tsl256x_int_process);
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  uint8_t buf[3];

  if((type != TSL256X_ACTIVE) && (type != TSL256X_INT_OVER) &&
     (type != TSL256X_INT_BELOW) && (type != TSL256X_INT_DISABLE) &&
     (type != TSL256X_TIMMING_CFG)) {
    PRINTF("TSL256X: invalid start value\n");
    return TSL256X_ERROR;
  }

  /* As default the power-on values of the sensor are gain 1X, 402ms integration
   * time (not nominal), with manual control disabled
   */

  if(type == TSL256X_ACTIVE) {
    if(value) {
      i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN,
               I2C_SCL_NORMAL_BUS_SPEED);

      /*  Initialize interrupts handlers */
      tsl256x_int_callback = NULL;

      /* Power on the sensor and check for the part number */
      if(tsl256x_on() == TSL256X_SUCCESS) {
        if(tsl256x_id_register(&buf[0]) == TSL256X_SUCCESS) {
          if((buf[0] & TSL256X_ID_PARTNO_MASK) == TSL256X_EXPECTED_PARTNO) {

            /* Read the timming/gain configuration */
            if(tsl256x_read_reg((TSL256X_COMMAND + TSL256X_TIMMING),
                                &buf[0], 1) == TSL256X_SUCCESS) {
              gain = buf[0] & TSL256X_TIMMING_GAIN;
              timming = buf[0] & TSL256X_TIMMING_INTEG_MASK;
              PRINTF("TSL256X: enabled, timming %u gain %u\n", timming, gain);

              /* Restart the over interrupt threshold */
              buf[0] = (TSL256X_COMMAND + TSL256X_THRHIGHLOW);
              buf[1] = 0xFF;
              buf[2] = 0xFF;

              if(tsl256x_write_reg(buf, 3) != TSL256X_SUCCESS) {
                PRINTF("TSL256X: failed to clear over interrupt\n");
                return TSL256X_ERROR;
              }

              /* Restart the below interrupt threshold */
              buf[0] = (TSL256X_COMMAND + TSL256X_THRLOWLOW);
              buf[1] = 0x00;
              buf[2] = 0x00;

              if(tsl256x_write_reg(buf, 3) != TSL256X_SUCCESS) {
                PRINTF("TSL256X: failed to clear below interrupt\n");
                return TSL256X_ERROR;
              }

              /* Clear any pending interrupt */
              if(tsl256x_clear_interrupt() == TSL256X_SUCCESS) {
                enabled = 1;
                return TSL256X_SUCCESS;
              }
            }
          }
        }
      }
      return TSL256X_ERROR;
    } else {
      if(tsl256x_off() == TSL256X_SUCCESS) {
        PRINTF("TSL256X: stopped\n");
        enabled = 0;
        return TSL256X_SUCCESS;
      }
      return TSL256X_ERROR;
    }
  }

  if(!enabled) {
    PRINTF("TSL256X: sensor not started\n");
    return TSL256X_ERROR;
  }

  if(type == TSL256X_INT_DISABLE) {

    /* Ensure the GPIO doesn't generate more interrupts, this may affect others
     * I2C digital sensors using the bus and sharing this pin, so an user may
     * comment the line below
     */
    GPIO_DISABLE_INTERRUPT(TSL256X_INT_PORT_BASE, TSL256X_INT_PIN_MASK);

    /* This also wipes out the persistance value, to be reconfigured when
     * enabling back the interruption
     */
    buf[0] = (TSL256X_COMMAND + TSL256X_INTERRUPT);
    buf[1] = TSL256X_INTR_DISABLED;

    if(tsl256x_write_reg(buf, 2) != TSL256X_SUCCESS) {
      PRINTF("TSL256X: failed to disable the interrupt\n");
      return TSL256X_ERROR;
    }
    return TSL256X_SUCCESS;
  }

  /* Configure the timming and gain */
  if(type == TSL256X_TIMMING_CFG) {
    if((value != TSL256X_G16X_402MS) && (value != TSL256X_G1X_402MS) &&
       (value != TSL256X_G1X_101MS) && (value != TSL256X_G1X_13_7MS)) {
      PRINTF("TSL256X: invalid timming configuration values\n");
      return TSL256X_ERROR;
    }

    buf[0] = (TSL256X_COMMAND + TSL256X_TIMMING);
    buf[1] = value;

    if(tsl256x_write_reg(buf, 2) == TSL256X_SUCCESS) {
      if(value == TSL256X_G16X_402MS) {
        gain = 1;
      }

      switch(value) {
      case TSL256X_G16X_402MS:
      case TSL256X_G1X_402MS:
        timming = TSL256X_TIMMING_INTEG_402MS;
        break;
      case TSL256X_G1X_101MS:
        timming = TSL256X_TIMMING_INTEG_101MS;
        break;
      case TSL256X_G1X_13_7MS:
        timming = TSL256X_TIMMING_INTEG_13_7MS;
        break;
      }

      PRINTF("TSL256X: new timming %u gain %u\n", timming, gain);
      return TSL256X_SUCCESS;
    }
    PRINTF("TSL256X: failed to configure timming\n");
    return TSL256X_ERROR;
  }

  /* From here we handle the interrupt configuration, it requires the interrupt
   * callback handler to have been previously set using the TSL256X_REGISTER_INT
   * macro
   */

  buf[1] = ((uint8_t *)&value)[0];
  buf[2] = ((uint8_t *)&value)[1];

  if(type == TSL256X_INT_OVER) {
    buf[0] = (TSL256X_COMMAND + TSL256X_THRHIGHLOW);
  } else if(type == TSL256X_INT_BELOW) {
    buf[0] = (TSL256X_COMMAND + TSL256X_THRLOWLOW);
  }

  if(tsl256x_write_reg(buf, 3) != TSL256X_SUCCESS) {
    PRINTF("TSL256X: failed to set interrupt level\n");
    return TSL256X_ERROR;
  }

  /* Now configure the interruption register (level interrupt, 2 integration
   * cycles after threshold has been reached (roughly 804ms if timming is 402ms)
   */
  buf[0] = (TSL256X_COMMAND + TSL256X_INTERRUPT);
  buf[1] = (TSL256X_INTR_LEVEL << TSL256X_INTR_SHIFT);
  buf[1] += TSL256X_INT_PERSIST_2_CYCLES;

  if(tsl256x_write_reg(buf, 2) != TSL256X_SUCCESS) {
    PRINTF("TSL256X: failed to enable interrupt\n");
    return TSL256X_ERROR;
  }

  /* Configure the interrupts pins */
  GPIO_SOFTWARE_CONTROL(TSL256X_INT_PORT_BASE, TSL256X_INT_PIN_MASK);
  GPIO_SET_INPUT(TSL256X_INT_PORT_BASE, TSL256X_INT_PIN_MASK);

  /* Pull-up resistor, detect falling edge */
  GPIO_DETECT_EDGE(TSL256X_INT_PORT_BASE, TSL256X_INT_PIN_MASK);
  GPIO_TRIGGER_SINGLE_EDGE(TSL256X_INT_PORT_BASE, TSL256X_INT_PIN_MASK);
  GPIO_DETECT_FALLING(TSL256X_INT_PORT_BASE, TSL256X_INT_PIN_MASK);
  gpio_register_callback(tsl256x_interrupt_handler, I2C_INT_PORT, I2C_INT_PIN);

  /* Spin process until an interrupt is received */
  process_start(&tsl256x_int_process, NULL);

  /* Enable interrupts */
  GPIO_ENABLE_INTERRUPT(TSL256X_INT_PORT_BASE, TSL256X_INT_PIN_MASK);

  /* The RE-Mote revision A has this pin shared and with a pull-down resistor,
   * for other platforms (like the firefly), change to enable pull-up internal
   * resistor instead if no external pull-up is present.
   */
  ioc_set_over(I2C_INT_PORT, I2C_INT_PIN, IOC_OVERRIDE_PUE);
  NVIC_EnableIRQ(I2C_INT_VECTOR);

  PRINTF("TSL256X: Interrupt configured\n");
  return TSL256X_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return enabled;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  uint16_t lux;

  if(!enabled) {
    PRINTF("TSL256X: sensor not started\n");
    return TSL256X_ERROR;
  }

  if(type == TSL256X_VAL_READ) {
    if(tsl256x_read_sensor(&lux) != TSL256X_ERROR) {
      return lux;
    }
    PRINTF("TSL256X: fail to read\n");
  }
  return TSL256X_ERROR;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(tsl256x, TSL256X_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
