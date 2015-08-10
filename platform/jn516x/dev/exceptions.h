/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Exception handlers
 *
 * VERSION:            $Name$
 *
 * REVISION:           $Revision: 11579 $
 *
 * DATED:              $Date: 2009-03-24 08:28:35 +0000 (Tue, 24 Mar 2009) $
 *
 * STATUS:             $State$
 *
 * AUTHOR:             Thomas Haydon
 *
 * DESCRIPTION:
 * Exception handlers
 *
 * CHANGE HISTORY:
 *
 * LAST MODIFIED BY:   $Author: thayd $
 *                     $Modtime: $
 *
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
 *
 * Copyright NXP B.V. 2012. All rights reserved
 *
 ***************************************************************************/

/* Integrated into Contiki by Beshr Al Nahas */

#ifndef  EXCEPTIONS_H_INCLUDED
#define  EXCEPTIONS_H_INCLUDED

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/** Enumerated type of CPU exception numbers */
typedef enum {
	E_EXC_BUS_ERROR				= 0x02,
	E_EXC_TICK_TIMER			= 0x05,
	E_EXC_UNALIGNED_ACCESS		= 0x06,
	E_EXC_ILLEGAL_INSTRUCTION	= 0x07,
	E_EXC_EXTERNAL_INTERRUPT	= 0x08,
	E_EXC_SYSCALL				= 0x0C,
	E_EXC_TRAP					= 0x0E,
	E_EXC_GENERIC				= 0x0F,
	E_EXC_STACK_OVERFLOW		= 0x10
} eExceptionType;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/* Exceptions set up function */
PUBLIC void vEXC_Register(void);
/* For debugging */
void debug_file_line(const char *file, int line);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#endif  /* EXCEPTIONS_H */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

