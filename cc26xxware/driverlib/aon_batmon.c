/******************************************************************************
*  Filename:       aon_batmon.c
*  Revised:        2015-01-15 11:07:05 +0100 (to, 15 jan 2015)
*  Revision:       42389
*
*  Description:    Driver for the AON Battery and Temperature Monitor
*
*  Copyright (c) 2015, Texas Instruments Incorporated
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
*  1) Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2) Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*
*  3) Neither the name of the ORGANIZATION nor the names of its contributors may
*     be used to endorse or promote products derived from this software without
*     specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
*  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#include <driverlib/aon_batmon.h>
#include <inc/hw_fcfg1.h>

//*****************************************************************************
// Need to know the AON_BATMON:TEMP.INT fild width in order to sign extend correctly
// (This is not given in the hw_aon_batmon.h file and therefore hard coded here)
//*****************************************************************************
#define AON_BATMON_TEMP_INT_FIELD_WIDTH   9

//*****************************************************************************
//
// AON_BatmonTempGetDegC()
// Returns sign exteded temperature in Deg C (-256 .. +255)
//
//*****************************************************************************
int32_t
AON_BatmonTempGetDegC( void )
{
   int32_t  signedTemp        ; // Signed extended temperature with 8 fractional bits
   int32_t  tempCorrection    ; // Voltage dependent temp correction with 8 fractional bits
   int8_t   voltageSlope      ; // Signed byte value representing the TEMP slope with battery voltage, in degrees C/V, with 4 fractional bits.

   //
   // Shift left then right to sign extend the BATMON_TEMP field
   //
   signedTemp = ((((int32_t)HWREG( AON_BATMON_BASE + AON_BATMON_O_TEMP ))
     << ( 32 - AON_BATMON_TEMP_INT_FIELD_WIDTH - AON_BATMON_TEMP_INT_S ))
     >> ( 32 - AON_BATMON_TEMP_INT_FIELD_WIDTH - AON_BATMON_TEMP_INT_S ));

   //
   // Typecasting voltageSlope to int8_t prior to assignment in order to make sure sign extension works properly
   // Using byte read (HWREGB) in order to make more efficient code since FCFG1_O_MISC_TRIM is assigned to bits[7:0]
   //
   voltageSlope      = ((int8_t)HWREGB( FCFG1_BASE + FCFG1_O_MISC_TRIM ));
   tempCorrection    = (( voltageSlope * (((int32_t)HWREG( AON_BATMON_BASE + AON_BATMON_O_BAT )) - 0x300 )) >> 4 );

   return ((( signedTemp - tempCorrection ) + 0x80 ) >> 8 );
}


// See aon_batmon.h for implementation of remaining functions
