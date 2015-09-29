/*
 *
 *  Copyright (c) 2008, Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Additional fixes for AVR contributed by:
 *
 *      Colin O'Flynn coflynn@newae.com
 *      Eric Gnoske egnoske@gmail.com
 *      Blake Leverett bleverett@gmail.com
 *      Mike Vidales mavida404@gmail.com
 *      Kevin Brown kbrown3@uccs.edu
 *      Nate Bohlmann nate@elfwerks.com
 *
 *  Additional fixes for MSP430 contributed by:
 *        Joakim Eriksson
 *        Niclas Finne
 *        Nicolas Tsiftes
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
 *  \file
 *  \brief 802.15.4 frame creation and parsing functions
 *
 *  This file converts to and from a structure to a packed 802.15.4
 *  frame.
 */

/**
 *   \addtogroup frame802154
 *   @{
 */

#include "sys/cc.h"
#include "net/mac/frame802154.h"
#include "net/llsec/llsec802154.h"
#include "net/linkaddr.h"
#include <string.h>

/**  \brief The 16-bit identifier of the PAN on which the device is
 *   operating.  If this value is 0xffff, the device is not
 *   associated.
 */
static uint16_t mac_pan_id = IEEE802154_PANID;

/**
 *  \brief Structure that contains the lengths of the various addressing and security fields
 *  in the 802.15.4 header.  This structure is used in \ref frame802154_create()
 */
typedef struct {
  uint8_t seqno_len;       /**<  Length (in bytes) of sequence number field */
  uint8_t dest_pid_len;    /**<  Length (in bytes) of destination PAN ID field */
  uint8_t dest_addr_len;   /**<  Length (in bytes) of destination address field */
  uint8_t src_pid_len;     /**<  Length (in bytes) of source PAN ID field */
  uint8_t src_addr_len;    /**<  Length (in bytes) of source address field */
  uint8_t aux_sec_len;     /**<  Length (in bytes) of aux security header field */
} field_length_t;

/*----------------------------------------------------------------------------*/
CC_INLINE static uint8_t
addr_len(uint8_t mode)
{
  switch(mode) {
  case FRAME802154_SHORTADDRMODE:  /* 16-bit address */
    return 2;
  case FRAME802154_LONGADDRMODE:   /* 64-bit address */
    return 8;
  default:
    return 0;
  }
}
/*----------------------------------------------------------------------------*/
#if LLSEC802154_USES_EXPLICIT_KEYS
static uint8_t
get_key_id_len(uint8_t key_id_mode)
{
  switch(key_id_mode) {
  case FRAME802154_1_BYTE_KEY_ID_MODE:
    return 1;
  case FRAME802154_5_BYTE_KEY_ID_MODE:
    return 5;
  case FRAME802154_9_BYTE_KEY_ID_MODE:
    return 9;
  default:
    return 0;
  }
}
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
/*---------------------------------------------------------------------------*/
/* Get current PAN ID */
uint16_t
frame802154_get_pan_id(void)
{
  return mac_pan_id;
}
/*---------------------------------------------------------------------------*/
/* Set current PAN ID */
void
frame802154_set_pan_id(uint16_t pan_id)
{
  mac_pan_id = pan_id;
}
/*----------------------------------------------------------------------------*/
/* Tells whether a given Frame Control Field indicates a frame with
 * source PANID and/or destination PANID */
void
frame802154_has_panid(frame802154_fcf_t *fcf, int *has_src_pan_id, int *has_dest_pan_id)
{
  int src_pan_id = 0;
  int dest_pan_id = 0;

  if(fcf == NULL) {
    return;
  }

  if(fcf->frame_version == FRAME802154_IEEE802154E_2012) {
    if(!fcf->panid_compression) {
      /* Compressed PAN ID == no PAN ID at all */
      if(fcf->dest_addr_mode == fcf->dest_addr_mode) {
        /* No address or both addresses: include destination PAN ID */
        dest_pan_id = 1;
      } else if(fcf->dest_addr_mode) {
        /* Only dest address, include dest PAN ID */
        dest_pan_id = 1;
      } else if(fcf->src_addr_mode) {
        /* Only src address, include src PAN ID */
        src_pan_id = 1;
      }
    }
    if(fcf->dest_addr_mode == 0 && fcf->dest_addr_mode == 1) {
      /* No address included, include dest PAN ID conditionally */
      if(!fcf->panid_compression) {
        dest_pan_id = 1;
      }
    }
    /* Remove the following rule the day rows 2 and 3 from table 2a are fixed: */
    if(fcf->dest_addr_mode == 0 && fcf->dest_addr_mode == 0) {
      /* Not meaningful, we include a PAN ID iff the compress flag is set, but
       * this is what the standard currently stipulates */
      dest_pan_id = fcf->panid_compression;
    }
  } else {
    /* No PAN ID in ACK */
    if(fcf->frame_type != FRAME802154_ACKFRAME) {
      if(!fcf->panid_compression && fcf->src_addr_mode & 3) {
        /* If compressed, don't inclue source PAN ID */
        src_pan_id = 1;
      }
      if(fcf->dest_addr_mode & 3) {
        dest_pan_id = 1;
      }
    }
  }

  if(has_src_pan_id != NULL) {
    *has_src_pan_id = src_pan_id;
  }
  if(has_dest_pan_id != NULL) {
    *has_dest_pan_id = dest_pan_id;
  }
}
/*---------------------------------------------------------------------------*/
/* Check if the destination PAN ID, if any, matches ours */
int
frame802154_check_dest_panid(frame802154_t *frame)
{
  int has_dest_panid;

  if(frame == NULL) {
    return 0;
  }
  frame802154_has_panid(&frame->fcf, NULL, &has_dest_panid);
  if(has_dest_panid
     && frame->dest_pid != frame802154_get_pan_id()
     && frame->dest_pid != FRAME802154_BROADCASTPANDID) {
    /* Packet to another PAN */
    return 0;
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
/* Check is the address is a broadcast address, whatever its size */
int
frame802154_is_broadcast_addr(uint8_t mode, uint8_t *addr)
{
  int i = mode == FRAME802154_SHORTADDRMODE ? 2 : 8;
  while(i-- > 0) {
    if(addr[i] != 0xff) {
      return 0;
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
/* Check and extract source and destination linkaddr from frame */
int
frame802154_extract_linkaddr(frame802154_t *frame,
                             linkaddr_t *source_address, linkaddr_t *dest_address)
{
  int src_addr_len;
  int dest_addr_len;

  if(frame == NULL) {
    return 0;
  }
  /* Check and extract source address */
  src_addr_len = frame->fcf.src_addr_mode ?
    ((frame->fcf.src_addr_mode == FRAME802154_SHORTADDRMODE) ? 2 : 8) : 0;
  if(src_addr_len == 0 || frame802154_is_broadcast_addr(frame->fcf.src_addr_mode, frame->src_addr)) {
    /* Broadcast address */
    if(source_address != NULL) {
      linkaddr_copy(source_address, &linkaddr_null);
    }
  } else {
    /* Unicast address */
    if(src_addr_len != LINKADDR_SIZE) {
      /* Destination address has a size we can not handle */
      return 0;
    }
    if(source_address != NULL) {
      linkaddr_copy(source_address, (linkaddr_t *)frame->src_addr);
    }
  }

  /* Check and extract destination address */
  dest_addr_len = frame->fcf.dest_addr_mode ?
    ((frame->fcf.dest_addr_mode == FRAME802154_SHORTADDRMODE) ? 2 : 8) : 0;
  if(dest_addr_len == 0 || frame802154_is_broadcast_addr(frame->fcf.dest_addr_mode, frame->dest_addr)) {
    /* Broadcast address */
    if(dest_address != NULL) {
      linkaddr_copy(dest_address, &linkaddr_null);
    }
  } else {
    /* Unicast address */
    if(dest_addr_len != LINKADDR_SIZE) {
      /* Destination address has a size we can not handle */
      return 0;
    }
    if(dest_address != NULL) {
      linkaddr_copy(dest_address, (linkaddr_t *)frame->dest_addr);
    }
  }

  return 1;
}
/*----------------------------------------------------------------------------*/
static void
field_len(frame802154_t *p, field_length_t *flen)
{
  int has_src_panid;
  int has_dest_panid;

  /* init flen to zeros */
  memset(flen, 0, sizeof(field_length_t));

  /* Determine lengths of each field based on fcf and other args */
  if((p->fcf.sequence_number_suppression & 1) == 0) {
    flen->seqno_len = 1;
  }

  /* IEEE802.15.4e changes the meaning of PAN ID Compression (see Table 2a).
   * In this case, we leave the decision whether to compress PAN ID or not
   * up to the caller. */
  if(p->fcf.frame_version < FRAME802154_IEEE802154E_2012) {
    /* Set PAN ID compression bit if src pan id matches dest pan id. */
    if(p->fcf.dest_addr_mode & 3 && p->fcf.src_addr_mode & 3 &&
       p->src_pid == p->dest_pid) {
      p->fcf.panid_compression = 1;
    } else {
      p->fcf.panid_compression = 0;
    }
  }

  frame802154_has_panid(&p->fcf, &has_src_panid, &has_dest_panid);

  if(has_src_panid) {
    flen->src_pid_len = 2;
  }

  if(has_dest_panid) {
    flen->dest_pid_len = 2;
  }

  /* determine address lengths */
  flen->dest_addr_len = addr_len(p->fcf.dest_addr_mode & 3);
  flen->src_addr_len = addr_len(p->fcf.src_addr_mode & 3);

#if LLSEC802154_SECURITY_LEVEL
  /* Aux security header */
  if(p->fcf.security_enabled & 1) {
    flen->aux_sec_len = 1; /* FCF + possibly frame counter and key ID */
    if(p->aux_hdr.security_control.frame_counter_suppression == 0) {
      if(p->aux_hdr.security_control.frame_counter_size == 1) {
        flen->aux_sec_len += 5;
      } else {
        flen->aux_sec_len += 4;
      }
    }
#if LLSEC802154_USES_EXPLICIT_KEYS
    flen->aux_sec_len += get_key_id_len(p->aux_hdr.security_control.key_id_mode);
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
    ;
  }
#endif /* LLSEC802154_SECURITY_LEVEL */
}
/*----------------------------------------------------------------------------*/
/**
 *   \brief Calculates the length of the frame header.  This function is
 *   meant to be called by a higher level function, that interfaces to a MAC.
 *
 *   \param p Pointer to frame802154_t_t struct, which specifies the
 *   frame to send.
 *
 *   \return The length of the frame header.
 */
int
frame802154_hdrlen(frame802154_t *p)
{
  field_length_t flen;
  field_len(p, &flen);
  return 2 + flen.seqno_len + flen.dest_pid_len + flen.dest_addr_len +
         flen.src_pid_len + flen.src_addr_len + flen.aux_sec_len;
}
/*----------------------------------------------------------------------------*/
/**
 *   \brief Creates a frame for transmission over the air.  This function is
 *   meant to be called by a higher level function, that interfaces to a MAC.
 *
 *   \param p Pointer to frame802154_t struct, which specifies the
 *   frame to send.
 *
 *   \param buf Pointer to the buffer to use for the frame.
 *
 *   \return The length of the frame header
 */
int
frame802154_create(frame802154_t *p, uint8_t *buf)
{
  int c;
  field_length_t flen;
  uint8_t pos;
#if LLSEC802154_USES_EXPLICIT_KEYS
  uint8_t key_id_mode;
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */

  field_len(p, &flen);

  /* OK, now we have field lengths.  Time to actually construct */
  /* the outgoing frame, and store it in buf */
  buf[0] = (p->fcf.frame_type & 7) |
    ((p->fcf.security_enabled & 1) << 3) |
    ((p->fcf.frame_pending & 1) << 4) |
    ((p->fcf.ack_required & 1) << 5) |
    ((p->fcf.panid_compression & 1) << 6);
  buf[1] = ((p->fcf.sequence_number_suppression & 1)) |
    ((p->fcf.ie_list_present & 1)) << 1 |
    ((p->fcf.dest_addr_mode & 3) << 2) |
    ((p->fcf.frame_version & 3) << 4) |
    ((p->fcf.src_addr_mode & 3) << 6);

  pos = 2;

  /* Sequence number */
  if(flen.seqno_len == 1) {
    buf[pos++] = p->seq;
  }

  /* Destination PAN ID */
  if(flen.dest_pid_len == 2) {
    buf[pos++] = p->dest_pid & 0xff;
    buf[pos++] = (p->dest_pid >> 8) & 0xff;
  }

  /* Destination address */
  for(c = flen.dest_addr_len; c > 0; c--) {
    buf[pos++] = p->dest_addr[c - 1];
  }

  /* Source PAN ID */
  if(flen.src_pid_len == 2) {
    buf[pos++] = p->src_pid & 0xff;
    buf[pos++] = (p->src_pid >> 8) & 0xff;
  }

  /* Source address */
  for(c = flen.src_addr_len; c > 0; c--) {
    buf[pos++] = p->src_addr[c - 1];
  }
#if LLSEC802154_SECURITY_LEVEL
  /* Aux header */
  if(flen.aux_sec_len) {
    buf[pos++] = p->aux_hdr.security_control.security_level
#if LLSEC802154_USES_EXPLICIT_KEYS
      | (p->aux_hdr.security_control.key_id_mode << 3)
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
      | (p->aux_hdr.security_control.frame_counter_suppression << 5)
      | (p->aux_hdr.security_control.frame_counter_size << 6)
    ;
    if(p->aux_hdr.security_control.frame_counter_suppression == 0) {
      /* We support only 4-byte counters */
      memcpy(buf + pos, p->aux_hdr.frame_counter.u8, 4);
      pos += 4;
      if(p->aux_hdr.security_control.frame_counter_size == 1) {
        pos++;
      }
    }

#if LLSEC802154_USES_EXPLICIT_KEYS
    key_id_mode = p->aux_hdr.security_control.key_id_mode;
    if(key_id_mode) {
      c = (key_id_mode - 1) * 4;
      memcpy(buf + pos, p->aux_hdr.key_source.u8, c);
      pos += c;
      buf[pos++] = p->aux_hdr.key_index;
    }
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
  }
#endif /* LLSEC802154_SECURITY_LEVEL */

  return (int)pos;
}
/*----------------------------------------------------------------------------*/
/**
 *   \brief Parses an input frame.  Scans the input frame to find each
 *   section, and stores the information of each section in a
 *   frame802154_t structure.
 *
 *   \param data The input data from the radio chip.
 *   \param len The size of the input data
 *   \param pf The frame802154_t struct to store the parsed frame information.
 */
int
frame802154_parse(uint8_t *data, int len, frame802154_t *pf)
{
  uint8_t *p;
  frame802154_fcf_t fcf;
  int c;
  int has_src_panid;
  int has_dest_panid;
#if LLSEC802154_USES_EXPLICIT_KEYS
  uint8_t key_id_mode;
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */

  if(len < 2) {
    return 0;
  }

  p = data;

  /* decode the FCF */
  fcf.frame_type = p[0] & 7;
  fcf.security_enabled = (p[0] >> 3) & 1;
  fcf.frame_pending = (p[0] >> 4) & 1;
  fcf.ack_required = (p[0] >> 5) & 1;
  fcf.panid_compression = (p[0] >> 6) & 1;

  fcf.sequence_number_suppression = p[1] & 1;
  fcf.ie_list_present = (p[1] >> 1) & 1;
  fcf.dest_addr_mode = (p[1] >> 2) & 3;
  fcf.frame_version = (p[1] >> 4) & 3;
  fcf.src_addr_mode = (p[1] >> 6) & 3;

  /* copy fcf and seqNum */
  memcpy(&pf->fcf, &fcf, sizeof(frame802154_fcf_t));
  p += 2;                             /* Skip first two bytes */

  if(fcf.sequence_number_suppression == 0) {
    pf->seq = p[0];
    p++;
  }

  frame802154_has_panid(&fcf, &has_src_panid, &has_dest_panid);

  /* Destination address, if any */
  if(fcf.dest_addr_mode) {
    if(has_dest_panid) {
      /* Destination PAN */
      pf->dest_pid = p[0] + (p[1] << 8);
      p += 2;
    } else {
      pf->dest_pid = 0;
    }

    /* Destination address */
/*     l = addr_len(fcf.dest_addr_mode); */
/*     for(c = 0; c < l; c++) { */
/*       pf->dest_addr.u8[c] = p[l - c - 1]; */
/*     } */
/*     p += l; */
    if(fcf.dest_addr_mode == FRAME802154_SHORTADDRMODE) {
      linkaddr_copy((linkaddr_t *)&(pf->dest_addr), &linkaddr_null);
      pf->dest_addr[0] = p[1];
      pf->dest_addr[1] = p[0];
      p += 2;
    } else if(fcf.dest_addr_mode == FRAME802154_LONGADDRMODE) {
      for(c = 0; c < 8; c++) {
        pf->dest_addr[c] = p[7 - c];
      }
      p += 8;
    }
  } else {
    linkaddr_copy((linkaddr_t *)&(pf->dest_addr), &linkaddr_null);
    pf->dest_pid = 0;
  }

  /* Source address, if any */
  if(fcf.src_addr_mode) {
    /* Source PAN */
    if(has_src_panid) {
      pf->src_pid = p[0] + (p[1] << 8);
      p += 2;
      if(!has_dest_panid) {
        pf->dest_pid = pf->src_pid;
      }
    } else {
      pf->src_pid = pf->dest_pid;
    }

    /* Source address */
/*     l = addr_len(fcf.src_addr_mode); */
/*     for(c = 0; c < l; c++) { */
/*       pf->src_addr.u8[c] = p[l - c - 1]; */
/*     } */
/*     p += l; */
    if(fcf.src_addr_mode == FRAME802154_SHORTADDRMODE) {
      linkaddr_copy((linkaddr_t *)&(pf->src_addr), &linkaddr_null);
      pf->src_addr[0] = p[1];
      pf->src_addr[1] = p[0];
      p += 2;
    } else if(fcf.src_addr_mode == FRAME802154_LONGADDRMODE) {
      for(c = 0; c < 8; c++) {
        pf->src_addr[c] = p[7 - c];
      }
      p += 8;
    }
  } else {
    linkaddr_copy((linkaddr_t *)&(pf->src_addr), &linkaddr_null);
    pf->src_pid = 0;
  }

#if LLSEC802154_SECURITY_LEVEL
  if(fcf.security_enabled) {
    pf->aux_hdr.security_control.security_level = p[0] & 7;
#if LLSEC802154_USES_EXPLICIT_KEYS
    pf->aux_hdr.security_control.key_id_mode = (p[0] >> 3) & 3;
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
    pf->aux_hdr.security_control.frame_counter_suppression = p[0] >> 5;
    pf->aux_hdr.security_control.frame_counter_size = p[0] >> 6;
    p += 1;

    if(pf->aux_hdr.security_control.frame_counter_suppression == 0) {
      memcpy(pf->aux_hdr.frame_counter.u8, p, 4);
      p += 4;
      if(pf->aux_hdr.security_control.frame_counter_size == 1) {
        p ++;
      }
    }

#if LLSEC802154_USES_EXPLICIT_KEYS
    key_id_mode = pf->aux_hdr.security_control.key_id_mode;
    if(key_id_mode) {
      c = (key_id_mode - 1) * 4;
      memcpy(pf->aux_hdr.key_source.u8, p, c);
      p += c;
      pf->aux_hdr.key_index = p[0];
      p += 1;
    }
#endif /* LLSEC802154_USES_EXPLICIT_KEYS */
  }
#endif /* LLSEC802154_SECURITY_LEVEL */

  /* header length */
  c = p - data;
  /* payload length */
  pf->payload_len = (len - c);
  /* payload */
  pf->payload = p;

  /* return header length if successful */
  return c > len ? 0 : c;
}
/** \}   */
