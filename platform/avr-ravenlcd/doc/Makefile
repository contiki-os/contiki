# This target requires and graphviz and doxygen
dox:
	doxygen Doxyfile


pdf:
	$(MAKE) -C latex refman.pdf

upload: dox  #pdf
#	scp -C latex/refman.pdf adamdunkels@shell.sourceforge.net:/home/groups/c/co/contiki/htdocs/
#	tar czfv - html | ssh adamdunkels@shell.sourceforge.net "cd /home/groups/c/co/contiki/htdocs/; tar xfz -"
	(cd html; rsync -rv . kloster.sics.se:/home/adam/public_html/contiki/docs)
