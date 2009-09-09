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


#define PROGRAMMER_VERSION "v1.3"

typedef enum
{
	PAGE_SDCC,
	PAGE_LINEAR,
	PAGE_UNDEFINED
}page_mode_t;

typedef struct {
#ifdef PLATFORM_WINDOWS
	int device;
#else
	char device[128];
#endif	
	int target_type;
	int prg_type;
	int action;
	char ihex_file[128];
	unsigned char write_mac[8];
	page_mode_t page_mode;
}conf_opts_t;

enum target { UNDEFINED, VERSION, CDI };

#ifdef PLATFORM_WINDOWS
extern int programmer_init(int device, port_t **port);
#else
extern int programmer_init(char *device, port_t **port);
#endif

extern void programmer_close(port_t *port);

