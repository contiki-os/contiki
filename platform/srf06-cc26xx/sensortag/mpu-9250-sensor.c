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
 * \addtogroup sensortag-cc26xx-mpu
 * @{
 *
 * \file
 *  Driver for the Sensortag-CC26XX Invensense MPU9250 motion processing unit
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "lib/sensors.h"
#include "mpu-9250-sensor.h"
#include "sys/rtimer.h"
#include "sensor-common.h"
#include "board-i2c.h"

#include "ti-lib.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
/* Sensor I2C address */
#define SENSOR_I2C_ADDRESS            0x68
#define SENSOR_MAG_I2_ADDRESS         0x0C
/*---------------------------------------------------------------------------*/
/* Registers */
#define SELF_TEST_X_GYRO              0x00 /* R/W */
#define SELF_TEST_Y_GYRO              0x01 /* R/W */
#define SELF_TEST_Z_GYRO              0x02 /* R/W */
#define SELF_TEST_X_ACCEL             0x0D /* R/W */
#define SELF_TEST_Z_ACCEL             0x0E /* R/W */
#define SELF_TEST_Y_ACCEL             0x0F /* R/W */
/*---------------------------------------------------------------------------*/
#define XG_OFFSET_H                   0x13 /* R/W */
#define XG_OFFSET_L                   0x14 /* R/W */
#define YG_OFFSET_H                   0x15 /* R/W */
#define YG_OFFSET_L                   0x16 /* R/W */
#define ZG_OFFSET_H                   0x17 /* R/W */
#define ZG_OFFSET_L                   0x18 /* R/W */
/*---------------------------------------------------------------------------*/
#define SMPLRT_DIV                    0x19 /* R/W */
#define CONFIG                        0x1A /* R/W */
#define GYRO_CONFIG                   0x1B /* R/W */
#define ACCEL_CONFIG                  0x1C /* R/W */
#define ACCEL_CONFIG_2                0x1D /* R/W */
#define LP_ACCEL_ODR                  0x1E /* R/W */
#define WOM_THR                       0x1F /* R/W */
#define FIFO_EN                       0x23 /* R/W */
/*---------------------------------------------------------------------------*/
/*
 * Registers 0x24 - 0x36 are not applicable to the SensorTag HW configuration
 * (IC2 Master)
 */
#define INT_PIN_CFG                   0x37 /* R/W */
#define INT_ENABLE                    0x38 /* R/W */
#define INT_STATUS                    0x3A /* R */
#define ACCEL_XOUT_H                  0x3B /* R */
#define ACCEL_XOUT_L                  0x3C /* R */
#define ACCEL_YOUT_H                  0x3D /* R */
#define ACCEL_YOUT_L                  0x3E /* R */
#define ACCEL_ZOUT_H                  0x3F /* R */
#define ACCEL_ZOUT_L                  0x40 /* R */
#define TEMP_OUT_H                    0x41 /* R */
#define TEMP_OUT_L                    0x42 /* R */
#define GYRO_XOUT_H                   0x43 /* R */
#define GYRO_XOUT_L                   0x44 /* R */
#define GYRO_YOUT_H                   0x45 /* R */
#define GYRO_YOUT_L                   0x46 /* R */
#define GYRO_ZOUT_H                   0x47 /* R */
#define GYRO_ZOUT_L                   0x48 /* R */
/*---------------------------------------------------------------------------*/
/*
 * Registers 0x49 - 0x60 are not applicable to the SensorTag HW configuration
 * (external sensor data)
 *
 * Registers 0x63 - 0x67 are not applicable to the SensorTag HW configuration
 * (I2C master)
 */
#define SIGNAL_PATH_RESET             0x68 /* R/W */
#define ACCEL_INTEL_CTRL              0x69 /* R/W */
#define USER_CTRL                     0x6A /* R/W */
#define PWR_MGMT_1                    0x6B /* R/W */
#define PWR_MGMT_2                    0x6C /* R/W */
#define FIFO_COUNT_H                  0x72 /* R/W */
#define FIFO_COUNT_L                  0x73 /* R/W */
#define FIFO_R_W                      0x74 /* R/W */
#define WHO_AM_I                      0x75 /* R/W */
/*---------------------------------------------------------------------------*/
/* Masks is mpuConfig valiable */
#define ACC_CONFIG_MASK               0x38
#define GYRO_CONFIG_MASK              0x07
/*---------------------------------------------------------------------------*/
/* Values PWR_MGMT_1 */
#define MPU_SLEEP                     0x4F  /* Sleep + stop all clocks */
#define MPU_WAKE_UP                   0x09  /* Disable temp. + intern osc */
/*---------------------------------------------------------------------------*/
/* Values PWR_MGMT_2 */
#define ALL_AXES                      0x3F
#define GYRO_AXES                     0x07
#define ACC_AXES                      0x38
/*---------------------------------------------------------------------------*/
/* Data sizes */
#define DATA_SIZE                     6
/*---------------------------------------------------------------------------*/
/* Output data rates */
#define INV_LPA_0_3125HZ              0
#define INV_LPA_0_625HZ               1
#define INV_LPA_1_25HZ                2
#define INV_LPA_2_5HZ                 3
#define INV_LPA_5HZ                   4
#define INV_LPA_10HZ                  5
#define INV_LPA_20HZ                  6
#define INV_LPA_40HZ                  7
#define INV_LPA_80HZ                  8
#define INV_LPA_160HZ                 9
#define INV_LPA_320HZ                 10
#define INV_LPA_640HZ                 11
#define INV_LPA_STOPPED               255
/*---------------------------------------------------------------------------*/
/* Bit values */
#define BIT_ANY_RD_CLR                0x10
#define BIT_RAW_RDY_EN                0x01
#define BIT_WOM_EN                    0x40
#define BIT_LPA_CYCLE                 0x20
#define BIT_STBY_XA                   0x20
#define BIT_STBY_YA                   0x10
#define BIT_STBY_ZA                   0x08
#define BIT_STBY_XG                   0x04
#define BIT_STBY_YG                   0x02
#define BIT_STBY_ZG                   0x01
#define BIT_STBY_XYZA                 (BIT_STBY_XA | BIT_STBY_YA | BIT_STBY_ZA)
#define BIT_STBY_XYZG                 (BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG)
/*---------------------------------------------------------------------------*/
/* User control register */
#define BIT_ACTL                      0x80
#define BIT_LATCH_EN                  0x20
/*---------------------------------------------------------------------------*/
/* INT Pin / Bypass Enable Configuration */
#define BIT_AUX_IF_EN                 0x20 /* I2C_MST_EN */
#define BIT_BYPASS_EN                 0x02
/*---------------------------------------------------------------------------*/
#define ACC_RANGE_INVALID -1

#define ACC_RANGE_2G      0
#define ACC_RANGE_4G      1
#define ACC_RANGE_8G      2
#define ACC_RANGE_16G     3

#define MPU_AX_GYR_X      2
#define MPU_AX_GYR_Y      1
#define MPU_AX_GYR_Z      0
#define MPU_AX_GYR        0x07

#define MPU_AX_ACC_X      5
#define MPU_AX_ACC_Y      4
#define MPU_AX_ACC_Z      3
#define MPU_AX_ACC        0x38

#define MPU_AX_MAG        6
/*---------------------------------------------------------------------------*/
#define MPU_DATA_READY    0x01
#define MPU_MOVEMENT      0x40
/*---------------------------------------------------------------------------*/
/* Sensor selection/deselection */
#define SENSOR_SELECT()     board_i2c_select(BOARD_I2C_INTERFACE_1, SENSOR_I2C_ADDRESS)
#define SENSOR_DESELECT()   board_i2c_deselect()
/*---------------------------------------------------------------------------*/
/* Delay */
#define delay_ms(i) (ti_lib_cpu_delay(8000 * (i)))
/*---------------------------------------------------------------------------*/
static uint8_t mpu_config;
static uint8_t acc_range;
static uint8_t acc_range_reg;
static uint8_t val;
static uint8_t interrupt_status;
/*---------------------------------------------------------------------------*/
#define SENSOR_STATE_DISABLED     0
#define SENSOR_STATE_BOOTING      1
#define SENSOR_STATE_ENABLED      2

static int state = SENSOR_STATE_DISABLED;
static int elements = MPU_9250_SENSOR_TYPE_NONE;
/*---------------------------------------------------------------------------*/
/* 3 16-byte words for all sensor readings */
#define SENSOR_DATA_BUF_SIZE   3

static uint16_t sensor_value[SENSOR_DATA_BUF_SIZE];
/*---------------------------------------------------------------------------*/
/*
 * Wait SENSOR_BOOT_DELAY ticks for the sensor to boot and
 * SENSOR_STARTUP_DELAY for readings to be ready
 * Gyro is a little slower than Acc
 */
#define SENSOR_BOOT_DELAY     8
#define SENSOR_STARTUP_DELAY  5

static struct ctimer startup_timer;
/*---------------------------------------------------------------------------*/
/* Wait for the MPU to have data ready */
rtimer_clock_t t0;

/*
 * Wait timeout in rtimer ticks. This is just a random low number, since the
 * first time we read the sensor status, it should be ready to return data
 */
#define READING_WAIT_TIMEOUT 10
/*---------------------------------------------------------------------------*/
/**
 * \brief Place the MPU in low power mode
 */
static void
sensor_sleep(void)
{
  SENSOR_SELECT();

  val = ALL_AXES;
  sensor_common_write_reg(PWR_MGMT_2, &val, 1);

  val = MPU_SLEEP;
  sensor_common_write_reg(PWR_MGMT_1, &val, 1);
  SENSOR_DESELECT();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Exit low power mode
 */
static void
sensor_wakeup(void)
{
  SENSOR_SELECT();
  val = MPU_WAKE_UP;
  sensor_common_write_reg(PWR_MGMT_1, &val, 1);

  /* All axis initially disabled */
  val = ALL_AXES;
  sensor_common_write_reg(PWR_MGMT_2, &val, 1);
  mpu_config = 0;

  /* Restore the range */
  sensor_common_write_reg(ACCEL_CONFIG, &acc_range_reg, 1);

  /* Clear interrupts */
  sensor_common_read_reg(INT_STATUS, &val, 1);
  SENSOR_DESELECT();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Select gyro and accelerometer axes
 */
static void
select_axes(void)
{
  val = ~mpu_config;
  SENSOR_SELECT();
  sensor_common_write_reg(PWR_MGMT_2, &val, 1);
  SENSOR_DESELECT();
}
/*---------------------------------------------------------------------------*/
static void
convert_to_le(uint8_t *data, uint8_t len)
{
  int i;
  for(i = 0; i < len; i += 2) {
    uint8_t tmp;
    tmp = data[i];
    data[i] = data[i + 1];
    data[i + 1] = tmp;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set the range of the accelerometer
 * \param new_range: ACC_RANGE_2G, ACC_RANGE_4G, ACC_RANGE_8G, ACC_RANGE_16G
 * \return true if the write to the sensor succeeded
 */
static bool
acc_set_range(uint8_t new_range)
{
  bool success;

  if(new_range == acc_range) {
    return true;
  }

  success = false;

  acc_range_reg = (new_range << 3);

  /* Apply the range */
  SENSOR_SELECT();
  success = sensor_common_write_reg(ACCEL_CONFIG, &acc_range_reg, 1);
  SENSOR_DESELECT();

  if(success) {
    acc_range = new_range;
  }

  return success;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Check whether a data or wake on motion interrupt has occurred
 * \return Return the interrupt status
 *
 * This driver does not use interrupts, however this function allows us to
 * determine whether a new sensor reading is available
 */
static uint8_t
int_status(void)
{
  SENSOR_SELECT();
  sensor_common_read_reg(INT_STATUS, &interrupt_status, 1);
  SENSOR_DESELECT();

  return interrupt_status;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Enable the MPU
 * \param axes: Gyro bitmap [0..2], X = 1, Y = 2, Z = 4. 0 = gyro off
 *              Acc  bitmap [3..5], X = 8, Y = 16, Z = 32. 0 = accelerometer off
 */
static void
enable_sensor(uint16_t axes)
{
  if(mpu_config == 0 && axes != 0) {
    /* Wake up the sensor if it was off */
    sensor_wakeup();
  }

  mpu_config = axes;

  if(mpu_config != 0) {
    /* Enable gyro + accelerometer readout */
    select_axes();
    delay_ms(10);
  } else if(mpu_config == 0) {
    sensor_sleep();
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Read data from the accelerometer - X, Y, Z - 3 words
 * \return True if a valid reading could be taken, false otherwise
 */
static bool
acc_read(uint16_t *data)
{
  bool success;

  if(interrupt_status & BIT_RAW_RDY_EN) {
    /* Burst read of all accelerometer values */
    SENSOR_SELECT();
    success = sensor_common_read_reg(ACCEL_XOUT_H, (uint8_t *)data, DATA_SIZE);
    SENSOR_DESELECT();

    if(success) {
      convert_to_le((uint8_t *)data, DATA_SIZE);
    } else {
      sensor_common_set_error_data((uint8_t *)data, DATA_SIZE);
    }
  } else {
    /* Data not ready */
    success = false;
  }

  return success;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Read data from the gyroscope - X, Y, Z - 3 words
 * \return True if a valid reading could be taken, false otherwise
 */
static bool
gyro_read(uint16_t *data)
{
  bool success;

  if(interrupt_status & BIT_RAW_RDY_EN) {
    /* Select this sensor */
    SENSOR_SELECT();

    /* Burst read of all gyroscope values */
    success = sensor_common_read_reg(GYRO_XOUT_H, (uint8_t *)data, DATA_SIZE);

    if(success) {
      convert_to_le((uint8_t *)data, DATA_SIZE);
    } else {
      sensor_common_set_error_data((uint8_t *)data, DATA_SIZE);
    }

    SENSOR_DESELECT();
  } else {
    success = false;
  }

  return success;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Convert accelerometer raw reading to a value in G
 * \param raw_data The raw accelerometer reading
 * \return The converted value
 */
static float
acc_convert(int16_t raw_data)
{
  float v = 0;

  switch(acc_range) {
  case ACC_RANGE_2G:
    /* Calculate acceleration, unit G, range -2, +2 */
    v = (raw_data * 1.0) / (32768 / 2);
    break;
  case ACC_RANGE_4G:
    /* Calculate acceleration, unit G, range -4, +4 */
    v = (raw_data * 1.0) / (32768 / 4);
    break;
  case ACC_RANGE_8G:
    /* Calculate acceleration, unit G, range -8, +8 */
    v = (raw_data * 1.0) / (32768 / 8);
    break;
  case ACC_RANGE_16G:
    /* Calculate acceleration, unit G, range -16, +16 */
    v = (raw_data * 1.0) / (32768 / 16);
    break;
  default:
    v = 0;
    break;
  }

  return v;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Convert gyro raw reading to a value in deg/sec
 * \param raw_data The raw accelerometer reading
 * \return The converted value
 */
static float
gyro_convert(int16_t raw_data)
{
  /* calculate rotation, unit deg/s, range -250, +250 */
  return (raw_data * 1.0) / (65536 / 500);
}
/*---------------------------------------------------------------------------*/
static void
notify_ready(void *not_used)
{
  state = SENSOR_STATE_ENABLED;
  sensors_changed(&mpu_9250_sensor);
}
/*---------------------------------------------------------------------------*/
static void
initialise(void *not_used)
{
  /* Configure the accelerometer range */
  if((elements & MPU_9250_SENSOR_TYPE_ACC) != 0) {
    acc_set_range(MPU_9250_SENSOR_ACC_RANGE);
  }

  enable_sensor(elements & MPU_9250_SENSOR_TYPE_ALL);

  ctimer_set(&startup_timer, SENSOR_STARTUP_DELAY, notify_ready, NULL);
}
/*---------------------------------------------------------------------------*/
static void
power_up(void)
{
  ti_lib_gpio_set_dio(BOARD_IOID_MPU_POWER);
  state = SENSOR_STATE_BOOTING;

  ctimer_set(&startup_timer, SENSOR_BOOT_DELAY, initialise, NULL);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns a reading from the sensor
 * \param type MPU_9250_SENSOR_TYPE_ACC_[XYZ] or MPU_9250_SENSOR_TYPE_GYRO_[XYZ]
 * \return centi-G (ACC) or centi-Deg/Sec (Gyro)
 */
static int
value(int type)
{
  int rv;
  float converted_val = 0;

  if(state == SENSOR_STATE_DISABLED) {
    PRINTF("MPU: Sensor Disabled\n");
    return CC26XX_SENSOR_READING_ERROR;
  }

  memset(sensor_value, 0, sizeof(sensor_value));

  if((type & MPU_9250_SENSOR_TYPE_ACC) != 0) {
    t0 = RTIMER_NOW();

    while(!int_status() &&
          (RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + READING_WAIT_TIMEOUT)));

    rv = acc_read(sensor_value);

    if(rv == 0) {
      return CC26XX_SENSOR_READING_ERROR;
    }

    PRINTF("MPU: ACC = 0x%04x 0x%04x 0x%04x = ",
           sensor_value[0], sensor_value[1], sensor_value[2]);

    /* Convert */
    if(type == MPU_9250_SENSOR_TYPE_ACC_X) {
      converted_val = acc_convert(sensor_value[0]);
    } else if(type == MPU_9250_SENSOR_TYPE_ACC_Y) {
      converted_val = acc_convert(sensor_value[1]);
    } else if(type == MPU_9250_SENSOR_TYPE_ACC_Z) {
      converted_val = acc_convert(sensor_value[2]);
    }
    rv = (int)(converted_val * 100);
  } else if((type & MPU_9250_SENSOR_TYPE_GYRO) != 0) {
    t0 = RTIMER_NOW();

    while(!int_status() &&
          (RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + READING_WAIT_TIMEOUT)));

    rv = gyro_read(sensor_value);

    if(rv == 0) {
      return CC26XX_SENSOR_READING_ERROR;
    }

    PRINTF("MPU: Gyro = 0x%04x 0x%04x 0x%04x = ",
           sensor_value[0], sensor_value[1], sensor_value[2]);

    if(type == MPU_9250_SENSOR_TYPE_GYRO_X) {
      converted_val = gyro_convert(sensor_value[0]);
    } else if(type == MPU_9250_SENSOR_TYPE_GYRO_Y) {
      converted_val = gyro_convert(sensor_value[1]);
    } else if(type == MPU_9250_SENSOR_TYPE_GYRO_Z) {
      converted_val = gyro_convert(sensor_value[2]);
    }
    rv = (int)(converted_val * 100);
  } else {
    PRINTF("MPU: Invalid type\n");
    rv = CC26XX_SENSOR_READING_ERROR;
  }

  PRINTF("%ld\n", (long int)(converted_val * 100));

  return rv;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the MPU9250 sensor.
 *
 * \param type Activate, enable or disable the sensor. See below
 * \param enable
 *
 * When type == SENSORS_HW_INIT we turn on the hardware
 * When type == SENSORS_ACTIVE and enable==1 we enable the sensor
 * When type == SENSORS_ACTIVE and enable==0 we disable the sensor
 */
static int
configure(int type, int enable)
{
  switch(type) {
  case SENSORS_HW_INIT:
    ti_lib_rom_ioc_pin_type_gpio_input(BOARD_IOID_MPU_INT);
    ti_lib_ioc_io_port_pull_set(BOARD_IOID_MPU_INT, IOC_IOPULL_DOWN);
    ti_lib_ioc_io_hyst_set(BOARD_IOID_MPU_INT, IOC_HYST_ENABLE);

    ti_lib_rom_ioc_pin_type_gpio_output(BOARD_IOID_MPU_POWER);
    ti_lib_ioc_io_drv_strength_set(BOARD_IOID_MPU_POWER, IOC_CURRENT_4MA,
                                   IOC_STRENGTH_MAX);
    ti_lib_gpio_clear_dio(BOARD_IOID_MPU_POWER);
    elements = MPU_9250_SENSOR_TYPE_NONE;
    break;
  case SENSORS_ACTIVE:
    if(((enable & MPU_9250_SENSOR_TYPE_ACC) != 0) ||
       ((enable & MPU_9250_SENSOR_TYPE_GYRO) != 0)) {
      PRINTF("MPU: Enabling\n");
      elements = enable & MPU_9250_SENSOR_TYPE_ALL;

      power_up();

      state = SENSOR_STATE_BOOTING;
    } else {
      PRINTF("MPU: Disabling\n");
      if(HWREG(GPIO_BASE + GPIO_O_DOUT31_0) & BOARD_MPU_POWER) {
        /* Then check our state */
        elements = MPU_9250_SENSOR_TYPE_NONE;
        ctimer_stop(&startup_timer);
        sensor_sleep();
        while(ti_lib_i2c_master_busy(I2C0_BASE));
        state = SENSOR_STATE_DISABLED;
        ti_lib_gpio_clear_dio(BOARD_IOID_MPU_POWER);
      }
    }
    break;
  default:
    break;
  }
  return state;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns the status of the sensor
 * \param type SENSORS_ACTIVE or SENSORS_READY
 * \return 1 if the sensor is enabled
 */
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return state;
    break;
  default:
    break;
  }
  return SENSOR_STATE_DISABLED;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(mpu_9250_sensor, "MPU9250", value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
