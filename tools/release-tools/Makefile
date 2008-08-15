ifndef RELEASE
  ${error You must specify the RELEASE variable (e.g. make RELEASE=2.2)}
endif

RELEASETAG=$(subst .,-,$(RELEASE))
USERNAME=:pserver:anonymous
CVSROOT=$(USERNAME)@contiki.cvs.sourceforge.net:/cvsroot/contiki

package: clean export copy tgz zip collectpack almost-clean

test: clean checkout compile

almost-clean:
	rm -rf contiki-2.x contiki-$(RELEASE) contiki-collect-$(RELEASE)

clean:
	rm -rf contiki-2.x contiki-$(RELEASE) contiki-collect-$(RELEASE) *.zip *.tar.gz

copy:
	cp -r contiki-2.x contiki-$(RELEASE)

doc:
	(cd contiki-2.x/doc; make dox pdf)

copydoc:
	cp contiki-2.x/doc/latex/refman.pdf contiki-$(RELEASE)/doc/
	cp -r contiki-2.x/doc/html contiki-$(RELEASE)/doc/

tgz:
	tar czf contiki-$(RELEASE).tar.gz contiki-$(RELEASE)
	chmod 644 contiki-$(RELEASE).tar.gz

zip:
	zip -r contiki-$(RELEASE).zip contiki-$(RELEASE)
	chmod 644 contiki-$(RELEASE).zip

compile:
	(cd contiki-2.x/examples/compile-platforms; make)

checkout:
	cvs -d $(CVSROOT) co -r contiki-$(RELEASETAG) contiki-2.x

export:
	cvs -d $(CVSROOT) export -r contiki-$(RELEASETAG) contiki-2.x

contiki-collect:
	(cd contiki-2.x/examples/sky-shell; make sky-shell.ihex; ant dist; chmod 755 dist/tools/*-linux)
	cp -r contiki-2.x/examples/sky-shell/dist contiki-collect-$(RELEASE)

collectpack: contiki-collect
	tar czf contiki-collect-$(RELEASE).tar.gz contiki-collect-$(RELEASE)
	chmod 644 contiki-collect-$(RELEASE).tar.gz
	zip -r contiki-collect-$(RELEASE).zip contiki-collect-$(RELEASE)
	chmod 644 contiki-collect-$(RELEASE).zip
