/******************************************************************************
*  Filename:       obs.h
*  Revised:        2015-01-13 16:59:55 +0100 (ti, 13 jan 2015)
*  Revision:       42365
*
*  This file holds defines for possible values for the registers:
*      OBS
*      OBS_MODULEBUS
*
*  Defines based on values in .xls in CC26xx/Architecture/si/integration notes
*
*  The value configured in OBS is sent towards the subdomain defined in
*  OBSERVEDOMAIN.
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
#define IOC_OBS_AONSS_SYSCTRL_0 0x00000001
#define IOC_OBS_AONSS_SYSCTRL_1 0x00000003
#define IOC_OBS_AONSS_SYSCTRL_2 0x00000005
#define IOC_OBS_AONSS_SYSCTRL_3 0x00000007
#define IOC_OBS_AONSS_SYSCTRL_4 0x00000009
#define IOC_OBS_AONSS_SYSCTRL_5 0x0000000b
#define IOC_OBS_AONSS_SYSCTRL_6 0x0000000d
#define IOC_OBS_AONSS_SYSCTRL_7 0x0000000f
#define IOC_OBS_AONSS_SYSCTRL_8 0x00000011
#define IOC_OBS_AONSS_SYSCTRL_9 0x00000013
#define IOC_OBS_AONSS_WUC_0     0x00000021
#define IOC_OBS_AONSS_WUC_1     0x00000023
#define IOC_OBS_AONSS_WUC_2     0x00000025
#define IOC_OBS_AONSS_WUC_3     0x00000027
#define IOC_OBS_AONSS_WUC_4     0x00000029
#define IOC_OBS_AONSS_WUC_5     0x0000002b
#define IOC_OBS_AONSS_WUC_6     0x0000002d
#define IOC_OBS_AONSS_WUC_7     0x0000002f
#define IOC_OBS_AONSS_WUC_8     0x00000031
#define IOC_OBS_AONSS_WUC_9     0x00000033
#define IOC_OBS_AONSS_WUC_10    0x00000035
#define IOC_OBS_AONSS_WUC_11    0x00000037
#define IOC_OBS_AONSS_WUC_12    0x00000039
#define IOC_OBS_AONSS_BATMON_0  0x00000041
#define IOC_OBS_AONSS_BATMON_1  0x00000043
#define IOC_OBS_AONSS_BATMON_2  0x00000045
#define IOC_OBS_AONSS_BATMON_3  0x00000047
#define IOC_OBS_AONSS_BATMON_4  0x00000049
#define IOC_OBS_AONSS_BATMON_5  0x0000004b
#define IOC_OBS_AONSS_BATMON_6  0x0000004d
#define IOC_OBS_AONSS_BATMON_7  0x0000004f
#define IOC_OBS_AONSS_BATMON_8  0x00000051
#define IOC_OBS_AONSS_BATMON_9  0x00000053
#define IOC_OBS_AONSS_BATMON_10 0x00000055
#define IOC_OBS_AONSS_BATMON_11 0x00000057
#define IOC_OBS_AONSS_BATMON_12 0x00000059
#define IOC_OBS_AONSS_BATMON_13 0x0000005b
#define IOC_OBS_AONSS_BATMON_14 0x0000005d
#define IOC_OBS_AONSS_BATMON_15 0x0000005f
#define IOC_OBS_AONSS_IOC_0     0x00000071
#define IOC_OBS_AONSS_IOC_1     0x00000073
#define IOC_OBS_AONSS_IOC_2     0x00000075
#define IOC_OBS_AONSS_IOC_3     0x00000077
#define IOC_OBS_AONSS_IOC_4     0x00000079
#define IOC_OBS_AONSS_EVENT_0   0x00000081
#define IOC_OBS_AONSS_EVENT_1   0x00000083
#define IOC_OBS_AONSS_EVENT_2   0x00000085
#define IOC_OBS_AONSS_EVENT_3   0x00000087
#define IOC_OBS_AONSS_EVENT_4   0x00000089
#define IOC_OBS_AONSS_EVENT_5   0x0000008b
#define IOC_OBS_AONSS_EVENT_6   0x0000008d
#define IOC_OBS_AONSS_EVENT_7   0x0000008f
#define IOC_OBS_AONSS_EVENT_8   0x00000091
#define IOC_OBS_AONSS_EVENT_9   0x00000093
#define IOC_OBS_AONSS_EVENT_10  0x00000095
#define IOC_OBS_AONSS_RTC_0     0x000000a1
#define IOC_OBS_AONSS_RTC_1     0x000000a3
#define IOC_OBS_AONSS_RTC_2     0x000000a5
#define IOC_OBS_AONSS_RTC_3     0x000000a7
#define IOC_OBS_AONSS_RTC_4     0x000000a9
#define IOC_OBS_AONSS_RTC_5     0x000000ab
#define IOC_OBS_AONSS_RTC_6     0x000000ad
#define IOC_OBS_AONSS_RTC_7     0x000000af
#define IOC_OBS_AONSS_SPIS_0    0x000000c1
#define IOC_OBS_AONSS_SPIS_1    0x000000c3
#define IOC_OBS_AONSS_SPIS_2    0x000000c5
#define IOC_OBS_AONSS_SPIS_3    0x000000c7

#define IOC_OBS_AUXSS_0         0x00000003
#define IOC_OBS_AUXSS_1         0x00000005
#define IOC_OBS_AUXSS_2         0x00000007

#define IOC_OBS_DEBUGSS_0       0x00000001
#define IOC_OBS_DEBUGSS_1       0x00000003

#define IOC_OBS_ANAOSC_0        0x00000001
#define IOC_OBS_ANAOSC_1        0x00000002
#define IOC_OBS_ANAOSC_2        0x00000003
#define IOC_OBS_ANAOSC_3        0x00000004
#define IOC_OBS_ANAOSC_4        0x00000005
#define IOC_OBS_ANAOSC_5        0x00000006
#define IOC_OBS_ANAOSC_6        0x00000007
#define IOC_OBS_ANAOSC_7        0x00000008
#define IOC_OBS_ANAOSC_8        0x00000009
#define IOC_OBS_ANAOSC_9        0x0000000a
#define IOC_OBS_ANAOSC_10       0x0000000b
#define IOC_OBS_ANAOSC_11       0x0000000c
#define IOC_OBS_ANAOSC_12       0x0000000d
#define IOC_OBS_ANAOSC_13       0x0000000e
#define IOC_OBS_ANAOSC_14       0x0000000f

#define IOC_OBS_ANADLO_0        0x00000001
#define IOC_OBS_ANADLO_1        0x00000002
#define IOC_OBS_ANADLO_2        0x00000003
#define IOC_OBS_ANADLO_3        0x00000004
#define IOC_OBS_ANADLO_4        0x00000005
#define IOC_OBS_ANADLO_5        0x00000006
#define IOC_OBS_ANADLO_6        0x00000007
#define IOC_OBS_ANADLO_7        0x00000008
#define IOC_OBS_ANADLO_8        0x00000009
#define IOC_OBS_ANADLO_9        0x0000000a
#define IOC_OBS_ANADLO_10       0x0000000b
#define IOC_OBS_ANADLO_11       0x0000000c
#define IOC_OBS_ANADLO_12       0x0000000d
#define IOC_OBS_ANADLO_13       0x0000000e
#define IOC_OBS_ANADLO_14       0x0000000f

#define IOC_OBS_MODULEBUS_0     0x00000000
#define IOC_OBS_MODULEBUS_1     0x00000001
#define IOC_OBS_MODULEBUS_2     0x00000002
#define IOC_OBS_MODULEBUS_3     0x00000003
#define IOC_OBS_MODULEBUS_4     0x00000004
#define IOC_OBS_MODULEBUS_5     0x00000005
#define IOC_OBS_MODULEBUS_6     0x00000006
#define IOC_OBS_MODULEBUS_7     0x00000007
#define IOC_OBS_MODULEBUS_8     0x00000008
#define IOC_OBS_MODULEBUS_9     0x00000009
