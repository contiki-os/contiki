CONTIKI_PROJECT = klk-bootloader

LINKERSCRIPT= $(CONTIKI_PROJECT).ld

COFFEE=1

# To disable in final bootloader !!! (use direct function call from contiki-main)
#CFLAGS+=-DAUTOSTART_ENABLE
CFLAGS+= -DNO_WATCHDOG -DKLK_BOOTLOADER

CFLAGS+= -DNO_WATCHDOG -DNO_NETWORK 
#CFLAGS+= -DNO_NETWORK -DNO_RTIMER

# Optimization / debug options (-g, -O3, -Os)
# Best compromise between debug and size : -g -O
#CFLAGS+= -g
CFLAGS+= -g -O
#CFLAGS += -Os

PROJECT_SOURCEFILES += ymodem.c

all: $(CONTIKI_PROJECT) $(CONTIKI_PROJECT).size $(CONTIKI_PROJECT).bin #$(CONTIKI_PROJECT).flashprof $(CONTIKI_PROJECT).ramprof

APPS = serial-shell logger

CONTIKI = ../..

include $(CONTIKI)/Makefile.include

