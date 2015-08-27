COOJADIR=../../tools/cooja
LOGDIR=$(COOJADIR)/build/reports/findbugs/

all: summary

findbugs:
	@gradle -b $(COOJADIR)/build.gradle clean check

summary: findbugs
	@( chkerr=`cat findbugs_errors.cnt`; curerr=`cat $(LOGDIR)/main.xml | grep "</BugInstance>" | wc -l` ; delta=`expr $$curerr - $$chkerr`;  \
		if [ $$delta -lt 0 ] ; then  echo "cooja_findbugs: FAIL ಠ_ಠ\nCongratulations you fixed `expr $$delta \"*\" -1 ` of cooja findbugs's warnings (old: $$chkerr new: $$curerr). Please adjust regression-tests/22-cooja-findbugs/findbugs_errors.cnt accordingly." > summary; fi ;\
		if [ $$delta -gt 0 ] ; then  echo "cooja_findbugs: FAIL ಠ_ಠ\nYou increased the number of warnings caused by cooja_findbugs by $$delta (old: $$chkerr new: $$curerr). Please fix these." > summary; fi ; \
		if [ $$delta -eq 0 ] ; then  echo "cooja_findbugs: OK\ncooja_findbugs: gradle check found $$chkerr errors" > summary ; fi ; \
		cat summary \
	)

clean:
	@gradle -b $(COOJADIR) clean
