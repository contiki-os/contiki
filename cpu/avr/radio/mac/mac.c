/*   Copyright (c) 2008, Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Additional fixes for AVR contributed by:
 *
 *	Colin O'Flynn coflynn@newae.com
 *	Eric Gnoske egnoske@gmail.com
 *	Blake Leverett bleverett@gmail.com
 *	Mike Vidales mavida404@gmail.com
 *	Kevin Brown kbrown3@uccs.edu
 *	Nate Bohlmann nate@elfwerks.com
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of the copyright holders nor the names of
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 **/
/**
 *  \addtogroup wireless
 *  @{
 */
/**
 *  \defgroup rf230mac RF230 MAC
 *  @{
 */
/**
 *    \file
 *    \brief  The IEEE 802.15.4 (2003/2006) MAC utility functions.
 *
 */

/**
 *  \author 
 *         Eric Gnoske <egnoske@gmail.com>
 *         Blake Leverett <bleverett@gmail.com>
 *         Mike Vidales <mavida404@gmail.com>
 *         Colin O'Flynn <coflynn@newae.com>
 *
 */

/* Includes */

#include <stdlib.h>
#include <string.h>
#include "zmac.h"
#include "radio.h"
#include "hal.h"
#include "tcpip.h"
#include "uip.h"
#include "sicslowpan.h"
#include "sicslowmac.h"

/* Globals */
/** \brief Interface structure for this module */
ieee_15_4_manager_t ieee15_4ManagerAddress;

//dataRequest_t dataRequestStructAddress;

/* Macros & Defines */

uint8_t msduHandle;
bool iAmCoord;
bool autoModes;


/** \brief The RF channel to use for all following transmissions and
 *  receptions (see 6.1.2).  Allowable values are 0-26
 */
uint8_t phyCurrentChannel;

/**  \brief The 64-bit address of the coordinator/router through which
 *   the network layer wishes to communicate.
 */
uint64_t macCoordExtendedAddress;

/**  \brief The 16-bit short address assigned to the coordinator
 *   through which the network layer wishes to communicate.  A value
 *   of 0xfffe indicates th the coordinator is only using it's 64-bit
 *   extended address.  A value of 0xffff indicates that this value is
 *   unknown.  The default value is 0xfff.
 */
uint16_t macCoordShortAddress;

/**  \brief This address is the 64-bit address that will be used as
 *   the mechanism to provide a destination to the upper layers.  The
 *   default value is 0xfff.
 */
uint64_t macDestAddress;

/**  \brief The sequence number (0x00 - 0xff) added to the transmitted
 *   data or MAC command frame. The default is a random value within
 *   the range.
 */
uint8_t macDSN;

/**  \brief The 16-bit identifier of the PAN on which the device is
 *   sending to.  If this value is 0xffff, the device is not
 *   associated.  The default value is 0xffff.
 */
uint16_t macDstPANId;

/**  \brief The 16-bit identifier of the PAN on which the device is
 *   operating.  If this value is 0xffff, the device is not
 *   associated.  The default value is 0xffff.
 */
uint16_t macSrcPANId;

/**  \brief The 16-bit address that the device uses to communicate in
 *   the PAN.  If the device is the PAN coordinator, this value shall
 *   be chosen before a PAN is started.  Otherwise, the address is
 *   allocated by a coordinator during association.  A value of 0xfffe
 *   indicates that the device has associated but has not been
 *   allocated an address.  A value of 0xffff indicates that the
 *   device does not have a short address.  The default value is
 *   0xffff.
 */
uint16_t macShortAddress;


/** \brief Our own long address. This needs to be read from EEPROM or
 *  other secure memory storage.
 */
uint64_t macLongAddr;

/* Implementation */

/** \brief Initializes the (quasi) 802.15.4 MAC.  This function should
 * be called only once on startup.
 */
void
mac_init(void)
{
    volatile uint8_t buf[8];

    sicslowmac_resetRequest(true);

    /* Set up the radio for auto mode operation. */
    hal_subregister_write( SR_MAX_FRAME_RETRIES, 2 );

    /* Need to laod PANID for auto modes */
    radio_set_pan_id(DEST_PAN_ID);

    /* Buffer the uint64_t address for easy loading and debug. */
    /** \todo   Find a better location to load the IEEE address. */
    buf[0] = macLongAddr & 0xFF;
    buf[1] = (macLongAddr >> 8) & 0xFF;
    buf[2] = (macLongAddr >> 16) & 0xFF;
    buf[3] = (macLongAddr >> 24) & 0xFF;
    buf[4] = (macLongAddr >> 32) & 0xFF;
    buf[5] = (macLongAddr >> 40) & 0xFF;
    buf[6] = (macLongAddr >> 48) & 0xFF;
    buf[7] = (macLongAddr >> 56) & 0xFF;
    /* Load the long address into the radio. This is required for auto mode */
    /* operation. */
    radio_set_extended_address((uint8_t *)&macLongAddr);

    srand(1234 );
    msduHandle = rand();

    /* Ping6 debug */
    memcpy(uip_lladdr.addr, &macLongAddr, 8);
	
	/* Convert expected byte order */
	byte_reverse((uint8_t *)uip_lladdr.addr, 8);
}


/** @} */
/** @} */
