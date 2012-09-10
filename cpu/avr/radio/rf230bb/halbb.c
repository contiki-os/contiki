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
 *  Ivan Delamer delamer@ieee.com
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
#include "contiki-conf.h"
#if DEBUGFLOWSIZE
extern uint8_t debugflowsize,debugflow[DEBUGFLOWSIZE];
#define DEBUGFLOW(c) if (debugflowsize<(DEBUGFLOWSIZE-1)) debugflow[debugflowsize++]=c
#else
#define DEBUGFLOW(c)
#endif


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

volatile extern signed char rf230_last_rssi;

/*============================ CALLBACKS =====================================*/


/*============================ IMPLEMENTATION ================================*/
#if defined(__AVR_ATmega128RFA1__)

/* AVR1281 with internal RF231 radio */
#include <avr/interrupt.h>

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

void
hal_init(void)
{
    /*Reset variables used in file.*/
    /* (none at the moment) */
}

#elif defined(__AVR__)

#define HAL_RF230_ISR() ISR(RADIO_VECT)

void
hal_init(void)
{
    /*Reset variables used in file.*/

    /*IO Specific Initialization - sleep and reset pins. */
    /* Set pins low before they are initialized as output? Does not seem to matter */
//  hal_set_rst_low();
//  hal_set_slptr_low();
    DDR_SLP_TR |= (1 << SLP_TR); /* Enable SLP_TR as output. */
    DDR_RST    |= (1 << RST);    /* Enable RST as output. */

    /*SPI Specific Initialization.*/
    /* Set SS, CLK and MOSI as output. */
    /* To avoid a SPI glitch, the port register shall be set before the DDR register */ 
    HAL_PORT_SPI |= (1 << HAL_DD_SS) | (1 << HAL_DD_SCK); /* Set SS and CLK high */
    HAL_DDR_SPI  |= (1 << HAL_DD_SS) | (1 << HAL_DD_SCK) | (1 << HAL_DD_MOSI);
    HAL_DDR_SPI  &=~ (1<< HAL_DD_MISO);                   /* MISO input */ 

    /* Run SPI at max speed */
    SPCR         = (1 << SPE) | (1 << MSTR); /* Enable SPI module and master operation. */
    SPSR         = (1 << SPI2X); /* Enable doubled SPI speed in master mode. */

    /* Enable interrupts from the radio transceiver. */
    hal_enable_trx_interrupt();
}

#else /* __AVR__ */

#define HAL_RF230_ISR() M16C_INTERRUPT(M16C_INT1)
#define HAL_TIME_ISR()  M16C_INTERRUPT(M16C_TMRB4)
#define HAL_TICK_UPCNT() (0xFFFF-TB4) // TB4 counts down so we need to convert it to upcounting

void
hal_init(void)
{
    /*Reset variables used in file.*/

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

    /* Enable interrupts from the radio transceiver. */
    hal_enable_trx_interrupt();
}
#endif  /* !__AVR__ */


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
    HAL_ENTER_CRITICAL_REGION();

    uint8_t register_value = _SFR_MEM8(address);
    register_value &= ~mask;
    value <<= position;
    value &= mask;
    value |= register_value;
    _SFR_MEM8(address) = value;

    HAL_LEAVE_CRITICAL_REGION();
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
/** \brief  Transfer a frame from the radio transceiver to a RAM buffer
 *
 *          This version is optimized for use with contiki RF230BB driver.
 *          The callback routine and CRC are left out for speed in reading the rx buffer.
 *          Any delays here can lead to overwrites by the next packet!
 *
 *          If the frame length is out of the defined bounds, the length, lqi and crc
 *          are set to zero.
 *
 *  \param  rx_frame    Pointer to the data structure where the frame is stored.
 */
void
hal_frame_read(hal_rx_frame_t *rx_frame)
{
#if defined(__AVR_ATmega128RFA1__)

    uint8_t frame_length,*rx_data,*rx_buffer;
 
    /* Get length from the TXT_RX_LENGTH register, not including LQI
     * Bypassing the length check can result in overrun if buffer is < 256 bytes.
     */
    frame_length = TST_RX_LENGTH;
    if ((frame_length < HAL_MIN_FRAME_LENGTH) || (frame_length > HAL_MAX_FRAME_LENGTH)) {
        /* Length test failed */
        rx_frame->length = 0;
        rx_frame->lqi    = 0;
        rx_frame->crc    = false;
        return;
    }
    rx_frame->length = frame_length;

    /* Start of buffer in I/O space, pointer to RAM buffer */
    rx_buffer=(uint8_t *)0x180;
    rx_data = (rx_frame->data);

    do{
        *rx_data++ = _SFR_MEM8(rx_buffer++);
    } while (--frame_length > 0);

    /*Read LQI value for this frame.*/
    rx_frame->lqi = *rx_buffer;

    /* If crc was calculated set crc field in hal_rx_frame_t accordingly.
     * Else show the crc has passed the hardware check.
     */
    rx_frame->crc   = true;
    
#else /* defined(__AVR_ATmega128RFA1__) */

    uint8_t frame_length, *rx_data;

    /*Send frame read (long mode) command.*/
    HAL_SPI_TRANSFER_OPEN();
    HAL_SPI_TRANSFER(0x20);

    /*Read frame length. This includes the checksum. */
    frame_length = HAL_SPI_TRANSFER(0);

    /*Check for correct frame length. Bypassing this test can result in a buffer overrun! */
    if ((frame_length < HAL_MIN_FRAME_LENGTH) || (frame_length > HAL_MAX_FRAME_LENGTH)) {
        /* Length test failed */
        rx_frame->length = 0;
        rx_frame->lqi    = 0;
        rx_frame->crc    = false;
    }
    else {
        rx_data = (rx_frame->data);
        rx_frame->length = frame_length;

        /*Transfer frame buffer to RAM buffer */

        HAL_SPI_TRANSFER_WRITE(0);
        HAL_SPI_TRANSFER_WAIT();
        do{
            *rx_data++ = HAL_SPI_TRANSFER_READ();
            HAL_SPI_TRANSFER_WRITE(0);

            /* CRC was checked in hardware, but redoing the checksum here ensures the rx buffer
             * is not being overwritten by the next packet. Since that lengthy computation makes
             * such overwrites more likely, we skip it and hope for the best.
             * Without the check a full buffer is read in 320us at 2x spi clocking.
             * The 802.15.4 standard requires 640us after a greater than 18 byte frame.
             * With a low interrupt latency overwrites should never occur.
             */
    //          crc = _crc_ccitt_update(crc, tempData);

            HAL_SPI_TRANSFER_WAIT();

        } while (--frame_length > 0);


        /*Read LQI value for this frame.*/
        rx_frame->lqi = HAL_SPI_TRANSFER_READ();

        /* If crc was calculated set crc field in hal_rx_frame_t accordingly.
         * Else show the crc has passed the hardware check.
         */
        rx_frame->crc   = true;
    }

    HAL_SPI_TRANSFER_CLOSE();

#endif /* defined(__AVR_ATmega128RFA1__) */
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
#if 0  //Uses 80 bytes (on Raven) omit unless needed
void
hal_sram_read(uint8_t address, uint8_t length, uint8_t *data)
{
    HAL_SPI_TRANSFER_OPEN();

    /*Send SRAM read command and address to start*/
    HAL_SPI_TRANSFER(0x00);
    HAL_SPI_TRANSFER(address);

    HAL_SPI_TRANSFER_WRITE(0);
    HAL_SPI_TRANSFER_WAIT();

    /*Upload the chosen memory area.*/
    do{
        *data++ = HAL_SPI_TRANSFER_READ();
        HAL_SPI_TRANSFER_WRITE(0);
        HAL_SPI_TRANSFER_WAIT();
    } while (--length > 0);

    HAL_SPI_TRANSFER_CLOSE();
}
#endif
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
#if 0  //omit unless needed
void
hal_sram_write(uint8_t address, uint8_t length, uint8_t *data)
{
    HAL_SPI_TRANSFER_OPEN();

    /*Send SRAM write command.*/
    HAL_SPI_TRANSFER(0x40);

    /*Send address where to start writing to.*/
    HAL_SPI_TRANSFER(address);

    /*Upload the chosen memory area.*/
    do{
        HAL_SPI_TRANSFER(*data++);
    } while (--length > 0);

    HAL_SPI_TRANSFER_CLOSE();

}
#endif

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
/* The atmega128rfa1 has individual interrupts for the integrated radio'
 * Whichever are enabled by the RF230 driver must be present even if not used!
 */
/* Received packet interrupt */
ISR(TRX24_RX_END_vect)
{
/* Get the rssi from ED if extended mode */
#if RF230_CONF_AUTOACK
	rf230_last_rssi=hal_register_read(RG_PHY_ED_LEVEL);
#endif

/* Buffer the frame and call rf230_interrupt to schedule poll for rf230 receive process */
/* Is a ram buffer available? */
	if (rxframe[rxframe_tail].length) {DEBUGFLOW('0');} else /*DEBUGFLOW('1')*/;

#ifdef RF230_MIN_RX_POWER		 
/* Discard packets weaker than the minimum if defined. This is for testing miniature meshes */
/* This does not prevent an autoack. TODO:rfa1 radio can be set up to not autoack weak packets */
	if (rf230_last_rssi >= RF230_MIN_RX_POWER) {
#else
	if (1) {
#endif
//		DEBUGFLOW('2');
		hal_frame_read(&rxframe[rxframe_tail]);
		rxframe_tail++;if (rxframe_tail >= RF230_CONF_RX_BUFFERS) rxframe_tail=0;
		rf230_interrupt();
	}
}
/* Preamble detected, starting frame reception */
ISR(TRX24_RX_START_vect)
{
//	DEBUGFLOW('3');
/* Save RSSI for this packet if not in extended mode, scaling to 1dB resolution */
#if !RF230_CONF_AUTOACK
    rf230_last_rssi = 3 * hal_subregister_read(SR_RSSI);
#endif

}

/* PLL has locked, either from a transition out of TRX_OFF or a channel change while on */
ISR(TRX24_PLL_LOCK_vect)
{
//	DEBUGFLOW('4');
}

/* PLL has unexpectedly unlocked */
ISR(TRX24_PLL_UNLOCK_vect)
{
	DEBUGFLOW('5');
}
/* Flag is set by the following interrupts */
extern volatile uint8_t rf230_wakewait, rf230_txendwait,rf230_ccawait;

/* Wake has finished */
ISR(TRX24_AWAKE_vect)
{
//	DEBUGFLOW('6');
  rf230_wakewait=0;
}

/* Transmission has ended */
ISR(TRX24_TX_END_vect)
{
//	DEBUGFLOW('7');
  rf230_txendwait=0;
}

/* Frame address has matched ours */
ISR(TRX24_XAH_AMI_vect)
{
//	DEBUGFLOW('8');
}

/* CCAED measurement has completed */
ISR(TRX24_CCA_ED_DONE_vect)
{
	DEBUGFLOW('4');
	rf230_ccawait=0;
}

#else /* defined(__AVR_ATmega128RFA1__) */
/* Separate RF230 has a single radio interrupt and the source must be read from the IRQ_STATUS register */
HAL_RF230_ISR()
{
    volatile uint8_t state;
    uint8_t interrupt_source; /* used after HAL_SPI_TRANSFER_OPEN/CLOSE block */

    INTERRUPTDEBUG(1);

    
    /* Using SPI bus from ISR is generally a bad idea... */
    /* Note: all IRQ are not always automatically disabled when running in ISR */
    HAL_SPI_TRANSFER_OPEN();

    /*Read Interrupt source.*/
    /*Send Register address and read register content.*/
    HAL_SPI_TRANSFER_WRITE(0x80 | RG_IRQ_STATUS);

    HAL_SPI_TRANSFER_WAIT(); /* AFTER possible interleaved processing */

    interrupt_source = HAL_SPI_TRANSFER(0);

    HAL_SPI_TRANSFER_CLOSE();

    /*Handle the incomming interrupt. Prioritized.*/
    if ((interrupt_source & HAL_RX_START_MASK)){
	   INTERRUPTDEBUG(10);
    /* Save RSSI for this packet if not in extended mode, scaling to 1dB resolution */
#if !RF230_CONF_AUTOACK
#if 0  // 3-clock shift and add is faster on machines with no hardware multiply
       // With -Os avr-gcc saves a byte by using the general routine for multiply by 3
        rf230_last_rssi = hal_subregister_read(SR_RSSI);
        rf230_last_rssi = (rf230_last_rssi <<1)  + rf230_last_rssi;
#else  // Faster with 1-clock multiply. Raven and Jackdaw have 2-clock multiply so same speed while saving 2 bytes of program memory
        rf230_last_rssi = 3 * hal_subregister_read(SR_RSSI);
#endif
#endif

    } else if (interrupt_source & HAL_TRX_END_MASK){
	   INTERRUPTDEBUG(11);	    	    
        
       state = hal_subregister_read(SR_TRX_STATUS);
       if((state == BUSY_RX_AACK) || (state == RX_ON) || (state == BUSY_RX) || (state == RX_AACK_ON)){
         /* Received packet interrupt */
         /* Buffer the frame and call rf230_interrupt to schedule poll for rf230 receive process */
         if (rxframe[rxframe_tail].length) INTERRUPTDEBUG(42); else INTERRUPTDEBUG(12);
 
#ifdef RF230_MIN_RX_POWER		 
         /* Discard packets weaker than the minimum if defined. This is for testing miniature meshes.*/
         /* Save the rssi for printing in the main loop */
#if RF230_CONF_AUTOACK
         //rf230_last_rssi=hal_subregister_read(SR_ED_LEVEL);
         rf230_last_rssi=hal_register_read(RG_PHY_ED_LEVEL);
#endif
         if (rf230_last_rssi >= RF230_MIN_RX_POWER) {
#endif
           hal_frame_read(&rxframe[rxframe_tail]);
           rxframe_tail++;if (rxframe_tail >= RF230_CONF_RX_BUFFERS) rxframe_tail=0;
           rf230_interrupt();
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
        ;
    } else if (interrupt_source & HAL_BAT_LOW_MASK){
        /*  Disable BAT_LOW interrupt to prevent endless interrupts. The interrupt */
        /*  will continously be asserted while the supply voltage is less than the */
        /*  user-defined voltage threshold. */
        uint8_t trx_isr_mask = hal_register_read(RG_IRQ_MASK);
        trx_isr_mask &= ~HAL_BAT_LOW_MASK;
        hal_register_write(RG_IRQ_MASK, trx_isr_mask);
        INTERRUPTDEBUG(16);
        ;
     } else {
        INTERRUPTDEBUG(99);
	    ;
    }
}
#endif /* defined(__AVR_ATmega128RFA1__) */ 
#   endif /* defined(DOXYGEN) */

/** @} */
/** @} */

/*EOF*/
