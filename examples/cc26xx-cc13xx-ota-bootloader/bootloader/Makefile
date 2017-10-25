TARGET=srf06-cc26xx
ifndef BOARD
BOARD=sensortag/cc2650
#BOARD=srf06/cc26xx
#BOARD=launchpad/cc1310
endif

CC      = arm-none-eabi-gcc
CPP     = arm-none-eabi-cpp
LD      = arm-none-eabi-gcc
AR      = arm-none-eabi-ar
TRACE_CC = @echo "  CC       " $<
TRACE_LD = @echo "  LD       " $@
TRACE_AR = @echo "  AR       " $@
TRACE_AS = @echo "  AS       " $<
Q=@

OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
NM      = arm-none-eabi-nm
SIZE    = arm-none-eabi-size
SREC_CAT = srec_cat

CONTIKI = ../../..
OTA_SOURCE = ../apps/ota

CPU_ABS_PATH = $(CONTIKI)/cpu/cc26xx-cc13xx
TARGET=srf06-cc26xx
OBJECTDIR=obj_$(TARGET)

# Optimize for code size
CFLAGS += -Os

CPU_FAMILY=$(notdir $(BOARD))

LDSCRIPT = cc26xx-cc13xx.bootloader.ld
LDFLAGS += -mcpu=cortex-m3 -mthumb -mlittle-endian -nostartfiles
LDFLAGS += -T $(LDSCRIPT)
LDFLAGS += -g
LDFLAGS += -Wl,--gc-sections,--sort-section=alignment
LDFLAGS += -Wl,-Map=$(@:.elf=-$(TARGET).map),--cref,--no-warn-mismatch
OBJCOPY_FLAGS += -O binary --gap-fill 0xff
OBJDUMP_FLAGS += --disassemble --source --disassembler-options=force-thumb

#	TI libs
ifeq ($(CPU_FAMILY), cc1310)
	TI_XXWARE_PATH = lib/cc13xxware
else ifeq ($(CPU_FAMILY), cc1350)
	TI_XXWARE_PATH = lib/cc13xxware
else ifeq ($(CPU_FAMILY), cc2650)
	TI_XXWARE_PATH = lib/cc26xxware
else
  ${error Cannot determine CPU family of BOARD "$(BOARD)" (maybe it was misspelled?)}
endif
TI_XXWARE = $(CPU_ABS_PATH)/$(TI_XXWARE_PATH)
CFLAGS += -I$(CPU_ABS_PATH) -I$(OTA_SOURCE) -I./ext-flash -I$(TI_XXWARE) -I$(TI_XXWARE)/driverlib -I$(TI_XXWARE)/inc
SOURCEDIRS = . ./ext-flash $(TI_XXWARE)/startup_files $(TI_XXWARE)/driverlib
vpath %.c $(SOURCEDIRS)
SOURCE_FILES = $(foreach dir, $(SOURCEDIRS), $(notdir $(wildcard $(dir)/*.c)))

SOURCE_FILES += ota.c
vpath ota.c $(OTA_SOURCE)

CFLAGS += -mcpu=cortex-m3 -mthumb -mlittle-endian
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fshort-enums -fomit-frame-pointer -fno-strict-aliasing
CFLAGS += -Wall -std=c99
CFLAGS += -g

#	If CLEAR_OTA_SLOTS=1, the bootloader will always delete OTA slots 1-3
#	before executing any other code.  This can be used to clear the ext-flash.
ifeq ($(CLEAR_OTA_SLOTS),1)
	CFLAGS += -DCLEAR_OTA_SLOTS=1
endif

# If BURN_GOLDEN_IMAGE=1, the bootloader will backup the current firmware
#	to OTA slot 0 before executing any other code.
ifeq ($(BURN_GOLDEN_IMAGE),1)
	CFLAGS += -DBURN_GOLDEN_IMAGE=1
endif

### Are we building with code size optimisations?
ifdef WERROR
CFLAGS += -Werror
endif

### Compilation rules
### Don't treat the .elf as intermediate
.PRECIOUS: %.elf %.hex %.bin

oname = ${patsubst %.c,%.o,${patsubst %.S,%.o,$(1)}}
OBJECTFILES = ${addprefix $(OBJECTDIR)/,${call oname, $(SOURCE_FILES)}}

all: $(OBJECTFILES)

$(OBJECTDIR)/%.o: %.c | $(OBJECTDIR)
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

$(OBJECTDIR):
	mkdir $@

%.elf: $(OBJECTFILES) $(LDSCRIPT)
	$(TRACE_LD)
	$(Q)$(LD) $(LDFLAGS) ${filter-out $(LDSCRIPT) %.a,$^} ${filter %.a,$^} $(TARGET_LIBFILES) -lm -o $@

%.i16hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.hex: %.i16hex
	$(SREC_CAT) $< -intel -o $@ -intel

%.bin: %.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

%.lst: %.elf
	$(OBJDUMP) $(OBJDUMP_FLAGS) $< > $@

clean:
	-rm -f *~ *core core *.srec \
	*.lst *.map \
	*.cprg *.bin *.elf *.hex *.data contiki*.a *.firmware core-labels.S *.ihex *.ini \
	*.ce *.co
	rm -rf $(CLEAN)
	-rm -rf $(OBJECTDIR)
