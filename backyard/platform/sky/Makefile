# $Id: Makefile,v 1.1 2008/02/24 22:22:17 adamdunkels Exp $
#
# This makefile requires GNU make!

LIB = libcontiki.a
KERNELS = gateway.ihex client.ihex dhclient.ihex
PROGS = loadable_prg.ko udpsend.ko udprecv.ko
TOOLS = tunslip scat codeprop

all: $(LIB) $(KERNELS) $(PROGS) $(TOOLS)

loadable_prg.ko: loadable_prg.o
udpsend.ko: udpsend.o
udprecv.ko: udprecv.o

gateway.out: gateway.o $(LIB)
client.out: client.o $(LIB)
dhclient.out: dhclient.o $(LIB)

TOP=../..
INCDIRS = -I. -I$(TOP)/cpu/msp430 -I$(TOP)/core -I$(TOP)/apps
SRCDIRS = dev  $(TOP)/cpu/msp430 ${addprefix $(TOP)/core/,sys dev net lib loader} $(TOP)/apps/codeprop

MCU=msp430x1611

ARCH=msp430.o minileds.o watchdog.o light.o button.o spi.o ds2411.o rom.o xmem.o i2c.o cle_msp430.o
SYSTEM=process.o procinit.o clock.o etimer.o timer.o \
        cmod.o cle.o sym.o
UIP=uip.o uiplib.o tcpip.o uip-fw.o uip-fw-drv.o uipbuf.o \
    tcpdump.o psock.o dhcpc.o uaodv.o uaodv-rt.o uip-udp-packet.o \
    uip-ipchksum.o
UIPDRIVERS=slip.o slip_uart1.o \
           cc2420.o cc2420_uart0.o cc2420_send_ip.o cc2420_send_uaodv.o 
LIBS=memb.o list.o malloc.o realloc.o malloc_compact.o rand.o assert.o
SYSAPPS=tcp_loader2.o
LIBFILES=$(SYSTEM) $(ARCH) $(UIP) $(UIPDRIVERS) $(LIBS) $(SYSAPPS)

MKNMLIST=awk -f $(TOP)/tools/mknmlist-ansi
CP=cp
LIBC=-lgcc -lc -lgcc
CC=msp430-gcc
LD=msp430-ld
AS=msp430-as
NM=msp430-nm
OBJCOPY=msp430-objcopy
STRIP=msp430-strip
AR=msp430-ar
RANLIB=msp430-ranlib
BSL=msp430-bsl
CFLAGS=-mmcu=$(MCU) $(INCDIRS) -Wall -Os
LDFLAGS=-mmcu=$(MCU)

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
	$(BSL) --telosb -D -c $(COMPORT) -r -e -I -p $<

$(LIB): $(LIBFILES)
	$(AR) rcf $@ $^
	$(RANLIB) $@

codeprop: $(TOP)/tools/codeprop.c
	cc -g -Wall $< -o $@

tunslip: $(TOP)/tools/tunslip.c
	cc -g -Wall $< -o $@

scat: $(TOP)/tools/scat.c
	cc -g -Wall $< -o $@

### TEST ###

test.out: test.o $(LIB)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

depend: $(LIBFILES:.o=.c)
	-$(CC) $(CCDEPFLAGS) -MM $(CFLAGS) $(INCDIRS) $^ >Makefile.depend

tags: $(LIBFILES:.o=.c)
	etags $^

clean:
	rm -f *.o *.ko *~ *.core *.out *.ihex *_nm.c
	rm -f $(LIB) $(KERNELS) $(TOOLS)

.PHONY: all

-include Makefile.depend
