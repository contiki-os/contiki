COOJADIR=../../tools/cooja
LOGDIR=$(COOJADIR)/build/reports/checkstyle/

all: summary

checkstyle:
	@gradle -b $(COOJADIR)/build.gradle clean check

summary: checkstyle
	@( chkerr=`cat checkstyle_errors.cnt`; curerr=`cat $(LOGDIR)/main.xml | grep "error" | wc -l` ; delta=`expr $$curerr - $$chkerr`;  \
		if [ $$delta -lt 0 ] ; then  echo "cooja_checkstyle: FAIL ಠ_ಠ\nCongratulations you fixed `expr $$delta \"*\" -1 ` of cooja checkstyle's warnings (old: $$chkerr new: $$curerr). Please adjust regression-tests/21-cooja-check/checkstyle_errors.cnt accordingly." > summary; fi ;\
		if [ $$delta -gt 0 ] ; then  echo "cooja_checkstyle: FAIL ಠ_ಠ\nYou increased the number of warnings caused by cooja_checkstyle by $$delta (old: $$chkerr new: $$curerr). Please fix these." > summary; fi ; \
		if [ $$delta -eq 0 ] ; then  echo "cooja_checkstyle: OK\ncooja_checkstyle: gradle check found $$chkerr errors" > summary ; fi ; \
		cat summary \
	)

clean:
	@gradle -b $(COOJADIR) clean
