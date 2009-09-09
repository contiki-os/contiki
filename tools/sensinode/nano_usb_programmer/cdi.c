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
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif
#include "cdi.h"

/*read none/8*/
#define CDI_CHIP_ERASE		0x10
#define CDI_WR_CONFIG			0x19
#define CDI_SET_HW_BRKPNT 0x3B

/*read 8*/
#define CDI_RD_CONFIG			0x24
#define CDI_READ_STATUS		0x34
#define CDI_HALT 					0x44
#define CDI_RESUME				0x4C
#define CDI_STEP_INSTR		0x5C

/*variable len, add data length to command byte*/
#define CDI_DBGINSTR			0x54
#define CDI_STEP_REPLACE	0x64

/*response = 16 bits*/
#define CDI_GET_PC			 	0x28
#define CDI_GET_CHIP_ID 	0x68


/* internals */
void cdi_command_push(uint8_t *byte, uint8_t bytes_out, uint16_t *retval, uint8_t return_bits);

void cdi_flash_bank(uint8_t bank, uint8_t unif_mode);

void cdi_dptr_write(uint16_t value);

uint8_t cdi_reg_read(uint8_t reg_addr);
void cdi_reg_write(uint8_t reg_addr, uint8_t value);

void cdi_xdata_write(uint8_t value);
uint8_t cdi_xdata_read(void);

void cdi_flash_init(void);
void cdi_flash_write_page(void);

uint32_t cdi_addr = 0;

#define pause_ms(x) usleep(x*1000)
#define pause_us(x) usleep(x)

void cdi_command_push(uint8_t *byte, uint8_t bytes_out, uint16_t *retval, uint8_t return_bits)
{
	uint8_t i, val;
		
	for (i=0; i<bytes_out; i++)
	{
		prog_write(*byte);
		byte++;
	}

	return_bits >>= 3;	
	for (i=0; i<return_bits; i++)
	{
		*retval <<= 8;
		
		prog_read(&val);
		*retval += val;
	}
}

void cdi_reg_write(uint8_t reg_addr, uint8_t value)
{
	uint8_t out[4];
	uint16_t retval;
	
	out[0] = CDI_DBGINSTR + 3; /*command length 3 bytes*/
	out[1] = 0x75;		/*mov sfr, #immediate*/
	out[2] = reg_addr;		/*sfr = reg_addr*/
	out[3] = value;		/*immediate*/
	cdi_command_push(out, 4, &retval, 8);
}

void cdi_xdata_write(uint8_t value)
{
	uint8_t out[3];
	uint16_t retval;

	out[0] = CDI_DBGINSTR + 2; /*command length 3 bytes*/
	out[1] = 0x74;		/*mov a, #immediate*/
	out[2] = value;		/*immediate*/
	cdi_command_push(out, 3, &retval, 8);
	
	out[0] = CDI_DBGINSTR + 1; /*command length 3 bytes*/
	out[1] = 0xF0;		/*movx @dptr, a*/
	cdi_command_push(out, 2, &retval, 8);
	
	out[0] = CDI_DBGINSTR + 1; /*command length 1 byte*/
	out[1] = 0xA3;		/*inc DPTR*/
	cdi_command_push(out, 2, &retval, 8);	
}

uint8_t cdi_xdata_read(void)
{
	uint8_t out[2];
	uint16_t retval;
	uint8_t retval8;
	
	out[0] = CDI_DBGINSTR + 1; /*command length 3 bytes*/
	out[1] = 0xE0;		/*movx a, @dptr*/
	cdi_command_push(out, 2, &retval, 8);
	retval8 = retval;
		
	out[0] = CDI_DBGINSTR + 1; /*command length 1 byte*/
	out[1] = 0xA3;		/*inc DPTR*/
	cdi_command_push(out, 2, &retval, 8);
	
	return retval8;
}

void cdi_dptr_write(uint16_t value)
{
	uint8_t out[4];
	uint16_t retval;

	/*set DPTR to address*/	
	out[0] = CDI_DBGINSTR + 3; /*command length 3 bytes*/
	out[1] = 0x90;		/*mov DPTR, #immediate16*/
	out[2] = (value >> 8);  /*immediateh*/
	out[3] = value;		/*immediatel*/
	cdi_command_push(out, 4, &retval, 8);
}


uint8_t cdi_reg_read(uint8_t reg_addr)
{
	uint8_t out[3];
	uint16_t retval;
	
	out[0] = CDI_DBGINSTR + 2; /*command length 2 bytes*/
	out[1] = 0xE5;		/*mov a, sfr*/
	out[2] = reg_addr;		/*sfr = reg_addr*/
	cdi_command_push(out, 3, &retval, 8);
	
	return (uint8_t) retval;
}


void cdi_flash_bank(uint8_t bank, uint8_t unif_mode)
{
	uint8_t out;
	
	out = (bank << 4) + 1;
	out &= 0x31;
	if (unif_mode) out |= 0x40;	/*set unified memory model*/
	
	cdi_reg_write(0xC7, out);
}


void cdi_flash_read(uint8_t *ptr, uint16_t bytes)
{
	uint16_t i;
	uint8_t out[4];
	uint16_t retval;
	
	cdi_flash_bank((cdi_addr >> 15), 0);

	cdi_dptr_write(cdi_addr | 0x8000);

	for (i=0; i<bytes; i++)
	{
		out[0] = CDI_DBGINSTR + 1; /*command length 1 byte*/
		out[1] = 0xE4;		/*clr a*/
		cdi_command_push(out, 2, &retval, 8);
		
		out[0] = CDI_DBGINSTR + 1; /*command length 1 byte*/
		out[1] = 0x93;		/*movc a, @A+DPTR*/
		cdi_command_push(out, 2, &retval, 8);
		*ptr++ = retval;
		
		out[0] = CDI_DBGINSTR + 1; /*command length 1 byte*/
		out[1] = 0xA3;		/*inc DPTR*/
		cdi_command_push(out, 2, &retval, 8);
	}	

	cdi_addr += bytes;
}

const uint8_t cdi_ram_code[] =
{
	0x7E, 0x00, 			/*mov r6, #0*/
	0x7D, 0x04,				/*mov r5, #4*/
/*C0:*/
	0xE5, 0xAE,				/*MOV A,_FCTL*/
	0x20, 0xE7, 0xFB,	/*JB ACC.7,C0*/
	0x75, 0xAE, 0x02,	/*mov _FCTL,#0x02*/
/*C1:*/
	0xE0,							/*movx a, @DPTR*/
	0xF5, 0xAF,				/*MOV _FWDATA, a*/
	0xA3,							/*inc DPTR*/
	0xE0,							/*movx a, @DPTR*/
	0xF5, 0xAF,				/*MOV _FWDATA, a*/
	0xA3,							/*inc DPTR*/
	0xE0,							/*movx a, @DPTR*/
	0xF5, 0xAF,				/*MOV _FWDATA, a*/
	0xA3,							/*inc DPTR*/
	0xE0,							/*movx a, @DPTR*/
	0xF5, 0xAF,				/*MOV _FWDATA, a*/
	0xA3,							/*inc DPTR*/
/*C2:*/
	0xE5, 0xAE,				/*MOV A,_FCTL*/
	0x20, 0xE6, 0xFB,	/*JB ACC.6,C2*/
	0x1E,							/*dec r6*/
	0xDE, 0xE8,				/*djnz r6,C1*/
	0xDD, 0xE6,				/*djnz r5,C1*/
/*C3:*/
	0xE5, 0xAE,				/*MOV A,_FCTL*/
	0x20, 0xE7, 0xFB,	/*JB ACC.7,C3*/
	0x00,				/*nop*/
	0x00,				/*nop*/
/*C4:*/
	0x80, 0xFE	/*sjmp C4*/
};

void cdi_flash_init(void)
{
	uint8_t i;
	
	/*set clock*/
	cdi_reg_write(0xC6, 0xC9);	/*sfr = CLKCON*/
	/*set timing*/	
	cdi_reg_write(0xAB, 0x15);	/*sfr = FWT*/
	/*copy code to ram*/
	cdi_dptr_write(0xE000);
	for (i=0; i<sizeof(cdi_ram_code); i++)
	{
		cdi_xdata_write(cdi_ram_code[i]);
	}
}

void cdi_flash_write_page(void)
{
	uint16_t retval;
	uint8_t out[4];
		
	/*set unified memory model*/
	cdi_flash_bank(0, 1);
	
	/*set FADDR to page start*/	
	cdi_reg_write(0xAD, (cdi_addr >> 10) & 0x7F); 	/*sfr = FADDRH*/
	cdi_reg_write(0xAC, (cdi_addr >> 2)); 					/*sfr = FADDRL*/

	/*erase page*/
	cdi_reg_write(0xAE, 0x01);
	pause_ms(40);
	
	/*set program counter*/	
	out[0] = CDI_DBGINSTR + 3; /*command length 3 bytes*/
	out[1] = 0x02;		/*ljmp immediate16*/
	out[2] = 0xE0;  /*immediateh*/
	out[3] = 0x00;		/*immediatel*/
	cdi_command_push(out, 4, &retval, 8);

	/*set breakpoint*/	
	out[0] = CDI_SET_HW_BRKPNT; /*command length 3 bytes*/
	out[1] = 0x04;
	out[2] = 0xE0;  /*immediateh*/
	out[3] = sizeof(cdi_ram_code)-2;		/*immediatel*/
	cdi_command_push(out, 4, &retval, 0);

	cdi_dptr_write(0xE800);	/*data area, set your page data here*/
	/*execute*/
	out[0] = CDI_RESUME; /*command length 3 bytes*/
	cdi_command_push(out, 1, &retval, 8);
	
	pause_ms(30);
	out[3]= 0;
	do
	{
		pause_ms(20);
		out[0] = CDI_READ_STATUS;
		cdi_command_push(out, 1, &retval, 8);
		printf("-");
		fflush(stdout);
	}while( ((retval & 0x20) == 0) && (out[3]++ < 200) );
}

int cdi_flash_write(uint8_t *ptr, uint16_t length)
{
	uint16_t i, retval;
	uint8_t out[3];
	
	if (length > 2048) return -1;
	
	cdi_addr &= 0x1F800; /*make sure address is on page boundary*/

	printf("0x%6.6X: ", cdi_addr);
	fflush(stdout);
	cdi_dptr_write(0xE800);	/*our page data buffer is here*/
	for (i=0; i< length; i++)
	{
		cdi_xdata_write(*ptr++);
		if ((i & 0x3F) == 0)
		{
			printf(".");
		}
		if ((i & 0x0F) == 0x00)
		{
			printf("\bo");
		}
		if ((i & 0x0F) == 0x08)
		{
			printf("\b.");
		}
		fflush(stdout);
	}
	while(i<2048)
	{
		cdi_xdata_write(0xFF);
		if ((i & 0x3F) == 0)
		{
			printf(".");
		}
		if ((i & 0x0F) == 0x00)
		{
			printf("\bo");
		}
		if ((i & 0x0F) == 0x08)
		{
			printf("\b.");
		}
		fflush(stdout);
		i++;
	}
	
	out[0] = CDI_HALT;									
	cdi_command_push(out, 1, &retval, 8);

	out[0] = CDI_READ_STATUS;
	retval = 0;
	cdi_command_push(out, 1, &retval, 8);
	if ((retval & 0xFF) == 0xB2)
	{
		/*restore config*/
		out[0] = CDI_WR_CONFIG; /*command length 3 bytes*/
		out[1] = 0x0E;		/*write flash area*/
		cdi_command_push(out, 2, &retval, 0);
		/*set flash timings and copy code to ram*/
		cdi_flash_init();
		/*write page*/
		cdi_flash_write_page();
		cdi_addr += 2048;	/*increment page address*/
		pause_ms(10);
	}
	else
	{
		return -1;
	}
	return 0;
}

int cdi_flash_write_mac(uint8_t *ptr)
{
	uint16_t i, retval;
	uint8_t out[3];
	
	cdi_addr = 0x1F800; /*last page*/

	printf("0x%6.6X", cdi_addr);
	fflush(stdout);
	cdi_dptr_write(0xEFF8);	/*our page data buffer is here*/
	for (i=0; i<8; i++)
	{
		cdi_xdata_write(*ptr++);
		if ((i & 0x0F) == 0)
		{
			printf(".");
			fflush(stdout);
		}
	}
	
	out[0] = CDI_HALT;									
	cdi_command_push(out, 1, &retval, 8);

	out[0] = CDI_READ_STATUS;
	retval = 0;
	cdi_command_push(out, 1, &retval, 8);
	if ((retval & 0xFF) == 0xB2)
	{
		/*restore config*/
		out[0] = CDI_WR_CONFIG; /*command length 3 bytes*/
		out[1] = 0x0E;		/*write flash area*/
		cdi_command_push(out, 2, &retval, 0);
		/*set flash timings and copy code to ram*/
		cdi_flash_init();
		/*write page*/
		cdi_flash_write_page();
		cdi_addr += 2048;	/*increment page address*/
		pause_ms(10);
	}
	else
	{
		return -1;
	}
	return 0;
}

int cdi_start(uint16_t *chip_id)
{
	uint8_t out[3];
	uint16_t retval;

	prog_start();	/*do the CDI startup sequence*/

	out[0] = CDI_READ_STATUS;
	cdi_command_push(out, 1, &retval, 8);
	
	printf("Status: %2.2X.\n", retval & 0xFF);
	
	out[0] = CDI_GET_CHIP_ID;
	cdi_command_push(out, 1, &retval, 16);

	*chip_id = retval;

	out[0] = CDI_HALT;
	cdi_command_push(out, 1, &retval, 8);
	
	pause_ms(100);

	out[0] = CDI_WR_CONFIG; /*command length 3 bytes*/
	out[1] = 0x0E;		/*write flash area*/
	cdi_command_push(out, 2, &retval, 0);

	pause_ms(10);

	cdi_reg_write(0xC6, 0xC9);	/*sfr = CLKCON*/

	pause_ms(10);
	
	cdi_reg_write(0xAB, 0x15);	/*sfr = FWT*/

	cdi_addr = 0;
	
	return 0;
}				

int cdi_erase(void)
{
	uint8_t out[3];
	uint16_t retval;
	uint8_t i;
	
	out[0] = CDI_WR_CONFIG;
	out[1] = 0x0E;
	cdi_command_push(out, 2, &retval, 0);
	out[0] = CDI_CHIP_ERASE;
	cdi_command_push(out, 1, &retval, 0);
	retval = 0;					
	i = 0;					
	do 
	{
		pause_ms(30);
		out[0] = CDI_READ_STATUS;
		cdi_command_push(out, 1, &retval, 8);
	}while( ((retval & 0x84) != 0x80) && (i++ < 100) );

	cdi_addr = 0;

	if (i >= 100) 
	{	
		return -1;
	}
	return 0;
}

void cdi_set_address(uint32_t address)
{
	cdi_addr = address;
}
