/**
 ******************************************************************************
 * @file    lsm6ds0.h
 * @author  MEMS Application Team
 * @version V1.3.0
 * @date    28-May-2015
 * @brief   This file contains definitions for the lsm6ds0.c
 *          firmware driver.
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
#ifndef __LSM6DS0_H
#define __LSM6DS0_H

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

/** @addtogroup LSM6DS0
 * @{
 */

/** @defgroup LSM6DS0_Exported_Defines LSM6DS0_Exported_Defines
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
 * @brief Interrupt config register
 * \code
 * Read/write
 * Default value: 0x00
 * [7] INT_IG_G:    Gyroscope interrupt enable on INT pin
 * [6] INT_IG_XL:   Accelerometer interrupt generator on INT pin
 * [5] INT_FSS5:    FSS5 interrupt enable on INT pin
 * [4] INT_OVR:     Overrun interrupt on INT pin
 * [3] INT_FTH:     Gyroscope interrupt enable on INT pin
 * [2] INT_BOOT:    Accelerometer interrupt generator on INT pin
 * [1] INT_DRDY_G:  FSS5 interrupt enable on INT pin
 * [0] INT_DRDY_XL: Overrun interrupt on INT pin
 * \endcode
 */
#define LSM6DS0_XG_INT_CTRL                                 0x0C


/**
  * @brief Device identifier register.
  * \code
  * Read
  * Default value:
  * [7:0] This read-only register contains the device identifier
  * \endcode
*/
#define LSM6DS0_XG_WHO_AM_I_ADDR                            0x0F


/**
  * @brief Control Register 4
  * \code
  * Read/write
  * Default value: 0x38
  * [5] Zen_G: Gyroscope’s Z-axis output enable
  * [4] Yen_G: Gyroscope’s Y-axis output enable
  * [3] Xen_G: Gyroscope’s X-axis output enable
  * \endcode
*/
#define LSM6DS0_XG_CTRL_REG4                                0x1E


/**
  * @brief Control Register 10
  * \code
  * Read/write
  * Default value: 0x00
  * [2] ST_G:  Gyro  selftest disable (0) / enable (1)
  * [0] ST_XL: Accel selftest disable (0) / enable (1)
  * \endcode
*/
#define LSM6DS0_XG_CTRL_REG10                               0x24


/***************************************** GYROSCOPE REGISTERS ********************************************/

/**
 * @brief Angular rate sensor Control Register 1
 * \code
 * Read/write
 * Default value: 0x00
 * [7:5] ODR_G2-0: Gyroscope output data rate selection
 * [4:3] FS_G1-0: Gyroscope full-scale selection
 * [2] This bit must be set to ‘0’ for the correct operation of the device
 * [1:0] BW_G1-0: Gyroscope bandwidth selection
 * \endcode
 */
#define LSM6DS0_XG_CTRL_REG1_G                              0x10


/**
 * @brief Gyroscope data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS0_XG_OUT_X_L_G                                0x18


/**
 * @brief Gyroscope data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS0_XG_OUT_X_H_G                                0x19


/**
 * @brief Gyroscope data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS0_XG_OUT_Y_L_G                                0x1A


/**
 * @brief Gyroscope data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS0_XG_OUT_Y_H_G                                0x1B


/**
 * @brief Gyroscope data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS0_XG_OUT_Z_L_G                                0x1C


/**
 * @brief Gyroscope data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS0_XG_OUT_Z_H_G                                0x1D



/*************************************** ACCELEROMETER REGISTERS *******************************************/

/**
 * @brief Linear acceleration sensor Control Register 6
 * \code
 * Read/write
 * Default value: 0x00
 * [7:5] ODR_XL2-0: Accelerometer Output data rate and power mode selection
 * [4:3] FS1_XL-FS0_XL: Accelerometer full-scale selection
 * [2] BW_SCAL_ODR: Bandwidth selection
 * [1:0] BW_XL1-0: Anti-aliasing filter bandwidth selection
 * \endcode
 */
#define LSM6DS0_XG_CTRL_REG6_XL                              0x20


/**
 * @brief Linear acceleration sensor Control Register 5
 * \code
 * Read/write
 * Default value: 0x38
 * [7:6] DEC1-0: Decimation of acceleration data on OUT REG and FIFO
 * [5] Zen_XL: Accelerometer’s Z-axis output enable
 * [4] Yen_XL: Accelerometer’s Y-axis output enable
 * [3] Xen_XL: Accelerometer’s X-axis output enable
 * [2:0] These bits must be set to ‘0’ for the correct operation of the device
 * \endcode
 */
#define LSM6DS0_XG_CTRL_REG5_XL                              0x1F


/**
 * @brief Accelerometer data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS0_XG_OUT_X_L_XL                                0x28


/**
 * @brief Accelerometer data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS0_XG_OUT_X_H_XL                                0x29


/**
 * @brief Accelerometer data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS0_XG_OUT_Y_L_XL                                0x2A


/**
 * @brief Accelerometer data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS0_XG_OUT_Y_H_XL                                0x2B


/**
 * @brief Accelerometer data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS0_XG_OUT_Z_L_XL                                0x2C


/**
 * @brief Accelerometer data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LSM6DS0_XG_OUT_Z_H_XL                                0x2D

/******************************************************************************/
/************* END ACCELEROMETER AND GYROSCOPE REGISTER MAPPING  **************/
/******************************************************************************/

/**
 * @brief Multiple Byte. Mask for enabling multiple byte read/write command.
 */
#define LSM6DS0_I2C_MULTIPLEBYTE_CMD                      ((uint8_t)0x80)

/**
* @brief Device Address
*/
#define LSM6DS0_ADDRESS_LOW                                 0xD4    // SAD[0] = 0
#define LSM6DS0_ADDRESS_HIGH                                0xD6    // SAD[0] = 1
#define LSM6DS0_XG_MEMS_ADDRESS                             LSM6DS0_ADDRESS_HIGH    // SAD[0] = 1

/**
 * @brief Device Identifier. Default value of the WHO_AM_I register.
 */
#define I_AM_LSM6DS0_XG                                 ((uint8_t)0x68)



/************************************** GYROSCOPE REGISTERS VALUE *******************************************/


/** @defgroup LSM6DS0_XG_Gyroscope_Output_Data_Rate_Selection_CTRL_REG1_G LSM6DS0_XG_Gyroscope_Output_Data_Rate_Selection_CTRL_REG1_G
 * @{
 */
#define LSM6DS0_G_ODR_PD                                ((uint8_t)0x00) /*!< Output Data Rate: Power-down*/
#define LSM6DS0_G_ODR_14_9HZ                            ((uint8_t)0x20) /*!< Output Data Rate: 14.9 Hz, cutoff 5Hz */
#define LSM6DS0_G_ODR_59_5HZ                            ((uint8_t)0x40) /*!< Output Data Rate: 59.5 Hz, cutoff 19Hz */
#define LSM6DS0_G_ODR_119HZ                             ((uint8_t)0x60) /*!< Output Data Rate: 119 Hz, cutoff 38Hz*/
#define LSM6DS0_G_ODR_238HZ                             ((uint8_t)0x80) /*!< Output Data Rate: 238 Hz, cutoff 76Hz*/
#define LSM6DS0_G_ODR_476HZ                             ((uint8_t)0xA0) /*!< Output Data Rate: 476 Hz, cutoff 100Hz*/
#define LSM6DS0_G_ODR_952HZ                             ((uint8_t)0xC0) /*!< Output Data Rate: 952 Hz, cutoff 100Hz*/

#define LSM6DS0_G_ODR_MASK                              ((uint8_t)0xE0)
/**
 * @}
 */


/** @defgroup LSM6DS0_XG_Gyroscope_Bandwidth_Selection_CTRL_REG1_G LSM6DS0_XG_Gyroscope_Bandwidth_Selection_CTRL_REG1_G
 * @{
 */
#define LSM6DS0_G_BW_00                          ((uint8_t)0x00) /*!< Bandwidth selection:  - cutoff = n.a. when ODR = Power-down
                                                                                            - cutoff = n.a. when ODR = 14.9
                                                                                            - cutoff = 16 when ODR = 59.5
                                                                                            - cutoff = 14 when ODR = 119
                                                                                            - cutoff = 14 when ODR = 238
                                                                                            - cutoff = 21 when ODR = 476
                                                                                            - cutoff = 33 when ODR = 952  */
#define LSM6DS0_G_BW_01                          ((uint8_t)0x01) /*!< Bandwidth selection:  - cutoff = n.a. when ODR = Power-down
                                                                                            - cutoff = n.a. when ODR = 14.9
                                                                                            - cutoff = 16 when ODR = 59.5
                                                                                            - cutoff = 31 when ODR = 119
                                                                                            - cutoff = 29 when ODR = 238
                                                                                            - cutoff = 28 when ODR = 476
                                                                                            - cutoff = 40 when ODR = 952  */
#define LSM6DS0_G_BW_10                          ((uint8_t)0x02) /*!< Bandwidth selection:  - cutoff = n.a. when ODR = Power-down
                                                                                            - cutoff = n.a. when ODR = 14.9
                                                                                            - cutoff = 16 when ODR = 59.5
                                                                                            - cutoff = 31 when ODR = 119
                                                                                            - cutoff = 63 when ODR = 238
                                                                                            - cutoff = 57 when ODR = 476
                                                                                            - cutoff = 58 when ODR = 952  */
#define LSM6DS0_G_BW_11                          ((uint8_t)0x03) /*!< Bandwidth selection:  - cutoff = n.a. when ODR = Power-down
                                                                                            - cutoff = n.a. when ODR = 14.9
                                                                                            - cutoff = 16 when ODR = 59.5
                                                                                            - cutoff = 31 when ODR = 119
                                                                                            - cutoff = 78 when ODR = 238
                                                                                            - cutoff = 100 when ODR = 476
                                                                                            - cutoff = 100 when ODR = 952  */

#define LSM6DS0_G_BW_MASK                              ((uint8_t)0x03)
/**
 * @}
 */

/** @defgroup LSM6DS0_XG_Gyroscope_Full_Scale_Selection_CTRL_REG1_G LSM6DS0_XG_Gyroscope_Full_Scale_Selection_CTRL_REG1_G
 * @{
 */
#define LSM6DS0_G_FS_245                               ((uint8_t)0x00) /*!< Full scale: 245 dps*/
#define LSM6DS0_G_FS_500                               ((uint8_t)0x08) /*!< Full scale: 500 dps */
#define LSM6DS0_G_FS_2000                              ((uint8_t)0x18) /*!< Full scale: 2000 dps */

#define LSM6DS0_G_FS_MASK                              ((uint8_t)0x18)
/**
 * @}
 */

/** @defgroup LSM6DS0_XG_Gyroscope_Z_Axis_Output_Enable_Selection_CTRL_REG4 LSM6DS0_XG_Gyroscope_Z_Axis_Output_Enable_Selection_CTRL_REG4
 * @{
 */
#define LSM6DS0_G_ZEN_DISABLE                          ((uint8_t)0x00) /*!< Gyroscope’s Z-axis output enable: disable */
#define LSM6DS0_G_ZEN_ENABLE                           ((uint8_t)0x20) /*!< Gyroscope’s Z-axis output enable: enable */

#define LSM6DS0_G_ZEN_MASK                             ((uint8_t)0x20)
/**
 * @}
 */

/** @defgroup LSM6DS0_XG_Gyroscope_Y_Axis_Output_Enable_Selection_CTRL_REG4 LSM6DS0_XG_Gyroscope_Y_Axis_Output_Enable_Selection_CTRL_REG4
 * @{
 */
#define LSM6DS0_G_YEN_DISABLE                          ((uint8_t)0x00) /*!< Gyroscope’s Y-axis output enable: disable */
#define LSM6DS0_G_YEN_ENABLE                           ((uint8_t)0x10) /*!< Gyroscope’s Y-axis output enable: enable */

#define LSM6DS0_G_YEN_MASK                             ((uint8_t)0x10)
/**
 * @}
 */

/** @defgroup LSM6DS0_XG_Gyroscope_X_Axis_Output_Enable_Selection_CTRL_REG4 LSM6DS0_XG_Gyroscope_X_Axis_Output_Enable_Selection_CTRL_REG4
 * @{
 */
#define LSM6DS0_G_XEN_DISABLE                          ((uint8_t)0x00) /*!< Gyroscope’s X-axis output enable: disable */
#define LSM6DS0_G_XEN_ENABLE                           ((uint8_t)0x08) /*!< Gyroscope’s X-axis output enable: enable */

#define LSM6DS0_G_XEN_MASK                             ((uint8_t)0x08)
/**
 * @}
 */

/** @defgroup LSM6DS0_XG_Gyroscope_Selftest_Enable_Selection_CTRL_REG10 LSM6DS0_XG_Gyroscope_Selftest_Enable_Selection_CTRL_REG10
 * @{
 */
#define LSM6DS0_G_ST_DISABLE                            ((uint8_t)0x00) /*!< Gyro selftest disable */
#define LSM6DS0_G_ST_ENABLE                             ((uint8_t)0x04) /*!< Gyro selftest enable */

#define LSM6DS0_G_ST_MASK                               ((uint8_t)0x04)
/**
 * @}
 */


/************************************ ACCELEROMETER REGISTERS VALUE *****************************************/

/** @defgroup LSM6DS0_XG_Accelerometer_Output_Data_Rate_Selection_CTRL_REG6_XL LSM6DS0_XG_Accelerometer_Output_Data_Rate_Selection_CTRL_REG6_XL
 * @{
 */
#define LSM6DS0_XL_ODR_PD                               ((uint8_t)0x00) /*!< Output Data Rate: Power-down*/
#define LSM6DS0_XL_ODR_10HZ                             ((uint8_t)0x20) /*!< Output Data Rate: 10 Hz*/
#define LSM6DS0_XL_ODR_50HZ                             ((uint8_t)0x40) /*!< Output Data Rate: 50 Hz */
#define LSM6DS0_XL_ODR_119HZ                            ((uint8_t)0x60) /*!< Output Data Rate: 119 Hz */
#define LSM6DS0_XL_ODR_238HZ                            ((uint8_t)0x80) /*!< Output Data Rate: 238 Hz */
#define LSM6DS0_XL_ODR_476HZ                            ((uint8_t)0xA0) /*!< Output Data Rate: 476 Hz */
#define LSM6DS0_XL_ODR_952HZ                            ((uint8_t)0xC0) /*!< Output Data Rate: 952 Hz */

#define LSM6DS0_XL_ODR_MASK                             ((uint8_t)0xE0)
/**
 * @}
 */

/** @defgroup LSM6DS0_XG_Accelerometer_Full_Scale_Selection_CTRL_REG6_XL LSM6DS0_XG_Accelerometer_Full_Scale_Selection_CTRL_REG6_XL
 * @{
 */
#define LSM6DS0_XL_FS_2G                                ((uint8_t)0x00) /*!< Full scale: +- 2g */
#define LSM6DS0_XL_FS_4G                                ((uint8_t)0x10) /*!< Full scale: +- 4g */
#define LSM6DS0_XL_FS_8G                                ((uint8_t)0x18) /*!< Full scale: +- 8g */
#define LSM6DS0_XL_FS_16G                               ((uint8_t)0x08) /*!< Full scale: +- 16g */

#define LSM6DS0_XL_FS_MASK                              ((uint8_t)0x18)
/**
 * @}
 */

/** @defgroup LSM6DS0_XG_Accelerometer_Bandwidth_Selection_CTRL_REG6_XL LSM6DS0_XG_Accelerometer_Bandwidth_Selection_CTRL_REG6_XL
 * @{
 */
#define LSM6DS0_XL_BW_SCAL_ODR                          ((uint8_t)0x00) /*!< Bandwidth selection: determined by ODR:
                                                                                                  - BW = 408Hz when ODR = 952Hz, 50Hz, 10Hz
                                                                                                  - BW = 211Hz when ODR = 476Hz
                                                                                                  - BW = 105Hz when ODR = 238Hz
                                                                                                  - BW = 50Hz when ODR = 119Hz */
#define LSM6DS0_XL_BW_SCAL_BW                           ((uint8_t)0x04) /*!< Bandwidth selection: selected according to Anti aliasing filter bandwidth */

#define LSM6DS0_XL_BW_SCAL_MASK                         ((uint8_t)0x04)
/**
 * @}
 */


/** @defgroup LSM6DS0_XG_Accelerometer_Anti_Aliasing_Filter_Bandwidth_Selection_CTRL_REG6_XL LSM6DS0_XG_Accelerometer_Anti_Aliasing_Filter_Bandwidth_Selection_CTRL_REG6_XL
 * @{
 */
#define LSM6DS0_XL_BW_408HZ                             ((uint8_t)0x00) /*!< Anti-aliasing filter bandwidht: 408 Hz */
#define LSM6DS0_XL_BW_211HZ                             ((uint8_t)0x01) /*!< Anti-aliasing filter bandwidht: 211 Hz */
#define LSM6DS0_XL_BW_105HZ                             ((uint8_t)0x02) /*!< Anti-aliasing filter bandwidht: 105 Hz */
#define LSM6DS0_XL_BW_50HZ                              ((uint8_t)0x03) /*!< Anti-aliasing filter bandwidht: 50 Hz */

#define LSM6DS0_XL_BW_MASK                              ((uint8_t)0x03)
/**
 * @}
 */

/** @defgroup LSM6DS0_XG_Accelerometer_Decimation_Acceleration_Data_Selection_CTRL_REG5_XL LSM6DS0_XG_Accelerometer_Decimation_Acceleration_Data_Selection_CTRL_REG5_XL
 * @{
 */
#define LSM6DS0_XL_DEC_NO                               ((uint8_t)0x00) /*!< Decimation of acceleration data: no decimation */
#define LSM6DS0_XL_DEC_EVERY_2S                         ((uint8_t)0x40) /*!< Decimation of acceleration data: update every 2 samples */
#define LSM6DS0_XL_DEC_EVERY_4S                         ((uint8_t)0x80) /*!< Decimation of acceleration data: update every 4 samples */
#define LSM6DS0_XL_DEC_EVERY_8S                         ((uint8_t)0xC0) /*!< Decimation of acceleration data: update every 8 samples */

#define LSM6DS0_XL_DEC_MASK                             ((uint8_t)0xC0)
/**
 * @}
 */


/** @defgroup LSM6DS0_XG_Accelerometer_Z_Axis_Output_Enable_Selection_CTRL_REG5_XL LSM6DS0_XG_Accelerometer_Z_Axis_Output_Enable_Selection_CTRL_REG5_XL
 * @{
 */
#define LSM6DS0_XL_ZEN_DISABLE                          ((uint8_t)0x00) /*!< Accelerometer’s Z-axis output enable: disable */
#define LSM6DS0_XL_ZEN_ENABLE                           ((uint8_t)0x20) /*!< Accelerometer’s Z-axis output enable: enable */

#define LSM6DS0_XL_ZEN_MASK                             ((uint8_t)0x20)
/**
 * @}
 */

/** @defgroup LSM6DS0_XG_Accelerometer_Y_Axis_Output_Enable_Selection_CTRL_REG5_XL LSM6DS0_XG_Accelerometer_Y_Axis_Output_Enable_Selection_CTRL_REG5_XL
 * @{
 */
#define LSM6DS0_XL_YEN_DISABLE                          ((uint8_t)0x00) /*!< Accelerometer’s Y-axis output enable: disable */
#define LSM6DS0_XL_YEN_ENABLE                           ((uint8_t)0x10) /*!< Accelerometer’s Y-axis output enable: enable */

#define LSM6DS0_XL_YEN_MASK                             ((uint8_t)0x10)
/**
 * @}
 */


/** @defgroup LSM6DS0_XG_Accelerometer_X_Axis_Output_Enable_Selection_CTRL_REG5_XL LSM6DS0_XG_Accelerometer_X_Axis_Output_Enable_Selection_CTRL_REG5_XL
 * @{
 */
#define LSM6DS0_XL_XEN_DISABLE                          ((uint8_t)0x00) /*!< Accelerometer’s X-axis output enable: disable */
#define LSM6DS0_XL_XEN_ENABLE                           ((uint8_t)0x08) /*!< Accelerometer’s X-axis output enable: enable */

#define LSM6DS0_XL_XEN_MASK                             ((uint8_t)0x08)

/**
 * @}
 */


/** @defgroup LSM6DS0_XG_Accelerometer_Selftest_Enable_Selection_CTRL_REG10 LSM6DS0_XG_Accelerometer_Selftest_Enable_Selection_CTRL_REG10
 * @{
 */
#define LSM6DS0_XL_ST_DISABLE                           ((uint8_t)0x00) /*!< Accel selftest disable */
#define LSM6DS0_XL_ST_ENABLE                            ((uint8_t)0x01) /*!< Accel selftest enable */

#define LSM6DS0_XL_ST_MASK                              ((uint8_t)0x01)

/**
 * @}
 */

/**
 * @}
 */


/** @defgroup LSM6DS0_Imported_Functions LSM6DS0_Imported_Functions
 * @{
 */

/* Six axes sensor IO functions */
extern IMU_6AXES_StatusTypeDef LSM6DS0_IO_Init(void);
extern IMU_6AXES_StatusTypeDef LSM6DS0_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite);
extern IMU_6AXES_StatusTypeDef LSM6DS0_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead);
extern void LSM6DS0_IO_ITConfig( void );

/**
 * @}
 */

/* ------------------------------------------------------- */
/* Here you should declare the internal struct of          */
/* extended features of LIS3MDL. See the example of        */
/* LSM6DS3 in lsm6ds3.h                                    */
/* ------------------------------------------------------- */

/** @addtogroup LSM6DS0_Exported_Variables LSM6DS0_Exported_Variables
 * @{
 */

/* Six axes sensor driver structure */
extern IMU_6AXES_DrvTypeDef LSM6DS0Drv;
extern IMU_6AXES_DrvExtTypeDef LSM6DS0Drv_ext;

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

#endif /* __LSM6DS0_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
