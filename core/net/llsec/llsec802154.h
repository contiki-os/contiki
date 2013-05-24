/**
 * \addtogroup llsec
 * @{
 */

/**
 * \defgroup llsec802154
 * 
 * Common functionality of 802.15.4-compliant llsec_drivers.
 * 
 * @{
 */

/*
 * Copyright (c) 2013, Hasso-Plattner-Institut.
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
 *
 */

/**
 * \file
 *         Common functionality of 802.15.4-compliant llsec_drivers.
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#ifndef LLSEC802154_H_
#define LLSEC802154_H_

#include "net/mac/frame802154.h"

#ifdef LLSEC802154_CONF_SECURITY_LEVEL
#define LLSEC802154_SECURITY_LEVEL     LLSEC802154_CONF_SECURITY_LEVEL
#else /* LLSEC802154_CONF_SECURITY_LEVEL */
#define LLSEC802154_SECURITY_LEVEL     FRAME802154_SECURITY_LEVEL_NONE
#endif /* LLSEC802154_CONF_SECURITY_LEVEL */

#define LLSEC802154_MIC_LENGTH         ((LLSEC802154_SECURITY_LEVEL & 3) * 4)

#ifdef LLSEC802154_CONF_USES_ENCRYPTION
#define LLSEC802154_USES_ENCRYPTION    LLSEC802154_CONF_USES_ENCRYPTION
#else /* LLSEC802154_CONF_USES_ENCRYPTION */
#define LLSEC802154_USES_ENCRYPTION    (LLSEC802154_SECURITY_LEVEL & (1 << 2))
#endif /* LLSEC802154_CONF_USES_ENCRYPTION */

#endif /* LLSEC802154_H_ */

/** @} */
/** @} */
