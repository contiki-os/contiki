/*
    NanoStack: MCU software and PC tools for IP-based wireless sensor networking.
		
    Copyright (C) 2006-2007 Sensinode Ltd.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

		Address:
		Sensinode Ltd.
		Teknologiantie 6	
		90570 Oulu, Finland

		E-mail:
		info@sensinode.com
*/


#include <stdio.h>
#include <inttypes.h>
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif
#include <ftd2xx.h>

FT_HANDLE curr_handle = 0;
uint8_t curr_mode = 0; /* 0 = in, 1=out */

/*
 *	This function checks if the CBUS2 ise set to SLEEP and modifies it if necessary.
 *	The CBUS2 must be set to sleep in order the programming to succeed.
 */
int check_cbus2(FT_HANDLE fthandle)
{
	FT_PROGRAM_DATA eeprom_data;
	char manufacturer_buf[32];
	char manufacturer_id[16];
	char description_buf[64];
	char serialnumber_buf[16];
	eeprom_data.Signature1 = 0x00000000;		// This is a given value from the FT232R programming guide
	eeprom_data.Signature2 = 0xffffffff;		// This is a given value from the FT232R programming guide
	eeprom_data.Version = 0x00000002;				// This is a given value from the FT232R programming guide
	eeprom_data.Manufacturer = manufacturer_buf;
	eeprom_data.ManufacturerId = manufacturer_id;
	eeprom_data.Description = description_buf;
	eeprom_data.SerialNumber = serialnumber_buf;


	if(FT_EE_Read(fthandle, &eeprom_data) != FT_OK)
	{
		printf("FTDI EEPROM read failed.\n");
		return(-1);
	}

	if(eeprom_data.Cbus2 != 0x05)
	{
		printf("Need to re-program the CBUS2 to 0x05\n");
		eeprom_data.Cbus2 = 0x05;
	
		if(FT_EE_Program(fthandle, &eeprom_data) != FT_OK)
		{
			printf("FTDI EEPROM program error.\n");
			return(-1);
		}
		else
		{
			printf("FTDI EEPROM program ok\n");
			return(1);
		}
	}
	else
	{
		return(1);
	}
	
	return(1);
}


void prog_scan(void)
{
	FT_STATUS	ftStatus;
	FT_DEVICE_LIST_INFO_NODE *info;
	unsigned long n_devices = 0;
	uint8_t out;

	ftStatus = FT_CreateDeviceInfoList(&n_devices);
	if (ftStatus == FT_OK) 
	{
		FT_DEVICE_LIST_INFO_NODE devices[n_devices];

		ftStatus = FT_GetDeviceInfoList(devices,&n_devices);
		if (ftStatus == FT_OK) 
		{
    	for (out = 0; out < n_devices; out++) 
			{
				printf("Dev %d:",n_devices - out -1);
				printf(" Type=0x%x",devices[n_devices - out -1].Type);
				printf(" SerialNumber=%s",devices[n_devices - out -1].SerialNumber);
				printf(" Description=%s\n",devices[n_devices - out -1].Description);
    	}
		}
		else
		{
			printf("Failed to fetch device list.\n");
		}
	}
	else
	{
		printf("Failed to fetch device list.\n");
	}
}

FT_HANDLE prog_open(int iport)
{
	FT_HANDLE ftHandle;
	FT_STATUS	ftStatus;
	FT_DEVICE_LIST_INFO_NODE *info;
	unsigned long n_devices = 0;
	uint8_t out;

	if (curr_handle) return 0;
	
	ftStatus = FT_CreateDeviceInfoList(&n_devices);
	if (ftStatus == FT_OK) 
	{
		FT_DEVICE_LIST_INFO_NODE devices[n_devices];

		ftStatus = FT_GetDeviceInfoList(devices,&n_devices);
		if (ftStatus == FT_OK) 
		{
			iport = n_devices - iport - 1;

			if (iport < 0)
			{
				printf("Invalid port id.\n");
    		for (out = 0; out < n_devices; out++) 
				{
					printf("Dev %d:",n_devices - out -1);
					printf(" Type=0x%x",devices[n_devices - out -1].Type);
					printf(" SerialNumber=%s",devices[n_devices - out -1].SerialNumber);
					printf(" Description=%s\n",devices[n_devices - out -1].Description);
    		}
				return 0;
			}
		}
	}
	
	ftStatus = FT_Open(iport, &ftHandle);
	if(ftStatus != FT_OK) {
		/* 
			This can fail if the ftdi_sio driver is loaded
		 	use lsmod to check this and rmmod ftdi_sio to remove
			also rmmod usbserial
		 */
		printf("FT_Open(%d) failed\n", iport);
		return 0;
	}
	
	if(check_cbus2(ftHandle) < 0)
	{
		printf("Nano USB Programmer exiting...\n");
		return(0);
	}

	FT_ResetDevice(ftHandle);
	
	FT_SetBaudRate(ftHandle, 115200);
	
	FT_SetUSBParameters(ftHandle, 64, 0);

	out = 0x04;
	
	ftStatus = FT_SetBitMode(ftHandle, out, 0x20);
	if (ftStatus == FT_OK)
	{
		ftStatus = FT_SetLatencyTimer(ftHandle, 2);
	}
	if (ftStatus == FT_OK)
	{
		DWORD bytes;
		out = 0xE0;
		ftStatus = FT_SetBitMode(ftHandle, out, 0x04);
		out = 0x80;
		FT_Write(ftHandle, &out, 1, &bytes);	/*write reset high*/
		FT_Read(ftHandle, &out, 1, &bytes);		/*read out*/
		curr_mode = 1;
	}
	if (ftStatus != FT_OK) 
	{
		printf("Failed to set CBUS2/bit bang mode.\n");
		
		FT_ResetDevice(ftHandle);
		sleep(3);
		FT_Close(ftHandle);
		ftHandle = 0;
	}
	curr_handle = ftHandle;
	return ftHandle;
}

void prog_close(void)
{
	FT_STATUS	ftStatus;
	DWORD bytes;
	
	if (curr_handle)
	{
		FT_HANDLE ftHandle = curr_handle;
		uint8_t out = 0x00;

		FT_Write(ftHandle, &out, 1, &bytes);	/*write reset low*/
		FT_Read(ftHandle, &out, 1, &bytes);		/*read out*/
		sleep(1);
		out = 0x80;
		FT_Write(ftHandle, &out, 1, &bytes);	/*write reset high*/
		FT_Read(ftHandle, &out, 1, &bytes);		/*read out*/
		sleep(1);
		out = 0x00;
		ftStatus = FT_SetBitMode(ftHandle, out, 0x04);
		FT_ResetDevice(ftHandle);
		FT_Close(ftHandle);

		ftHandle = 0;
		curr_handle = ftHandle;
	}
}

int prog_write(uint8_t byte)
{
	FT_STATUS	ftStatus;
	uint8_t out[16];
	uint8_t mask = 0x80;
	int i;
	DWORD bytes;

	if (curr_mode == 0)
	{	
		out[0] = 0xE0;
		ftStatus = FT_SetBitMode(curr_handle, out[0], 0x04);	/*Set DD as output*/	
		if (ftStatus != FT_OK) 
		{	printf("!WR");
			fflush(stdout);
			return -1;
		}
		curr_mode = 1;
	}
	i = 0;
	while (mask)
	{
		out[i] = 0xC0;	/*clock high, reset high*/
		if (byte & mask) out[i] |= 0x20;
		i++;
		out[i] = 0x80;	/*clock low, reset high*/
		if (byte & mask) out[i] |= 0x20;
		i++;
		mask >>= 1;
	}
	ftStatus = FT_Write(curr_handle, out, 16, &bytes);	/*write clock high and data bit*/
	if (ftStatus != FT_OK)
	{
		printf("!W");
		fflush(stdout);
		return -1;
	}
	
	if(FT_Read(curr_handle, out, 16, &bytes) != FT_OK)
	{
		printf("!R");
		return(-1);
	}
	
	return 0;
}

int prog_read(uint8_t *byte)
{
	FT_STATUS	ftStatus;
	uint8_t rd;
	uint8_t mask = 0x80;
	DWORD bytes;
	uint8_t out[17];
	uint8_t i=0;

	*byte = 0;
	if (curr_mode == 1)
	{
		out[0] = 0xC0;
		ftStatus = FT_SetBitMode(curr_handle, out[0], 0x04);	/*Set DD as input*/
		if (ftStatus != FT_OK)
		{	printf("!RD");
			fflush(stdout);
			return -1;
		}
		curr_mode = 0;
	}
	
	while (mask)
	{
		out[i] = 0xC0;	/*clock high, reset high*/
		if (*byte & mask) out[i] |= 0x20;
		i++;
		out[i] = 0x80;	/*clock low, reset high*/
		if (*byte & mask) out[i] |= 0x20;
		i++;
		mask >>= 1;
	}
	
	out[16] = out[15];
	
	ftStatus = FT_Write(curr_handle, out, 17, &bytes);	/*write clock high and data bit*/

	if(FT_Read(curr_handle, out, 17, &bytes) != FT_OK)
	{
		printf("!R");
		return(-1);
	}

	mask = 0x80;
	i = 1;
	while (mask)
	{
		if (out[i] & 0x20) *byte |= mask;
		mask >>= 1;
		i+=2;
	}
	
	return 0;
}

int prog_start(void)
{
	FT_STATUS	ftStatus;
	uint8_t wr;
	uint8_t mask = 0x80;
	DWORD bytes;
	uint8_t out[16] = { 0x80, 0x00, 0x40, 0x00, 0x40, 0x00, 0x80 };

	printf("prog_start()\n");

	if (curr_mode == 0)
	{	
		wr = 0xE0;
	
		ftStatus = FT_SetBitMode(curr_handle, wr, 0x04);	/*Set DD as output*/	
		curr_mode = 1;
	}

	if(FT_Write(curr_handle, out, 7, &bytes) != FT_OK)
	{
		printf("!W\n");
		return(-1);
	}
	
	if(FT_Read(curr_handle, out, 7, &bytes) != FT_OK)
	{
		printf("!R\n");
		return(-1);
	}
	
}
