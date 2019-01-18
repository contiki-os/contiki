/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: rs232.c,v 1.1 2006/08/21 12:11:19 fros4943 Exp $
 */

#include "lib/sensors.h"
#include "dev/rs232.h"
#include "dev/serial.h"
#include "lib/simEnvChange.h"
#include <string.h>
#include <stdio.h>

const struct simInterface rs232_interface;

#define SERIAL_BUF_SIZE 1024

// COOJA variables
char simSerialReceivingData[SERIAL_BUF_SIZE];
int simSerialReceivingLength;
char simSerialReceivingFlag;
char simSerialSendingData[SERIAL_BUF_SIZE];
int simSerialSendingLength;
char simSerialSendingFlag;

static int (* input_handler)(unsigned char) = NULL;

/*-----------------------------------------------------------------------------------*/
void rs232_init(void) { }
/*-----------------------------------------------------------------------------------*/
void rs232_set_speed(unsigned char speed) { }
/*-----------------------------------------------------------------------------------*/
void
rs232_set_input(int (*f)(unsigned char))
{
  input_handler = f;
}
/*-----------------------------------------------------------------------------------*/
void rs232_send(char c) {
  simSerialSendingData[simSerialSendingLength] = c;
  simSerialSendingLength += 1;
  simSerialSendingFlag = 1;
}
/*-----------------------------------------------------------------------------------*/
void
rs232_print(char *message)
{
  memcpy(&simSerialSendingData[0] + simSerialSendingLength, &message[0], strlen(message));
  simSerialSendingLength += strlen(message);
  simSerialSendingFlag = 1;
}
/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsBeforeTick(void)
{
  int i;
	
  // Check if this mote has received data on RS232
  if (simSerialReceivingFlag && simSerialReceivingLength > 0) {
    // Tell user specified poll function
      if(input_handler != NULL)
        for (i=0; i < simSerialReceivingLength; i++)
          input_handler(simSerialReceivingData[i]);

    // Tell serial process
    for (i=0; i < simSerialReceivingLength; i++)
      serial_input_byte(simSerialReceivingData[i]);

    serial_input_byte(0x0a);

    simSerialReceivingLength = 0;
    simSerialReceivingFlag = 0;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsAfterTick(void)
{
}
/*-----------------------------------------------------------------------------------*/

SIM_INTERFACE(rs232_interface,
	      doInterfaceActionsBeforeTick,
	      doInterfaceActionsAfterTick);
