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

// Static variables
unsigned char wifi_mac_addr[SL_MAC_ADDR_LEN + 2];
unsigned long wifi_status;

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
void sl_GeneralEvtHdlr(SlDeviceEvent_t *pSlDeviceEvent)
{

}
/*---------------------------------------------------------------------------*/
void sl_WlanEvtHdlr(SlWlanEvent_t *pSlWlanEvent)
{
	switch(pSlWlanEvent->Event)
	{

	}
}
/*---------------------------------------------------------------------------*/
void sl_NetAppEvtHdlr(SlNetAppEvent_t *pSlNetApp)
{

}
/*---------------------------------------------------------------------------*/
void sl_SockEvtHdlr(SlSockEvent_t *pSlSockEvent)
{

}
/*---------------------------------------------------------------------------*/
void sl_HttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent, SlHttpServerResponse_t *pSlHttpServerResponse)
{

}
