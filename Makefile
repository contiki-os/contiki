#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

# Deal with colliding definitions from tcsh etc.
VENDOR=

#########################################################################

TOPDIR	:= $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
export	TOPDIR
ARCH  = arm
CPU   = arm7tdmi-s
export	ARCH CPU VENDOR

#CROSS_COMPILE = arm-unknown-linux-gnu-
#CROSS_COMPILE = arm-softfloat-linux-gnu-
#export	CROSS_COMPILE

# load other configuration
include $(TOPDIR)/config.mk

#########################################################################
# blink objects....order is important (i.e. start must be first)

AOBJS =
COBJS = $(patsubst %.c,%.o,$(wildcard src/*.c))
TESTS = $(wildcard tests/*.c)
TARGETS = $(patsubst %.c,%.o,$(TESTS))
#TARGETS = tests/blink-white.o 

# Add GCC lib
PLATFORM_LIBS += --no-warn-mismatch -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc

#########################################################################

#ALL = blink.srec blink.bin blink.dis blink.System.map
ALL = $(TESTS:.c=.srec) $(TESTS:.c=.bin) $(TESTS:.c=.dis) 

.PRECIOUS: 	$(COBJS) $(TARGETS) $(TESTS:.c=.obj)

all:		src/start.o src/isr.o $(ALL)

tests/flasher.obj: src/maca.o src/nvm.o
tests/nvm-read.obj: src/maca.o src/nvm.o
tests/nvm-write.obj: src/maca.o src/nvm.o
tests/rftest-rx.obj: src/maca.o src/nvm.o
tests/rftest-tx.obj: src/maca.o src/nvm.o
tests/tmr-ints.obj: src/isr.o
tests/sleep.obj: src/isr.o src/maca.o src/nvm.o

NOTHUMB_CPPFLAGS := $(DBGFLAGS) $(OPTFLAGS) $(RELFLAGS)         \
        -D__KERNEL__ -DTEXT_BASE=$(TEXT_BASE)           \
        -I$(TOPDIR)/include                             \
        -fno-builtin -ffreestanding -nostdinc -isystem  \
        $(gccincdir) -pipe 
NOTHUMB_CPPFLAGS_EXTRA = -march=armv4t -mlong-calls -mtune=arm7tdmi-s -DCONFIG_ARM -D__ARM__ -mthumb-interwork 


src/isr.o: src/isr.c
		$(CC) $(NOTHUMB_CPPFLAGS) $(NOTHUMB_CPPFLAGS_EXTRA) -c -o $@ $<

%.srec:		%.obj
		$(OBJCOPY) ${OBJCFLAGS} -O srec $< $@

%.ihex: 	%.obj
		$(OBJCOPY) ${OBJCFLAGS} -O ihex $< $@

%.bin:		%.obj
		$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

%.dis:		%.obj
		$(OBJDUMP) -SD $< > $@

%.obj:		$(LDSCRIPT) %.o src/isr.o
		$(LD) $(LDFLAGS) $(AOBJS) \
			--start-group $(PLATFORM_LIBS) --end-group \
			-Map $*.map $^ -o $@


%.System.map:	%.obj
		@$(NM) $< | \
		grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
		sort > $*.System.map


#########################################################################

.depend:	Makefile $(AOBJS:.o=.S) $(COBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(AOBJS:.o=.S) $(COBJS:.o=.c) > $@

sinclude .depend

#########################################################################

clean:
	find . -type f \
		\( -name 'core' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o'  -o -name '*.a' \) -print \
		| xargs rm -f

clobber:	clean
	find . -type f \
		\( -name .depend -o -name '*.srec' -o -name '*.bin' -o -name '*.dis' -o -name '*.map' -o -name '*.obj' \) \
		-print \
		| xargs rm -f
	rm -f $(OBJS) *.bak tags TAGS
	rm -fr *.*~
	rm -f $(ALL)

mrproper \
distclean:	clobber

backup:
	F=`basename $(TOPDIR)` ; cd .. ; \
	tar --force-local -zcvf `date "+$$F-%Y-%m-%d-%T.tar.gz"` $$F

#########################################################################
