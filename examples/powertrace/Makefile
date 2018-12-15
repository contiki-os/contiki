CONTIKI_PROJECT = example-powertrace
APPS+=powertrace
all: $(CONTIKI_PROJECT)


CFLAGS+= -lm
LDFLAGS += -lm
TARGET_LIBFILES+=-lm


CONTIKI = ../..
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
