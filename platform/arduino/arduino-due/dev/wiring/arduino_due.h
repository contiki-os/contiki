#include <stdint-gcc.h>
#include "component\component_pio.h"
#include "pio.h"

/*
 * arduino.h
 *
 * Created: 5/10/2013 11:00:44 PM
 *  Author: Ioannis Glaropoulos
 */ 


#ifndef ARDUINO_H_
#define ARDUINO_H_

/**
 * Pin Attributes to be OR-ed
 */
#define PIN_ATTR_COMBO         (1UL<<0)
#define PIN_ATTR_ANALOG        (1UL<<1)
#define PIN_ATTR_DIGITAL       (1UL<<2)
#define PIN_ATTR_PWM           (1UL<<3)
#define PIN_ATTR_TIMER         (1UL<<4)

/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
typedef enum _EAnalogChannel
{
	NO_ADC=-1,
	ADC0=0,
	ADC1,
	ADC2,
	ADC3,
	ADC4,
	ADC5,
	ADC6,
	ADC7,
	ADC8,
	ADC9,
	ADC10,
	ADC11,
	ADC12,
	ADC13,
	ADC14,
	ADC15,
	DA0,
	DA1
} EAnalogChannel ;

#define ADC_CHANNEL_NUMBER_NONE 0xffffffff

// Definitions for PWM channels
typedef enum _EPWMChannel
{
	NOT_ON_PWM=-1,
	PWM_CH0=0,
	PWM_CH1,
	PWM_CH2,
	PWM_CH3,
	PWM_CH4,
	PWM_CH5,
	PWM_CH6,
	PWM_CH7
} EPWMChannel ;

// Definitions for TC channels
typedef enum _ETCChannel
{
	NOT_ON_TIMER=-1,
	TC0_CHA0=0,
	TC0_CHB0,
	TC0_CHA1,
	TC0_CHB1,
	TC0_CHA2,
	TC0_CHB2,
	TC1_CHA3,
	TC1_CHB3,
	TC1_CHA4,
	TC1_CHB4,
	TC1_CHA5,
	TC1_CHB5,
	TC2_CHA6,
	TC2_CHB6,
	TC2_CHA7,
	TC2_CHB7,
	TC2_CHA8,
	TC2_CHB8
} ETCChannel ;

typedef  enum _pio_type EPioType;
/* Types used for the tables below */
typedef struct _PinDescription
{
	Pio* pPort ;
	uint32_t ulPin ;
	uint32_t ulPeripheralId ;
	EPioType ulPinType ;
	uint32_t ulPinConfiguration ;
	uint32_t ulPinAttribute ;
	EAnalogChannel ulAnalogChannel ; /* Analog pin in the Arduino context (label on the board) */
	EAnalogChannel ulADCChannelNumber ; /* ADC Channel number in the SAM device */
	EPWMChannel ulPWMChannel ;
	ETCChannel ulTCChannel ;
} PinDescription ;

/* Pins table to be instantiated into arduino_due.c */
extern const PinDescription g_APinDescription[] ;

/*
 * SPI Interfaces
 */
#define SPI_INTERFACES_COUNT 1

#define SPI_INTERFACE        SPI0
#define SPI_INTERFACE_ID     ID_SPI0
#define SPI_CHANNELS_NUM 4
#define PIN_SPI_SS0          (77u)
#define PIN_SPI_SS1          (87u)
#define PIN_SPI_SS2          (86u)
#define PIN_SPI_SS3          (78u)
#define PIN_SPI_MOSI         (75u)
#define PIN_SPI_MISO         (74u)
#define PIN_SPI_SCK          (76u)
#define BOARD_SPI_SS0        (10u)
#define BOARD_SPI_SS1        (4u)
#define BOARD_SPI_SS2        (52u)
#define BOARD_SPI_SS3        PIN_SPI_SS3
#define BOARD_SPI_DEFAULT_SS BOARD_SPI_SS2

#define BOARD_PIN_TO_SPI_PIN(x) \
	(x==BOARD_SPI_SS0 ? PIN_SPI_SS0 : \
	(x==BOARD_SPI_SS1 ? PIN_SPI_SS1 : \
	(x==BOARD_SPI_SS2 ? PIN_SPI_SS2 : PIN_SPI_SS3 )))
#define BOARD_PIN_TO_SPI_CHANNEL(x) \
	(x==BOARD_SPI_SS0 ? 0 : \
	(x==BOARD_SPI_SS1 ? 1 : \
	(x==BOARD_SPI_SS2 ? 2 : 3)))
	
	
	
// Get the bit location within the hardware port of the given virtual pin.
// This comes from the pins_*.c file for the active board configuration.
//
#define digitalPinToPort(P)        ( g_APinDescription[P].pPort )
#define digitalPinToBitMask(P)     ( g_APinDescription[P].ulPin )
#define digitalPinToTimer(P)       (  )
//#define analogInPinToBit(P)        ( )
#define portOutputRegister(port)   ( &(port->PIO_ODSR) )
#define portInputRegister(port)    ( &(port->PIO_PDSR) )
//#define portModeRegister(P)        (  )

//#define NOT_A_PIN 0  // defined in pio.h/EPioType
#define NOT_A_PORT           0	
#endif /* ARDUINO_H_ */