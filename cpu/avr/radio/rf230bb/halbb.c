/*   Copyright (c) 2009, Swedish Institute of Computer Science
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
 *	David Kopf dak664@embarqmail.com
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
 *  This version is optimized for use with the "barebones" RF230bb driver,
 *  which communicates directly with the contiki core MAC layer.
 *  It is optimized for speed at the expense of generality.
 */



/*============================ INCLUDE =======================================*/
#include <stdlib.h>

#include "hal.h"

#if defined(__AVR_ATmega128RFA1__)
#include <avr/io.h>
#include "atmega128rfa1_registermap.h"
#else
#include "at86rf230_registermap.h"
#endif

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
volatile extern signed char rf230_last_rssi;

//static uint8_t volatile hal_bat_low_flag;
//static uint8_t volatile hal_pll_lock_flag;

/*============================ CALLBACKS =====================================*/

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
//static hal_rx_start_isr_event_handler_t rx_start_callback;

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
//static hal_trx_end_isr_event_handler_t trx_end_callback;

/*============================ IMPLEMENTATION ================================*/
#if defined(__AVR_ATmega128RFA1__)
//#include <avr/io.h>
#include <avr/interrupt.h>
/* AVR1281 with internal RF231 radio */
#define HAL_SPI_TRANSFER_OPEN() 
//#define HAL_SPI_TRANSFER_WRITE(to_write) (SPDR = (to_write))
#define HAL_SPI_TRANSFER_WAIT()
#define HAL_SPI_TRANSFER_READ() (SPDR)
#define HAL_SPI_TRANSFER_CLOSE()
#if 0
#define HAL_SPI_TRANSFER(to_write) (	  \
				    HAL_SPI_TRANSFER_WRITE(to_write),	\
				    HAL_SPI_TRANSFER_WAIT(),		\
				    HAL_SPI_TRANSFER_READ() )
#endif
#elif defined(__AVR__)
/*
 * AVR with hardware SPI tranfers (TODO: move to hw spi hal for avr cpu)
 */
#include <avr/io.h>
#include <avr/interrupt.h>

#define HAL_SPI_TRANSFER_OPEN() { \
  HAL_ENTER_CRITICAL_REGION();	  \
  HAL_SS_LOW(); /* Start the SPI transaction by pulling the Slave Select low. */
#define HAL_SPI_TRANSFER_WRITE(to_write) (SPDR = (to_write))
#define HAL_SPI_TRANSFER_WAIT() ({while ((SPSR & (1 << SPIF)) == 0) {;}}) /* gcc extension, alternative inline function */
#define HAL_SPI_TRANSFER_READ() (SPDR)
#define HAL_SPI_TRANSFER_CLOSE() \
    HAL_SS_HIGH(); /* End the transaction by pulling the Slave Select High. */ \
    HAL_LEAVE_CRITICAL_REGION(); \
    }
#define HAL_SPI_TRANSFER(to_write) (	  \
				    HAL_SPI_TRANSFER_WRITE(to_write),	\
				    HAL_SPI_TRANSFER_WAIT(),		\
				    HAL_SPI_TRANSFER_READ() )

#else /* __AVR__ */
/*
 * Other SPI architecture (parts to core, parts to m16c6Xp 
 */
#include "contiki-mulle.h" // MULLE_ENTER_CRITICAL_REGION

// Software SPI transfers
#define HAL_SPI_TRANSFER_OPEN() { uint8_t spiTemp; \
  HAL_ENTER_CRITICAL_REGION();	  \
  HAL_SS_LOW(); /* Start the SPI transaction by pulling the Slave Select low. */
#define HAL_SPI_TRANSFER_WRITE(to_write) (spiTemp = spiWrite(to_write))
#define HAL_SPI_TRANSFER_WAIT()  ({0;})
#define HAL_SPI_TRANSFER_READ() (spiTemp)
#define HAL_SPI_TRANSFER_CLOSE() \
    HAL_SS_HIGH(); /* End the transaction by pulling the Slave Select High. */ \
    HAL_LEAVE_CRITICAL_REGION(); \
    }
#define HAL_SPI_TRANSFER(to_write) (spiTemp = spiWrite(to_write))

inline uint8_t spiWrite(uint8_t byte)
{
    uint8_t data = 0;
    uint8_t mask = 0x80;
    do
    {
        if( (byte & mask) != 0 )
            HAL_PORT_MOSI |= (1 << HAL_MOSI_PIN); //call MOSI.set();
        else
            HAL_PORT_MOSI &= ~(1 << HAL_MOSI_PIN); //call MOSI.clr();

        if( (HAL_PORT_MISO & (1 << HAL_MISO_PIN)) > 0) //call MISO.get() )
            data |= mask;

        HAL_PORT_SCK &= ~(1 << HAL_SCK_PIN); //call SCLK.clr();
        HAL_PORT_SCK |= (1 << HAL_SCK_PIN); //call SCLK.set();
    } while( (mask >>= 1) != 0 );
    return data;
}

#endif  /* !__AVR__ */
 
/** \brief  This function initializes the Hardware Abstraction Layer.
 */
#if defined(__AVR_ATmega128RFA1__)
//#define HAL_RF230_ISR() ISR(RADIO_VECT)
#define HAL_TIME_ISR()  ISR(TIMER1_OVF_vect)
#define HAL_TICK_UPCNT() (TCNT1)
void
hal_init(void)
{
    /*Reset variables used in file.*/
    hal_system_time = 0;
 //   TCCR1B = HAL_TCCR1B_CONFIG;       /* Set clock prescaler */
 //   TIFR1 |= (1 << ICF1);             /* Clear Input Capture Flag. */
 //   HAL_ENABLE_OVERFLOW_INTERRUPT(); /* Enable Timer1 overflow interrupt. */
    hal_enable_trx_interrupt();    /* Enable interrupts from the radio transceiver. */
}

#elif defined(__AVR__)
#define HAL_RF230_ISR() ISR(RADIO_VECT)
#define HAL_TIME_ISR()  ISR(TIMER1_OVF_vect)
#define HAL_TICK_UPCNT() (TCNT1)
void
hal_init(void)
{
    /*Reset variables used in file.*/
    hal_system_time = 0;
//  hal_reset_flags();

    /*IO Specific Initialization - sleep and reset pins. */
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

#else /* __AVR__ */

#define HAL_RF230_ISR() M16C_INTERRUPT(M16C_INT1)
#define HAL_TIME_ISR()  M16C_INTERRUPT(M16C_TMRB4)
#define HAL_TICK_UPCNT() (0xFFFF-TB4) // TB4 counts down so we need to convert it to upcounting

void
hal_init(void)
{
    /*Reset variables used in file.*/
    hal_system_time = 0;
//  hal_reset_flags();

    /*IO Specific Initialization - sleep and reset pins. */
    DDR_SLP_TR |= (1 << SLP_TR); /* Enable SLP_TR as output. */
    DDR_RST    |= (1 << RST);    /* Enable RST as output. */

    /*SPI Specific Initialization.*/
    /* Set SS, CLK and MOSI as output. */
    HAL_DDR_SS  |= (1 << HAL_SS_PIN);
    HAL_DDR_SCK  |= (1 << HAL_SCK_PIN);
    HAL_DDR_MOSI  |= (1 << HAL_MOSI_PIN);
    HAL_DDR_MISO  &= ~(1 << HAL_MISO_PIN);

    /* Set SS */
    HAL_PORT_SS |= (1 << HAL_SS_PIN); // HAL_SS_HIGH()
    HAL_PORT_SCK &= ~(1 << HAL_SCK_PIN); // SCLK.clr()

    /*TIMER Specific Initialization.*/
    // Init count source (Timer B3)
    TB3 = ((16*10) - 1); // 16 us ticks
    TB3MR.BYTE = 0b00000000; // Timer mode, F1
    TBSR.BIT.TB3S = 1; // Start Timer B3

    TB4 = 0xFFFF; //
    TB4MR.BYTE = 0b10000001; // Counter mode, count TB3
    TBSR.BIT.TB4S = 1; // Start Timer B4
    INT1IC.BIT.POL = 1; // Select rising edge
    HAL_ENABLE_OVERFLOW_INTERRUPT(); /* Enable Timer overflow interrupt. */
    hal_enable_trx_interrupt();    /* Enable interrupts from the radio transceiver. */
}
#endif  /* !__AVR__ */

/*----------------------------------------------------------------------------*/
/** \brief  This function reset the interrupt flags and interrupt event handlers
 *          (Callbacks) to their default value.
 */
//void
//hal_reset_flags(void)
//{
//    HAL_ENTER_CRITICAL_REGION();

    /* Reset Flags. */
//    hal_bat_low_flag     = 0;
//    hal_pll_lock_flag    = 0;

    /* Reset Associated Event Handlers. */
//    rx_start_callback = NULL;
//    trx_end_callback  = NULL;

//    HAL_LEAVE_CRITICAL_REGION();
//}

/*----------------------------------------------------------------------------*/
/** \brief  This function returns the current value of the BAT_LOW flag.
 *
 *  The BAT_LOW flag is incremented each time a BAT_LOW event is signaled from the
 *  radio transceiver. This way it is possible for the end user to poll the flag
 *  for new event occurances.
 */
//uint8_t
//hal_get_bat_low_flag(void)
//{
//    return hal_bat_low_flag;
//}

/*----------------------------------------------------------------------------*/
/** \brief  This function clears the BAT_LOW flag.
 */
//void
//hal_clear_bat_low_flag(void)
//{
//    HAL_ENTER_CRITICAL_REGION();
//    hal_bat_low_flag = 0;
//    HAL_LEAVE_CRITICAL_REGION();
//}

/*----------------------------------------------------------------------------*/
/** \brief  This function is used to set new TRX_END event handler, overriding
 *          old handler reference.
 */
//hal_trx_end_isr_event_handler_t
//hal_get_trx_end_event_handler(void)
//{
//    return trx_end_callback;
//}

/*----------------------------------------------------------------------------*/
/** \brief  This function is used to set new TRX_END event handler, overriding
 *          old handler reference.
 */
//void
//hal_set_trx_end_event_handler(hal_trx_end_isr_event_handler_t trx_end_callback_handle)
//{
//    HAL_ENTER_CRITICAL_REGION();
//    trx_end_callback = trx_end_callback_handle;
//    HAL_LEAVE_CRITICAL_REGION();
//}

/*----------------------------------------------------------------------------*/
/** \brief  Remove event handler reference.
 */
//void
//hal_clear_trx_end_event_handler(void)
//{
//    HAL_ENTER_CRITICAL_REGION();
//    trx_end_callback = NULL;
//    HAL_LEAVE_CRITICAL_REGION();
//}

/*----------------------------------------------------------------------------*/
/** \brief  This function returns the active RX_START event handler
 *
 *  \return Current RX_START event handler registered.
 */
//hal_rx_start_isr_event_handler_t
//hal_get_rx_start_event_handler(void)
//{
//    return rx_start_callback;
//}

/*----------------------------------------------------------------------------*/
/** \brief  This function is used to set new RX_START event handler, overriding
 *          old handler reference.
 */
//void
//hal_set_rx_start_event_handler(hal_rx_start_isr_event_handler_t rx_start_callback_handle)
//{
//    HAL_ENTER_CRITICAL_REGION();
//    rx_start_callback = rx_start_callback_handle;
//    HAL_LEAVE_CRITICAL_REGION();
//}

/*----------------------------------------------------------------------------*/
/** \brief  Remove event handler reference.
 */
//void
//hal_clear_rx_start_event_handler(void)
//{
//    HAL_ENTER_CRITICAL_REGION();
//    rx_start_callback = NULL;
//    HAL_LEAVE_CRITICAL_REGION();
//}

/*----------------------------------------------------------------------------*/
/** \brief  This function returns the current value of the PLL_LOCK flag.
 *
 *  The PLL_LOCK flag is incremented each time a PLL_LOCK event is signaled from the
 *  radio transceiver. This way it is possible for the end user to poll the flag
 *  for new event occurances.
 */
//uint8_t
//hal_get_pll_lock_flag(void)
//{
//    return hal_pll_lock_flag;
//}

/*----------------------------------------------------------------------------*/
/** \brief  This function clears the PLL_LOCK flag.
 */
//void
//hal_clear_pll_lock_flag(void)
//{
//    HAL_ENTER_CRITICAL_REGION();
//    hal_pll_lock_flag = 0;
//    HAL_LEAVE_CRITICAL_REGION();
//}

#if defined(__AVR_ATmega128RFA1__)
/* Hack for internal radio registers. hal_register_read and hal_register_write are
   handled through defines, but the preprocesser can't parse a macro containing
   another #define with multiple arguments, e.g. using
   #define hal_subregister_read( address, mask, position ) (address&mask)>>position
   #define SR_TRX_STATUS         TRX_STATUS, 0x1f, 0
   the following only sees 1 argument to the macro
   return hal_subregister_read(SR_TRX_STATUS);
   
   Possible fix is through two defines:
   #define x_hal_subregister_read(x) hal_subregister_read(x);
   #define hal_subregister_read( address, mask, position ) (address&mask)>>position
   but the subregister defines in atmega128rfa1_registermap.h are currently set up without
   the _SFR_MEM8 attribute, for use by hal_subregister_write.
   
 */
uint8_t
hal_subregister_read(uint16_t address, uint8_t mask, uint8_t position)
{
    return (_SFR_MEM8(address)&mask)>>position;
}
void
hal_subregister_write(uint16_t address, uint8_t mask, uint8_t position,
                            uint8_t value)
{
 cli();
    uint8_t register_value = _SFR_MEM8(address);
    register_value &= ~mask;
    value <<= position;
    value &= mask;
    value |= register_value;
    _SFR_MEM8(address) = value;
 sei();
}

#else /* defined(__AVR_ATmega128RFA1__) */
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
    uint8_t register_value;
    /* Add the register read command to the register address. */
    /* Address should be < 0x2f so no need to mask */
//  address &= 0x3f;
    address |= 0x80;

    HAL_SPI_TRANSFER_OPEN();

    /*Send Register address and read register content.*/
    HAL_SPI_TRANSFER(address);
    register_value = HAL_SPI_TRANSFER(0);

    HAL_SPI_TRANSFER_CLOSE();

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
    /* Add the Register Write (short mode) command to the address. */
    address = 0xc0 | address;

    HAL_SPI_TRANSFER_OPEN();

    /*Send Register address and write register content.*/
    HAL_SPI_TRANSFER(address);
    HAL_SPI_TRANSFER(value);

    HAL_SPI_TRANSFER_CLOSE();
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
    volatile uint8_t register_value = hal_register_read(address);
    register_value &= ~mask;

    /* Start preparing the new subregister value. shift in place and mask. */
    value <<= position;
    value &= mask;

    value |= register_value; /* Set the new subregister value. */

    /* Write the modified register value. */
    hal_register_write(address, value);
}
#endif /* defined(__AVR_ATmega128RFA1__) */
/*----------------------------------------------------------------------------*/
/** \brief  This function will upload a frame from the radio transceiver's frame
 *          buffer.
 *
 *          If the frame currently available in the radio transceiver's frame buffer
 *          is out of the defined bounds. Then the frame length, lqi value and crc
 *          be set to zero. This is done to indicate an error.
 *          This version is optimized for use with contiki RF230BB driver.
 *          The callback routine and CRC are left out for speed in reading the rx buffer.
 *          Any delays here can lead to overwrites by the next packet!
 *
 *  \param  rx_frame    Pointer to the data structure where the frame is stored.
 *  \param  rx_callback Pointer to callback function for receiving one byte at a time.
 */
void
//hal_frame_read(hal_rx_frame_t *rx_frame, rx_callback_t rx_callback)
hal_frame_read(hal_rx_frame_t *rx_frame)
{
#if defined(__AVR_ATmega128RFA1__)

    uint8_t frame_length,*rx_data,*rx_buffer;
  
    rx_data = (rx_frame->data);
    frame_length =  TST_RX_LENGTH;  //frame length, not including lqi?
    rx_frame->length = frame_length;
    rx_buffer=(uint8_t *)0x180;  //start of fifo in i/o space

    do{
        *rx_data++ = _SFR_MEM8(rx_buffer++);

    } while (--frame_length > 0);

    /*Read LQI value for this frame.*/
    rx_frame->lqi = *rx_buffer;
    if (1) {  
    
#else /* defined(__AVR_ATmega128RFA1__) */

    uint8_t *rx_data;

    /*  check that we have either valid frame pointer or callback pointer */
//  if (!rx_frame && !rx_callback)
//      return;

    HAL_SPI_TRANSFER_OPEN();

    /*Send frame read (long mode) command.*/
    HAL_SPI_TRANSFER(0x20);

    /*Read frame length. This includes the checksum. */
    uint8_t frame_length = HAL_SPI_TRANSFER(0);

    /*Check for correct frame length.*/
//   if ((frame_length >= HAL_MIN_FRAME_LENGTH) && (frame_length <= HAL_MAX_FRAME_LENGTH)){
     if (1) {
//      uint16_t crc = 0;
//      if (rx_frame){
            rx_data = (rx_frame->data);
            rx_frame->length = frame_length;
//      } else {
//          rx_callback(frame_length);
//      }
        /*Upload frame buffer to data pointer */

	    HAL_SPI_TRANSFER_WRITE(0);
	    HAL_SPI_TRANSFER_WAIT();

        do{
            *rx_data++ = HAL_SPI_TRANSFER_READ();
            HAL_SPI_TRANSFER_WRITE(0);

//           if (rx_frame){
//             *rx_data++ = tempData;
//          } else {
//              rx_callback(tempData);
//          }
/* RF230 does crc in hardware, doing the checksum here ensures the rx buffer has not been overwritten by the next packet */
/* Since doing the checksum makes such overwrites more probable, we skip it and hope for the best. */
/* A full buffer should be read in 320us at 2x spi clocking, so with a low interrupt latency overwrites should not occur */
//         crc = _crc_ccitt_update(crc, tempData);

	    HAL_SPI_TRANSFER_WAIT();

        } while (--frame_length > 0);

        /*Read LQI value for this frame.*/
//      if (rx_frame){
	    rx_frame->lqi = HAL_SPI_TRANSFER_READ();
//      } else {
//          rx_callback(HAL_SPI_TRANSFER_READ());
//      }
        
#endif /* defined(__AVR_ATmega128RFA1__) */

        /*Check calculated crc, and set crc field in hal_rx_frame_t accordingly.*/
//      if (rx_frame){
            rx_frame->crc = 1;
//      } else {
//          rx_callback(crc != 0);
//      }
    } else {
//      if (rx_frame){
            rx_frame->length = 0;
            rx_frame->lqi    = 0;
            rx_frame->crc    = false;
//      }
    }

    HAL_SPI_TRANSFER_CLOSE();
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
#if defined(__AVR_ATmega128RFA1__)
    uint8_t *tx_buffer;
    tx_buffer=(uint8_t *)0x180;  //start of fifo in i/o space
    /* Write frame length, including the two byte checksum */
    /* The top bit of the length field shall be set to 0 for IEEE 802.15.4 compliant frames */
    /* It should already be clear, so bypassing the masking is sanity check of the uip stack */
//  length &= 0x7f;
    _SFR_MEM8(tx_buffer++) = length;
    
    /* Download to the Frame Buffer.
     * When the FCS is autogenerated there is no need to transfer the last two bytes
     * since they will be overwritten.
     */
#if !RF230_CONF_CHECKSUM
    length -= 2;
#endif
    do  _SFR_MEM8(tx_buffer++)= *write_buffer++; while (--length);

#else /* defined(__AVR_ATmega128RFA1__) */
    /* Optionally truncate length to maximum frame length.
     * Not doing this is a fast way to know when the application needs fixing!
     */
//  length &= 0x7f; 

    HAL_SPI_TRANSFER_OPEN();

    /* Send Frame Transmit (long mode) command and frame length */
    HAL_SPI_TRANSFER(0x60);
    HAL_SPI_TRANSFER(length);

    /* Download to the Frame Buffer.
     * When the FCS is autogenerated there is no need to transfer the last two bytes
     * since they will be overwritten.
     */
#if !RF230_CONF_CHECKSUM
    length -= 2;
#endif
    do HAL_SPI_TRANSFER(*write_buffer++); while (--length);

    HAL_SPI_TRANSFER_CLOSE();
#endif /* defined(__AVR_ATmega128RFA1__) */
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
//void
//hal_sram_read(uint8_t address, uint8_t length, uint8_t *data)
//{
//    HAL_SPI_TRANSFER_OPEN();

    /*Send SRAM read command.*/
//    HAL_SPI_TRANSFER(0x00);

    /*Send address where to start reading.*/
//    HAL_SPI_TRANSFER(address);

    /*Upload the chosen memory area.*/
//    do{
//        *data++ = HAL_SPI_TRANSFER(0);
//    } while (--length > 0);

//    HAL_SPI_TRANSFER_CLOSE();

//}

/*----------------------------------------------------------------------------*/
/** \brief Write SRAM
 *
 * This function writes into the SRAM of the radio transceiver. It can reduce
 * SPI transfers if only part of a frame is to be changed before retransmission.
 *
 * \param address Address in the TRX's SRAM where the write burst should start
 * \param length  Length of the write burst
 * \param data    Pointer to an array of bytes that should be written
 */
//void
//hal_sram_write(uint8_t address, uint8_t length, uint8_t *data)
//{
//    HAL_SPI_TRANSFER_OPEN();

    /*Send SRAM write command.*/
//    HAL_SPI_TRANSFER(0x40);

    /*Send address where to start writing to.*/
//    HAL_SPI_TRANSFER(address);

    /*Upload the chosen memory area.*/
//    do{
//        HAL_SPI_TRANSFER(*data++);
//    } while (--length > 0);

//    HAL_SPI_TRANSFER_CLOSE();

//}

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
/* These link to the RF230BB driver in rf230.c */
void rf230_interrupt(void);

extern hal_rx_frame_t rxframe[RF230_CONF_RX_BUFFERS];
extern uint8_t rxframe_head,rxframe_tail;

/* rf230interruptflag can be printed in the main idle loop for debugging */
#define DEBUG 0
#if DEBUG
volatile char rf230interruptflag;
#define INTERRUPTDEBUG(arg) rf230interruptflag=arg
#else
#define INTERRUPTDEBUG(arg)
#endif

#if defined(__AVR_ATmega128RFA1__)
/* The atmega128rfa1 has individual interrupts for the integrated radio */
/* Whichever are enabled by the RF230 driver must be present even if not used! */
ISR(TRX24_RX_END_vect)
{
	   INTERRUPTDEBUG(11);	    	    
       /* Received packet interrupt */ 
       /* Buffer the frame and call rf230_interrupt to schedule poll for rf230 receive process */
//         if (rxframe.length) break;			//toss packet if last one not processed yet
         if (rxframe[rxframe_tail].length) INTERRUPTDEBUG(42); else INTERRUPTDEBUG(12);

#ifdef RF230_MIN_RX_POWER		 
       /* Discard packets weaker than the minimum if defined. This is for testing miniature meshes.*/
       /* Save the rssi for printing in the main loop */
#if RF230_CONF_AUTOACK
 //       rf230_last_rssi=hal_subregister_read(SR_ED_LEVEL);
        rf230_last_rssi=hal_register_read(RG_PHY_ED_LEVEL);
#endif
//      if (rf230_last_rssi >= RF230_MIN_RX_POWER) {
        if (1) {        
#endif
         hal_frame_read(&rxframe[rxframe_tail]);
         rxframe_tail++;if (rxframe_tail >= RF230_CONF_RX_BUFFERS) rxframe_tail=0;
         rf230_interrupt();
#ifdef RF230_MIN_RX_POWER
        }
#endif
}
ISR(TRX24_RX_START_vect)
{
    INTERRUPTDEBUG(10);
    /* Save RSSI for this packet if not in extended mode, scaling to 1dB resolution */
#if !RF230_CONF_AUTOACK
    rf230_last_rssi = 3 * hal_subregister_read(SR_RSSI);
#endif

}
ISR(TRX24_PLL_LOCK_vect)
{
}
ISR(TRX24_PLL_UNLOCK_vect)
{
}

#if 0
HAL_RF230_ISR() //for reference, for now
{
    /*The following code reads the current system time. This is done by first
      reading the hal_system_time and then adding the 16 LSB directly from the
      hardware counter.
     */
//    uint32_t isr_timestamp = hal_system_time;
//    isr_timestamp <<= 16;
//    isr_timestamp |= HAL_TICK_UPCNT(); // TODO: what if this wraps after reading hal_system_time?
//   isr_timestamp /= HAL_US_PER_SYMBOL; /* Divide so that we get time in 16us resolution. */
//   isr_timestamp &= HAL_SYMBOL_MASK;

    uint8_t interrupt_source;

    INTERRUPTDEBUG(1);

    /*Read Interrupt source.*/
    interrupt_source = IRQ_STATUS;

    /*Handle the incomming interrupt. Prioritized.*/
    if (interrupt_source & (1>>RX_START)){
	   INTERRUPTDEBUG(10);
    /* Save RSSI for this packet if not in extended mode, scaling to 1dB resolution */
#if !RF230_CONF_AUTOACK
       rf230_last_rssi = 3 * hal_subregister_read(SR_RSSI);
#endif

    } else if (interrupt_source & (1<<RX_END)){
	   INTERRUPTDEBUG(11);	    	    
       /* Received packet interrupt */ 
       /* Buffer the frame and call rf230_interrupt to schedule poll for rf230 receive process */
//         if (rxframe.length) break;			//toss packet if last one not processed yet
         if (rxframe[rxframe_tail].length) INTERRUPTDEBUG(42); else INTERRUPTDEBUG(12);

#ifdef RF230_MIN_RX_POWER		 
       /* Discard packets weaker than the minimum if defined. This is for testing miniature meshes.*/
       /* Save the rssi for printing in the main loop */
#if RF230_CONF_AUTOACK
//        rf230_last_rssi=hal_subregister_read(SR_ED_LEVEL);
        rf230_last_rssi=hal_register_read(RG_PHY_ED_LEVEL);
#endif
//      if (rf230_last_rssi >= RF230_MIN_RX_POWER) {
        if (1) {        
#endif
         hal_frame_read(&rxframe[rxframe_tail]);
         rxframe_tail++;if (rxframe_tail >= RF230_CONF_RX_BUFFERS) rxframe_tail=0;
         rf230_interrupt();
//       trx_end_callback(isr_timestamp);
#ifdef RF230_MIN_RX_POWER
        }
#endif
              
    } else if (interrupt_source & (1<<TX_END)){
        INTERRUPTDEBUG(13);
        ;
    } else if (interrupt_source & (1<<PLL_UNLOCK)){
        INTERRUPTDEBUG(14);
	    ;
    } else if (interrupt_source & (1<<PLL_LOCK)){
        INTERRUPTDEBUG(15);
//      hal_pll_lock_flag++;
        ;
     } else {
        INTERRUPTDEBUG(99);
	    ;
    }
}
#endif
#else /* defined(__AVR_ATmega128RFA1__) */
/* Separate RF230 has a single radio interrupt and the source must be read from the IRQ_STATUS register */
HAL_RF230_ISR()
{
    /*The following code reads the current system time. This is done by first
      reading the hal_system_time and then adding the 16 LSB directly from the
      hardware counter.
     */
//    uint32_t isr_timestamp = hal_system_time;
//    isr_timestamp <<= 16;
//    isr_timestamp |= HAL_TICK_UPCNT(); // TODO: what if this wraps after reading hal_system_time?

    volatile uint8_t state;
    uint8_t interrupt_source; /* used after HAL_SPI_TRANSFER_OPEN/CLOSE block */

    INTERRUPTDEBUG(1);

    
    /* Using SPI bus from ISR is generally a bad idea... */
    /* Note: all IRQ are not always automatically disabled when running in ISR */
    HAL_SPI_TRANSFER_OPEN();

    /*Read Interrupt source.*/
    /*Send Register address and read register content.*/
    HAL_SPI_TRANSFER_WRITE(0x80 | RG_IRQ_STATUS);

    /* This is the second part of the convertion of system time to a 16 us time
       base. The division is moved here so we can spend less time waiting for SPI
       data.
     */
//   isr_timestamp /= HAL_US_PER_SYMBOL; /* Divide so that we get time in 16us resolution. */
//   isr_timestamp &= HAL_SYMBOL_MASK;

    HAL_SPI_TRANSFER_WAIT(); /* AFTER possible interleaved processing */

#if 0 //dak
    interrupt_source = HAL_SPI_TRANSFER_READ(); /* The interrupt variable is used as a dummy read. */

    interrupt_source = HAL_SPI_TRANSFER(interrupt_source);
#else
    interrupt_source = HAL_SPI_TRANSFER(0);
#endif
    HAL_SPI_TRANSFER_CLOSE();

    /*Handle the incomming interrupt. Prioritized.*/
    if ((interrupt_source & HAL_RX_START_MASK)){
	   INTERRUPTDEBUG(10);
    /* Save RSSI for this packet if not in extended mode, scaling to 1dB resolution */
#if !RF230_CONF_AUTOACK
#if 0  // 3-clock shift and add is faster on machines with no hardware multiply
       // While the compiler should use similar code for multiply by 3 there may be a bug with -Os in avr-gcc that calls the general subroutine
        rf230_last_rssi = hal_subregister_read(SR_RSSI);
        rf230_last_rssi = (rf230_last_rssi <<1)  + rf230_last_rssi;
#else  // Faster with 1-clock multiply. Raven and Jackdaw have 2-clock multiply so same speed while saving 2 bytes of program memory
        rf230_last_rssi = 3 * hal_subregister_read(SR_RSSI);
#endif
#endif
//       if(rx_start_callback != NULL){
//            /* Read Frame length and call rx_start callback. */
//            HAL_SPI_TRANSFER_OPEN();
//            uint8_t frame_length = HAL_SPI_TRANSFER(0x20);
//            frame_length = HAL_SPI_TRANSFER(frame_length);

//            HAL_SPI_TRANSFER_CLOSE();

//            rx_start_callback(isr_timestamp, frame_length);
//       }
    } else if (interrupt_source & HAL_TRX_END_MASK){
	   INTERRUPTDEBUG(11);	    	    
//	   if(trx_end_callback != NULL){
//       trx_end_callback(isr_timestamp);
//     }
        
       state = hal_subregister_read(SR_TRX_STATUS);
       if((state == BUSY_RX_AACK) || (state == RX_ON) || (state == BUSY_RX) || (state == RX_AACK_ON)){
       /* Received packet interrupt */ 
       /* Buffer the frame and call rf230_interrupt to schedule poll for rf230 receive process */
//         if (rxframe.length) break;			//toss packet if last one not processed yet
         if (rxframe[rxframe_tail].length) INTERRUPTDEBUG(42); else INTERRUPTDEBUG(12);
 
#ifdef RF230_MIN_RX_POWER		 
       /* Discard packets weaker than the minimum if defined. This is for testing miniature meshes.*/
       /* Save the rssi for printing in the main loop */
#if RF230_CONF_AUTOACK
 //       rf230_last_rssi=hal_subregister_read(SR_ED_LEVEL);
        rf230_last_rssi=hal_register_read(RG_PHY_ED_LEVEL);
#endif
        if (rf230_last_rssi >= RF230_MIN_RX_POWER) {       
#endif
         hal_frame_read(&rxframe[rxframe_tail]);
         rxframe_tail++;if (rxframe_tail >= RF230_CONF_RX_BUFFERS) rxframe_tail=0;
         rf230_interrupt();
//       trx_end_callback(isr_timestamp);
#ifdef RF230_MIN_RX_POWER
        }
#endif

       }
              
    } else if (interrupt_source & HAL_TRX_UR_MASK){
        INTERRUPTDEBUG(13);
        ;
    } else if (interrupt_source & HAL_PLL_UNLOCK_MASK){
        INTERRUPTDEBUG(14);
	    ;
    } else if (interrupt_source & HAL_PLL_LOCK_MASK){
        INTERRUPTDEBUG(15);
//      hal_pll_lock_flag++;
        ;
    } else if (interrupt_source & HAL_BAT_LOW_MASK){
        /*  Disable BAT_LOW interrupt to prevent endless interrupts. The interrupt */
        /*  will continously be asserted while the supply voltage is less than the */
        /*  user-defined voltage threshold. */
        uint8_t trx_isr_mask = hal_register_read(RG_IRQ_MASK);
        trx_isr_mask &= ~HAL_BAT_LOW_MASK;
        hal_register_write(RG_IRQ_MASK, trx_isr_mask);
//      hal_bat_low_flag++; /* Increment BAT_LOW flag. */
        INTERRUPTDEBUG(16);
        ;
     } else {
        INTERRUPTDEBUG(99);
	    ;
    }
}
#endif /* defined(__AVR_ATmega128RFA1__) */ 
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
HAL_TIME_ISR()
{
    hal_system_time++;
}
#endif

/** @} */
/** @} */

/*EOF*/
