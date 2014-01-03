/**
 * ymodem.c
 *
 * based on ymodem.c for RTDSR, copyright (c) 2011 Pete B. <xtreamerdev@gmail.com>
 * based on ymodem.c for bootldr, copyright (c) 2001 John G Dorsey
 * baded on ymodem.c for reimage, copyright (c) 2009 Rich M Legrand
 * crc16 function from PIC CRC16, by Ashley Roll & Scott Dattalo
 * crc32 function from crc32.c by Craig Bruce
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * These Ymodem calls are aimed at embedded software and tailored to
 * work against Microsoft's HyperTerminal. Some of the Ymodem protocol
*  operations have been voluntarily left out.
 *
 * To be able to use these functions, you must provide:
 * o int _getchar(int timeout): A serial getchar() call, with a
 *   timeout expressed in seconds. Negative means infinite timeout.
 *   returns a character as an int, or negative on error/timeout.
 * o void _putchar(int c): A serial putchar() call
 */

/**
 * \file
 *         Ymodem implementation
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "ymodem.h"

#include "clock.h"
#include "crc16.h"
#include "shell.h"
#include "cfs/cfs.h"
#include "dev/flash.h"

#include "lib/ringbuf.h"

/*********************************************
 * Contiki Adaptation
 *
 */
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#define _sleep sleep

#if DEBUG
// Force another uart
#include "uart0.h"
#define defaultuart_set_input uart0_set_input
#define defaultuart_get_input uart0_get_input
#define defaultuart_writeb    uart0_writeb
#endif


void _putchar(int c)
{
	defaultuart_writeb(c);
}

static uint8_t _rxbuf_data[128];
static struct ringbuf rxbuf;
static char _packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
static uint32_t _flash_addr = 0;


int
ymodem_input_byte(unsigned char c)
{
	ringbuf_put(&rxbuf, c);
	//PRINTF("rx %02X\n\r",c);
	return 0;
}


#define wait_until_cond_or_timeout(cond, timeout_ms)   \
 do {                                              \
	uint32_t timeout = timeout_ms;                  \
	while(1) {                                     \
	  if(cond) break;                              \
	  clock_wait(1);                                \
	  timeout--;                                   \
	  if(timeout == 0) break;                      \
	}                                              \
  } while(0)


static int _getchar_ms(int timeout_ms)
{
	int tmo = timeout_ms;
	 do {
		while(1) {
		  if(ringbuf_elements(&rxbuf) > 0) break;
		  clock_wait(1);
		  tmo--;
		  if(tmo <= 0) break;
		}
	  } while(0);

	if(tmo > 0)
	{
		return ringbuf_get(&rxbuf);
	}
	else
	{
		return -1; // -1
	}
}

#define _getchar(timeout) _getchar_ms(timeout*1000)

static void flushrx(void)
{
	while(_getchar_ms(10) > 0);
}

static const char *u32_to_str(unsigned int val)
{
	/* Maximum number of decimal digits in u32 is 10 */
	static char num_str[11];
	int  pos = 10;
	num_str[10] = 0;

	if (val == 0) {
		/* If already zero then just return zero */
		return "0";
	}

	while ((val != 0) && (pos > 0)) {
		num_str[--pos] = (val % 10) + '0';
		val /= 10;
	}

	return &num_str[pos];
}

static unsigned long str_to_u32(char* str)
{
	return shell_strtolong(str, NULL);
#if 0
	const char *s = str;
	unsigned long acc;
	int c;

	/* strip leading spaces if any */
	do {
		c = *s++;
	} while (c == ' ');

	for (acc = 0; (c >= '0') && (c <= '9'); c = *s++) {
		c -= '0';
		acc *= 10;
		acc += c;
	}
	return acc;
#endif
}

/* Returns 0 on success, 1 on corrupt packet, -1 on error (timeout): */
static int receive_packet(char *data, int *length)
{
	int i, c;
	unsigned int packet_size;

	*length = 0;

	c = _getchar(PACKET_TIMEOUT);
	if (c < 0) {
		return -1;
	}

	switch(c) {
	case SOH:
		packet_size = PACKET_SIZE;
		break;
	case STX:
		packet_size = PACKET_1K_SIZE;
		break;
	case EOT:
		return 0;
	case CAN:
		c = _getchar(PACKET_TIMEOUT);
		if (c == CAN) {
			*length = -1;
			return 0;
		}
	default:
		/* This case could be the result of corruption on the first octet
		* of the packet, but it's more likely that it's the user banging
		* on the terminal trying to abort a transfer. Technically, the
		* former case deserves a NAK, but for now we'll just treat this
		* as an abort case.
		*/
		*length = -1;
		return 0;
	}

	*data = (char)c;

	for(i = 1; i < (packet_size + PACKET_OVERHEAD); ++i) {
		c = _getchar(PACKET_TIMEOUT);
		if (c < 0) {
			return -1;
		}
		data[i] = (char)c;
	}

	/* Just a sanity check on the sequence number/complement value.
	 * Caller should check for in-order arrival.
	 */
	if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff)) {
		return 1;
	}

	if (crc16((const char *)(data + PACKET_HEADER), packet_size + PACKET_TRAILER) != 0) {
		return 1;
	}
	*length = packet_size;

#if 0
    int no=0;
    /* Dump Packet */
    PRINTF("Packet received (%d bytes) :\n\r",packet_size + PACKET_OVERHEAD);
    for(no=0; no<packet_size + PACKET_OVERHEAD; no++)
    {
  	  PRINTF("%02X ",data[no]);
  	  if((no%16) == 15) PRINTF("\n\r");
    }
    PRINTF("\n\r");
#endif

	return 0;
}

/* Returns the length of the file received, or 0 on error: */
//unsigned long ymodem_receive(unsigned char *buf, unsigned long length)
unsigned long ymodem_receive(int (*io_callback_storedata)(char* in_filename, unsigned char *in_buf, int in_length))
{
	int packet_length, i, file_done, session_done, crc_tries, crc_nak;
	unsigned int packets_received, errors, first_try = 1;
	char file_name[FILE_NAME_LENGTH], file_size[FILE_SIZE_LENGTH], *file_ptr;
	unsigned long wsize, filesize = 0, remain = 0;
	int fd = 0;

	flushrx();
	PRINTF("Ymodem rcv:\n\r");
	file_name[0] = 0;

	for (session_done = 0, errors = 0; ; ) {
		crc_tries = crc_nak = 1;
		if (!first_try) {
			_putchar(CRC);
		}
		first_try = 0;
		for (packets_received = 0, file_done = 0; ; ) {
			switch (receive_packet(_packet_data, &packet_length)) {

			case 0:
				errors = 0;
				switch (packet_length) {
					case -1:  /* abort */
						PRINTF("Packet_length unreadable\r\n");
						_putchar(CAN);
						_putchar(CAN);
						return 0;
					case 0:   /* end of transmission */
						_putchar(ACK);
						/* Should add some sort of sanity check on the number of
						 * packets received and the advertised file length.
						 */
						file_done = 1;
						break;
					default:  /* normal packet */
					if ((_packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff)) {
						_putchar(NAK);
					} else {
						if (packets_received == 0) {
							/* The spec suggests that the whole data section should
							 * be zeroed, but I don't think all senders do this. If
							 * we have a NULL filename and the first few digits of
							 * the file length are zero, we'll call it empty.
							 */
							for (i = PACKET_HEADER; i < PACKET_HEADER + 4; i++) {
								if (_packet_data[i] != 0) {
									break;
								}
							}
							if (i < PACKET_HEADER + 4) {  /* filename packet has data */
								for (file_ptr = _packet_data + PACKET_HEADER, i = 0; *file_ptr && i < FILE_NAME_LENGTH; ) {
									file_name[i++] = *file_ptr++;
								}
								file_name[i++] = '\0';
								for (++file_ptr, i = 0; *file_ptr != ' ' && i < FILE_SIZE_LENGTH; ) {
									file_size[i++] = *file_ptr++;
								}
								file_size[i++] = '\0';
								filesize = str_to_u32(file_size);
								remain = filesize;
#if 0
								if (filesize > length) {
									_putchar(CAN);
									_putchar(CAN);
									_sleep(1);
									PRINTF("\nrcv buffer too small (0x%08x vs 0x%08x)\n", length, filesize);
									return 0;
								}
#else
#ifdef COFFEE
								if(NULL == io_callback_storedata)
								{
									PRINTF("Create file %s\n",file_name);
									cfs_remove(file_name);
									fd = cfs_open(file_name,CFS_WRITE | CFS_APPEND);
									if(fd < 0)
									{
										_putchar(CAN);
										_putchar(CAN);
										_sleep(1);
										PRINTF("Can't Open file %s\n", file_name);
										return 0;
									}
								}
#endif
#endif
								_putchar(ACK);
								_putchar(crc_nak ? CRC : NAK);
								crc_nak = 0;
							} else {  /* filename packet is empty; end session */
								_putchar(ACK);
								file_done = 1;
								session_done = 1;
								break;
							}
						} else {
#if 0
							/* This shouldn't happen, but we check anyway in case the
							 * sender lied in its filename packet:
							 */
							if ((buf_ptr + packet_length) - buf > length) {
								_putchar(CAN);
								_putchar(CAN);
								_sleep(1);
								PRINTF("\nbuffer overflow: %d\n", length);
								return 0;
							}
							for (i=0; i<packet_length; i++) {
								buf_ptr[i] = _packet_data[PACKET_HEADER+i];
							}
							buf_ptr += packet_length;
#else
							if(remain < packet_length)
							{
								PRINTF("remain = %ld, pkt_length = %ld\r\n",remain,packet_length);
								packet_length = remain;
							}

							if(NULL != io_callback_storedata)
							{
								wsize = io_callback_storedata(file_name,
															 &_packet_data[PACKET_HEADER],
															 packet_length);
							}
#ifdef COFFEE
							else
							{
								// Append in file
								wsize = cfs_write(fd, &_packet_data[PACKET_HEADER], packet_length);
							}
#endif

							if( wsize != packet_length )
							{
								// Abort
								_putchar(CAN);
								_putchar(CAN);
								PRINTF("\nFailed to store data (%d)\n", wsize);
								goto ENDP;
							}
							remain = remain - packet_length;
#endif
							_putchar(ACK);
						}
						++packets_received;
					}  /* sequence number ok */
				}
				break;

			default:
				if (packets_received != 0) {
					if (++errors >= MAX_ERRORS) {
						_putchar(CAN);
						_putchar(CAN);
						_sleep(1);
						PRINTF("\ntoo many errors - aborted.\n");
						goto ENDP;
					}
				}
				_putchar(CRC);
			}
			if (file_done) {
				break;
			}
		}  /* receive packets */

		if (session_done)
			break;

	}  /* receive files */

	PRINTF("\n");
	if (filesize > 0) {
		PRINTF("read:%s\n", file_name);
#ifdef WITH_CRC32
		PRINTF("crc32:0x%08x, len:0x%08x\n", crc32(buf, filesize), filesize);
#else
		PRINTF("len:0x%08x\n", filesize);
#endif
	}
ENDP:
	if( (NULL != io_callback_storedata) && (fd > 0) )
		cfs_close(fd);
	return filesize;
}

static void send_packet(unsigned char *data, int block_no)
{
	int count, crc, packet_size;

	/* We use a short packet for block 0 - all others are 1K */
	if (block_no == 0) {
		packet_size = PACKET_SIZE;
	} else {
		packet_size = PACKET_1K_SIZE;
	}
	crc = crc16(data, packet_size);
	/* 128 byte packets use SOH, 1K use STX */
	_putchar((block_no==0)?SOH:STX);
	_putchar(block_no & 0xFF);
	_putchar(~block_no & 0xFF);

	for (count=0; count<packet_size; count++) {
		_putchar(data[count]);
	}
	_putchar((crc >> 8) & 0xFF);
	_putchar(crc & 0xFF);
}

/* Send block 0 (the filename block). filename might be truncated to fit. */
static void send_packet0(char* filename, unsigned long size)
{
	unsigned long count = 0;
	unsigned char block[PACKET_SIZE];
	const char* num;

	if (filename) {
		while (*filename && (count < PACKET_SIZE-FILE_SIZE_LENGTH-2)) {
			block[count++] = *filename++;
		}
		block[count++] = 0;


		num = u32_to_str(size);
		while(*num) {
			block[count++] = *num++;
		}
	}

	while (count < PACKET_SIZE) {
		block[count++] = 0;
	}

	send_packet(block, 0);
}


//static void send_data_packets(unsigned char* data, unsigned long size)
static void send_data_packets(int in_fd, unsigned long size)
{
	int blockno = 1;
	unsigned long send_size;
	int ch;

	while (size > 0) {
		if (size > PACKET_1K_SIZE) {
			send_size = PACKET_1K_SIZE;
		} else {
			send_size = size;
		}
		memset(_packet_data,0,sizeof(_packet_data));
		cfs_read(in_fd,_packet_data,send_size);

		send_packet(_packet_data, blockno);
		ch = _getchar(PACKET_TIMEOUT);
		if (ch == ACK) {
			blockno++;
			//data += send_size;
			size -= send_size;
		} else {
			if((ch == CAN) || (ch == -1)) {
				return;
			}
		}
	}

	do {
		_putchar(EOT);
		ch = _getchar(PACKET_TIMEOUT);
	} while((ch != ACK) && (ch != -1) );

	/* Send last data packet */
	if (ch == ACK) {
		PRINTF("File Successfully sent\n\r");
		ch = _getchar(2*PACKET_TIMEOUT);
		if (ch == CRC) {
			do {
				send_packet0(0, 0);
				ch = _getchar(PACKET_TIMEOUT);
			} while((ch != ACK) && (ch != -1));
		}
	}
}

unsigned long ymodem_send_file(char* filename)
{
	int ch, crc_nak = 1;
	int fd = -1;
	unsigned long size = 0;

	PRINTF("Ymodem send:\n");

	flushrx();

	/* Open File */
	fd = cfs_open(filename,CFS_READ);
	if(fd < 0)
	{
		PRINTF("Error opening %s\n",filename);
		return 0;
	}
	size = cfs_getsize(fd);

	/* Not in the specs, just for balance */
	do {
		//_putchar(CRC);
		ch = _getchar(1);
	} while (ch < 0);

	if (ch == CRC) {
		do {
			send_packet0(filename, size);
			/* When the receiving program receives this block and successfully
			 * opened the output file, it shall acknowledge this block with an ACK
			 * character and then proceed with a normal XMODEM file transfer
			 * beginning with a "C" or NAK tranmsitted by the receiver.
			 */
			ch = _getchar(PACKET_TIMEOUT);

			if (ch == ACK) {
				ch = _getchar(PACKET_TIMEOUT);
				if (ch == CRC) {
					send_data_packets(fd,size);
					PRINTF("\nsent:%s\n", filename);
#ifdef WITH_CRC32
					PRINTF("crc32:0x%08x, len:0x%08x\n", crc32(buf, size), size);
#else
					PRINTF("len:0x%08x\n", size);
#endif
					return size;
				}
			} else if ((ch == CRC) && (crc_nak)) {
				crc_nak = 0;
				continue;
			} else if ((ch != NAK) || (crc_nak)) {
				break;
			}
		} while(1);
	}
	_putchar(CAN);
	_putchar(CAN);
	_sleep(1);
	PRINTF("\naborted.\n");
	return 0;
}


/*---------------------------------------------------------------------------*/
PROCESS(shell_ry_process, "ry");
SHELL_COMMAND(ry_command,
	      "ry",
	      "ry <off>: Store Receiving Ymodem file in filesystem or flash offset",
	      &shell_ry_process);
/*---------------------------------------------------------------------------*/

static int store_file_in_flash(char* in_filename, unsigned char *in_buf, int in_length)
{
	int ret = 0;

	// If address is aligned, erase sector
	if( (_flash_addr & (FLASH_PAGE_SIZE - 1)) == 0)
	{
	  flash_erase(_flash_addr,FLASH_PAGE_SIZE);
	}
	ret = flash_write(_flash_addr, (const char *)in_buf, in_length);
	_flash_addr = _flash_addr + in_length;
	return ret;
}

PROCESS_THREAD(shell_ry_process, ev, data)
{
  int *uart_rx_func;
  char *args;

  _flash_addr = 0;
  args = data;

  PROCESS_BEGIN();

  if(args != NULL)
  {
	  _flash_addr = shell_strtolong(args, NULL);
  }

  if(_rxbuf_data != NULL)
  {
	  ringbuf_init(&rxbuf, _rxbuf_data, 128);

	  // Backup right rx function
	  uart_rx_func = defaultuart_get_input();
	  // Set our own serial RX parsing function
	  defaultuart_set_input(&ymodem_input_byte);

#ifdef COFFEE
	  if(_flash_addr == 0)
	  {
		  ymodem_receive(NULL);
	  }
	  else
	  {
		  ymodem_receive(&store_file_in_flash);
	  }
#else
	  ymodem_receive(&store_file_in_flash);
#endif

	  // Reset old receive function
	  defaultuart_set_input(uart_rx_func);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

#ifdef COFFEE
/*---------------------------------------------------------------------------*/
PROCESS(shell_sy_process, "sy");
SHELL_COMMAND(sy_command,
	      "sy",
	      "sy filename: Send file through Ymodem",
	      &shell_sy_process);
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(shell_sy_process, ev, data)
{
  int *uart_rx_func;

  PROCESS_BEGIN();

  if(_rxbuf_data != NULL)
  {
	  ringbuf_init(&rxbuf, _rxbuf_data, 128);

	  // Backup right rx function
	  uart_rx_func = defaultuart_get_input();
	  // Set our own serial RX parsing function
	  defaultuart_set_input(&ymodem_input_byte);

	  ymodem_send_file(data);

	  // Reset old receive function
	  defaultuart_set_input(uart_rx_func);
  }

  PROCESS_END();
}
#endif
/*---------------------------------------------------------------------------*/

void
shell_ymodem_init(void)
{
  shell_register_command(&ry_command);
#ifdef COFFEE
  shell_register_command(&sy_command);
#endif
}
/*---------------------------------------------------------------------------*/

