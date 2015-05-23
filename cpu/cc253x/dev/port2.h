/*
Copyright (c) 2012, Philippe Retornaz
Copyright (c) 2012, EPFL STI IMT LSRO1 -- Mobots group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote
   products derived from this software without specific prior
   written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CC253x_P2_INTR_H_
#define CC253x_P2_INTR_H_

#include "contiki.h"

#define CC253x_P2_ACK 	0x1
#define CC253x_P2_NACK 0x0

struct cc253x_p2_handler {
  struct cc253x_p2_handler *next;
  uint8_t (* cb)(void);
};

void cc253x_p2_register_handler(struct cc253x_p2_handler *h);

void cc253x_p2_unregister_handler(struct cc253x_p2_handler *h);

/* When the ISR is not needed, hide its prototype from the module containing
 * main() to prevent linking it */
#if PORT_2_ISR_ENABLED
void port_2_isr(void) __interrupt(P2INT_VECTOR);
#endif

#define cc253x_p2_irq_disable(flag) do { flag = IEN2 & 0x2; IEN2 &= ~0x2; } while(0)
#define cc253x_p2_irq_enable(flag) do { IEN2 |= flag; } while(0)

#define cc253x_p2_irq_force_enable() cc253x_p2_irq_enable(0x2)

#endif
