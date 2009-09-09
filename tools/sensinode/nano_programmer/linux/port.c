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


#include <sys/poll.h>

#include "port.h"

int port_open(port_t **port, char *device)
{
	port_t *new_port = (port_t *) malloc(sizeof(port_t));
	char err_string[128];
	
	*port = new_port;
	
	new_port->device = 0;
	new_port->handle = 0;

	new_port->handle = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);

	if (new_port->handle <= 0)
	{
		strerror_r(errno, err_string, 128);
		
		printf("Serial port open failed with error message: %s.\n", err_string);
		return(-1);
	}
	else
	{
		tcgetattr(new_port->handle, &(new_port->old_params));
	
		fcntl(new_port->handle, F_SETFL, FASYNC);
		printf("Serial port %s opened succesfully.\n", device);

		return(0);
	}	
}

int port_close(port_t *port)
{
	if (!port)
		return(-1);
	
	if ((port->handle) > 0)
	{
		tcflush(port->handle, TCIFLUSH);
		tcsetattr(port->handle,TCSANOW,&(port->old_params));
	
		close(port->handle);
		port->handle = 0;
	}

	if (port->device) free(port->device);
	port->device = 0;
	free(port);
	
	return(1);
}


/** @todo port_write() function probably needs mutexes -mjs */

int port_write(port_t *port, unsigned char *buffer, size_t buflen)
{
	int i=0;

	if (!port) return -1;
	
	/** @todo The write to serial port is at the moment done one octet at a time with 10ms interval between each write operation due to some minor problems in MCU interrupts. -mjs */
	while(i < buflen)
	{		
		write(port->handle, &(buffer[i]), 1);
		tcflush(port->handle, TCIFLUSH);
		i++;
	}

/*		write(port->handle, &(buffer[i]), buflen);*/

	tcflush(port->handle, TCIFLUSH);
	
	return(0);
}

int port_read(port_t *port, unsigned char *buffer, size_t buflen)
{
	unsigned int l = 0;
	l = read(port->handle, buffer, buflen);
	return(l);
}

int port_set_params(port_t *port, uint32_t speed, uint8_t rtscts)
{
	int rate = B115200;
	struct termios newtio;
	
	if (!port) return -1;
	
	switch (speed)
	{
		case 230400:
			rate = B230400;
			break;
			
		case 0:
		case 115200:
			rate = B115200;
			break;
			
		case 57600:
			rate = B57600;
			break;
			
		case 38400:
			rate = B38400;
			break;
			
		case 19200:
			rate = B19200;
			break;
			
		case 9600:
			rate = B9600;
			break;
		
		default:
			return -1;
			
	}
	bzero(&newtio, sizeof(newtio));

	if (speed == 9600)
	{	
		newtio.c_cflag |= CS8 | CSTOPB | CLOCAL | CREAD;
	}
	else
	{
		newtio.c_cflag |= CS8 |  CLOCAL | CREAD;
	}
	if (rtscts)
	{
		newtio.c_cflag |= CRTSCTS;
	}
	newtio.c_iflag = IGNPAR;
	
	cfsetispeed(&newtio, rate);
	cfsetospeed(&newtio, rate);
	
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;
	
#if 0	
	newtio.c_cflag = rate | CS8 | CLOCAL | CREAD | CSTOPB;
/*	if (rts_cts) newtio.c_cflag |= CRTSCTS;*/
	
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	newtio.c_lflag = 0;
	
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;
#endif

	tcflush(port->handle, TCIFLUSH);
	tcsetattr(port->handle,TCSANOW,&newtio);

	return(0);
}

int port_dtr_set(port_t *port)
{
	int port_state = TIOCM_DTR;
	
	if (!port) return(-1);
	
/*	error = ioctl(port->handle, TIOCMGET, &port_state);
	port_state |= TIOCM_RTS;
	ioctl(port->handle, TIOCMSET, &port_state);*/
	
	ioctl(port->handle, TIOCMBIS, &port_state);
	return 0;
}

int port_dtr_clear(port_t *port)
{
	int port_state = TIOCM_DTR;
	
	if (!port) return(-1);
	
	ioctl(port->handle, TIOCMBIC, &port_state);
	return 0;
}

int port_rts_set(port_t *port)
{
	int port_state = TIOCM_RTS;
	
	if (!port) return(-1);
	
	ioctl(port->handle, TIOCMBIS, &port_state);
	return 0;
}

int port_rts_clear(port_t *port)
{
	int port_state = TIOCM_RTS;
	
	if (!port) return(-1);
	
	ioctl(port->handle, TIOCMBIC, &port_state);
	return 0;
}

int port_get(port_t *port, unsigned char *buffer, int timeout)
{
	struct pollfd pfds;
	unsigned int nfds = 1;
	int bytes = 0;
	int rval;

	pfds.fd = (int)(port->handle);
	pfds.events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

	rval = poll(&pfds, nfds, timeout);
	
	if((rval & POLLIN) != POLLIN)
	{
		return(-1);
	}
	else
	{
		bytes = port_read(port, buffer, 1);
	}
	return bytes;
}

int port_readline(port_t *port, unsigned char *buffer, int buf_size, int timeout)
{
	int length = 0;
	struct pollfd pfds;
	unsigned int nfds = 1;
	int bytes = 0;
	int rval;

	pfds.fd = (int)(port->handle);
	pfds.events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

	do
	{
		rval = poll(&pfds, nfds, timeout);
		if((rval & POLLIN) != POLLIN)
		{
			return(length);
		}
		else
		{
			bytes = port_read(port, &(buffer[length]), 1);
			if (buffer[length] == '\n')
			{ 
				buf_size = length;
			}
			length += bytes;
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

