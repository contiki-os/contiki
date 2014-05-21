/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Pin mux handling functions for Kinetis platforms.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include "port.h"

void port_module_enable(PORT_MemMapPtr port)
{
  if (port == PORTA_BASE_PTR)
  {
    SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
  }
  else if (port == PORTB_BASE_PTR)
  {
    SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
  }
  else if (port == PORTC_BASE_PTR)
  {
    SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
  }
  else if (port == PORTD_BASE_PTR)
  {
    SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
  }
  else if (port == PORTE_BASE_PTR)
  {
    SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
  }
  else
  {
    asm("bkpt #66\n");
    return;
  }
}
