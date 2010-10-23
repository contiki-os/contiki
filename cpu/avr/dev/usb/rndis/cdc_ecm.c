#include <stdbool.h>
#include "cdc_ecm.h"
#include "contiki.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_specific_request.h"
#include "rndis/rndis_task.h"
#include "rndis/rndis_protocol.h"
#include "uip.h"
#include "sicslow_ethernet.h"
#include <stdio.h>
#if RF230BB
#include "rf230bb.h"
#endif

#include <avr/pgmspace.h>
#include <util/delay.h>

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define PRINTF printf
#define PRINTF_P printf_P

extern uint8_t usb_eth_data_buffer[64];
static U16 usb_ecm_packet_filter = 0;

#define PACKET_TYPE_PROMISCUOUS		(1<<0)
#define PACKET_TYPE_ALL_MULTICAST	(1<<1)
#define PACKET_TYPE_DIRECTED		(1<<2)
#define PACKET_TYPE_BROADCAST		(1<<3)
#define PACKET_TYPE_MULTICAST		(1<<4)

#define Usb_write_word(x)	Usb_write_byte((x)&0xFF),Usb_write_byte((x>>8)&0xFF)
#define Usb_write_long(x)	Usb_write_word((x)&0xFFFF),Usb_write_word((x>>16)&0xFFFF)

#define Usb_read_word()	((U16)Usb_read_byte()+((U16)Usb_read_byte()<<8))

void
cdc_ecm_set_ethernet_packet_filter(void) {
	usb_ecm_packet_filter = Usb_read_word();

  	Usb_ack_receive_setup();
	Usb_send_control_in();
	usb_endpoint_wait_for_read_control_enabled();
	
	PRINTF_P(PSTR("cdc_ecm: Received SET_ETHERNET_PACKET_FILTER: (0x%04X) "),usb_ecm_packet_filter);
	if(usb_ecm_packet_filter & PACKET_TYPE_PROMISCUOUS) {
		PRINTF_P(PSTR("PROMISCUOUS "));
		USB_ETH_HOOK_SET_PROMISCIOUS_MODE(true);
	} else {
		USB_ETH_HOOK_SET_PROMISCIOUS_MODE(false);
	}

	if(usb_ecm_packet_filter & PACKET_TYPE_ALL_MULTICAST)
		PRINTF_P(PSTR("ALL_MULTICAST "));
	if(usb_ecm_packet_filter & PACKET_TYPE_DIRECTED)
		PRINTF_P(PSTR("DIRECTED "));
	if(usb_ecm_packet_filter & PACKET_TYPE_BROADCAST)
		PRINTF_P(PSTR("BROADCAST "));
	if(usb_ecm_packet_filter & PACKET_TYPE_MULTICAST)
		PRINTF_P(PSTR("MULTICAST "));
		
	PRINTF_P(PSTR("\n"));
}


#define CDC_NOTIFY_NETWORK_CONNECTION	(0x00)
#define CDC_NOTIFY_CONNECTION_SPEED_CHANGE	(0x2A)

void
cdc_ecm_notify_network_connection(uint8_t value) {
#if CDC_ECM_USES_INTERRUPT_ENDPOINT
	Usb_select_endpoint(INT_EP);

	if(!Is_usb_endpoint_enabled()) {
		//PRINTF_P(PSTR("cdc_ecm: cdc_ecm_notify_network_connection: endpoint not enabled\n"));
		return;
	}

	if(usb_endpoint_wait_for_IN_ready()!=0) {
		//PRINTF_P(PSTR("cdc_ecm: cdc_ecm_notify_network_connection: Timeout waiting for interrupt endpoint to be available\n"));
		return;
	}

	Usb_send_control_in();

	Usb_write_byte(0x51); // 10100001b
	Usb_write_byte(CDC_NOTIFY_NETWORK_CONNECTION);
	Usb_write_byte(value);
	Usb_write_byte(0x00);
	Usb_write_word(ECM_INTERFACE0_NB);
	Usb_write_word(0x0000);

	Usb_send_in();
	PRINTF_P(PSTR("cdc_ecm: CDC_NOTIFY_NETWORK_CONNECTION %d\n"),value);
#endif
}

#define CDC_ECM_DATA_ENDPOINT_SIZE		64

void cdc_ecm_configure_endpoints() {
#if CDC_ECM_USES_INTERRUPT_ENDPOINT
	usb_configure_endpoint(INT_EP,      \
						 TYPE_INTERRUPT,     \
						 DIRECTION_IN,  \
						 SIZE_8,       \
						 TWO_BANKS,     \
						 NYET_ENABLED);
#endif

	usb_configure_endpoint(TX_EP,      \
						 TYPE_BULK,  \
						 DIRECTION_IN,  \
						 SIZE_64,     \
						 TWO_BANKS,     \
						 NYET_ENABLED);

	usb_configure_endpoint(RX_EP,      \
						 TYPE_BULK,     \
						 DIRECTION_OUT,  \
						 SIZE_64,       \
						 TWO_BANKS,     \
						 NYET_ENABLED);
#if CDC_ECM_USES_INTERRUPT_ENDPOINT
	Usb_reset_endpoint(INT_EP);
#endif
	Usb_reset_endpoint(TX_EP);
	Usb_reset_endpoint(RX_EP);
	usb_eth_is_active = 1;
}


void
cdc_ecm_notify_connection_speed_change(uint32_t upstream,uint32_t downstream) {
#if CDC_ECM_USES_INTERRUPT_ENDPOINT
	Usb_select_endpoint(INT_EP);

	if(!Is_usb_endpoint_enabled())
		return;

	if(usb_endpoint_wait_for_IN_ready()!=0)
		return;

	Usb_send_control_in();

	Usb_write_byte(0x51); // 10100001b
	Usb_write_byte(CDC_NOTIFY_CONNECTION_SPEED_CHANGE);
	Usb_write_word(0x0000);
	Usb_write_word(ECM_INTERFACE0_NB);
	Usb_write_word(0x0008);

	Usb_send_in();

	if(usb_endpoint_wait_for_write_enabled()!=0)
		return;

	Usb_write_long(upstream);
	Usb_write_long(downstream);
	Usb_send_in();

	PRINTF_P(PSTR("cdc_ecm: CDC_NOTIFY_CONNECTION_SPEED_CHANGE UP:%d DOWN:%d\n"),upstream,downstream);
#endif
}

void cdc_ecm_set_active(uint8_t value) {
	if(value!=usb_eth_is_active) {
		Led3_on();

		usb_eth_is_active = value;
		cdc_ecm_notify_network_connection(value);
		if(value) {
			cdc_ecm_notify_connection_speed_change(250000,250000);
		} else {
			cdc_ecm_notify_connection_speed_change(0,0);
		}
	}
}

uint8_t
cdc_ecm_process(void) {
	static uint8_t doInit = 1;
	
	Usb_select_endpoint(RX_EP);

	if(!Is_usb_endpoint_enabled()) {
		return 0;
	}

	if (doInit) {
#ifdef USB_ETH_HOOK_INIT
		USB_ETH_HOOK_INIT();
#endif
		cdc_ecm_notify_network_connection(1);
		cdc_ecm_notify_connection_speed_change(250000,250000);
		doInit = 0;
		if(usb_ecm_packet_filter & PACKET_TYPE_PROMISCUOUS) {
#if RF230BB
			rf230_set_promiscuous_mode(true);
#else		
			radio_set_trx_state(RX_ON);
#endif
		}

		// Select again, just to make sure.
		Usb_select_endpoint(RX_EP);
	}

	if(!usb_eth_is_active) {
		// If we aren't active, just eat the packets.
		if(Is_usb_read_enabled()) {
			Usb_ack_receive_out();
		}
		return 0;
	}

	//Connected!
	Led0_on();

	if(Is_usb_read_enabled()) {
		uint16_t bytecounter;
		uint16_t bytes_received = 0;
		U8 * buffer = uip_buf;

		if(!usb_eth_ready_for_next_packet()) {
			// Since we aren't ready for a packet yet,
			// just return.
			goto bail;
		}

#ifdef USB_ETH_HOOK_RX_START
		USB_ETH_HOOK_RX_START();
#endif

		while((bytecounter=Usb_byte_counter_8())==CDC_ECM_DATA_ENDPOINT_SIZE) {
			while((bytes_received<USB_ETH_MTU) && (bytecounter--)) {
				*buffer++ = Usb_read_byte();
				bytes_received++;
			}
			bytes_received+=bytecounter+1;

			//ACK previous data
			Usb_ack_receive_out();

			//Wait for new data
			if(usb_endpoint_wait_for_read_enabled()!=0) {
				USB_ETH_HOOK_RX_ERROR("Timeout: read enabled");
				goto bail;
			}
		}
		bytecounter = Usb_byte_counter_8();
		while((bytes_received<USB_ETH_MTU) && (bytecounter--)) {
			*buffer++ = Usb_read_byte();
			bytes_received++;
		}
		bytes_received+=bytecounter+1;
		
		//Ack final data packet
		Usb_ack_receive_out();

		//PRINTF_P(PSTR("cdc_ecm: Got packet %d bytes long\n"),bytes_received);

#ifdef USB_ETH_HOOK_RX_END
		USB_ETH_HOOK_RX_END();
#endif
		
		//Send data over RF or to local stack
		if(bytes_received<=USB_ETH_MTU) {

			USB_ETH_HOOK_HANDLE_INBOUND_PACKET(uip_buf,bytes_received);
		} else {
			USB_ETH_HOOK_RX_ERROR("Oversized packet");
		}
	}
bail:
	return 1;
}

uint8_t
ecm_send(uint8_t * senddata, uint16_t sendlen, uint8_t led) {
	U8 byte_in_packet = 0;

	//Send Data
	Usb_select_endpoint(TX_EP);

	if(usb_endpoint_wait_for_write_enabled()!=0) {
		USB_ETH_HOOK_TX_ERROR("Timeout: write enabled");
		return 0;
	}

#ifdef USB_ETH_HOOK_TX_START
	USB_ETH_HOOK_TX_START();
#endif

	//Send packet
	while(sendlen) {
		Usb_write_byte(*senddata);
		senddata++;
		sendlen--;
		byte_in_packet++;
		
		//If endpoint is full, send data in
		//And then wait for data to transfer
		if (!Is_usb_write_enabled()) {
			Usb_ack_in_ready();

			if(usb_endpoint_wait_for_write_enabled()!=0) {
				USB_ETH_HOOK_TX_ERROR("Timeout: write enabled");
				return 0;
			}
			byte_in_packet=0;
		}

	}

	//Send last data in - also handles sending a ZLP if needed
	Usb_ack_in_ready();

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


