/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
 */

/*
 * Copyright (c) 2011, CSIRO
 *
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
 */
/**
 * \file
 *         Private constants and declarations for the CC1101 radio driver
 * \author
 *         Adam Dunkels <adam@thingsquare.com>
 *         Matt D'Souza <Matthew.D'Souza@csiro.au>
 *         Chris Crossman <christopher.crossman@csiro.au>
 */


#ifndef CC1101_CONST_H
#define CC1101_CONST_H


#define CONF_REG_SIZE  47    /* There are 47 8-bit configuration registers */


/**
 * CC1101 Configuration Registers
 *
 */
#define CC1101_IOCFG2       0x00        // GDO2 output pin configuration
#define CC1101_IOCFG1       0x01        // GDO1 output pin configuration
#define CC1101_IOCFG0       0x02        // GDO0 output pin configuration
#define CC1101_FIFOTHR      0x03        // RX FIFO and TX FIFO thresholds
#define CC1101_SYNC1        0x04        // Sync word, high byte
#define CC1101_SYNC0        0x05        // Sync word, low byte
#define CC1101_PKTLEN       0x06        // Packet length
#define CC1101_PKTCTRL1     0x07        // Packet automation control
#define CC1101_PKTCTRL0     0x08        // Packet automation control
#define CC1101_ADDR         0x09        // Device address
#define CC1101_CHANNR       0x0A        // Channel number
#define CC1101_FSCTRL1      0x0B        // Frequency synthesizer control
#define CC1101_FSCTRL0      0x0C        // Frequency synthesizer control
#define CC1101_FREQ2        0x0D        // Frequency control word, high byte
#define CC1101_FREQ1        0x0E        // Frequency control word, middle byte
#define CC1101_FREQ0        0x0F        // Frequency control word, low byte
#define CC1101_MDMCFG4      0x10        // Modem configuration
#define CC1101_MDMCFG3      0x11        // Modem configuration
#define CC1101_MDMCFG2      0x12        // Modem configuration
#define CC1101_MDMCFG1      0x13        // Modem configuration
#define CC1101_MDMCFG0      0x14        // Modem configuration
#define CC1101_DEVIATN      0x15        // Modem deviation setting
#define CC1101_MCSM2        0x16        // Main Radio Control State Machine configuration
#define CC1101_MCSM1        0x17        // Main Radio Control State Machine configuration
#define CC1101_MCSM0        0x18        // Main Radio Control State Machine configuration
#define CC1101_FOCCFG       0x19        // Frequency Offset Compensation configuration
#define CC1101_BSCFG        0x1A        // Bit Synchronization configuration
#define CC1101_AGCCTRL2     0x1B        // AGC control
#define CC1101_AGCCTRL1     0x1C        // AGC control
#define CC1101_AGCCTRL0     0x1D        // AGC control
#define CC1101_WOREVT1      0x1E        // High byte Event 0 timeout
#define CC1101_WOREVT0      0x1F        // Low byte Event 0 timeout
#define CC1101_WORCTRL      0x20        // Wake On Radio control
#define CC1101_FREND1       0x21        // Front end RX configuration
#define CC1101_FREND0       0x22        // Front end TX configuration
#define CC1101_FSCAL3       0x23        // Frequency synthesizer calibration
#define CC1101_FSCAL2       0x24        // Frequency synthesizer calibration
#define CC1101_FSCAL1       0x25        // Frequency synthesizer calibration
#define CC1101_FSCAL0       0x26        // Frequency synthesizer calibration
#define CC1101_RCCTRL1      0x27        // RC oscillator configuration
#define CC1101_RCCTRL0      0x28        // RC oscillator configuration
#define CC1101_FSTEST       0x29        // Frequency synthesizer calibration control
#define CC1101_PTEST        0x2A        // Production test
#define CC1101_AGCTEST      0x2B        // AGC test
#define CC1101_TEST2        0x2C        // Various test settings
#define CC1101_TEST1        0x2D        // Various test settings
#define CC1101_TEST0        0x2E        // Various test settings

// Strobe commands
#define CC1101_SRES         0x30        // Reset chip.
#define CC1101_SFSTXON      0x31        // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1).
// If in RX/TX: Go to a wait state where only the synthesizer is
// running (for quick RX / TX turnaround).

#define CC1101_SXOFF        0x32        // Turn off crystal oscillator.
#define CC1101_SCAL         0x33        // Calibrate frequency synthesizer and turn it off
// (enables quick start).

#define CC1101_SRX          0x34        // Enable RX. Perform calibration first if coming from IDLE and
// MCSM0.FS_AUTOCAL=1.

#define CC1101_STX          0x35        // In IDLE state: Enable TX. Perform calibration first if
// MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled:
// Only go to TX if channel is clear.

#define CC1101_SIDLE        0x36        // Exit RX / TX, turn off frequency synthesizer and exit
// Wake-On-Radio mode if applicable.

#define CC1101_SAFC         0x37        // Perform AFC adjustment of the frequency synthesizer
#define CC1101_SWOR         0x38        // Start automatic RX polling sequence (Wake-on-Radio)
#define CC1101_SPWD         0x39        // Enter power down mode when CSn goes high.
#define CC1101_SFRX         0x3A        // Flush the RX FIFO buffer.
#define CC1101_SFTX         0x3B        // Flush the TX FIFO buffer.
#define CC1101_SWORRST      0x3C        // Reset real time clock.
#define CC1101_SNOP         0x3D        // No operation. May be used to pad strobe commands to two
// bytes for simpler software.

#define CC1101_PARTNUM          0x30
#define CC1101_VERSION          0x31
#define CC1101_FREQEST          0x32
#define CC1101_LQI              0x33
#define CC1101_RSSI             0x34
#define CC1101_MARCSTATE        0x35
#define CC1101_WORTIME1         0x36
#define CC1101_WORTIME0         0x37
#define CC1101_PKTSTATUS        0x38
#define CC1101_VCO_VC_DAC       0x39
#define CC1101_TXBYTES          0x3A
#define CC1101_RXBYTES          0x3B
#define CC1101_RCCTRL1_STATUS   0x3C
#define CC1101_RCCTRL0_STATUS   0x3D
#define CC1101_PATABLE          0x3E
#define CC1101_TXFIFO           0x3F
#define CC1101_RXFIFO           0x3F


//Power TX setings
//See Page 57 Table 37, CC1101 datasheet
#define CC1101_PA_M30 	0x03
#define CC1101_PA_M20 	0x0E
#define CC1101_PA_M15 	0x1E
#define CC1101_PA_M10 	0x27
#define CC1101_PA_M6 		0x38
#define CC1101_PA_0 		0x8E
#define CC1101_PA_5 		0x84
#define CC1101_PA_7 		0xCC
#define CC1101_PA_10 		0xC3
#define CC1101_PA_11 		0xC0

#define CC1101_STATE_SLEEP             0
#define CC1101_STATE_IDLE              1
#define CC1101_STATE_XOFF              2
#define CC1101_STATE_VCOON_MC          3
#define CC1101_STATE_REGON_MC          4
#define CC1101_STATE_MANCAL            5
#define CC1101_STATE_VCOON             6
#define CC1101_STATE_REGON             7
#define CC1101_STATE_STARTCAL          8
#define CC1101_STATE_BWBOOST           9
#define CC1101_STATE_FS_LOCK          10
#define CC1101_STATE_IFADCON          11
#define CC1101_STATE_ENDCAL           12
#define CC1101_STATE_RX               13
#define CC1101_STATE_RX_END           14
#define CC1101_STATE_RX_RST           15
#define CC1101_STATE_TXRX_SWITCH      16
#define CC1101_STATE_RXFIFO_OVERFLOW  17
#define CC1101_STATE_FSTXON           18
#define CC1101_STATE_TX               19
#define CC1101_STATE_TX_END           20
#define CC1101_STATE_RXTX_SWITCH      21
#define CC1101_STATE_TXFIFO_UNDERFLOW 22


/* alternative names for RF1AIV interrupt flags */
#define CC1101_IFG_GDO0               RF1AIV_RFIFG0
#define CC1101_IFG_GDO1               RF1AIV_RFIFG1
#define CC1101_IFG_GDO2               RF1AIV_RFIFG2


#endif /* CC1101_CONST_H */
