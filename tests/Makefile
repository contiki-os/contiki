MC1322X := ..

# all off the common objects for each target
# a COBJ is made for EACH board and goes the obj_$(BOARD)_board directory
# board specific code is OK in these files
COBJS := tests.o 

# all of the target programs to build
TARGETS := blink-red blink-green blink-blue blink-white blink-allio \
           uart1-loopback \
           tmr tmr-ints \
           sleep \
           printf 

# these targets are built with space reserved for variables needed by ROM services
# this space is initialized with a rom call to rom_data_init
TARGETS_WITH_ROM_VARS := nvm-read nvm-write romimg flasher rftest-rx rftest-tx    

include $(MC1322X)/Makefile.include






