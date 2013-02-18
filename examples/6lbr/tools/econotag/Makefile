MC1322X := libmc1322x
BOARD=redbee-econotag
CFLAGS = -I$(MC1322X)/tests

# all off the common objects for each target
# a COBJ is made for EACH board and goes the obj_$(BOARD)_board directory
# board specific code is OK in these files
COBJS := put.o

# these targets are built with space reserved for variables needed by ROM services
# this space is initialized with a rom call to rom_data_init
TARGETS_WITH_ROM_VARS := 6lbr_nvm_flasher

##################################################
# you shouldn't need to edit anything below here #
##################################################

# This Makefile includes the default rule at the top
# it needs to be included first
-include $(MC1322X)/Makefile.include



