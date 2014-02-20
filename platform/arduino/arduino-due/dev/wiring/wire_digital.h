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

#ifndef WIRE_DIGITAL_H_
#define WIRE_DIGITAL_H_

#include "sam3x8e.h"
#include "pio.h"

void configure_input_pin_interrupt_enable(uint32_t pin_number, uint32_t pin_attribute, 
	uint32_t irq_attr, void (*pin_irq_handler) (uint32_t, uint32_t));
void configure_input_pin_interrupt_disable(uint32_t pin_number);
void configure_output_pin( uint32_t pin_number, const uint32_t pin_default_level, 
	const uint32_t ul_multidrive_enable, const uint32_t ul_pull_up_enable );
	
void digital_write(uint32_t pin_number, uint32_t val);
uint32_t digital_read(uint32_t pin_number, const pio_type_t pin_type);

	
#endif /* WIRE_DIGITAL_H_ */