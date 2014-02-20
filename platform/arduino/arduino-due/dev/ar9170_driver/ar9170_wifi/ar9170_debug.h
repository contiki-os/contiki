/**
 * Copyright (c) 2013, Calipso project consortium
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or
 * other materials provided with the distribution.
 * 
 * 3. Neither the name of the Calipso nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific
 * prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef AR9170_DEBUG_H_
#define AR9170_DEBUG_H_






#define USB_LOCK_DEBUG				0

#define AR9170_MAIN_DEBUG			1
#define AR9170_MAIN_DEBUG_DEEP		0

/* USB line */
#define USB_WRAPPER_DEBUG			1
#define USB_WRAPPER_DEBUG_DEEP		0

/* CMD */
#define USB_CMD_WRAPPER_DEBUG		1
#define USB_CMD_WRAPPER_DEBUG_DEEP	0

/* Data */
#define USB_DATA_WRAPPER_DEBUG		1
#define USB_DATA_WRAPPER_DEBUG_DEEP	0

/* PSM */
#define AR9170_PSM_DEBUG			1
#define AR9170_PSM_DEBUG_DEEP		0
/* FW */
#define USB_FW_WRAPPER_DEBUG		1
#define USB_FW_WRAPPER_DEBUG_DEEP	0
/* RX */
#define AR9170_RX_DEBUG				1
#define AR9170_RX_DEBUG_DEEP		0

/* TX */
#define AR9170_TX_DEBUG					1
#define AR9170_TX_DEBUG_DEEP		0

/* MAC */
#define AR9170_MAC_DEBUG			1
#define AR9170_MAC_DEBUG_DEEP		0

/* PHY*/
#define AR9170_PHY_DEBUG			1
#define AR9170_PHY_DEBUG_DEEP		0

/* SCHEDULER */
#define AR9170_SCHEDULER_DEBUG		0
#define AR9170_SCHEDULER_DEBUG_DEEP	0

#endif /* AR9170_DEBUG_H_ */