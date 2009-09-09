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


#ifndef _PORT_H
#define _PORT_H

#ifndef PLATFORM_WINDOWS
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#else
#include "windows.h"
#include <stdint.h>
#include "ftd2xx.h"
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

#ifndef PLATFORM_WINDOWS
typedef struct
{
	int handle;
	char *device;
	struct termios old_params;
}port_t;

extern int port_open(port_t **port, char *device);
#else
typedef struct port_t
{
	FT_HANDLE handle;
	int device;
	HANDLE event_handle;
}port_t;

extern int port_open(port_t **port, int device);
#endif

extern int port_close(port_t *port);
extern int port_write(port_t *port, unsigned char *buffer, size_t buflen);
extern int port_read(port_t *port, unsigned char *buffer, size_t buflen);
extern int port_get(port_t *port, unsigned char *buffer, int timeout);
extern int port_set_params(port_t *port, uint32_t speed, uint8_t rtscts);
extern int port_dtr_set(port_t *port);
extern int port_dtr_clear(port_t *port);
extern int port_rts_set(port_t *port);
extern int port_rts_clear(port_t *port);
extern int port_readline(port_t *port, unsigned char *buffer, int buf_size, int timeout);
extern int port_write_echo(port_t *port, char *string);
extern int port_write_8byte_no_echo(port_t *port, int dlen, char *string);

#ifdef __cplusplus
}
#endif
#endif /* _PORT_H */
