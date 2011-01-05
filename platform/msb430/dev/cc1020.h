/*
Copyright 2006, Freie Universitaet Berlin. All rights reserved.

These sources were developed at the Freie Universitaet Berlin, Computer
Systems and Telematics group.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
- Neither the name of Freie Universitaet Berlin (FUB) nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

This software is provided by FUB and the contributors on an "as is"
basis, without any representations or warranties of any kind, express
or implied including, but not limited to, representations or
warranties of non-infringement, merchantability or fitness for a
particular purpose. In no event shall FUB or contributors be liable
for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or
business interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even
if advised of the possibility of such damage.

This implementation was developed by the CST group at the FUB.

For documentation and questions please use the web site
http://scatterweb.mi.fu-berlin.de and the mailinglist
scatterweb@lists.spline.inf.fu-berlin.de (subscription via the Website).
Berlin, 2006
*/

/**
 * \file	cc1020.h
 * \author	FUB ScatterWeb Developers, Michael Baar, Nicolas Tsiftes
 **/

#ifndef CC1020_H
#define CC1020_H

#include "dev/radio.h"

extern const uint8_t cc1020_config_19200[];
extern const uint8_t cc1020_config_115200[];

#if 0
void cc1020_init(const uint8_t* config);
#endif
void cc1020_set_rx(void);
void cc1020_set_tx(void);
void cc1020_set_power(uint8_t pa_power);
int cc1020_carrier_sense(void);
uint8_t cc1020_get_rssi(void);
uint8_t cc1020_get_packet_rssi(void);
int cc1020_sending(void);
#if 0
int cc1020_send(const void *buf, unsigned short size);
int cc1020_read(void *buf, unsigned short size);
void cc1020_set_receiver(void (*recv)(const struct radio_driver *));
int cc1020_on(void);
int cc1020_off(void);
#endif
extern const struct radio_driver cc1020_driver;

PROCESS_NAME(cc1020_sender_process);

#endif
