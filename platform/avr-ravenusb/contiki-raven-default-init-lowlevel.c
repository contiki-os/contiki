/*
 * Copyright (c) 2006, Technical University of Munich
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
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Simon Barner <barner@in.tum.de>
 *
 * @(#)$$
 */
#include "contiki-raven.h"

#include "contiki.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_specific_request.h"
#include <util/delay.h>
#include "bootloader.h"

uint8_t checkForFinger(void);

uint8_t fingerPresent = 0;

/* Defining this allows you to mount the USB Stick as a mass storage device by shorting the two pins. See docs. */
//#define WINXPSP2

void
init_lowlevel(void)
{
    Leds_init();
    Leds_off();

        if (checkForFinger()) {
			if(bootloader_is_present())
				Jump_To_Bootloader();
#ifdef WINXPSP2
				usb_mode = mass_storage;
#else
                fingerPresent = 1;
#endif
        }

 return;

}


uint8_t checkForFinger(void)
{
  uint8_t tests;
  uint8_t matches;
 
 /*    
         Three pads on RZUSBSTICK go: GND PD3 PD2
     
         We pulse PD3, and check for that pattern on PD2.

          A (moist) finger across those three pads should be enough
          to bridge these
  */

  //Output
  DDRD |= 1<<PD3;
 
  //Input
  DDRD &= ~(1<<PD2);



  tests = 100;
  matches = 0;
  while(tests) {

      //Set bit PD3 to value of LSB of 'tests'
          PORTD = (PORTD & ~(1<<PD3)) | ( (tests & 0x01) << PD3);

          //Allow changes to propogate
          _delay_us(1);

          //Check if PD2 matches what we set PD3 to
          if ((PIND & (1<<PD2)) == ((tests & 0x01) << PD2)) {
                        matches++;      
          }

          tests--;
   }

   if (matches > 70) {
      return 1;
   }

   return 0;
}
