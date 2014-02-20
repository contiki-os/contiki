/**
 * Copyright (c) 2013, Calipso project consortium
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or
 * other materials provided with the distribution.
 * 
 * 3. Neither the name of the Calipso nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific
 * prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef IBSS_SETUP_PROCESS_H_
#define IBSS_SETUP_PROCESS_H_

/* Timeout for scanning for the default IBSS network. */
#define IBSS_PROC_SCAN_TIMEOUT		7 * CLOCK_SECOND

#define IBSS_PROC_INIT_DELAY		3 * CLOCK_SECOND

/* Beacon period */
#define BEACON_INTERVAL			200u

/* ATIM Window Duration as a [%] percentage of beacon period. This one is not used, use the setting below!*/
#define ATIM_WINDOW_PERCENTAGE	10

/* ATIM Window Duration, in case it is not a fixed percentage of BCN period. */
#define ATIM_WINDOW_DURATION	100u
	
/* Soft Beacon Period. */
#define SOFT_BEACON_PERIOD		20u

/* Default BSSID which STAs will create or join to. */
#define DEFAULT_BSSID			{0x11, 0x22, 0x33, 0x44, 0x55, 0x66}
/*---------------------------------------------------------------------------*/
PROCESS_NAME(ibss_setup_process);


/* Indicates whether the process of creating / joining the default IBSS is 
 * completed.
 */
bool is_ibss_setup_completed();

/*---------------------------------------------------------------------------*/
#endif /* IBSS_SETUP_PROCESS_H_ */