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
#define RTIMER_DEBUG 0

#if RTIMER_DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "platform-conf.h"
#include "tc.h"
#include "component\component_tc.h"
#include "pmc.h"
#include "rtimer-arch.h"
#include <stdint-gcc.h>
#include "core_cm3.h"
#include "rtimer.h"
#include "sam3x8e.h"
#include "energest.h"








/* Most significant bits of the current time */
uint64_t time_msb = 0;

/* Time of the next rtimer event. Initially set to the maximum value. */
rtimer_clock_t next_rtimer_time = 0;


/************************************************************************/
/* Interrupt handler for the ID_TC0 Interrupt								
 * [Universal time register overflow]									*/
/************************************************************************/
void TC0_Handler(void)
{
	volatile uint32_t ul_dummy, TC_value;
	
	/* Get the current timer counter value from a 32-bit register*/
	TC_value = tc_read_tc(TC0,0);
	
	/* Clear status bit to acknowledge interrupt */
	ul_dummy = tc_get_status(TC0, 0);
	
	if ((ul_dummy & TC_SR_CPCS) == TC_SR_CPCS) {
		
		#if DEBUG
		printf("%08x OV \n",TC_value);
		#endif
		
		time_msb++;
		
		rtimer_clock_t now =  ((rtimer_clock_t)time_msb << 32)|tc_read_tc(TC0,0);
		
		rtimer_clock_t clock_to_wait = next_rtimer_time - now;
		
		if(clock_to_wait <= 0x100000000 && clock_to_wait > 0)
		{ 
			// We must set now the Timer Compare Register.
			
			// Set the auxiliary timer (TC0,1) at the write time interrupt
			tc_write_rc(TC0,1,(uint32_t)clock_to_wait);
			// Set and enable interrupt on RC compare
			tc_enable_interrupt(TC0,1,TC_IER_CPCS);
			// Start the auxiliary timer
			tc_start(TC0,1);
		}
	}
	
	else {
		printf("ERROR: TC: Unknown interrupt.\n");
	}
}

/************************************************************************/
/* Interrupt handler for ID_TC1 interrupt 
   [Auxiliary (real-time event) timer]                                  */
/************************************************************************/
void TC1_Handler(void)
{
	volatile uint32_t ul_dummy;
	/* Clear status bit to acknowledge interrupt */
	ul_dummy = tc_get_status(TC0,1);
	
	#if RTIMER_DEBUG
	if ((ul_dummy & TC_SR_CPCS) == TC_SR_CPCS) {
	#endif
		tc_stop(TC0,1);	// Crucial, since the execution may take long time
	#if RTIMER_DEBUG
		printf("Compare event %16x\r\n", ul_dummy);
	#endif
		tc_disable_interrupt(TC0,1,TC_IDR_CPCS);  // Disable the next compare interrupt XXX XXX
		ENERGEST_ON(ENERGEST_TYPE_IRQ);
		rtimer_run_next(); /* Run the posted task. */
		ENERGEST_OFF(ENERGEST_TYPE_IRQ);
		
	#if RTIMER_DEBUG
	}
	else {
		printf("Unknown interrupt.\n\n");
	}
	#endif
	
}


/************************************************************************/
/* Initialize the rtimer                                                */
/************************************************************************/
void
rtimer_arch_init(void)
{	
	/*
	 * In this implementation we will make use of two Timer Counters (TC).
	 * The first (TC0) is used to count the universal time, and triggers 
	 * an interrupt on overflow. The timer runs always, and counts the total 
	 * time elapsed since device boot. It is used to extract the current time 
	 * (now)
	 *
	 * The second timer (TC1) is used to schedule events in real time.
	 */
	
	/* Configure PMC: Enable peripheral clock on component: TC0, channel 0 [see sam3x8e.h] */
	pmc_enable_periph_clk(ID_TC0);
	/* Configure PMC: Enable peripheral clock on component: TC0, channel 1 [see sam3x8e.h] */
	pmc_enable_periph_clk(ID_TC1);
	
	/* Configure the TC0 for 10500000 Hz and trigger on RC register compare */
	tc_init(TC0, 0, RT_PRESCALER | TC_CMR_CPCTRG);
	/* Configure the TC1 for 10500000 Hz and trigger on RC register compare */
	tc_init(TC0, 1, RT_PRESCALER | TC_CMR_CPCTRG);
	
	/* Disable all interrupts on both counters*/
	tc_disable_interrupt(TC0,0,0b11111111);
	tc_disable_interrupt(TC0,1,0b11111111);
	
	/* Configure the main counter to trigger an interrupt on RC compare (overflow) */
	tc_write_rc(TC0, 0, 0xffffffff);
	
	/* Configure interrupt on the selected timer channel, setting the corresponding callback */
	NVIC_EnableIRQ((IRQn_Type) ID_TC0);
	/* Configure interrupt on the selected timer channel, setting the corresponding callback */
	NVIC_EnableIRQ((IRQn_Type) ID_TC1);
	
	/* Enable TC0 interrupt on RC Compare */
	tc_enable_interrupt(TC0, 0, TC_IER_CPCS);
	
	/* Start the universal time counter */
	tc_start(TC0, 0);
#if DEBUG	
	printf("DEBUG: Universal Real-Timer started.\n");
#endif	
}

/*---------------------------------------------------------------------------*/
void rtimer_arch_disable_irq(void) // XXX Not PORTED!
{
	//ATOMIC(
	//saved_TIM1CFG = INT_TIM1CFG;
	//INT_TIM1CFG = 0;
	//)
}
/*---------------------------------------------------------------------------*/
void rtimer_arch_enable_irq(void) // XXX Not PORTED!
{
	//INT_TIM1CFG = saved_TIM1CFG;
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t rtimer_arch_now(void)
{
	/* Modified, as now time is 64 bit */
	return  ((rtimer_clock_t)time_msb << 32)|(uint64_t)tc_read_tc(TC0,0);
}

/*---------------------------------------------------------------------------*/

void
rtimer_arch_schedule(rtimer_clock_t t)
{
	#if DEBUG
	printf("DEBUG: rtimer_arch_schedule time %llu\r\n", /*((uint32_t*)&t)+1,*/(uint64_t)t);
	#endif

	next_rtimer_time = t;
	
	rtimer_clock_t now = rtimer_arch_now();
	
	rtimer_clock_t clock_to_wait = t - now;
		
	if(clock_to_wait <= 0x100000000){ // We must set now the Timer Compare Register.
	
		// Set the auxiliary timer (TC0,1) at the write time interrupt
		// [clock_to_wait] TODO Check the standard drift and perhaps remove it from the waiting time
		tc_write_rc(TC0,1,(uint32_t)(t-rtimer_arch_now())); 
		// Set and enable interrupt on RC compare
		tc_enable_interrupt(TC0,1,TC_IER_CPCS);
		// Start the auxiliary timer
		tc_start(TC0,1);
		
		#if DEBUG
		now = rtimer_arch_now();
		printf("DEBUG: Timer started on time %llu.\n",now);
		#endif
	}
// else compare register will be set at overflow interrupt closer to the rtimer event.

}