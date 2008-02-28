
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
 * @file	ScatterWeb.sd.erase.c
 * @ingroup	libsd
 * @brief	MMC-/SD-Card library, Block erase
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

bool
sd_erase_blocks(uint32_t address, uint16_t numBlocks)
{
  uint8_t ret, r1;
  uint32_t endAdr;

  if (sd_protected())
    return FALSE;
  ret =
    sd_send_cmd(SD_CMD_ERASE_WR_BLK_START_ADDR, SD_RESPONSE_TYPE_R1,
		&address, &r1);
  if (!ret | r1)
    return FALSE;
  endAdr = (numBlocks - 1) * sd_state.BlockLen;
  endAdr += address;
  ret =
    sd_send_cmd(SD_CMD_ERASE_WR_BLK_END_ADDR, SD_RESPONSE_TYPE_R1, &endAdr,
		&r1);
  if (!ret | r1)
    return FALSE;
  ret = sd_send_cmd(SD_CMD_ERASE, SD_RESPONSE_TYPE_R1, NULL, &r1);
  return ret;
}

/** @} */
