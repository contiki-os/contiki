NUMPAR=20
IHEXFILE=tmpimage.ihex

# Check if we are running under Windows
ifdef OS
  ifneq (,$(findstring Windows,$(OS)))
    SERIALDUMP = $(CONTIKI)/tools/sky/serialdump-windows
    MOTELIST = $(CONTIKI)/tools/sky/motelist-windows
    BSL = $(CONTIKI)/tools/sky/msp430-bsl-windows
    MOTES = $(shell $(MOTELIST) | grep COM | \
       cut -f 4 -d \  | \
       perl -ne 'print $$1 - 1 . " " if(/COM(\d+)/);')
    CMOTES = $(shell $(MOTELIST) | grep COM | \
       cut -f 4 -d \  | \
       perl -ne 'print $$1 . " " if(/COM(\d+)/);')
  endif
endif

# If we are not running under Windows, we assume Linux
ifndef MOTELIST
  SERIALDUMP = $(CONTIKI)/tools/sky/serialdump-linux
  MOTELIST = $(CONTIKI)/tools/sky/motelist-linux
  BSL = $(CONTIKI)/tools/sky/msp430-bsl-linux
  MOTES = $(shell $(MOTELIST) 2>&- | grep USB | \
     cut -f 4 -d \  | \
     perl -ne 'print $$1 . " " if(m-(/dev/\w+)-);')
  CMOTES=$(MOTES)
endif

sky-motelist:
	$(MOTELIST)
sky-motes:
	@echo $(MOTES)

%.upload: %.ihex sky-reset
	cp $< $(IHEXFILE)
	$(MAKE) sky-upload

sky-upload: sky-reset
	$(MAKE) -j $(NUMPAR) sky-upload-sequence

sky-upload-sequence:	$(foreach PORT, $(MOTES), $(PORT).sky-u)
	@echo Done

sky-reset:
	$(MAKE) -k -j $(NUMPAR) sky-reset-sequence

sky-reset-sequence:	$(foreach PORT, $(MOTES), $(PORT).sky-r)
	@echo Done

%.sky-u:
	@echo +++++ Erasing $(basename $@) ; \
	$(BSL) --telosb -c $(basename $@) -e && sleep 2 ; \
	echo +++++ Programming $(basename $@) ; \
	$(BSL) --telosb -c $(basename $@) -I -p $(IHEXFILE) && sleep 2 ; \
	echo +++++ Resetting $(basename $@) ; \
	$(BSL) --telosb -c $(basename $@) -r

%.sky-r:
	$(BSL) --telosb -c $(basename $@) -r
