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
 * Implementation of the cc32xx wireless network wrapper
 */

#include "contiki-net.h"
#include "sys/log.h"
#include "net/wifi-drv.h"

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

// Static variables
unsigned char wifi_mac_addr[SL_MAC_ADDR_LEN + 2];
unsigned long wifi_status, wifi_client_ip;
unsigned long wifi_own_ip, wifi_gateway;

/*---------------------------------------------------------------------------*/
void wifi_init(void)
{
	unsigned char wifi_mac_addr_len = SL_MAC_ADDR_LEN + 2;

#if STARTUP_CONF_VERBOSE
	unsigned char configOpt = 0;
	unsigned char configLen = 0;
	SlVersionFull ver = {0};
#endif

	int retVal = -1;
	int mode = -1;

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
		// Call simple link worker
		_SlNonOsMainLoopTask();
	}

#if STARTUP_CONF_VERBOSE
	// Get the device's version-information
	configOpt = SL_DEVICE_GENERAL_VERSION;
	configLen = sizeof(ver);

	retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen, (unsigned char *)(&ver));
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
	retVal = sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &wifi_mac_addr_len, (unsigned char *)&wifi_mac_addr);
	ASSERT_ON_ERROR(retVal);
#if STARTUP_CONF_VERBOSE
	PRINTF(" MAC address: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n\n", wifi_mac_addr[0], wifi_mac_addr[1], wifi_mac_addr[2], wifi_mac_addr[3], wifi_mac_addr[4], wifi_mac_addr[5], wifi_mac_addr[6], wifi_mac_addr[7]);
#endif
}
/*---------------------------------------------------------------------------*/
uint16_t wifi_poll(void)
{
	// Call simple link worker
	_SlNonOsMainLoopTask();

	// Check if client is connected and has an IP leased.
	if (IS_CONNECTED(wifi_status) && IS_IP_LEASED(wifi_status))
	{
		// Check if raw socket is not opened
		if (!(IS_RAW_SOCKET_OPEN(wifi_status)))
		{
			// Set raw socket open flag
			SET_STATUS_BIT(wifi_status, STATUS_BIT_RAW_SOCKET_OPEN);

#if STARTUP_CONF_VERBOSE
			PRINTF(" SimpleLink Open RAW Socket\n");
#endif
		}
	}
	else
	{
		// Check if raw socket is opened
		if (IS_RAW_SOCKET_OPEN(wifi_status))
		{
			// Clear raw socket open flag
			CLR_STATUS_BIT(wifi_status, STATUS_BIT_RAW_SOCKET_OPEN);

#if STARTUP_CONF_VERBOSE
			PRINTF(" SimpleLink Close RAW Socket\n");
#endif
		}
	}

	// TODO: Implement non OS behavior
	return 0;
}
/*---------------------------------------------------------------------------*/
void wifi_send(void)
{

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
			PRINTF(" SimpleLink WlanEvent: Device connected to the AP.\n");
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
				PRINTF(" SimpleLink WlanEvent: Device disconnected from the AP on application's request.\n");
			}
			else
			{
				PRINTF(" SimpleLink WlanEvent: Device disconnected from the AP on an ERROR!\n");
			}
#endif
		}
		break;

		case SL_WLAN_STA_CONNECTED_EVENT:
		{
			// when device is in AP mode and any client connects to device cc3xxx
			SET_STATUS_BIT(wifi_status, STATUS_BIT_CONNECTION);

#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF(" SimpleLink WlanEvent: Client connected to the AP.\n");
#endif
		}
		break;

		case SL_WLAN_STA_DISCONNECTED_EVENT:
		{
			// when client disconnects from device (AP)
			CLR_STATUS_BIT(wifi_status, STATUS_BIT_CONNECTION);
			CLR_STATUS_BIT(wifi_status, STATUS_BIT_IP_LEASED);

#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF(" SimpleLink WlanEvent: Client disconnects from the AP.\n");
#endif
		}
		break;

		default:
		{
#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF(" SimpleLink WlanEvent: Unexpected event [0x%x]\n", pSlWlanEvent->Event);
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

            // Gateway IP address
            wifi_gateway = pSlNetApp->EventData.ipAcquiredV4.gateway;

#if STARTUP_CONF_VERBOSE
			PRINTF(" Own IP Acquired, IP = %d.%d.%d.%d, Gateway = %d.%d.%d.%d\n",
					SL_IPV4_BYTE(wifi_own_ip, 3), SL_IPV4_BYTE(wifi_own_ip, 2),
					SL_IPV4_BYTE(wifi_own_ip, 1), SL_IPV4_BYTE(wifi_own_ip, 0),
					SL_IPV4_BYTE(wifi_gateway, 3), SL_IPV4_BYTE(wifi_gateway, 2),
					SL_IPV4_BYTE(wifi_gateway, 1), SL_IPV4_BYTE(wifi_gateway, 0));
#endif
        }
        break;

        case SL_NETAPP_IP_LEASED_EVENT:
        {
            SET_STATUS_BIT(wifi_status, STATUS_BIT_IP_LEASED);
            wifi_client_ip = pSlNetApp->EventData.ipLeased.ip_address;

#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF(" SimpleLink NetAppEvent: IP Leased to Client with IP = %d.%d.%d.%d\n",
					SL_IPV4_BYTE(wifi_client_ip, 3), SL_IPV4_BYTE(wifi_client_ip, 2),
					SL_IPV4_BYTE(wifi_client_ip, 1), SL_IPV4_BYTE(wifi_client_ip, 0));
#endif
        }
        break;

        case SL_NETAPP_IP_RELEASED_EVENT:
        {
            CLR_STATUS_BIT(wifi_status, STATUS_BIT_IP_LEASED);

#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF(" SimpleLink NetAppEvent: IP Released from Client with IP = %d.%d.%d.%d\n",
					SL_IPV4_BYTE(wifi_client_ip,3), SL_IPV4_BYTE(wifi_client_ip,2),
					SL_IPV4_BYTE(wifi_client_ip,1), SL_IPV4_BYTE(wifi_client_ip,0));
#endif
        }
        break;

        default:
        {
#if STARTUP_CONF_VERBOSE && DEBUG
			PRINTF(" SimpleLink NetAppEvent: Unexpected event [0x%x]\n", pSlNetApp->Event);
#endif
        }
        break;
    }
}
/*---------------------------------------------------------------------------*/
void sl_SockEvtHdlr(SlSockEvent_t *pSlSockEvent)
{

}
/*---------------------------------------------------------------------------*/
void sl_HttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent, SlHttpServerResponse_t *pSlHttpServerResponse)
{
}
