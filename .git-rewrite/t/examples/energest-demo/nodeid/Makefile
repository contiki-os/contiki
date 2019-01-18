
burn-nodeid-%.ihex:
	$(MAKE) nodeid=$* burn-nodeid.ihex && mv burn-nodeid.ihex $@

burn-nodeids: $(foreach NODEID, 41 42 43 44 45 46 47 48, burn-nodeid-$(NODEID).ihex)

ifndef CONTIKI
CONTIKI = ../../..
endif

ifndef TARGET
TARGET=sky
endif

include $(CONTIKI)/Makefile.include
