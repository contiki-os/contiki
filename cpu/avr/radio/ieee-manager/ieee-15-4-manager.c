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
 */

/**
 *
 * \addtogroup rf230mac
 * \{
 */

/**
 * \file
 * \brief  Interfaces the 802.15.4 MAC to upper network layers.
 *
 * \author
 *         Mike Vidales <mavida404@gmail.com>
 */

#include "zmac.h"
#include "radio.h"
#include "ieee-15-4-manager.h"

/*---------------------------------------------------------------------------*/
static int
wake(void)
{
  /* Wake the radio. */
  return radio_leave_sleep_mode();
}
/*---------------------------------------------------------------------------*/
static int
sleep(void)
{
  /* Sleep the radio. */
  return radio_enter_sleep_mode();
}
/*---------------------------------------------------------------------------*/
static void
set_channel(int channel)
{
  /* Set the channel. */
  phyCurrentChannel = channel;
  radio_set_operating_channel(phyCurrentChannel);
}
/*---------------------------------------------------------------------------*/
static int
get_channel(void)
{
  /* Reads the current channel. */
  phyCurrentChannel = radio_get_operating_channel();
  return phyCurrentChannel;
}
/*---------------------------------------------------------------------------*/
static void
set_dst_panid(int panid)
{
  macDstPANId = panid;
}
/*---------------------------------------------------------------------------*/
static int
get_dst_panid(void)
{
  return macDstPANId;
}
/*---------------------------------------------------------------------------*/
static void
set_src_panid(int panid)
{
  /* Writes the PAN_ID to the radio. */
  macSrcPANId = panid;
  radio_set_pan_id(macSrcPANId);
}
/*---------------------------------------------------------------------------*/
static int
get_src_panid(void)
{
  /* Gets the PAN_ID from the radio. */
  macSrcPANId = radio_get_pan_id();
  return macSrcPANId;
}
/*---------------------------------------------------------------------------*/
static void
set_auto_mode(bool mode)
{
  autoModes = mode;
}
/*---------------------------------------------------------------------------*/
static bool
get_auto_mode(void)
{
  return autoModes;
}
/*---------------------------------------------------------------------------*/
static void
set_long_addr(uint64_t address)
{
  /* Set the Long address in the radio. */
  macLongAddr = address;
  radio_set_extended_address((uint8_t *)&macLongAddr);
}
/*---------------------------------------------------------------------------*/
static uint64_t
get_long_addr(void)
{
  /* Get the Long address from the radio. */
  radio_get_extended_address((uint8_t *)&macLongAddr);
  return macLongAddr;
}
/*---------------------------------------------------------------------------*/
static void
set_short_addr(int address)
{
  /* Set the Short address in the radio. */
  macShortAddress = address;
  radio_set_short_address(macShortAddress);
}
/*---------------------------------------------------------------------------*/
static int
get_short_addr(void)
{
  /* Get the Short address from the radio. */
  macShortAddress = radio_get_short_address();
  return macShortAddress;
}
/*---------------------------------------------------------------------------*/
static void
set_iamcoord_bit(bool iamcoord)
{
    /** Set the iAmCoord bit. */
    iAmCoord = iamcoord;
    radio_set_device_role(iAmCoord);
}
/*---------------------------------------------------------------------------*/
static bool
get_iamcoord_bit(void)
{
    /** Get the iAmCoord bit. */
    iAmCoord = radio_get_device_role();
    return iAmCoord;
}
/*---------------------------------------------------------------------------*/
static void
set_coord_long_addr(uint64_t address)
{
    macCoordExtendedAddress = address;
}
/*---------------------------------------------------------------------------*/
static uint64_t
get_coord_long_addr(void)
{
    return macCoordExtendedAddress;
}
/*---------------------------------------------------------------------------*/
static void
set_coord_short_addr(int address)
{
    macCoordShortAddress = address;
}
/*---------------------------------------------------------------------------*/
static int
get_coord_short_addr(void)
{
    return macCoordShortAddress;
}
/*---------------------------------------------------------------------------*/
static void
set_dest_long_addr(uint64_t address)
{
    macDestAddress = address;
}
/*---------------------------------------------------------------------------*/
static uint64_t
get_dest_long_addr(void)
{
    return macDestAddress;
}
/*---------------------------------------------------------------------------*/
/** \brief initializes the 802.15.4 manager layer.
 *  \param pieee_15_4_manager Pointer to \ref ieee_15_4_manager
 */
void ieee_15_4_init(ieee_15_4_manager_t *pieee_15_4_manager)
{
/* Initialize the IEEE 15.4 manager. */
  pieee_15_4_manager->wake = wake;
  pieee_15_4_manager->sleep = sleep;
  pieee_15_4_manager->set_channel = set_channel;
  pieee_15_4_manager->get_channel = get_channel;
  pieee_15_4_manager->set_dst_panid = set_dst_panid;
  pieee_15_4_manager->get_dst_panid = get_dst_panid;
  pieee_15_4_manager->set_src_panid = set_src_panid;
  pieee_15_4_manager->get_src_panid = get_src_panid;
  pieee_15_4_manager->set_auto_mode = set_auto_mode;
  pieee_15_4_manager->get_auto_mode = get_auto_mode;
  pieee_15_4_manager->set_long_addr = set_long_addr;
  pieee_15_4_manager->get_long_addr = get_long_addr;
  pieee_15_4_manager->set_short_addr = set_short_addr;
  pieee_15_4_manager->get_short_addr = get_short_addr;
  pieee_15_4_manager->set_iamcoord_bit = set_iamcoord_bit;
  pieee_15_4_manager->get_iamcoord_bit = get_iamcoord_bit;
  pieee_15_4_manager->set_coord_long_addr = set_coord_long_addr;
  pieee_15_4_manager->get_coord_long_addr = get_coord_long_addr;
  pieee_15_4_manager->set_coord_short_addr = set_coord_short_addr;
  pieee_15_4_manager->get_coord_short_addr = get_coord_short_addr;
  pieee_15_4_manager->set_dest_long_addr = set_dest_long_addr;
  pieee_15_4_manager->get_dest_long_addr = get_dest_long_addr;
}

/** \} */
