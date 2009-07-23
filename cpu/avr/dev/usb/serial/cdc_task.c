/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file cdc_task.c **********************************************************
 *
 * \brief
 *      Manages the CDC-ACM Virtual Serial Port Dataclass for the USB Device
 *
 * \addtogroup usbstick
 *
 * \author 
 *        Colin O'Flynn <coflynn@newae.com>
 *
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
/**
 \ingroup usbstick
 \defgroup cdctask CDC Task
 @{
 */

//_____  I N C L U D E S ___________________________________________________


#include "contiki.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_specific_request.h"
#include "serial/cdc_task.h"
#include "serial/uart_usb_lib.h"
#include "rndis/rndis_protocol.h"
#include "sicslow_ethernet.h"
#include "radio.h"
#include <stdio.h>
#include <stdlib.h>

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define PRINTF printf
#define PRINTF_P printf_P

//_____ M A C R O S ________________________________________________________





//_____ D E F I N I T I O N S ______________________________________________


#define IAD_TIMEOUT_DETACH 300
#define IAD_TIMEOUT_ATTACH 600
#define PBUF ((rndis_data_packet_t *) data_buffer)

//_____ D E C L A R A T I O N S ____________________________________________


void menu_print(void);
void menu_process(char c);

extern char usb_busy;

//! Counter for USB Serial port
extern U8    tx_counter;

//! Timers for LEDs
uint8_t led3_timer;


//! Was USB device *just* enumerated?
uint8_t justenumerated = 1;


static uint8_t timer = 0;
static struct etimer et;


PROCESS(cdc_process, "CDC process");

/**
 * \brief Communication Data Class (CDC) Process
 *
 *   This is the link between USB and the "good stuff". In this routine data
 *   is received and processed by CDC-ACM Class
 */
PROCESS_THREAD(cdc_process, ev, data_proc)
{

	PROCESS_BEGIN();
	uart_usb_init();

	while(1) {
      

	    // turn off LED's if necessary
		if (led3_timer) led3_timer--;
		else			Led3_off();

 		if(Is_device_enumerated() && (usb_mode == rndis_debug) && rndis_state && (!usb_busy)) {

			if (justenumerated) {

				//If we have serial port, set it as output
			    if (usb_mode == rndis_debug) {
					uart_usb_set_stdout();
					menu_print();
				}
				justenumerated = 0;
			}

			//Flush buffer if timeout
	        if(timer >= 4 && tx_counter!=0 ){
	            timer = 0;
	            uart_usb_flush();
	        } else {
				timer++;
			}

			while (uart_usb_test_hit()){
  		  	   menu_process(uart_usb_getchar());   // See what they want
            }


		}//if (Is_device_enumerated())


		if (usb_mode == rndis_debug) {
			etimer_set(&et, CLOCK_SECOND/80);
		} else {
			etimer_set(&et, CLOCK_SECOND);
		}

		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));	
		
	} // while(1)

	PROCESS_END();
}

/**
 \brief Print debug menu
 */
void menu_print(void)
{
		PRINTF_P(PSTR("\n\n\r********** Jackdaw Menu ******************\n\r"));
		PRINTF_P(PSTR("*                                        *\n\r"));
		PRINTF_P(PSTR("* Main Menu:                             *\n\r"));
		PRINTF_P(PSTR("*  h,?             Print this menu       *\n\r"));
		PRINTF_P(PSTR("*  m               Print current mode    *\n\r"));
		PRINTF_P(PSTR("*  s               Set to sniffer mode   *\n\r"));
		PRINTF_P(PSTR("*  n               Set to network mode   *\n\r"));
		PRINTF_P(PSTR("*  c               Set RF channel        *\n\r"));
		PRINTF_P(PSTR("*  6               Toggle 6lowpan        *\n\r"));
		PRINTF_P(PSTR("*  r               Toggle raw mode       *\n\r"));
		PRINTF_P(PSTR("*  u               Switch to mass-storage*\n\r"));
		PRINTF_P(PSTR("*                                        *\n\r"));
		PRINTF_P(PSTR("* Make selection at any time by pressing *\n\r"));
		PRINTF_P(PSTR("* your choice on keyboard.               *\n\r"));
		PRINTF_P(PSTR("******************************************\n\r"));
}

/**
 \brief Process incomming char on debug port
 */
void menu_process(char c)
{

	static enum menustate_enum            /* Defines an enumeration type    */
	{
		normal,
		channel
	} menustate = normal;
	
	static char channel_string[3];
	static uint8_t channel_string_i = 0;
	
	int tempchannel;
	

	if (menustate == channel) {

		switch(c) {
			case '\r':
			case '\n':		
				channel_string[channel_string_i] = 0;
								
				//Will return zero in event of error...
				tempchannel = atoi(channel_string);
				
				//Bounds check only if user had real input
				if ( ((channel_string_i) && (tempchannel < 11)) || (tempchannel > 26))  {
					PRINTF_P(PSTR("\n\rInvalid input\n\r"));				
				}
				
				//If valid input, change it
				if (tempchannel) {
					radio_set_operating_channel(tempchannel);
					eeprom_write_byte((uint8_t *) 9, tempchannel);   //Write channel
					eeprom_write_byte((uint8_t *)10, ~tempchannel); //Bit inverse as check
				}

				menustate = normal;
				break;
		
			case '\b':
			
				if (channel_string_i)
					channel_string_i--;
				break;
					
			default:
			
				if (channel_string_i > 1) {
					menustate = normal;
					PRINTF_P(PSTR("\n\rInput too long!\n\r"));
					break;
				}
				
				channel_string[channel_string_i] = c;
				channel_string_i++;
		}


	} else {

		uint8_t i;
		switch(c) {
			case '\r':
			case '\n':
				break;

			case 'h':
			case '?':
				menu_print();
				break;

			case 's':
				PRINTF_P(PSTR("Jackdaw now in sniffer mode\n\r"));
				usbstick_mode.sendToRf = 0;
				usbstick_mode.translate = 0;
				break;

			case 'n':
				PRINTF_P(PSTR("Jackdaw now in network mode\n\r"));
				usbstick_mode.sendToRf = 1;
				usbstick_mode.translate = 1;
				break;

			case '6':
				if (usbstick_mode.sicslowpan) {
					PRINTF_P(PSTR("Jackdaw does not perform 6lowpan translation\n\r"));
					usbstick_mode.sicslowpan = 0;
				} else {
					PRINTF_P(PSTR("Jackdaw now performs 6lowpan translations\n\r"));
					usbstick_mode.sicslowpan = 1;
				}	
				
				break;

			case 'r':
				if (usbstick_mode.raw) {
					PRINTF_P(PSTR("Jackdaw does not capture raw frames\n\r"));
					usbstick_mode.raw = 0;
				} else {
					PRINTF_P(PSTR("Jackdaw now captures raw frames\n\r"));
					usbstick_mode.raw = 1;
				}	
				break;

			case 'c':
				PRINTF_P(PSTR("Select 802.15.4 Channel in range 11-26 [%d]: "), radio_get_operating_channel());
				menustate = channel;
				channel_string_i = 0;
				break;
				
				
			
			case 'm':
				PRINTF_P(PSTR("Currently Jackdaw:\n\r  * Will "));
				if (usbstick_mode.sendToRf == 0) { PRINTF_P(PSTR("not "));}
				PRINTF_P(PSTR("send data over RF\n\r  * Will "));
				if (usbstick_mode.translate == 0) { PRINTF_P(PSTR("not "));}
				PRINTF_P(PSTR("change link-local addresses inside IP messages\n\r  * Will "));
				if (usbstick_mode.sicslowpan == 0) { PRINTF_P(PSTR("not "));}
				PRINTF_P(PSTR("decompress 6lowpan headers\n\r  * Will "));
				if (usbstick_mode.raw == 0) { PRINTF_P(PSTR("not "));}
				PRINTF_P(PSTR("Output raw 802.15.4 frames\n\r "));
				PRINTF_P(PSTR("  * Operates on channel %d\n\r"), radio_get_operating_channel());
				break;

			case 'u':

				//Mass storage mode
				usb_mode = mass_storage;

				//No more serial port
				stdout = NULL;

				//RNDIS is over
				rndis_state = 	rndis_uninitialized;
				Leds_off();

				//Deatch USB
				Usb_detach();

				//Wait a few seconds
				for(i = 0; i < 50; i++)
					_delay_ms(100);

				//Attach USB
				Usb_attach();


				break;

			default:
				PRINTF_P(PSTR("%c is not a valid option! h for menu\n\r"), c);
				break;
		}


	}

	return;

}


/**
    @brief This will enable the VCP_TRX_END LED for a period
*/
void vcptx_end_led(void)
{
    Led3_on();
    led3_timer = 10;
}
/** @}  */

