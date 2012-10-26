/*
 *
 *  Copyright (c) 2008, Swedish Institute of Computer Science
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
 *
*/
/*
 *  \brief This file is where the main functions that relate to frame
 *  manipulation will reside.
*/
/**
 *   \addtogroup wireless
 *   @{
*/
/**
 *  \defgroup frame RF230 Frame handling
 *  @{
 */
/**
 *  \file
 *  \brief 802.15.4 frame creation and parsing functions
 *
 *  This file converts to and from a structure to a packed 802.15.4
 *  frame.
 */

/* Includes */
#if defined( __GNUC__ )
#include <avr/io.h>
#include <util/delay.h>
#else /* IAR */
#include <iom1284.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "at86rf230_registermap.h"
#include "radio.h"
#include "frame.h"
//#include "mac_event.h"
#include "zmac.h"
#include "process.h"
#include "sicslowmac.h"



/* Macros & Defines */

/* Some version of radio chip need this set to 2, so define it in Makefile */
#ifndef AUTO_CRC_PADDING
#define AUTO_CRC_PADDING 0
#endif


/* Protoypes */

/* Globals */

/*  Frame handling global variables. */
//FRAME_t rx_frame;    /**< Structure that holds received frames. */
static uint8_t tx_frame_buffer[130];

/* Implementation */

/*----------------------------------------------------------------------------*/
/**
 *   \brief Creates a frame for transmission over the air.  This function is
 *   meant to be called by a higher level function, that interfaces to a MAC.
 *
 *   \param p Pointer to frame_create_params_t struct, which specifies the
 *   frame to send.
 *
 *   \param frame_result Pointer to frame_result_t struct, which will
 *   receive the results of this function, a pointer to the frame
 *   created, and the length of the frame.
 *
 *   \return Nothing directly, though the frame_result structure will be filled
 *   in with a pointer to the frame and the frame length.
*/
void
frame_tx_create(frame_create_params_t *p,frame_result_t *frame_result)
{
    field_length_t flen;
    uint8_t index=0;

    /* init flen to zeros */
    memset(&flen, 0, sizeof(field_length_t));

    /* Determine lengths of each field based on fcf and other args */
    if (p->fcf.destAddrMode){
        flen.dest_pid_len = 2;
    }
    if (p->fcf.srcAddrMode){
        flen.src_pid_len = 2;
    }
    /* Set PAN ID compression bit it src pan if matches dest pan id. */
    if(p->fcf.destAddrMode == p->fcf.srcAddrMode){
        p->fcf.panIdCompression = 1;
    }
    if (p->fcf.panIdCompression){
        /* compressed header, only do dest pid */
        flen.src_pid_len = 0;
    }
    /* determine address lengths */
    switch (p->fcf.destAddrMode){
    case 2:  /* 16-bit address */
        flen.dest_addr_len = 2;
        break;
    case 3:  /* 64-bit address */
        flen.dest_addr_len = 8;
        break;
    default:
        break;
    }
    switch (p->fcf.srcAddrMode){
    case 2:  /* 16-bit address */
        flen.src_addr_len = 2;
        break;
    case 3:  /* 64-bit address */
        flen.src_addr_len = 8;
        break;
    default:
        break;
    }
    /* Aux security header */
    if (p->fcf.securityEnabled){
        switch (p->aux_hdr.security_control.key_id_mode){
        case 0:
            flen.aux_sec_len = 5; /* minimum value */
            break;
        case 1:
            flen.aux_sec_len = 6;
            break;
        case 2:
            flen.aux_sec_len = 10;
            break;
        case 3:
            flen.aux_sec_len = 14;
            break;
        default:
            break;
        }
    }

    /* OK, now we have field lengths.  Time to actually construct */
    /* the outgoing frame, and store it in tx_frame_buffer */
    *(uint16_t *)tx_frame_buffer  = p->fcf.word_val; /* FCF */
    index = 2;
    tx_frame_buffer[index++] = p->seq;           /* sequence number */
    /* Destination PAN ID */
    if (flen.dest_pid_len == 2){
        *(uint16_t *)&tx_frame_buffer[index] = p->dest_pid;
        index += 2;
    }
    /* Destination address */
    switch (flen.dest_addr_len){
    case 2:    /* two-byte address */
        *(uint16_t *)&tx_frame_buffer[index] = p->dest_addr.addr16;
        index += 2;
        break;
    case 8:    /* 8-byte address */
        *(uint64_t *)&tx_frame_buffer[index] = p->dest_addr.addr64;
        index += 8;
        break;
    case 0:
    default:
        break;
    }
    /* Source PAN ID */
    if (flen.src_pid_len == 2){
        *(uint16_t *)&tx_frame_buffer[index] = p->src_pid;
        index += 2;
    }
    /* Source address */
    switch (flen.src_addr_len){
    case 2:    /* two-byte address */
        *(uint16_t *)&tx_frame_buffer[index] = p->src_addr.addr16;
        index += 2;
        break;
    case 8:    /* 8-byte address */
        *(uint64_t *)&tx_frame_buffer[index] = p->src_addr.addr64;
        index += 8;
        break;
    case 0:
    default:
        break;
    }
    /* Aux header */
    if (flen.aux_sec_len){
        memcpy((char *)&tx_frame_buffer[index],
                (char *)&p->aux_hdr,
                flen.aux_sec_len);
        index += flen.aux_sec_len;
    }
    /* Frame payload */
    memcpy((char *)&tx_frame_buffer[index],
            (char *)p->payload,
            p->payload_len);
    index += p->payload_len;

    /* return results */
    frame_result->length = index + AUTO_CRC_PADDING;
    frame_result->frame = tx_frame_buffer;
    return;
}

/*----------------------------------------------------------------------------*/
/**
 *   \brief Parses an input frame.  Scans the input frame to find each
 *   section, and stores the resulting addresses of each section in a
 *   parsed_frame_t structure.
 *
 *   \param rx_frame The input data from the radio chip.
 *   \param pf The parsed_frame_t struct that stores a pointer to each
 *   section of the frame payload.
 */
void rx_frame_parse(hal_rx_frame_t *rx_frame, parsed_frame_t *pf)
{
    /* Pointer to start of AT86RF2xx frame */
    uint8_t *p = rx_frame->data;
    fcf_t *fcf = (fcf_t *)&rx_frame->data;
    static uint8_t frame_dropped = 0;

    /* Uh-oh... please don't overwrite me! */
    if (pf->in_use) {
        
        /* Only post this once when buffer is busy, otherwise you get many postings... */
        if (!frame_dropped) {
            event_object_t event;
            event.event = MAC_EVENT_DROPPED;
            event.data = NULL;
            mac_put_event(&event);
            process_post(&mac_process, event.event, event.data);
        }
        frame_dropped = 1;
        return;
    }


    pf->fcf = (fcf_t *)p;
    pf->seqNum = p+2;
    p += 3;                             /* Skip first three bytes */
	
	if (fcf->frameType == ACKFRAME) {	
		//ACK frames have no addresses and no payload!
		pf->payload_length = 0;
	
	} else {      
		
		/* Destination PID, if any */
		if (fcf->frameType != BEACONFRAME){ /* No destination addresses in Beacon frame */
			pf->dest_pid = (uint16_t *)p;
			p += 2;
			/* Destination address */
			pf->dest_addr = 0;
			if (fcf->destAddrMode == SHORTADDRMODE ||
				fcf->destAddrMode == LONGADDRMODE){
				pf->dest_addr = (addr_t *)p;
				/* Update pointer to account for possible missing addr field */
				if (fcf->destAddrMode == SHORTADDRMODE){
					p += 2;
				}
				if (fcf->destAddrMode == LONGADDRMODE){
					p += 8;
				}
			}
		}
		/* Source PANID */
		pf->src_pid = 0;
		if (!fcf->panIdCompression){
			pf->src_pid = (uint16_t *)p;
			p += 2;
		} else {
			pf->src_pid = pf->dest_pid;
		}
		/* Source address */
		pf->src_addr = (addr_t *)p;
		if (fcf->srcAddrMode == SHORTADDRMODE){
			p += 2;
		}
		if (fcf->srcAddrMode == LONGADDRMODE){
			p += 8;
		}
		/* aux security header, not yet implemented */
		pf->aux_sec_hdr = 0;
		/* payload length */
		pf->payload_length = rx_frame->length - (p - (uint8_t*)&rx_frame->data) - 2;
		/* payload */
		pf->payload = p;
	}

    pf->lqi = rx_frame->lqi;
    pf->fcs = rx_frame->crc;

    /* pass frame to sicslowmac layer */
    event_object_t event;
    event.event = MAC_EVENT_RX;
    event.data = (uint8_t*)pf;
    pf->in_use = 1;
    mac_put_event(&event);
    process_poll(&mac_process);
}

/** \}   */
/** \}   */
