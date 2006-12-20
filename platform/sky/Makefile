# $Id: Makefile,v 1.8 2006/12/20 13:46:14 bg- Exp $
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

CORE = ../../core
INCDIRS = -I. -I../../cpu/msp430 -I$(CORE) -I../../apps
SRCDIRS = dev  ../../cpu/msp430 ${addprefix $(CORE)/,sys dev net lib loader} ../../apps/codeprop

MCU=msp430x1611

ARCH=msp430.o minileds.o watchdog.o light.o button.o spi.o ds2411.o rom.o xmem.o i2c.o
SYSTEM=process.o procinit.o service.o clock.o etimer.o timer.o \
        elfloader_compat.o cle.o symtab.o
UIP=uip.o uiplib.o tcpip.o uip-fw.o uip-fw-service.o uipbuf.o \
    tcpdump.o psock.o dhcpc.o uaodv.o uaodv-rt.o uip-udp-packet.o
UIPDRIVERS=slip.o slip_uart1.o \
           cc2420.o cc2420_uart0.o cc2420_send_ip.o cc2420_send_uaodv.o 
LIBS=memb.o list.o malloc.o realloc.o malloc_compact.o rand.o assert.o crtk.o
SYSAPPS=tcp_loader.o
LIBFILES=$(SYSTEM) $(ARCH) $(UIP) $(UIPDRIVERS) $(LIBS) $(SYSAPPS)

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
# FreeBSD: /dev/ttyU0, /dev/ttyU1, etc.
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
	: | awk -f ../../tools/mknmlist > $@_tmp.c && mv $@_tmp.c $@_nm.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBC) $@_nm.c
	$(NM) $@ | awk -f ../../tools/mknmlist > $@_tmp.c && mv $@_tmp.c $@_nm.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBC) $@_nm.c

%.u: %.ihex
	$(BSL) --telosb -D -c $(COMPORT) -r -e -I -p $<

$(LIB): $(LIBFILES)
	$(AR) rcf $@ $^
	$(RANLIB) $@

codeprop: ../../tools/codeprop.c
	cc -g -Wall $< -o $@

tunslip: ../../tools/tunslip.c
	cc -g -Wall $< -o $@

scat: ../../tools/scat.c
	cc -g -Wall $< -o $@

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
