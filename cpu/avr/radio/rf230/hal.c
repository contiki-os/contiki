/*   Copyright (c) 2008, Swedish Institute of Computer Science
 *  All rights reserved. 
 *
 *  Additional fixes for AVR contributed by:
 *
 *	Colin O'Flynn coflynn@newae.com
 *	Eric Gnoske egnoske@gmail.com
 *	Blake Leverett bleverett@gmail.com
 *	Mike Vidales mavida404@gmail.com
 *	Kevin Brown kbrown3@uccs.edu
 *	Nate Bohlmann nate@elfwerks.com
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of the copyright holders nor the names of
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
*/

/**
 *   \addtogroup wireless
 *  @{
*/

/**
 *   \defgroup hal RF230 hardware level drivers
 *   @{
 */

/**
 *  \file
 *  This file contains low-level radio driver code.
 */



/*============================ INCLUDE =======================================*/
#include <stdlib.h>

#include "hal.h"
#include "at86rf230_registermap.h"
/*============================ MACROS ========================================*/

/*
 * Macros defined for the radio transceiver's access modes.
 *
 * These functions are implemented as macros since they are used very often.
 */
#define HAL_DUMMY_READ         (0x00) /**<  Dummy value for the SPI. */

#define HAL_TRX_CMD_RW         (0xC0) /**<  Register Write (short mode). */
#define HAL_TRX_CMD_RR         (0x80) /**<  Register Read (short mode). */
#define HAL_TRX_CMD_FW         (0x60) /**<  Frame Transmit Mode (long mode). */
#define HAL_TRX_CMD_FR         (0x20) /**<  Frame Receive Mode (long mode). */
#define HAL_TRX_CMD_SW         (0x40) /**<  SRAM Write. */
#define HAL_TRX_CMD_SR         (0x00) /**<  SRAM Read. */
#define HAL_TRX_CMD_RADDRM     (0x7F) /**<  Register Address Mask. */

#define HAL_CALCULATED_CRC_OK   (0) /**<  CRC calculated over the frame including the CRC field should be 0. */
/*============================ TYPDEFS =======================================*/
/*============================ VARIABLES =====================================*/
/** \brief This is a file internal variable that contains the 16 MSB of the
 *         system time.
 *
 *         The system time (32-bit) is the current time in microseconds. For the
 *         AVR microcontroller implementation this is solved by using a 16-bit
 *         timer (Timer1) with a clock frequency of 1MHz. The hal_system_time is
 *         incremented when the 16-bit timer overflows, representing the 16 MSB.
 *         The timer value it self (TCNT1) is then the 16 LSB.
 *
 *  \see hal_get_system_time
 */
static uint16_t hal_system_time = 0;

/*Flag section.*/
static uint8_t volatile hal_bat_low_flag; /**<  BAT_LOW flag. */
static uint8_t volatile hal_pll_lock_flag;   /**<  PLL_LOCK flag. */

/*Callbacks.*/

/** \brief This function is called when a rx_start interrupt is signaled.
 *
 *         If this function pointer is set to something else than NULL, it will
 *         be called when a RX_START event is signaled. The function takes two
 *         parameters: timestamp in IEEE 802.15.4 symbols (16 us resolution) and
 *         frame length. The event handler will be called in the interrupt domain,
 *         so the function must be kept short and not be blocking! Otherwise the
 *         system performance will be greatly degraded.
 *
 *  \see hal_set_rx_start_event_handler
 */
static hal_rx_start_isr_event_handler_t rx_start_callback;

/** \brief This function is called when a trx_end interrupt is signaled.
 *
 *         If this function pointer is set to something else than NULL, it will
 *         be called when a TRX_END event is signaled. The function takes one
 *         parameter: timestamp in IEEE 802.15.4 symbols (16 us resolution).
 *         The event handler will be called in the interrupt domain,
 *         so the function must not block!
 *
 *  \see hal_set_trx_end_event_handler
 */
static hal_trx_end_isr_event_handler_t trx_end_callback;
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

/** \brief  This function initializes the Hardware Abstraction Layer.
 */
void
hal_init(void)
{
    /*Reset variables used in file.*/
    hal_system_time = 0;
    hal_reset_flags();

    /*IO Specific Initialization.*/
    DDR_SLP_TR |= (1 << SLP_TR); /* Enable SLP_TR as output. */
    DDR_RST    |= (1 << RST);    /* Enable RST as output. */

    /*SPI Specific Initialization.*/
    /* Set SS, CLK and MOSI as output. */
    HAL_DDR_SPI  |= (1 << HAL_DD_SS) | (1 << HAL_DD_SCK) | (1 << HAL_DD_MOSI);
    HAL_PORT_SPI |= (1 << HAL_DD_SS) | (1 << HAL_DD_SCK); /* Set SS and CLK high */
    /* Run SPI at max speed */
    SPCR         = (1 << SPE) | (1 << MSTR); /* Enable SPI module and master operation. */
    SPSR         = (1 << SPI2X); /* Enable doubled SPI speed in master mode. */

    /*TIMER1 Specific Initialization.*/
    TCCR1B = HAL_TCCR1B_CONFIG;       /* Set clock prescaler */
    TIFR1 |= (1 << ICF1);             /* Clear Input Capture Flag. */
    HAL_ENABLE_OVERFLOW_INTERRUPT(); /* Enable Timer1 overflow interrupt. */
    hal_enable_trx_interrupt();    /* Enable interrupts from the radio transceiver. */
}

/*----------------------------------------------------------------------------*/
/** \brief  This function reset the interrupt flags and interrupt event handlers
 *          (Callbacks) to their default value.
 */
void
hal_reset_flags(void)
{
    AVR_ENTER_CRITICAL_REGION();

    /* Reset Flags. */
    hal_bat_low_flag     = 0;
    hal_pll_lock_flag    = 0;

    /* Reset Associated Event Handlers. */
    rx_start_callback = NULL;
    trx_end_callback  = NULL;

    AVR_LEAVE_CRITICAL_REGION();
}

/*----------------------------------------------------------------------------*/
/** \brief  This function returns the current value of the BAT_LOW flag.
 *
 *  The BAT_LOW flag is incremented each time a BAT_LOW event is signaled from the
 *  radio transceiver. This way it is possible for the end user to poll the flag
 *  for new event occurances.
 */
uint8_t
hal_get_bat_low_flag(void)
{
    return hal_bat_low_flag;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function clears the BAT_LOW flag.
 */
void
hal_clear_bat_low_flag(void)
{
    AVR_ENTER_CRITICAL_REGION();
    hal_bat_low_flag = 0;
    AVR_LEAVE_CRITICAL_REGION();
}

/*----------------------------------------------------------------------------*/
/** \brief  This function is used to set new TRX_END event handler, overriding
 *          old handler reference.
 */
hal_trx_end_isr_event_handler_t
hal_get_trx_end_event_handler(void)
{
    return trx_end_callback;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function is used to set new TRX_END event handler, overriding
 *          old handler reference.
 */
void
hal_set_trx_end_event_handler(hal_trx_end_isr_event_handler_t trx_end_callback_handle)
{
    AVR_ENTER_CRITICAL_REGION();
    trx_end_callback = trx_end_callback_handle;
    AVR_LEAVE_CRITICAL_REGION();
}

/*----------------------------------------------------------------------------*/
/** \brief  Remove event handler reference.
 */
void
hal_clear_trx_end_event_handler(void)
{
    AVR_ENTER_CRITICAL_REGION();
    trx_end_callback = NULL;
    AVR_LEAVE_CRITICAL_REGION();
}

/*----------------------------------------------------------------------------*/
/** \brief  This function returns the active RX_START event handler
 *
 *  \return Current RX_START event handler registered.
 */
hal_rx_start_isr_event_handler_t
hal_get_rx_start_event_handler(void)
{
    return rx_start_callback;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function is used to set new RX_START event handler, overriding
 *          old handler reference.
 */
void
hal_set_rx_start_event_handler(hal_rx_start_isr_event_handler_t rx_start_callback_handle)
{
    AVR_ENTER_CRITICAL_REGION();
    rx_start_callback = rx_start_callback_handle;
    AVR_LEAVE_CRITICAL_REGION();
}

/*----------------------------------------------------------------------------*/
/** \brief  Remove event handler reference.
 */
void
hal_clear_rx_start_event_handler(void)
{
    AVR_ENTER_CRITICAL_REGION();
    rx_start_callback = NULL;
    AVR_LEAVE_CRITICAL_REGION();
}

/*----------------------------------------------------------------------------*/
/** \brief  This function returns the current value of the PLL_LOCK flag.
 *
 *  The PLL_LOCK flag is incremented each time a PLL_LOCK event is signaled from the
 *  radio transceiver. This way it is possible for the end user to poll the flag
 *  for new event occurances.
 */
uint8_t
hal_get_pll_lock_flag(void)
{
    return hal_pll_lock_flag;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function clears the PLL_LOCK flag.
 */
void
hal_clear_pll_lock_flag(void)
{
    AVR_ENTER_CRITICAL_REGION();
    hal_pll_lock_flag = 0;
    AVR_LEAVE_CRITICAL_REGION();
}

/*----------------------------------------------------------------------------*/
/** \brief  This function reads data from one of the radio transceiver's registers.
 *
 *  \param  address Register address to read from. See datasheet for register
 *                  map.
 *
 *  \see Look at the at86rf230_registermap.h file for register address definitions.
 *
 *  \returns The actual value of the read register.
 */
uint8_t
hal_register_read(uint8_t address)
{
    /* Add the register read command to the register address. */
    address &= HAL_TRX_CMD_RADDRM;
    address |= HAL_TRX_CMD_RR;

    uint8_t register_value = 0;

    AVR_ENTER_CRITICAL_REGION();

    HAL_SS_LOW(); /* Start the SPI transaction by pulling the Slave Select low. */

    /*Send Register address and read register content.*/
    SPDR = address;
    while ((SPSR & (1 << SPIF)) == 0) {;}
    register_value = SPDR;

    SPDR = register_value;
    while ((SPSR & (1 << SPIF)) == 0) {;}
    register_value = SPDR;

    HAL_SS_HIGH(); /* End the transaction by pulling the Slave Select High. */

    AVR_LEAVE_CRITICAL_REGION();

    return register_value;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function writes a new value to one of the radio transceiver's
 *          registers.
 *
 *  \see Look at the at86rf230_registermap.h file for register address definitions.
 *
 *  \param  address Address of register to write.
 *  \param  value   Value to write.
 */
void
hal_register_write(uint8_t address, uint8_t value)
{
    /* Add the Register Write command to the address. */
    address = HAL_TRX_CMD_RW | (HAL_TRX_CMD_RADDRM & address);

    AVR_ENTER_CRITICAL_REGION();

    HAL_SS_LOW(); /* Start the SPI transaction by pulling the Slave Select low. */

    /*Send Register address and write register content.*/
    SPDR = address;
    while ((SPSR & (1 << SPIF)) == 0) {;}
    uint8_t dummy_read = SPDR;

    SPDR = value;
    while ((SPSR & (1 << SPIF)) == 0) {;}
    dummy_read = SPDR;

    HAL_SS_HIGH(); /* End the transaction by pulling the Slave Slect High. */

    AVR_LEAVE_CRITICAL_REGION();
}

/*----------------------------------------------------------------------------*/
/** \brief  This function reads the value of a specific subregister.
 *
 *  \see Look at the at86rf230_registermap.h file for register and subregister
 *       definitions.
 *
 *  \param  address  Main register's address.
 *  \param  mask  Bit mask of the subregister.
 *  \param  position   Bit position of the subregister
 *  \retval Value of the read subregister.
 */
uint8_t
hal_subregister_read(uint8_t address, uint8_t mask, uint8_t position)
{
    /* Read current register value and mask out subregister. */
    uint8_t register_value = hal_register_read(address);
    register_value &= mask;
    register_value >>= position; /* Align subregister value. */

    return register_value;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function writes a new value to one of the radio transceiver's
 *          subregisters.
 *
 *  \see Look at the at86rf230_registermap.h file for register and subregister
 *       definitions.
 *
 *  \param  address  Main register's address.
 *  \param  mask  Bit mask of the subregister.
 *  \param  position  Bit position of the subregister
 *  \param  value  Value to write into the subregister.
 */
void
hal_subregister_write(uint8_t address, uint8_t mask, uint8_t position,
                            uint8_t value)
{
    /* Read current register value and mask area outside the subregister. */
    uint8_t register_value = hal_register_read(address);
    register_value &= ~mask;

    /* Start preparing the new subregister value. shift in place and mask. */
    value <<= position;
    value &= mask;

    value |= register_value; /* Set the new subregister value. */

    /* Write the modified register value. */
    hal_register_write(address, value);
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will upload a frame from the radio transceiver's frame
 *          buffer.
 *
 *          If the frame currently available in the radio transceiver's frame buffer
 *          is out of the defined bounds. Then the frame length, lqi value and crc
 *          be set to zero. This is done to indicate an error.
 *
 *  \param  rx_frame    Pointer to the data structure where the frame is stored.
 *  \param  rx_callback Pointer to callback function for receiving one byte at a time.
 */
void
hal_frame_read(hal_rx_frame_t *rx_frame, rx_callback_t rx_callback)
{
    uint8_t *rx_data=0;

    /*  check that we have either valid frame pointer or callback pointer */
    if (!rx_frame && !rx_callback)
        return;

    AVR_ENTER_CRITICAL_REGION();

    HAL_SS_LOW();

    /*Send frame read command.*/
    SPDR = HAL_TRX_CMD_FR;
    while ((SPSR & (1 << SPIF)) == 0) {;}
    uint8_t frame_length = SPDR;

    /*Read frame length.*/
    SPDR = frame_length;
    while ((SPSR & (1 << SPIF)) == 0) {;}
    frame_length = SPDR;

    /*Check for correct frame length.*/
    if ((frame_length >= HAL_MIN_FRAME_LENGTH) && (frame_length <= HAL_MAX_FRAME_LENGTH)){
        uint16_t crc = 0;
        if (rx_frame){
            rx_data = (rx_frame->data);
            rx_frame->length = frame_length; /* Store frame length. */
        } else {
            rx_callback(frame_length);
        }
        /*Upload frame buffer to data pointer. Calculate CRC.*/
        SPDR = frame_length;
        while ((SPSR & (1 << SPIF)) == 0) {;}

        do{
            uint8_t tempData = SPDR;
            SPDR = 0;       /*  dummy write */

            if (rx_frame){
                *rx_data++ = tempData;
            } else {
                rx_callback(tempData);
            }

            crc = _crc_ccitt_update(crc, tempData);

            while ((SPSR & (1 << SPIF)) == 0) {;}

        } while (--frame_length > 0);

        /*Read LQI value for this frame.*/
        if (rx_frame){
            rx_frame->lqi = SPDR;
        } else {
            rx_callback(SPDR);
        }
        
        HAL_SS_HIGH();

        /*Check calculated crc, and set crc field in hal_rx_frame_t accordingly.*/
        if (rx_frame){
            rx_frame->crc = (crc == HAL_CALCULATED_CRC_OK);
        } else {
            rx_callback(crc != HAL_CALCULATED_CRC_OK);
        }
    } else {
        HAL_SS_HIGH();

        if (rx_frame){
            rx_frame->length = 0;
            rx_frame->lqi    = 0;
            rx_frame->crc    = false;
        }
    }

    AVR_LEAVE_CRITICAL_REGION();
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will download a frame to the radio transceiver's frame
 *          buffer.
 *
 *  \param  write_buffer    Pointer to data that is to be written to frame buffer.
 *  \param  length          Length of data. The maximum length is 127 bytes.
 */
void
hal_frame_write(uint8_t *write_buffer, uint8_t length)
{
    length &= HAL_TRX_CMD_RADDRM; /* Truncate length to maximum frame length. */

    AVR_ENTER_CRITICAL_REGION();

    HAL_SS_LOW(); /* Initiate the SPI transaction. */

    /*SEND FRAME WRITE COMMAND AND FRAME LENGTH.*/
    SPDR = HAL_TRX_CMD_FW;
    while ((SPSR & (1 << SPIF)) == 0) {;}
    uint8_t dummy_read = SPDR;

    SPDR = length;
    while ((SPSR & (1 << SPIF)) == 0) {;}
    dummy_read = SPDR;

    /* Download to the Frame Buffer. */
    do{
        SPDR = *write_buffer++;
        --length;

        while ((SPSR & (1 << SPIF)) == 0) {;}

        dummy_read = SPDR;
    } while (length > 0);

    HAL_SS_HIGH(); /* Terminate SPI transaction. */

    AVR_LEAVE_CRITICAL_REGION();
}

/*----------------------------------------------------------------------------*/
/** \brief Read SRAM
 *
 * This function reads from the SRAM of the radio transceiver.
 *
 * \param address Address in the TRX's SRAM where the read burst should start
 * \param length Length of the read burst
 * \param data Pointer to buffer where data is stored.
 */
void
hal_sram_read(uint8_t address, uint8_t length, uint8_t *data)
{
    AVR_ENTER_CRITICAL_REGION();

    HAL_SS_LOW(); /* Initiate the SPI transaction. */

    /*Send SRAM read command.*/
    SPDR = HAL_TRX_CMD_SR;
    while ((SPSR & (1 << SPIF)) == 0) {;}
    uint8_t dummy_read = SPDR;

    /*Send address where to start reading.*/
    SPDR = address;
    while ((SPSR & (1 << SPIF)) == 0) {;}

    dummy_read = SPDR;

    /*Upload the chosen memory area.*/
    do{
        SPDR = HAL_DUMMY_READ;
        while ((SPSR & (1 << SPIF)) == 0) {;}
        *data++ = SPDR;
    } while (--length > 0);

    HAL_SS_HIGH();

    AVR_LEAVE_CRITICAL_REGION();
}

/*----------------------------------------------------------------------------*/
/** \brief Write SRAM
 *
 * This function writes into the SRAM of the radio transceiver.
 *
 * \param address Address in the TRX's SRAM where the write burst should start
 * \param length  Length of the write burst
 * \param data    Pointer to an array of bytes that should be written
 */
void
hal_sram_write(uint8_t address, uint8_t length, uint8_t *data)
{
    AVR_ENTER_CRITICAL_REGION();

    HAL_SS_LOW();

    /*Send SRAM write command.*/
    SPDR = HAL_TRX_CMD_SW;
    while ((SPSR & (1 << SPIF)) == 0) {;}
    uint8_t dummy_read = SPDR;

    /*Send address where to start writing to.*/
    SPDR = address;
    while ((SPSR & (1 << SPIF)) == 0) {;}
    dummy_read = SPDR;

    /*Upload the chosen memory area.*/
    do{
        SPDR = *data++;
        while ((SPSR & (1 << SPIF)) == 0) {;}
        dummy_read = SPDR;
    } while (--length > 0);

    HAL_SS_HIGH();

    AVR_LEAVE_CRITICAL_REGION();
}

/*----------------------------------------------------------------------------*/
/* This #if compile switch is used to provide a "standard" function body for the */
/* doxygen documentation. */
#if defined(DOXYGEN)
/** \brief ISR for the radio IRQ line, triggered by the input capture.
 *  This is the interrupt service routine for timer1.ICIE1 input capture.
 *  It is triggered of a rising edge on the radio transceivers IRQ line.
 */
void RADIO_VECT(void);
#else  /* !DOXYGEN */
ISR(RADIO_VECT)
{
    /*The following code reads the current system time. This is done by first
      reading the hal_system_time and then adding the 16 LSB directly from the
      TCNT1 register.
     */
    uint32_t isr_timestamp = hal_system_time;
    isr_timestamp <<= 16;
    isr_timestamp |= TCNT1;

    /*Read Interrupt source.*/
    HAL_SS_LOW();

    /*Send Register address and read register content.*/
    SPDR = RG_IRQ_STATUS | HAL_TRX_CMD_RR;

    /* This is the second part of the convertion of system time to a 16 us time
       base. The division is moved here so we can spend less time waiting for SPI
       data.
     */
    isr_timestamp /= HAL_US_PER_SYMBOL; /* Divide so that we get time in 16us resolution. */
    isr_timestamp &= HAL_SYMBOL_MASK;

    while ((SPSR & (1 << SPIF)) == 0) {;}
    uint8_t interrupt_source = SPDR; /* The interrupt variable is used as a dummy read. */

    SPDR = interrupt_source;
    while ((SPSR & (1 << SPIF)) == 0) {;}
    interrupt_source = SPDR; /* The interrupt source is read. */

    HAL_SS_HIGH();

    /*Handle the incomming interrupt. Prioritized.*/
    if ((interrupt_source & HAL_RX_START_MASK)){
        if(rx_start_callback != NULL){
            /* Read Frame length and call rx_start callback. */
            HAL_SS_LOW();

            SPDR = HAL_TRX_CMD_FR;
            while ((SPSR & (1 << SPIF)) == 0) {;}
            uint8_t frame_length = SPDR;

            SPDR = frame_length; /*  frame_length used for dummy data */
            while ((SPSR & (1 << SPIF)) == 0) {;}
            frame_length = SPDR;

            HAL_SS_HIGH();

            rx_start_callback(isr_timestamp, frame_length);
        }
    } else if (interrupt_source & HAL_TRX_END_MASK){
        if(trx_end_callback != NULL){
            trx_end_callback(isr_timestamp);
        }
    } else if (interrupt_source & HAL_TRX_UR_MASK){
        ;
    } else if (interrupt_source & HAL_PLL_UNLOCK_MASK){
        ;
    } else if (interrupt_source & HAL_PLL_LOCK_MASK){
        hal_pll_lock_flag++;
        ;
    } else if (interrupt_source & HAL_BAT_LOW_MASK){
        /*  Disable BAT_LOW interrupt to prevent endless interrupts. The interrupt */
        /*  will continously be asserted while the supply voltage is less than the */
        /*  user-defined voltage threshold. */
        uint8_t trx_isr_mask = hal_register_read(RG_IRQ_MASK);
        trx_isr_mask &= ~HAL_BAT_LOW_MASK;
        hal_register_write(RG_IRQ_MASK, trx_isr_mask);
        hal_bat_low_flag++; /* Increment BAT_LOW flag. */
    } else {
        ;
    }
}
#   endif /* defined(DOXYGEN) */

/*----------------------------------------------------------------------------*/
/* This #if compile switch is used to provide a "standard" function body for the */
/* doxygen documentation. */
#if defined(DOXYGEN)
/** \brief Timer Overflow ISR
 * This is the interrupt service routine for timer1 overflow.
 */
void TIMER1_OVF_vect(void);
#else  /* !DOXYGEN */
ISR(TIMER1_OVF_vect)
{
    hal_system_time++;
}
#endif

/** @} */
/** @} */

/*EOF*/
