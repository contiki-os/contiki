all: tunslip

tunslip6: tools-utils.c tunslip6.c

gitclean:
	@git clean -d -x -n ..
	@echo "Enter yes to delete these files";
	@read response;if [[ "$$response" = "yes" ]]; then git clean -d -f -x ..;fi;
	
distclean: cleanobj cleanfiles cleantargets cleandone
cleanobj:
	${info Removing obj_* directories...}
	@find ../examples/ -name "obj_*" -print -exec rm -R '{}' +
cleanfiles:
	${info Removing .map, .co, .ce, contiki*.a files...}
	@find ../examples -name "*.map" -print -delete
	@find ../examples -name "*.co" -print -delete
	@find ../examples -name "*.ce" -print -delete
	@find ../examples -name "contiki-*.a" -delete 
cleantargets:
	${info Removing .TARGET builds...}
	@rm -f -v ${addprefix ../examples/*/*., ${shell ls ../platform/}}
	@rm -f -v ${addprefix ../examples/*/*/*., ${shell ls ../platform/}}
cleandone:
	@echo ${info All done!}
