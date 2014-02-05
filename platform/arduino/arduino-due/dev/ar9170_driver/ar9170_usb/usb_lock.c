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
#include "usb_lock.h"
#include "sleepmgr.h"
#include "ar9170_debug.h"
#include <stdint-gcc.h>
#include "compiler.h"
#include "ar9170.h"


static volatile bool ar9170_usb_semaphore;
static volatile bool ar9170_eject_semaphore;
volatile bool read_lock_flag = false;

/* Device connection status register. */
static volatile uint8_t ar9170_connection_status;

bool ar9170_is_mass_storage_device_plugged() {
	
	return ar9170_connection_status & AR9170_MSC_DEVICE_PLUGGED;
}

bool ar9170_is_wlan_device_plugged() {
	
	return ar9170_connection_status & AR9170_WLAN_DEVICE_PLUGGED;
}

bool ar9170_is_wlan_device_allocated()
{
	return ar9170_connection_status & AR9170_WLAN_DEVICE_ALLOCATED;
}

bool ar9170_is_wlan_device_added() {
	
	return ar9170_connection_status & AR9170_WLAN_DEVICE_ADDED;
}


bool ar9170_is_eject_command_sent() {
	
	return ar9170_connection_status & AR9170_MSC_DEVICE_EJECT_SENT;
}

 void ar9170_set_eject_command_not_sent()
 {
	ar9170_connection_status &= AR9170_MSC_DEVICE_EJECT_NOT_SENT; 
 }
 

void ar9170_set_device_off() {
	
	ar9170_connection_status = 0;
}

/* Indicates that the mass storage device has 
 * just been connected. If the MSC device is 
 * already connected we must signal an error.
 * If the WLAN device is connected, we should
 * signal an error. Finally, we need to update
 * the bits corresponding to the updated status
 * of the device.
 */
void ar9170_set_mass_storage_device_plugged() {
	
	if (ar9170_connection_status & AR9170_MSC_DEVICE_PLUGGED) {
		printf("ERROR: MSC Device is already plugged.\n");
		return;
	
	} else {
		/* MSC is not plugged */
		if (ar9170_connection_status & AR9170_WLAN_DEVICE_PLUGGED) {
			printf("ERROR: MSC Device was plugged while WLAN device is already plugged.\n");
			return;
		
		} else {
			/* MSC and WLAN are not plugged. */
			if (ar9170_connection_status & AR9170_MSC_DEVICE_EJECT_SENT) {
				printf("ERROR: The MSC device has just been plugged, but the EJECT command has been already sent.\n");
				return;
			}
			/* Everything is fine. Continue with flag updates. */
			
			/* SET MSC connection status. */
			ar9170_connection_status |= AR9170_MSC_DEVICE_PLUGGED;
		}
	}	
	/*
	
	if (ar9170_device_started_semaphore == true) {
		printf("ERROR: Device has already started.\n");
	} else {
		printf("Starting device now.\n");
		ar9170_device_started_semaphore = true;
	}
	*/
}

void ar9170_set_mass_storage_device_unplugged() {
	
	if (!(ar9170_connection_status & AR9170_MSC_DEVICE_PLUGGED)) {
		printf("ERROR: MSC DEVICE status was already UNPLUGGED.\n ");
		return;
		
	} else {
		/* MSC was previously plugged. */
		if (ar9170_connection_status & AR9170_WLAN_DEVICE_PLUGGED) {
			printf("ERROR: WLAN device is already plugged, when the MSC was just unplugged.\n");
			return;
			
		} else {
			/* MSC was plugged and WLAN was unplugged. */
			if (!(ar9170_connection_status & AR9170_MSC_DEVICE_EJECT_SENT)) {
				printf("WARNING: The MSC Device was unplugged while the EJECT command was not sent.\n");
				/* It is possible that the unplug did not occur because of the EJECT command. */
			
			} else {
				#if USB_LOCK_DEBUG
				printf("DEBUG: Switching devices in progress...\n");
				#endif
			}
			/* Everything is fine. Continue with flag updates. */
			/* CLEAR the MSC connection status. */
			ar9170_connection_status &= AR9170_MSC_DEVICE_UNPLUGGED;
		}
		
	}
/*	
	if (ar9170_device_started_semaphore == false) {
		printf("ERROR: Device has not yet started.\n");
	} else {
		printf("Unplugging MSC device now.\n");
		ar9170_device_started_semaphore = false;
	}
*/	
}

void ar9170_set_wlan_device_plugged() {
	
	if (ar9170_connection_status & AR9170_MSC_DEVICE_PLUGGED) {
		printf("ERROR: WLAN Device has been plugged, while MSC Device is still plugged.\n");
		return;
	
	} else {
		/* MSC Device is not plugged. */
		if (ar9170_connection_status & AR9170_WLAN_DEVICE_PLUGGED) {
			printf("ERROR: AR9170 WLAN Device is already plugged.\n");
			return;
		
		} else {
			/* MSC and WLAN Devices are not plugged. */
			if (ar9170_connection_status & AR9170_MSC_DEVICE_EJECT_SENT) {
				#if USB_LOCK_DEBUG
				printf("DEBUG: This is a COLD reset.\n");
				#endif
				
			} else {				
				#if USB_LOCK_DEBUG
				printf("DEBUG: This is a WARM reset.\n");
				#endif				
			}
			/* Everything is fine. Continue with flag updates. */
			/* SET the WLAN connection status. */
			ar9170_connection_status |= AR9170_WLAN_DEVICE_PLUGGED;
			/* CLEAR the EJECT command sent status. */
			ar9170_connection_status &= AR9170_MSC_DEVICE_EJECT_NOT_SENT;			
		}
	}
	
/*		
	if (ar9170_wlan_device_started_semaphore == true) {
		printf("ERROR: WLAN Device has already started.\n");
	} else {
		printf("Starting WLAN device now.\n");
		ar9170_wlan_device_started_semaphore = true;
	}
*/	
}

void ar9170_set_wlan_device_unplugged() {
	
	if (ar9170_connection_status & AR9170_MSC_DEVICE_PLUGGED) {
		printf("ERROR: WLAN device has just been unplugged, while the MSC device is still plugged.\n");
		return;
		
	} else {
		/* The MSC Device is unplugged. */
		if (!(ar9170_connection_status & AR9170_WLAN_DEVICE_PLUGGED)) {
			printf("ERROR: WLAN Device was just unplugged, while it was not previously plugged.\n");
			return;
		
		} else {
			/* MSC was unplugged and WLAN was plugged. */
			if (ar9170_connection_status & AR9170_MSC_DEVICE_EJECT_SENT) {
				printf("ERROR: WLAN Device was unplugged, while the EJECT command sent flag is still on.\n");
				return;
			
			} else {
				/* Everything is fine. Continue with flag updates. */
				/* CLEAR the WLAN connection status. */
				ar9170_connection_status &= AR9170_WLAN_DEVICE_UNPLUGGED;
			}
		}
	}
/*	
	if (ar9170_device_unplugged_semaphore == true) {
		printf("ERROR: The device is already unplugged\n");
	} else {
		ar9170_device_unplugged_semaphore = true;
	}
*/	
}

void ar9170_set_eject_command_sent() {
	
	if (!(ar9170_connection_status & AR9170_MSC_DEVICE_PLUGGED)) {
		printf("ERROR: The MSC device is not plugged while the EJECT command was just sent.\n");
		return;
	
	} else {
		if(ar9170_connection_status & AR9170_WLAN_DEVICE_PLUGGED) {
			printf("ERROR: WLAN device is already plugged when the EJECT command is sent.\n");
			return;
		
		} else {
			if (ar9170_connection_status & AR9170_MSC_DEVICE_EJECT_SENT) {
				printf("ERROR: EJECT command has already been sent when trying to set it sent.\n");
				return;
			
			} else {
				/* Everything is fine. Continue with flag updates. */
				/* SET the EJECT command flag.*/
				ar9170_connection_status |= AR9170_MSC_DEVICE_EJECT_SENT;
			}
		}
	}
}

void ar9170_set_wlan_device_allocated() {
	
	if (ar9170_connection_status & AR9170_MSC_DEVICE_PLUGGED) {
		printf("ERROR: Request to allocate device when the MSC device is connected.\n");
		return;
	
	} else {
		if (!(ar9170_connection_status & AR9170_WLAN_DEVICE_PLUGGED)) {
			printf("ERROR: Request to allocate device when WLAN device is not plugged.\n");
			return;
		
		} else {
			if (ar9170_connection_status & AR9170_WLAN_DEVICE_ALLOCATED) {
				printf("ERROR: Request to allocate device when already allocated.\n");
				return;
			
			} else {
				/* Everything is fine. Continue with flag updates. */
				/* SET device allocation flag. */
				ar9170_connection_status |= AR9170_WLAN_DEVICE_ALLOCATED;
			}
		}
		
	}
	
}

void ar9170_set_wlan_device_non_allocated() {
	
	/* There can possibly be many reasons for freeing the resources, 
	 * not only due to device disconnection. 
	 */
	if (ar9170_connection_status & AR9170_MSC_DEVICE_PLUGGED) {
		printf("ERROR: Request to un-allocate device when the MSC device is connected.\n");
		return;
		
	} else {
		if (!(ar9170_connection_status & AR9170_WLAN_DEVICE_ALLOCATED)) {
			printf("ERROR: Request to un-allocate a non-allocated device.\n");
			return;
		
		} else {
			/* Everything is fine. Continue with flag updates. */
			/* CLEAR device allocation flag. */
			ar9170_connection_status &= AR9170_WLAN_DEVICE_NOT_ALLOCATED;
		}
	}					
}

void ar9170_set_wlan_device_added() {
	
	if (ar9170_connection_status & AR9170_MSC_DEVICE_PLUGGED) {
		printf("ERROR: Request to add AR9170 interface while MSC Device is plugged.\n");
		return;
		
	} else {
		if (!(ar9170_connection_status & AR9170_WLAN_DEVICE_PLUGGED)) {
			printf("ERROR: Request to add AR91790 interface while WLAN Device is not plugged.\n");
			return;
			
		} else {
			if (!(ar9170_connection_status & AR9170_WLAN_DEVICE_ALLOCATED)) {
				printf("ERROR: Request to add AR9170 interface while Device not allocated.\n");
				return;
				
			} else {
				if (ar9170_connection_status & AR9170_WLAN_DEVICE_ADDED) {
					printf("ERROR: Request to add AR9170 interface while already added.\n");
					return;
					
				} else {
					/* Everything is fine. Continue with flag updates. */
					/* SET the device added flag. */
					ar9170_connection_status |= AR9170_WLAN_DEVICE_ADDED;
				}
			}
		}
	}	
}

void ar9170_set_wlan_device_not_added() {
	
	if (ar9170_connection_status & AR9170_MSC_DEVICE_PLUGGED) {
		printf("ERROR: Request to remove AR9170 interface while MSC Device is plugged.\n");
		return;
		
	} else {
		if (ar9170_connection_status & AR9170_WLAN_DEVICE_PLUGGED) {
			printf("ERROR: Request to remove AR91790 interface while WLAN Device is plugged.\n");
			return;
			
		} else {
			if (ar9170_connection_status & AR9170_WLAN_DEVICE_ALLOCATED) {
				printf("ERROR: Request to remove AR9170 interface while Device allocated.\n");
				return;
				
			} else {
				if (!(ar9170_connection_status & AR9170_WLAN_DEVICE_ADDED)) {
					printf("ERROR: Request to remove AR9170 interface while already removed.\n");
					return;
					
				} else {
					/* Everything is fine. Continue with flag updates. */
					/* Clear the device added flag. */
					ar9170_connection_status &= AR9170_WLAN_DEVICE_NOT_ADDED;
				}
			}
		}
	}
}



void ar9170_usb_ctrl_out_lock()
{
	if (ar9170_usb_semaphore == false) {
		printf("Locking USB semaphore.\n");
		ar9170_usb_semaphore = true;
	} else {
		printf("ERROR: The USB semaphore is already locked!\n");
	}		
}



void ar9170_usb_ctrl_out_release_lock()
{
	if (ar9170_usb_semaphore == true) {
		printf("Releasing USB semaphore.\n");
		ar9170_usb_semaphore = false;
	} else {
		printf("ERROR: The USB semaphore was NOT locked!\n");
	}
}

bool ar9170_usb_is_semaphore_locked()
{
	return ar9170_usb_semaphore;
}



void ar9170_usb_ctrl_out_init_lock()
{
	
	printf("Initializing USB lock.\n");
	ar9170_usb_semaphore = false;
}

void __complete( completion_t* flag )
{
	if (not_expected(*flag == false)) {
		printf("ERROR: __complete(): Flag should have been set.\n");
		struct ar9170* ar = ar9170_get_device();
		if (flag == (&ar->cmd_lock))
			printf("cmd_lock\n");
		else if (flag == (&ar->cmd_wait)) 			
			printf("cmd_wait\n");
		else if (flag == (&ar->cmd_buf_lock))
			printf("cmd_buf_lock\n");
		else if (flag == (&ar->tx_buf_lock))
			printf("tx_buf_lock\n");
		else if (flag == (&ar->tx_async_lock))		
			printf("tx_async_lock\n");
		else if (flag == (&ar->tx_data_wait))
			printf("tx_data_wait\n");
		else if (flag == (&ar->tx_atim_wait))
			printf("tx_atim_wait\n");
		else if (flag == (&ar->clear_tx_data_wait_at_next_tbtt))			
			printf("clear_tx\n");
		else if (flag == (&ar->clear_tx_async_lock_at_next_tbtt))	
			printf("clear_async\n");
		else if (flag == (&ar->clear_cmd_async_lock_at_next_tbtt))
			printf("clear_cmd\n");
		else if (flag == (&ar->cmd_async_lock))
			printf("cmd_async_lock\n");
		else if (flag == (&ar->state_lock))
			printf("state_lock\n");
		else if (flag == (&ar->fw_boot_wait))
			printf("fw_Boot_wait\n");
		else		
			printf("wtf?\n");
		
	} else {
		*flag = false;
		#if USB_LOCK_DEBUG
		printf("DEBUG: Task completed.\n");
		#endif
	}
}


void __wait_for_completion( completion_t* flag )
{
	#if USB_LOCK_DEBUG
	printf("DEBUG: Must wait for command completion...\n");
	#endif
	do {
		sleepmgr_enter_sleep();
		completion_t not_ready = *flag;
		if (not_ready == false) {
			break;
		}
	} while(true);
}


void __start(completion_t* flag )
{
	if (not_expected(*flag == true)) {
		printf("ERROR: _start(): Flag should not have been set!\n");
		struct ar9170* ar = ar9170_get_device();
		if (flag == (&ar->cmd_lock))
			printf("cmd_lock\n");
		else if (flag == (&ar->cmd_wait))
			printf("cmd_wait\n");
		else if (flag == (&ar->cmd_buf_lock))
			printf("cmd_buf_lock\n");
		else if (flag == (&ar->tx_buf_lock))
			printf("tx_buf_lock\n");
		else if (flag == (&ar->tx_async_lock))
			printf("tx_async_lock\n");
		else if (flag == (&ar->tx_data_wait))
			printf("tx_data_wait\n");
		else if (flag == (&ar->tx_atim_wait))
			printf("tx_atim_wait\n");
		else if (flag == (&ar->clear_tx_data_wait_at_next_tbtt))
			printf("clear_tx\n");
		else if (flag == (&ar->clear_cmd_async_lock_at_next_tbtt))
			printf("clear_async\n");
		else if (flag == (&ar->clear_cmd_async_lock_at_next_tbtt))
			printf("clear_cmd\n");
		else
			printf("wtf?\n");
	} else {
		#if USB_LOCK_DEBUG
		printf("DEBUG: Task started.\n");
		#endif
		*flag = true;
	}
}

void __read_lock() {
	
	__wait_for_completion((completion_t*)&read_lock_flag);
	read_lock_flag = true;
}

void __read_unlock() {
	read_lock_flag = false;
}
