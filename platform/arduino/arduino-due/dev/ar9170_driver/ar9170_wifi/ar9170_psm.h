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
#include "ar9170.h"
#include "rtimer.h"
#include "bitops.h"
#include "compiler.h"



#ifndef AR9170_PSM_H_
#define AR9170_PSM_H_


/* Masks for PS update*/
#define		AR9170_PS_UPDATE_TRANSITION_FLAG	0x0f
#define		AR9170_PS_UPDATE_ACTION_SLEEP		0xf0
#define		AR9170_PS_UPDATE_ACTION_WAKE		0x00

#define		AR9170_MAX_ATIM_QUEUE_LEN			16
#define		AR9170_PSM_MAX_AWAKE_NODES_LIST_LEN	32

/* States' enumeration for the AR9170 PSM */
enum ar9170_ps_state {
	/* Device is UP */	
	AR9170_PS_WAKE,
	/* Device is OFF */
	AR9170_PS_SLEEP
};

/* PSM time states */	
enum ar9170_op_state {
	
	AR9170_PRE_TBTT,
	AR9170_ATIM_WINDOW,
	AR9170_TX_WINDOW,
	AR9170_SOFT_BCN_WINDOW
};	

/* PSM Flags */	
#define	AR9170_BCN_SENT		= BIT(1)
#define	AR9170_ATIM_SENT	= BIT(2)
#define	AR9170_ATIM_RECV	= BIT(3)
#define	AR9170_DATA_RECV	= BIT(4)

/*
ar9170_tx_queue* ar9170_psm_can_send_first_pkt( struct ar9170* ar );
bool ar9170_psm_has_sent_beacon(struct ar9170*);
*/
void ar9170_psm_schedule_atim_window_start_interrupt(rtimer_clock_t pre_tbtt_time);
void ar9170_psm_init_rtimer();
void ar9170_psm_schedule_powersave(struct ar9170* ar, bool new_state);
void ar9170_psm_async_tx_data( struct ar9170* ar );
void ar9170_psm_async_tx_mgmt( struct ar9170* ar );
void ar9170_psm_start_soft_beaconing( struct ar9170* ar, rtimer_clock_t current_time );
#endif /* AR9170_PSM_H_ */