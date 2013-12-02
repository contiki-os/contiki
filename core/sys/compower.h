/** \addtogroup sys
 * @{ */

/**
 * \defgroup compower Communication power accounting
 * @{
 *
 * The compower module accumulates power consumption information and
 * attributes it to communication activities. Examples of
 * communication activities are packet transmission, packet reception,
 * and idle listening.
 *
 */

/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * \file
 *         Header file for the communication power accounting module
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef COMPOWER_H_
#define COMPOWER_H_

/**
 * \brief      An activity record that contains power consumption information for a specific communication activity.
 *
 *             This is a structure that holds power information about
 *             a communication activity. It is an opaque structure
 *             with no user-visible elements.
 */
struct compower_activity {
  uint32_t listen, transmit;
};

/**
 * \brief      The default idle communication activity.
 *
 *             This is the idle communication activity, to which all
 *             energy that is not possible to attribute to individual
 *             packets, is attributed. Examples include idle listening
 *             for incoming packets and MAC-level beacon
 *             transmissions.
 */
extern struct compower_activity compower_idle_activity;

/**
 * \brief      Initialize the communication power accounting module.
 *
 *             This function initializes the communication power
 *             accounting module. The function is called by the system
 *             during boot-up.
 */
void compower_init(void);

/**
 * \brief      Accumulate power contumption for a communication activity
 * \param a    A pointer to an activity structure.
 *
 *             This function accumulates power consumption information
 *             for a communication activity. The function typically is
 *             called by a power-saving MAC protocol when the radio is
 *             switched off, or when a packet is received or
 *             transmitted.
 *
 */
void compower_accumulate(struct compower_activity *a);

/**
 * \brief      Clear power consumption information for a communication activity
 * \param a    A pointer to an activity structure.
 *
 *             This function clears any power contumption information
 *             that has previously been accumulated in an activity
 *             structure.
 *
 */
void compower_clear(struct compower_activity *a);

/**
 * \brief      Convert power contumption information to packet attributes
 * \param a    A pointer to an activity structure.
 *
 *             This function converts accumulated power consumption
 *             information for a communication activity to packet
 *             attributes (see \ref packetbufattr "packet attributes").
 */
void compower_attrconv(struct compower_activity *a);

/**
 * \brief      Accumulate power contumption for a communication activity based on energy data in packet attributes
 * \param a    A pointer to an activity structure.
 *
 *             This function accumulates power consumption information
 *             for a communication activity from packet attributes
 *             (see \ref packetbufattr "packet attributes").
 */
void compower_accumulate_attrs(struct compower_activity *a);

#endif /* COMPOWER_H_ */

/** @} */
/** @} */
