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


#ifndef __WINDOWS_TYPES__
#define __WINDOWS_TYPES__

#define MAX_NUM_DEVICES 50
#include <sys/time.h>

typedef unsigned long 	DWORD;
typedef unsigned long 	ULONG;
typedef unsigned short 	USHORT;
typedef short 			SHORT;
typedef unsigned char	UCHAR;
typedef unsigned short	WORD;
typedef unsigned char	BYTE;
typedef unsigned char	*LPBYTE;
typedef int 		BOOL;
typedef char 		BOOLEAN;
typedef char 		CHAR;
typedef int 		*LPBOOL;
typedef unsigned char 	*PUCHAR;
typedef const char	*LPCSTR;
typedef char		*PCHAR;
typedef void 		*PVOID;
typedef void 		*HANDLE;
typedef long 		LONG;
typedef int 		INT;
typedef unsigned int 	UINT;
typedef char 		*LPSTR;
typedef char 		*LPTSTR;
typedef DWORD 		*LPDWORD;
typedef WORD 		*LPWORD;
typedef ULONG		*PULONG;
typedef PVOID		LPVOID;
typedef void		VOID;
typedef unsigned long long int ULONGLONG;

typedef struct _OVERLAPPED {
	DWORD Internal;
	DWORD InternalHigh;
	DWORD Offset;
	DWORD OffsetHigh;
	HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef struct _SECURITY_ATTRIBUTES {
	DWORD nLength;
	LPVOID lpSecurityDescriptor;
	BOOL bInheritHandle;
} SECURITY_ATTRIBUTES , *LPSECURITY_ATTRIBUTES;

typedef struct timeval SYSTEMTIME;
typedef struct timeval FILETIME;
#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

//
// Modem Status Flags
//
#define MS_CTS_ON           ((DWORD)0x0010)
#define MS_DSR_ON           ((DWORD)0x0020)
#define MS_RING_ON          ((DWORD)0x0040)
#define MS_RLSD_ON          ((DWORD)0x0080)

//
// Error Flags
//

#define CE_RXOVER           0x0001  // Receive Queue overflow
#define CE_OVERRUN          0x0002  // Receive Overrun Error
#define CE_RXPARITY         0x0004  // Receive Parity Error
#define CE_FRAME            0x0008  // Receive Framing error
#define CE_BREAK            0x0010  // Break Detected
#define CE_TXFULL           0x0100  // TX Queue is full
#define CE_PTO              0x0200  // LPTx Timeout
#define CE_IOE              0x0400  // LPTx I/O Error
#define CE_DNS              0x0800  // LPTx Device not selected
#define CE_OOP              0x1000  // LPTx Out-Of-Paper
#define CE_MODE             0x8000  // Requested mode unsupported

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE 0xFFFFFFFF
#endif

#endif
