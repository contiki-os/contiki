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

int hexfile_build_tables(char *ihexfile, unsigned char *page_buffer, unsigned char *page_table)
{
	unsigned char buffer[256];
	int length;
	int i;
	int pages;
	
	unsigned long ext_addr=0;
	unsigned short int addr=0;
	FILE *ihex_fp;
	
	if((ihex_fp = (FILE *)fopen(ihexfile, "r")) == NULL)
	{
		printf("Failed to open .ihex file %s\n", ihexfile);
		return(-1);
	}
	else
	{
		printf(".ihex file ok.\n");
	}

	bzero(buffer, sizeof(buffer));

	/*initialize page data*/
	memset(page_table, 0, 64);
	memset(page_buffer, 0xFF, sizeof(page_buffer));
	pages = 0;
	
	while((fscanf(ihex_fp, "%s", buffer) == 1))
	{	
		unsigned char data_len = 0;

		if (memcmp(&buffer[7], "00", 2) == 0)
		{	/*Data record*/
			i=0;
			sscanf((char *)&buffer[1], "%2hhx", &data_len);
			sscanf((char *)&(buffer[3]),"%4hx", &addr);
			while(i<data_len)
			{
				uint32_t absolute_address = ext_addr+addr+i;
				
				if (page_table[absolute_address/2048] == 0) 
				{
					page_table[absolute_address/2048] = 1;
					printf("Writing %d pages.\r", ++pages);
				}
				sscanf((char *)&buffer[2*i+9], "%2hhx", &page_buffer[absolute_address]);
				i++;
			}
		}
		else if (memcmp(&buffer[7], "01", 2) == 0)
		{	/*end file*/
			printf("\nFile read complete.\n");
			break;
		}
		else if (memcmp(&buffer[7], "04", 2) == 0)
		{
			sscanf((char *)&(buffer[3]),"%4hx", &addr);
			sscanf((char *)&(buffer[9]),"%4lx", &ext_addr);
			printf("\nExtended page address: 0x%8.8lX\n", ext_addr*0x8000 );
			if (ext_addr) ext_addr--;
			ext_addr *= 0x8000;
		}
	}
	
	fclose(ihex_fp);
	return(pages);
}


int hexfile_program(unsigned char *page_buffer, unsigned char *page_table)
{
	int retry = 0;
	int error = 0;
	unsigned char buffer[256];
	int length;
	int i;
	int pages;
	
	unsigned long ext_addr=0;
	unsigned short int addr=0;

	printf("Starting programming.\n");
	error = 0;
	for (i=0; i<64; i++)
	{
		if (page_table[i] != 0)
		{
			ext_addr = 2048*i;

			bzero(buffer, sizeof(buffer));

			// Write the start address and check return
			usleep(3000);
			cdi_set_address(ext_addr);

			printf("\r                                                       \r");
			fflush(stdout);
			
			cdi_flash_write(&page_buffer[ext_addr], 2048);

			usleep(20000);

		}
	}

	return(1);
}



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

