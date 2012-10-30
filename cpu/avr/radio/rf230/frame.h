/*
 *  Copyright (c) 2008, Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Additional fixes for AVR contributed by:
 *        Colin O'Flynn coflynn@newae.com
 *        Eric Gnoske egnoske@gmail.com
 *        Blake Leverett bleverett@gmail.com
 *        Mike Vidales mavida404@gmail.com
 *        Kevin Brown kbrown3@uccs.edu
 *        Nate Bohlmann nate@elfwerks.com
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
 *    \addtogroup frame
 *    @{
 */
/**
 *  \file
 *  \brief 802.15.4 frame creation and parsing functions
 *
 *  This file converts to and from a structure to a packed 802.15.4
 *  frame.
 *
*/



/* Includes */
#ifndef FRAME_UTILS_H
#define FRAME_UTILS_H

#include "hal.h"

/* Macros & Defines */


/**
 * \brief Defines the bitfields of the frame control field (FCF).
 */
typedef union{
    /** \brief Structure of bitfields for the FCF */
    struct{
        uint8_t frameType : 3;          /**< Frame type field, see 802.15.4 */
        bool    securityEnabled : 1;    /**< True if security is used in this frame */
        bool    framePending : 1;       /**< True if sender has more data to send */
        bool    ackRequired : 1;        /**< Is an ack frame required? */
        bool    panIdCompression : 1;   /**< Is this a compressed header? */
        uint8_t reserved : 3;           /**< Unused bits */
        uint8_t destAddrMode : 2;       /**< Destination address mode, see 802.15.4 */
        uint8_t frameVersion : 2;       /**< 802.15.4 frame version */
        uint8_t srcAddrMode : 2;        /**< Source address mode, see 802.15.4 */
    };
    uint16_t word_val; /**< A word-wide value for the entire FCF */
}fcf_t;

/**
 *  \brief Structure that contains the lengths of the various addressing and security fields
 *  in the 802.15.4 header.  This structure is used in \ref frame_tx_create()
 */
typedef struct{
    uint8_t dest_pid_len;    /**<  Length (in bytes) of destination PAN ID field */
    uint8_t dest_addr_len;   /**<  Length (in bytes) of destination address field */
    uint8_t src_pid_len;     /**<  Length (in bytes) of source PAN ID field */
    uint8_t src_addr_len;    /**<  Length (in bytes) of source address field */
    uint8_t aux_sec_len;     /**<  Length (in bytes) of aux security header field */
} field_length_t;

/** \brief 802.15.4 security control bitfield.  See section 7.6.2.2.1 in 802.15.4 specification */
typedef struct{
    uint8_t  security_level : 3; /**< security level      */
    uint8_t  key_id_mode    : 2; /**< Key identifier mode */
    uint8_t  reserved       : 3; /**< Reserved bits       */
} scf_t;

/** \brief 802.15.4 Aux security header */
typedef struct{
    scf_t  security_control;  /**< Security control bitfield */
    uint32_t frame_counter;   /**< Frame counter, used for security */
    uint8_t  key[9];          /**< The key itself, or an index to the key */
} aux_hdr_t;

/**
 *    @brief  Some constants for frame length calculations.
 *            The IEEE 802.15.4 frame has a number of constant/fixed fields that
 *            can be counted to make frame construction and max payload
 *            calculations easier.
 *
 *            These include:
 *            1. FCF                  - 2 bytes       - Fixed
 *            2. Sequence number      - 1 byte        - Fixed
 *            3. Addressing fields    - 4 - 20 bytes  - Variable
 *            4. Aux security header  - 0 - 14 bytes  - Variable
 *            5. CRC                  - 2 bytes       - Fixed
*/
#define FIXEDFRAMEOVERHEAD  (5)

/** \brief A union of short and long address types. Although a node can have
 * both long and short addresses a frame will contain
 * only one of these. Therefore, a union is appropriate here. */
typedef union{
    uint16_t shortAddr;     /**< Short address, two bytes */
    uint64_t longAddr;      /**< Long address, eight bytes */
}ADDR_SIZE_SPEC_t;

/** \brief Structure containing a PAN ID and an address */
typedef struct{
    uint16_t  panId;            /**< PAN ID */
    ADDR_SIZE_SPEC_t addrSpec;  /**< A short or long address */
}PAN_ID_ADDR_SPEC_t;

/** \brief Structure containing both source and destination addresses */
typedef struct{
    PAN_ID_ADDR_SPEC_t  destAddrFields; /**< Destination address */
    PAN_ID_ADDR_SPEC_t  srcAddrFields;  /**< Source address */
}ADDR_FIELD_SPEC_t;

/** \brief Union of both short and long addresses */
typedef union{
    uint16_t addr16;  /**< Short address */
    uint64_t addr64;  /**< Long address */
} addr_t;

/** \brief Strucure used to return that status of the frame create process.
 *   See frame_tx_create() function.*/
typedef struct{
    uint8_t *frame; /**< Pointer to created frame */
    uint8_t length; /**< Length (in bytes) of created frame */
} frame_result_t;

/** \brief Parameters used by the frame_tx_create() function.  These
 *  parameters are used in the 802.15.4 frame header.  See the 802.15.4
 *  specification for details.
 */
typedef struct{
    fcf_t fcf;            /**< Frame control field  */
    uint8_t seq;          /**< Sequence number */
    uint16_t dest_pid;    /**< Destination PAN ID */
    addr_t dest_addr;     /**< Destination address */
    uint16_t src_pid;     /**< Source PAN ID */
    addr_t src_addr;      /**< Source address */
    aux_hdr_t aux_hdr;    /**< Aux security header */
    uint8_t *payload;     /**< Pointer to 802.15.4 frame payload */
    uint8_t payload_len;  /**< Length of payload field */
} frame_create_params_t;


typedef struct{
    fcf_t      * fcf;               /**< The FCF of the frame. */
    uint8_t    * seqNum;            /**< The sequence number of the frame. */
    uint16_t   * dest_pid;          /**< Destination PAN ID. */
    addr_t     * dest_addr;         /**< Destination address. */
    uint16_t   * src_pid;           /**< PAN ID */
    addr_t     * src_addr;          /**< Source address */
    uint8_t    * aux_sec_hdr;       /**< 802.15.4 Aux security header */
    uint8_t    * payload;           /**< Frame payload */
    uint8_t      payload_length;    /**< Length of payload section of frame */
    uint8_t     lqi;                /**< Link quality indication value */
    uint8_t     rssi;               /**< Received signal strength indication value */
    uint32_t    time;               /**< Time stamp of received frame */
    bool   fcs:1;                   /**< True if checksum has passed */
    bool   in_use:1;                /**< Is this frame struct being used? */
} parsed_frame_t;

/* Globals */

//extern FRAME_t rx_frame;

/* Protoypes */

void frame_tx_create(frame_create_params_t *p,frame_result_t *frame_result);
void frame_rx_callback(uint16_t data);
void rx_frame_parse(hal_rx_frame_t *rx_frame, parsed_frame_t *pf);

/** @} */
#endif /* FRAME_UTILS_H */
