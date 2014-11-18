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

DOCDIR=../../doc

all: summary

doxygen:
	@make -C $(DOCDIR) doxygen.log 2> doxygen.runerr > doxygen.runlog

summary: doxygen
	@( doxyerr=`cat doxyerrors.cnt`; curerr=`cat $(DOCDIR)/doxygen.log | grep ": warning: " | wc -l` ; delta=`expr $$curerr - $$doxyerr`;  \
		if [ $$delta -lt 0 ] ; then  echo "doxygen: FAIL ಠ_ಠ\nCongratulations you fixed `expr $$delta \"*\" -1 ` of doxyen's warnings (old: $$doxyerr new: $$curerr). Please adjust regression-tests/00-doxygen/doxyerrors.cnt accordingly." > summary; fi ;\
		if [ $$delta -gt 0 ] ; then  echo "doxygen: FAIL ಠ_ಠ\nYou increased the number of warnings caused by doxygen by $$delta (old: $$doxyerr new: $$curerr). Please fix these." > summary; fi ; \
		if [ $$delta -eq 0 ] ; then  echo "doxygen: OK\nDoxygen found $$doxyerr errors" > summary ; fi ; \
		if [ -s doxygen.runerr ] ; then \
			echo "Runtime warnings:" >> summary; \
			cat doxygen.runerr >> summary; \
			echo >> summary; \
		fi ; \
		if [ -s $(DOCDIR)/doxygen.log ] ; then \
			echo "Warnings:" >> summary; \
			cat $(DOCDIR)/doxygen.log >> summary; \
		fi ; \
		cat summary \
	)


clean:
	@make -C $(DOCDIR) clean
