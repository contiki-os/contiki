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
#include "compiler.h"

#ifndef USB_LOCK_H_
#define USB_LOCK_H_

/* USB Action Indicators. Used by the USB host
 * controlled that can inform relevant processes.
 */
#define AR9170_MSC_DEVICE_PLUGGED			BIT(0)
#define AR9170_MSC_DEVICE_UNPLUGGED			~BIT(0)
#define AR9170_WLAN_DEVICE_PLUGGED			BIT(1)
#define AR9170_WLAN_DEVICE_UNPLUGGED		~BIT(1)
#define AR9170_MSC_DEVICE_EJECT_SENT		BIT(2)
#define AR9170_MSC_DEVICE_EJECT_NOT_SENT	~BIT(2)
#define AR9170_WLAN_DEVICE_ALLOCATED		BIT(3)
#define AR9170_WLAN_DEVICE_NOT_ALLOCATED	~BIT(3)
#define AR9170_WLAN_DEVICE_ADDED			BIT(4)
#define AR9170_WLAN_DEVICE_NOT_ADDED		~BIT(4)

	
typedef volatile bool completion_t;
typedef bool mutex_lock_t;

void __complete(completion_t* flag);
void __wait_for_completion(completion_t* flag);
void __start(completion_t* flag);
void __reset(completion_t* flag);

void __read_lock();
void __read_unlock();

/* Indicators for device connection status. */
void ar9170_set_device_off();
void ar9170_set_mass_storage_device_plugged();
void ar9170_set_mass_storage_device_unplugged();
void ar9170_set_wlan_device_plugged();
void ar9170_set_wlan_device_unplugged();
void ar9170_set_wlan_device_non_allocated();
void ar9170_set_wlan_device_allocated();
void ar9170_set_wlan_device_added();
void ar9170_set_wlan_device_not_added();
void ar9170_set_eject_command_sent();
void ar9170_set_eject_command_not_sent();

bool ar9170_is_mass_storage_device_plugged();
bool ar9170_is_wlan_device_allocated();
bool ar9170_is_wlan_device_added();
bool ar9170_is_wlan_device_plugged();
bool ar9170_is_eject_command_sent();

/* Lock EJECT command execution. */
void ar9170_usb_ctrl_out_init_lock();
void ar9170_usb_ctrl_out_lock();
void ar9170_usb_ctrl_out_release_lock();
bool ar9170_usb_is_semaphore_locked();



#endif /* USB_LOCK_H_ */







