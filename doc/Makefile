# This target requires and graphviz and doxygen
dox:
	doxygen Doxyfile


pdf:
	gmake -C latex refman.pdf

upload: dox  pdf
	scp -C latex/refman.pdf adamdunkels@shell.sourceforge.net:/home/groups/c/co/contiki/htdocs/
	tar czfv - html | ssh adamdunkels@shell.sourceforge.net "cd /home/groups/c/co/contiki/htdocs/; tar xfz -"
