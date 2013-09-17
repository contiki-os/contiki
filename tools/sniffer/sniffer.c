/*
 * Copyright (c) 2012-2013, Centre National de la Recherche Scientifique.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * -----
 * 
 * Implemented by Etienne Duble (CNRS / LIG).
 * A part of this work was done for the ANR ARESA2 project.
 * 
 * This file is part of a generic sniffer implementation for contiki.
 * See Readme.txt for more details.
 * 
 * This code is implements the contiki process and
 * the callback called for each packet.
 */

#include "contiki.h"
#if CONTIKI_TARGET_MBXXX
#include "dev/stm32w-radio.h"
#endif
#include "dev/slip.h"
#include "net/packetbuf.h"

/*---------------------------------------------------------------------------*/
PROCESS(sniffer_init, "sniffer_init");
AUTOSTART_PROCESSES(&sniffer_init);
/*---------------------------------------------------------------------------*/

void sniffer_callback()
{
	static int len;
	static uint8_t buffer[PACKETBUF_SIZE + PACKETBUF_HDR_SIZE];
	packetbuf_copyto(buffer);

	len = packetbuf_totlen();

	/* the sniffer seemed to be fast enough to us (we did not miss any packet).
	 * if you have heavier traffic and start to lose some packets,
	 * this sniffer could surely be improved by buffering the bytes that 
	 * must be written on the serial link (supposedly the bottleneck).
	 * (i.e. you can define a 2nd process that would write bytes 1 by 1 
	 * (or few at once) and release the control in between) */
	if(len>0) {
		slip_write(buffer, len);
	}
}

PROCESS_THREAD(sniffer_init, ev, data)
{
	PROCESS_BEGIN();

	/* If you need code specific to your platform 
	   in order to turn the mote into sniffing mode,
	   insert it here. */

#if CONTIKI_TARGET_MBXXX
	ST_RadioEnableReceiveCrc(FALSE);
	ST_RadioEnableAutoAck(FALSE);
	ST_RadioEnableAddressFiltering(FALSE);
#endif

	PROCESS_END();
}

