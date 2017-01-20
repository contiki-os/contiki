CFLAGS+=-DPROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = client ifttt_client
all: $(CONTIKI_PROJECT)

BOARD = orion

MODULES += core/net/http-socket

WITH_IP64 = 1

CONTIKI = ../../../../..
include $(CONTIKI)/Makefile.include
