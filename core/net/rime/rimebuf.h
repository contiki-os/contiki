/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimebuf Rime buffer management
 * @{
 *
 * The rimebuf module does Rime's buffer management.
 */

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: rimebuf.h,v 1.15 2008/06/30 19:57:10 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for the Rime buffer (rimebuf) management
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __RIMEBUF_H__
#define __RIMEBUF_H__

#include "contiki-conf.h"
#include "net/rime/rimeaddr.h"

/**
 * \brief      The size of the rimebuf, in bytes
 */
#ifdef RIMEBUF_CONF_SIZE
#define RIMEBUF_SIZE RIMEBUF_CONF_SIZE
#else
#define RIMEBUF_SIZE 128
#endif

/**
 * \brief      The size of the rimebuf header, in bytes
 */
#ifdef RIMEBUF_CONF_HDR_SIZE
#define RIMEBUF_HDR_SIZE RIMEBUF_CONF_HDR_SIZE
#else
#define RIMEBUF_HDR_SIZE 32
#endif

/**
 * \brief      Clear and reset the rimebuf
 *
 *             This function clears the rimebuf and resets all
 *             internal state pointers (header size, header pointer,
 *             external data pointer). It is used before preparing a
 *             packet in the rimebuf.
 *
 */
void rimebuf_clear(void);

/**
 * \brief      Get a pointer to the data in the rimebuf
 * \return     Pointer to the rimebuf data
 *
 *             This function is used to get a pointer to the data in
 *             the rimebuf. The data is either stored in the rimebuf,
 *             or referenced to an external location.
 *
 *             For outbound packets, the rimebuf consists of two
 *             parts: header and data. The header is accessed with the
 *             rimebuf_hdrptr() function.
 *
 *             For incoming packets, both the packet header and the
 *             packet data is stored in the data portion of the
 *             rimebuf. Thus this function is used to get a pointer to
 *             the header for incoming packets.
 *
 */
void *rimebuf_dataptr(void);

/**
 * \brief      Get a pointer to the header in the rimebuf, for outbound packets
 * \return     Pointer to the rimebuf header
 *
 *             For outbound packets, the rimebuf consists of two
 *             parts: header and data. This function is used to get a
 *             pointer to the header in the rimebuf. The header is
 *             stored in the rimebuf.
 *
 */
void *rimebuf_hdrptr(void);

/**
 * \brief      Get the length of the header in the rimebuf, for outbound packets
 * \return     Length of the header in the rimebuf
 *
 *             For outbound packets, the rimebuf consists of two
 *             parts: header and data. This function is used to get
 *             the length of the header in the rimebuf. The header is
 *             stored in the rimebuf and accessed via the
 *             rimebuf_hdrptr() function.
 *
 */
uint8_t rimebuf_hdrlen(void);


/**
 * \brief      Get the length of the data in the rimebuf
 * \return     Length of the data in the rimebuf
 *
 *             For outbound packets, the rimebuf consists of two
 *             parts: header and data. This function is used to get
 *             the length of the data in the rimebuf. The data is
 *             stored in the rimebuf and accessed via the
 *             rimebuf_dataptr() function.
 *
 *             For incoming packets, both the packet header and the
 *             packet data is stored in the data portion of the
 *             rimebuf. This function is then used to get the total
 *             length of the packet - both header and data.
 *
 */
uint16_t rimebuf_datalen(void);

/**
 * \brief      Get the total length of the header and data in the rimebuf
 * \return     Length of data and header in the rimebuf
 *
 */
uint16_t rimebuf_totlen(void);

/**
 * \brief      Set the length of the data in the rimebuf
 * \param len  The length of the data
 *
 *             For outbound packets, the rimebuf consists of two
 *             parts: header and data. This function is used to set
 *             the length of the data in the rimebuf.
 */
void rimebuf_set_datalen(uint16_t len);

/**
 * \brief      Point the rimebuf to external data
 * \param ptr  A pointer to the external data
 * \param len  The length of the external data
 *
 *             For outbound packets, the rimebuf consists of two
 *             parts: header and data. This function is used to make
 *             the rimebuf point to external data. The function also
 *             specifies the length of the external data that the
 *             rimebuf references.
 */
void rimebuf_reference(void *ptr, uint16_t len);

/**
 * \brief      Check if the rimebuf references external data
 * \retval     Non-zero if the rimebuf references external data, zero otherwise.
 *
 *             For outbound packets, the rimebuf consists of two
 *             parts: header and data. This function is used to check
 *             if the rimebuf points to external data that has
 *             previously been referenced with rimebuf_reference().
 *
 */
int rimebuf_is_reference(void);

/**
 * \brief      Get a pointer to external data referenced by the rimebuf
 * \retval     A pointer to the external data
 *
 *             For outbound packets, the rimebuf consists of two
 *             parts: header and data. The data may point to external
 *             data that has previously been referenced with
 *             rimebuf_reference(). This function is used to get a
 *             pointer to the external data.
 *
 */
void *rimebuf_reference_ptr(void);

/**
 * \brief      Compact the rimebuf
 *
 *             This function compacts the rimebuf by copying the data
 *             portion of the rimebuf so that becomes consecutive to
 *             the header. It also copies external data that has
 *             previously been referenced with rimebuf_reference()
 *             into the rimebuf.
 *
 *             This function is called by the Rime code before a
 *             packet is to be sent by a device driver. This assures
 *             that the entire packet is consecutive in memory.
 *
 */
void rimebuf_compact(void);

/**
 * \brief      Copy from external data into the rimebuf
 * \param from A pointer to the data from which to copy
 * \param len  The size of the data to copy
 * \retval     The number of bytes that was copied into the rimebuf
 *
 *             This function copies data from a pointer into the
 *             rimebuf. If the data that is to be copied is larger
 *             than the rimebuf, only the data that fits in the
 *             rimebuf is copied. The number of bytes that could be
 *             copied into the rimbuf is returned.
 *
 */
int rimebuf_copyfrom(const void *from, uint16_t len);

/**
 * \brief      Copy the entire rimebuf to an external buffer
 * \param to   A pointer to the buffer to which the data is to be copied
 * \retval     The number of bytes that was copied to the external buffer
 *
 *             This function copies the rimebuf to an external
 *             buffer. Both the data portion and the header portion of
 *             the rimebuf is copied. If the rimebuf referenced
 *             external data (referenced with rimebuf_reference()) the
 *             external data is copied.
 *
 *             The external buffer to which the rimebuf is to be
 *             copied must be able to accomodate at least
 *             (RIMEBUF_SIZE + RIMEBUF_HDR_SIZE) bytes. The number of
 *             bytes that was copied to the external buffer is
 *             returned.
 *
 */
int rimebuf_copyto(void *to);

/**
 * \brief      Copy the header portion of the rimebuf to an external buffer
 * \param to   A pointer to the buffer to which the data is to be copied
 * \retval     The number of bytes that was copied to the external buffer
 *
 *             This function copies the header portion of the rimebuf
 *             to an external buffer.
 *
 *             The external buffer to which the rimebuf is to be
 *             copied must be able to accomodate at least
 *             RIMEBUF_HDR_SIZE bytes. The number of bytes that was
 *             copied to the external buffer is returned.
 *
 */
int rimebuf_copyto_hdr(uint8_t *to);

/**
 * \brief      Extend the header of the rimebuf, for outbound packets
 * \param size The number of bytes the header should be extended
 * \retval     Non-zero if the header could be extended, zero otherwise
 *
 *             This function is used to allocate extra space in the
 *             header portion in the rimebuf, when preparing outbound
 *             packets for transmission. If the function is unable to
 *             allocate sufficient header space, the function returns
 *             zero and does not allocate anything.
 *
 */
int rimebuf_hdralloc(int size);

/**
 * \brief      Reduce the header in the rimebuf, for incoming packets
 * \param size The number of bytes the header should be reduced
 * \retval     Non-zero if the header could be reduced, zero otherwise
 *
 *             This function is used to remove the first part of the
 *             header in the rimebuf, when processing incoming
 *             packets. If the function is unable to remove the
 *             requested amount of header space, the function returns
 *             zero and does not allocate anything.
 *
 */
int rimebuf_hdrreduce(int size);

/* Packet attributes stuff below: */

typedef uint16_t rimebuf_attr_t;

struct rimebuf_attr {
/*   uint8_t type; */
  rimebuf_attr_t val;
};
struct rimebuf_addr {
/*   uint8_t type; */
  rimeaddr_t addr;
};

extern const char *rimebuf_attr_strings[];

#define RIMEBUF_ATTR_PACKET_TYPE_DATA 0
#define RIMEBUF_ATTR_PACKET_TYPE_ACK 1
enum {
  RIMEBUF_ATTR_NONE,
  RIMEBUF_ATTR_CHANNEL,
  RIMEBUF_ATTR_PACKET_ID,
  RIMEBUF_ATTR_PACKET_TYPE,
  RIMEBUF_ATTR_EPACKET_ID,
  RIMEBUF_ATTR_EPACKET_TYPE,
  RIMEBUF_ATTR_HOPS,
  RIMEBUF_ATTR_TTL,
  RIMEBUF_ATTR_REXMIT,
  RIMEBUF_ATTR_MAX_REXMIT,
  RIMEBUF_ATTR_NUM_REXMIT,
  RIMEBUF_ATTR_LINK_QUALITY,
  RIMEBUF_ATTR_RSSI,
  RIMEBUF_ATTR_TIMESTAMP,
  RIMEBUF_ATTR_NETWORK_ID,

  RIMEBUF_ATTR_RELIABLE,
  RIMEBUF_ATTR_ERELIABLE,

  RIMEBUF_ADDR_SENDER,
  RIMEBUF_ADDR_RECEIVER,
  RIMEBUF_ADDR_ESENDER,
  RIMEBUF_ADDR_ERECEIVER,

  RIMEBUF_ATTR_MAX
};

#define RIMEBUF_NUM_ADDRS 4
#define RIMEBUF_NUM_ATTRS (RIMEBUF_ATTR_MAX - RIMEBUF_NUM_ADDRS)
#define RIMEBUF_ADDR_FIRST RIMEBUF_ADDR_SENDER


#if RIMEBUF_CONF_ATTRS_INLINE

extern struct rimebuf_attr rimebuf_attrs[];
extern struct rimebuf_addr rimebuf_addrs[];

static int               rimebuf_set_attr(uint8_t type, const rimebuf_attr_t val);
static rimebuf_attr_t    rimebuf_attr(uint8_t type);
static int               rimebuf_set_addr(uint8_t type, const rimeaddr_t *addr);
static const rimeaddr_t *rimebuf_addr(uint8_t type);

static inline int
rimebuf_set_attr(uint8_t type, const rimebuf_attr_t val)
{
/*   rimebuf_attrs[type].type = type; */
  rimebuf_attrs[type].val = val;
  return 1;
}
static inline rimebuf_attr_t
rimebuf_attr(uint8_t type)
{
  return rimebuf_attrs[type].val;
}

static inline int
rimebuf_set_addr(uint8_t type, const rimeaddr_t *addr)
{
/*   rimebuf_addrs[type - RIMEBUF_ADDR_FIRST].type = type; */
  rimeaddr_copy(&rimebuf_addrs[type - RIMEBUF_ADDR_FIRST].addr, addr);
  return 1;
}

static inline const rimeaddr_t *
rimebuf_addr(uint8_t type)
{
  return &rimebuf_addrs[type - RIMEBUF_ADDR_FIRST].addr;
}
#else /* RIMEBUF_CONF_ATTRS_INLINE */
int               rimebuf_set_attr(uint8_t type, const rimebuf_attr_t val);
rimebuf_attr_t rimebuf_attr(uint8_t type);
int               rimebuf_set_addr(uint8_t type, const rimeaddr_t *addr);
const rimeaddr_t *rimebuf_addr(uint8_t type);
#endif /* RIMEBUF_CONF_ATTRS_INLINE */

void              rimebuf_attr_clear(void);
int               rimebuf_attr_isset(uint8_t type);

void              rimebuf_attr_copyto(struct rimebuf_attr *attrs,
				      struct rimebuf_addr *addrs);
void              rimebuf_attr_copyfrom(struct rimebuf_attr *attrs,
					struct rimebuf_addr *addrs);

#define RIMEBUF_ATTRIBUTES(...) { __VA_ARGS__ RIMEBUF_ATTR_LAST }
#define RIMEBUF_ATTR_LAST { RIMEBUF_ATTR_NONE, 0 }

#define RIMEBUF_ATTR_BIT  1
#define RIMEBUF_ATTR_BYTE 8
#define RIMEBUF_ADDRSIZE (sizeof(rimeaddr_t) * RIMEBUF_ATTR_BYTE)

struct rimebuf_attrlist {
  uint8_t type;
  uint8_t len;
};

/* XXX The definitions below will be placed the corresponding .h files: */

#define RMH_ATTRIBUTES  { RIMEBUF_ADDR_ESENDER, RIMEBUF_ADDRSIZE }, \
                        { RIMEBUF_ADDR_ERECEIVER, RIMEBUF_ADDRSIZE }, \
                        { RIMEBUF_ATTR_TTL, RIMEBUF_ATTR_BIT * 5 }, \
                        { RIMEBUF_ATTR_MAX_REXMIT, RIMEBUF_ATTR_BIT * 5 }, \
                        RUC_ATTRIBUTES

#define MH_ATTRIBUTES   { RIMEBUF_ADDR_ESENDER, RIMEBUF_ADDRSIZE }, \
                        { RIMEBUF_ADDR_ERECEIVER, RIMEBUF_ADDRSIZE }, \
                        { RIMEBUF_ATTR_TTL, RIMEBUF_ATTR_BIT * 5 }, \
                        UC_ATTRIBUTES

#define POLITE_ATTRIBUTES ABC_ATTRIBUTES

#define IPOLITE_ATTRIBUTES IBC_ATTRIBUTES

#define NF_ATTRIBUTES   { RIMEBUF_ADDR_ESENDER, RIMEBUF_ADDRSIZE }, \
                        { RIMEBUF_ATTR_HOPS, RIMEBUF_ATTR_BIT * 5 }, \
                        { RIMEBUF_ATTR_EPACKET_ID, RIMEBUF_ATTR_BIT * 4 }, \
                        IPOLITE_ATTRIBUTES

#endif /* __RIMEBUF_H__ */
/** @} */
/** @} */
