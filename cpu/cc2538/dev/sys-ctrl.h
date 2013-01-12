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
 * \defgroup cc2538-sys-ctrl cc2538 System Control
 *
 * Driver for the cc2538 System Control Module
 * @{
 *
 * \file
 * Header file for the cc2538 System Control driver
 */
#ifndef SYS_CTRL_H_
#define SYS_CTRL_H_
/*---------------------------------------------------------------------------*/
/** \name SysCtrl Constants, used by the SYS_DIV and IO_DIV bits of the
 * SYS_CTRL_CLOCK_CTRL register
 * @{
 */
#define SYS_CTRL_32MHZ               32000000
#define SYS_CTRL_16MHZ               16000000
#define SYS_CTRL_8MHZ                 8000000
#define SYS_CTRL_4MHZ                 4000000
#define SYS_CTRL_2MHZ                 2000000
#define SYS_CTRL_1MHZ                 1000000
#define SYS_CTRL_500KHZ                500000
#define SYS_CTRL_250KHZ                250000
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Definitions of Sys Ctrl registers
 * @{
 */
#define SYS_CTRL_CLOCK_CTRL     0x400D2000  /**< Clock control register */
#define SYS_CTRL_CLOCK_STA      0x400D2004  /**< Clock status register */
#define SYS_CTRL_RCGCGPT        0x400D2008  /**< GPT[3:0] clocks - active mode */
#define SYS_CTRL_SCGCGPT        0x400D200C  /**< GPT[3:0] clocks - sleep mode */
#define SYS_CTRL_DCGCGPT        0x400D2010  /**< GPT[3:0] clocks - PM0 */
#define SYS_CTRL_SRGPT          0x400D2014  /**< GPT[3:0] reset control */
#define SYS_CTRL_RCGCSSI        0x400D2018  /**< SSI[1:0] clocks - active mode */
#define SYS_CTRL_SCGCSSI        0x400D201C  /**< SSI[1:0] clocks - sleep mode */
#define SYS_CTRL_DCGCSSI        0x400D2020  /**< SSI[1:0] clocks - PM0 mode */
#define SYS_CTRL_SRSSI          0x400D2024  /**< SSI[1:0] reset control */
#define SYS_CTRL_RCGCUART       0x400D2028  /**< UART[1:0] clocks - active mode */
#define SYS_CTRL_SCGCUART       0x400D202C  /**< UART[1:0] clocks - sleep mode */
#define SYS_CTRL_DCGCUART       0x400D2030  /**< UART[1:0] clocks - PM0 */
#define SYS_CTRL_SRUART         0x400D2034  /**< UART[1:0] reset control */
#define SYS_CTRL_RCGCI2C        0x400D2038  /**< I2C clocks - active mode */
#define SYS_CTRL_SCGCI2C        0x400D203C  /**< I2C clocks - sleep mode */
#define SYS_CTRL_DCGCI2C        0x400D2040  /**< I2C clocks - PM0 */
#define SYS_CTRL_SRI2C          0x400D2044  /**< I2C clocks - reset control */
#define SYS_CTRL_RCGCSEC        0x400D2048  /**< Sec Mod clocks - active mode */
#define SYS_CTRL_SCGCSEC        0x400D204C  /**< Sec Mod clocks - sleep mode */
#define SYS_CTRL_DCGCSEC        0x400D2050  /**< Sec Mod clocks - PM0 */
#define SYS_CTRL_SRSEC          0x400D2054  /**< Sec Mod reset control */
#define SYS_CTRL_PMCTL          0x400D2058  /**< Power Mode Control */
#define SYS_CTRL_SRCRC          0x400D205C  /**< CRC on state retention */
#define SYS_CTRL_PWRDBG         0x400D2074  /**< Power debug register */
#define SYS_CTRL_CLD            0x400D2080  /**< clock loss detection feature */
#define SYS_CTRL_IWE            0x400D2094  /**< interrupt wake-up. */
#define SYS_CTRL_I_MAP          0x400D2098  /**< Interrupt map select */
#define SYS_CTRL_RCGCRFC        0x400D20A8  /**< RF Core clocks - active mode */
#define SYS_CTRL_SCGCRFC        0x400D20AC  /**< RF Core clocks - Sleep mode */
#define SYS_CTRL_DCGCRFC        0x400D20B0  /**< RF Core clocks - PM0 */
#define SYS_CTRL_EMUOVR         0x400D20B4  /**< Emulator override */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SYS_CTRL_CLOCK_CTRL register bit masks
 * @{
 */
#define SYS_CTRL_CLOCK_CTRL_OSC32K_CALDIS   0x02000000
#define SYS_CTRL_CLOCK_CTRL_OSC32K          0x01000000
#define SYS_CTRL_CLOCK_CTRL_AMP_DET         0x00200000
#define SYS_CTRL_CLOCK_CTRL_OSC_PD          0x00020000
#define SYS_CTRL_CLOCK_CTRL_OSC             0x00010000
#define SYS_CTRL_CLOCK_CTRL_IO_DIV          0x00000700
#define SYS_CTRL_CLOCK_CTRL_SYS_DIV         0x00000007
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SYS_CTRL_CLOCK_STA register bit masks
 * @{
 */
#define SYS_CTRL_CLOCK_STA_SYNC_32K         0x04000000
#define SYS_CTRL_CLOCK_STA_OSC32K_CALDIS    0x02000000
#define SYS_CTRL_CLOCK_STA_OSC32K           0x01000000
#define SYS_CTRL_CLOCK_STA_RST              0x00C00000
#define SYS_CTRL_CLOCK_STA_SOURCE_CHANGE    0x00100000
#define SYS_CTRL_CLOCK_STA_XOSC_STB         0x00080000
#define SYS_CTRL_CLOCK_STA_HSOSC_STB        0x00040000
#define SYS_CTRL_CLOCK_STA_OSC_PD           0x00020000
#define SYS_CTRL_CLOCK_STA_OSC              0x00010000
#define SYS_CTRL_CLOCK_STA_IO_DIV           0x00000700
#define SYS_CTRL_CLOCK_STA_RTCLK_FREQ       0x00000018
#define SYS_CTRL_CLOCK_STA_SYS_DIV          0x00000007
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SYS_CTRL_RCGCGPT register bit masks
 * @{
 */
#define SYS_CTRL_RCGCGPT_GPT3   0x00000008  /**< GPT3 clock enable, CPU running */
#define SYS_CTRL_RCGCGPT_GPT2   0x00000004  /**< GPT2 clock enable, CPU running */
#define SYS_CTRL_RCGCGPT_GPT1   0x00000002  /**< GPT1 clock enable, CPU running */
#define SYS_CTRL_RCGCGPT_GPT0   0x00000001  /**< GPT0 clock enable, CPU running */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SYS_CTRL_SCGCGPT register bit masks
 * @{
 */
#define SYS_CTRL_SCGCGPT_GPT3   0x00000008  /**< GPT3 clock enable, CPU IDLE */
#define SYS_CTRL_SCGCGPT_GPT2   0x00000004  /**< GPT2 clock enable, CPU IDLE */
#define SYS_CTRL_SCGCGPT_GPT1   0x00000002  /**< GPT1 clock enable, CPU IDLE */
#define SYS_CTRL_SCGCGPT_GPT0   0x00000001  /**< GPT0 clock enable, CPU IDLE */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SYS_CTRL_DCGCGPT register bit masks
 * @{
 */
#define SYS_CTRL_DCGCGPT_GPT3   0x00000008  /**< GPT3 clock enable, PM0 */
#define SYS_CTRL_DCGCGPT_GPT2   0x00000004  /**< GPT2 clock enable, PM0 */
#define SYS_CTRL_DCGCGPT_GPT1   0x00000002  /**< GPT1 clock enable, PM0 */
#define SYS_CTRL_DCGCGPT_GPT0   0x00000001  /**< GPT0 clock enable, PM0 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SYS_CTRL_SRGPT register bits
 * @{
 */
#define SYS_CTRL_SRGPT_GPT3     0x00000008  /**< GPT3 is reset */
#define SYS_CTRL_SRGPT_GPT2     0x00000004  /**< GPT2 is reset */
#define SYS_CTRL_SRGPT_GPT1     0x00000002  /**< GPT1 is reset */
#define SYS_CTRL_SRGPT_GPT0     0x00000001  /**< GPT0 is reset */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SYS_CTRL_PWRDBG register bits
 * @{
 */
#define SYS_CTRL_PWRDBG_FORCE_WARM_RESET    0x00000008
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Possible values for the SYS_CTRL_CLOCK_CTRL_SYS_DIV bits
 * @{
 */
#define SYS_CTRL_CLOCK_CTRL_SYS_DIV_32MHZ   0x00000000
#define SYS_CTRL_CLOCK_CTRL_SYS_DIV_16MHZ   0x00000001
#define SYS_CTRL_CLOCK_CTRL_SYS_DIV_8MHZ    0x00000002
#define SYS_CTRL_CLOCK_CTRL_SYS_DIV_4MHZ    0x00000003
#define SYS_CTRL_CLOCK_CTRL_SYS_DIV_2MHZ    0x00000004
#define SYS_CTRL_CLOCK_CTRL_SYS_DIV_1MHZ    0x00000005
#define SYS_CTRL_CLOCK_CTRL_SYS_DIV_500KHZ  0x00000006
#define SYS_CTRL_CLOCK_CTRL_SYS_DIV_250KHZ  0x00000007
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Possible values for the SYS_CTRL_CLOCK_CTRL_IO_DIV bits
 * @{
 */
#define SYS_CTRL_CLOCK_CTRL_IO_DIV_32MHZ    0x00000000
#define SYS_CTRL_CLOCK_CTRL_IO_DIV_16MHZ    0x00000100
#define SYS_CTRL_CLOCK_CTRL_IO_DIV_8MHZ     0x00000200
#define SYS_CTRL_CLOCK_CTRL_IO_DIV_4MHZ     0x00000300
#define SYS_CTRL_CLOCK_CTRL_IO_DIV_2MHZ     0x00000400
#define SYS_CTRL_CLOCK_CTRL_IO_DIV_1MHZ     0x00000500
#define SYS_CTRL_CLOCK_CTRL_IO_DIV_500KHZ   0x00000600
#define SYS_CTRL_CLOCK_CTRL_IO_DIV_250KHZ   0x00000700
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SYS_CTRL_RCGCUART Register Bit-Masks
 * @{
 */
#define SYS_CTRL_RCGCUART_UART1             0x00000002  /**< UART1 Clock, CPU running */
#define SYS_CTRL_RCGCUART_UART0             0x00000001  /**< UART0 Clock, CPU running */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SYS_CTRL_SCGCUART Register Bit-Masks
 * @{
 */
#define SYS_CTRL_SCGCUART_UART1             0x00000002  /**< UART1 Clock, CPU IDLE */
#define SYS_CTRL_SCGCUART_UART0             0x00000001  /**< UART0 Clock, CPU IDLE */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SYS_CTRL_RCGCUART Register Bit-Masks
 * @{
 */
#define SYS_CTRL_DCGCUART_UART1             0x00000002  /**< UART1 Clock, PM0 */
#define SYS_CTRL_DCGCUART_UART0             0x00000001  /**< UART0 Clock, PM0 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SYS_CTRL_SRUART register bits
 * @{
 */
#define SYS_CTRL_SRUART_UART1               0x00000002  /**< UART1 module is reset */
#define SYS_CTRL_SRUART_UART0               0x00000001  /**< UART0 module is reset  */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SYS_CTRL_PMCTL register values
 * @{
 */
#define SYS_CTRL_PMCTL_PM3                  0x00000003  /**< PM3 */
#define SYS_CTRL_PMCTL_PM2                  0x00000002  /**< PM2 */
#define SYS_CTRL_PMCTL_PM1                  0x00000001  /**< PM1 */
#define SYS_CTRL_PMCTL_PM0                  0x00000000  /**< PM0 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SysCtrl functions
 * @{
 */
/** \brief Initialises the System Control Driver. The main purpose of this
 * function is to power up and select clocks and oscillators */
void sys_ctrl_init();

/** \brief Generates a warm reset through the SYS_CTRL_PWRDBG register */
void sys_ctrl_reset();

/** @} */

#endif /* SYS_CTRL_H_ */

/**
 * @}
 * @}
 */
