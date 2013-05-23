/***************************************************************************//**
 *   @file   Communication.c
 *   @brief  Implementation of the Communication Driver for RL78G14 processor.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2012(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
********************************************************************************
 *   SVN Revision: $WCREV$
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "Communication.h"	// Communication definitions
#include "RDKRL78G14.h"		// RDKRL78G14 definitions

char IICA0_Flag;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/***************************************************************************//**
 * @brief I2C interrupt service routine.
 *
 * @return None.
*******************************************************************************/
#pragma vector = INTIICA0_vect
__interrupt static void IICA0_Interrupt(void)
{
    IICA0_Flag = 1;
}

/***************************************************************************//**
 * @brief Initializes the SPI communication peripheral.
 *
 * @param lsbFirst  - Transfer format (0 or 1).
 *                    Example: 0x0 - MSB first.
 *                             0x1 - LSB first.
 * @param clockFreq - SPI clock frequency (Hz).
 *                    Example: 1000 - SPI clock frequency is 1 kHz.
 * @param clockPol  - SPI clock polarity (0 or 1).
 *                    Example: 0x0 - Idle state for clock is a low level; active
 *                                   state is a high level;
 *                             0x1 - Idle state for clock is a high level; active
 *                                   state is a low level.
 * @param clockEdg  - SPI clock edge (0 or 1).
 *                    Example: 0x0 - Serial output data changes on transition
 *                                   from idle clock state to active clock state;
 *                             0x1 - Serial output data changes on transition
 *                                   from active clock state to idle clock state.
 *
 * @return status   - Result of the initialization procedure.
 *                    Example:  0 - if initialization was successful;
 *                             -1 - if initialization was unsuccessful.
*******************************************************************************/
char SPI_Init(char lsbFirst,
              long clockFreq,
              char clockPol,
              char clockEdg)
{
    long mckFreq  = 32000000;
    char sdrValue = 0;
    char delay    = 0;

    /* Configure the CS pins. */
    PMOD1_CS_OUT;
    PMOD1_CS_HIGH;
    PMOD2_CS_OUT;
    PMOD2_CS_HIGH;
    ST7579_CS_OUT;
    ST7579_CS_HIGH;

    /* Enable input clock supply. */
    SAU1EN = 1;

    /* After setting the SAUmEN bit to 1, be sure to set serial clock select
       register m (SPSm) after 4 or more fCLK clocks have elapsed. */
    NOP;
    NOP;
    NOP;
    NOP;

    /* Select the fCLK as input clock.  */
    SPS1 = 0x0000;

    /* Select the CSI operation mode. */
    SMR11 = 0x0020;

    clockPol = 1 - clockPol;
    SCR11    = (clockEdg << 13) |
               (clockPol << 12) |
                0xC000 |           // Operation mode: Transmission/reception.
                0x0007;            // 8-bit data length.

    /* clockFreq =  mckFreq / (sdrValue * 2 + 2) */
    sdrValue = mckFreq / (2 * clockFreq) - 1;
    SDR11    = sdrValue << 9;

    /* Set the clock and data initial level. */
    clockPol = 1 - clockPol;
    SO1     &= ~0x0202;
    SO1     |= (clockPol << 9) |
           (clockPol << 1);

    /* Enable output for serial communication operation. */
    SOE1 |= 0x0002;

    /*  Configure the MISO pin as input. */
    PM7  |= 0x02;
    
    /*  Configure SCLK and MOSI pins as output. */
    P7  |= 0x05;
    PM7 &= ~0x05;
    
    /* Wait for the changes to take place. */
    for(delay = 0; delay < 50; delay++)
    {
        NOP;
    }
    
    /* Set the SEmn bit to 1 and enter the communication wait status */
    SS1 |= 0x0002;
    
    return 0;
}

/***************************************************************************//**
 * @brief Writes data to SPI.
 *
 * @param slaveDeviceId - The ID of the selected slave device.
 * @param data          - Data represents the write buffer.
 * @param bytesNumber   - Number of bytes to write.
 *
 * @return Number of written bytes.
*******************************************************************************/
char SPI_Write(char slaveDeviceId,
               unsigned char* data,
               char bytesNumber)
{
    char           byte        = 0;
    unsigned char  read        = 0;
    unsigned short originalSCR = 0;
    unsigned short originalSO1 = 0;

    if(slaveDeviceId == 1)
    {
        PMOD1_CS_LOW;
    }
    if(slaveDeviceId == 2)
    {
        PMOD2_CS_LOW;
    }
    if(slaveDeviceId == 3)
    {
        ST1        |= 0x0002;
        originalSO1 = SO1;
        originalSCR = SCR11;
        SO1        &= ~0x0202;
        SCR11      &= ~0x3000;
        SS1        |= 0x0002;
        ST7579_CS_LOW;
    }
    for(byte = 0; byte < bytesNumber; byte++)
    {
        SIO21 = data[byte];
        NOP;
        while(SSR11 & 0x0040);
        read = SIO21;
    }
    if(slaveDeviceId == 1)
    {
        PMOD1_CS_HIGH;
    }
    if(slaveDeviceId == 2)
    {
        PMOD2_CS_HIGH;
    }
    if(slaveDeviceId == 3)
    {
        ST7579_CS_HIGH;
        ST1  |= 0x0002;
        SO1   = originalSO1;
        SCR11 = originalSCR;
        SS1  |= 0x0002;
    }

    return bytesNumber;
}

/***************************************************************************//**
 * @brief Reads data from SPI.
 *
 * @param slaveDeviceId - The ID of the selected slave device.
 * @param data          - Data represents the write buffer as an input parameter 
 *                        and the read buffer as an output parameter.
 * @param bytesNumber   - Number of bytes to read.
 *
 * @return Number of read bytes.
*******************************************************************************/
char SPI_Read(char slaveDeviceId,
              unsigned char* data,
              char bytesNumber)
{
    char           byte        = 0;
    unsigned short originalSCR = 0;
    unsigned short originalSO1 = 0;

    if(slaveDeviceId == 1)
    {
        PMOD1_CS_LOW;
    }
    if(slaveDeviceId == 2)
    {
        PMOD2_CS_LOW;
    }
    if(slaveDeviceId == 3)
    {
        ST1        |= 0x0002;
        originalSO1 = SO1;
        originalSCR = SCR11;
        SO1        &= ~0x0202;
        SCR11      &= ~0x3000;
        SS1        |= 0x0002;
        ST7579_CS_LOW;
    }
    for(byte = 0; byte < bytesNumber; byte++)
    {
        SIO21 = data[byte];
        NOP;
        while(SSR11 & 0x0040);
        data[byte] = SIO21;
    }
    if(slaveDeviceId == 1)
    {
        PMOD1_CS_HIGH;
    }
    if(slaveDeviceId == 2)
    {
        PMOD2_CS_HIGH;
    }
    if(slaveDeviceId == 3)
    {
        ST7579_CS_HIGH;
        ST1  |= 0x0002;
        SO1   = originalSO1;
        SCR11 = originalSCR;
        SS1  |= 0x0002;
    }

    return bytesNumber;
}

/***************************************************************************//**
 * @brief Initializes the I2C communication peripheral.
 *
 * @param clockFreq - I2C clock frequency (Hz).
 *                    Example: 100000 - SPI clock frequency is 100 kHz.
 * @return status   - Result of the initialization procedure.
 *                    Example:  0 - if initialization was successful;
 *                             -1 - if initialization was unsuccessful.
*******************************************************************************/
char I2C_Init(long clockFreq)
{
    long          fckFreq = 32000000;
    unsigned char wlValue = 0;
    unsigned char whValue = 0;

    /* Enable interrupts */
    EI;

    /* Enable input clock supply. */
    IICA0EN = 1;

    /* Set the fast mode plus operation. */
    SMC0 = 1;

    /* Set transfer rate. */
    wlValue = (unsigned char)((0.5 * fckFreq) / clockFreq);
    whValue = (unsigned char)(wlValue - (fckFreq / (10 * clockFreq)));
    IICWL0  = wlValue;
    IICWH0  = whValue;

    STCEN0  = 1; // After operation is enabled, enable generation of a start
                 // condition without detecting a stop condition.
    WTIM0  = 1;  // Interrupt request is generated at the ninth clock’s
                 // falling edge.

    /* Enable I2C operation. */
    IICE0 = 1;

    /* Configure SCLA0 and SDAA0 pins as digital output. */
    P6 &= ~0x03;
    PM6 &= ~0x03;

    return 0;
}

/***************************************************************************//**
 * @brief Writes data to a slave device.
 *
 * @param slaveAddress - Adress of the slave device.
 * @param dataBuffer   - Pointer to a buffer storing the transmission data.
 * @param bytesNumber  - Number of bytes to write.
 * @param stopBit      - Stop condition control.
 *                       Example: 0 - A stop condition will not be sent;
 *                                1 - A stop condition will be sent.
 *
 * @return status      - Number of read bytes or 0xFF if the slave address was 
 *                       not acknowledged by the device.
*******************************************************************************/
char I2C_Write(char slaveAddress,
               unsigned char* dataBuffer,
               char bytesNumber,
               char stopBit)
{
    char byte   = 0;
    char status = 0;

    IICAMK0 = 1;    // Interrupt servicing disabled.
    STT0    = 1;    // Generate a start condition.
    IICAMK0 = 0;    // Interrupt servicing enabled.

    /* Send the first byte. */
    IICA0_Flag = 0;
    IICA0      = (slaveAddress << 1);
    while(IICA0_Flag == 0);

    if(ACKD0)   // Acknowledge was detected.
    {
        for(byte = 0; byte < bytesNumber; byte++)
        {
            IICA0_Flag = 0;
            IICA0      = *dataBuffer;
            while(IICA0_Flag == 0);
            dataBuffer++;
        }
        status = bytesNumber;
    }
    else        // Acknowledge was not detected.
    {
        status = 0xFF;
    }
    if(stopBit)
    {
        SPT0 = 1;       // Generate a stop condition.
        while(IICBSY0); // Wait until the I2C bus status flag is cleared.
    }

    return status;
}

/***************************************************************************//**
 * @brief Reads data from a slave device.
 *
 * @param slaveAddress - Adress of the slave device.
 * @param dataBuffer   - Pointer to a buffer that will store the received data.
 * @param bytesNumber  - Number of bytes to read.
 * @param stopBit      - Stop condition control.
 *                       Example: 0 - A stop condition will not be sent;
 *                                1 - A stop condition will be sent.
 *
 * @return status      - Number of read bytes or 0xFF if the slave address was 
 *                       not acknowledged by the device.
*******************************************************************************/
char I2C_Read(char slaveAddress,
              unsigned char* dataBuffer,
              char bytesNumber,
              char stopBit)
{
    char byte	= 0;
    char status = 0;

    IICAMK0 = 1;	// Interrupt servicing disabled.
    STT0    = 1;	// Generate a start condition.
    IICAMK0 = 0;        // Interrupt servicing enabled.

    /* Send the first byte. */
    IICA0_Flag = 0;
    IICA0      = (slaveAddress << 1) + 1;
    while(IICA0_Flag == 0);

    if(ACKD0)           // Acknowledge was detected.
    {
        ACKE0 = 1;      // Enable acknowledgment.
        for(byte = 0; byte < bytesNumber; byte++)
        {
            if(byte == (bytesNumber - 1))
            {
                ACKE0 = 0U;           // Disable acknowledgment.
            }
            WREL0       = 1U;         // Cancel wait.
            IICA0_Flag  = 0;
            while(IICA0_Flag == 0);
            *dataBuffer = IICA0;
            dataBuffer++;
        }
        status = bytesNumber;
    }
    else                     // Acknowledge was not detected.
    {
        status = 0xFF;
    }
    if(stopBit)
    {
        SPT0 = 1;           // Generate a stop condition.
        while(IICBSY0);     // Wait until the I2C bus status flag is cleared.
    }

    return status;
}
