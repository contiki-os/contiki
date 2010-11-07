all:
	@rm -f *.output
	@echo Compiling on:
	@uname -a
	@$(MAKE) -s -k compile || cat *.output

compile: 6502 msp430 native avr

6502:	c64.platform c128.platform apple2enh.platform atari.platform
msp430: sky.platform esb.platform # msb430.platform
native: native.platform netsim.platform minimal-net.platform
avr:    avr-raven.platform

clean:
	@rm -rf [a-z]*

%.platform:
	@mkdir -p $*
	@cp *.c $*
	@cp Makefile.platform $*/Makefile
	@((echo; echo ------------------- $* -------------------; \
           cd $* ; ($(MAKE) TARGET=$* > /dev/null 2>&1 ) ; \
                   $(MAKE) TARGET=$*) > $*.output 2>&1 && echo "$* succeeded") || \
           (echo; echo "$* failed"; exit 1)
