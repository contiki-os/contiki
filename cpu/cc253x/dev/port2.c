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

#include "cc253x.h"
#include "port2.h"

struct cc253x_p2_handler *handlers = NULL;

void
cc253x_p2_register_handler(struct cc253x_p2_handler *h)
{
  uint8_t flags;

  if(!h) {
    return;
  }

  cc253x_p2_irq_disable(flags);

  h->next = handlers;
  handlers = h;

  cc253x_p2_irq_enable(flags);
}

void
cc253x_p2_unregister_handler(struct cc253x_p2_handler *remove)
{
  uint8_t flags;
  struct cc253x_p2_handler *h = handlers;

  // Protect against dumb users
  if(!h || !remove) {
    return;
  }

  cc253x_p2_irq_disable(flags);

  if(h == remove) {
    // First element in the list
    handlers = h->next;
  } else {
    while(h->next) {
      if(h->next == remove) {
        h->next = h->next->next;
        break;
      }
      h = h->next;
    }
  }

  cc253x_p2_irq_enable(flags);
}
