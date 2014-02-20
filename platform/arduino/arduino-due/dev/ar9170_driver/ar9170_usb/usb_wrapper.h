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
#include "usb_protocol.h"
#include "compiler.h"
#include "uhd.h"
#include "ar9170.h"
#include "usb_lock.h"
#include <stdint-gcc.h>



#ifndef USB_WRAPPER_H_
#define USB_WRAPPER_H_

// USB device endpoint transfer sizes
#define INTR_ENDPOINT_MAX_SIZE			AR9170_USB_REG_MAX_BUF_SIZE
#define CTRL_ENDPOINT_MAX_SIZE			AR9170_USB_REG_MAX_BUF_SIZE
/* The maximum size of he BULK Endpoint is chosen in such a way that
 * all possible cases regarding response sizes are covered. It seems
 * there is a bug in the USB stack, which makes the program crash if
 * a response larger than the declared maximum size arrives. 2048 is
 * a workaround, i.e. we have measured the device responses for long
 * times and conclude that this value covers all cases. Note that we
 * can not theoretically compute the maximum response size directly
 * from the maximum 802.11 frame-in-the-air size, as, unfortunately, 
 * the AR9170 may group responses.
 */
#define BULK_ENDPOINT_MAX_SIZE			2048
#define BULK_ENDPOINT_MAX_OUT_SIZE		1024
#define BULK_ENDPOINT_MAX_SIZE_WORD		BULK_ENDPOINT_MAX_SIZE / 4

#define AR9170_CMD_RETRIES	3

// USB transfer buffers
COMPILER_WORD_ALIGNED uint8_t int_in_buffer[INTR_ENDPOINT_MAX_SIZE];
COMPILER_WORD_ALIGNED uint8_t int_out_buffer[INTR_ENDPOINT_MAX_SIZE];

COMPILER_WORD_ALIGNED uint32_t bulk_in_buffer[BULK_ENDPOINT_MAX_SIZE_WORD];
COMPILER_WORD_ALIGNED uint32_t bulk_in_buffer_pool[AR9170_BULK_TRANSFER_IN_BUFFER_NUM][BULK_ENDPOINT_MAX_SIZE_WORD];
COMPILER_WORD_ALIGNED uint8_t bulk_out_buffer[BULK_ENDPOINT_MAX_SIZE];
//COMPILER_WORD_ALIGNED uint8_t bulk_out_buffer[AR9170_BULK_TRANSFER_OUT_BUFFER_NUM][BULK_ENDPOINT_MAX_SIZE];
COMPILER_WORD_ALIGNED uint8_t ctrl_in_buffer[CTRL_ENDPOINT_MAX_SIZE];
COMPILER_WORD_ALIGNED uint8_t ctrl_out_buffer[CTRL_ENDPOINT_MAX_SIZE];

// USB transfer IN functions
void ar9170_listen_on_interrupt_in();
void ar9170_listen_on_bulk_in();
void ar9170_listen_on_ctrl_in();

// USB transfer OUT functions

//************************************
// Method:    ar9170_usb_write_reg
// FullName:  ar9170_usb_write_reg
// Access:    public 
// Returns:   bool	TRUE if the write register command was executed successfully
// Qualifier:
// Parameter: uint8_t * cmd
// Parameter: uint16_t cmd_len
//************************************
bool ar9170_usb_write_reg(completion_t* lock, uint8_t* cmd, uint16_t cmd_len );
bool ar9170_write_data( uint8_t* data, uint16_t len, bool zero_packet_flag );

// Callback functions
void ar9170_interrupt_in_transfer_done(usb_add_t add, usb_ep_t ep, uhd_trans_status_t status, iram_size_t nb_transfered);
void ar9170_bulk_in_transfer_done(usb_add_t add, usb_ep_t ep, uhd_trans_status_t status, iram_size_t nb_transfered);
void ar9170_control_in_transfer_done(usb_add_t add, uhd_trans_status_t status, uint16_t nb_transfered);

void ar9170_interrupt_out_transfer_done(usb_add_t add, usb_ep_t ep, uhd_trans_status_t status, iram_size_t nb_transfered);
void bulk_out_transfer_done(usb_add_t add, usb_ep_t ep, uhd_trans_status_t status, iram_size_t nb_transfered);
void ar9170_control_out_transfer_done(usb_add_t add, uhd_trans_status_t status, uint16_t nb_transfered);
void ar9170_bulk_out_transfer_done( usb_add_t add, usb_ep_t ep, uhd_trans_status_t status, iram_size_t nb_transfered );
bool ar9170_usb_send_cmd(completion_t* lock, uint8_t* cmd, uint16_t cmd_len);
void ar9170_usb_memcpy(uint32_t *pDest, uint32_t *pSrc, uint32_t len);
#endif /* USB_WRAPPER_H_ */




