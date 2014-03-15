/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Implementation of LIS3DH SPI driver used in the Mulle platform.
 *
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include "lis3dh.h"
#include <stdint.h>
#include <stdio.h>

/**
 * Set bits of an 8-bit register on the LIS3DH.
 */
void
lis3dh_set_bits(const lis3dh_reg_addr_t addr, const uint8_t mask)
{
  uint8_t reg;

  reg = lis3dh_read_byte(addr);
  reg |= mask;
  lis3dh_write_byte(addr, reg);
}


/**
 * Clear bits of an 8-bit register on the LIS3DH.
 */
void
lis3dh_clear_bits(const lis3dh_reg_addr_t addr, const uint8_t mask)
{
  uint8_t reg;

  reg = lis3dh_read_byte(addr);
  reg &= ~mask;
  lis3dh_write_byte(addr, reg);
}

/**
 * Write (both set and clear) bits of an 8-bit register on the LIS3DH.
 *
 * \param addr Register address on the LIS3DH.
 * \param mask Bitmask for the bits to modify.
 * \param values The values to write to the masked bits.
 */
void
lis3dh_write_bits(const lis3dh_reg_addr_t addr, const uint8_t mask,
                  const uint8_t values)
{
  uint8_t reg;

  reg = lis3dh_read_byte(addr);
  reg &= ~mask;
  reg |= (values & mask);
  lis3dh_write_byte(addr, reg);
}

/**
 * Get one X-axis reading from the accelerometer.
 *
 * \return The oldest X axis acceleration measurement available.
 */
int16_t
lis3dh_read_xaxis()
{
  return lis3dh_read_int16(OUT_X_L);
}

/**
 * Get one Y-axis reading from the accelerometer.
 *
 * \return The oldest Y axis acceleration measurement available.
 */
int16_t
lis3dh_read_yaxis()
{
  return lis3dh_read_int16(OUT_Y_L);
}

/**
 * Get one Z-axis reading from the accelerometer.
 *
 * \return The oldest Z axis acceleration measurement available.
 */
int16_t
lis3dh_read_zaxis()
{
  return lis3dh_read_int16(OUT_Z_L);
}

/**
 * Read all three axes in a single transaction.
 *
 * \param buffer Pointer to an int16_t[3] buffer.
 */
void
lis3dh_read_xyz(int16_t * buffer)
{
  lis3dh_memcpy_from_device(OUT_X_L, (uint8_t *) buffer,
                            LIS3DH_ACC_DATA_SIZE * 3);
}

/**
 * Get one reading from the first channel of the auxiliary ADC.
 *
 * \return The current ADC reading.
 */
int16_t
lis3dh_read_aux_adc1()
{
  return lis3dh_read_int16(OUT_AUX_ADC1_L);
}

/**
 * Get one reading from the second channel of the auxiliary ADC.
 *
 * \return The current ADC reading.
 */
int16_t
lis3dh_read_aux_adc2()
{
  return lis3dh_read_int16(OUT_AUX_ADC2_L);
}

/**
 * Get one reading from the third channel of the auxiliary ADC.
 *
 * \note The internal temperature sensor is connected to the third channel on
 *       the auxiliary ADC when the TEMP_EN bit of TEMP_CFG_REG is set.
 *
 * \return The current ADC reading.
 */
int16_t
lis3dh_read_aux_adc3()
{
  return lis3dh_read_int16(OUT_AUX_ADC3_L);
}

/**
 * Turn on/off power to the auxiliary ADC in LIS3DH.
 *
 * \param enable Power state of the auxiliary ADC
 * \param temperature If not zero, switch the ADC mux so that a temperature
 *        reading is available on OUT_3_L, OUT_3_H.
 *
 * \note This ADC is only used for the temperature reading and the external ADC
 *       pins. The accelerometer ADC is turned on by lis3dh_set_odr().
 */
void
lis3dh_set_aux_adc(const uint8_t enable, const uint8_t temperature)
{
  lis3dh_write_bits(TEMP_CFG_REG, LIS3DH_TEMP_CFG_REG_ADC_PD_MASK,
                    (enable ? LIS3DH_TEMP_CFG_REG_ADC_PD_MASK : 0) |
                    (temperature ? LIS3DH_TEMP_CFG_REG_TEMP_EN_MASK : 0));
}

/**
 * Enable/disable accelerometer axes.
 *
 * \param axes An OR-ed combination of LIS3DH_AXES_X, LIS3DH_AXES_Y, LIS3DH_AXES_Z.
 *
 * \note The macro LIS3DH_AXES_XYZ is a convenience shortcut to enable all axes.
 */
void
lis3dh_set_axes(const uint8_t axes)
{
  lis3dh_write_bits(CTRL_REG1, LIS3DH_CTRL_REG1_XYZEN_MASK, axes);
}

/**
 * Set the FIFO mode.
 *
 * \param mode The chosen FIFO mode.
 */
void
lis3dh_set_fifo_mode(const lis3dh_fifo_mode_t mode)
{
  lis3dh_write_bits(FIFO_CTRL_REG, LIS3DH_FIFO_CTRL_REG_FM_MASK,
                    (mode << LIS3DH_FIFO_CTRL_REG_FM_SHIFT));
}

/**
 * Enable/disable the FIFO.
 *
 * \param enable If zero, disable the FIFO, otherwise enables the FIFO.
 */
void
lis3dh_set_fifo(const uint8_t enable)
{
  lis3dh_write_bits(CTRL_REG5, LIS3DH_CTRL_REG5_FIFO_EN_MASK,
                    (enable ? LIS3DH_CTRL_REG5_FIFO_EN_MASK : 0));
}

/**
 * Set the output data rate of the sensor.
 *
 * \param odr Chosen output data rate.
 */
void
lis3dh_set_odr(const lis3dh_odr_t odr)
{
  lis3dh_write_bits(CTRL_REG1, LIS3DH_CTRL_REG1_ODR_MASK,
                    (odr << LIS3DH_CTRL_REG1_ODR_SHIFT));
}

/**
 * Set the full scale range of the sensor.
 *
 * Valid values for scale is 2, 4, 8, 16 and represents the full range of the sensor.
 *
 * \param scale The chosen sensitivity scale.
 */
void
lis3dh_set_scale(const lis3dh_scale_t scale)
{
  switch (scale) {
  case SCALE_2G:
    lis3dh_write_bits(CTRL_REG4, LIS3DH_CTRL_REG4_FS_MASK,
                      LIS3DH_CTRL_REG4_FS_2G);
    break;
  case SCALE_4G:
    lis3dh_write_bits(CTRL_REG4, LIS3DH_CTRL_REG4_FS_MASK,
                      LIS3DH_CTRL_REG4_FS_4G);
    break;
  case SCALE_8G:
    lis3dh_write_bits(CTRL_REG4, LIS3DH_CTRL_REG4_FS_MASK,
                      LIS3DH_CTRL_REG4_FS_8G);
    break;
  case SCALE_16G:
    lis3dh_write_bits(CTRL_REG4, LIS3DH_CTRL_REG4_FS_MASK,
                      LIS3DH_CTRL_REG4_FS_16G);
    break;
  default:
    /* Invalid scale value */
    return;
  }
}

/**
 * Initialize a LIS3DH accelerometer.
 *
 * \todo Signal errors when initializing the LIS3DH hardware.
 */
void
lis3dh_init()
{
  uint8_t databyte;

  lis3dh_arch_init();

  databyte = lis3dh_read_byte(WHO_AM_I);
  if(databyte != LIS3DH_WHO_AM_I_RESPONSE) {
    /* Chip is not responding correctly */
    return;
  }

  /* Set block data update and little endian mode. */
  lis3dh_write_byte(CTRL_REG4,
                    (LIS3DH_CTRL_REG4_BDU_ENABLE |
                     LIS3DH_CTRL_REG4_BLE_LITTLE_ENDIAN));
}
