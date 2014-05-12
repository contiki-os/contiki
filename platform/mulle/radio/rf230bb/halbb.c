/*   Copyright (c) 2009, Swedish Institute of Computer Science
 *  All rights reserved.
 *
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
//#include "contiki-conf.h"

#if DEBUGFLOWSIZE
extern uint8_t debugflowsize,debugflow[DEBUGFLOWSIZE];
#define DEBUGFLOW(c) if (debugflowsize<(DEBUGFLOWSIZE-1)) debugflow[debugflowsize++]=c
#else
#define DEBUGFLOW(c)
#endif


/*============================ INCLUDE =======================================*/
#include <stdlib.h>

#include "hal.h"

#include "at86rf230_registermap.h"

#include "stdio.h"

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
volatile extern signed char rf230_last_rssi;

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
#if 0 //defined(__AVR_ATmega128RFA1__)
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
#endif
#if 0
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


//#include "MK60DZ10.h"

/* K60: TODO */
#define HAL_SPI_TRANSFER_OPEN() HAL_ENTER_CRITICAL_REGION();
//#define HAL_SPI_TRANSFER_WRITE(data)
//#define HAL_SPI_TRANSFER_WAIT()
//#define HAL_SPI_TRANSFER_READ()
#define HAL_SPI_TRANSFER_CLOSE() HAL_LEAVE_CRITICAL_REGION();
#include "K60.h"
static inline void hal_spi_send(uint8_t data, int cont)
{
	/* Send data */
	if(cont)
		SPI0_PUSHR = SPI_PUSHR_PCS((1<<HAL_SS_PIN)) | SPI_PUSHR_CONT_MASK | data;
	else
		SPI0_PUSHR = SPI_PUSHR_PCS((1<<HAL_SS_PIN)) | data;
	SPI0_SR |= SPI_SR_TCF_MASK;
	while (!(SPI0_SR & SPI_SR_TCF_MASK));

	/* Dummy read */
	SPI0_SR |= SPI_SR_TCF_MASK;
	data = (0xFF & SPI0_POPR);
}

static inline uint8_t hal_spi_receive(int cont)
{
	/* Dummy write */
	if(cont)
		SPI0_PUSHR = SPI_PUSHR_PCS((1<<HAL_SS_PIN)) | SPI_PUSHR_CONT_MASK;
	else
		SPI0_PUSHR = SPI_PUSHR_PCS((1<<HAL_SS_PIN));
	SPI0_SR |= SPI_SR_TCF_MASK;
	while (!(SPI0_SR & SPI_SR_TCF_MASK));

	/* Read data */
	SPI0_SR |= SPI_SR_TCF_MASK;
	return (0xFF & SPI0_POPR);
}


#define HAL_RF230_ISR() void __attribute__((interrupt))  _isr_gpio_b(void)

/** \brief  This function initializes the Hardware Abstraction Layer.
 */
void
hal_init(void)
{
    /*** IO Specific Initialization.****/

	/* Enable PORTC clock gate */
	SIM_SCGC5  |= SIM_SCGC5_PORTD_MASK;
	SIM_SCGC5  |= SIM_SCGC5_PORTE_MASK;
	SIM_SCGC5  |= SIM_SCGC5_PORTB_MASK;

	PORTE_PCR6 |= 0x0100;     // Sleep
        PORTB_PCR9 |= 0x00090100; // Set PTB9 (IRQ)    as GPIO with active high interrupt
        PORTD_PCR7 |= 0x0100;     // Vp

	GPIOE_PDDR |= 0x0040; /* Setup PTE6 (Sleep) as output */
	GPIOD_PDDR |= 0x0080; /* Setup PTD7 (Vp) as output */

	/* Enable power switch to radio */
	hal_set_pwr_high();

    /*** SPI Specific Initialization.****/

	/* Mux SPI0 on port B */
	PORTD_PCR4 |= 0x0200; /* SPI0_PCS1 */
	PORTD_PCR2 |= 0x0200; /* SPI0_MOSI */
	PORTD_PCR1 |= 0x0200; /* SPI0_SCLK */
	PORTD_PCR3 |= 0x0200; /* SPI0_MISO */

	/* Enable clock gate for SPI0 module */
	SIM_SCGC6 |= SIM_SCGC6_SPI0_MASK;

	/* Configure SPI1 */
	SPI0_MCR   = 0x803F3000;
	SPI0_CTAR0 = 0x38002224; /* TODO: Should be able to speed up */

	/*** Enable interrupts from the radio transceiver. ***/
	hal_enable_trx_interrupt();
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
    uint8_t register_value = 0;

    /* Add the register read command to the register address. */
    address &= HAL_TRX_CMD_RADDRM;
    address |= HAL_TRX_CMD_RR;

    HAL_ENTER_CRITICAL_REGION();

    /*** Send Register address and read register content. ***/
    hal_spi_send(address, true); /* Write address */
    register_value = hal_spi_receive(false); /* Read register */

    HAL_LEAVE_CRITICAL_REGION();

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
hal_register_write(uint8_t address, uint8_t value) /* K60: OK, tested */
{
    /* Add the Register Write command to the address. */
    address = HAL_TRX_CMD_RW | (HAL_TRX_CMD_RADDRM & address);

    HAL_ENTER_CRITICAL_REGION();

    /*** Send Register address and write register content. ***/
    hal_spi_send(address, true); /* Write address */
    hal_spi_send(value, false); /* Write data */

    HAL_LEAVE_CRITICAL_REGION();
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
hal_frame_read(hal_rx_frame_t *rx_frame) /* TODO: Make sure this is working */
{
    uint8_t *rx_data;
    uint8_t frame_length;

    HAL_ENTER_CRITICAL_REGION();

    /*Send frame read (long mode) command.*/
    hal_spi_send(HAL_TRX_CMD_FR, true);

    /*Read frame length. This includes the checksum. */
    frame_length = hal_spi_receive(true);


    /*Check for correct frame length. Bypassing this test can result in a buffer overrun! */
    if ((frame_length < HAL_MIN_FRAME_LENGTH) || (frame_length > HAL_MAX_FRAME_LENGTH))
    {
        /* Length test failed */
        rx_frame->length = 0;
        rx_frame->lqi    = 0;
        rx_frame->crc    = false;
    }
    else
    {
        rx_data = (rx_frame->data);
        rx_frame->length = frame_length;
        /*Transfer frame buffer to RAM buffer */

        do{
        	*rx_data++ =  hal_spi_receive(true);
        } while (--frame_length > 0);

        /*Read LQI value for this frame.*/
	    rx_frame->lqi = *rx_data++ =  hal_spi_receive(false);

        /* If crc was calculated set crc field in hal_rx_frame_t accordingly.
         * Else show the crc has passed the hardware check.
         */
        rx_frame->crc   = true;
    }

    HAL_LEAVE_CRITICAL_REGION();
#if 0
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
hal_frame_write(uint8_t *write_buffer, uint8_t length) /* TODO: Make sure this is working */
{
#if 0 /* Print frame to uart */
	{
		int i;
		for(i=0; i<length; i++)
		{
			printf("%02x ", write_buffer[i]);
			if((i+1)%16 == 0)
				printf("\r\n");
		}
		printf("\r\n");
	}
#endif
    HAL_ENTER_CRITICAL_REGION();

    /* Send frame transmitt (long mode) command. */
    hal_spi_send(HAL_TRX_CMD_FW, true);

    /* Sendframe length.  */
    hal_spi_send(length, true);

    /* Download to the Frame Buffer.
     * When the FCS is autogenerated there is no need to transfer the last two bytes
     * since they will be overwritten.
     */
#if !RF230_CONF_CHECKSUM
    length -= 2;
#endif

    do{
    	if(length == 1) /* Do not assert) CS on last byte */
    		hal_spi_send(*write_buffer++, false);
    	else
    		hal_spi_send(*write_buffer++, true);
    } while (--length);

    HAL_LEAVE_CRITICAL_REGION();
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

/* Separate RF230 has a single radio interrupt and the source must be read from the IRQ_STATUS register */
HAL_RF230_ISR()
{
    volatile uint8_t state;
    uint8_t interrupt_source; /* used after HAL_SPI_TRANSFER_OPEN/CLOSE block */

	/* Clear Interrupt Status Flag */
    PORTB_PCR9 |= 0x01000000;  /* Clear interrupt */
    NVICICPR2 = (1<<24);

    INTERRUPTDEBUG(1);

    /*Read Interrupt source.*/
    interrupt_source = hal_register_read(RG_IRQ_STATUS); /* K60: OK, tested */

    /*Handle the incomming interrupt. Prioritized.*/
    if ((interrupt_source & HAL_RX_START_MASK))
    {
		INTERRUPTDEBUG(10);
		/* Save RSSI for this packet if not in extended mode, scaling to 1dB resolution */
#if !RF230_CONF_AUTOACK
        rf230_last_rssi = 3 * hal_subregister_read(SR_RSSI);
#endif

    }
    else if (interrupt_source & HAL_TRX_END_MASK)
    {
	   INTERRUPTDEBUG(11);

       state = hal_subregister_read(SR_TRX_STATUS);
       if((state == BUSY_RX_AACK) || (state == RX_ON) || (state == BUSY_RX) || (state == RX_AACK_ON))
       {
			/* Received packet interrupt */
			/* Buffer the frame and call rf230_interrupt to schedule poll for rf230 receive process */
    	   if (rxframe[rxframe_tail].length) INTERRUPTDEBUG(42); else INTERRUPTDEBUG(12);

#ifdef RF230_MIN_RX_POWER
       /* Discard packets weaker than the minimum if defined. This is for testing miniature meshes.*/
       /* Save the rssi for printing in the main loop */
#if RF230_CONF_AUTOACK
        rf230_last_rssi=hal_register_read(RG_PHY_ED_LEVEL);
#endif
        if (rf230_last_rssi >= RF230_MIN_RX_POWER) {
#endif
         hal_frame_read(&rxframe[rxframe_tail]);
         rxframe[rxframe_tail].rssi = rf230_last_rssi;
         rxframe_tail++;if (rxframe_tail >= RF230_CONF_RX_BUFFERS) rxframe_tail=0;
         rf230_interrupt();
#ifdef RF230_MIN_RX_POWER
        }
#endif
       }

    }
    else if (interrupt_source & HAL_TRX_UR_MASK)
    {
        INTERRUPTDEBUG(13);
        ;
    }
    else if (interrupt_source & HAL_PLL_UNLOCK_MASK)
    {
        INTERRUPTDEBUG(14);
	    ;
    }
    else if (interrupt_source & HAL_PLL_LOCK_MASK)
    {
        INTERRUPTDEBUG(15);
        ;
    }
    else if (interrupt_source & HAL_BAT_LOW_MASK)
    {
        /*  Disable BAT_LOW interrupt to prevent endless interrupts. The interrupt */
        /*  will continously be asserted while the supply voltage is less than the */
        /*  user-defined voltage threshold. */
        uint8_t trx_isr_mask = hal_register_read(RG_IRQ_MASK);
        trx_isr_mask &= ~HAL_BAT_LOW_MASK;
        hal_register_write(RG_IRQ_MASK, trx_isr_mask);
        INTERRUPTDEBUG(16);
        ;
     }
    else
    {
        INTERRUPTDEBUG(99);
	    ;
    }
}

#if 0
/* Separate RF230 has a single radio interrupt and the source must be read from the IRQ_STATUS register */
HAL_RF230_ISR()
{
    /*The following code reads the current system time. This is done by first
      reading the hal_system_time and then adding the 16 LSB directly from the
      hardware counter.
     */
//    uint32_t isr_timestamp = hal_system_time;
//    isr_timestamp <<= 16;
//    isr_timestamp |= HAL_TICK_UPCNT();

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
       // With -Os avr-gcc saves a byte by using the general routine for multiply by 3
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


/** @} */
/** @} */

/*EOF*/
