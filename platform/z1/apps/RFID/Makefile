ifndef TARGET
TARGET=z1
endif

# Enable to pull-in Z1SP specific test/source files
ZOLERTIA_Z1SP=0

CONTIKI_PROJECT = readTagUID

CONTIKI_SOURCEFILES += mfrc522.c 
APPS=serial-shell

ifeq ($(ZOLERTIA_Z1SP),1)
CONTIKI_PROJECT += test-potent
endif

all: $(CONTIKI_PROJECT)
CONTIKI = ../../../..
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
