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

#ifndef  FIRMWARE_H_INCLUDED 
#define  FIRMWARE_H_INCLUDED 

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdint.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/


typedef enum
{
    E_FW_OK,
    E_FW_ERROR,
    E_FW_INVALID_FILE,
} teFWStatus;


/** Abstracted firmware information common across bootloaders. */
typedef struct
{
    uint32_t    u32ROMVersion;                  /**< ROM Version that the FW was built for */
    uint32_t    u32TextSectionLoadAddress;      /**< Address to load .text section */
    uint32_t    u32TextSectionLength;           /**< Length (bytes) of .text section */
    uint32_t    u32BssSectionLoadAddress;       /**< Address of start of .bss section */
    uint32_t    u32BssSectionLength;            /**< Length (bytes) of .bss section */
    uint32_t    u32WakeUpEntryPoint;            /**< Address of wake up (warm start) entry point */
    uint32_t    u32ResetEntryPoint;             /**< Address of rest (cold start) entry point */
    
    uint8_t*    pu8ImageData;                   /**< Pointer to loaded image data for Flash */
    uint32_t    u32ImageLength;                 /**< Length (bytes) of image for Flash */
    
    uint32_t    u32MacAddressLocation;          /**< Offset in file of MAC address */
    uint8_t*    pu8TextData;                    /**< Pointer to loaded .text section for RAM */
} tsFW_Info;


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


/** Open a firmware file.
 *  \param[out]     psFW_Info Pointer to location to store firmware file infrmation
 *  \param[in]      pcFirmwareFile Filename to open
 *  \return E_FW_OK on success
 */
teFWStatus iFW_Open(tsFW_Info *psFW_Info, char *pcFirmwareFile);


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* FIRMWARE_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/



