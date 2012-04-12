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
 *  David Kopf dak664@embarqmail.com
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
 */

/**
 *    \addtogroup hal
 *    @{
 */

/**
 *  \file
 *  \brief This file contains low-level radio driver code.
 *
 *   $Id: hal.h,v 1.5 2010/12/03 20:42:01 dak664 Exp $
*/

#ifndef HAL_AVR_H
#define HAL_AVR_H
/*============================ INCLUDE =======================================*/
#include <stdint.h>
#include <stdbool.h>
//#include <util/crc16.h>
#include "contiki-conf.h"
/*============================ MACROS ========================================*/

// TEST CODE
#define TRIG1 DDRB |= 0x04, PINB |= 0x04
#define TRIG2 DDRD |= 0x80, PIND |= 0x80

/** \name This is the list of pin configurations needed for a given platform.
 * \brief Change these values to port to other platforms.
 * \{
 */
/* Define all possible platform types/revisions here. */
// Don't use zero, it will match if undefined!
// RAVEN_D : Raven kit with LCD display
// RAVENUSB_C : used for RZRAVEN USB key
// RCB_B : RZ200 kit from Atmel based on 1281V
// ZIGBIT : Zigbit module from Meshnetics
// ATMEGA128RFA1 : Bare chip with internal radio
// IRIS : IRIS Mote from MEMSIC
#define RAVENUSB_C      1
#define RAVEN_D	        2
#define RCB_B	    	3
#define ZIGBIT			4
#define IRIS			5
#define ATMEGA128RFA1   6

#if PLATFORM_TYPE == RCB_B
/* 1281 rcb */
#   define SSPORT     B
#   define SSPIN      (0x00)
#   define SPIPORT    B
#   define MOSIPIN    (0x02)
#   define MISOPIN    (0x03)
#   define SCKPIN     (0x01)
#   define RSTPORT    B
#   define RSTPIN     (0x05)
#   define IRQPORT    D
#   define IRQPIN     (0x04)
#   define SLPTRPORT  B
#   define SLPTRPIN   (0x04)
#   define USART      1
#   define USARTVECT  USART1_RX_vect
#   define TICKTIMER  3
#   define HAS_SPARE_TIMER

#elif PLATFORM_TYPE == ZIGBIT
/* 1281V Zigbit */
#   define SSPORT     B
#   define SSPIN      (0x00)
#   define SPIPORT    B
#   define MOSIPIN    (0x02)
#   define MISOPIN    (0x03)
#   define SCKPIN     (0x01)
#   define RSTPORT    A
#   define RSTPIN     (0x07)
#   define IRQPORT    E
#   define IRQPIN     (0x05)
#   define SLPTRPORT  B
#   define SLPTRPIN   (0x04)
#   define TXCWPORT   B
#   define TXCWPIN    (0x07)
#   define USART      1
#   define USARTVECT  USART1_RX_vect
//#   define TICKTIMER  3
//#   define HAS_SPARE_TIMER // Not used


#elif PLATFORM_TYPE == RAVEN_D
/* 1284 raven */
#   define SSPORT     B
#   define SSPIN      (0x04)
#   define SPIPORT    B
#   define MOSIPIN    (0x05)
#   define MISOPIN    (0x06)
#   define SCKPIN     (0x07)
#   define RSTPORT    B
#   define RSTPIN     (0x01)
#   define IRQPORT    D
#   define IRQPIN     (0x06)
#   define SLPTRPORT  B
#   define SLPTRPIN   (0x03)
#   define TXCWPORT   B
#   define TXCWPIN    (0x00)
#   define USART      1
#   define USARTVECT  USART1_RX_vect
#   define TICKTIMER  3
#   define HAS_CW_MODE
#   define HAS_SPARE_TIMER

#elif PLATFORM_TYPE == RAVENUSB_C
/* 1287USB raven */
#   define SSPORT     B
#   define SSPIN      (0x00)
#   define SPIPORT    B
#   define MOSIPIN    (0x02)
#   define MISOPIN    (0x03)
#   define SCKPIN     (0x01)
#   define RSTPORT    B
#   define RSTPIN     (0x05)
#   define IRQPORT    D
#   define IRQPIN     (0x04)
#   define SLPTRPORT  B
#   define SLPTRPIN   (0x04)
#   define TXCWPORT   B
#   define TXCWPIN    (0x07)
#   define USART      1
#   define USARTVECT  USART1_RX_vect
#   define TICKTIMER  3
#   define HAS_CW_MODE
#   define HAS_SPARE_TIMER

#elif PLATFORM_TYPE == ATMEGA128RFA1
/* ATmega1281 with internal AT86RF231 radio */
#   define SLPTRPORT  TRXPR
#   define SLPTRPIN   1
#   define USART      1
#   define USARTVECT  USART1_RX_vect
#   define TICKTIMER  3
#   define HAS_CW_MODE
#   define HAS_SPARE_TIMER

#elif CONTIKI_TARGET_MULLE
/* mulle 5.2 (TODO: move to platform specific) */
#   define SSPORT     3
#   define SSPIN      5
#   define MOSIPORT   1
#   define MOSIPIN    1
#   define MISOPORT   1
#   define MISOPIN    0
#   define SCKPORT    3
#   define SCKPIN     3
#   define RSTPORT    4
#   define RSTPIN     3
#   define IRQPORT    8
#   define IRQPIN     3
#   define SLPTRPORT  0
#   define SLPTRPIN   7
#   define HAS_SPARE_TIMER

#elif PLATFORM_TYPE == IRIS
/* 1281 IRIS */
#   define SSPORT     B
#   define SSPIN      (0x00)
#   define SPIPORT    B
#   define MOSIPIN    (0x02)
#   define MISOPIN    (0x03)
#   define SCKPIN     (0x01)
#   define RSTPORT    A
#   define RSTPIN     (0x06)
#   define IRQPORT    D
#   define IRQPIN     (0x04)
#   define SLPTRPORT  B
#   define SLPTRPIN   (0x07)
//#   define TXCWPORT   B
//#   define TXCWPIN    (0x07)
#   define USART      1
#   define USARTVECT  USART1_RX_vect
//#   define TICKTIMER  3
//#   define HAS_SPARE_TIMER // Not used
#else

#error "PLATFORM_TYPE undefined in hal.h"

#endif

/* For architectures that have all SPI signals on the same port */
#ifndef SSPORT
#define SSPORT SPIPORT
#endif

#ifndef SCKPORT
#define SCKPORT SPIPORT
#endif

#ifndef MOSIPORT
#define MOSIPORT SPIPORT
#endif

#ifndef MISOPORT
#define MISOPORT SPIPORT
#endif

/** \} */

/**
 * \name Macros used to generate read register names from platform-specific definitions of ports.
 * \brief The various CAT macros (DDR, PORT, and PIN) are used to
 * assign port/pin/DDR names to various macro variables.  The
 * variables are assigned based on the specific connections made in
 * the hardware.  For example TCCR(TICKTIMER,A) can be used in place of TCCR0A
 * if TICKTIMER is defined as 0.
 * \{
 */
#if defined(__AVR__)
#define CAT(x, y)      x##y
#define CAT2(x, y, z)  x##y##z
#define DDR(x)         CAT(DDR,  x)
#define PORT(x)        CAT(PORT, x)
#define PIN(x)         CAT(PIN,  x)
#define UCSR(num, let) CAT2(UCSR,num,let)
#define RXEN(x)        CAT(RXEN,x)
#define TXEN(x)        CAT(TXEN,x)
#define TXC(x)         CAT(TXC,x)
#define RXC(x)         CAT(RXC,x)
#define RXCIE(x)       CAT(RXCIE,x)
#define UCSZ(x,y)      CAT2(UCSZ,x,y)
#define UBRR(x,y)      CAT2(UBRR,x,y)
#define UDRE(x)        CAT(UDRE,x)
#define UDRIE(x)       CAT(UDRIE,x)
#define UDR(x)         CAT(UDR,x)
#define TCNT(x)        CAT(TCNT,x)
#define TIMSK(x)       CAT(TIMSK,x)
#define TCCR(x,y)      CAT2(TCCR,x,y)
#define COM(x,y)       CAT2(COM,x,y)
#define OCR(x,y)       CAT2(OCR,x,y)
#define CS(x,y)        CAT2(CS,x,y)
#define WGM(x,y)       CAT2(WGM,x,y)
#define OCIE(x,y)      CAT2(OCIE,x,y)
#define COMPVECT(x)    CAT2(TIMER,x,_COMPA_vect)
#define UDREVECT(x)    CAT2(USART,x,_UDRE_vect)
#define RXVECT(x)      CAT2(USART,x,_RX_vect)
#endif

/* TODO: Move to CPU specific */
#if defined(CONTIKI_TARGET_MULLE)
#define CAT(x, y)      x##y.BYTE
#define CAT2(x, y, z)  x##y##z.BYTE
#define DDR(x)         CAT(PD,  x)
#define PORT(x)        CAT(P, x)
#define PIN(x)         CAT(P, x)
#define UCSR(num, let) CAT2(UCSR,num,let)
#define RXEN(x)        CAT(RXEN,x)
#define TXEN(x)        CAT(TXEN,x)
#define TXC(x)         CAT(TXC,x)
#define RXC(x)         CAT(RXC,x)
#define RXCIE(x)       CAT(RXCIE,x)
#define UCSZ(x,y)      CAT2(UCSZ,x,y)
#define UBRR(x,y)      CAT2(UBRR,x,y)
#define UDRE(x)        CAT(UDRE,x)
#define UDRIE(x)       CAT(UDRIE,x)
#define UDR(x)         CAT(UDR,x)
#define TCNT(x)        CAT(TCNT,x)
#define TIMSK(x)       CAT(TIMSK,x)
#define TCCR(x,y)      CAT2(TCCR,x,y)
#define COM(x,y)       CAT2(COM,x,y)
#define OCR(x,y)       CAT2(OCR,x,y)
#define CS(x,y)        CAT2(CS,x,y)
#define WGM(x,y)       CAT2(WGM,x,y)
#define OCIE(x,y)      CAT2(OCIE,x,y)
#define COMPVECT(x)    CAT2(TIMER,x,_COMPA_vect)
#define UDREVECT(x)    CAT2(USART,x,_UDRE_vect)
#define RXVECT(x)      CAT2(USART,x,_RX_vect)
#endif

/** \} */

/**
 * \name Pin macros
 * \brief These macros convert the platform-specific pin defines into names and functions
 *       that the source code can directly use.
 * \{
 */
#if defined(__AVR_ATmega128RFA1__)

#define hal_set_rst_low( )    ( TRXPR &= ~( 1 << TRXRST ) ) /**< This macro pulls the RST pin low. */
#define hal_set_rst_high( )   ( TRXPR |= ( 1 << TRXRST ) ) /**< This macro pulls the RST pin high. */
#define hal_set_slptr_high( ) ( TRXPR |= ( 1 << SLPTR ) )      /**< This macro pulls the SLP_TR pin high. */
#define hal_set_slptr_low( )  ( TRXPR &= ~( 1 << SLPTR ) )     /**< This macro pulls the SLP_TR pin low. */
//#define hal_get_slptr( ) (    ( TRXPR & ( 1 << SLPTR ) ) >> SLPTR )  /**< Read current state of the SLP_TR pin (High/Low). */
#define hal_get_slptr( )      ( TRXPR & ( 1 << SLPTR ) )  /**< Read current state of the SLP_TR pin (High/Low). */

#else
#define SLP_TR                SLPTRPIN            /**< Pin number that corresponds to the SLP_TR pin. */
#define DDR_SLP_TR            DDR( SLPTRPORT )    /**< Data Direction Register that corresponds to the port where SLP_TR is connected. */
#define PORT_SLP_TR           PORT( SLPTRPORT )   /**< Port (Write Access) where SLP_TR is connected. */
#define PIN_SLP_TR            PIN( SLPTRPORT )    /**< Pin (Read Access) where SLP_TR is connected. */
#define hal_set_slptr_high( ) ( PORT_SLP_TR |= ( 1 << SLP_TR ) )      /**< This macro pulls the SLP_TR pin high. */
#define hal_set_slptr_low( )  ( PORT_SLP_TR &= ~( 1 << SLP_TR ) )     /**< This macro pulls the SLP_TR pin low. */
//#define hal_get_slptr( ) (    ( PIN_SLP_TR & ( 1 << SLP_TR ) ) >> SLP_TR )  /**< Read current state of the SLP_TR pin (High/Low). */
#define hal_get_slptr( )      ( PIN_SLP_TR & ( 1 << SLP_TR ) )   /**< Read current state of the SLP_TR pin (High/Low). */
#define RST                   RSTPIN              /**< Pin number that corresponds to the RST pin. */
#define DDR_RST               DDR( RSTPORT )      /**< Data Direction Register that corresponds to the port where RST is */
#define PORT_RST              PORT( RSTPORT )     /**< Port (Write Access) where RST is connected. */
#define PIN_RST               PIN( RSTPORT /* BUG? */)      /**< Pin (Read Access) where RST is connected. */
#define hal_set_rst_high( )   ( PORT_RST |= ( 1 << RST ) )  /**< This macro pulls the RST pin high. */
#define hal_set_rst_low( )    ( PORT_RST &= ~( 1 << RST ) ) /**< This macro pulls the RST pin low. */
#define hal_get_rst( )        ( ( PIN_RST & ( 1 << RST )  ) >> RST )  /**< Read current state of the RST pin (High/Low). */
#define HAL_SS_PIN            SSPIN               /**< The slave select pin. */
#define HAL_SCK_PIN           SCKPIN              /**< Data bit for SCK. */
#define HAL_MOSI_PIN          MOSIPIN
#define HAL_MISO_PIN          MISOPIN
#define HAL_PORT_SPI          PORT( SPIPORT )     /**< The SPI module is located on PORTB. */
#define HAL_PORT_SS            PORT( SSPORT )
#define HAL_PORT_SCK           PORT( SCKPORT )
#define HAL_PORT_MOSI          PORT( MOSIPORT )     /**< The SPI module uses GPIO might be split on different ports. */
#define HAL_PORT_MISO          PORT( MISOPORT )     /**< The SPI module uses GPIO might be split on different ports. */
#define HAL_DDR_SPI           DDR( SPIPORT )      /**< Data Direction Register for PORTB. */
#define HAL_DDR_SS             DDR( SSPORT )      /**< Data Direction Register for MISO GPIO pin. */
#define HAL_DDR_SCK            DDR( SCKPORT )      /**< Data Direction Register for MISO GPIO pin. */
#define HAL_DDR_MOSI           DDR( MOSIPORT )      /**< Data Direction Register for MISO GPIO pin. */
#define HAL_DDR_MISO           DDR( MISOPORT )      /**< Data Direction Register for MOSI GPIO pin. */
#define HAL_DD_SS             SSPIN               /**< Data Direction bit for SS. */
#define HAL_DD_SCK            SCKPIN              /**< Data Direction bit for SCK. */
#define HAL_DD_MOSI           MOSIPIN             /**< Data Direction bit for MOSI. */
#define HAL_DD_MISO           MISOPIN             /**< Data Direction bit for MISO. */
#endif /* defined(__AVR_ATmega128RFA1__) */

/** \} */


#define HAL_SS_HIGH( ) (HAL_PORT_SS |= ( 1 << HAL_SS_PIN )) /**< MACRO for pulling SS high. */
#define HAL_SS_LOW( )  (HAL_PORT_SS &= ~( 1 << HAL_SS_PIN )) /**< MACRO for pulling SS low. */

/** \brief Macros defined for HAL_TIMER1.
 *
 *  These macros are used to define the correct setupt of the AVR's Timer1, and
 *  to ensure that the hal_get_system_time function returns the system time in
 *  symbols (16 us ticks).
 */

#if defined(__AVR__)
#if ( F_CPU == 16000000UL )
    #define HAL_TCCR1B_CONFIG ( ( 1 << ICES1 ) | ( 1 << CS12 ) )
    #define HAL_US_PER_SYMBOL ( 1 )
    #define HAL_SYMBOL_MASK   ( 0xFFFFffff )
#elif ( F_CPU == 0x800000UL )
    #define HAL_TCCR1B_CONFIG ( ( 1 << ICES1 ) | ( 1 << CS11 ) | ( 1 << CS10 ) )
    #define HAL_US_PER_SYMBOL ( 2 )
    #define HAL_SYMBOL_MASK   ( 0x7FFFffff )
#elif ( F_CPU == 8000000UL )
    #define HAL_TCCR1B_CONFIG ( ( 1 << ICES1 ) | ( 1 << CS11 ) | ( 1 << CS10 ) )
    #define HAL_US_PER_SYMBOL ( 2 )
    #define HAL_SYMBOL_MASK   ( 0x7FFFffff )
//#elif ( F_CPU == 7953408UL )
#elif ( F_CPU == 7954432UL )
    #define HAL_TCCR1B_CONFIG ( ( 1 << ICES1 ) | ( 1 << CS11 ) | ( 1 << CS10 ) )
    #define HAL_US_PER_SYMBOL ( 2 )
    #define HAL_SYMBOL_MASK   ( 0x7FFFffff )
#elif ( F_CPU == 4000000UL )
    #define HAL_TCCR1B_CONFIG ( ( 1 << ICES1 ) | ( 1 << CS11 ) | ( 1 << CS10 ) )
    #define HAL_US_PER_SYMBOL ( 1 )
    #define HAL_SYMBOL_MASK   ( 0xFFFFffff )
#elif ( F_CPU == 1000000UL )
    #define HAL_TCCR1B_CONFIG ( ( 1 << ICES1 ) | ( 1 << CS11 ) )
    #define HAL_US_PER_SYMBOL ( 2 )
    #define HAL_SYMBOL_MASK   ( 0x7FFFffff )
#else
    #error "Clock speed not supported."
#endif

#if PLATFORM_TYPE == ZIGBIT
// IRQ E5 for Zigbit example
#define RADIO_VECT INT5_vect
#define HAL_ENABLE_RADIO_INTERRUPT( ) { ( EIMSK |= ( 1 << INT5 ) ) ; EICRB |= 0x0C ; PORTE &= ~(1<<PE5);  DDRE &= ~(1<<DDE5); }
#define HAL_DISABLE_RADIO_INTERRUPT( ) ( EIMSK &= ~( 1 << INT5 ) )
#else
#define RADIO_VECT TIMER1_CAPT_vect
// Raven and Jackdaw
#define HAL_ENABLE_RADIO_INTERRUPT( ) ( TIMSK1 |= ( 1 << ICIE1 ) )
#define HAL_DISABLE_RADIO_INTERRUPT( ) ( TIMSK1 &= ~( 1 << ICIE1 ) )
#endif

#define HAL_ENABLE_OVERFLOW_INTERRUPT( ) ( TIMSK1 |= ( 1 << TOIE1 ) )
#define HAL_DISABLE_OVERFLOW_INTERRUPT( ) ( TIMSK1 &= ~( 1 << TOIE1 ) )

/** This macro will protect the following code from interrupts.*/
#define HAL_ENTER_CRITICAL_REGION( ) {uint8_t volatile saved_sreg = SREG; cli( )

/** This macro must always be used in conjunction with HAL_ENTER_CRITICAL_REGION
    so that interrupts are enabled again.*/
#define HAL_LEAVE_CRITICAL_REGION( ) SREG = saved_sreg;}

#else /* MULLE */

#define HAL_ENABLE_RADIO_INTERRUPT( ) ( INT1IC.BYTE |= 1 )
#define HAL_DISABLE_RADIO_INTERRUPT( ) ( INT1IC.BYTE &= ~(1) )

#define HAL_ENABLE_OVERFLOW_INTERRUPT( ) ( TB4IC.BYTE = 1 )
#define HAL_DISABLE_OVERFLOW_INTERRUPT( ) ( TB4IC.BYTE = 0 )

/** This macro will protect the following code from interrupts.*/
#define HAL_ENTER_CRITICAL_REGION( ) MULLE_ENTER_CRITICAL_REGION( )

/** This macro must always be used in conjunction with HAL_ENTER_CRITICAL_REGION
    so that interrupts are enabled again.*/
#define HAL_LEAVE_CRITICAL_REGION( ) MULLE_LEAVE_CRITICAL_REGION( )

#endif /* !__AVR__ */


/** \brief  Enable the interrupt from the radio transceiver.
 */
#define hal_enable_trx_interrupt( ) HAL_ENABLE_RADIO_INTERRUPT( )

/** \brief  Disable the interrupt from the radio transceiver.
 *
 *  \retval 0 if the pin is low, 1 if the pin is high.
 */
#define hal_disable_trx_interrupt( ) HAL_DISABLE_RADIO_INTERRUPT( )
/*============================ TYPDEFS =======================================*/
/*============================ PROTOTYPES ====================================*/
/*============================ MACROS ========================================*/
/** \name Macros for radio operation.
 * \{ 
 */
#define HAL_BAT_LOW_MASK       ( 0x80 ) /**< Mask for the BAT_LOW interrupt. */
#define HAL_TRX_UR_MASK        ( 0x40 ) /**< Mask for the TRX_UR interrupt. */
#define HAL_TRX_END_MASK       ( 0x08 ) /**< Mask for the TRX_END interrupt. */
#define HAL_RX_START_MASK      ( 0x04 ) /**< Mask for the RX_START interrupt. */
#define HAL_PLL_UNLOCK_MASK    ( 0x02 ) /**< Mask for the PLL_UNLOCK interrupt. */
#define HAL_PLL_LOCK_MASK      ( 0x01 ) /**< Mask for the PLL_LOCK interrupt. */

#define HAL_MIN_FRAME_LENGTH   ( 0x03 ) /**< A frame should be at least 3 bytes. */
#define HAL_MAX_FRAME_LENGTH   ( 0x7F ) /**< A frame should no more than 127 bytes. */
/** \} */
/*============================ TYPDEFS =======================================*/
/** \struct hal_rx_frame_t
 *  \brief  This struct defines the rx data container.
 *
 *  \see hal_frame_read
 */
typedef struct{
    uint8_t length;                       /**< Length of frame. */
    uint8_t data[ HAL_MAX_FRAME_LENGTH ]; /**< Actual frame data. */
    uint8_t lqi;                          /**< LQI value for received frame. */
    bool crc;                             /**< Flag - did CRC pass for received frame? */
} hal_rx_frame_t;

/** RX_START event handler callback type. Is called with timestamp in IEEE 802.15.4 symbols and frame length. See hal_set_rx_start_event_handler(). */
typedef void (*hal_rx_start_isr_event_handler_t)(uint32_t const isr_timestamp, uint8_t const frame_length);

/** RRX_END event handler callback type. Is called with timestamp in IEEE 802.15.4 symbols and frame length. See hal_set_trx_end_event_handler(). */
typedef void (*hal_trx_end_isr_event_handler_t)(uint32_t const isr_timestamp);

typedef void (*rx_callback_t) (uint16_t data);

/*============================ PROTOTYPES ====================================*/
void hal_init( void );

void hal_reset_flags( void );
uint8_t hal_get_bat_low_flag( void );
void hal_clear_bat_low_flag( void );

hal_trx_end_isr_event_handler_t hal_get_trx_end_event_handler( void );
void hal_set_trx_end_event_handler( hal_trx_end_isr_event_handler_t trx_end_callback_handle );
void hal_clear_trx_end_event_handler( void );

hal_rx_start_isr_event_handler_t hal_get_rx_start_event_handler( void );
void hal_set_rx_start_event_handler( hal_rx_start_isr_event_handler_t rx_start_callback_handle );
void hal_clear_rx_start_event_handler( void );

uint8_t hal_get_pll_lock_flag( void );
void hal_clear_pll_lock_flag( void );

/* Hack for atmega128rfa1 with integrated radio. Access registers directly, not through SPI */
#if defined(__AVR_ATmega128RFA1__)
//#define hal_register_read(address) _SFR_MEM8((uint16_t)address)
#define hal_register_read(address) address
uint8_t hal_subregister_read( uint16_t address, uint8_t mask, uint8_t position );
void hal_subregister_write( uint16_t address, uint8_t mask, uint8_t position,
                            uint8_t value );

//#define hal_register_write(address, value) _SFR_MEM8((uint16_t)address)=value
#define hal_register_write(address, value) address=value
//#define hal_subregister_read( address, mask, position ) (_SFR_MEM8((uint16_t)address)&mask)>>position
//#define hal_subregister_read1( address, mask, position ) (address&mask)>>position
//#define hal_subregister_write( address, mask, position, value ) address=(address<<position)&mask
#else
uint8_t hal_register_read( uint8_t address );
void hal_register_write( uint8_t address, uint8_t value );
uint8_t hal_subregister_read( uint8_t address, uint8_t mask, uint8_t position );
void hal_subregister_write( uint8_t address, uint8_t mask, uint8_t position,
                            uint8_t value );
#endif



//void hal_frame_read(hal_rx_frame_t *rx_frame, rx_callback_t rx_callback);
/* For speed RF230BB does not use a callback */
void hal_frame_read(hal_rx_frame_t *rx_frame);
void hal_frame_write( uint8_t *write_buffer, uint8_t length );
void hal_sram_read( uint8_t address, uint8_t length, uint8_t *data );
void hal_sram_write( uint8_t address, uint8_t length, uint8_t *data );
/* Number of receive buffers in RAM. */
#ifndef RF230_CONF_RX_BUFFERS
#define RF230_CONF_RX_BUFFERS 1
#endif

#endif
/** @} */
/*EOF*/
