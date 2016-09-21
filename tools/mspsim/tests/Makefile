### Check if we are running under Windows

ifndef WINDIR
  ifdef OS
    ifneq (,$(findstring Windows,$(OS)))
      WINDIR := Windows
    endif
  endif
endif

.SUFFIXES:

#MCU=msp430x149
MCU=msp430x1611

### Compiler definitions
CC       = msp430-gcc
LD       = msp430-ld
AS       = msp430-as
AR       = msp430-ar
OBJCOPY  = msp430-objcopy
STRIP    = msp430-strip
BSL      = msp430-bsl
CFLAGSNO = -I. -DWITH_ASCII \
           -Wall -mmcu=$(MCU) -g
CFLAGS  += $(CFLAGSNO) -Os

SOURCES := msp430setup.c

OBJECTS := $(SOURCES:.c=.o)

#all:	cputest.ihex
all:	cputest.firmware timertest.firmware


%.firmware:	%.co $(OBJECTS)
	$(CC) -mmcu=$(MCU) -Wl,-Map=$(@:.firmware=.map) $(CFLAGS) -o $@ $^

%.ihex: %.firmware
	$(OBJCOPY) $^ -O ihex $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.co: %.c
	$(CC) $(CFLAGS) -DAUTOSTART_ENABLE -c $< -o $@

%.u:	%.ihex
	msp430-jtag -eI $^

clean:
	rm -f *~ *.lst *.map *.co *.o *.ihex *.firmware
