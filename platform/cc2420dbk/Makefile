# $Id: Makefile,v 1.7 2007/05/21 14:30:52 bg- Exp $
#
# This makefile requires GNU make!

LIB = libcontiki.a
KERNELS = gateway.ihex
PROGS = #loadable_prg.ko udpsend.ko udprecv.ko
TOOLS = tunslip scat codeprop

all: $(LIB) $(KERNELS) $(PROGS) $(TOOLS)

loadable_prg.ko: loadable_prg.o
udpsend.ko: udpsend.o
udprecv.ko: udprecv.o
test.ko: test.o

gateway.out: gateway.o $(LIB)
client.out: client.o $(LIB)
dhclient.out: dhclient.o $(LIB)

TOP=../..
INCDIRS = -I. -I$(TOP)/cpu/avr -I$(TOP)/core -I$(TOP)/apps
SRCDIRS = dev $(TOP)/cpu/avr $(TOP)/cpu/avr/dev ${addprefix $(TOP)/core/,sys dev net lib loader} $(TOP)/apps/codeprop

MCU=atmega128

ARCH=avr.o spi.o qleds.o rom.o xmem.o cle_avr.o
SYSTEM=process.o procinit.o service.o clock.o etimer.o timer.o \
	sym.o cle.o elfloader_compat.o
UIP=uip.o uiplib.o tcpip.o uip-fw.o uip-fw-drv.o uipbuf.o \
    tcpdump.o psock.o dhcpc.o uaodv.o uaodv-rt.o uip-udp-packet.o
UIPDRIVERS=slip.o slip_uart1.o \
           cc2420.o cc2420_spi.o cc2420_send_ip.o cc2420_send_uaodv.o 
LIBS=memb.o list.o rand.o assert.o crtk.o
SYSAPPS=tcp_loader.o
LIBFILES=$(SYSTEM) $(ARCH) $(UIP) $(UIPDRIVERS) $(LIBS) $(SYSAPPS)

MKNMLIST=awk -f $(TOP)/tools/mknmlist-ansi
CP=cp
LIBC=-lgcc -lc -lgcc
CC=avr-gcc
LD=avr-ld
AS=avr-as
NM=avr-nm
OBJCOPY=avr-objcopy
STRIP=avr-strip
AR=avr-ar
RANLIB=avr-ranlib
BSL=avrdude
CFLAGS=-mmcu=$(MCU) $(INCDIRS) -Wall -Os
LDFLAGS=-mmcu=$(MCU) -Wl,--section-start=.bootloader=$(BOOTLOADER_START)

# Check fuse high byte, bits BOOTSZ0 and BOOTSZ1.
BOOTLOADER_START=0x1fc00

# Setup directory search path for source files
vpath %.c $(SRCDIRS) ${filter-out CVS,${wildcard labs/*}}

# Set COM port if not already set.
# DOS: 1 => COM2, 2 => COM3, etc.
# Linux: /dev/ttyUSB0, /dev/ttyUSB1, etc.
# FreeBSD 6.2: /dev/cuaU0, /dev/cuaU1, etc.
ifndef COMPORT
  COMPORT := /dev/ttyUSB0
endif


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $(notdir $(<:.c=.o))

# Combine several .o:s to one .ko and put COMMONs into the BSS segment
%.ko:
	$(LD) -r -dc --strip-all --unresolved-symbols=ignore-in-object-files $^ -o $@

%.ihex: %.out
	$(OBJCOPY) $^ -O ihex $@

#%.out:
#	$(if $^,$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^,$(error "NO TARGET $@"))

# Add a namelist to the kernel
%.out: $^
	: | $(MKNMLIST) > $@_tmp.c && mv $@_tmp.c $@_nm.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBC) $@_nm.c
	$(NM) $@ | $(MKNMLIST) > $@_tmp.c && mv $@_tmp.c $@_nm.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBC) $@_nm.c

%.u: %.ihex
	$(BSL) -p $(MCU) -P $(COMPORT) -c stk500 -U flash:w:$<

$(LIB): $(LIBFILES)
	$(AR) rcf $@ $^
	$(RANLIB) $@

codeprop: $(TOP)/tools/codeprop.c
	cc -g -Wall $< -o $@

tunslip: $(TOP)/tools/tunslip.c
	cc -g -Wall -DBAUDRATE="B38400" $< -o $@

scat: $(TOP)/tools/scat.c
	cc -g -Wall -DBAUDRATE="B38400" $< -o $@

### TEST ###

test.out: test.o $(LIB)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

depend: $(LIBFILES:.o=.c)
	-$(CC) $(CCDEPFLAGS) -MM $(CFLAGS) $(INCDIRS) $^ *.c >Makefile.depend

tags: $(LIBFILES:.o=.c)
	etags $^

clean:
	rm -f *.o *.ko *~ *.core *.out *.ihex *_nm.c
	rm -f $(LIB) $(KERNELS) $(TOOLS)

.PHONY: all

-include Makefile.depend
