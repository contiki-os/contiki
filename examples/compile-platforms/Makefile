all:
	@rm -f *.output
	@$(MAKE) -s -k compile || cat *.output

compile: msp430 native

msp430: sky.platform esb.platform # msb430.platform
native: native.platform netsim.platform minimal-net.platform

%.platform:
	@mkdir -p $*
	@cp *.c $*
	@cp Makefile.platform $*/Makefile
	@(echo; echo ------------------- $* -------------------; \
          cd $* ; ($(MAKE) TARGET=$* >& /dev/null) ; \
                  $(MAKE) TARGET=$*) >& $*.output || \
          (echo; echo "$* failed"; exit 1)
