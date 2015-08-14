DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = remote-demo test-tsl2563 test-sht25
CONTIKI_TARGET_SOURCEFILES += tsl2563.c sht25.c

all: $(CONTIKI_PROJECT)

CONTIKI = ../..
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
