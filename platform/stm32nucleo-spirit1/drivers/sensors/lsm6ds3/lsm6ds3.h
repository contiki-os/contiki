/**
 ******************************************************************************
 * @file    lsm6ds3.h
 * @author  MEMS Application Team
 * @version V1.2.0
 * @date    28-May-2015
 * @brief   This file contains definitions for the lsm6ds3.c firmware driver
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LSM6DS3_H
#define __LSM6DS3_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "imu_6axes.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Components
 * @{
 */

/** @addtogroup LSM6DS3
 * @{
 */

/** @addtogroup LSM6DS3_Exported_Defines LSM6DS3_Exported_Defines
 * @{
 */
#ifndef NULL
#define NULL      (void *) 0
#endif

/******************************************************************************/
/*********** START ACCELEROMETER AND GYROSCOPE REGISTER MAPPING  **************/
/******************************************************************************/


/***************************************** COMMON REGISTERS ********************************************/

/**
  * @brief FIFO control register
  * \code
  * Read/write
  * Default value: 0x00
  * [7] FUNC_CFG_EN: Enable access to the embedded functions configuration registers (1) from address 02h to 32h. Default value: 0.
  * [6:0] This bit must be set to ‘0’ for the correct operation of the device
  * \endcode
*/
#define  LSM6DS3_XG_FUNC_CFG_ACCESS                                      0x01

/**
  * @brief FIFO control register
  * \code
  * Read/write
  * Default value: 0x00
  * [7:0] TPH_[7:0]: Sensor SyncronizationTime Frame with the step of 500ms and full range of 5s. Default: 0000 0000
  * \endcode
*/
#define  LSM6DS3_XG_SENSOR_SYNC_TIME_FRAME                           0x04

/**
  * @brief FIFO control register
  * \code
  * Read/write
  * Default value: 0x00
  * [7:2] This bit must be set to ‘0’ for the correct operation of the device
  * [1] HP_RST: Gyro digital HP filter reset. Default: 0
  * [1] SENSOR_SYNC_ENHP_RST: Enable sensor synchronization feature. Default 0
  * \endcode
*/
#define  LSM6DS3_XG_SENSOR_SYNC_ENABLE                             0x05


/**
  * @brief FIFO control register
  * \code
  * Read/write
  * Default value: 0x00
  * [7:0] FTH_7-0 FIFO threshold level setting - watermark flag is toggled when the number of bytes written to FIFO is greater or equal to threshold level.
  * \endcode
*/
#define LSM6DS3_XG_FIFO_CTRL1                               0x06

/**
  * @brief FIFO control register
  * \code
  * Read/write
  * Default value: 0x00
  * [7] TIMER_PEDO_FIFO_EN: Enable Pedometer step counter and time stamp as 4th sensor FIFO data. Default: 0 disabled
  * [6] TIMER_PEDO_FIFO_DRDY : Pedometer FIFO write mode. Default: 0 disabled
  * [5:4] This bit must be set to ‘0’ for the correct operation of the device
  * [3:0] FTH_[11:8] FIFO threshold level setting(1). Default value: 0000
  * \endcode
*/
#define LSM6DS3_XG_FIFO_CTRL2                               0x07

/**
  * @brief FIFO control register
  * \code
  * Read/write
  * Default value: 0x00
  * [7:6] This bit must be set to ‘0’ for the correct operation of the device
  * [5:3] DEC_FIFO_GYRO[2:0]: Gyro FIFO decimation setting. Default value: 000
  * [2:0] DEC_FIFO_XL[2:0]: XL FIFO decimation setting. Default value: 000
  * \endcode
*/
#define LSM6DS3_XG_FIFO_CTRL3                               0x08

/**
  * @brief FIFO control register
  * \code
  * Read/write
  * Default value: 0x00
  * [7] This bit must be set to ‘0’ for the correct operation of the device
  * [6] ONLY_HIGH_DATA :8 bit data storing in FIFO. Default: 0
  * [5:3] DEC_SLV1_FIFO[2:0] Second external sensor FIFO decimation setting. Default: 000
  * [2:0] DEC_SLV1_FIFO[2:0] First external sensor FIFO decimation setting. Default: 000
  * \endcode
*/
#define LSM6DS3_XG_FIFO_CTRL4                               0x09


/**
  * @brief FIFO control register
  * \code
  * Read/write
  * Default value: 0x00
  * [7] This bit must be set to ‘0’ for the correct operation of the device
  * [6:3] ODR_FIFO_[3:0]: FIFO ODR selection. Default: 0000
  * [2:0] FIFO_MODE_[2:0] : FIFO mode selection bits. Default value: 000
  * \endcode
*/
#define LSM6DS3_XG_FIFO_CTRL5                               0x0A


/**
  * @brief Angular rate sensor sign and orientation register
  * \code
  * Read/write
  * Default value: 0x00
  * [7:6] This bit must be set to ‘0’ for the correct operation of the device
  * [5] SignX_G:  Pitch axis (X) Angular rate sign. Default value: 0
  * [5] SignY_G:  Roll axis (Y) Angular rate sign. Default value: 0
  * [5] SignZ_G:  Pitch axis (Z) Angular rate sign. Default value: 0
  * [2:0] Orient[2:0] : Directional user orientation selection. Default Value: 000
  * \endcode
*/
#define LSM6DS3_XG_ORIENT_CFG_G                               0x0B

/**
  * @brief INT1 pad control registrer
  * \code
  * Read/write
  * Default value: 0x00
  * [7] INT1_PEDO: Pedometer step recognition interrupt enable on INT1 pad. Default value: 0
  * [6] INT1_SIGN_MOT: Significant motion interrupt enable on INT1 pad. Default value: 0
  * [5] INT1_FULL_FLAG: Full flag Interrupt Enable on INT1 pad. Default value: 0
  * [4] INT1_OVR: Overrun Interrupt on INT1 pad. Default value: 0
  * [3] INT1_FTH: FIFO threshold interrupt on INT1 pad. Default value: 0
  * [2] INT1_BOOT: Overrun Interrupt on INT1 pad. Default value: 0
  * [1] INT1_DRDY_G: Gyroscope Data Ready on INT1 pad. Default value: 0.
  * [0] INT1_DRDY_XL: Accelerometer Data Ready on INT1 pad. Default value: 0
  * \endcode
*/
#define LSM6DS3_XG_INT1_CTRL                                 0x0D


//new, done
/**
  * @brief INT2 pad control registrer
  * \code
  * Read/write
  * Default value: 0x00
  * [7] INT2_PEDO: Pedometer step recognition interrupt enable on INT1 pad. Default value: 0
  * [6] INT2_SIGN_MOT: Significant motion interrupt enable on INT1 pad. Default value: 0
  * [5] INT2_FULL_FLAG: Full flag Interrupt Enable on INT1 pad. Default value: 0
  * [4] INT2_OVR: Overrun Interrupt on INT1 pad. Default value: 0
  * [3] INT2_FTH: FIFO threshold interrupt on INT1 pad. Default value: 0
  * [2] INT2_BOOT: Overrun Interrupt on INT1 pad. Default value: 0
  * [1] INT2_DRDY_G: Gyroscope Data Ready on INT1 pad. Default value: 0.
  * [0] INT2_DRDY_XL: Accelerometer Data Ready on INT1 pad. Default value: 0
  * \endcode
*/
#define LSM6DS3_XG_INT2_CTRL                                 0x0E


/**
  * @brief Device identifier register.
  * \code
  * Read
  * Default value:69
  * [7:0] This read-only register contains the device identifier
  * \endcode
*/
#define LSM6DS3_XG_WHO_AM_I_ADDR                            0x0F


/**
  * @brief Control Register 3
  * \code
  * Read/write
  * Default value: 0x00
  * [7] BOOT: Reboot memory content. Default value: 0
  * [6] BDU: Block Data Update. Default value: 0
  * [5] H_LACTIVE: Interrupt activation level. Default value: 0
  * [4] PP_OD: Push-pull/Open Drain selection on INT pad. Default value: 0
  * [3] SIM: SPI Serial Interface Mode selection. Default value: 0
  * [2] IF_INC: Register address automatically incremented during a multiple byte access with a serial interface (I2C or SPI). Default value: 0
  * [1] BLE: Big/Little Endian Data Selection. Default value: 0
  * [0] SW_RESET: Software Reset. Default value: 0
  * \endcode
*/
#define LSM6DS3_XG_CTRL3_C                                  0x12

/**
  * @brief Control Register 4
  * \code
  * Read/write
  * Default value: 0x00
  * [7] BW_SCAL_ODR: Accelerometer bandwidth selection. Default value: 0
  * [6] SLEEP_G: Gyroscope sleep mode enable. Default value: 0
  * [5] INT2_on_INT1: All interrupt signals available on INT1 pad enable. Default value: 0
  * [4] This bit must be set to ‘0’ for the correct operation of the device
  * [3] DRDY_MASK: Configuration 1(3) Data Available Enable bit. Default Value: 0
  * [2] I2C_disable Disable I2C interface. Default value: 0
  * [1] MODE3_EN Enable auxiliary SPI interface (Mode3, refer to Table 1.). Default value: 0
  * [0] STOP_ON_FTH Enable FIFO threshold level use. Default value: 0.

  * \endcode
*/
#define LSM6DS3_XG_CTRL4_C                                  0x13

/**
  * @brief Control Register 4
  * \code
  * Read/write
  * Default value: 0x00
  * [7:4] This bit must be set to ‘0’ for the correct operation of the device
  * [3:2] ST_G[1:0]: Angular rate sensor Self Test Enable. Default value: 00
  * [1:0] ST_XL[1:0]: Linear acceleration sensor Self Test Enable. Default value: 00
  * \endcode
*/
#define LSM6DS3_XG_CTRL5_C                                  0x14


/**
  * @brief Control Register 10
  * \code
  * Read/write
  * Default value: 0x38
  * [7:6] These bits must be set to ‘0’ for the correct operation of the device
  * [5] Zen_G: Gyroscope’s Z-axis output enable. Default value: 1
  * [4] Yen_G: Gyroscope’s Y-axis output enable. Default value: 1
  * [3] Xen_G: Gyroscope’s X-axis output enable. Default value: 1
  * [2] FUNC_EN: Enable embedded functionalities (pedometer, tilt and significant motion) .  Default value: 0
  * [1] PEDO_RST_STEP: Reset pedometer step counter .  Default value: 0
  * [0] SIGN_MOTION_EN: Enable significant motion function. For a correct functionality of significant motion function, TILT_EN bit in FUNC_SRC (53h) register must be set to 1 .  Default value: 1
  * \endcode
*/
#define LSM6DS3_XG_CTRL10_C                                 0x19


/**
  * @brief Sensor hub Master config Register
  * \code
  * Read/write
  * Default value: 0x00
  * [7] DRDY_ON_INT1:Manage the DRDY signal on INT1 pad. Default: 0
  * [6] DATA_VALID_SEL_FIFO: Selection of FIFO data-valid signal. Default value: 0
  * [5] This bit must be set to ‘0’ for the correct operation of the device
  * [4] START_CONFIG: Sensor Hub trigger signal selection. Default value: 0
  * [3] PULL_UP_EN: Auxiliary I2C pull-up. Default value: 0
  * [2] PASS_THROUGH_MODE: I2C interface pass-through. Default value: 0
  * [1] IRON_EN:Enable soft iron correction algorithm for magnetometer. Default value: 0.
  * [0] MASTER_ON: Sensor Hub I2C master enable. Default: 0
  * \endcode
*/
#define LSM6DS3_XG_MASTER_CONFIG                                 0x1A


/**
  * @brief Wake up interrupt source register
  * \code
  * Read
  * Default value: output
  * [7:6] This bit must be set to ‘0’ for the correct operation of the device
  * [5] FF_IA: Free fall event detection status. Default: 0
  * [4] SLEEP_STATE_IA: Sleep event status. Default value: 0
  * [3] WU_IA: Wake up event detection status. Default
  * [2] X_WU: detection status on X axis. Default value: 0
  * [1] Y_WU: detection status on Y axis. Default value: 0
  * [0] Z_WU: detection status on Z axis. Default value: 0
  * \endcode
*/
#define LSM6DS3_XG_WAKE_UP_SRC                                 0x1B

/**
  * @brief Tap source register
  * \code
  * Read
  * Default value: output
  * [7] This bit must be set to ‘0’ for the correct operation of the device
  * [6] TAP_IA: Tap event detection status. Default: 0
  * [5] SINGLE_TAP: Single tap event status. Default value: 0
  * [4] DOUBLE_TAP: Double tap event detection status. Default value: 0
  * [3] TAP_SIGN: Sign of acceleration detected by tap event. Default: 0
  * [2] X_TAP: Tap event detection status on X axis. Default value: 0
  * [1] Y_TAP: Tap event detection status on Y axis. Default value: 0
  * [0] Z_TAP: Tap event detection status on Z axis. Default value: 0
  * \endcode
*/
#define LSM6DS3_XG_TAP_SRC                                    0x1C

/**
  * @brief Portrait, landscape face-up and face-down source register
  * \code
  * Read
  * Default value: output
  * [7] This bit must be set to ‘0’ for the correct operation of the device
  * [6] D6D_IA: Interrupt active for change position portrait, landscape, face-up, face-down. Default value: 0
  * [5] ZH: Z-axis high event (over-threshold). Default value: 0
  * [4] ZL: Z-axis low event (under-threshold). Default value: 0
  * [5] YH: Y-axis high event (over-threshold). Default value: 0
  * [4] YL: Y-axis low event (under-threshold). Default value: 0
  * [5] X_H: X-axis high event (over-threshold). Default value: 0
  * [4] X_L: X-axis low event (under-threshold). Default value: 0
  * \endcode
*/
#define LSM6DS3_XG_D6D_SRC                                   0x1D

/**
  * @brief Status register
  * \code
  * Read
  * Default value: output
  * [7:4] No meaning set
  * [3] EV_BOOT: Boot running flag signal. Default value: 0
  * [2] No meaning set
  * [1] GDA: Gyroscope new data avaialble. Default value: 0
  * [0] XLDA: Accelerometer new data avaialble. Default value: 0
  * \endcode
*/
#define LSM6DS3_XG_STATUS_REG                                  0x1E

/**
  * @brief FIFO status control register
  * \code
  * Read
  * Default value: 0x00
  * [7:0] DIFF_FIFO_[7:0]: Number of unread words (16 bit axes) stored in FIFO . For a complete number of unread samples, consider DIFF_FIFO [11:8] in FIFO_STATUS2 (3Bh)
  * \endcode
*/
#define LSM6DS3_XG_FIFO_STATUS1                                   0x3A

/**
  * @brief FIFO status control register (r). For a proper reading of the register it is suggested to set BDU bit in CTRL3_C (12h) to 0.
  * \code
  * Read
  * Default value: 0x00
  * [7] FTH FIFO watermark status. Deafult value: 0
  * [6] OVER_RUN: FIFO overrun status. Default value: 0
  * [5] FIFO_FULL: FIFO full status. Default value: 0
  * [5] FIFO_EMPTY: FIFO empty bit. Default value: 0; 0: FIFO contains data; 1: FIFO is empty
  * [3:0] DIFF_FIFO_[11:8] Number of unread words (16 bit axes) stored in FIFO : For a complete number of unread samples, consider DIFF_FIFO [11:8] in FIFO_STATUS1 (3Ah)
  * \endcode
*/
#define LSM6DS3_XG_FIFO_STATUS2                                  0x3B

/**
  * @brief FIFO status control register (r). For a proper reading of the register it is suggested to set BDU bit in CTRL3_C (12h) to 0
  * \code
  * Read
  * Default value: 0x00
  * [7:0] FIFO_PATTERN_[7:0] : Word of recursive pattern read at the next reading
  * \endcode
*/
#define LSM6DS3_XG_FIFO_STATUS3                                  0x3C

/**
  * @brief FIFO status control register (r). For a proper reading of the register it is suggested to set BDU bit in CTRL3_C (12h) to 0
  * \code
  * Read
  * Default value: 0x00
  * [1:0] FIFO_PATTERN_[9:8] : Word of recursive pattern read at the next reading
  * \endcode
*/
#define LSM6DS3_XG_FIFO_STATUS4                                  0x3D

/**
  * @brief FIFO status control register (r). For a proper reading of the register it is suggested to set BDU bit in CTRL3_C (12h) to 0.
  * \code
  * Read
  * Default value: 0x00
  * [4:0] FIFO_PATTERN_[9:8] : Word of recursive pattern read at the next reading
  * \endcode
*/
#define LSM6DS3_XG_FIFO_STATUS4                                  0x3D

/**
  * @brief FIFO data output register (r). For a proper reading of the register it is suggested to set BDU bit in CTRL3_C (12h) to 0.
  * \code
  * Read
  * Default value: 0x00
  * [7:0] DATA_OUT_FIFO_L_[7:0]: FIFO data output (First byte)
  * \endcode
*/
#define LSM6DS3_XG_FIFO_DATA_OUT_L                                  0x3E

/**
  * @brief FIFO data output register (r). For a proper reading of the register it is suggested to set BDU bit in CTRL3_C (12h) to 0.
  * \code
  * Read
  * Default value: 0x00
  * [7:0] DATA_OUT_FIFO_H_[7:0]: FIFO data output (second byte)
  * \endcode
*/
#define LSM6DS3_XG_FIFO_DATA_OUT_H                                  0x3F

/**
  * @brief Time stamp first byte data output register (r). The value is expressed as 24 bit and the bit resolution is defined by setting value in WAKE_UP_DUR (5Ch).
  * \code
  * Read
  * Default value: output
  * [7:0] TIMESTAMP0_[7:0]: FIFO first byte data output
  * \endcode
*/
#define LSM6DS3_XG_TIMESTAMP0_REG                                  0x40

/**
  * @brief Time stamp second byte data output register (r). The value is expressed as 24 bit and the bit resolution is defined by setting value in WAKE_UP_DUR (5Ch).
  * \code
  * Read
  * Default value: output
  * [7:0] TIMESTAMP1_[7:0]: FIFO second byte data output
  * \endcode
*/
#define LSM6DS3_XG_TIMESTAMP1_REG                                  0x41

/**
  * @brief Time stamp third byte data output register (r). The value is expressed as 24 bit and the bit resolution is defined by setting value in WAKE_UP_DUR (5Ch).
  * \code
  * Read
  * Default value: output
  * [7:0] TIMESTAMP2_[7:0]: FIFO third byte data output
  * \endcode
*/
#define LSM6DS3_XG_TIMESTAMP2_REG                                  0x42

/**
  * @brief Step counter output register (r).
  * \code
  * Read
  * Default value: output
  * [7:0] STEP_COUNTER_L_[7:0]: Step counter output (LSbyte)
  * \endcode
*/
#define LSM6DS3_XG_STEP_COUNTER_L                                  0x4B

/**
  * @brief Step counter output register (r).
  * \code
  * Read
  * Default value: output
  * [7:0] STEP_COUNTER_H_[7:0]: Step counter output (MSbyte)
  * \endcode
*/
#define LSM6DS3_XG_STEP_COUNTER_H                                  0x4C

/**
  * @brief Significant motion, tilt, step detector, soft iron and sensor hub interrupt source register
  * \code
  * Read
  * Default value: output
  * [7] This bit must be set to ‘0’ for the correct operation of the device
  * [6] SIGN_MOTION_IA: Significant motion event detection status. Default value: 0
  * [5] TILT_IA: Tilt event detection status. Default value: 0
  * [5] STEP_DETECTED: Step detector event detection status. Default value: 0
  * [3:2]  This bit must be set to ‘0’ for the correct operation of the device
  * [1] SI_END_OP:Soft iron calculation status. Default value: 0
  * [0] SENSORHUB_END_OP:Senso hub communication status.  Default value: 0
  * \endcode
*/
#define LSM6DS3_XG_FUNC_SRC                                             0x53

/**
  * @brief Time stamp, pedometer, tilt, filtering, and tap recognition functions configuration register
  * \code
  * Read/write
  * Default value: 0x00
  * [7] TIMER_EN: Time stamp count enable, output data are collected in TIMESTAMP0_REG (40h), TIMESTAMP1_REG (41h), TIMESTAMP2_REG (42h) register. Default: 0
  * [6] PEDO_EN: Pedometer algorithm enable(1). Default value: 0
  * [5] TILT_EN: Tilt calculation enable.(2) Default value: 0
  * [4] This bit must be set to ‘0’ for the correct operation of the device
  * [3] TAP_X_EN: Enable X direction in tap recognition. Default value: 0
  * [2] TAP_Y_EN: Enable Z direction in tap recognition. Default value: 0
  * [1] TAP_Z_EN: Enable Z direction in tap recognition. Default value: 0
  * [0] LIR: Relatch of the time stamp, pedometer, tilt, filtering, and tap recognition functions routed to PINs.
  * \endcode
*/
#define LSM6DS3_XG_TAP_CFG                                             0x58

/**
  * @brief Portrait/landscape position and tap function threshold register
  * \code
  * Read/write
  * Default value: 0x00
  * [7] This bit must be set to ‘0’ for the correct operation of the device
  * [6:5] SIXD_THS[1:0]: Threshold for D6D function. Default value: 00
  * [4:0] TAP_THS[4:0]: Threshold for tap recognition. Default value: 0000
  * \endcode
*/
#define LSM6DS3_XG_TAP_THS_6D                                             0x59

/**
  * @brief Tap recognition function setting register (r/w)
  * \code
  * Read/write
  * Default value: 0x00
  * [7:4] DUR[3:0]: Duration of maximum time gap for double tap recognition. Default: 0000
  * [3:2] QUIET[1:0]: Expected quiet time after a tap detection. Default value: 00
  * [1:0] SHOCK[1:0]: Maximum duration of over-threshold event. Default value: 00
  * \endcode
*/
#define LSM6DS3_XG_INT_DUR2                                             0x5A

/**
  * @brief Tap recognition function setting register
  * \code
  * Read/write
  * Default value: 0x00
  * [7] SINGLE_DOUBLE_TAP: Single/double tap event detection. Default: 0
  * [6] INACTIVITY: Inactivity event enable. Default value: 0
  * [5:0] WK_THS[5:0]:Threshold for wake-up. Default value: 0000
  * \endcode
*/
#define LSM6DS3_XG_WAKE_UP_THS                                                   0x5B

/**
  * @brief Free-fall, wake-up, time stamp and sleep mode functions duration setting register (r/w).
  * \code
  * Read/write
  * Default value: 0x00
  * [7] FF_DUR5:Free fall duration event. Default: 0
  * [6:5] WAKE_DUR[1:0]: Wake up duration event. Default: 00
  * [4] TIMER_HR: Time stamp register resolution setting(1). Default value: 0
  * [3:0] SLEEP_DUR[3:0] : Duration to go in sleep mode. Default value: 0000
  * \endcode
*/
#define LSM6DS3_XG_WAKE_UP_DUR                                                   0x5C

/**
  * @brief Free-fall function duration setting register
  * \code
  * Read/write
  * Default value: 0x00
  * [7:3] FF_DUR[4:0]: Free fall duration event. Default: 0. For the complete configuration of the free fall duration, refer to FF_DUR5 in WAKE_UP_DUR (5Ch) configuration
  * [2:0] FF_THS[2:0]: Free fall threshold setting. Default: 000.
  * \endcode
*/
#define LSM6DS3_XG_WAKE_FREE_FALL                                                   0x5D

/**
  * @brief Functions routing on INT1 register
  * \code
  * Read/write
  * Default value: 0x00
  * [7] INT1_INACT_STATE: Routing on INT1 of inactivity mode. Default: 0
  * [6] INT1_SINGLE_TAP: Single tap recognition routing on INT1. Default: 0
  * [5] INT1_WU: Routing of wake-up event on INT1. Default value: 0
  * [4] INT1_FF: Routing of free-fall event on INT1. Default value: 0
  * [3] INT1_TAP: Routing of tap event on INT1. Default value: 0
  * [2] INT1_6D: Routing of 6D event on INT1. Default value: 0
  * [1] INT1_TILT: Routing of tilt event on INT1. Default value: 0
  * [0] INT1_TIMER: Routing of end counter event of timer on INT1. Default value: 0
  * \endcode
*/
#define LSM6DS3_XG_MD1_CFG                                                   0x5E

/**
  * @brief Functions routing on INT2 register
  * \code
  * Read/write
  * Default value: 0x00
  * [7] INT2_INACT_STATE: Routing on INT1 of inactivity mode. Default: 0
  * [6] INT2_SINGLE_TAP: Single tap recognition routing on INT1. Default: 0
  * [5] INT2_WU: Routing of wake-up event on INT1. Default value: 0
  * [4] INT2_FF: Routing of free-fall event on INT1. Default value: 0
  * [3] INT2_TAP: Routing of tap event on INT1. Default value: 0
  * [2] INT2_6D: Routing of 6D event on INT1. Default value: 0
  * [1] INT2_TILT: Routing of tilt event on INT1. Default value: 0
  * [0] INT2_TIMER: Routing of end counter event of timer on INT1. Default value: 0
  * \endcode
*/
#define LSM6DS3_XG_MD2_CFG                                                   0x5F

/***************************************** SENSORHUB REGISTERS ********************************************/

/**
  * @brief SENSORHUB REGISTER 1 : SLV0 first external sensor, first axis output register (r). The value is expressed as 16bit word in two’s complement
  * \code
  * Read
  * Default value: 0x00
  * [7:0] SHUB1[7:0]: SLV0 first external sensor, first byte of the first axis.
  * \endcode
*/
#define  LSM6DS3_XG_SENSORHUB1_REG                       0x2E

/**
  * @brief SENSORHUB REGISTER 2 : SLV0 first external sensor, first axis output register (r). The value is expressed as 16bit word in two’s complement
  * \code
  * Read
  * Default value: 0x00
  * [7:0] SHUB2[7:0]: SLV0 first external sensor, second byte of the first axis.
  * \endcode
*/
#define  LSM6DS3_XG_SENSORHUB2_REG                       0x2F

/**
  * @brief SENSORHUB REGISTER 3 : SLV0 first external sensor, second axis output register (r). The value is expressed as 16bit word in two’s complement
  * \code
  * Read
  * Default value: 0x00
  * [7:0] SHUB3[7:0]: SLV0 first external sensor, first byte of the second axis.
  * \endcode
*/
#define  LSM6DS3_XG_SENSORHUB3_REG                       0x30

/**
  * @brief SENSORHUB REGISTER 4 : SLV0 first external sensor, second axis output register (r). The value is expressed as 16bit word in two’s complement
  * \code
  * Read
  * Default value: 0x00
  * [7:0] SHUB4[7:0]: SLV0 first external sensor, second byte of the second axis.
  * \endcode
*/
#define  LSM6DS3_XG_SENSORHUB4_REG                       0x31

/**
  * @brief SENSORHUB REGISTER 5 : SLV0 first external sensor, third axis output register (r). The value is expressed as 16bit word in two’s complement
  * \code
  * Read
  * Default value: 0x00
  * [7:0] SHUB5[7:0]: SLV0 first external sensor, second byte of the second axis.
  * \endcode
*/
#define  LSM6DS3_XG_SENSORHUB5_REG                       0x32

/**
  * @brief SENSORHUB REGISTER 6 : SLV0 first external sensor, third axis output register (r). The value is expressed as 16bit word in two’s complement
  * \code
  * Read
  * Default value: 0x00
  * [7:0] SHUB6[7:0]: SLV0 first external sensor, second byte of the third axis
  * \endcode
*/
#define  LSM6DS3_XG_SENSORHUB6_REG                       0x33

/**
  * @brief SENSORHUB REGISTER 7 : SLV1 second external sensor, first axis output register (r). The value is expressed as 16bit word in two’s complement
  * \code
  * Read
  * Default value: 0x00
  * [7:0] SHUB7[7:0]: SLV1 second external sensor, first byte of the first axis.
  * \endcode
*/
#define  LSM6DS3_XG_SENSORHUB7_REG                       0x34

/**
  * @brief SENSORHUB REGISTER 8 : SLV1 second external sensor, first axis output register (r). The value is expressed as 16bit word in two’s complement
  * \code
  * Read
  * Default value: 0x00
  * [7:0] SHUB8[7:0]: SLV1 second external sensor, second byte of the first axis.
  * \endcode
*/
#define  LSM6DS3_XG_SENSORHUB8_REG                       0x35

/**
  * @brief SENSORHUB REGISTER 9 : SLV1 second external sensor,, second axis output register (r). The value is expressed as 16bit word in two’s complement
  * \code
  * Read
  * Default value: 0x00
  * [7:0] SHUB9[7:0]: SLV1 second external sensor, first byte of the second axis.
  * \endcode
*/
#define  LSM6DS3_XG_SENSORHUB9_REG                       0x36

/**
  * @brief SLV1 second external sensor, second axis output register (r). The value is expressed as 16bit word in two’s complement
  * \code
  * Read
  * Default value: 0x00
  * [7:0] SHUB10[7:0]: SLV1 second external sensor, second byte of the second axis.
  * \endcode
*/
#define  LSM6DS3_XG_SENSORHUB10_REG                       0x37

/**
  * @brief SLV1 second external sensor, third axis output register (r). The value is expressed as 16bit word in two’s complement
  * \code
  * Read
  * Default value: 0x00
  * [7:0] SHUB11[7:0]: SLV1 second external sensor, first byte of the third axis.
  * \endcode
*/
#define  LSM6DS3_XG_SENSORHUB11_REG                       0x38

/**
  * @brief SLV1 second external sensor, third axis output register (r). The value is expressed as 16bit word in two’s complement
  * \code
  * Read
  * Default value: 0x00
  * [7:0] SHUB12[7:0]: SLV1 second external sensor, second byte of the third axis.
  * \endcode
*/
#define  LSM6DS3_XG_SENSORHUB12_REG                       0x39




/***************************************** GYROSCOPE REGISTERS ********************************************/

/**
  * @brief Angular rate sensor Control Register 2
  * \code
  * Read/write
  * Default value: 0x00
  * [7:4] ODR_G[3:0]: Gyroscope output data rate selection
  * [3:2] FS_G[1-0]: Gyroscope full-scale selection
  * [1] FS_125: Gyroscope full-scale at 125 dps
  * [0] This bit must be set to ‘0’ for the correct operation of the device
  * \endcode
 */
#define LSM6DS3_XG_CTRL2_G                                  0x11


/**
  * @brief Angular rate sensor Control Register 6
  * \code
  * Read/write
  * Default value: 0x00
  * [7] TRIG_EN: Data edge sensitive trigger Enable. Default value: 0
  * [6] LVLen: Data level sensitive trigger Enable. Default value: 0
  * [5] LVL2en: Level sensitive latched Enable. Default value: 0
  * [4] XL_H_MODE: High Performance operating mode disable for accelerometer(1). Default value: 0
  * [3:0] This bit must be set to ‘0’ for the correct operation of the device
  * \endcode
 */
#define LSM6DS3_XG_CTRL6_G                                  0x15


/**
* @brief Angular rate sensor Control Register 7
* \code
* Read/write
* Default value: 0x00
* [7] G_H_MODE: High Performance operating mode disable for Gyroscope(1) . Default: 0
* [6] HP_EN: High Pass filter Enable. Default Value: 0
* [5:4] HPCF_G[1:0]: Gyroscope High Pass filter Cut Off frequency selection. Default value: 00
* [3:0] This bit must be set to ‘0’ for the correct operation of the device
* \endcode
*/
#define LSM6DS3_XG_CTRL7_G                                  0x16

/**
  * @brief Gyroscope data (LSB)
  * \code
  * Read
  * \endcode
 */
#define LSM6DS3_XG_OUT_X_L_G                                0x22


/**
  * @brief Gyroscope data (MSB)
  * \code
  * Read
  * \endcode
 */
#define LSM6DS3_XG_OUT_X_H_G                                0x23


/**
 * @brief Gyroscope data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS3_XG_OUT_Y_L_G                                0x24


/**
 * @brief Gyroscope data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS3_XG_OUT_Y_H_G                                0x25


/**
 * @brief Gyroscope data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS3_XG_OUT_Z_L_G                                0x26


/**
 * @brief Gyroscope data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS3_XG_OUT_Z_H_G                                0x27



/*************************************** ACCELEROMETER REGISTERS *******************************************/

/**
 * @brief Linear acceleration sensor Control Register 1
 * \code
 * Read/write
 * Default value: 0x00
 * [7:4] ODR_XL3-0: Accelerometer Output data rate and power mode selection
 * [3:2] FS_XL1-0: Accelerometer full-scale selection
 * [1:0] BW_XL1-0: Anti-aliasing filter bandwidth selection
 * \endcode
 */
#define LSM6DS3_XG_CTRL1_XL                                 0x10

/**
* @brief XL sensor Control Register 8
* \code
* Read/write
* Default value: 0x00
* [7:3] This bit must be set to ‘0’ for the correct operation of the device
* [2] SLOPE_FDS: Enable HP filter on output registers and FIFO. Default value: 0
* [1:0] This bit must be set to ‘0’ for the correct operation of the device
* \endcode
*/
#define LSM6DS3_XG_CTRL8_XL                                 0x17

/**
  * @brief Linear acceleration sensor Control Register 9
  * \code
  * Read/write
  * Default value: 0x38
  * [7:6] These bits must be set to ‘0’ for the correct operation of the device
  * [5] Zen_XL: Accelerometers’s Z-axis output enable
  * [4] Yen_XL: Accelerometers’s Y-axis output enable
  * [3] Xen_XL: Accelerometers’s X-axis output enable
  * [2:0] These bits must be set to ‘0’ for the correct operation of the device
  * \endcode
*/
#define LSM6DS3_XG_CTRL9_XL                                 0x18


/**
 * @brief Accelerometer data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS3_XG_OUT_X_L_XL                                0x28


/**
 * @brief Accelerometer data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS3_XG_OUT_X_H_XL                                0x29


/**
 * @brief Accelerometer data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS3_XG_OUT_Y_L_XL                                0x2A


/**
 * @brief Accelerometer data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS3_XG_OUT_Y_H_XL                                0x2B


/**
 * @brief Accelerometer data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS3_XG_OUT_Z_L_XL                                0x2C


/**
 * @brief Accelerometer data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS3_XG_OUT_Z_H_XL                                0x2D

/******************************************************************************/
/************* END ACCELEROMETER AND GYROSCOPE REGISTER MAPPING  **************/
/******************************************************************************/


/************************************** COMMON REGISTERS VALUE *******************************************/

/**
* @brief Device Address
*/
#define LSM6DS3_ADDRESS_LOW                                 0xD4    // SAD[0] = 0
#define LSM6DS3_ADDRESS_HIGH                                0xD6    // SAD[0] = 1
#define LSM6DS3_XG_MEMS_ADDRESS                             LSM6DS3_ADDRESS_LOW    // SAD[0] = 0


/**
 * @brief Device Identifier. Default value of the WHO_AM_I register.
 */
#define I_AM_LSM6DS3_XG                                 ((uint8_t)0x69)


/**
  * @brief  Register address automatically incremented during a multiple byte
  *         access with a serial interface (I2C or SPI). Default value of the
  *         LSM6DS3_XG_CTRL3_C register.
  */
#define LSM6DS3_XG_IF_INC                               ((uint8_t)0x04)

#define LSM6DS3_XG_IF_INC_MASK                          ((uint8_t)0x04)

/** @defgroup LSM6DS3_XG_FIFO_Output_Data_Rate_Selection_FIFO_CTRL5 LSM6DS3_XG_FIFO_Output_Data_Rate_Selection_FIFO_CTRL5
 * @{
 */
#define LSM6DS3_XG_FIFO_ODR_NA                          ((uint8_t)0x00) /*!< FIFO ODR NA */
#define LSM6DS3_XG_FIFO_ODR_10HZ                        ((uint8_t)0x08) /*!< FIFO ODR 10Hz */
#define LSM6DS3_XG_FIFO_ODR_25HZ                        ((uint8_t)0x10) /*!< FIFO ODR 25Hz */
#define LSM6DS3_XG_FIFO_ODR_50HZ                        ((uint8_t)0x18) /*!< FIFO ODR 50Hz */
#define LSM6DS3_XG_FIFO_ODR_100HZ                       ((uint8_t)0x20) /*!< FIFO ODR 100Hz */
#define LSM6DS3_XG_FIFO_ODR_200HZ                       ((uint8_t)0x28) /*!< FIFO ODR 200Hz */
#define LSM6DS3_XG_FIFO_ODR_400HZ                       ((uint8_t)0x30) /*!< FIFO ODR 400Hz */
#define LSM6DS3_XG_FIFO_ODR_800HZ                       ((uint8_t)0x38) /*!< FIFO ODR 800Hz */
#define LSM6DS3_XG_FIFO_ODR_1600HZ                      ((uint8_t)0x40) /*!< FIFO ODR 1600Hz */
#define LSM6DS3_XG_FIFO_ODR_3300HZ                      ((uint8_t)0x48) /*!< FIFO ODR 3300Hz */
#define LSM6DS3_XG_FIFO_ODR_6600HZ                      ((uint8_t)0x50) /*!< FIFO ODR 6600Hz */

#define LSM6DS3_XG_FIFO_ODR_MASK                        ((uint8_t)0x78)
/**
 * @}
 */

/** @defgroup LSM6DS3_XG_FIFO_Mode_Selection_FIFO_CTRL5 LSM6DS3_XG_FIFO_Mode_Selection_FIFO_CTRL5
 * @{
 */
#define LSM6DS3_XG_FIFO_MODE_BYPASS                     ((uint8_t)0x00) /*!< BYPASS Mode. FIFO turned off */
#define LSM6DS3_XG_FIFO_MODE_FIFO                       ((uint8_t)0x01) /*!< FIFO Mode. Stop collecting data when FIFO is full */
#define LSM6DS3_XG_FIFO_MODE_CONTINUOUS_THEN_FIFO       ((uint8_t)0x03) /*!< CONTINUOUS mode until trigger is deasserted, then FIFO mode */
#define LSM6DS3_XG_FIFO_MODE_BYPASS_THEN_CONTINUOUS     ((uint8_t)0x04) /*!< BYPASS mode until trigger is deasserted, then CONTINUOUS mode */
#define LSM6DS3_XG_FIFO_MODE_CONTINUOUS_OVERWRITE       ((uint8_t)0x05) /*!< CONTINUOUS mode. If the FIFO is full the new sample overwrite the older one */

#define LSM6DS3_XG_FIFO_MODE_MASK                       ((uint8_t)0x07)
/**
 * @}
 */


/************************************** GYROSCOPE REGISTERS VALUE *******************************************/


/** @addtogroup LSM6DS3_XG_Gyroscope_Output_Data_Rate_Selection_CTRL_REG1_G LSM6DS3_XG_Gyroscope_Output_Data_Rate_Selection_CTRL_REG1_G
 * @{
 */
#define LSM6DS3_G_ODR_PD                                ((uint8_t)0x00) /*!< Output Data Rate: Power-down*/
#define LSM6DS3_G_ODR_13HZ                              ((uint8_t)0x10) /*!< Output Data Rate: 13 Hz*/
#define LSM6DS3_G_ODR_26HZ                              ((uint8_t)0x20) /*!< Output Data Rate: 26 Hz*/
#define LSM6DS3_G_ODR_52HZ                              ((uint8_t)0x30) /*!< Output Data Rate: 52 Hz */
#define LSM6DS3_G_ODR_104HZ                             ((uint8_t)0x40) /*!< Output Data Rate: 104 Hz */
#define LSM6DS3_G_ODR_208HZ                             ((uint8_t)0x50) /*!< Output Data Rate: 208 Hz */
#define LSM6DS3_G_ODR_416HZ                             ((uint8_t)0x60) /*!< Output Data Rate: 416 Hz */
#define LSM6DS3_G_ODR_833HZ                             ((uint8_t)0x70) /*!< Output Data Rate: 833 Hz */
#define LSM6DS3_G_ODR_1660HZ                            ((uint8_t)0x80) /*!< Output Data Rate: 1.66 kHz */

#define LSM6DS3_G_ODR_MASK                              ((uint8_t)0xF0)

/**
 * @}
 */


/** @addtogroup LSM6DS3_XG_Gyroscope_Full_Scale_Selection_CTRL2_G LSM6DS3_XG_Gyroscope_Full_Scale_Selection_CTRL2_G
 * @{
 */
#define LSM6DS3_G_FS_125_DISABLE                       ((uint8_t)0x00) /*!< Full scale: 125 dps enable: disable */
#define LSM6DS3_G_FS_125_ENABLE                        ((uint8_t)0x02) /*!< Full scale: 125 dps enable: enable */

#define LSM6DS3_G_FS_125_MASK                          ((uint8_t)0x02)

#define LSM6DS3_G_FS_245                               ((uint8_t)0x00) /*!< Full scale: 245 dps*/
#define LSM6DS3_G_FS_500                               ((uint8_t)0x04) /*!< Full scale: 500 dps */
#define LSM6DS3_G_FS_1000                              ((uint8_t)0x08) /*!< Full scale: 1000 dps */
#define LSM6DS3_G_FS_2000                              ((uint8_t)0x0C) /*!< Full scale: 2000 dps */

#define LSM6DS3_G_FS_MASK                              ((uint8_t)0x0C)

/**
 * @}
 */


/** @addtogroup LSM6DS3_XG_Gyroscope_Z_Axis_Output_Enable_Selection_CTRL10_C LSM6DS3_XG_Gyroscope_Z_Axis_Output_Enable_Selection_CTRL10_C
 * @{
 */
#define LSM6DS3_G_ZEN_DISABLE                          ((uint8_t)0x00) /*!< Gyroscope’s Z-axis output enable: disable */
#define LSM6DS3_G_ZEN_ENABLE                           ((uint8_t)0x20) /*!< Gyroscope’s Z-axis output enable: enable */

#define LSM6DS3_G_ZEN_MASK                             ((uint8_t)0x20)

/**
 * @}
 */


/** @addtogroup LSM6DS3_XG_Gyroscope_Y_Axis_Output_Enable_Selection_CTRL10_C LSM6DS3_XG_Gyroscope_Y_Axis_Output_Enable_Selection_CTRL10_C
 * @{
 */
#define LSM6DS3_G_YEN_DISABLE                          ((uint8_t)0x00) /*!< Gyroscope’s Y-axis output enable: disable */
#define LSM6DS3_G_YEN_ENABLE                           ((uint8_t)0x10) /*!< Gyroscope’s Y-axis output enable: enable */

#define LSM6DS3_G_YEN_MASK                             ((uint8_t)0x10)

/**
 * @}
 */


/** @addtogroup LSM6DS3_XG_Gyroscope_X_Axis_Output_Enable_Selection_CTRL10_C LSM6DS3_XG_Gyroscope_X_Axis_Output_Enable_Selection_CTRL10_C
 * @{
 */
#define LSM6DS3_G_XEN_DISABLE                          ((uint8_t)0x00) /*!< Gyroscope’s X-axis output enable: disable */
#define LSM6DS3_G_XEN_ENABLE                           ((uint8_t)0x08) /*!< Gyroscope’s X-axis output enable: enable */

#define LSM6DS3_G_XEN_MASK                             ((uint8_t)0x08)

/**
 * @}
 */


/************************************ ACCELEROMETER REGISTERS VALUE *****************************************/

/** @addtogroup LSM6DS3_XG_Accelerometer_Output_Data_Rate_Selection_CTRL1_XL LSM6DS3_XG_Accelerometer_Output_Data_Rate_Selection_CTRL1_XL
 * @{
 */
#define LSM6DS3_XL_ODR_PD                               ((uint8_t)0x00) /*!< Output Data Rate: Power-down*/
#define LSM6DS3_XL_ODR_13HZ                             ((uint8_t)0x10) /*!< Output Data Rate: 13 Hz*/
#define LSM6DS3_XL_ODR_26HZ                             ((uint8_t)0x20) /*!< Output Data Rate: 26 Hz*/
#define LSM6DS3_XL_ODR_52HZ                             ((uint8_t)0x30) /*!< Output Data Rate: 52 Hz */
#define LSM6DS3_XL_ODR_104HZ                            ((uint8_t)0x40) /*!< Output Data Rate: 104 Hz */
#define LSM6DS3_XL_ODR_208HZ                            ((uint8_t)0x50) /*!< Output Data Rate: 208 Hz */
#define LSM6DS3_XL_ODR_416HZ                            ((uint8_t)0x60) /*!< Output Data Rate: 416 Hz */
#define LSM6DS3_XL_ODR_833HZ                            ((uint8_t)0x70) /*!< Output Data Rate: 833 Hz */
#define LSM6DS3_XL_ODR_1660HZ                           ((uint8_t)0x80) /*!< Output Data Rate: 1.66 kHz */
#define LSM6DS3_XL_ODR_3330HZ                           ((uint8_t)0x90) /*!< Output Data Rate: 3.33 kHz */
#define LSM6DS3_XL_ODR_6660HZ                           ((uint8_t)0xA0) /*!< Output Data Rate: 6.66 kHz */

#define LSM6DS3_XL_ODR_MASK                             ((uint8_t)0xF0)

/**
 * @}
 */


/** @addtogroup LSM6DS3_XG_Accelerometer_Full_Scale_Selection_CTRL1_XL LSM6DS3_XG_Accelerometer_Full_Scale_Selection_CTRL1_XL
 * @{
 */
#define LSM6DS3_XL_FS_2G                                ((uint8_t)0x00) /*!< Full scale: +- 2g */
#define LSM6DS3_XL_FS_4G                                ((uint8_t)0x08) /*!< Full scale: +- 4g */
#define LSM6DS3_XL_FS_8G                                ((uint8_t)0x0C) /*!< Full scale: +- 8g */
#define LSM6DS3_XL_FS_16G                               ((uint8_t)0x04) /*!< Full scale: +- 16g */

#define LSM6DS3_XL_FS_MASK                              ((uint8_t)0x0C)

/**
 * @}
 */


/** @addtogroup LSM6DS3_XG_Accelerometer_Anti_Aliasing_Filter_Bandwidth_Selection_CTRL1_XL LSM6DS3_XG_Accelerometer_Anti_Aliasing_Filter_Bandwidth_Selection_CTRL1_XL
 * @{
 */
#define LSM6DS3_XL_BW_400HZ                             ((uint8_t)0x00) /*!< Anti-aliasing filter bandwidht: 400 Hz */
#define LSM6DS3_XL_BW_200HZ                             ((uint8_t)0x01) /*!< Anti-aliasing filter bandwidht: 200 Hz */
#define LSM6DS3_XL_BW_100HZ                             ((uint8_t)0x02) /*!< Anti-aliasing filter bandwidht: 100 Hz */
#define LSM6DS3_XL_BW_50HZ                              ((uint8_t)0x03) /*!< Anti-aliasing filter bandwidht: 50 Hz */

#define LSM6DS3_XL_BW_MASK                              ((uint8_t)0x03)

/**
 * @}
 */


/** @addtogroup LSM6DS3_XG_Accelerometer_Z_Axis_Output_Enable_Selection_CTRL9_XL LSM6DS3_XG_Accelerometer_Z_Axis_Output_Enable_Selection_CTRL9_XL
 * @{
 */
#define LSM6DS3_XL_ZEN_DISABLE                          ((uint8_t)0x00) /*!< Accelerometer’s Z-axis output enable: disable */
#define LSM6DS3_XL_ZEN_ENABLE                           ((uint8_t)0x20) /*!< Accelerometer’s Z-axis output enable: enable */

#define LSM6DS3_XL_ZEN_MASK                             ((uint8_t)0x20)

/**
 * @}
 */


/** @addtogroup LSM6DS3_XG_Accelerometer_Y_Axis_Output_Enable_Selection_CTRL9_XL LSM6DS3_XG_Accelerometer_Y_Axis_Output_Enable_Selection_CTRL9_XL
 * @{
 */
#define LSM6DS3_XL_YEN_DISABLE                          ((uint8_t)0x00) /*!< Accelerometer’s Y-axis output enable: disable */
#define LSM6DS3_XL_YEN_ENABLE                           ((uint8_t)0x10) /*!< Accelerometer’s Y-axis output enable: enable */

#define LSM6DS3_XL_YEN_MASK                             ((uint8_t)0x10)

/**
 * @}
 */


/** @addtogroup LSM6DS3_XG_Accelerometer_X_Axis_Output_Enable_Selection_CTRL9_XL LSM6DS3_XG_Accelerometer_X_Axis_Output_Enable_Selection_CTRL9_XL
 * @{
 */
#define LSM6DS3_XL_XEN_DISABLE                          ((uint8_t)0x00) /*!< Accelerometer’s X-axis output enable: disable */
#define LSM6DS3_XL_XEN_ENABLE                           ((uint8_t)0x08) /*!< Accelerometer’s X-axis output enable: enable */

#define LSM6DS3_XL_XEN_MASK                             ((uint8_t)0x08)

/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_FF_DUR5_Selection_WAKE_UP_DUR LSM6DS3_XG_Accelerometer_FF_DUR5_Selection_WAKE_UP_DUR
 * @{
 */
#define LSM6DS3_XG_WAKE_UP_DUR_FF_DUR5_DEFAULT          ((uint8_t)0x00)

#define LSM6DS3_XG_WAKE_UP_DUR_FF_DUR5_MASK             ((uint8_t)0x80)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_WAKE_DUR_Selection_WAKE_UP_DUR LSM6DS3_XG_Accelerometer_WAKE_DUR_Selection_WAKE_UP_DUR
 * @{
 */
#define LSM6DS3_XG_WAKE_UP_DUR_WAKE_DUR_DEFAULT         ((uint8_t)0x00)

#define LSM6DS3_XG_WAKE_UP_DUR_WAKE_DUR_MASK            ((uint8_t)0x60)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_TIMER_HR_Selection_WAKE_UP_DUR LSM6DS3_XG_Accelerometer_TIMER_HR_Selection_WAKE_UP_DUR
 * @{
 */
#define LSM6DS3_XG_WAKE_UP_DUR_TIMER_HR_DEFAULT         ((uint8_t)0x00)

#define LSM6DS3_XG_WAKE_UP_DUR_TIMER_HR_MASK            ((uint8_t)0x10)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_SLEEP_DUR_Selection_WAKE_UP_DUR LSM6DS3_XG_Accelerometer_SLEEP_DUR_Selection_WAKE_UP_DUR
 * @{
 */
#define LSM6DS3_XG_WAKE_UP_DUR_SLEEP_DUR_DEFAULT        ((uint8_t)0x00)

#define LSM6DS3_XG_WAKE_UP_DUR_SLEEP_DUR_MASK           ((uint8_t)0x0F)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_FF_DUR_Selection_FREE_FALL LSM6DS3_XG_Accelerometer_FF_DUR_Selection_FREE_FALL
 * @{
 */
#define LSM6DS3_XG_WAKE_FREE_FALL_FF_DUR_DEFAULT        ((uint8_t)0x00)
#define LSM6DS3_XG_WAKE_FREE_FALL_FF_DUR_TYPICAL        ((uint8_t)0x30)

#define LSM6DS3_XG_WAKE_FREE_FALL_FF_DUR_MASK           ((uint8_t)0xF8)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_FF_THS_Selection_FREE_FALL LSM6DS3_XG_Accelerometer_FF_THS_Selection_FREE_FALL
 * @{
 */
#define LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_156MG          ((uint8_t)0x00)
#define LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_219MG          ((uint8_t)0x01)
#define LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_250MG          ((uint8_t)0x02)
#define LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_312MG          ((uint8_t)0x03)
#define LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_344MG          ((uint8_t)0x04)
#define LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_406MG          ((uint8_t)0x05)
#define LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_469MG          ((uint8_t)0x06)
#define LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_500MG          ((uint8_t)0x07)

#define LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_MASK           ((uint8_t)0x07)

/**
 * @}
 */


/** @addtogroup LSM6DS3_XG_Accelerometer_INT1_INACT_STATE_Selection_MD1_CFG LSM6DS3_XG_Accelerometer_INT1_INACT_STATE_Selection_MD1_CFG
 * @{
 */
#define LSM6DS3_XG_MD1_CFG_INT1_INACT_STATE_DISABLE     ((uint8_t)0x00)
#define LSM6DS3_XG_MD1_CFG_INT1_INACT_STATE_ENABLE      ((uint8_t)0x80)

#define LSM6DS3_XG_MD1_CFG_INT1_INACT_STATE_MASK        ((uint8_t)0x80)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_INT1_SINGLE_TAP_Selection_MD1_CFG LSM6DS3_XG_Accelerometer_INT1_SINGLE_TAP_Selection_MD1_CFG
 * @{
 */
#define LSM6DS3_XG_MD1_CFG_INT1_SINGLE_TAP_DISABLE      ((uint8_t)0x00)
#define LSM6DS3_XG_MD1_CFG_INT1_SINGLE_TAP_ENABLE       ((uint8_t)0x40)

#define LSM6DS3_XG_MD1_CFG_INT1_SINGLE_TAP_MASK         ((uint8_t)0x40)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_INT1_WU_Selection_MD1_CFG LSM6DS3_XG_Accelerometer_INT1_WU_Selection_MD1_CFG
 * @{
 */
#define LSM6DS3_XG_MD1_CFG_INT1_WU_DISABLE              ((uint8_t)0x00)
#define LSM6DS3_XG_MD1_CFG_INT1_WU_ENABLE               ((uint8_t)0x20)

#define LSM6DS3_XG_MD1_CFG_INT1_WU_MASK                 ((uint8_t)0x20)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_INT1_FF_Selection_MD1_CFG LSM6DS3_XG_Accelerometer_INT1_FF_Selection_MD1_CFG
 * @{
 */
#define LSM6DS3_XG_MD1_CFG_INT1_FF_DISABLE              ((uint8_t)0x00)
#define LSM6DS3_XG_MD1_CFG_INT1_FF_ENABLE               ((uint8_t)0x10)

#define LSM6DS3_XG_MD1_CFG_INT1_FF_MASK                 ((uint8_t)0x10)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_INT1_DOUBLE_TAP_Selection_MD1_CFG LSM6DS3_XG_Accelerometer_INT1_DOUBLE_TAP_Selection_MD1_CFG
 * @{
 */
#define LSM6DS3_XG_MD1_CFG_INT1_DOUBLE_TAP_DISABLE      ((uint8_t)0x00)
#define LSM6DS3_XG_MD1_CFG_INT1_DOUBLE_TAP_ENABLE       ((uint8_t)0x08)

#define LSM6DS3_XG_MD1_CFG_INT1_DOUBLE_TAP_MASK         ((uint8_t)0x08)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_INT1_6D_Selection_MD1_CFG LSM6DS3_XG_Accelerometer_INT1_6D_Selection_MD1_CFG
 * @{
 */
#define LSM6DS3_XG_MD1_CFG_INT1_6D_DISABLE              ((uint8_t)0x00)
#define LSM6DS3_XG_MD1_CFG_INT1_6D_ENABLE               ((uint8_t)0x04)

#define LSM6DS3_XG_MD1_CFG_INT1_6D_MASK                 ((uint8_t)0x04)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_INT1_TILT_Selection_MD1_CFG LSM6DS3_XG_Accelerometer_INT1_TILT_Selection_MD1_CFG
 * @{
 */
#define LSM6DS3_XG_MD1_CFG_INT1_TILT_DISABLE            ((uint8_t)0x00)
#define LSM6DS3_XG_MD1_CFG_INT1_TILT_ENABLE             ((uint8_t)0x02)

#define LSM6DS3_XG_MD1_CFG_INT1_TILT_MASK               ((uint8_t)0x02)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_INT1_TIMER_Selection_MD1_CFG LSM6DS3_XG_Accelerometer_INT1_TIMER_Selection_MD1_CFG
 * @{
 */
#define LSM6DS3_XG_MD1_CFG_INT1_TIMER_DISABLE           ((uint8_t)0x00)
#define LSM6DS3_XG_MD1_CFG_INT1_TIMER_ENABLE            ((uint8_t)0x01)

#define LSM6DS3_XG_MD1_CFG_INT1_TIMER_MASK              ((uint8_t)0x01)

/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_FF_IA_Enable_WAKE_UP_SRC LSM6DS3_XG_Accelerometer_FF_IA_Enable_WAKE_UP_SRC
 * @{
 */
#define LSM6DS3_XG_WAKE_UP_SRC_FF_IA_DISABLE            ((uint8_t)0x00)
#define LSM6DS3_XG_WAKE_UP_SRC_FF_IA_ENABLE             ((uint8_t)0x20)

#define LSM6DS3_XG_WAKE_UP_SRC_FF_IA_MASK               ((uint8_t)0x20)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_SLEEP_STATE_IA_Enable_WAKE_UP_SRC LSM6DS3_XG_Accelerometer_SLEEP_STATE_IA_Enable_WAKE_UP_SRC
 * @{
 */
#define LSM6DS3_XG_WAKE_UP_SRC_SLEEP_STATE_IA_DISABLE   ((uint8_t)0x00)
#define LSM6DS3_XG_WAKE_UP_SRC_SLEEP_STATE_IA_ENABLE    ((uint8_t)0x10)

#define LSM6DS3_XG_WAKE_UP_SRC_SLEEP_STATE_IA_MASK      ((uint8_t)0x10)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_WU_IA_Enable_WAKE_UP_SRC LSM6DS3_XG_Accelerometer_WU_IA_Enable_WAKE_UP_SRC
 * @{
 */
#define LSM6DS3_XG_WAKE_UP_SRC_WU_IA_DISABLE            ((uint8_t)0x00)
#define LSM6DS3_XG_WAKE_UP_SRC_WU_IA_ENABLE             ((uint8_t)0x08)

#define LSM6DS3_XG_WAKE_UP_SRC_WU_IA_MASK               ((uint8_t)0x08)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_X_WU_Enable_WAKE_UP_SRC LSM6DS3_XG_Accelerometer_X_WU_Enable_WAKE_UP_SRC
 * @{
 */
#define LSM6DS3_XG_WAKE_UP_SRC_X_WU_DISABLE             ((uint8_t)0x00)
#define LSM6DS3_XG_WAKE_UP_SRC_X_WU_ENABLE              ((uint8_t)0x04)

#define LSM6DS3_XG_WAKE_UP_SRC_X_WU_MASK                ((uint8_t)0x04)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_Y_WU_Enable_WAKE_UP_SRC LSM6DS3_XG_Accelerometer_Y_WU_Enable_WAKE_UP_SRC
 * @{
 */
#define LSM6DS3_XG_WAKE_UP_SRC_Y_WU_DISABLE             ((uint8_t)0x00)
#define LSM6DS3_XG_WAKE_UP_SRC_Y_WU_ENABLE              ((uint8_t)0x02)


#define LSM6DS3_XG_WAKE_UP_SRC_Y_WU_MASK                ((uint8_t)0x02)
/**
 * @}
 */

/** @addtogroup LSM6DS3_XG_Accelerometer_Z_WU_Enable_WAKE_UP_SRC LSM6DS3_XG_Accelerometer_Z_WU_Enable_WAKE_UP_SRC
 * @{
 */
#define LSM6DS3_XG_WAKE_UP_SRC_Z_WU_DISABLE             ((uint8_t)0x00)
#define LSM6DS3_XG_WAKE_UP_SRC_Z_WU_ENABLE              ((uint8_t)0x01)

#define LSM6DS3_XG_WAKE_UP_SRC_Z_WU_MASK                ((uint8_t)0x01)
/**
 * @}
 */

/**
 * @}
 */

/** @addtogroup LSM6DS3_Imported_Functions LSM6DS3_Imported_Functions
 * @{
 */

/* Six axes sensor IO functions */
extern IMU_6AXES_StatusTypeDef LSM6DS3_IO_Init( void );
extern IMU_6AXES_StatusTypeDef LSM6DS3_IO_Write( uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite );
extern IMU_6AXES_StatusTypeDef LSM6DS3_IO_Read( uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead );
extern void LSM6DS3_IO_ITConfig( void );

/**
 * @}
 */

/** @addtogroup LSM6DS3_Exported_Types LSM6DS3_Exported_Types
  * @{
  */
/**
 * @brief  LSM6DS3 driver extended internal structure definition
 */
typedef struct
{
  IMU_6AXES_StatusTypeDef (*Enable_Free_Fall_Detection) (void);
  IMU_6AXES_StatusTypeDef (*Disable_Free_Fall_Detection) (void);
  IMU_6AXES_StatusTypeDef (*Get_Status_Free_Fall_Detection) (uint8_t *);
} LSM6DS3_DrvExtTypeDef;

/**
 * @}
 */

/** @addtogroup LSM6DS3_Exported_Variables LSM6DS3_Exported_Variables
 * @{
 */

/* Six axes sensor driver structure */
extern IMU_6AXES_DrvTypeDef LSM6DS3Drv;
extern IMU_6AXES_DrvExtTypeDef LSM6DS3Drv_ext;

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __LSM6DS3_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
