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


#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>

#include <inttypes.h>

int hexfile_parse(char *line, unsigned int *type, unsigned int *addr, unsigned char *buffer)
{
	unsigned int row_len = 0;
	unsigned int row_index = 7;
	unsigned int i;
	int tmp;
	
	uint8_t cksum = 0;
	int retval = 0;
	
	retval = sscanf(line, ":%2x%4x%2x", &row_len, addr, type);
	
	cksum += row_len;
	cksum += *addr >> 8;
	cksum += *addr & 0xFF;
	cksum += *type;
	
	i = 0;
	if (retval == 3)
	{
		while(i < row_len)
		{
			
			if (sscanf(&line[row_index], "%2x", &tmp) == 1)
			{
				cksum += tmp;
				buffer[i++] = (unsigned char) tmp;
				row_index += 2;
			}
			else return -1;
		}
		if (sscanf(&line[row_index], "%2x", &tmp) == 1)
		{
			if ((cksum + (uint8_t) tmp) == 0) return row_len;
		}
	}
	return -1;
}

int hexfile_out(char *line, unsigned int type, unsigned int address, unsigned char *data, unsigned int bytes)
{
	uint8_t cksum = 0;
	uint8_t i = 0;
	char tmp[8];
	
	sprintf(line, ":%2.2X%4.4X%2.2X", bytes, address, type);	
	cksum -= bytes;
	cksum -= address >> 8;
	cksum -= address & 0xFF;
	cksum -= type;

	for (i=0; i<bytes; i++)
	{
		sprintf(tmp, "%2.2X", data[i]);
		strcat(line, tmp);
		cksum -= data[i];
	}
	sprintf(tmp, "%2.2X\r\n", cksum);
	strcat(line, tmp);
	
	return strlen(line);
}

