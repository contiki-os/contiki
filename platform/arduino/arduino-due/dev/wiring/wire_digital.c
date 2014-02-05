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
#include "wire_digital.h"
#include "arduino_due.h"
#include "component\component_pio.h"
#include "pio_handler.h"
#include "pmc.h"
#include "core_cm3.h"
#include "pio.h"



void configure_input_pin_interrupt_enable(uint32_t pin_number, uint32_t pin_attribute,
	uint32_t irq_attr, void (*pin_irq_handler) (uint32_t, uint32_t))
{
	/* Derive the PIN Bank of the desired PIN number. */
	Pio* pin_pio_bank = (g_APinDescription[pin_number]).pPort;
	
	/* Extract the ID of the desired PIN bank. */
	uint32_t pin_pio_id = (g_APinDescription[pin_number]).ulPeripheralId; 
		
	/* Extract the ID of the desired PIN. */
	uint32_t pin_id = (g_APinDescription[pin_number]).ulPin;
	
	/* Enable peripheral clocking on the desired port. */
	pmc_enable_periph_clk(pin_pio_id);
	
	/* Configure the desired PIN as input. */
	pio_set_input(pin_pio_bank, pin_id, pin_attribute);
	
	/* Configure the interrupt mode of the desired PIN. */
	pio_handler_set(pin_pio_bank, pin_pio_id, pin_id, irq_attr, pin_irq_handler);
	
	switch(pin_pio_id) {
			
		case ID_PIOA:
			/* The interrupt on the Cortex-M3 is given highest priority. */
			NVIC_SetPriority(PIOA_IRQn,0);
			/* Register the interrupt on the Cortex-M3. */
			NVIC_EnableIRQ(PIOA_IRQn);
			break;
		case ID_PIOB:
			/* The interrupt on the Cortex-M3 is given highest priority. */
			NVIC_SetPriority(PIOB_IRQn,0);
			/* Register the interrupt on the Cortex-M3. */
			NVIC_EnableIRQ(PIOB_IRQn);
			break;
		case ID_PIOC:
			/* The interrupt on the Cortex-M3 is given highest priority. */
			NVIC_SetPriority(PIOC_IRQn,0);
			/* Register the interrupt on the Cortex-M3. */
			NVIC_EnableIRQ(PIOC_IRQn);
			break;
		case ID_PIOD:
			/* The interrupt on the Cortex-M3 is given highest priority. */
			NVIC_SetPriority(PIOD_IRQn,0);
			/* Register the interrupt on the Cortex-M3. */
			NVIC_EnableIRQ(PIOD_IRQn);
			break;
		default:
			printf("ERROR IRQ Type resolve error!\n");
			return;
	}
		
	/* Enable the interrupt on the desired input PIN. */
	pio_enable_interrupt(pin_pio_bank, pin_id);
}



void configure_input_pin_interrupt_disable(uint32_t pin_number)
{
	/* Derive the PIN Bank of the desired PIN number. */
	Pio* pin_pio_bank = (g_APinDescription[pin_number]).pPort;
	
	/* Extract the ID of the desired PIN bank. */
	uint32_t pin_pio_id = (g_APinDescription[pin_number]).ulPeripheralId;
	
	/* Extract the ID of the desired PIN. */
	uint32_t pin_id = (g_APinDescription[pin_number]).ulPin;
	
	/* Enable peripheral clocking on the desired port. */
	pmc_disable_periph_clk(pin_pio_id);
	
	switch(pin_pio_id) {
		
		case ID_PIOA:
		/* Register the interrupt on the Cortex-M3. */
		NVIC_DisableIRQ(PIOA_IRQn);
		break;
		case ID_PIOB:
		/* Register the interrupt on the Cortex-M3. */
		NVIC_DisableIRQ(PIOB_IRQn);
		break;
		case ID_PIOC:
		/* Register the interrupt on the Cortex-M3. */
		NVIC_DisableIRQ(PIOC_IRQn);
		break;
		case ID_PIOD:
		/* Register the interrupt on the Cortex-M3. */
		NVIC_DisableIRQ(PIOD_IRQn);
		break;
		default:
		printf("ERROR IRQ Type resolve error!\n");
		return;
	}
	
	/* Enable the interrupt on the desired input PIN. */
	pio_disable_interrupt(pin_pio_bank, pin_id);
}


void configure_output_pin( uint32_t pin_number,	const uint32_t pin_default_level, 
	const uint32_t ul_multidrive_enable, const uint32_t ul_pull_up_enable )
{
	/* Derive the PIN Bank of the desired PIN number. */
	Pio* pin_pio_bank = (g_APinDescription[pin_number]).pPort;
	
	/* Extract the ID of the desired PIN bank. */
	uint32_t pin_pio_id = (g_APinDescription[pin_number]).ulPeripheralId;
	
	/* Extract the ID of the desired PIN. */
	uint32_t pin_id = (g_APinDescription[pin_number]).ulPin;
	
	/* Enable peripheral clocking on the desired port. */
	pmc_enable_periph_clk(pin_pio_id);
	
	/* Configure the desired PIN as output. */
	pio_set_output(pin_pio_bank, pin_id, pin_default_level, 
		ul_multidrive_enable, ul_pull_up_enable);
		
	/* DEBUG */
	if (pin_default_level == LOW)
		printf("Output PIN %u: %lu.\n", pin_number, pio_get(pin_pio_bank, PIO_TYPE_PIO_OUTPUT_0, pin_id));
	else if (pin_default_level == HIGH)
		printf("Output PIN %u: %lu.\n", pin_number, pio_get(pin_pio_bank, PIO_TYPE_PIO_OUTPUT_1, pin_id));
	else
		printf("ERROR: Unrecognized default output pin level.\n");	
}


void digital_write( uint32_t pin_number, uint32_t val )
{
	/* Derive the PIN Bank of the desired PIN number. */
	Pio* pin_pio_bank = (g_APinDescription[pin_number]).pPort;
	
	/* Extract the ID of the desired PIN. */
	uint32_t pin_id = (g_APinDescription[pin_number]).ulPin;
	
	if (val == LOW) {
		/* Clear the pin. */
		pio_clear(pin_pio_bank, pin_id);	
	
	} else if (val == HIGH) {
		
		/* Set the pin. */
		pio_set(pin_pio_bank, pin_id);	
	}	
}


uint32_t digital_read(uint32_t pin_number, const pio_type_t pin_type)
{	
	if ( g_APinDescription[pin_number].ulPinType == PIO_NOT_A_PIN )
	{
		/* This is not a correct pin type.  */
		return LOW ;
	}	
	/* Derive the PIN Bank of the desired PIN number. */
	Pio* pin_pio_bank = (g_APinDescription[pin_number]).pPort;
	
	/* Extract the ID of the desired PIN. */
	uint32_t pin_id = (g_APinDescription[pin_number]).ulPin;
	
	if (pio_get(pin_pio_bank, pin_type, pin_id) != 0)
		return HIGH;
	else
		return LOW;
}

