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


#include "port.h"
#include <stdio.h>

int port_open(port_t **port, int device)
{
	port_t *new_port = (port_t *) malloc(sizeof(port_t));
	FT_STATUS status;
	
	*port = new_port;
	
	new_port->device = device;
	new_port->handle = 0;

	status = FT_Open(device, &(new_port->handle));
	
	if (status != FT_OK)
	{
		new_port->handle = 0;
		
		printf("Serial port open failed with error message: %d.\n", (int)status);
		return(-1);
	}
	else
	{
		DWORD mask;
		new_port->event_handle = CreateEvent(NULL, TRUE, FALSE, "SN_USB_UART_EVENTS");
		if (new_port->event_handle == NULL)
		{
			printf("Event handle creation failed.\n");
			FT_Close(new_port->handle);
			new_port->handle = 0;
			return(-1);
		}
		mask = FT_EVENT_RXCHAR | FT_EVENT_MODEM_STATUS;
		status = FT_SetEventNotification(new_port->handle,mask,new_port->event_handle);
		if (status != FT_OK)
		{
			printf("Setting event notification failed.\n");
		}
		FT_SetTimeouts(new_port->handle,400,0);
		return(0);
	}	
}

int port_close(port_t *port)
{
	if (!port)
		return(-1);
	
	if ((port->event_handle) != NULL)
	{
		CloseHandle(port->event_handle);
		port->event_handle = NULL;
		
		FT_Purge(port->handle, FT_PURGE_RX | FT_PURGE_TX);
		FT_Close(port->handle);
		port->handle = 0;
	}

	port->device = 0;
	free(port);
	
	return(1);
}

int port_set_params(port_t *port, uint32_t speed, uint8_t rtscts)
{
	FT_STATUS status;
	
	if (!port) return -1;
	status = FT_SetBaudRate (port->handle, speed);
	if (status != FT_OK) return -1;
	if (speed == 9600)
	{	
		status = FT_SetDataCharacteristics(port->handle, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
	}
	else
	{
		status = FT_SetDataCharacteristics(port->handle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
	}
	
	if (status != FT_OK) return -1;
	
	if (rtscts)
	{
		status = FT_SetFlowControl(port->handle, FT_FLOW_RTS_CTS, 0, 0);
	}
	else
	{
		status = FT_SetFlowControl(port->handle, FT_FLOW_NONE, 0, 0);
	}
	
	if (status != FT_OK) return -1;
	
	FT_Purge(port->handle, FT_PURGE_RX | FT_PURGE_TX);

	return(0);
}

int port_dtr_set(port_t *port)
{
	FT_STATUS status;
	
	if (!port) return(-1);
	
	status = FT_SetDtr(port->handle);
	if (status != FT_OK)
	{
		printf("Control failed.\n");
		return -1;
	}
	return 0;
}

int port_dtr_clear(port_t *port)
{
	FT_STATUS status;
	
	if (!port) return(-1);
	
	status = FT_ClrDtr(port->handle);
	if (status != FT_OK)
	{
		printf("Control failed.\n");
		return -1;
	}
	return 0;
}

int port_rts_set(port_t *port)
{
	FT_STATUS status;
	
	if (!port) return(-1);
	
	status = FT_SetRts(port->handle);
	if (status != FT_OK)
	{
		printf("Control failed.\n");
		return -1;
	}
	return 0;
}

int port_rts_clear(port_t *port)
{
	FT_STATUS status;
	
	if (!port) return(-1);
	
	status = FT_ClrRts(port->handle);
	if (status != FT_OK)
	{
		printf("Control failed.\n");
		return -1;
	}
	return 0;
}

int port_write(port_t *port, unsigned char *buffer, size_t buflen)
{
	FT_STATUS status;
	DWORD bytes_out;
	
	if (!port) return -1;
	
//	FT_Purge(port->handle, FT_PURGE_RX);
	
	status = FT_Write (port->handle, (LPVOID) buffer, (DWORD) buflen,&bytes_out);
	if (status != FT_OK) return -1;
	
	return 0;
}

int port_read(port_t *port, unsigned char *buffer, size_t buflen)
{
	DWORD l = 0;
	FT_STATUS status;
	status = FT_Read(port->handle, buffer, buflen, &l);
	return((int) l);
}

int port_get(port_t *port, unsigned char *buffer, int timeout)
{
	DWORD bytes = 0;
	FT_STATUS status;

	FT_SetTimeouts(port->handle, timeout, 0);
	
	status = FT_Read(port->handle, buffer, 1, &bytes);
	if (status != FT_OK)
	{
		return(-1);
	}
	return bytes;
}

int port_readline(port_t *port, unsigned char *buffer, int buf_size, int timeout)
{
	int length = 0;
	DWORD bytes = 0;
	FT_STATUS status;

	FT_SetTimeouts(port->handle, timeout, 0);
	
	do
	{
		status = FT_Read(port->handle, &buffer[length], 1, &bytes);
		length += bytes;
		if ((status != FT_OK) || (bytes == 0))
		{
			return(length);
		}
		else
		{
			if (buffer[length-1] == '\n')
			{ 
				buf_size = length;
			}
		}
	}while(length < buf_size);
	
	buffer[length] = 0;
	
	if(length != 0)
		return length;
	else
		return(-1);
}

int port_write_echo(port_t *port, char *string)
{
	int length = 0;
	int retry = 0;
	unsigned char byte;
	DWORD bytes_out;
	FT_STATUS status;
	
	FT_Purge(port->handle, FT_PURGE_RX | FT_PURGE_TX);
	
	while( (string[length]) && (retry < 100) )
	{
		retry = 0;
		while (retry++ < 100) 
		{
			status = FT_Write (port->handle, (LPVOID) &string[length], (DWORD) 1,&bytes_out);
			if (status != FT_OK) return -1;
			if (port_get(port, &byte, 1000) == 1)
			{
/*				printf("%c",byte);*/
				if (byte == string[length])
				{
					retry = 0;
					length++;
					break;
				}
				else retry = 100;
			}
			else usleep(500);
		}
	}
	if ((string[strlen(string)-1] == '\r') && (retry < 100) )
	{ /*wait for \n*/
		retry = 0;
		while (retry++ < 100) 
		{
			if (port_get(port, &byte, 1000) == 1)
			{
/*				printf("%c",byte);*/
				break;
			}
			else	usleep(500);
		}
	}
	
	if (retry >= 100) return 0;
	else return length;
}

#if 0

int port_write_echo(port_t *port, char *string)
{
	int length = 0;
	int retry = 0;
	unsigned char byte;
	
	while( (string[length]) && (retry < 100) )
	{
		port_write(port, (unsigned char *) &string[length], 1);
		while (retry++ < 100) 
		{
			if (port_read(port, &byte, 1) == 1)
			{
/*				printf("%c",byte);*/
				if (byte == string[length])
				{
					retry = 0;
					length++;
					break;
				}
				else retry = 100;
			}
			else	usleep(1000);
		}
	}
	if ((string[strlen(string)-1] == '\r') && (retry < 100) )
	{ /*wait for \n*/
		retry = 0;
		while (retry++ < 100) 
		{
			if (port_read(port, &byte, 1) == 1)
			{
/*				printf("%c",byte);*/
				break;
			}
			else	usleep(1000);
		}
	}
	
	if (retry >= 100) return 0;
	else return length;
}


int port_write_8byte_no_echo(port_t *port, int dlen, char *string)
{
	int length = 0;
	int total_len;
	int wrbytes = 4;
	
	total_len = dlen;

/*	printf("total: %d, length: %d, dlen: %d.\n", total_len, length, dlen);	*/
	while(total_len > length)
	{
		if((total_len - length) >= wrbytes)
		{
			port_write(port, (unsigned char *)&string[length], wrbytes);
			length +=  wrbytes;
		}
		else
		{
			port_write(port, (unsigned char *)&string[length], total_len - length);
			length +=  total_len - length;
		}
		usleep(1250);
	
	}

	return(length);
}

#endif
