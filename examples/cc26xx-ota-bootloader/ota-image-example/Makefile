TARGET=srf06-cc26xx
BOARD=sensortag/cc2650

CONTIKI = ../../..
OTA=1
include $(CONTIKI)/Makefile.include

clean:
	-rm -f *~ *core core *.srec \
	*.lst *.map \
	*.cprg *.bin *.elf *.hex *.data contiki*.a *.firmware core-labels.S *.ihex *.ini \
	*.ce *.co
	rm -rf $(CLEAN)
	-rm -rf $(OBJECTDIR)
