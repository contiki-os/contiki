OUTFILE=loadable_prg

all:
	make -f Makefile.avr-zigbit TARGET=avr-zigbit loadable_prg.elf

	@if [ $(OUTFILE) != "loadable_prg" ]; then cp loadable_prg.elf $(OUTFILE).elf;fi
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE).elf $(OUTFILE).hex
	avr-size -C --mcu=atmega1281 $(OUTFILE).elf

clean:
	make -f Makefile.avr-zigbit TARGET=avr-zigbit clean
	rm symbols.c symbols.h loadable_prg.elf loadable_prg.hex
	rm -rf obj_avr-zigbit



