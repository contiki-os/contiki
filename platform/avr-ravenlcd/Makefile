# Hey Emacs, this is a -*- makefile -*-
###############################################################################
# Makefile for the RES-Raven-Mega3290 project
###############################################################################

## General Flags
PROJECT = ravenlcd_3290
MCU = atmega3290
TARGET = $(PROJECT).elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)
COMMON += -DF_CPU=8000000UL

## Compile options common for all C compilation units.
CFLAGS = $(COMMON) $(CEXTRA)
CFLAGS += -D AVRGCC -Wall -gdwarf-2  -Os -fsigned-char
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d
CFLAGS += -fshort-enums

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf-2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map=$(PROJECT).map,--cref

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0

## Include Directories
INCLUDES =

## Objects that must be built in order to link
SRC = adc.c  key.c  lcd.c  raven3290.c  uart.c menu.c sleep.c beep.c temp.c timer.c

OBJECTS = $(SRC:.c=.o)

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: $(TARGET) $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss size

## Compile: create object files from C source files.
.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@  || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(PROJECT).elf dep/* $(PROJECT).hex $(PROJECT).eep $(PROJECT).map $(PROJECT).lss

## Other dependencies
## In cygwin the /dep folder causes make to fail after the initial make.
## $make CYG=1 allows cleans and makes based on .c dependencies (but not .h)
ifndef CYG
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)
endif

