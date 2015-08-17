/****************************************************************************
 *
 * MODULE:             Firmware
 *
 * COMPONENT:          $RCSfile: Firmware.c,v $
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.2 $
 *
 * DATED:              $Date: 2009/03/02 13:33:44 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Matt Redfearn
 *
 * DESCRIPTION:
 *
 *
 * LAST MODIFIED BY:   $Author: lmitch $
 *                     $Modtime: $
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5148, JN5142, JN5139]. 
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the 
 * software.
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

 * Copyright NXP B.V. 2012. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdio.h>
#include <endian.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "Firmware.h"
#include "JN51xx_BootLoader.h"
#include "dbg.h"


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef DEBUG_FIRMWARE
#define TRACE_FIRMWARE      TRUE
#else
#define TRACE_FIRMWARE      FALSE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/


typedef struct
{
    uint8_t     u8ConfigByte0;
    uint8_t     u8ConfigByte1;
    uint16_t    u16SpiScrambleIndex;
    uint32_t    u32TextStartAddress;
    uint32_t    u32TextLength;
    uint32_t    u32ROMVersion;
    uint32_t    u32Unused1;
    uint32_t    u32BssStartAddress;
    uint32_t    u32BssLength;
    uint32_t    u32WakeUpEntryPoint;
    uint32_t    u32ResetEntryPoint;
    uint8_t     au8OadData[12];
    uint8_t     u8TextDataStart;
} __attribute__ ((packed)) tsBL_BinHeaderV1;


typedef struct
{
    uint32_t    u32ROMVersion;
    uint32_t    au32BootImageRecord[4];
    uint64_t    u64MacAddress;
    uint32_t    au32EncryptionInitialisationVector[4];
    uint32_t    u32DataSectionInfo;
    uint32_t    u32BssSectionInfo;
    uint32_t    u32WakeUpEntryPoint;
    uint32_t    u32ResetEntryPoint;
    uint8_t     u8TextDataStart;
} __attribute__ ((packed)) tsBL_BinHeaderV2;


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/** Get information from firmare.
 *  \param[out]     ppu8Firmware Pointer to location to store mapped file.
 *  \param[in]      pcFirmwareFile Filename to open
 *  \return E_FW_OK on success
 */
static teFWStatus iFW_GetInfo(uint8_t *pu8Firmware, tsFW_Info *psFW_Info);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

extern int iVerbosity;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

static int firmware_fd = 0;
static uint8_t* pu8Firmware = NULL;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


/****************************************************************************
 *
 * NAME: iFW_Open
 *
 * DESCRIPTION:
 *  Reads required information from the binary image
 *
 * PARAMETERS:  Name            RW  Usage
 *              pu8Firmware     R   Pointer to firmware image to analyse
 *              psFW_Info       W   Pointer to Info structure to be filled in
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occured
 *
 ****************************************************************************/
teFWStatus iFW_Open(tsFW_Info *psFW_Info, char *pcFirmwareFile)
{
    struct stat sb;
    
    firmware_fd = open(pcFirmwareFile, O_RDONLY);
    if (firmware_fd < 0)
    {
        fprintf(stderr, "Could not open firmware file\n");
        return E_FW_INVALID_FILE;
    }
    
    if (fstat(firmware_fd, &sb) == -1)           /* To obtain file size */
    {
        perror("Could not stat file");
        return -1;
    }
    
    psFW_Info->u32ImageLength = (uint32_t)sb.st_size;
    
    /* Copy-on-write, changes are not written to the underlying file. */
    pu8Firmware = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, firmware_fd, 0);
    
    if (pu8Firmware == MAP_FAILED)
    {
        perror("Could not mmap file");
        return E_FW_ERROR;
    }

    if (iVerbosity > 2)
    {
        printf("Opened firmware file size %d\n", (int)sb.st_size);
    }
    
    return iFW_GetInfo(pu8Firmware, psFW_Info);
}



/****************************************************************************
 *
 * NAME: iFW_GetInfo
 *
 * DESCRIPTION:
 *  Reads required information from the binary image
 *
 * PARAMETERS:  Name            RW  Usage
 *              pu8Firmware     R   Pointer to firmware image to analyse
 *              psFW_Info       W   Pointer to Info structure to be filled in
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occured
 *
 ****************************************************************************/
teFWStatus iFW_GetInfo(uint8_t *pu8Firmware, tsFW_Info *psFW_Info)
{
    tsBL_BinHeaderV2 *psHeader              = (tsBL_BinHeaderV2 *)pu8Firmware;

    // JN5148-J01 onwards uses multiimage bootloader - check for it's magic number.   
    if ((ntohl(psHeader->au32BootImageRecord[0]) == 0x12345678) &&
        (ntohl(psHeader->au32BootImageRecord[1]) == 0x11223344) &&
        (ntohl(psHeader->au32BootImageRecord[2]) == 0x55667788))
    {
        if (iVerbosity > 2)
        {
            printf("Module uses Bootloader v2 Header\n");
        }
        
        psFW_Info->u32ROMVersion                = ntohl(psHeader->u32ROMVersion);
        
        psFW_Info->u32TextSectionLoadAddress    = 0x04000000 + (((ntohl(psHeader->u32DataSectionInfo)) >> 16) * 4);
        psFW_Info->u32TextSectionLength         = (((ntohl(psHeader->u32DataSectionInfo)) & 0x0000FFFF) * 4);
        psFW_Info->u32BssSectionLoadAddress     = 0x04000000 + (((ntohl(psHeader->u32BssSectionInfo)) >> 16) * 4);
        psFW_Info->u32BssSectionLength          = (((ntohl(psHeader->u32BssSectionInfo)) & 0x0000FFFF) * 4);

        psFW_Info->u32ResetEntryPoint           = psHeader->u32ResetEntryPoint;
        psFW_Info->u32WakeUpEntryPoint          = psHeader->u32WakeUpEntryPoint;
        
        /* Pointer to and length of image for flash */
        psFW_Info->pu8ImageData                 = (uint8_t*)&(psHeader->au32BootImageRecord[0]);
        
        psFW_Info->u32MacAddressLocation        = 0x10;
        
        /* Pointer to text section in image for RAM */
        psFW_Info->pu8TextData                  = &(psHeader->u8TextDataStart);
    }
    else
    {
        tsBL_BinHeaderV1 *psHeader              = (tsBL_BinHeaderV1 *)pu8Firmware;
        if (iVerbosity > 2)
        {
            printf("Module uses Bootloader v1 Header\n");
        }

        psFW_Info->u32ROMVersion                = ntohl(psHeader->u32ROMVersion);
        
        psFW_Info->u32TextSectionLoadAddress    = ntohl(psHeader->u32TextStartAddress);
        psFW_Info->u32TextSectionLength         = ntohl(psHeader->u32TextLength);
        psFW_Info->u32BssSectionLoadAddress     = ntohl(psHeader->u32BssStartAddress);
        psFW_Info->u32BssSectionLength          = ntohl(psHeader->u32BssLength);
        
        psFW_Info->u32ResetEntryPoint           = psHeader->u32ResetEntryPoint;
        psFW_Info->u32WakeUpEntryPoint          = psHeader->u32WakeUpEntryPoint;
        
        /* Pointer to and length of image for flash */
        psFW_Info->pu8ImageData                 = &(psHeader->u8ConfigByte0);
        //psFW_Info->u32ImageLength               = sizeof(tsBL_BinHeaderV1) + psFW_Info->u32TextSectionLength;
        
        psFW_Info->u32MacAddressLocation        = 0x30;
        
        /* Pointer to text section in image for RAM */
        psFW_Info->pu8TextData                  = &(psHeader->u8TextDataStart);
        
    }
    
    DBG_vPrintf(TRACE_FIRMWARE, "u32ROMVersion:         0x%08x\n", psFW_Info->u32ROMVersion);
    DBG_vPrintf(TRACE_FIRMWARE, "u32DataSectionInfo:    0x%08x\n", ntohl(psHeader->u32DataSectionInfo));
    DBG_vPrintf(TRACE_FIRMWARE, "u32TextSectionLength:  0x%08x\n", (((ntohl(psHeader->u32DataSectionInfo)) & 0x0000FFFF) * 4));

    return E_FW_OK;
}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

