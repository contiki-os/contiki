MC1322X := ..

# all off the common objects for each target
# a COBJ is made for EACH board and goes the obj_$(BOARD)_board directory
# board specific code is OK in these files
COBJS := tests.o 

# all of the target programs to build
TARGETS := blink-green blink-blue blink-white blink-allio \
           uart1-loopback \
           nvm-read nvm-write romimg flasher \
           tmr tmr-ints \
           sleep \
           rftest-rx rftest-tx \
           printf

# these targets are built with space reserved for variables needed by ROM services
# this space is initialized with a rom call to rom_data_init
TARGETS_WITH_ROM_VARS := blink-red

include $(MC1322X)/Makefile.include






