#include "cdc_eem.h"
#include "contiki.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_specific_request.h"
#include "rndis/rndis_task.h"
#include "rndis/rndis_protocol.h"
#include "uip.h"
#include "sicslow_ethernet.h"
#include <stdio.h>

#include <avr/pgmspace.h>
#include <util/delay.h>

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define PRINTF printf
#define PRINTF_P printf_P

extern uint8_t usb_eth_data_buffer[64];


//_____ M A C R O S ________________________________________________________


#define EEMCMD_ECHO                    0x00 ///bmEEMCmd Echo
#define EEMCMD_ECHO_RESPONSE           0x01 ///bmEEMCmd Echo Response
#define EEMCMD_SUSPEND_HINT            0x02 ///bmEEMCmd Suspend Hint
#define EEMCMD_RESPONSE_HINT           0x03 ///bmEEMCmd Response Hint
#define EEMCMD_RESPONSE_COMPLETE_HINT  0x04 ///bmEEMCmd Response Complete Hint
#define EEMCMD_TICKLE                  0x05 ///bmEEMCmd Tickle


void cdc_eem_configure_endpoints() {
	usb_configure_endpoint(TX_EP,         \
					  TYPE_BULK,     \
					  DIRECTION_IN,  \
					  SIZE_64,       \
					  TWO_BANKS,     \
					  NYET_ENABLED);

	usb_configure_endpoint(RX_EP,          \
						  TYPE_BULK,     \
					  DIRECTION_OUT, \
					  SIZE_64,       \
					  TWO_BANKS,     \
					  NYET_ENABLED);
	Usb_reset_endpoint(TX_EP);
	Usb_reset_endpoint(RX_EP);
}

void cdc_eem_process(void) {
	uint16_t datalength;
	uint8_t bytecounter, headercounter;
	uint16_t i;

#ifdef USB_ETH_HOOK_INIT
	static uint8_t doInit = 1;
	if (doInit) {
		USB_ETH_HOOK_INIT();
		doInit = 0;
	}
#endif

	//Connected!

	Usb_select_endpoint(RX_EP);

	//If we have data and a free buffer
	if(Is_usb_receive_out() && (uip_len == 0)) {

		//Read how much (endpoint only stores up to 64 bytes anyway)
		bytecounter = Usb_byte_counter_8();

		//EEM uses 2 bytes as a header
		headercounter = 2;

		uint8_t fail = 0;

		//Hmm.. what's going on here?
		 if (bytecounter < headercounter) {
			 Usb_ack_receive_out();
					//TODO CO done = 1;
		 }

		 //Read EEM Header
		 i = 0;
		 while (headercounter) {
			usb_eth_data_buffer[i] = Usb_read_byte();
			bytecounter--;
			headercounter--;
			i++;
		 }

		//Order is LSB/MSB, so MSN is in usb_eth_data_buffer[1]
		//Bit 15 indicates command packet when set
		if (usb_eth_data_buffer[1] & 0x80) {
			//not a data payload
			datalength = 0;
		} else {
			//'0' indicates data packet
			//Length is lower 14 bits
			datalength = usb_eth_data_buffer[0] | ((usb_eth_data_buffer[1] & 0x3F) << 8);
		}

		/* EEM Command Packet */
		if ((datalength == 0) && (fail == 0))
		{
			uint8_t command;
			uint16_t echoLength;

			//Strip command off
			command = usb_eth_data_buffer[1] & 0x38;
			command = command >> 3;

			//Decode command type
			switch (command)
			{
				/* Echo Request */
				case EEMCMD_ECHO:

					//Get echo length
					echoLength  = (usb_eth_data_buffer[1] & 0x07) << 8; //MSB
					echoLength |= usb_eth_data_buffer[0];               //LSB

					//TODO: everything. oops.

					break;

				/* Everything else: Whatever. */
				case EEMCMD_ECHO_RESPONSE:
				case EEMCMD_SUSPEND_HINT:
				case EEMCMD_RESPONSE_HINT:
				case EEMCMD_RESPONSE_COMPLETE_HINT:
				case EEMCMD_TICKLE:
					break;

				default: break;
			}
		}
		/* EEM Data Packet */
		else if (datalength && (fail == 0))
		{
			//Looks like we've got a live one
#ifdef USB_ETH_HOOK_RX_START
			USB_ETH_HOOK_RX_START();
#endif

			uint16_t bytes_received = 0;
			uint16_t dataleft = datalength;
			U8 * buffer = uip_buf;

			while(dataleft)
			{
				*buffer++ = Usb_read_byte();

				dataleft--;
				bytecounter--;
				bytes_received++;

				//Check if endpoint is done but we are expecting more data
				if ((bytecounter == 0) && (dataleft))
				{
					//ACK previous data
					Usb_ack_receive_out();

					//Wait for new data
					while (!Is_usb_receive_out());

					//Get new data
					bytecounter = Usb_byte_counter_8();

					//ZLP?
					if (bytecounter == 0)
					{
						//Incomplete!!
						break;
					}
				}
			}

			//Ack final data packet
			Usb_ack_receive_out();

#ifdef USB_ETH_HOOK_RX_END
			USB_ETH_HOOK_RX_END();
#endif

			//Packet has CRC, nobody wants that garbage
			datalength -= 4;

			//Send data over RF or to local stack
			if(datalength <= USB_ETH_MTU) {
				USB_ETH_HOOK_HANDLE_INBOUND_PACKET(uip_buf,datalength);
			} else {
				USB_ETH_HOOK_RX_ERROR("Oversized packet");
			}
		} //if (datalength)
	}  //if(Is_usb_receive_out() && (uip_len == 0))
}


/**
 \brief Send a single ethernet frame using EEM
 */
uint8_t eem_send(uint8_t * senddata, uint16_t sendlen, uint8_t led)
{
	//Make a header
	uint8_t header[2];

	//Fake CRC! Add 4 to length for CRC
	sendlen += 4;
	header[0] = (sendlen >> 8) & 0x3f;
	header[1] = sendlen & 0xff;

	//We send CRC seperatly..
	sendlen -= 4;

	//Send Data
	Usb_select_endpoint(TX_EP);
	//Usb_send_in();

	//Wait for ready
	if(usb_endpoint_wait_for_write_enabled()!=0) {
		USB_ETH_HOOK_TX_ERROR("Timeout: write enabled");
		return 0;
	}

#ifdef USB_ETH_HOOK_TX_START
	USB_ETH_HOOK_TX_START();
#endif

	//Send header (LSB then MSB)
	Usb_write_byte(header[1]);
	Usb_write_byte(header[0]);

	//Send packet
	while(sendlen) {
		Usb_write_byte(*senddata);
		senddata++;
		sendlen--;

		//If endpoint is full, send data in
		//And then wait for data to transfer
		if (!Is_usb_write_enabled()) {
			Usb_send_in();

			if(usb_endpoint_wait_for_write_enabled()!=0) {
				USB_ETH_HOOK_TX_ERROR("Timeout: write enabled");
				return 0;
			}
		}

	}

	//CRC = 0xdeadbeef
	//Linux kernel 2.6.31 needs 0xdeadbeef in wrong order,
	//like this: uint8_t crc[4] = {0xef, 0xbe, 0xad, 0xde};
	//This is fixed in 2.6.32 to the correct order (0xde, 0xad, 0xbe, 0xef)
	uint8_t crc[4] = {0xde, 0xad, 0xbe, 0xef};

	sendlen = 4;
	uint8_t i = 0;

	//Send fake CRC
	while(sendlen) {
		Usb_write_byte(crc[i]);
		i++;
		sendlen--;

		//If endpoint is full, send data in
		//And then wait for data to transfer
		if (!Is_usb_write_enabled()) {
			Usb_send_in();
			if(usb_endpoint_wait_for_write_enabled()!=0) {
				USB_ETH_HOOK_TX_ERROR("Timeout: write enabled");
				return 0;
			}
		}
	}

	//Send last data in - also handles sending a ZLP if needed
	Usb_send_in();

#ifdef USB_ETH_HOOK_TX_END
	USB_ETH_HOOK_TX_END();
#endif

    //Wait for ready
	if(usb_endpoint_wait_for_IN_ready()!=0) {
		USB_ETH_HOOK_TX_ERROR("Timeout: IN ready");
		return 0;
	}

	return 1;
}
