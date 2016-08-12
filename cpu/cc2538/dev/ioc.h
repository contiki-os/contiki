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
 * \defgroup cc2538-ioc cc2538 I/O Control
 *
 * Driver for the cc2538 I/O Control Module
 * @{
 *
 * \file
 * Header file with declarations for the I/O Control module
 */
#ifndef IOC_H_
#define IOC_H_

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name IOC Signal Select Registers
 * @{
 */
#define IOC_PA0_SEL                 0x400D4000
#define IOC_PA1_SEL                 0x400D4004
#define IOC_PA2_SEL                 0x400D4008
#define IOC_PA3_SEL                 0x400D400C
#define IOC_PA4_SEL                 0x400D4010
#define IOC_PA5_SEL                 0x400D4014
#define IOC_PA6_SEL                 0x400D4018
#define IOC_PA7_SEL                 0x400D401C
#define IOC_PB0_SEL                 0x400D4020
#define IOC_PB1_SEL                 0x400D4024
#define IOC_PB2_SEL                 0x400D4028
#define IOC_PB3_SEL                 0x400D402C
#define IOC_PB4_SEL                 0x400D4030
#define IOC_PB5_SEL                 0x400D4034
#define IOC_PB6_SEL                 0x400D4038
#define IOC_PB7_SEL                 0x400D403C
#define IOC_PC0_SEL                 0x400D4040
#define IOC_PC1_SEL                 0x400D4044
#define IOC_PC2_SEL                 0x400D4048
#define IOC_PC3_SEL                 0x400D404C
#define IOC_PC4_SEL                 0x400D4050
#define IOC_PC5_SEL                 0x400D4054
#define IOC_PC6_SEL                 0x400D4058
#define IOC_PC7_SEL                 0x400D405C
#define IOC_PD0_SEL                 0x400D4060
#define IOC_PD1_SEL                 0x400D4064
#define IOC_PD2_SEL                 0x400D4068
#define IOC_PD3_SEL                 0x400D406C
#define IOC_PD4_SEL                 0x400D4070
#define IOC_PD5_SEL                 0x400D4074
#define IOC_PD6_SEL                 0x400D4078
#define IOC_PD7_SEL                 0x400D407C
/** @} */
/*---------------------------------------------------------------------------*/
/** \name IOC Override Configuration Registers
 * @{
 */
#define IOC_PA0_OVER                0x400D4080
#define IOC_PA1_OVER                0x400D4084
#define IOC_PA2_OVER                0x400D4088
#define IOC_PA3_OVER                0x400D408C
#define IOC_PA4_OVER                0x400D4090
#define IOC_PA5_OVER                0x400D4094
#define IOC_PA6_OVER                0x400D4098
#define IOC_PA7_OVER                0x400D409C
#define IOC_PB0_OVER                0x400D40A0
#define IOC_PB1_OVER                0x400D40A4
#define IOC_PB2_OVER                0x400D40A8
#define IOC_PB3_OVER                0x400D40AC
#define IOC_PB4_OVER                0x400D40B0
#define IOC_PB5_OVER                0x400D40B4
#define IOC_PB6_OVER                0x400D40B8
#define IOC_PB7_OVER                0x400D40BC
#define IOC_PC0_OVER                0x400D40C0
#define IOC_PC1_OVER                0x400D40C4
#define IOC_PC2_OVER                0x400D40C8
#define IOC_PC3_OVER                0x400D40CC
#define IOC_PC4_OVER                0x400D40D0
#define IOC_PC5_OVER                0x400D40D4
#define IOC_PC6_OVER                0x400D40D8
#define IOC_PC7_OVER                0x400D40DC
#define IOC_PD0_OVER                0x400D40E0
#define IOC_PD1_OVER                0x400D40E4
#define IOC_PD2_OVER                0x400D40E8
#define IOC_PD3_OVER                0x400D40EC
#define IOC_PD4_OVER                0x400D40F0
#define IOC_PD5_OVER                0x400D40F4
#define IOC_PD6_OVER                0x400D40F8
#define IOC_PD7_OVER                0x400D40FC
/** @} */
/*---------------------------------------------------------------------------*/
/** \name IOC Input Pin Select Registers
 * @{
 */
#define IOC_UARTRXD_UART0           0x400D4100  /**< UART0 RX */
#define IOC_UARTCTS_UART1           0x400D4104  /**< UART1 CTS */
#define IOC_UARTRXD_UART1           0x400D4108  /**< UART1 RX */
#define IOC_CLK_SSI_SSI0            0x400D410C  /**< SSI0 Clock */
#define IOC_SSIRXD_SSI0             0x400D4110  /**< SSI0 RX */
#define IOC_SSIFSSIN_SSI0           0x400D4114  /**< SSI0 FSSIN */
#define IOC_CLK_SSIIN_SSI0          0x400D4118  /**< SSI0 Clock SSIIN */
#define IOC_CLK_SSI_SSI1            0x400D411C  /**< SSI1 Clock */
#define IOC_SSIRXD_SSI1             0x400D4120  /**< SSI1 RX */
#define IOC_SSIFSSIN_SSI1           0x400D4124  /**< SSI1 FSSIN Select */
#define IOC_CLK_SSIIN_SSI1          0x400D4128  /**< SSI1 Clock SSIIN */
#define IOC_I2CMSSDA                0x400D412C  /**< I2C SDA */
#define IOC_I2CMSSCL                0x400D4130  /**< I2C SCL */
#define IOC_GPT0OCP1                0x400D4134  /**< GPT0OCP1 */
#define IOC_GPT0OCP2                0x400D4138  /**< GPT0OCP2 */
#define IOC_GPT1OCP1                0x400D413C  /**< GPT1OCP1 */
#define IOC_GPT1OCP2                0x400D4140  /**< GPT1OCP2 */
#define IOC_GPT2OCP1                0x400D4144  /**< GPT2OCP1 */
#define IOC_GPT2OCP2                0x400D4148  /**< GPT2OCP2 */
#define IOC_GPT3OCP1                0x400D414C  /**< GPT3OCP1 */
#define IOC_GPT3OCP2                0x400D4150  /**< GPT3OCP2 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name I/O Control Register Bit Masks
 * @{
 */
#define IOC_OVR_MASK                0x0000000F  /**< IOC_Pxn_OVER registers */
#define IOC_PXX_SEL_MASK            0x0000001F  /**< IOC_Pxn_SEL registers */
#define IOC_INPUT_SEL_MASK          0x0000001F  /**< All other IOC registers */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name INPUT_SEL Values (For Pin Selection Registers)
 * @{
 */
#define IOC_INPUT_SEL_PA0           0x00000000
#define IOC_INPUT_SEL_PA1           0x00000001
#define IOC_INPUT_SEL_PA2           0x00000002
#define IOC_INPUT_SEL_PA3           0x00000003
#define IOC_INPUT_SEL_PA4           0x00000004
#define IOC_INPUT_SEL_PA5           0x00000005
#define IOC_INPUT_SEL_PA6           0x00000006
#define IOC_INPUT_SEL_PA7           0x00000007
#define IOC_INPUT_SEL_PB0           0x00000008
#define IOC_INPUT_SEL_PB1           0x00000009
#define IOC_INPUT_SEL_PB2           0x0000000A
#define IOC_INPUT_SEL_PB3           0x0000000B
#define IOC_INPUT_SEL_PB4           0x0000000C
#define IOC_INPUT_SEL_PB5           0x0000000D
#define IOC_INPUT_SEL_PB6           0x0000000E
#define IOC_INPUT_SEL_PB7           0x0000000F
#define IOC_INPUT_SEL_PC0           0x00000010
#define IOC_INPUT_SEL_PC1           0x00000011
#define IOC_INPUT_SEL_PC2           0x00000012
#define IOC_INPUT_SEL_PC3           0x00000013
#define IOC_INPUT_SEL_PC4           0x00000014
#define IOC_INPUT_SEL_PC5           0x00000015
#define IOC_INPUT_SEL_PC6           0x00000016
#define IOC_INPUT_SEL_PC7           0x00000017
#define IOC_INPUT_SEL_PD0           0x00000018
#define IOC_INPUT_SEL_PD1           0x00000019
#define IOC_INPUT_SEL_PD2           0x0000001A
#define IOC_INPUT_SEL_PD3           0x0000001B
#define IOC_INPUT_SEL_PD4           0x0000001C
#define IOC_INPUT_SEL_PD5           0x0000001D
#define IOC_INPUT_SEL_PD6           0x0000001E
#define IOC_INPUT_SEL_PD7           0x0000001F
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Peripheral Signal Select Values (for IOC_Pxx_SEL registers)
 * @{
 */
#define IOC_PXX_SEL_UART0_TXD       0x00000000
#define IOC_PXX_SEL_UART1_RTS       0x00000001
#define IOC_PXX_SEL_UART1_TXD       0x00000002
#define IOC_PXX_SEL_SSI0_TXD        0x00000003
#define IOC_PXX_SEL_SSI0_CLKOUT     0x00000004
#define IOC_PXX_SEL_SSI0_FSSOUT     0x00000005
#define IOC_PXX_SEL_SSI0_STXSER_EN  0x00000006
#define IOC_PXX_SEL_SSI1_TXD        0x00000007
#define IOC_PXX_SEL_SSI1_CLKOUT     0x00000008
#define IOC_PXX_SEL_SSI1_FSSOUT     0x00000009
#define IOC_PXX_SEL_SSI1_STXSER_EN  0x0000000A
#define IOC_PXX_SEL_I2C_CMSSDA      0x0000000B
#define IOC_PXX_SEL_I2C_CMSSCL      0x0000000C
#define IOC_PXX_SEL_GPT0_ICP1       0x0000000D
#define IOC_PXX_SEL_GPT0_ICP2       0x0000000E
#define IOC_PXX_SEL_GPT1_ICP1       0x0000000F
#define IOC_PXX_SEL_GPT1_ICP2       0x00000010
#define IOC_PXX_SEL_GPT2_ICP1       0x00000011
#define IOC_PXX_SEL_GPT2_ICP2       0x00000012
#define IOC_PXX_SEL_GPT3_ICP1       0x00000013
#define IOC_PXX_SEL_GPT3_ICP2       0x00000014
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Values for IOC_PXX_OVER
 * @{
 */
#define IOC_OVERRIDE_OE   0x00000008    /**< Output Enable */
#define IOC_OVERRIDE_PUE  0x00000004    /**< Pull Up Enable */
#define IOC_OVERRIDE_PDE  0x00000002    /**< Pull Down Enable */
#define IOC_OVERRIDE_ANA  0x00000001    /**< Analog Enable */
#define IOC_OVERRIDE_DIS  0x00000000    /**< Override Disabled */
/** @} */

/*---------------------------------------------------------------------------*/
/** \name IOC Functions
 * @{
 */
/** \brief Initialise the IOC driver */
void ioc_init();

/**
 * \brief Set Port:Pin override function
 * \param port The port as a number (PA: 0, PB: 1 etc)
 * \param pin The pin as a number
 * \param over The desired override configuration
 *
 * \e over can take the following values:
 *
 * - IOC_OVERRIDE_OE: Output
 * - IOC_OVERRIDE_PUE: Pull-Up
 * - IOC_OVERRIDE_PDE: Pull-Down
 * - IOC_OVERRIDE_ANA: Analog
 * - IOC_OVERRIDE_DIS: Disabled
 */
void ioc_set_over(uint8_t port, uint8_t pin, uint8_t over);

/**
 * \brief Function select for Port:Pin
 * \param port The port as a number (PA: 0, PB: 1 etc)
 * \param pin The pin as a number
 * \param sel The desired function
 *
 * The value of \e sel can be any of the IOC_PXX_SEL_xyz defines. For example
 * IOC_PXX_SEL_UART0_TXD will set the port to act as UART0 TX
 */
void ioc_set_sel(uint8_t port, uint8_t pin, uint8_t sel);

/**
 * \brief Generates an IOC_INPUT_SEL_PXn value from a port/pin number
 * \param port The port as a number (PA: 0, PB: 1 etc)
 * \param pin The pin as a number
 * \return A value which can be written in the INPUT_SEL bits of various IOC
 *         registers
 */
#define ioc_input_sel(port, pin) ((port << 3) | pin)

/** @} */
#endif /* IOC_H_ */

/**
 * @}
 * @}
 */
