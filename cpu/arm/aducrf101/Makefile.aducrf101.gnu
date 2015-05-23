# -*- makefile -*-

# Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted (subject to the limitations in the
# disclaimer below) provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the
#   distribution.
#
# - Neither the name of Analog Devices, Inc. nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
# GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
# HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Author: Jim Paris <jim.paris@rigado.com>

CROSS_COMPILE = arm-none-eabi-

CC      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)gcc
AS      = $(CROSS_COMPILE)gcc
AR      = $(CROSS_COMPILE)gcc-ar
NM      = $(CROSS_COMPILE)gcc-nm
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
STRIP   = $(CROSS_COMPILE)strip

CFLAGS_OPT ?= -Os
CFLAGS_DEBUG ?= -ggdb3 -fomit-frame-pointer
CFLAGS += $(CFLAGS_OPT) $(CFLAGS_DEBUG)
CFLAGS += -std=gnu99
CFLAGS += -ffreestanding -mcpu=cortex-m3 -mthumb -mno-thumb-interwork
CFLAGS += -ffunction-sections -fdata-sections -fno-common -fno-builtin
CFLAGS += -flto

ifdef WERROR
  CFLAGS += -Wall -Werror
  # These warnings are triggered by existing Contiki code
  CFLAGS += -Wno-error=pointer-sign
  CFLAGS += -Wno-error=char-subscripts
  CFLAGS += -Wno-error=unused-variable
  CFLAGS += -Wno-error=unused-but-set-variable
endif

# UIP code does not follow C aliasing rules
CFLAGS += -fno-strict-aliasing

LDFLAGS  = $(CFLAGS)
LDFLAGS += -specs=nosys.specs -nostartfiles

# TODO: When it becomes more commonly available, switch to newlib-nano
# for significant size reduction, by uncommenting this:
# LDFLAGS += -specs=nano.specs

LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-T$(CONTIKI_CPU)/Common/GCC/ADuCRF101.ld

ASFLAGS += -c $(CFLAGS)

# Compiler-specific startup code
CONTIKI_CPU_DIRS += Common/GCC
CONTIKI_SOURCEFILES += crt0.S

# Rules
%.hex: %
	$(OBJCOPY) -O ihex $^ $@