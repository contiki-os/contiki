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
 *         Interface of LIS3DH SPI driver used in the Mulle platform.
 *
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#ifndef LIS3DH_H_
#define LIS3DH_H_

#include <stdint.h>

/**
 * The WHO_AM_I register should contain this value in order to correctly
 * identify the chip.
 */
#define LIS3DH_WHO_AM_I_RESPONSE (0b00110011)

/**
 * All LIS3DH hardware registers are enumerated here.
 */
typedef enum {
  STATUS_AUX = 0x07,
  OUT_AUX_ADC1_L = 0x08,
  OUT_AUX_ADC1_H = 0x09,
  OUT_AUX_ADC2_L = 0x0A,
  OUT_AUX_ADC2_H = 0x0B,
  OUT_AUX_ADC3_L = 0x0C,
  OUT_AUX_ADC3_H = 0x0D,
  INT_COUNTER_REG = 0x0E,
  WHO_AM_I = 0x0F,
  TEMP_CFG_REG = 0x1F,
  CTRL_REG1 = 0x20,
  CTRL_REG2 = 0x21,
  CTRL_REG3 = 0x22,
  CTRL_REG4 = 0x23,
  CTRL_REG5 = 0x24,
  CTRL_REG6 = 0x25,
  REFERENCE = 0x26,
  STATUS_REG = 0x27,
  OUT_X_L = 0x28,
  OUT_X_H = 0x29,
  OUT_Y_L = 0x2A,
  OUT_Y_H = 0x2B,
  OUT_Z_L = 0x2C,
  OUT_Z_H = 0x2D,
  FIFO_CTRL_REG = 0x2E,
  FIFO_SRC_REG = 0x2F,
  INT1_CFG = 0x30,
  INT1_SOURCE = 0x31,
  INT1_THS = 0x32,
  INT1_DURATION = 0x33,
  CLICK_CFG = 0x38,
  CLICK_SRC = 0x39,
  CLICK_THS = 0x3A,
  TIME_LIMIT = 0x3B,
  TIME_LATENCY = 0x3C,
  TIME_WINDOW = 0x3D,
} lis3dh_reg_addr_t;


/*
 * Bit offsets within the individual registers
 * source: LIS3DH datasheet
 */

/**
 * ADC enable. Default value: 0
 * (0: ADC disabled; 1: ADC enabled)
 */
#define LIS3DH_TEMP_CFG_REG_ADC_PD_MASK (1 << 7)
/**
 * Temperature sensor (T) enable. Default value: 0
 * (0: T disabled; 1: T enabled)
 */
#define LIS3DH_TEMP_CFG_REG_TEMP_EN_MASK (1 << 6)

/**
 * Data rate selection. Default value: 00
 * (0000: Power down; Others: Refer to Table 25, “Data rate configuration”)
 */
#define LIS3DH_CTRL_REG1_ODR_SHIFT (4)
#define LIS3DH_CTRL_REG1_ODR3_MASK (1 << (LIS3DH_CTRL_REG1_ODR_SHIFT+3))
#define LIS3DH_CTRL_REG1_ODR2_MASK (1 << (LIS3DH_CTRL_REG1_ODR_SHIFT+2))
#define LIS3DH_CTRL_REG1_ODR1_MASK (1 << (LIS3DH_CTRL_REG1_ODR_SHIFT+1))
#define LIS3DH_CTRL_REG1_ODR0_MASK (1 << LIS3DH_CTRL_REG1_ODR_SHIFT)

#define LIS3DH_CTRL_REG1_ODR_MASK  (LIS3DH_CTRL_REG1_ODR3_MASK | \
		LIS3DH_CTRL_REG1_ODR2_MASK | \
		LIS3DH_CTRL_REG1_ODR1_MASK | \
		LIS3DH_CTRL_REG1_ODR0_MASK)

/*
 * Convenience macros for power modes
 */
#define LIS3DH_CTRL_REG1_ODR_POWERDOWN (0)
#define LIS3DH_CTRL_REG1_ODR_1HZ      (LIS3DH_CTRL_REG1_ODR0_MASK)
#define LIS3DH_CTRL_REG1_ODR_10HZ     (LIS3DH_CTRL_REG1_ODR1_MASK)
#define LIS3DH_CTRL_REG1_ODR_25HZ     (LIS3DH_CTRL_REG1_ODR1_MASK | LIS3DH_CTRL_REG1_ODR0_MASK)
#define LIS3DH_CTRL_REG1_ODR_50HZ     (LIS3DH_CTRL_REG1_ODR2_MASK)
#define LIS3DH_CTRL_REG1_ODR_100HZ    (LIS3DH_CTRL_REG1_ODR2_MASK | LIS3DH_CTRL_REG1_ODR0_MASK)
#define LIS3DH_CTRL_REG1_ODR_200HZ    (LIS3DH_CTRL_REG1_ODR2_MASK | LIS3DH_CTRL_REG1_ODR1_MASK)
#define LIS3DH_CTRL_REG1_ODR_400HZ    (LIS3DH_CTRL_REG1_ODR2_MASK | LIS3DH_CTRL_REG1_ODR1_MASK | LIS3DH_CTRL_REG1_ODR0_MASK)
#define LIS3DH_CTRL_REG1_ODR_400HZ    (LIS3DH_CTRL_REG1_ODR2_MASK | LIS3DH_CTRL_REG1_ODR1_MASK | LIS3DH_CTRL_REG1_ODR0_MASK)
#define LIS3DH_CTRL_REG1_ODR_LP1600HZ (LIS3DH_CTRL_REG1_ODR3_MASK)
#define LIS3DH_CTRL_REG1_ODR_LP5000HZ (LIS3DH_CTRL_REG1_ODR3_MASK | LIS3DH_CTRL_REG1_ODR0_MASK)
#define LIS3DH_CTRL_REG1_ODR_NP1250HZ (LIS3DH_CTRL_REG1_ODR3_MASK | LIS3DH_CTRL_REG1_ODR0_MASK)

/**
 * Allowed values for the Output Data Rate of the sensor.
 * Use these when calling lis3dh_set_odr(odr).
 */
typedef enum {
  ODR_POWERDOWN = 0x00,
  ODR_1Hz = 0x01,
  ODR_10Hz = 0x02,
  ODR_25Hz = 0x03,
  ODR_50Hz = 0x04,
  ODR_100Hz = 0x05,
  ODR_200Hz = 0x06,
  ODR_400Hz = 0x07,
  ODR_LP1600Hz = 0x08,
  ODR_NP1250Hz_LP5000HZ = 0x09
} lis3dh_odr_t;

/**
 * Low power mode enable. Default value: 0
 * (0: normal mode, 1: low power mode)
 */
#define LIS3DH_CTRL_REG1_LPEN_MASK (1 << 3)

#define LIS3DH_CTRL_REG1_ZEN_SHIFT (2)
/**
 * Z axis enable. Default value: 1
 * (0: Z axis disabled; 1: Z axis enabled)
 */
#define LIS3DH_CTRL_REG1_ZEN_MASK (1 << LIS3DH_CTRL_REG1_ZEN_SHIFT)

#define LIS3DH_CTRL_REG1_YEN_SHIFT (1)
/**
 * Y axis enable. Default value: 1
 * (0: Y axis disabled; 1: Y axis enabled)
 */
#define LIS3DH_CTRL_REG1_YEN_MASK (1 << LIS3DH_CTRL_REG1_YEN_SHIFT)

#define LIS3DH_CTRL_REG1_XEN_SHIFT (0)
/**
 * X axis enable. Default value: 1
 * (0: X axis disabled; 1: X axis enabled)
 */
#define LIS3DH_CTRL_REG1_XEN_MASK (1 << LIS3DH_CTRL_REG1_XEN_SHIFT)

#define LIS3DH_CTRL_REG1_XYZEN_SHIFT (0)
#define LIS3DH_CTRL_REG1_XYZEN_MASK (LIS3DH_CTRL_REG1_XEN_MASK | \
		LIS3DH_CTRL_REG1_YEN_MASK | LIS3DH_CTRL_REG1_ZEN_MASK)

#define LIS3DH_AXES_X (LIS3DH_CTRL_REG1_XEN_MASK)
#define LIS3DH_AXES_Y (LIS3DH_CTRL_REG1_YEN_MASK)
#define LIS3DH_AXES_Z (LIS3DH_CTRL_REG1_ZEN_MASK)
/**
 * Convenience macro for enabling all axes.
 */
#define LIS3DH_AXES_XYZ (LIS3DH_CTRL_REG1_XYZEN_MASK)

/**
 * High pass filter mode selection. Default value: 00
 * Refer to Table 29, "High pass filter mode configuration"
 */
#define LIS3DH_CTRL_REG2_HPM1_MASK (1 << 7)
#define LIS3DH_CTRL_REG2_HPM0_MASK (1 << 6)

/**
 * High pass filter cut off frequency selection
 */
#define LIS3DH_CTRL_REG2_HPCF2_MASK (1 << 5)
#define LIS3DH_CTRL_REG2_HPCF1_MASK (1 << 4)

/**
 * Filtered data selection. Default value: 0
 * (0: internal filter bypassed; 1: data from internal filter sent to output register and FIFO)
 */
#define LIS3DH_CTRL_REG2_FDS_MASK  (1 << 3)

/**
 * High pass filter enabled for CLICK function.
 * (0: filter bypassed; 1: filter enabled)
 */
#define LIS3DH_CTRL_REG2_HPCLICK_MASK  (1 << 2)

/**
 * High pass filter enabled for AOI function on interrupt 2,
 * (0: filter bypassed; 1: filter enabled)
 */
#define LIS3DH_CTRL_REG2_HPIS2_MASK  (1 << 1)
#define LIS3DH_CTRL_REG2_HPIS1_MASK  (1 << 0)

/**
 * CLICK interrupt on INT1. Default value 0.
 * (0: Disable; 1: Enable)
 */
#define LIS3DH_CTRL_REG3_I1_CLICK_MASK  (1 << 7)

/**
 * AOI1 interrupt on INT1. Default value 0.
 * (0: Disable; 1: Enable)
 */
#define LIS3DH_CTRL_REG3_I1_AOI1_MASK    (1 << 6)
/**
 * AOI2 interrupt on INT1. Default value 0.
 * (0: Disable; 1: Enable)
 */
#define LIS3DH_CTRL_REG3_I1_AOI2_MASK    (1 << 5)
/**
 * DRDY1 interrupt on INT1. Default value 0.
 * (0: Disable; 1: Enable)
 */
#define LIS3DH_CTRL_REG3_I1_DRDY1_MASK   (1 << 4)
/**
 * DRDY2 interrupt on INT1. Default value 0.
 * (0: Disable; 1: Enable)
 */
#define LIS3DH_CTRL_REG3_I1_DRDY2_MASK   (1 << 3)
/**
 * FIFO Watermark interrupt on INT1. Default value 0.
 * (0: Disable; 1: Enable)
 */
#define LIS3DH_CTRL_REG3_I1_WTM_MASK     (1 << 2)
/**
 * FIFO Overrun interrupt on INT1. Default value 0.
 * (0: Disable; 1: Enable)
 */
#define LIS3DH_CTRL_REG3_I1_OVERRUN_MASK (1 << 1)

/**
 * Block data update. Default value: 0
 * (0: continuous update; 1: output registers not updated until MSB and LSB
 * reading)
 */
#define LIS3DH_CTRL_REG4_BDU_MASK (1 << 7)
#define LIS3DH_CTRL_REG4_BDU_ENABLE (LIS3DH_CTRL_REG4_BDU_MASK)
#define LIS3DH_CTRL_REG4_BDU_DISABLE (0)

/**
 * Big/little endian data selection. Default value 0.
 * (0: Data LSB @ lower address; 1: Data MSB @ lower address)
 */
#define LIS3DH_CTRL_REG4_BLE_MASK (1 << 6)

#define LIS3DH_CTRL_REG4_BLE_LITTLE_ENDIAN (0)
#define LIS3DH_CTRL_REG4_BLE_BIG_ENDIAN (LIS3DH_CTRL_REG4_BLE_MASK)


/**
 * Full scale selection. default value: 00
 * (00: +/- 2G; 01: +/- 4G; 10: +/- 8G; 11: +/- 16G)
 */
#define LIS3DH_CTRL_REG4_FS1_MASK (1 << 5)
#define LIS3DH_CTRL_REG4_FS0_MASK (1 << 4)
#define LIS3DH_CTRL_REG4_FS_MASK (LIS3DH_CTRL_REG4_FS1_MASK | LIS3DH_CTRL_REG4_FS0_MASK)

#define LIS3DH_CTRL_REG4_FS_2G  (0)
#define LIS3DH_CTRL_REG4_FS_4G  (LIS3DH_CTRL_REG4_FS0_MASK)
#define LIS3DH_CTRL_REG4_FS_8G  (LIS3DH_CTRL_REG4_FS1_MASK)
#define LIS3DH_CTRL_REG4_FS_16G (LIS3DH_CTRL_REG4_FS1_MASK | LIS3DH_CTRL_REG4_FS0_MASK)

/**
 * Scale parameters, use these names when calling lis3dh_set_scale(scale).
 */
typedef enum {
  SCALE_2G = 2,
  SCALE_4G = 4,
  SCALE_8G = 8,
  SCALE_16G = 16,
} lis3dh_scale_t;

/**
 * High resolution output mode: Default value: 0
 * (0: High resolution disable; 1: High resolution Enable)
 */
#define LIS3DH_CTRL_REG4_HR_MASK  (1 << 3)

/**
 * Self test enable. Default value: 00
 * (00: Self test disabled; Other: See Table 34)
 */
#define LIS3DH_CTRL_REG4_ST1_MASK (1 << 2)
#define LIS3DH_CTRL_REG4_ST0_MASK (1 << 1)

/**
 * SPI serial interface mode selection. Default value: 0
 * (0: 4-wire interface; 1: 3-wire interface).
 */
#define LIS3DH_CTRL_REG4_SIM_MASK (1 << 0)

/**
 * Reboot memory content. Default value: 0
 * (0: normal mode; 1: reboot memory content)
 */
#define LIS3DH_CTRL_REG5_REBOOT_MASK (1 << 7)

/**
 * FIFO enable. Default value: 0
 * (0: FIFO disable; 1: FIFO Enable)
 */
#define LIS3DH_CTRL_REG5_FIFO_EN_MASK (1 << 6)

/**
 * Latch interrupt request on INT1_SRC register, with INT1_SRC register
 * cleared by reading INT1_SRC itself. Default value: 0.
 * (0: interrupt request not latched; 1: interrupt request latched)
 */
#define LIS3DH_CTRL_REG5_LIR_I1_MASK  (1 << 3)

/**
 * 4D enable: 4D detection is enabled on INT1 when 6D bit on INT1_CFG is set to 1.
 */
#define LIS3DH_CTRL_REG5_D4D_I1_MASK  (1 << 2)

/**
 * X, Y and Z axis data overrun. Default value: 0
 * (0: no overrun has occurred; 1: a new set of data has overwritten the previous ones)
 */
#define LIS3DH_STATUS_REG_ZYXOR_MASK  (1 << 7)

/**
 * Z axis data overrun. Default value: 0
 * (0: no overrun has occurred;
 * 1: a new data for the Z-axis has overwritten the previous one)
 */
#define LIS3DH_STATUS_REG_ZOR_MASK  (1 << 6)

/**
 * Y axis data overrun. Default value: 0
 * (0: no overrun has occurred;
 * 1: a new data for the Y-axis has overwritten the previous one)
 */
#define LIS3DH_STATUS_REG_YOR_MASK  (1 << 5)

/**
 * X axis data overrun. Default value: 0
 * (0: no overrun has occurred;
 * 1: a new data for the X-axis has overwritten the previous one)
 */
#define LIS3DH_STATUS_REG_XOR_MASK  (1 << 4)

/**
 * X, Y and Z axis new data available. Default value: 0
 * (0: a new set of data is not yet available; 1: a new set of data is available)
 */
#define LIS3DH_STATUS_REG_ZYXDA_MASK (1 << 3)

/**
 * Z axis new data available. Default value: 0
 * (0: a new data for the Z-axis is not yet available;
 * 1: a new data for the Z-axis is available)
 */
#define LIS3DH_STATUS_REG_ZDA_MASK  (1 << 2)

/**
 * Y axis new data available. Default value: 0
 * (0: a new data for the Y-axis is not yet available;
 * 1: a new data for the Y-axis is available)
 */
#define LIS3DH_STATUS_REG_YDA_MASK  (1 << 1)

/**
 * X axis new data available. Default value: 0
 * (0: a new data for the X-axis is not yet available;
 * 1: a new data for the X-axis is available)
 */
#define LIS3DH_STATUS_REG_XDA_MASK  (1 << 0)


#define LIS3DH_FIFO_CTRL_REG_FM_SHIFT  (6)
#define LIS3DH_FIFO_CTRL_REG_FM1_MASK  (1 << 7)
#define LIS3DH_FIFO_CTRL_REG_FM0_MASK  (1 << 6)
#define LIS3DH_FIFO_CTRL_REG_FM_MASK   (LIS3DH_FIFO_CTRL_REG_FM1_MASK | \
                                        LIS3DH_FIFO_CTRL_REG_FM0_MASK)

#define LIS3DH_FIFO_CTRL_REG_TR_MASK   (1 << 5)
#define LIS3DH_FIFO_CTRL_REG_FTH4_MASK (1 << 4)
#define LIS3DH_FIFO_CTRL_REG_FTH3_MASK (1 << 3)
#define LIS3DH_FIFO_CTRL_REG_FTH2_MASK (1 << 2)
#define LIS3DH_FIFO_CTRL_REG_FTH1_MASK (1 << 1)
#define LIS3DH_FIFO_CTRL_REG_FTH0_MASK (1 << 0)
#define LIS3DH_FIFO_CTRL_REG_FTH_SHIFT  (0)
#define LIS3DH_FIFO_CTRL_REG_FTH_MASK \
	(LIS3DH_FIFO_CTRL_REG_FTH0_MASK | \
		LIS3DH_FIFO_CTRL_REG_FTH1_MASK | \
		LIS3DH_FIFO_CTRL_REG_FTH2_MASK | \
		LIS3DH_FIFO_CTRL_REG_FTH3_MASK | \
		LIS3DH_FIFO_CTRL_REG_FTH4_MASK)

/**
 * Allowed FIFO modes.
 * Used when calling lis3dh_set_fifo_mode(mode).
 */
typedef enum {
  FIFO_MODE_BYPASS = 0x00,
  FIFO_MODE_FIFO = 0x01,
  FIFO_MODE_STREAM = 0x02,
  FIFO_MODE_STREAM_TO_FIFO = 0x03
} lis3dh_fifo_mode_t;

#define LIS3DH_FIFO_SRC_REG_WTM_MASK (1 << 7)
#define LIS3DH_FIFO_SRC_REG_OVRN_FIFO_MASK (1 << 6)
#define LIS3DH_FIFO_SRC_REG_EMPTY_MASK (1 << 5)
#define LIS3DH_FIFO_SRC_REG_FSS4_MASK (1 << 4)
#define LIS3DH_FIFO_SRC_REG_FSS3_MASK (1 << 3)
#define LIS3DH_FIFO_SRC_REG_FSS2_MASK (1 << 2)
#define LIS3DH_FIFO_SRC_REG_FSS1_MASK (1 << 1)
#define LIS3DH_FIFO_SRC_REG_FSS0_MASK (1 << 0)
#define LIS3DH_FIFO_SRC_REG_FSS_SHIFT  (0)
#define LIS3DH_FIFO_SRC_REG_FSS_MASK \
	(LIS3DH_FIFO_SRC_REG_FSS0_MASK | \
		LIS3DH_FIFO_SRC_REG_FSS1_MASK | \
		LIS3DH_FIFO_SRC_REG_FSS2_MASK | \
		LIS3DH_FIFO_SRC_REG_FSS3_MASK | \
		LIS3DH_FIFO_SRC_REG_FSS4_MASK)


#define LIS3DH_SPI_WRITE_MASK (0<<7)
/**
 * The READ bit must be set when reading
 */
#define LIS3DH_SPI_READ_MASK (1<<7)
/**
 * Multi byte transfers must assert this bit when writing the address.
 */
#define LIS3DH_SPI_MULTI_MASK (1<<6)
/**
 * Opposite of LIS3DH_SPI_MULTI_MASK.
 */
#define LIS3DH_SPI_SINGLE_MASK (0<<6)
/**
 * Mask of the address bits in the address byte during transfers.
 */
#define LIS3DH_SPI_ADDRESS_MASK (0x3F)

/**
 * Length of X, Y, Z data in bytes.
 */
#define LIS3DH_ACC_DATA_SIZE (2)


/* Platform-independent functions, implemented in lis3dh.c */
void lis3dh_set_bits(const lis3dh_reg_addr_t addr, const uint8_t bitmask);
void lis3dh_clear_bits(const lis3dh_reg_addr_t addr, const uint8_t bitmask);
int16_t lis3dh_read_xaxis();
int16_t lis3dh_read_yaxis();
int16_t lis3dh_read_zaxis();
void lis3dh_read_xyz(int16_t * buffer);
int16_t lis3dh_read_aux_adc1();
int16_t lis3dh_read_aux_adc2();
int16_t lis3dh_read_aux_adc3();
void lis3dh_set_aux_adc(const uint8_t enable, const uint8_t temperature);
void lis3dh_set_axes(const uint8_t axes);
void lis3dh_set_fifo_mode(const lis3dh_fifo_mode_t mode);
void lis3dh_set_fifo(const uint8_t enable);
void lis3dh_set_odr(const lis3dh_odr_t odr);
void lis3dh_set_scale(const lis3dh_scale_t scale);
void lis3dh_init();

/*
 * Platform specific functions, should be implemented in lis3dh-arch.c for each
 * supported platform
 */
void lis3dh_write_byte(const lis3dh_reg_addr_t addr, const uint8_t value);
uint8_t lis3dh_read_byte(const lis3dh_reg_addr_t addr);
int16_t lis3dh_read_int16(const lis3dh_reg_addr_t lsb_addr);
void lis3dh_memcpy_from_device(const lis3dh_reg_addr_t start_address,
                               uint8_t * buffer, uint8_t count);
void lis3dh_memcpy_to_device(const lis3dh_reg_addr_t start_address,
                             const uint8_t * buffer, uint8_t count);
void lis3dh_arch_init();



#endif /* LIS3DH_H_ */
