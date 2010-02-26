MC1322X := ..

# all off the common objects for each target
# a COBJ is made for EACH board and goes the obj_$(BOARD)_board directory
# board specific code is OK in these files
COBJS := tests.o 

# all of the target programs to build
TARGETS := blink-red blink-green blink-blue blink-white blink-allio \
           uart1-loopback \
           nvm-read nvm-write romimg flasher \
           tmr tmr-ints \
           sleep \
           rftest-rx \


include $(MC1322X)/Makefile.include






