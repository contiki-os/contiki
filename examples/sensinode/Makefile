ifndef TARGET
TARGET=sensinode
endif

# Make absolutely certain that you specify your device here
DEFINES+=MODEL_N740

# These examples don't need code banking so we turn it off 
#HAVE_BANKING=1

CONTIKI_PROJECT += timer-test blink-hello broadcast-rime

all: $(CONTIKI_PROJECT) 

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
