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
#include "hw.h"
#include "ar9170_state.h"
#include "compiler.h"
#include <stdint-gcc.h>
#include "usb_data_wrapper.h"
#include "usb_wrapper.h"
#include "ar9170_debug.h"
#include "dsc.h"
#include "smalloc.h"






void ar9170_usb_tx( struct ar9170* ar, struct sk_buff* skb )
{
	struct ar9170_stream *tx_stream;
	uint8_t *data = (uint8_t*)smalloc(skb->len);
	if (data == NULL) {
		printf("ERROR: Could not allocate memory for data transmission.\n");
		return;
	}
	uint32_t len;

	if (!IS_STARTED(ar)) {
		printf("WARNING: Got packet tx request on a non-started device, so packet will be dropped.\n");
		goto err_drop;	
	}	

	if (ar->fw.tx_stream) {
		printf("WARNING: Firmware supports tx_stream. It should NOT enter here.\n");
		tx_stream = (void *) (skb->data - sizeof(*tx_stream));

		len = skb->len + sizeof(*tx_stream);
		tx_stream->length = cpu_to_le16(len);
		tx_stream->tag = cpu_to_le16(AR9170_TX_STREAM_TAG);
		data = (void*)tx_stream;
	} else {
		#if USB_DATA_WRAPPER_DEBUG_DEEP
		printf("DEBUG: No stream supported [default].\n");
		#endif
		if (skb->data == NULL) {
			printf("ERROR: Finally, packet data is null.\n");
			return;
		}
		memcpy(data ,skb->data, skb->len);
		len = skb->len;
	}

	/* 
	 * Prepare the tx request to be sent down as a bulk data request.
	 */
	if (len > BULK_ENDPOINT_MAX_OUT_SIZE) {
		printf("ERROR: Data chunk is too large [%u]. Drop.\n",len);
		goto err_drop;
	}
	
	if(!ar9170_write_data(data, (uint16_t)len, ZERO_PACKET_FLAG)) {
		printf("ERROR: Transferring data chunk unsuccessful.\n");
	}
	/* Do not free socket buffer resources. They will be freed by the AR9170_async_tx method. */
/*	
	if (skb->data != NULL) {
		free(skb->data);
		skb->data = NULL;
	} else {
		printf("ERROR: socket buffer content is already null.\n");
	}
*/	
	/* Socket buffer itself must be freed when the function returns.*/
	return;
	
/*	

	
	usb_fill_bulk_urb(urb, ar->udev, usb_sndbulkpipe(ar->udev,
	AR9170_USB_EP_TX), data, len,
	carl9170_usb_tx_data_complete, skb);

	urb->transfer_flags |= URB_ZERO_PACKET;

	usb_anchor_urb(urb, &ar->tx_wait);

	usb_free_urb(urb);
	
	carl9170_usb_submit_data_urb(ar);
	return;
*/
err_drop:
	printf("TX_DROP\n");
	ar9170_tx_drop(ar, skb);
	//carl9170_tx_callback(ar, skb);	
	/* Perhaps this freeing can be done in drop FIXME */
	free(skb->data);
	skb->data = NULL;
	/* Socket buffer itself must be freed when the function returns.*/
	return;
}
