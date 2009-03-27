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
COBJS = src/blink.o 

# Add GCC lib
PLATFORM_LIBS += --no-warn-mismatch -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc

#########################################################################

ALL = blink.dis blink.srec blink.bin System.map

all:		$(ALL)

blink.srec:	blink
		$(OBJCOPY) ${OBJCFLAGS} -O srec $< $@

blink.ihex:	blink
		$(OBJCOPY) ${OBJCFLAGS} -O ihex $< $@

blink.bin:	blink
		$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

blink.dis:	blink
		$(OBJDUMP) -DS $< > $@

blink:		$(AOBJS) $(COBJS) $(LDSCRIPT)
		$(LD) $(LDFLAGS) $(AOBJS) $(COBJS) \
			--start-group $(PLATFORM_LIBS) --end-group \
			-Map blink.map -o blink

System.map:	blink
		@$(NM) $< | \
		grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
		sort > System.map


#########################################################################

.depend:	Makefile $(AOBJS:.o=.S) $(COBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(AOBJS:.o=.S) $(COBJS:.o=.c) > $@

sinclude .depend

#########################################################################

clean:
	find . -type f \
		\( -name 'core' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o'  -o -name '*.a'  \) -print \
		| xargs rm -f

clobber:	clean
	find . -type f \
		\( -name .depend -o -name '*.srec' -o -name '*.bin' \) \
		-print \
		| xargs rm -f
	rm -f $(OBJS) *.bak tags TAGS
	rm -fr *.*~
	rm -f blink blink.map $(ALL)

mrproper \
distclean:	clobber

backup:
	F=`basename $(TOPDIR)` ; cd .. ; \
	tar --force-local -zcvf `date "+$$F-%Y-%m-%d-%T.tar.gz"` $$F

#########################################################################
