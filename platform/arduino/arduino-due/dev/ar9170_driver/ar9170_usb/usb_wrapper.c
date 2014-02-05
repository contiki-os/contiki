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
#include "uhi_vendor.h"
#include "usb_wrapper.h"
#include "string.h"
#include "delay.h"
#include "usb_lock.h"
#include "ar9170.h"
#include "ar9170_debug.h"
#include <stdio.h>
#include "uhd.h"
#include "compiler.h"
#include <time.h>
#include "list.h"
#include "usb_cmd_wrapper.h"
#include "bitops.h"
#include "ar9170.h"
#include "usb_wrapper.h"
#include <stdint-gcc.h>
#include "cc.h"
#include "smalloc.h"


//************************************
// Method:    ar9170_usb_memcpy
// FullName:  ar9170_usb_memcpy
// Access:    public static 
// Returns:   void
// Qualifier: Customized function to handle copying of blocks of 4 bytes [words]
// Parameter: uint32_t * pDest Destination pointer
// Parameter: uint32_t * pSrc  Source memory pointer
// Parameter: uint32_t len Number of words [4 bytes] to copy
//************************************
void ar9170_usb_memcpy(uint32_t *pDest, uint32_t *pSrc, uint32_t len)
{
	uint32_t i;
	
	/* Manually copy the data from the source to the destination */
	for (i=0; i<len; i++) {
		
		*pDest++ = *pSrc++;
	}
}


bool ar9170_usb_send_cmd(completion_t* lock, uint8_t* cmd, uint16_t cmd_len) {
	
	/* Lock asynchronous command transfer. The flag must
	 * be released by the callback function, which is 
	 * called upon transfer completion.
	 */
	__start(lock);
	
	/* Block execution while the endpoint is not available. */
	while(!uhi_vendor_int_is_available());
	
	/* Copy the command data to the 
	 * interrupt endpoint output buffer.
	 */
	memcpy(int_out_buffer, cmd, cmd_len);
	
	/* Start transferring OUT on the interrupt endpoint. 
	 * Return the status of the transfer upon completion. 
	 */
	return (bool) uhi_vendor_int_out_run(int_out_buffer, 
		(iram_size_t) cmd_len, ar9170_interrupt_out_transfer_done);
}


static bool ar9170_usb_send_data(uint8_t* data, uint16_t data_len) {
	
	#if USB_WRAPPER_DEBUG_DEEP
	printf("Send data down [%u].\n",data_len);
	#endif

	/* Wait for the bulk endpoint to become available. */
	if (!uhi_vendor_bulk_is_available()) {
		printf("ERROR: BULK endpoint not available.\n");
		while(!uhi_vendor_bulk_is_available());
	}
	
	
	/* Copy the packet data to the 
	 * interrupt endpoint output buffer.
	 */
	memcpy(bulk_out_buffer, data, data_len);
	
	/* Start transferring OUT on the interrupt endpoint. 
	 * Return the status of the transfer upon completion. 
	 */
	return (bool) uhi_vendor_bulk_out_run(bulk_out_buffer, 
		(iram_size_t) data_len, ar9170_bulk_out_transfer_done);
}


void ar9170_listen_on_interrupt_in()
{
	bool read_result;
	
	memset(int_in_buffer,0,INTR_ENDPOINT_MAX_SIZE);
	
	if (!uhi_vendor_int_is_available()) {
		printf("VENDOR: ERROR: Interrupt transfer endpoint is not available.\n");
		return;
	}
	#if USB_WRAPPER_DEBUG_DEEP
	printf("DEBUG: Register listening on the interrupt endpoint...\n");	
	#endif
	
	read_result = uhi_vendor_int_in_run(int_in_buffer, (iram_size_t)(INTR_ENDPOINT_MAX_SIZE), ar9170_interrupt_in_transfer_done);
	
	if (read_result == 0) {
		printf("ERROR: Interrupt IN Listening register completed with errors!\n");
	}
	
}


void ar9170_listen_on_ctrl_in()
{
	bool read_result;
	
	memset(ctrl_in_buffer,0,INTR_ENDPOINT_MAX_SIZE);
	
	if (!uhi_vendor_int_is_available()) {
		printf("VENDOR: ERROR: Interrupt transfer endpoint is not available.\n");
		return;
	}
	#if USB_WRAPPER_DEBUG_DEEP
	printf("DEBUG: Register listening on the control endpoint...\n");
	#endif
	
	read_result = uhi_vendor_control_in_run(ctrl_in_buffer, (iram_size_t)(CTRL_ENDPOINT_MAX_SIZE), ar9170_control_in_transfer_done);
	
	if (read_result == 0) {
		printf("ERROR: Interrupt IN Listening register completed with errors!\n");
	}
	
}


void ar9170_interrupt_in_transfer_done( usb_add_t add, usb_ep_t ep, uhd_trans_status_t status, iram_size_t nb_transfered )
{
	#if USB_WRAPPER_DEBUG
	int i;
	#endif
	bool reschedule_flag = false;
	uint8_t* buffer = NULL;
		
	switch (status)
	{
		case UHD_TRANS_NOERROR:
			#if USB_WRAPPER_DEBUG_DEEP
			printf("Interrupt IN success.\n");
			#endif
			reschedule_flag = true;
			#if USB_WRAPPER_DEBUG_DEEP
			printf("Number of bytes transfered: %u.\n",(unsigned int)nb_transfered);
			for (i=0; i<nb_transfered; i++) {
				printf("%02x ", int_in_buffer[i]);	
			}
			printf(" \n");
			#endif			
			buffer = (uint8_t*)malloc(nb_transfered); // TODO Must be freed later
			memcpy(buffer, int_in_buffer, nb_transfered);		
			memset(int_in_buffer, 0, INTR_ENDPOINT_MAX_SIZE);
			break;
		case UHD_TRANS_DISCONNECT:
			printf("ERROR: Interrupt IN disconnect error.\n");
			break;
		case UHD_TRANS_CRC:
		case UHD_TRANS_DT_MISMATCH:
		case UHD_TRANS_PIDFAILURE:
			printf("ERROR: Interrupt IN CRC / MISMATCH / PIDFAILURE error.\n");
			break;
		case UHD_TRANS_STALL:
			printf("ERROR: Interrupt IN STALL error.\n");
			break;
		case UHD_TRANS_NOTRESPONDING:
			printf("ERROR: Interrupt IN Not Responding error.\n");
			break;
		case UHD_TRANS_TIMEOUT:
			#if USB_WRAPPER_DEBUG_DEEP
			printf("Interrupt IN has timed-out. \n");
			#endif
			reschedule_flag = true;
			break;
		default:
			printf("ERROR: Unrecognized status: %d.\n",status);
	}
	if (reschedule_flag) {
		#if USB_WRAPPER_DEBUG_DEEP
		printf("DEBUG: Rescheduling listening on the interrupt endpoint.\n");
		#endif		
		ar9170_listen_on_interrupt_in();
	}
	if (status == UHD_TRANS_NOERROR) {
		struct ar9170* ar = ar9170_get_device();
		ar9170_handle_command_response(ar, buffer, (uint32_t)nb_transfered);
	}
	
}


void ar9170_listen_on_bulk_in()
{	
	/* Signal an error if the bulk endpoint is not available. */	
	if (!uhi_vendor_bulk_is_available()) {
		printf("ERROR: Bulk transfer endpoint is not available.\n");
		return;
	}
	
	#if USB_WRAPPER_DEBUG_DEEP
	printf("DEBUG: Register listening on the bulk endpoint...\n");
	#endif
	
	if (!uhi_vendor_bulk_in_run((COMPILER_WORD_ALIGNED uint8_t*)bulk_in_buffer, 
		(iram_size_t)(BULK_ENDPOINT_MAX_SIZE), ar9170_bulk_in_transfer_done)) {
		/* Signal an error. */	
		printf("ERROR: BULK IN Listening registration completed with errors!\n");		
	}	
}


void ar9170_bulk_in_transfer_done( usb_add_t add, usb_ep_t ep, uhd_trans_status_t status, 
		iram_size_t nb_transfered )
{	
	
	int i;
	switch (status)
	{ 
		case UHD_TRANS_NOERROR:
			/* Increment indicator so we make use of the ring buffer, designed 
			 * for fast data extraction and rescheduling.
			 */
			//bulk_in_buffer_indicator++;
			//bulk_in_buffer_indicator %= AR9170_BULK_TRANSFER_IN_BUFFER_NUM;
			/* Copy response to an available buffer */
			//memcpy(&bulk_in_buffer_pool[bulk_in_buffer_indicator][0], bulk_in_buffer, nb_transfered);	
						
			if (not_expected(nb_transfered >= BULK_ENDPOINT_MAX_SIZE)) {
				printf("ERROR: Cannot handle such large response.\n");			
				/* Just reschedule. However, I think here the program will crash, anyway. */
				ar9170_listen_on_bulk_in();
				return;
			}
			/* Copy response to an available buffer */	
			ar9170_usb_memcpy(&bulk_in_buffer_pool[0][0], bulk_in_buffer, DIV_ROUND_UP(nb_transfered,4));			
			
			/* Reschedule immediately the listening process on the Bulk IN endpoint. 
			 * Although a Bulk IN interrupt can only occur after this interrupt code
			 * is over, we must schedule this listening immediately, as data might be
			 * lost, if e.g. the re-scheduling is done after the current received data
			 * are processed.
			 */
			ar9170_listen_on_bulk_in();							
			/* Handle the bulk IN response now. */	
			goto handle_ok;
			
		case UHD_TRANS_DISCONNECT:
			printf("ERROR: Bulk IN disconnect error.\n");
		break;
			case UHD_TRANS_CRC:
			case UHD_TRANS_DT_MISMATCH:
			case UHD_TRANS_PIDFAILURE:
			printf("ERROR: Bulk IN CRC / MISMATCH / PIDFAILURE error.\n");
			break;
		case UHD_TRANS_STALL:
			printf("ERROR: Bulk IN STALL error.\n");
			break;
		case UHD_TRANS_ABORTED:
			printf("ERROR: Bulk IN Aborted.\n");
			break;
		case UHD_TRANS_NOTRESPONDING:
			printf("ERROR: Bulk IN Not Responding error.\n");
			break;
		case UHD_TRANS_TIMEOUT: /* Reschedule immediately. */
			printf("WARNING: Bulk IN has timed-out. Will reschedule.\n");
			ar9170_listen_on_bulk_in();
			break;
		default:
			printf("ERROR: Unrecognized status response: %d.\n",status);
	}	
	return;
	
handle_ok:	
			
	#if USB_WRAPPER_DEBUG_DEEP
	printf("BULK IN [%u]: ",(unsigned int)nb_transfered);		
	for (i=0; i<nb_transfered; i++) {
		printf("%02x ", ((uint8_t*)bulk_in_buffer)[i]);
	}
	printf(" \n");
	#endif
	/* Handle response. The response is handled within the interrupt
	 * context and can be a reason for delaying critical operations,
	 * so we are going to handle only the [critical] command responses
	 * but not the received packets.
	 */	
	__ar9170_rx(ar9170_get_device(),(uint8_t*)(&bulk_in_buffer_pool[0][0]), (uint32_t)nb_transfered);
}

                                      
void ar9170_control_out_transfer_done (usb_add_t add, uhd_trans_status_t status, uint16_t payload_trans)
{
	switch (status)
	{	
		case UHD_TRANS_NOERROR:
			#if USB_WRAPPER_DEBUG
			printf("CTRL OUT Success. Bytes Transfered: %u.\n",(unsigned int)payload_trans);
			#endif
			break;		
		case UHD_TRANS_TIMEOUT:
			printf("CTRL OUT Timeout.\n");
			break;
		case UHD_TRANS_DISCONNECT:
			printf("CTRL OUT disconnect error.\n");
			break;
		case UHD_TRANS_CRC:
		case UHD_TRANS_DT_MISMATCH:
		case UHD_TRANS_PIDFAILURE:
			printf("CTRL OUT CRC / MISMATCH / PIDFAILURE error.\n");
			break;
		case UHD_TRANS_STALL:
			printf("CTRL OUT STALL error.\n");
			break;
		case UHD_TRANS_ABORTED:
			printf("CTRL OUT Aborted.\n");
			break;
		case UHD_TRANS_NOTRESPONDING:
			printf("CTRL OUT Not Responding error.\n");
			break;
		default:
			printf("ERROR: Sending CTRL OUT unrecognized error: %d.\n",status);
			break;	
	}
	// Release USB lock - TODO - Check for a better way to do this synchronization for CTRL transfer
	ar9170_usb_ctrl_out_release_lock();
}


void ar9170_control_in_transfer_done( usb_add_t add, uhd_trans_status_t status, uint16_t nb_transfered )
{
	#if USB_WRAPPER_DEBUG
	int i;
	#endif
	bool reschedule_flag = false;
	uint8_t* buffer = NULL;
	
	switch (status)
	{
		case UHD_TRANS_NOERROR:
		#if USB_WRAPPER_DEBUG_DEEP
		printf("Control IN success.\n");
		#endif
		reschedule_flag = true;
		#if USB_WRAPPER_DEBUG_DEEP
		printf("Number of bytes transfered: %u.\n",(unsigned int)nb_transfered);
		for (i=0; i<nb_transfered; i++) {
			printf("%02x ", ctrl_in_buffer[i]);
		}
		printf(" \n");
		#endif
		buffer = (uint8_t*)malloc(nb_transfered); // TODO Must be freed later
		memcpy(buffer, ctrl_in_buffer, nb_transfered);
		memset(ctrl_in_buffer, 0, INTR_ENDPOINT_MAX_SIZE);
		break;
		case UHD_TRANS_DISCONNECT:
		printf("ERROR: Interrupt IN disconnect error.\n");
		break;
		case UHD_TRANS_CRC:
		case UHD_TRANS_DT_MISMATCH:
		case UHD_TRANS_PIDFAILURE:
		printf("ERROR: Control IN CRC / MISMATCH / PIDFAILURE error.\n");
		break;
		case UHD_TRANS_STALL:
		printf("ERROR: Control IN STALL error.\n");
		break;
		case UHD_TRANS_NOTRESPONDING:
		printf("ERROR: Control IN Not Responding error.\n");
		break;
		case UHD_TRANS_TIMEOUT:
		#if USB_WRAPPER_DEBUG
		printf("Control IN has timed-out. \n");
		#endif
		reschedule_flag = true;
		break;
		default:
		printf("ERROR: Unrecognized status: %d.\n",status);
	}
	if (reschedule_flag) {
		#if USB_WRAPPER_DEBUG_DEEP
		printf("DEBUG: Rescheduling listening on the Control endpoint.\n");
		#endif
		ar9170_listen_on_ctrl_in();
	}
	if (status == UHD_TRANS_NOERROR) {
		struct ar9170* ar = ar9170_get_device();
		ar9170_handle_command_response(ar, buffer, (uint32_t)nb_transfered);
	}
}


void ar9170_interrupt_out_transfer_done( usb_add_t add, usb_ep_t ep, uhd_trans_status_t status, iram_size_t nb_transfered )
{	
	struct ar9170* ar;	
	switch (status) 
		{
		case UHD_TRANS_NOERROR:
			#if USB_WRAPPER_DEBUG_DEEP
			printf("INT OUT Success. Bytes Transfered: %u.\n",(unsigned int)nb_transfered);
			#endif
			ar = ar9170_get_device();
			
			/*
			 * Remove command from the head of the command list. Update the
			 * head and free the memory of the executed command. 
			 */
			if (ar->cmd_list->buffer == NULL) {
				printf("ERROR: Command list is already NULL although we just received last callback!\n");
			
			} else {
				free(ar->cmd_list->buffer);
				ar->cmd_list->buffer = NULL;
				ar->cmd_list->send_chunk_len = 0;
				struct ar9170_send_list* next_command = ar->cmd_list->next_send_chunk;
				if (ar->cmd_list->next_send_chunk != NULL) {
					free(ar->cmd_list);
					ar->cmd_list = next_command;
				}					
			}
			/* Command transfered, so release the asynchronous wait lock */			
			__complete(&ar->cmd_async_lock);
			break;
		case UHD_TRANS_TIMEOUT:
			printf("ERROR: INT OUT Timeout.\n");
			break;
		case UHD_TRANS_DISCONNECT:
			printf("ERROR: INT OUT disconnect error.\n");
			break;
		case UHD_TRANS_CRC:
		case UHD_TRANS_DT_MISMATCH:
		case UHD_TRANS_PIDFAILURE:
			printf("ERROR: INT OUT CRC / MISMATCH / PIDFAILURE error.\n");
			break;
		case UHD_TRANS_STALL:
			printf("ERROR: INT OUT STALL error.\n");
			break;
		case UHD_TRANS_NOTRESPONDING:
			printf("ERROR: INT OUT Not Responding error.\n");
		/*
			if (!ar9170_usb_send_cmd(ar->cmd_list->buffer, ar->cmd_list->send_chunk_len)) {
				printf("ERROR: Re-submitting command returned errors.\n");
			}
		*/	
			break;
		case UHD_TRANS_ABORTED:
			printf("ERROR: INT OUT Aborted.\n");
			break;
		default:
			printf("ERROR: Sending INT OUT unrecognized error: %d.\n",status);
		break;
	}
	
}


void ar9170_bulk_out_transfer_done( usb_add_t add, usb_ep_t ep, uhd_trans_status_t status, iram_size_t nb_transfered )
{	
	struct ar9170* ar = ar9170_get_device();
	/* Release asynchronous TX lock, so further packets can be submitted down.
	 * Does not really matter where exactly we release this flag, since we do
	 * it inside the interrupt context. Note, however, that we may have lost
	 * race to the status response interrupt, which may have already cleared
	 * the flag.
	 */
	if(ar->tx_async_lock == true) {
		__complete(&ar->tx_async_lock);
	}
	
	
	switch (status) {
		
		case UHD_TRANS_NOERROR:
			
			#if USB_WRAPPER_DEBUG_DEEP
			printf("Bulk OUT Success. Bytes Transfered: %u.\n",(unsigned int)nb_transfered);
			#endif			
			/*
			 * Remove chunk from the head of the command list. Update the
			 * head and free the memory of the sent data chunk. TODO - why
			 * do we do all these inside the interrupt context? Would it be
			 * better if we had a fixed transmission queue and just move it
			 * once more data is sent?
			 */
			if (ar->tx_list->buffer == NULL) {
				printf("ERROR: TX chunk list is already NULL although we just received last callback!\n");
				
			} else {				
				/* Freeing takes roughly 1.5 microseconds. */
				free(ar->tx_list->buffer); 
				ar->tx_list->buffer = NULL;
				ar->tx_list->send_chunk_len = 0;
				struct ar9170_send_list* next_data = ar->tx_list->next_send_chunk;
				if (ar->tx_list->next_send_chunk != NULL) {
					free(ar->tx_list);
					ar->tx_list = next_data;
				}
			}						
			break;
		case UHD_TRANS_TIMEOUT:			
			printf("ERROR: Bulk OUT Timeout.\n");			
			break;
		case UHD_TRANS_DISCONNECT:
			printf("ERROR: Bulk OUT disconnect error.\n");
			break;
		case UHD_TRANS_CRC:
		case UHD_TRANS_DT_MISMATCH:
		case UHD_TRANS_PIDFAILURE:
			printf("Bulk OUT CRC / MISMATCH / PIDFAILURE error.\n");
			break;
		case UHD_TRANS_STALL:
			printf("ERROR: Bulk OUT STALL error.\n");
			break;
		case UHD_TRANS_ABORTED:
			printf("ERROR: Bulk OUT Aborted.\n");
			break;
		case UHD_TRANS_NOTRESPONDING:
			printf("ERROR: Bulk OUT Not Responding error.\n");
			break;
		default:
			printf("ERROR: Sending Bulk OUT unrecognized error: %d.\n",status);
			break;
	}
}


bool ar9170_write_data( uint8_t* data, uint16_t tx_len, bool zero_packet_flag )
{
	int i;
	bool result = true;
	UNUSED(i);
	UNUSED(zero_packet_flag); // TODO - add the option to send through a vendor API command with zero packet disabled

	if (tx_len > BULK_ENDPOINT_MAX_OUT_SIZE) {
		printf("ERROR: Data exceeds maximum length: %d.\n",tx_len);
		return false;
	}
	
	struct ar9170* ar = ar9170_get_device();
	/* Lock access to the tx buffer. */
	__start(&ar->tx_buf_lock);
	
	if (ar->tx_list->buffer == NULL) {
		/* Consistency check. */
		if(ar->tx_list->next_send_chunk != NULL) {
			printf("ERROR: TX buffer is null while next tx chunk is not.\n");
			__complete(&ar->tx_buf_lock);
			return false;
		}
		/*
		 * If the transmit buffer is empty, we place the current chunk in 
		 * the chunks list and execute the tx submit directly. TODO check
		 * how to implement atomic read and write for safer implementation.
		 */
		ar->tx_list->buffer = data;
		ar->tx_list->send_chunk_len = tx_len;
		
		__complete(&ar->tx_buf_lock);
		
		/* Lock asynchronous TX process, so no more
		 * packets can be sent down, until the current
		 * transfer finishes. 
		 */
		__start(&ar->tx_async_lock);
		/* The actual data is sent here. */
		result = ar9170_usb_send_data(data, tx_len);
		
		#if USB_CMD_WRAPPER_DEBUG_DEEP
		printf("Bulk OUT [%u]: ",(unsigned int)tx_len);
		for (i=0; i<tx_len; i++) {
			printf("%02x ", bulk_out_buffer[i]);
		}
		printf(" \n");
		#endif

		if(result == false) {
			printf("ERROR: Data could not be submitted correctly.\n");
			/* Release asynchronous TX process, because we normally 
			 * do not expect the callback to be called and release 
			 * the asynchronous transmission flag.
			 */
			__complete(&ar->tx_async_lock);
			return result;
		}
		
	} else {
		printf("WARNING: Non-empty usb packet list.\n");
		/* If the transmission buffer is non-empty, put the current command 
		 * in the tail of the list and return. 
		 */
		struct ar9170_send_list* next_pos = ar->tx_list;
		while(next_pos->next_send_chunk != NULL) {
			next_pos = next_pos->next_send_chunk;
		}
		next_pos->next_send_chunk = smalloc(sizeof(struct ar9170_send_list));
		if (next_pos->next_send_chunk == NULL) {
			printf("ERROR: Could not allocate memory for data bulk transfer.\n");
			__complete(&ar->tx_buf_lock);
			return false;
		
		} else {
			
			next_pos->next_send_chunk->buffer = data;
			next_pos->next_send_chunk->send_chunk_len = tx_len;
			next_pos->next_send_chunk->next_send_chunk = NULL;
		}
		__complete(&ar->tx_buf_lock);		
	}
	/* Note that the asynchronous transmission flag is locked, and will be
	 * released by the callback, so no more data can be transfered down to
	 * the USB line before the current transmission is over.
	 */
	return result;
}


bool ar9170_usb_write_reg(completion_t* lock, uint8_t* cmd, uint16_t cmd_len)
{	
	int i;
	bool result;
	UNUSED(i);
		
	if (cmd_len > INTR_ENDPOINT_MAX_SIZE) {
		printf("ERROR: Command exceeds maximum length: %d.\n",cmd_len);
		return false;
	}			
	struct ar9170* ar = ar9170_get_device();
	/* Lock execution */
	__start(&ar->cmd_buf_lock);
	
	if (ar->cmd_list->buffer == NULL) {
		if(ar->cmd_list->next_send_chunk != NULL) {
			printf("ERROR: Command buffer is null while next command is not.\n");
			return false;
		}
		/*
 		 * If command buffer is ready, we place the current command in
		 * the list of pending commands, and we attempt the command 
		 * transmission down to the device.
		 */		
		ar->cmd_list->buffer = cmd;
		ar->cmd_list->send_chunk_len = cmd_len;
		
		__complete(&ar->cmd_buf_lock);
		/* Actual command transfer occurs here */
		result = ar9170_usb_send_cmd(lock, cmd, cmd_len);
		
		if(result == false) {
			printf("ERROR: CMD could not be send correctly.\n");
			return result;
		}
		
	} else {
		printf("WARNING: Non-empty command buffer.\n");
		/* If the command buffer is non-empty, put the current command in
		 * the tail of the list and return.
		 */
		int counter = 0;
		struct ar9170_send_list* next_pos = ar->cmd_list;
		while(next_pos->next_send_chunk != NULL) {
			counter++;
			next_pos = next_pos->next_send_chunk;
		}		
		if (counter > AR9170_MAX_CMD_BUFFER_LEN) {
			printf("ERROR: Command buffer reached maximum length!\n");
			return false;
		}
		
		next_pos->next_send_chunk = smalloc(sizeof(struct ar9170_send_list));
		if (next_pos->next_send_chunk == NULL) {
			
			printf("ERROR: Could not allocate memory for register write.\n");
			__complete(&ar->cmd_buf_lock);
			return false;
			
		} else {
			
			next_pos->next_send_chunk->buffer = cmd;
			next_pos->next_send_chunk->send_chunk_len = cmd_len;
			next_pos->next_send_chunk->next_send_chunk = NULL;
		}
		
		__complete(&ar->cmd_buf_lock);
	}
		
	return true;
}
