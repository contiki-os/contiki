/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief
 *      This file contains the low level macros and definition for the USB PLL.
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
 * $RCSfile: pll_drv.h,v $
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

#ifndef PLL_DRV_H
#define PLL_DRV_H

//_____ I N C L U D E S ____________________________________________________

/**
   @addtogroup usbstick
   @{
*/
//_____ M A C R O S ________________________________________________________

   //! @defgroup PLL_macros PLL Macros
   //! These functions allow to control the PLL
   //! @{
#define PLLx24 			( (0<<PLLP2) | (0<<PLLP1) | (0<<PLLP0) )
#define PLLx12 			( (0<<PLLP2) | (0<<PLLP1) | (1<<PLLP0) )
#define PLLx08 			( (0<<PLLP2) | (1<<PLLP1) | (0<<PLLP0) )
#define PLLx06 			( (0<<PLLP2) | (1<<PLLP1) | (1<<PLLP0) )
#define PLLx04 			( (1<<PLLP2) | (0<<PLLP1) | (0<<PLLP0) )
#define PLLx03 			( (1<<PLLP2) | (0<<PLLP1) | (1<<PLLP0) )
#define PLLx04_8        ( (1<<PLLP2) | (1<<PLLP1) | (0<<PLLP0) )
#define PLLx02          ( (1<<PLLP2) | (1<<PLLP1) | (1<<PLLP0) )


      //! @brief Start the PLL at only 48 MHz, regarding CPU frequency
      //! Start the USB PLL with clockfactor
      //! clockfactor can be PLLx24, PLLx12, PLLx08
      //! PLLx06, PLLx04, PLLx03
#define Start_pll(clockfactor)                                                  \
           (PLLCSR = ( clockfactor  | (1<<PLLE)  ))

      //! return 1 when PLL locked
#define Is_pll_ready()       (PLLCSR & (1<<PLOCK) )

      //! Test PLL lock bit and wait until lock is set
#define Wait_pll_ready()     while (!(PLLCSR & (1<<PLOCK)))

      //! Stop the PLL
#define Stop_pll()           (PLLCSR  &= (~(1<<PLLE)) )

      // Start the PLL in autofactor mode
      // regarding FOSC define
#if   (FOSC==2000)
      //! Start the PLL in autofactor mode
      //! regarding FOSC define
   #define Pll_start_auto()   Start_pll(PLLx24)
#elif (FOSC==4000)
   #define Pll_start_auto()   Start_pll(PLLx12)
#elif (FOSC==6000)
   #define Pll_start_auto()   Start_pll(PLLx08)
#elif (FOSC==8000)
      //! Start the PLL in autofactor mode
      //! regarding FOSC define
   #define Pll_start_auto()   Start_pll(PLLx06)
#elif (FOSC==12000)
   #define Pll_start_auto()   Start_pll(PLLx04)
#elif (FOSC==16000)
   #define Pll_start_auto()   Start_pll(PLLx03)
#elif (FOSC==20000)
   #define Pll_start_auto()   Start_pll(PLLx04_8)
#elif (FOSC==24000)
   #define Pll_start_auto()   Start_pll(PLLx02)
#else
   #error   "FOSC should be defined in config.h"
#endif

   //! @}

//! @}
#endif  // PLL_DRV_H


