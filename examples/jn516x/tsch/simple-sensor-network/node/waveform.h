/*
* Copyright (c) 2015 NXP B.V.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. Neither the name of NXP B.V. nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*
* This file is part of the Contiki operating system.
*
* Author: Theo van Daele <theo.van.daele@nxp.com>
*
*/


/* This file contains arrays with waveforms.
   The arrays are of type int8. No limit on size, but for convenience of demonstrating take care that sample
   period may be 10secs or more. Size for all tables should be equal !! */

#define SIZE_OF_WAVEFORM  40

static const int8 sin_table[] = {
0,
19,
39,
57,
74,
89,
102,
113,
120,
125,
127,
125,
120,
113,
102,
89,
74,
57,
39,
19,
0,
-20,
-40,
-58,
-75,
-90,
-103,
-114,
-121,
-126,
-127,
-126,
-121,
-114,
-103,
-90,
-75,
-58,
-40,
-20
};
   
static const int8 triangle_table[] = {
-127,
-114,
-101,
-87,
-74,
-61,
-47,
-34,
-21,
-7,
6,
20,
33,
46,
60,
73,
86,
100,
113,
127,
127,
113,
100,
86,
73,
60,
46,
33,
20,
6,
-7,
-21,
-34,
-47,
-61,
-74,
-87,
-101,
-114,
-127
};

static const int8 pos_sawtooth_table[] = {
-127,
-121,
-115,
-108,
-102,
-96,
-89,
-83,
-76,
-70,
-64,
-57,
-51,
-45,
-38,
-32,
-25,
-19,
-13,
-6,
0,
6,
13,
19,
26,
32,
38,
45,
51,
57,
64,
70,
77,
83,
89,
96,
102,
108,
115,
121
};
    
static const int8 neg_sawtooth_table[] = {
127,
120,
114,
107,
101,
95,
88,
82,
76,
69,
63,
56,
50,
44,
37,
31,
25,
18,
12,
5,
-1,
-7,
-14,
-20,
-26,
-33,
-39,
-46,
-52,
-58,
-65,
-71,
-77,
-84,
-90,
-97,
-103,
-109,
-116,
-122  
};


