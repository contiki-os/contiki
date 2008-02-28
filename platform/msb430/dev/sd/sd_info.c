
/*
Copyright 2007, Freie Universitaet Berlin. All rights reserved.

These sources were developed at the Freie Universität Berlin, Computer
Systems and Telematics group.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
- Neither the name of Freie Universitaet Berlin (FUB) nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

This software is provided by FUB and the contributors on an "as is"
basis, without any representations or warranties of any kind, express
or implied including, but not limited to, representations or
warranties of non-infringement, merchantability or fitness for a
particular purpose. In no event shall FUB or contributors be liable
for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or
business interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even
if advised of the possibility of such damage.

This implementation was developed by the CST group at the FUB.

For documentation and questions please use the web site
http://scatterweb.mi.fu-berlin.de and the mailinglist
scatterweb@lists.spline.inf.fu-berlin.de (subscription via the Website).
Berlin, 2007
*/

/**
 * @file	ScatterWeb.sd.info.c
 * @ingroup	libsd
 * @brief	MMC-/SD-Card library, Additional Information
 * 
 * @author	Michael Baar	<baar@inf.fu-berlin.de>
 * @date	Feb 2007
 * @version	0.2
 */

/**
 * @addtogroup	libsd
 * @{
 */
#include "sd_internals.h"
#include "sd.h"

uint16_t
sd_read_cid(sd_cid_t * pCID)
{
  return sd_read_register(pCID, SD_CMD_SEND_CID, sizeof (sd_cid_t));
}

unsigned long
sd_get_size(void)
{
  uint32_t size = 0;

  if (uart_lock(UART_MODE_SPI)) {
    sd_csd_t csd;

    if (sd_read_register(&csd, SD_CMD_SEND_CSD, sizeof (sd_csd_t))) {
      size = SD_CSD_C_SIZE(csd) + 1;
      size <<= SD_CSD_C_MULT(csd);
      size <<= 2;
      size <<= sd_state.MaxBlockLen_bit;
    }
    uart_unlock(UART_MODE_SPI);
  }
  return size;
}

/** @} */
