/*
 * Copyright (c) 2011-2012, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *
 *  cc2420-radio.c
 *
 * \author
 *
 *	Kaleem Ullah <mscs14059@itu.edu.pk>
 *	Kaleem Ullah <kaleemullah360@live.com>
 *
 * \Usage
 *
 *	Case 1:	zero is passed in power/channel function i.e set_cc2420_txpower(0); will use predefined channel number in cc2420-radio lib.
 *	Case 2:	a custom value is passed in power/channel function i.e set_cc2420_txpower(13); will use channel 13.
 *	Case 3:	not using both/eithere of power/channel setting function will set Contiki OS default configurations i.e channel 26, tx power 31.
 */

/* -------- Set Radio Powers --------------- */

#include <cc2420-radio.h>

/*|Power (dBm)|PA_LEVEL|Power (mW)|
* |0          |  31    |1.0000    |
* |-0.0914    |  30    |0.9792    |
* |-25.0000   |  3     |0.0032    |
* |-28.6970   |  2     |0.0013    |
* |-32.9840   |  1     |0.0005    |
* |-37.9170   |  0     |0.0002    |
*/ 
uint8_t radioChannel = 26;  // default channel
uint8_t radioChannel_tx_power = 31; // default power

void set_cc2420_channel(custom_radioChannel){
	/* channel */
	if(!custom_radioChannel == 0){
		radioChannel = custom_radioChannel;
	}
	cc2420_set_channel(radioChannel);
	printf("CC2420 Radio channel %d\n", cc2420_get_channel());
}


void set_cc2420_txpower(custom_radioChannel_tx_power){
	/* tx power */
	if(!custom_radioChannel_tx_power == 0){
		radioChannel_tx_power = custom_radioChannel_tx_power;
	}
	cc2420_set_txpower(radioChannel_tx_power);
	printf("CC2420 Radio TX power %d\n", cc2420_get_txpower());
}

/* -------- End Set Radio Powers ------------ */