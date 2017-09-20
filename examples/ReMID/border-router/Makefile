CONTIKI_PROJECT = border-router
all: $(CONTIKI_PROJECT) 

TARGET ?= z1

CONTIKI=../../..

CONTIKI_WITH_IPV6 = 1

# Set to enable compiler and linker flags to minimize memory.
REDUCE_MEMORY_SIZE = 1

ifeq (REDUCE_MEMORY_SIZE, 1)
CFLAGS += -Os -ffunction-sections
LDFLAGS += -Wl,-Os,--gc-sections,--undefined=_reset_vector__,--undefined=InterruptVectors,--undefined=_copy_data_init__,--undefined=_clear_bss_init__,--undefined=_end_of_init__
endif

#CFLAGS += -DWITH_TSCH
#MODULES += core/net/mac/tsch

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
PROJECTDIRS += .. ../tools
PROJECT_SOURCEFILES += rpl-tools.c slip-bridge.c slip.c

CFLAGS += -DWITH_COAP
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
APPS += json
APPS += er-coap
APPS += rest-engine

ifeq ($(PREFIX),)
 PREFIX = aaaa::1/64
endif

include $(CONTIKI)/Makefile.include

ifeq ($(TARGET),jn516x)
 BAUDRATE = 1000000
else
 BAUDRATE = 115200
endif

#using XON/XOFF flow control
connect-router-sw:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -X -B $(BAUDRATE) $(PREFIX)

#using hw flow control	
connect-router-hw:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -H -B $(BAUDRATE) $(PREFIX)

#using no flow control	
connect-router-no:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -B $(BAUDRATE) $(PREFIX)
