/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief
 *      This file contains the system configuration definition.
 *
 * \par Application note:
 *      AVR280: USB Host CDC Demonstration
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Name:  $
 * $Revision: 1.1 $
 * $RCSfile: config.h,v $
 * $Date: 2008/10/14 20:16:36 $  \n
 ******************************************************************************/
/* Copyright (c) 2008  ATMEL Corporation
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CONFIG_H_
#define CONFIG_H_

/**
   @addtogroup usbstick
   @{
*/

//_____ I N C L U D E S ____________________________________________________


#include "compiler.h" //!< Compiler definitions

#include "contiki-raven.h"

#ifdef AVRGCC
   #define __AVR_AT90USBxxx__
   #include <avr/io.h>
#else
   #include "lib_mcu/mcu.h" //!< Register declaration
#endif

//#include "conf_scheduler.h" //!< Scheduler tasks declaration

//! Enable or not the ADC usage
#undef  USE_ADC

//! CPU core frequency in kHz
#define FOSC 8000


// -------- END Generic Configuration -------------------------------------

// UART Sample configuration, if we have one ... __________________________

#ifndef AVRGCC
   #define uart_usb_putchar putchar
#endif
#define r_uart_ptchar int
#define p_uart_ptchar int

#define NB_MS_BEFORE_FLUSH			50
#define REPEAT_KEY_PRESSED       100

// ADC Sample configuration, if we have one ... ___________________________

//! ADC Prescaler value
#define ADC_PRESCALER 64
//! Right adjust
#define ADC_RIGHT_ADJUST_RESULT 1
//! AVCC As reference voltage (See adc_drv.h)
#define ADC_INTERNAL_VREF  2

//!--------- Device Mass Storage Identifiers Signature -----------------------
#define SBC_VENDOR_ID         {'A','T','M','E','L',' ',' ',' '}      // 8 Bytes only
#define SBC_PRODUCT_ID        {'R','Z','R','A','V','E','N','U','S','B',' ','D','O','C','S',' '}  // 16 Bytes only
#define SBC_REVISION_ID       {'0','.','0','0'}  // 4 Bytes only

/** @}  */

#endif //CONFIG_H_

