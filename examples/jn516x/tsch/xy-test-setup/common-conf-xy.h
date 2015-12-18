/*
* Copyright (c) 2015 NXP B.V.
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
* 3. Neither the name of NXP B.V. nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
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
* Author: Theo van Daele <theo.van.daele@nxp.com>
*
*/

/***********************************************************************/
/* Modifiable settings per measurement                                 */
/***********************************************************************/
#ifdef XY_MEASUREMENT_CFG
typedef struct {
  int xstep;
  int ystep;     
  int number_packets;
  int scan_cnt;
} measurement_definition_t;

/* measurement_definition defines the measurement to be done. 
   Multiple entries are allowed. */
static measurement_definition_t measurement_definition[] = {
  {50,5,1250,2},  /* xstep=50mm; ystep=5mm; 1250 packets sent per position; scan in XY plane done twice */  
};
#endif

/* Define hopping sequence for test */
#undef TSCH_CONF_DEFAULT_HOPPING_SEQUENCE
#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE TSCH_HOPPING_SEQUENCE_16_16

/* TX power control for test packets to/from sink */
#define TX_SINK_POWER (-30)

/* Delay executed at start of udp client process in coordinator [minutes] 
   This way experiments that have run at night time can be started earlier */
#define PRE_MEASUREMENT_DELAY 360

/************************************************************************************/
/* Common configurable settings for all device types (coordinator, xy, sink)        */              
/************************************************************************************/
/* IPv6 pre-fix. Just needed to indicate coordinator in system. */
#define UDP_ADDR_PREFIX   0xaaaa
#define MAX_PAYLOAD_LEN   120

/* Use local IPv6 address for the devices in the network */
#define UDP_ADDR_COORDINATOR  fe80:0:0:0:215:8d00:32:daf0
#define UDP_ADDR_SINK         fe80:0:0:0:215:8d00:32:dbb1
#define UDP_ADDR_XY           fe80:0:0:0:215:8d00:35:c8cc

/* Interval to check whether XY and sink are connected */
#define CHECK_CONNECT_INTERVAL  (2 * CLOCK_SECOND)

/* XY control command strings */
#define XY_RESET      ("XY: RESET")
#define XY_OK         ("XY: OK")
#define XY_LOCATION   ("XY: LOCATION")

/* XY table limit values [mm] */
#define X_LIMIT       300
#define Y_LIMIT       350

