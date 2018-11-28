/*
 * Copyright (c) 2017, Weptech elektronik GmbH Germany
 * http://www.weptech.de
 *
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TARGET_CONF_H_
#define TARGET_CONF_H_

/*---------------------------------------------------------------------------*/
/* Radio configuration */
#define PLATFORM_CONF_USE_CC1200        0
/*---------------------------------------------------------------------------*/
#if PLATFORM_CONF_USE_CC1200
/* Enable sniff mode for cc1200 radio */
#define CC1200_CONF_SNIFFER             1
#else /* PLATFORM_CONF_USE_CC1200 */
/* Enable sniff mode for cc2538 2.4 GHz radio  */
#define CC2538_RF_CONF_SNIFFER          1
#define CC2538_RF_CONF_AUTOACK          0
#endif /* PLATFORM_CONF_USE_CC1200 */
/*---------------------------------------------------------------------------*/
#define UART0_CONF_BAUD_RATE            460800
/*---------------------------------------------------------------------------*/

#define SENSNIFF_IO_DRIVER_H            "pool/cc2538-io.h"
#endif /* TARGET_CONF_H_ */
