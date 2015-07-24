/**
 ******************************************************************************
 * @file    lis3mdl.h
 * @author  MEMS Application Team
 * @version V1.3.0
 * @date    28-May-2015
 * @brief   This file contains definitions for the lis3mdl.c
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
#ifndef __LIS3MDL_H
#define __LIS3MDL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "magneto.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Components
 * @{
 */

/** @addtogroup LIS3MDL
 * @{
 */

/** @defgroup LIS3MDL_Exported_Defines LIS3MDL_Exported_Defines
 * @{
 */
#ifndef NULL
#define NULL      (void *) 0
#endif


/******************************************************************************/
/***************** START MAGNETIC SENSOR REGISTER MAPPING  ********************/
/******************************************************************************/

/**
  * @brief Device identifier register.
  * \code
  * Read
  * Default value:
  * 7:0 This read-only register contains the device identifier
  * \endcode
*/
#define LIS3MDL_M_WHO_AM_I_ADDR                             0x0F


/**
 * @brief Magnetic sensor Control Register 1
 * \code
 * Read/write
 * Default value: 0x10
 * [7] TEMP_COMP: Temperature compensation enable
 * [6:5] OM1-0: X and Y axes operative mode selection
 * [4:2] DO2-0: Output data rate selection
 * [1] This bit must be set to ‘0’ for the correct operation of the device
 * [0] ST: Self-test enable
 * \endcode
 */
#define LIS3MDL_M_CTRL_REG1_M                               0x20


/**
 * @brief Magnetic sensor Control Register 2
 * \code
 * Read/write
 * Default value: 0x00
 * [7] These bits must be set to ‘0’ for the correct operation of the device
 * [6:5] FS1-0: Full-scale configuration
 * [4] These bits must be set to ‘0’ for the correct operation of the device
 * [3] REBOOT: Reboot memory content
 * [2] SOFT_RST: Configuration registers and user register reset function
 * [1:0] These bits must be set to ‘0’ for the correct operation of the device
 * \endcode
 */
#define LIS3MDL_M_CTRL_REG2_M                               0x21


/**
 * @brief Magnetic sensor Control Register 3
 * \code
 * Read/write
 * Default value: 0x03
 * [7] I2C_DISABLE: Disable I2C interface
 * [6] These bits must be set to ‘0’ for the correct operation of the device
 * [5] LP: Low-power mode configuration
 * [4:3] These bits must be set to ‘0’ for the correct operation of the device
 * [2] SIM: SPI Serial Interface mode selection
 * [1:0] MD1-0: Operating mode selection
 * \endcode
 */
#define LIS3MDL_M_CTRL_REG3_M                               0x22


/**
 * @brief Magnetic sensor data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LIS3MDL_M_OUT_X_L_M                                 0x28


/**
 * @brief Magnetic sensor data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LIS3MDL_M_OUT_X_H_M                                  0x29


/**
 * @brief Magnetic sensor data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LIS3MDL_M_OUT_Y_L_M                                  0x2A


/**
 * @brief Magnetic sensor data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LIS3MDL_M_OUT_Y_H_M                                  0x2B


/**
 * @brief Magnetic sensor data (LSB)
 * \code
 * Read
 * \endcode
 */
#define LIS3MDL_M_OUT_Z_L_M                                  0x2C


/**
 * @brief Magnetic sensor data (MSB)
 * \code
 * Read
 * \endcode
 */
#define LIS3MDL_M_OUT_Z_H_M                                  0x2D


/**
 * @brief Magnetic sensor Interrupt config register
 * \code
 * Read/write
 * Default value: 0x00
 * [7] XIEN: Enable interrupt generation on X axis
 * [6] YIEN: Enable interrupt generation on Y axis
 * [5] ZIEN: Enable interrupt generation on Z axis
 * [4:3] Must be 0
 * [2] IEA: Interrupt active configuration on INT
 * [1] LIR: Latch interrupt request
 * [0] IEN: Interrupt enable on INT pin
 * \endcode
 */
#define LIS3MDL_M_INT_CFG                                   0x30


/**
 * @brief Magnetic sensor Interrupt source register
 * \code
 * Read/write
 * Default value: 0x00
 * [7] PTH_X: Value on X-axis exceeds the threshold on the positive side
 * [6] PTH_Y: Value on Y-axis exceeds the threshold on the positive side
 * [5] PTH_Z: Value on Z-axis exceeds the threshold on the positive side
 * [4] NTH_X: Value on X-axis exceeds the threshold on the negative side
 * [3] NTH_Y: Value on Y-axis exceeds the threshold on the negative side
 * [2] NTH_Z: Value on Z-axis exceeds the threshold on the negative side
 * [1] MROI: Internal measurement range overflow on magnetic value
 * [0] INT: This bit signals when interrupt event occours
 * \endcode
 */
#define LIS3MDL_M_INT_SRC                                   0x31


/**
 * @brief Magnetic sensor Interrupt threshold register low
 * \code
 * Read/write
 * Default value: 0x00
 * [7:0] THS7-0: Least 8 significant bits of interrupt threshold
 * \endcode
 */
#define LIS3MDL_M_INT_THS_L_M                               0x32


/**
 * @brief Magnetic sensor Interrupt threshold register high
 * \code
 * Read/write
 * Default value: 0x00
 * [7] Must be 0
 * [6:0] THS14-8: Most 7 significant bits of interrupt threshold
 * \endcode
 */
#define LIS3MDL_M_INT_THS_H_M                               0x33

/******************************************************************************/
/******************* END MAGNETIC SENSOR REGISTER MAPPING  ********************/
/******************************************************************************/

/**
 * @brief Multiple Byte. Mask for enabling multiple byte read/write command.
 */
#define LIS3MDL_I2C_MULTIPLEBYTE_CMD                      ((uint8_t)0x80)

/**
 * @brief Device Address
 */

#define LIS3MDL_M_MEMS_ADDRESS                              0x3C    // SAD[1] = 1

/**
 * @brief Device Identifier. Default value of the WHO_AM_I register.
 */
#define I_AM_LIS3MDL_M                                  ((uint8_t)0x3D)


/*********************************** MAGNETIC SENSOR REGISTERS VALUE ****************************************/

/** @defgroup LIS3MDL_M_Temperature_Compensation_Enable_Selection_CTRL_REG1_M LIS3MDL_M_Temperature_Compensation_Enable_Selection_CTRL_REG1_M
 * @{
 */
#define LIS3MDL_M_TEMP_COMP_DISABLE                     ((uint8_t)0x00) /*!< Temperature compensation: disable */
#define LIS3MDL_M_TEMP_COMP_ENABLE                      ((uint8_t)0x80) /*!< Temperature compensation: enable */

#define LIS3MDL_M_TEMP_COMP_MASK                        ((uint8_t)0x80)
/**
  * @}
  */

/** @defgroup LIS3MDL_M_X_And_Y_Axes_Operative_Mode_Selection_CTRL_REG1_M LIS3MDL_M_X_And_Y_Axes_Operative_Mode_Selection_CTRL_REG1_M
 * @{
 */
#define LIS3MDL_M_OM_LP                                 ((uint8_t)0x00) /*!< X and Y axes operative mode: Low-power mode */
#define LIS3MDL_M_OM_MP                                 ((uint8_t)0x20) /*!< X and Y axes operative mode: Medium-performance mode */
#define LIS3MDL_M_OM_HP                                 ((uint8_t)0x40) /*!< X and Y axes operative mode: High-performance mode */
#define LIS3MDL_M_OM_UHP                                ((uint8_t)0x60) /*!< X and Y axes operative mode: Ultra-high performance mode */

#define LIS3MDL_M_OM_MASK                               ((uint8_t)0x60)
/**
  * @}
  */

/** @defgroup LIS3MDL_M_Output_Data_Rate_Selection_CTRL_REG1_M LIS3MDL_M_Output_Data_Rate_Selection_CTRL_REG1_M
 * @{
 */
#define LIS3MDL_M_DO_0_625                              ((uint8_t)0x00) /*!< Output data rate selection: 0.625 */
#define LIS3MDL_M_DO_1_25                               ((uint8_t)0x04) /*!< Output data rate selection: 1.25 */
#define LIS3MDL_M_DO_2_5                                ((uint8_t)0x08) /*!< Output data rate selection: 2.5 */
#define LIS3MDL_M_DO_5                                  ((uint8_t)0x0C) /*!< Output data rate selection: 5 */
#define LIS3MDL_M_DO_10                                 ((uint8_t)0x10) /*!< Output data rate selection: 10 */
#define LIS3MDL_M_DO_20                                 ((uint8_t)0x14) /*!< Output data rate selection: 20 */
#define LIS3MDL_M_DO_40                                 ((uint8_t)0x18) /*!< Output data rate selection: 40 */
#define LIS3MDL_M_DO_80                                 ((uint8_t)0x1C) /*!< Output data rate selection: 80 */

#define LIS3MDL_M_DO_MASK                               ((uint8_t)0x1C)
/**
  * @}
  */

/** @defgroup LIS3MDL_M_Self_Test_Enable_Selection_CTRL_REG1_M LIS3MDL_M_Self_Test_Enable_Selection_CTRL_REG1_M
 * @{
 */
#define LIS3MDL_M_ST_DISABLE                            ((uint8_t)0x00) /*!< Self-test: disable */
#define LIS3MDL_M_ST_ENABLE                             ((uint8_t)0x01) /*!< Self-test: enable */

#define LIS3MDL_M_ST_MASK                               ((uint8_t)0x01)
/**
  * @}
  */

/** @defgroup LIS3MDL_M_Full_Scale_Selection_CTRL_REG2_M LIS3MDL_M_Full_Scale_Selection_CTRL_REG2_M
 * @{
 */
#define LIS3MDL_M_FS_4                                  ((uint8_t)0x00) /*!< Full scale: +-4 guass */
#define LIS3MDL_M_FS_8                                  ((uint8_t)0x20) /*!< Full scale: +-8 gauss */
#define LIS3MDL_M_FS_12                                 ((uint8_t)0x40) /*!< Full scale: +-12 gauss */
#define LIS3MDL_M_FS_16                                 ((uint8_t)0x60) /*!< Full scale: +-16 gauss */

#define LIS3MDL_M_FS_MASK                               ((uint8_t)0x60)
/**
  * @}
  */

/** @defgroup LIS3MDL_M_Reboot_Memory_Selection_CTRL_REG2_M LIS3MDL_M_Reboot_Memory_Selection_CTRL_REG2_M
 * @{
 */
#define LIS3MDL_M_REBOOT_NORMAL                         ((uint8_t)0x00) /*!< Reboot mode: normal mode */
#define LIS3MDL_M_REBOOT_MEM_CONTENT                    ((uint8_t)0x08) /*!< Reboot mode: reboot memory content */

#define LIS3MDL_M_REBOOT_MASK                           ((uint8_t)0x08)
/**
  * @}
  */

/** @defgroup LIS3MDL_M_Configuration_Registers_And_User_Register_Reset_CTRL_REG2_M LIS3MDL_M_Configuration_Registers_And_User_Register_Reset_CTRL_REG2_M
 * @{
 */
#define LIS3MDL_M_SOFT_RST_DEFAULT                      ((uint8_t)0x00) /*!< Reset function: default value */
#define LIS3MDL_M_SOFT_RST_RESET                        ((uint8_t)0x04) /*!< Reset function: reset operation */

#define LIS3MDL_M_SOFT_RST_MASK                         ((uint8_t)0x04)
/**
  * @}
  */

/** @defgroup LIS3MDL_M_Disable_I2C_Interface_Selection_CTRL_REG3_M LIS3MDL_M_Disable_I2C_Interface_Selection_CTRL_REG3_M
 * @{
 */
#define LIS3MDL_M_I2C_ENABLE                            ((uint8_t)0x00) /*!< I2C interface: enable */
#define LIS3MDL_M_I2C_DISABLE                           ((uint8_t)0x80) /*!< I2C interface: disable */

#define LIS3MDL_M_I2C_MASK                              ((uint8_t)0x80)
/**
  * @}
  */

/** @defgroup LIS3MDL_M_Low_Power_Mode_Selection_CTRL_REG3_M LIS3MDL_M_Low_Power_Mode_Selection_CTRL_REG3_M
 * @{
 */
#define LIS3MDL_M_LP_ENABLE                            ((uint8_t)0x00) /*!< Low-power mode: magnetic data rate is configured by
                                                                                            the DO bits in the CTRL_REG1_M */
#define LIS3MDL_M_LP_DISABLE                           ((uint8_t)0x20) /*!< Low-power mode: the DO bits is set to 0.625 Hz and the system performs,
                                                                                            for each channel, the minimum number of averages */

#define LIS3MDL_M_LP_MASK                              ((uint8_t)0x20)
/**
  * @}
  */

/** @defgroup LIS3MDL_M_SPI_Serial_Interface_Mode_Selection_CTRL_REG3_M LIS3MDL_M_SPI_Serial_Interface_Mode_Selection_CTRL_REG3_M
 * @{
 */
#define LIS3MDL_M_SPI_R_ENABLE                          ((uint8_t)0x00) /*!< SPI Serial Interface mode: only write operations enabled */
#define LIS3MDL_M_SPI_R_DISABLE                         ((uint8_t)0x40) /*!< SPI Serial Interface mode: read and write operations enable */

#define LIS3MDL_M_SPI_R_MASK                            ((uint8_t)0x40)
/**
  * @}
  */

/** @defgroup LIS3MDL_M_Operating_Mode_Selection_CTRL_REG3_M LIS3MDL_M_Operating_Mode_Selection_CTRL_REG3_M
 * @{
 */
#define LIS3MDL_M_MD_CONTINUOUS                         ((uint8_t)0x00) /*!< Operating mode: Continuous-conversion mode */
#define LIS3MDL_M_MD_SINGLE                             ((uint8_t)0x01) /*!< Operating mode: Single-conversion mode has to be used with sampling frequency from 0.625 Hz to 80 Hz. */
#define LIS3MDL_M_MD_PD                                 ((uint8_t)0x02) /*!< Operating mode: Power-down mode */

#define LIS3MDL_M_MD_MASK                               ((uint8_t)0x03)
/**
 * @}
 */

/**
  * @}
  */

/** @defgroup LIS3MDL_Imported_Functions LIS3MDL_Imported_Functions
 * @{
 */

/* Magneto sensor IO functions */
extern MAGNETO_StatusTypeDef LIS3MDL_IO_Init(void);
extern MAGNETO_StatusTypeDef LIS3MDL_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite);
extern MAGNETO_StatusTypeDef LIS3MDL_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead);
extern void LIS3MDL_IO_ITConfig( void );

/**
 * @}
 */

/* ------------------------------------------------------- */
/* Here you should declare the internal struct of          */
/* extended features of LSM6DS0. See the example of        */
/* LSM6DS3 in lsm6ds3.h                                    */
/* ------------------------------------------------------- */

/** @addtogroup LIS3MDL_Exported_Variables LIS3MDL_Exported_Variables
 * @{
 */
/* Magneto sensor driver structure */
extern MAGNETO_DrvTypeDef LIS3MDLDrv;
extern MAGNETO_DrvExtTypeDef LIS3MDLDrv_ext;

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

#endif /* __LIS3MDL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
