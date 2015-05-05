/* Copyright (c) 2009  ARAGO SYSTEMS
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/
/*
*/


/**
      SHT15/75 Driver

!!!   be advise that the SHT15 and SHT75 are not i2C compliant sensors
      they are just designed to not disturb i2C devices on a 2 wire bus
      this driver allow to drive the sensor with GPIO and delay
*/
#include "contiki.h"

/***********************************************************************************
*   SHT15 functions
*/
extern float restemp, truehumid;



void comstart(void);
int comwrite(char iobyte);
int comread(void);
void comwait(void);
void comreset(void);
void sht_soft_reset(void);
int measuretemp(void);
int measurehumid(void);
void calculate_data(int temp, int humid, float *tc, float *rhlin, float *rhtrue);
void sht_rd(float *temp, float *truehumid);
void sht_init(void);
void Set_DataOutToSTH75_Hi(void);
void Set_DataOutToSTH75_Lo(void);