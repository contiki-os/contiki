all: 
	make -f Makefile.ipso TARGET=avr-raven ipso.elf
	avr-objcopy -O ihex -R .eeprom ipso.elf ipso.hex
	avr-size -C ipso.elf

clean:
	make -f Makefile.ipso TARGET=avr-raven clean
	rm symbols.c symbols.h ipso.elf ipso.hex
	rm -rf obj_avr-raven
