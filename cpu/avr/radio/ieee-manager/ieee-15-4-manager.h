/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: ieee-15-4-manager.h,v 1.2 2008/10/14 18:38:09 c_oflynn Exp $
 */
/**
 *  \addtogroup rf230mac
 *  \{
 */

/**
 * \file
 * \brief  Example glue code between the existing MAC code and the
 *         Contiki mac interface
 *
 * \author
 *         Mike Vidales <mavida404@gmail.com>
 *
 * \ingroup ieee_15_4
 *
 */

#ifndef __IEEEMANAGER_H__
#define __IEEEMANAGER_H__

/** \brief The interface structure for the 802.15.4 quasi-MAC. */
typedef struct ieee_15_4_manager{
    /** Turn the MAC layer on. */
    int (* wake)(void);
    /** Turn the MAC layer off. */
    int (* sleep)(void);

    /** Set the operating channel. */
    void (* set_channel)(int channel);
    /** Get the operating channel. */
    int (* get_channel)(void);

    /** Set the Destination PAN_ID. */
    void (* set_dst_panid)(int panid);
    /** Get the Destination PAN_ID. */
    int (* get_dst_panid)(void);

    /** Set the Source PAN_ID. */
    void (* set_src_panid)(int panid);
    /** Get the Source PAN_ID. */
    int (* get_src_panid)(void);

    /** Set the Automatic TRX modes. */
    void (* set_auto_mode)(bool mode);
    /** Get the current state of Automatic TRX modes. */
    bool (* get_auto_mode)(void);

    /** Set the Long Address. */
    void (* set_long_addr)(uint64_t address);
    /** Get the Long Address. */
    uint64_t (* get_long_addr)(void);

    /** Set the Short Address. */
    void (* set_short_addr)(int address);
    /** Get the short Address. */
    int (* get_short_addr)(void);

    /** Set the iAmCoord bit. */
    void (* set_iamcoord_bit)(bool iamcoord);
    /** Get the iAmCoord bit. */
    bool (* get_iamcoord_bit)(void);

    /** Set the Coordinator Long address. */
    void (* set_coord_long_addr)(uint64_t address);
    /** Get the Coordinator Long address. */
    uint64_t (* get_coord_long_addr)(void);

    /** Set the Coordinator Long address. */
    void (* set_coord_short_addr)(int address);
    /** Get the Coordinator Long address. */
    int (* get_coord_short_addr)(void);

    /** Set the Destination address. */
    void (* set_dest_long_addr)(uint64_t address);
    /** Get the Destination address. */
    uint64_t (* get_dest_long_addr)(void);
} ieee_15_4_manager_t;


void ieee_15_4_init(struct ieee_15_4_manager *pieee_15_4_manager);

#endif /* __IEEEMANAGER_H__ */
/** \} */
