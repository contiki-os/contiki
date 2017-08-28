/*
 * Copyright (c) 2016, Benoît Thébaudeau <benoit@wsystem.com>
 * All rights reserved.
 *
 * Based on the FatFs Module,
 * Copyright (c) 2016, ChaN
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup remote
 * @{
 *
 * \defgroup remote-fat RE-Mote FatFs
 *
 * Default port of FatFs on RE-Mote.
 * @{
 *
 * \file
 * Header file configuring FatFs for RE-Mote.
 */
#ifndef FFCONF_H_
#define FFCONF_H_

#include "contiki-conf.h"

#define _FFCONF 68020 /**< Revision ID */

/*----------------------------------------------------------------------------*/
/** \name Function Configuration
 * @{
 */

#ifndef _FS_READONLY
/** This option switches the read-only configuration
 * (\c 0: read/write or \c 1: read-only).
 *
 * The read-only configuration removes the writing functions from the API:
 * \c f_write(), \c f_sync(), \c f_unlink(), \c f_mkdir(), \c f_chmod(),
 * \c f_rename(), \c f_truncate(), \c f_getfree(), and optional writing
 * functions as well.
 */
#define _FS_READONLY    0
#endif

#ifndef _FS_MINIMIZE
/** This option defines the minimization level to remove some basic API
 * functions.
 *
 * \c 0: All the basic functions are enabled.
 * \c 1: \c f_stat(), \c f_getfree(), \c f_unlink(), \c f_mkdir(),
 *       \c f_truncate(), and \c f_rename() are removed.
 * \c 2: \c f_opendir(), \c f_readdir(), and \c f_closedir() are removed in
 *       addition to \c 1.
 * \c 3: \c f_lseek() is removed in addition to \c 2.
 */
#define _FS_MINIMIZE    0
#endif

#ifndef _USE_STRFUNC
/** This option switches the string functions: \c f_gets(), \c f_putc(),
 * \c f_puts(), and \c f_printf().
 *
 * \c 0: Disable string functions.
 * \c 1: Enable without LF-CRLF conversion.
 * \c 2: Enable with LF-CRLF conversion.
 */
#define _USE_STRFUNC    1
#endif

#ifndef _USE_FIND
/** This option switches the filtered directory read functions: \c f_findfirst()
 * and \c f_findnext() (\c 0: disable, \c 1: enable, \c 2: enable with matching
 * \c altname[] too).
 */
#define _USE_FIND       1
#endif

#ifndef _USE_MKFS
/** This option switches the \c f_mkfs() function
 * (\c 0: disable or \c 1: enable).
 */
#define _USE_MKFS       1
#endif

#ifndef _USE_FASTSEEK
/** This option switches the fast seek function
 * (\c 0: disable or \c 1: enable).
 */
#define _USE_FASTSEEK   0
#endif

#ifndef _USE_EXPAND
/** This option switches the \c f_expand() function
 * (\c 0: disable or \c 1: enable).
 */
#define _USE_EXPAND     0
#endif

#ifndef _USE_CHMOD
/** This option switches the attribute manipulation functions: \c f_chmod() and
 * \c f_utime() (\c 0: disable or \c 1: enable). Also, \c _FS_READONLY needs to
 * be \c 0 to enable this option.
 */
#define _USE_CHMOD      1
#endif

#ifndef _USE_LABEL
/** This option switches the volume label functions: \c f_getlabel() and
 * \c f_setlabel() (\c 0: disable or \c 1: enable).
 */
#define _USE_LABEL      1
#endif

#ifndef _USE_FORWARD
/** This option switches the \c f_forward() function
 * (\c 0: disable or \c 1: enable).
 */
#define _USE_FORWARD    0
#endif

/** @} */
/*----------------------------------------------------------------------------*/
/** \name Locale and Namespace Configuration
 * @{
 */

#ifndef _CODE_PAGE
/** This option specifies the OEM code page to be used on the target system.
 * Incorrect setting of the code page can cause a file open failure.
 *
 * \c 1   - ASCII (no extended character, non-LFN cfg. only)
 * \c 437 - U.S.
 * \c 720 - Arabic
 * \c 737 - Greek
 * \c 771 - KBL
 * \c 775 - Baltic
 * \c 850 - Latin 1
 * \c 852 - Latin 2
 * \c 855 - Cyrillic
 * \c 857 - Turkish
 * \c 860 - Portuguese
 * \c 861 - Icelandic
 * \c 862 - Hebrew
 * \c 863 - Canadian French
 * \c 864 - Arabic
 * \c 865 - Nordic
 * \c 866 - Russian
 * \c 869 - Greek 2
 * \c 932 - Japanese (DBCS)
 * \c 936 - Simplified Chinese (DBCS)
 * \c 949 - Korean (DBCS)
 * \c 950 - Traditional Chinese (DBCS)
 */
#define _CODE_PAGE      437
#endif

#ifndef _USE_LFN
/** \c _USE_LFN switches the support of long file name (LFN).
 *
 * \c 0: Disable LFN support. \c _MAX_LFN has no effect.
 * \c 1: Enable LFN with static working buffer on the BSS. Always thread-unsafe.
 * \c 2: Enable LFN with dynamic working buffer on the STACK.
 * \c 3: Enable LFN with dynamic working buffer on the HEAP.
 *
 * To enable LFN, the Unicode handling functions (<tt>option/unicode.c</tt>)
 * must be added to the project. The working buffer occupies
 * <tt>(_MAX_LFN + 1) * 2</tt> bytes, and 608 more bytes with exFAT enabled.
 * \c _MAX_LFN can be in the range from 12 to 255. It should be set to 255 to
 * support the full-featured LFN operations. When using the stack for the
 * working buffer, take care of stack overflow. When using the heap memory for
 * the working buffer, the memory management functions, \c ff_memalloc() and
 * \c ff_memfree(), must be added to the project.
 */
#define _USE_LFN        3
#endif
#ifndef _MAX_LFN
#define _MAX_LFN        255
#endif

#ifndef _LFN_UNICODE
/** This option switches the character encoding in the API
 * (\c 0: ANSI/OEM or \c 1: UTF-16).
 *
 * To use a Unicode string for the path name, enable LFN and set \c _LFN_UNICODE
 * to \c 1.
 * This option also affects the behavior of the string I/O functions.
 */
#define _LFN_UNICODE    0
#endif

#ifndef _STRF_ENCODE
/** If \c _LFN_UNICODE is set to \c 1, this option selects the character
 * encoding OF THE FILE to be read/written via the string I/O functions:
 * \c f_gets(), \c f_putc(), \c f_puts(), and \c f_printf().
 *
 * \c 0: ANSI/OEM
 * \c 1: UTF-16LE
 * \c 2: UTF-16BE
 * \c 3: UTF-8
 *
 * This option has no effect if \c _LFN_UNICODE is set to \c 0.
 */
#define _STRF_ENCODE    0
#endif

#ifndef _FS_RPATH
/** This option configures the support of relative path.
 *
 * \c 0: Disable relative path and remove related functions.
 * \c 1: Enable relative path. \c f_chdir() and \c f_chdrive() are available.
 * \c 2: \c f_getcwd() is available in addition to \c 1.
 */
#define _FS_RPATH       2
#endif

/** @} */
/*----------------------------------------------------------------------------*/
/** \name Drive/Volume Configuration
 * @{
 */

#ifndef _VOLUMES
/** Number of volumes (logical drives) to be used. */
#define _VOLUMES        1
#endif

#ifndef _STR_VOLUME_ID
/** \c _STR_VOLUME_ID switches the string support of volume ID.
 * If \c _STR_VOLUME_ID is set to \c 1, pre-defined strings can also be used as
 * drive number in the path name. \c _VOLUME_STRS defines the drive ID strings
 * for each logical drive. The number of items must be equal to \c _VOLUMES.
 * The valid characters for the drive ID strings are: A-Z and 0-9.
 */
#define _STR_VOLUME_ID  0
#endif
#ifndef _VOLUME_STRS
#define _VOLUME_STRS    "RAM","NAND","CF","SD","SD2","USB","USB2","USB3"
#endif

#ifndef _MULTI_PARTITION
/** This option switches support of multi-partition on a physical drive.
 * By default (0), each logical drive number is bound to the same physical drive
 * number and only an FAT volume found on the physical drive will be mounted.
 * When multi-partition is enabled (1), each logical drive number can be bound to
 * arbitrary physical drive and partition listed in the VolToPart[]. Also f_fdisk()
 * funciton will be available.
 */
#define _MULTI_PARTITION        0
#endif

#ifndef _MIN_SS
/** These options configure the range of sector size to be supported (512, 1024,
 * 2048, or 4096). Always set both to 512 for most systems, all types of memory
 * cards and harddisk. But a larger value may be required for on-board flash
 * memory and some types of optical media. When \c _MAX_SS is larger than
 * \c _MIN_SS, FatFs is configured to variable sector size and the
 * \c GET_SECTOR_SIZE command must be implemented in \c disk_ioctl().
 */
#define _MIN_SS         512
#endif
#ifndef _MAX_SS
#define _MAX_SS         512
#endif

#ifndef _USE_TRIM
/** This option switches the support of ATA-TRIM
 * (\c 0: disable or \c 1: enable).
 *
 * To enable the Trim function, the \c CTRL_TRIM command should also be
 * implemented in \c disk_ioctl().
 */
#define _USE_TRIM       0
#endif

#ifndef _FS_NOFSINFO
/** If you need to know the correct free space on the FAT32 volume, set the bit
 * 0 of this option, and the \c f_getfree() function will force a full FAT scan
 * on the first time after a volume mount. The bit 1 controls the use of the
 * last allocated cluster number.
 *
 * bit 0=0: Use the free cluster count in FSINFO if available.
 * bit 0=1: Do not trust the free cluster count in FSINFO.
 * bit 1=0: Use the last allocated cluster number in FSINFO if available.
 * bit 1=1: Do not trust the last allocated cluster number in FSINFO.
 */
#define _FS_NOFSINFO    3
#endif

/** @} */
/*----------------------------------------------------------------------------*/
/** \name System Configuration
 * @{
 */

#ifndef _FS_TINY
/** This option switches the tiny buffer configuration
 * (\c 0: normal or \c 1: tiny).
 *
 * With the tiny configuration, the size of a file object (FIL) is reduced to
 * \c _MAX_SS bytes. Instead of the private sector buffer eliminated from the
 * file object, a common sector buffer in the file system object (FATFS) is used
 * for the file data transfer.
 */
#define _FS_TINY        0
#endif

#ifndef _FS_EXFAT
/** This option switches the support of the exFAT file system
 * (\c 0: disable or \c 1: enable).
 *
 * With exFAT enabled, LFN also needs to be enabled (\c _USE_LFN >= 1).
 * Note that enabling exFAT discards C89 compatibility.
 */
#define _FS_EXFAT       1
#endif

#ifndef _FS_NORTC
/** The option \c _FS_NORTC switches the timestamp function. If the system does
 * not have any RTC function or if a valid timestamp is not needed, set
 * \c _FS_NORTC to \c 1 to disable the timestamp function. All the objects
 * modified by FatFs will have a fixed timestamp defined by \c _NORTC_MON,
 * \c _NORTC_MDAY, and \c _NORTC_YEAR in local time.
 * To enable the timestamp function (\c _FS_NORTC set to \c 0), \c get_fattime()
 * needs to be added to the project to get the current time from a real-time
 * clock. \c _NORTC_MON, \c _NORTC_MDAY, and \c _NORTC_YEAR have no effect.
 * These options have no effect with a read-only configuration (\c _FS_READONLY
 * set to \c 1).
 */
#define _FS_NORTC       (!RTC_CONF_INIT)
#endif
#ifndef _NORTC_MON
#define _NORTC_MON      1
#endif
#ifndef _NORTC_MDAY
#define _NORTC_MDAY     1
#endif
#ifndef _NORTC_YEAR
#define _NORTC_YEAR     2016
#endif

#ifndef _FS_LOCK
/** The option \c _FS_LOCK switches the file lock function controlling duplicate
 * file open and illegal operations on the open objects. This option must be set
 * to \c 0 if \c _FS_READONLY is \c 1.
 *
 * \c 0: Disable the file lock function. To avoid volume corruption, the
 *       application program should avoid illegal open, remove, and rename on
 *       the open objects.
 * \c >0: Enable the file lock function. The value defines how many
 *        files/sub-directories can be opened simultaneously under file lock
 *        control. Note that the file lock control is independent of
 *        re-entrancy.
 */
#define _FS_LOCK        0
#endif

#ifndef _FS_REENTRANT
/** The option \c _FS_REENTRANT switches the re-entrancy (thread-safe) of the
 * FatFs module itself. Note that, regardless of this option, file access to
 * different volumes is always re-entrant, and the volume control functions,
 * \c f_mount(), \c f_mkfs(), and \c f_fdisk(), are always non-re-entrant. Only
 * file/directory access to the same volume is under control of this function.
 *
 * \c 0: Disable re-entrancy. \c _FS_TIMEOUT and \c _SYNC_t have no effect.
 * \c 1: Enable re-entrancy. The user-provided synchronization handlers,
 *       \c ff_req_grant(), \c ff_rel_grant(), \c ff_del_syncobj(), and
 *       \c ff_cre_syncobj(), must also be added to the project. Samples are
 *       available in <tt>option/syscall.c</tt>.
 *
 * \c _FS_TIMEOUT defines the timeout period in unit of time tick.
 * \c _SYNC_t defines the OS-dependent sync object type, e.g. \c HANDLE, \c ID,
 * \c OS_EVENT*, \c SemaphoreHandle_t, etc. A header file for the OS definitions
 * needs to be included somewhere in the scope of <tt>ff.h</tt>.
 */
#define _FS_REENTRANT   0
#endif
#ifndef _FS_TIMEOUT
#define _FS_TIMEOUT     1000
#endif
#ifndef _SYNC_t
#define _SYNC_t         HANDLE
#endif

/** @} */
/*----------------------------------------------------------------------------*/

#endif /* FFCONF_H_ */

/**
 * @}
 * @}
 */
