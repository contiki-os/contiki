/*
 * Copyright (c) 2012, KTH, Royal Institute of Technology(Stockholm, Sweden)
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
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
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
 * This is work by the CSD master project. Fall 2012. Microgrid team.
 * Author: Javier Lara Peinado <javierlp@kth.se>
 *
 */

#ifndef SWITCHES_ACTUATORS_H_
#define SWITCHES_ACTUATORS_H_

#define NSWITCHES 4

//Sw1L: P2.0
//Connected to Ngate1 through the FAN3278
#define SW_NGATE1_PORT    2
#define SW_NGATE1_PIN     _BIT(0)
//Sw1H: P2.1
//Connected to Pgate1 through the FAN3278
#define SW_PGATE1_PORT   2
#define SW_PGATE1_PIN    _BIT(1)
//Sw2L: P2.2
//Connected to Ngate2 through the FAN3278
#define SW_NGATE2_PORT    2
#define SW_NGATE2_PIN     _BIT(2)
//Sw2H: P2.3
//Connected to Pgate2 through the FAN3278
#define SW_PGATE2_PORT   2
#define SW_PGATE2_PIN    _BIT(3)

typedef enum{
  SW_NGATE1=0,
  SW_NGATE2,
  SW_PGATE1,
  SW_PGATE2
}SW_ID;

typedef enum{
  SW_ON,
  SW_OFF
}SW_STATE;

//Initialize the GPIO for the switches
void
switches_init();

//Change the state of a switch to SW_ON or SW_OFF
void
switches_set_gate_state(int swId, int swState);

//Get the current state of a particular switch
int
switches_get_gate_state(int swId);


#endif /* SWITCHES_ACTUATORS_H_ */
