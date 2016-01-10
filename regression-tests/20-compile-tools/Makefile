# Copyright (c) 2014, Friedrich-Alexander University Erlangen-Nuremberg
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

TOOLS=sky  tools  stm32w  z80=hex2bin
FAILTOOLS=stm32w=uip6_bridge sky=uip6-bridge


TOOLSDIR=../../tools
LOGS=$(patsubst %,%.log, $(TOOLS))

all: summary

FRC:


sky.log: RMFILES=serialdump-linux
stm32w.log: RMFILES=serialdump-linux


tools.log:
		@$(MAKE) -C $(TOOLSDIR) > $@ 2>&1 && $(RM) $@.failed || touch $@.failed


%.log:  FRC
	@( cd $(TOOLSDIR)/$(subst =,/,$*) &&  $(RM) $(RMFILES) )
	@touch $@
	@$(MAKE) -C $(TOOLSDIR)/$(subst =,/,$*) > $@ 2>&1 && $(RM) $@.failed || touch $@.failed


summary: $(LOGS)
	@(\
		for T in $(TOOLS) ; do \
			if [ -f $$T.log.failed  ] ; then \
				echo tools/$$T: FAIL ಠ_ಠ >> $@;\
				cat $$T.log >> $@;\
			else \
				echo tools/$$T: OK >> $@;\
			fi\
		done \
	)
	@echo "Info: The following tools need fixing and are not tested:" $(subst =,/,$(FAILTOOLS)) >> $@
	@echo $@

clean:
	@make -C $(DOCDIR) clean


.PHONY: %.log
