/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 *
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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-12c cc2538 i2c
 *
 * Driver for the cc2538 i2c controller
 * @{
 *
 * \file
 * Header file for the cc2538 i2c driver
 *
 * \author
 *          Adam Rea <areairs@gmail.com>
 */
#ifndef I2C_H_
#define I2C_H_

#include "contiki.h"
#include "reg.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** /name I2C port/pin configs
 * @{
 */
#ifdef I2C_CONF_SCL_PORT
#define I2C_SCL_PORT          I2C_CONF_SCL_PORT
#else
#define I2C_SCL_PORT          GPIO_B_NUM
#endif

#ifdef I2C_CONF_SCL_PIN
#define I2C_SCL_PIN           I2C_CONF_SCL_PIN
#else
#define I2C_SCL_PIN           3
#endif

#ifdef I2C_CONF_SDA_PORT
#define I2C_SDA_PORT          I2C_CONF_SDA_PORT
#else
#define I2C_SDA_PORT          GPIO_B_NUM
#endif

#ifdef I2C_CONF_SDA_PIN
#define I2C_SDA_PIN           I2C_CONF_SDA_PIN
#else 
#define I2C_SDA_PIN           1
#endif

#define I2C_SCL_PORT_BASE     GPIO_PORT_TO_BASE(I2C_SCL_PORT)
#define I2C_SDA_PORT_BASE     GPIO_PORT_TO_BASE(I2C_SDA_PORT)


/** @} */
/*---------------------------------------------------------------------------*/
/**
 * /name I2C defines for the I2C master register offsets.
 *
 * @{ 
 */
#define I2CM_SA                 0x40020000 /**< I2C master slave address reg */
#define I2CM_CTRL               0x40020004 /**< I2C master control and status reg */
#define I2CM_STAT               0x40020004 /**< I2C master status register */
#define I2CM_DR                 0x40020008 /**< I2C data */
#define I2CM_TPR                0x4002000C /**< I2C timer period */
#define I2CM_IMR                0x40020010 /**< I2C interrupt mask */
#define I2CM_RIS                0x40020014 /**< I2C master raw interrupt status */
#define I2CM_MIS                0x40020018 /**< I2C master masked interrupt status */
#define I2CM_ICR                0x4002001C /**< I2C master interrupt clear */
#define I2CM_CR                 0x40020020 /**< I2C master configuration register */
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * \name defines for the bit fields in the I2CM_SA register.
 * @{ 
 */
#define I2CM_SA_SA_M            0x000000FE  /**< I2C slave address mask */
#define I2CM_SA_SA_S            1           /**< I2C slave address shift */
#define I2CM_SA_RS              0x00000001  /**< I2C receive and send bit */
#define I2CM_SA_RS_M            0x00000001  /**< I2C receive and send bit mask */
#define I2CM_SA_RS_S            0           /**< I2C receive and send bit shift */
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * \name I2C defines for the bit fields in the I2CM_CTRL register.
 * @{
 */
#define I2CM_CTRL_ACK           0x00000008  /**< I2C Data acknowledge bit */ 
#define I2CM_CTRL_ACK_M         0x00000008  /**< I2C Data acknowledge bit mask */
#define I2CM_CTRL_ACK_S         3           /**< I2C Data acknowledge bit shift */
#define I2CM_CTRL_STOP          0x00000004  /**< I2C Generate STOP bit */ 
#define I2CM_CTRL_STOP_M        0x00000004  /**< I2C Generate STOP bit mask */ 
#define I2CM_CTRL_STOP_S        2           /**< I2C Generate STOP bit shift */ 
#define I2CM_CTRL_START         0x00000002  /**< I2C Generate START bit */
#define I2CM_CTRL_START_M       0x00000002  /**< I2C Generate START bit mask */
#define I2CM_CTRL_START_S       1           /**< I2C Generate START bit shift */
#define I2CM_CTRL_RUN           0x00000001  /**< I2C master enable bit */ 
#define I2CM_CTRL_RUN_M         0x00000001  /**< I2C master enable bit mask */ 
#define I2CM_CTRL_RUN_S         0           /**< I2C master enable bit shift */ 
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * \name I2C defines for the bit fields in the I2CM_STAT register.
 * @{
 */
#define I2CM_STAT_BUSBSY        0x00000040 /**< I2C Bus busy bit */
#define I2CM_STAT_BUSBSY_M      0x00000040  /**< I2C Bus busy bit mask */
#define I2CM_STAT_BUSBSY_S      6           /**< I2C Bus busy bit shift */
#define I2CM_STAT_IDLE          0x00000020  /**< I2C idle bit */ 
#define I2CM_STAT_IDLE_M        0x00000020  /**< I2C idle bit mask */ 
#define I2CM_STAT_IDLE_S        5           /**< I2C idle bit shift*/ 
#define I2CM_STAT_ARBLST        0x00000010  /**< I2C Arbitration lost bit */ 
#define I2CM_STAT_ARBLST_M      0x00000010  /**< I2C Arbitration lost bit mask*/
#define I2CM_STAT_ARBLST_S      4           /**< I2C Arbitration lost bit shift */
#define I2CM_STAT_DATACK        0x00000008  /**< I2C Acknowledge data bit */ 
#define I2CM_STAT_DATACK_M      0x00000008  /**< I2C Acknowledge data bit mask */
#define I2CM_STAT_DATACK_S      3           /**< I2C Acknowledge data bit shift */
#define I2CM_STAT_ADRACK        0x00000004  /**< I2C Acknowledge address bit */
#define I2CM_STAT_ADRACK_M      0x00000004  /**< I2C Acknowledge address bit mask */
#define I2CM_STAT_ADRACK_S      2           /**< I2C Acknowledge address bit shift*/
#define I2CM_STAT_ERROR         0x00000002  /**< I2C Error bit */
#define I2CM_STAT_ERROR_M       0x00000002  /**< I2C Error bit mask */
#define I2CM_STAT_ERROR_S       1           /**< I2C Error bit shift*/
#define I2CM_STAT_BUSY          0x00000001  /**< I2C busy bit */
#define I2CM_STAT_BUSY_M        0x00000001  /**< I2C busy bit mask*/
#define I2CM_STAT_BUSY_S        0           /**< I2C busy bit shift*/
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * /name I2C defines for the bit fields in the I2CM_DR register.
 * @{
 */
#define I2CM_DR_DATA_M          0x000000FF  /**< I2C data transferred mask */
#define I2CM_DR_DATA_S          0           /**< I2C data transferred shift */
/** @} */
/*---------------------------------------------------------------------------*/

/**
 *  \name I2C defines for the bit fields in the I2CM_TPR register.
 * @{
 */
#define I2CM_TPR_TPR_M          0x0000007F  /**< I2C SCL clock period mask */
#define I2CM_TPR_TPR_S          0           /**< I2C SCL clock period shift */
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * /name I2C defines for the bit fields in the I2CM_IMR register.
 * @{
 */
#define I2CM_IMR_IM             0x00000001  /**< I2C Interrupt mask status */
#define I2CM_IMR_IM_M           0x00000001  /**< I2C Interrupt mask mask */
#define I2CM_IMR_IM_S           0           /**< I2C Interrupt mask shift */
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * \name I2C defines for the bit fields in the I2CM_RIS register.
 * @{
 */
#define I2CM_RIS_RIS            0x00000001  /**< I2C Raw interrupt status */
#define I2CM_RIS_RIS_M          0x00000001  /**< I2C Raw interrupt status mask */
#define I2CM_RIS_RIS_S          0           /**< I2C Raw interrupt status shift */
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * \name I2C defines for the bit fields in the I2CM_MIS register.
 * @{
 */
#define I2CM_MIS_MIS            0x00000001  /**< I2C Masked interrupt status */
#define I2CM_MIS_MIS_M          0x00000001  /**< I2C Masked interrupt status mask */
#define I2CM_MIS_MIS_S          0           /**< I2C Masked interrupt status shift */
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * \name I2C defines for the bit fields in the I2CM_ICR register.
 * @{
 */
#define I2CM_ICR_IC             0x00000001  /**< I2C Interrupt clear bit */
#define I2CM_ICR_IC_M           0x00000001  /**< I2C Interrupt clear bit mask */
#define I2CM_ICR_IC_S           0           /**< I2C Interrupt clear bit shift */
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * \name I2C defines for the bit fields in the I2CM_CR register.
 * @{
 */
#define I2CM_CR_SFE             0x00000020  /**< I2C slave function enable */ 
#define I2CM_CR_SFE_M           0x00000020  /**< I2C slave function enable mask */ 
#define I2CM_CR_SFE_S           5           /**< I2C slave function enable shift */ 
#define I2CM_CR_MFE             0x00000010  /**< I2C master function enable */
#define I2CM_CR_MFE_M           0x00000010  /**< I2C master function enable mask */
#define I2CM_CR_MFE_S           4           /**< I2C master function enable shift */
#define I2CM_CR_LPBK            0x00000001  /**< I2C loopback function enable */
#define I2CM_CR_LPBK_M          0x00000001  /**< I2C loopback function enable mask */
#define I2CM_CR_LPBK_S          0           /**< I2C loopback function enable shift*/
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * \name I2C interrupt defines.
 * @{
 */
#define I2C_INT_MASTER          0x00000001  /**< I2C Master Interrupt */
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * \name I2C Master commands.
 * @{
 */
#define I2C_MASTER_CMD_SINGLE_SEND              0x00000007  /**< I2C Master command single send */
#define I2C_MASTER_CMD_SINGLE_RECEIVE           0x00000007  /**< I2C Master command single Rx */
#define I2C_MASTER_CMD_BURST_SEND_START         0x00000003  /**< I2C Master command bursty send start */
#define I2C_MASTER_CMD_BURST_SEND_CONT          0x00000001  /**< I2C Master command bursty send cont send */
#define I2C_MASTER_CMD_BURST_SEND_FINISH        0x00000005  /**< I2C Master command bursty send finish */
#define I2C_MASTER_CMD_BURST_SEND_ERROR_STOP    0x00000004  /**< I2C Master command bursty send error */
#define I2C_MASTER_CMD_BURST_RECEIVE_START      0x0000000b  /**< I2C Master command bursty Rx Start */
#define I2C_MASTER_CMD_BURST_RECEIVE_CONT       0x00000009   /**< I2C Master command bursty Rx cont */
#define I2C_MASTER_CMD_BURST_RECEIVE_FINISH     0x00000005    /**< I2C Master command bursty Rx finish */
#define I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP 0x00000004  /**< I2C Master command bursty Rx error*/
/** @} */
/*---------------------------------------------------------------------------*/

/**
 * \name I2C Master error status.
 * @{
 */
#define I2C_MASTER_ERR_NONE     0           /**< I2C Master Error none */
#define I2C_MASTER_ERR_ADDR_ACK 0x00000004  /**< I2C Master Error on address ack */ 
#define I2C_MASTER_ERR_DATA_ACK 0x00000008  /**< I2C Master Error on data ack */
#define I2C_MASTER_ERR_ARB_LOST 0x00000010  /**< I2C Master Error on arbitration lost */
/** @} */
/*---------------------------------------------------------------------------*/


/**
 *  \name Miscellaneous I2C driver definitions.
 * @{
 */
#define I2C_MASTER_MAX_RETRIES  1000        /**< I2C Master max number of retries */

/** @} */
/*---------------------------------------------------------------------------*/

/**
 *  \name I2C prototypes
 * @{
 */

/**
 * \brief The init command for the i2c master driver. 
 *
 *  Sets pin functions and dirs and preps the i2c subsystem.
 */
void i2c_init(void);

/**
 * \breif This is a write command that supports arbitrary lenght i2c sends
 * \param b An array of what needs written to the port
 * \param len Number of bytes in the write buffer (arg b) to send
 * \param slaveaddr  The (raw) i2c address for the device (<0x80)
 * 
 * \retval 0 error, 2 successful byte write
 */
uint8_t i2c_write_bytes(uint8_t* b, uint8_t len, uint8_t slaveaddr);

/** 
 * \breif This is a write command that supports single byte i2c sends
 * \param value The unsigened byte to be written to the port
 * \param slaveaddr The (raw) i2c address for the device (<0x80)
 * 
 * \retval 0 error, 1 fell through (should be unreachable), 2 successful
 * byte write, 3 successful multibyte write
 */
uint8_t i2c_write_byte(uint8_t value, uint8_t slaveaddr);

/** \breif This is a read command that supports arbitrary lenght i2c reads
 * \param b An array that the function will fill while reading
 * \param len Lenght of the read requested
 * \param slaveaddr  The (raw) i2c address for the device (<0x80)
 * 
 * \retval 0 error, 1 fell through (should be unreachable), 2 successful
 * byte read, 3 successful multibyte read
 */
uint8_t i2c_read_bytes(uint8_t* b, uint8_t len, uint8_t slaveaddr);

/** 
 * \breif This is a read command that supports single byte i2c reads
 * \param slaveaddr  -- the (raw) i2c address for the device (<0x80)
 * 
 * \retval 0 error, else value read (uint8_t).
 *
 * \note The error return can collide with the byte return if the read value
 *         is 0x00
 */
uint8_t i2c_read_byte(uint8_t slaveaddr);
/** @} */

#endif /* I2C_H_ */


/**
 * @}
 * @}
 */
