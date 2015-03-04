/*
 * Copyright (c) 2015, 3B Scientific GmbH.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \addtogroup cc32xx
 * @{
 *
 * \defgroup cc32xx-wifi cc32xx wireless network wrapper
 *
 * Wrapper for the cc32xx wireless network controller
 * @{
 *
 * \file
 * 		Implementation of the cc32xx wireless network wrapper
 * \author
 *      Björn Rennfanz <bjoern.rennfanz@3bscientific.com>
 */

#include "contiki.h"
#include "contiki-net.h"

#include "sys/log.h"
#include "uip.h"

#include "net/wifi.h"
#include "simplelink.h"

#include <stdio.h>

#if STARTUP_CONF_VERBOSE
#define ERR_PRINT(x)		printf("Error [%d] at line [%d] in function [%s]\n", x, __LINE__, __FUNCTION__)
#define PRINTF(...) 		printf(__VA_ARGS__)
#define ASSERT_ON_ERROR(e)	{ if(e < 0) { ERR_PRINT(e); return; } }
#else
#define ERR_PRINT(x)
#define PRINTF(...)
#define ASSERT_ON_ERROR(e)
#endif

// Enable debug messages
#define DEBUG	1

// Internal structures
struct wifi_arp_hdr {
  uint16_t hwtype;
  uint16_t protocol;
  uint8_t hwlen;
  uint8_t protolen;
  uint16_t opcode;
  struct uip_eth_addr shwaddr;
  uip_ip4addr_t sipaddr;
  struct uip_eth_addr dhwaddr;
  uip_ip4addr_t dipaddr;
};

// Static variables
uint8_t wifi_mac_addr[SL_MAC_ADDR_LEN];
uint8_t wifi_client_mac_addr[SL_MAC_ADDR_LEN];
uint8_t wifi_raw_buffer[UIP_BUFSIZE];

int32_t wifi_socket_handle;

uint32_t wifi_status, wifi_client_ip;
uint32_t wifi_own_ip, wifi_gateway;
uint32_t wifi_netmask;

SlSocklen_t wifi_local_addr_size = sizeof(SlSockAddrIn_t);
SlSockAddrIn_t wifi_local_addr;

/*---------------------------------------------------------------------------*/
void wifi_init(void)
{
	uint8_t wifi_mac_addr_len = SL_MAC_ADDR_LEN;

#if STARTUP_CONF_VERBOSE
	uint8_t configOpt = 0;
	uint8_t configLen = 0;
	SlVersionFull ver;
#endif

	int retVal = -1;
	int mode = -1;

	// Setup local address
	wifi_local_addr.sin_family = SL_AF_INET;
	wifi_local_addr.sin_port = 0;
	wifi_local_addr.sin_addr.s_addr = 0;

	// Start WLAN network processor
	mode = sl_Start(0, 0, 0);
	ASSERT_ON_ERROR(mode);

#if STARTUP_CONF_VERBOSE
		PRINTF("Starting SimpleLink in SoftAP mode\n");
#endif

	// Check if network processor is not in SoftAP mode
	if (ROLE_AP != mode)
	{
		// Set network processor to SoftAP mode
		retVal = sl_WlanSetMode(ROLE_AP);
		ASSERT_ON_ERROR(retVal);

		// Restart network processor
        retVal = sl_Stop(0xFF);
        ASSERT_ON_ERROR(retVal);

        mode = sl_Start(0, 0, 0);
    	if (ROLE_AP != mode)
    	{
#if STARTUP_CONF_VERBOSE
    		PRINTF(" Error: Could not start SimpleLink in SoftAP mode\n");
#endif
    		return;
    	}
	}

	// On this point the device is in AP mode,
	// we need to wait for this event before
	// doing anything
	while(!IS_IP_ACQUIRED(wifi_status))
	{
#if !(defined(USE_FREERTOS) || defined(USE_TIRTOS))
		// Call simple link worker
		_SlNonOsMainLoopTask();
#endif
	}

#if STARTUP_CONF_VERBOSE
	// Get the device's version-information
	configOpt = SL_DEVICE_GENERAL_VERSION;
	configLen = sizeof(ver);

	retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen, (uint8_t *)(&ver));
	ASSERT_ON_ERROR(retVal);

	PRINTF(" Host Driver Version: %s\n",SL_DRIVER_VERSION);
	PRINTF(" Build Version: %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n",
	(int)ver.NwpVersion[0], (int)ver.NwpVersion[1], (int)ver.NwpVersion[2], (int)ver.NwpVersion[3],
	(int)ver.ChipFwAndPhyVersion.FwVersion[0], (int)ver.ChipFwAndPhyVersion.FwVersion[1],
	(int)ver.ChipFwAndPhyVersion.FwVersion[2], (int)ver.ChipFwAndPhyVersion.FwVersion[3],
	(int)ver.ChipFwAndPhyVersion.PhyVersion[0], (int)ver.ChipFwAndPhyVersion.PhyVersion[1],
	(int)ver.ChipFwAndPhyVersion.PhyVersion[2], (int)ver.ChipFwAndPhyVersion.PhyVersion[3]);
#endif

	// Read simplelink MAC address
	retVal = sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &wifi_mac_addr_len, (uint8_t *)&wifi_mac_addr);
	//ASSERT_ON_ERROR(retVal);
#if STARTUP_CONF_VERBOSE
	PRINTF(" MAC address is %02x:%02x:%02x:%02x:%02x:%02x\n\n", wifi_mac_addr[0], wifi_mac_addr[1], wifi_mac_addr[2], wifi_mac_addr[3], wifi_mac_addr[4], wifi_mac_addr[5]);
#endif
}
/*---------------------------------------------------------------------------*/
int wifi_read(uint8_t *buffer, uint16_t bufsize)
{
	int32_t retVal = 0;

#if !(defined(USE_FREERTOS) || defined(USE_TIRTOS))
	// Call simple link worker
	_SlNonOsMainLoopTask();
#endif

	// Check if client is connected and has an IP leased.
	if (IS_CONNECTED(wifi_status) && IS_IP_LEASED(wifi_status))
	{
		// Check if raw socket is not opened
		if (!(IS_RAW_SOCKET_OPEN(wifi_status)))
		{
			int enableHeader = 1;
			SlSockNonblocking_t enableOption;

			// Open RAW socket to bypass cc32xx embedded TCP/IP stack
			wifi_socket_handle = sl_Socket(SL_AF_INET, SL_SOCK_RAW, SL_IPPROTO_TCP);
			sl_SetSockOpt(wifi_socket_handle, SL_IPPROTO_IP, SL_IP_HDRINCL, &enableHeader, sizeof(enableHeader));

			 // Enable nonblocking mode
			enableOption.NonblockingEnabled = 1;
			sl_SetSockOpt(wifi_socket_handle,SL_SOL_SOCKET,SL_SO_NONBLOCKING, (unsigned char *)&enableOption, sizeof(enableOption));

			// Check if handle could be opened
			if (!(wifi_socket_handle < 0))
			{
				// Set raw socket open flag
				SET_STATUS_BIT(wifi_status, STATUS_BIT_RAW_SOCKET_OPEN);

#if STARTUP_CONF_VERBOSE && DEBUG
				PRINTF("simplelink_wifi: RAW socket created! Socket Descriptor: %d \n", (int)wifi_socket_handle);
#endif
			}
			else
			{
#if STARTUP_CONF_VERBOSE && DEBUG
				PRINTF("simplelink_wifi: Failed to create RAW socket\n");
#endif
			}
		}
		else
		{
			struct uip_eth_hdr *packet_eth_hdr = (struct uip_eth_hdr *)buffer;

			// Check if ARP request is pending
			if (IS_ARP_REQUESTED(wifi_status))
			{
				// Prepare both IPs and ARP header
				struct wifi_arp_hdr *packet_arp_hdr = (struct wifi_arp_hdr *)(buffer + UIP_LLH_LEN);
				uint32_t own_ip_uip = uip_htonl(wifi_own_ip);
				uint32_t client_ip_uip = uip_htonl(wifi_client_ip);

				// Add Ethernet header
				packet_eth_hdr->type = uip_htons(UIP_ETHTYPE_ARP);
				memcpy(packet_eth_hdr->dest.addr, wifi_mac_addr, SL_MAC_ADDR_LEN);
				memcpy(packet_eth_hdr->src.addr, wifi_client_mac_addr, SL_MAC_ADDR_LEN);

				// Setup ARP answer
				packet_arp_hdr->hwtype = uip_htons(1); 	// Ethernet
				packet_arp_hdr->protocol = uip_htons(0x8000); // IPv4
				packet_arp_hdr->hwlen = 6; // Ethernet MAC length
				packet_arp_hdr->protolen = 4; // IPv4 address length
				packet_arp_hdr->opcode = uip_htons(2); // ARP answer

				memcpy(&packet_arp_hdr->shwaddr, &wifi_client_mac_addr, SL_MAC_ADDR_LEN);
				uip_ip4addr_copy(&packet_arp_hdr->sipaddr, (uip_ip4addr_t *)&client_ip_uip);

				memcpy(&packet_arp_hdr->dhwaddr, &wifi_mac_addr, SL_MAC_ADDR_LEN);
				uip_ip4addr_copy(&packet_arp_hdr->dipaddr, (uip_ip4addr_t *)&own_ip_uip);

				// Reset ARP pending flag and calculate packet size
				retVal = sizeof(struct uip_eth_hdr) + sizeof(struct wifi_arp_hdr);
				CLR_STATUS_BIT(wifi_status, STATUS_BIT_ARP_REQUESTED);
			}
			else
			{
				uint8_t *packet_payload = buffer + UIP_LLH_LEN;

				// Try to read received IP packets
				retVal = sl_RecvFrom(wifi_socket_handle, wifi_raw_buffer, sizeof(wifi_raw_buffer), 0, (SlSockAddr_t *)&wifi_local_addr, &wifi_local_addr_size);
				if (retVal > 0)
				{
					// Add Ethernet header
					packet_eth_hdr->type = uip_htons(UIP_ETHTYPE_IP);
					memcpy(packet_eth_hdr->dest.addr, wifi_mac_addr, SL_MAC_ADDR_LEN);
					memcpy(packet_eth_hdr->src.addr, wifi_client_mac_addr, SL_MAC_ADDR_LEN);

					// Copy IP Packet
					memcpy(packet_payload, wifi_raw_buffer, retVal);

					// Increase packet size
					retVal += UIP_LLH_LEN;
				}
			}
		}
	}
	else
	{
		// Check if raw socket is opened
		if (IS_RAW_SOCKET_OPEN(wifi_status))
		{
			// Clear raw socket open flag
			CLR_STATUS_BIT(wifi_status, STATUS_BIT_RAW_SOCKET_OPEN);

			// Close RAW socket
			sl_Close(wifi_socket_handle);

#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF("simplelink_wifi: Close RAW socket\n");
#endif
		}
	}

	return retVal;
}
/*---------------------------------------------------------------------------*/
int wifi_send(uint8_t *data, uint16_t datalen)
{
	// Return on no active connection
	if (!(IS_RAW_SOCKET_OPEN(wifi_status)))
	{
		return -1;
	}

	// Get Ethernet header from packet and check for ARP request.
	struct uip_eth_hdr *packet_eth_hdr = (struct uip_eth_hdr *)data;
	if (packet_eth_hdr->type == uip_htons(UIP_ETHTYPE_ARP))
	{
		// Get ARP Header from packet
		struct wifi_arp_hdr *packet_arp_hdr = (struct wifi_arp_hdr *)(data + UIP_LLH_LEN);

		// Prepare client IP for comparing
		uint32_t client_ip_uip = uip_htonl(wifi_client_ip);

		// Check if ARP request is for client IP
		if ((packet_arp_hdr->opcode == uip_htons(1)) && (uip_ip4addr_cmp((uip_ip4addr_t *)&client_ip_uip, &packet_arp_hdr->dipaddr)))
		{
#if STARTUP_CONF_VERBOSE && DEBUG
			// Printout requested IP address
			PRINTF("simplelink_wifi: ARP request for IP %u.%u.%u.%u received\n", packet_arp_hdr->dipaddr.u8[0], packet_arp_hdr->dipaddr.u8[1], packet_arp_hdr->dipaddr.u8[2], packet_arp_hdr->dipaddr.u8[3]);
#endif

			// Set status flag that an ARP was requested
			SET_STATUS_BIT(wifi_status, STATUS_BIT_ARP_REQUESTED);
		}
	}
	else
	{
		// Check if packet is for connected client
		if (memcmp(&packet_eth_hdr->dest.addr, &wifi_client_mac_addr, SL_MAC_ADDR_LEN) == 0)
		{
			// Copy IP packet to internal buffer
			memcpy(&wifi_raw_buffer[0], (data + UIP_LLH_LEN), (datalen - UIP_LLH_LEN));

			// Send IP packet out over wireless network
			sl_SendTo(wifi_socket_handle, &wifi_raw_buffer, (datalen - UIP_LLH_LEN), 0, (SlSockAddr_t *)&wifi_local_addr, wifi_local_addr_size);
		}
	}

	return datalen;
}
/*---------------------------------------------------------------------------*/
void wifi_exit(void)
{
	// Stop WLAN network processor
	sl_Stop(0xFF);
}
/*---------------------------------------------------------------------------*/
void sl_WlanEvtHdlr(SlWlanEvent_t *pSlWlanEvent)
{
	switch(pSlWlanEvent->Event)
	{
		case SL_WLAN_CONNECT_EVENT:
		{
			SET_STATUS_BIT(wifi_status, STATUS_BIT_CONNECTION);
#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF("simplelink_wifi: WlanEvent -> Device connected to the AP.\n");
#endif
		}
		break;

		case SL_WLAN_DISCONNECT_EVENT:
		{
			CLR_STATUS_BIT(wifi_status, STATUS_BIT_CONNECTION);
			CLR_STATUS_BIT(wifi_status, STATUS_BIT_IP_AQUIRED);

#if STARTUP_CONF_VERBOSE && DEBUG
			slWlanConnectAsyncResponse_t*  pEventData = NULL;
			pEventData = &pSlWlanEvent->EventData.STAandP2PModeDisconnected;

			// If the user has initiated 'Disconnect' request,
			//'reason_code' is SL_USER_INITIATED_DISCONNECTION
			if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
			{
				PRINTF("simplelink_wifi: WlanEvent -> Device disconnected from the AP on application's request.\n");
			}
			else
			{
				PRINTF("simplelink_wifi: WlanEvent -> Device disconnected from the AP on an ERROR!\n");
			}
#endif
		}
		break;

		case SL_WLAN_STA_CONNECTED_EVENT:
		{
			// when device is in AP mode and any client connects to device cc3xxx
			SET_STATUS_BIT(wifi_status, STATUS_BIT_CONNECTION);

			// Copy MAC address of client
			memcpy(wifi_client_mac_addr, pSlWlanEvent->EventData.APModeStaConnected.mac, SL_MAC_ADDR_LEN);

#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF("simplelink_wifi: WlanEvent -> Client connected to the AP, MAC address is %02x:%02x:%02x:%02x:%02x:%02x\n", wifi_client_mac_addr[0], wifi_client_mac_addr[1], wifi_client_mac_addr[2], wifi_client_mac_addr[3], wifi_client_mac_addr[4], wifi_client_mac_addr[5]);
#endif
		}
		break;

		case SL_WLAN_STA_DISCONNECTED_EVENT:
		{
			// when client disconnects from device (AP)
			CLR_STATUS_BIT(wifi_status, STATUS_BIT_CONNECTION);
			CLR_STATUS_BIT(wifi_status, STATUS_BIT_IP_LEASED);

#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF("simplelink_wifi: WlanEvent -> Client disconnects from the AP.\n");
#endif
		}
		break;

		default:
		{
#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF("simplelink_wifi: WlanEvent -> Unexpected event [0x%x]\n", (unsigned int)pSlWlanEvent->Event);
#endif
		}
		break;
	}
}
/*---------------------------------------------------------------------------*/
void sl_NetAppEvtHdlr(SlNetAppEvent_t *pSlNetApp)
{
    switch(pSlNetApp->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        case SL_NETAPP_IPV6_IPACQUIRED_EVENT:
        {
            SET_STATUS_BIT(wifi_status, STATUS_BIT_IP_AQUIRED);

            // IP Acquired Event Data
            wifi_own_ip = pSlNetApp->EventData.ipAcquiredV4.ip;
            wifi_netmask = SL_IPV4_VAL(255, 255, 255, 0);

            // Gateway IP address
            wifi_gateway = pSlNetApp->EventData.ipAcquiredV4.gateway;

#if STARTUP_CONF_VERBOSE
			PRINTF(" Own IP Acquired, IP = %d.%d.%d.%d, Gateway = %d.%d.%d.%d\n",
					(int)SL_IPV4_BYTE(wifi_own_ip, 3), (int)SL_IPV4_BYTE(wifi_own_ip, 2),
					(int)SL_IPV4_BYTE(wifi_own_ip, 1), (int)SL_IPV4_BYTE(wifi_own_ip, 0),
					(int)SL_IPV4_BYTE(wifi_gateway, 3), (int)SL_IPV4_BYTE(wifi_gateway, 2),
					(int)SL_IPV4_BYTE(wifi_gateway, 1), (int)SL_IPV4_BYTE(wifi_gateway, 0));
#endif
        }
        break;

        case SL_NETAPP_IP_LEASED_EVENT:
        {
            SET_STATUS_BIT(wifi_status, STATUS_BIT_IP_LEASED);
            wifi_client_ip = pSlNetApp->EventData.ipLeased.ip_address;

#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF("simplelink_wifi: NetAppEvent -> IP Leased to Client with IP = %d.%d.%d.%d\n",
					(int)SL_IPV4_BYTE(wifi_client_ip, 3), (int)SL_IPV4_BYTE(wifi_client_ip, 2),
					(int)SL_IPV4_BYTE(wifi_client_ip, 1), (int)SL_IPV4_BYTE(wifi_client_ip, 0));
#endif
        }
        break;

        case SL_NETAPP_IP_RELEASED_EVENT:
        {
            CLR_STATUS_BIT(wifi_status, STATUS_BIT_IP_LEASED);

#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF("simplelink_wifi: NetAppEvent -> IP Released from Client with IP = %d.%d.%d.%d\n",
					(int)SL_IPV4_BYTE(wifi_client_ip,3), (int)SL_IPV4_BYTE(wifi_client_ip,2),
					(int)SL_IPV4_BYTE(wifi_client_ip,1), (int)SL_IPV4_BYTE(wifi_client_ip,0));
#endif
        }
        break;

        default:
        {
#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF("simplelink_wifi: NetAppEvent -> Unexpected event [0x%x]\n", (unsigned int)pSlNetApp->Event);
#endif
        }
        break;
    }
}
/*---------------------------------------------------------------------------*/
void sl_SockEvtHdlr(SlSockEvent_t *pSlSockEvent)
{
#if STARTUP_CONF_VERBOSE && DEBUG
	PRINTF("simplelink_wifi: SockEvent -> Unexpected event [0x%x]\n", (unsigned int)pSlSockEvent->Event);
#endif
}
/*---------------------------------------------------------------------------*/
void sl_HttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent, SlHttpServerResponse_t *pSlHttpServerResponse)
{
}
