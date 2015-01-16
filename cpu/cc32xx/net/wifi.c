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
#define ERR_PRINT(x)	printf("Error [%d] at line [%d] in function [%s]\n", x, __LINE__, __FUNCTION__)
#define PRINTF(...) 	printf(__VA_ARGS__)
#else
#define ERR_PRINT(x)
#define PRINTF(...)
#endif

void wifi_init(void)
{
	SlVersionFull ver = {0};
	int retVal = -1;
	int mode = -1;

	unsigned char configOpt = 0;
	unsigned char configLen = 0;

	retVal = sl_Start(0, 0, 0);

	// Get the device's version-information
	configOpt = SL_DEVICE_GENERAL_VERSION;
	configLen = sizeof(ver);

	retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen, (unsigned char *)(&ver));
	if (retVal < 0)
	{
		ERR_PRINT(retVal);
		abort();
	}

#if STARTUP_CONF_VERBOSE
	PRINTF("Host Driver Version: %s\n",SL_DRIVER_VERSION);
	PRINTF("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n",
	ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
	ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
	ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
	ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
	ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);
#endif
}

uint16_t wifi_poll(void)
{
	// TODO: Implement non OS behavior
	return 0;
}

void wifi_send(void)
{

}

void wifi_exit(void)
{
	sl_Stop(0xFF);
}

void sl_GeneralEvtHdlr(SlDeviceEvent_t *pSlDeviceEvent)
{

}

void sl_WlanEvtHdlr(SlWlanEvent_t *pSlWlanEvent)
{

}

void sl_NetAppEvtHdlr(SlNetAppEvent_t *pSlNetApp)
{

}

void sl_SockEvtHdlr(SlSockEvent_t *pSlSockEvent)
{

}

void sl_HttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent, SlHttpServerResponse_t *pSlHttpServerResponse)
{

}
