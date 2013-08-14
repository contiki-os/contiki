ifndef TARGET
TARGET=sensinode
endif

# Make absolutely certain that you specify your device here
DEFINES+=MODEL_N740

# These examples don't need code banking so we turn it off 
#HAVE_BANKING=1

CONTIKI_PROJECT = hello_world clock_test rf_test_rx rf_test_tx

# New examples added by George Oikonomou - Loughborough University
CONTIKI_PROJECT += timer-test blink-hello broadcast-rime

all: $(CONTIKI_PROJECT) 

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
