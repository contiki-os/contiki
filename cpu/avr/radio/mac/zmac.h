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
 */
/**
 *    \brief  The equivalent IEEE 802.15.4 (2003/2006) header file for
 *            the mac primitives.
 *
 */
/**
 *   \addtogroup rf230mac
 *   @{
*/
/**
 *    \file
 *    \brief  The IEEE 802.15.4 (2003/2006) MAC utility functions.
 */

#ifndef MAC_H
#define MAC_H

/*  Includes */
#include <stdint.h>
#include <stdbool.h>
#include "frame.h"
#include "ieee-15-4-manager.h"

#define EEPROMMACADDRESS                ((void*)0)
#define EEPROMDSTADDRESS                ((void*)8)
#define EEPROMCHANADDRESS               ((void*)16)
#define EEPROMPANIDADDRESS              ((void*)17)
#define EEPROMROLEADDRESS               ((void*)19)
#define SUCCESS                         (0)
#define CHANNEL_PAGE_0                  (0)




/* MAC command frames codes */

/* MAC enumerations */
/** \brief Pre-defined data frame control field (FCF) values. 
 *  \name FCF Values
 *  @{
*/
#define FCF_NO_ACK      (0x8841)
#define FCF_ACK_REQ     (0x8861)
/** @} */

/** \brief Hardcoded various "network" addresses, for use when testing.
 *  \name Pre-defined network addresses
 *  @{
 */
#define DEST_PAN_ID     (0xABCD)
#define SOURCE_PAN_ID   (0xABCD)
#define LONG_ADDR_1     (0xFFEEDDCCBBAA1100LL)
#define LONG_ADDR_2     (0x1122334455667788LL)
#define LONG_ADDR_3     (0xDDEEAADDBBEEEEFFLL)
#define LONG_ADDR_4     (0x0123456789ABCDEFLL)
#define SOURCE_ADDR     (0x1234)
#define FCF_ACK         (0x0002)
#define TX_OPTION_NOACK (0)
#define TX_OPTION_ACK   (1)
#define LONG_ADDR_LEN   (8)
/** @} */

#define MPDU_OVERHEAD   (11) /**< This overhead includes FCF, DSN, DEST_PAN_ID, DEST_ADDR, SOURCE_ADDR, & FCS */


/** \brief These are some definitions of values used in the FCF.  See the 802.15.4 spec for details.
 *  \name FCF element values definitions
 *  @{
 */
#define BEACONFRAME     (0x00)
#define DATAFRAME       (0x01)
#define ACKFRAME        (0x02)
#define CMDFRAME        (0x03)

#define BEACONREQ       (0x07)

#define IEEERESERVED    (0x00)
#define NOADDR          (0x00)      /**< Only valid for ACK or Beacon frames. */
#define SHORTADDRMODE   (0x02)
#define LONGADDRMODE    (0x03)

#define NOBEACONS       (0x0F)

#define BROADCASTADDR   (0xFFFF)
#define BROADCASTPANDID (0xFFFF)

#define IEEE802154_2003 (0x00)
#define IEEE802154_2006 (0x01)

#define SECURITY_LEVEL_NONE (0)
#define SECURITY_LEVEL_128  (3)

#define PSDULEN         (127)
/** @} */


/* typedef enum {TRUE, FALSE} bool; */

typedef struct dataRequest  {
    uint8_t srcAddrMode;
    uint8_t dstAddrMode;
    uint16_t dstPANId;
    addr_t dstAddr;
    uint8_t msduLength;
    uint8_t *msdu;
    uint8_t msduHandle;
    uint8_t txOptions;
    uint8_t securityLevel;
    uint8_t keyIdMode;
    uint8_t *keySource;
    uint8_t keyIndex;
} dataRequest_t;


/*  Macros & Defines */
extern ieee_15_4_manager_t ieee15_4ManagerAddress;
extern dataRequest_t dataRequestStructAddress;
#define ieee15_4Struct (&ieee15_4ManagerAddress)
#define dataRequestStruct (&dataRequestStructAddress)


/**
 *  \name Scan variables
 *  \brief Global variables and defines for scan.
 *  \{
*/
extern uint8_t msduHandle;
extern bool iAmCoord;
extern bool autoModes;
extern uint16_t macShortAddr;
extern uint64_t macLongAddr;
/** @} */

/*  PHY PIB Attributes */

/* uint8_t phyCurrentChannel		  Integer 0-26
 * The RF channel to use for all following transmissions and receptions (see6.1.2).
 */
extern uint8_t phyCurrentChannel;

/* uint64_t macCoordExtendedAddress -- no default
 *
 *   The 64-bit address of the coordinator/router through which the network layer wishes to communicate
 */
extern uint64_t macCoordExtendedAddress;

/* uint16_t macCoordShortAddress  -- default 0xffff
 *
 *   The 16-bit short address assigned to the coordinator through which the network layer wishes
 *   to communicate.  A value of 0xfffe indicates th the coordinator is only using it's 64-bit
 *   extended address.  A value of 0xffff indicates that this value is unknown.
 */
extern uint16_t macCoordShortAddress;

/* uint64_t macDestAddress  -- default 0xffff
 *
 *   This address is the 64-bit address that will be used as the mechanism to
 *   provide a destination to the upper layers.
 */
extern uint64_t macDestAddress;

/* uint8_t macDSN                 -- default is random value within the range
 *
 *   The sequence number (0x00 - 0xff) added to the transmitted data or MAC command frame.
 */
extern uint8_t macDSN;

/* uint16_t macDstPANId              -- default 0xffff
 *
 *   The 16-bit identifier of the PAN on which the device is sending to.  If this value
 *   is 0xffff, the device is not associated.
 */
extern uint16_t macDstPANId;

/* uint16_t macSrcPANId              -- default 0xffff
 *
 *   The 16-bit identifier of the PAN on which the device is operating.  If this value
 *   is 0xffff, the device is not associated.
 */
extern uint16_t macSrcPANId;

/* uint16_t macShortAddress       -- default 0xffff
 *
 *   The 16-bit address that the device uses to communicate in the PAN.  If the device is the
 *   PAN coordinator, this value shall be chosen before a PAN is started.  Otherwise, the
 *   address is allocated by a coordinator during association.  A value of 0xfffe indicates
 *   that the device has associated but has not been allocated an address.  A value of 0xffff
 *   indicates that the device does not have a short address.
 */
extern uint16_t macShortAddress;

/*  Scan defines */


/*  Protoypes */
void mac_init(void);

#endif

/** @} */
