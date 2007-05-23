.segment        "JUMPTABLE"
	.import _loader_appinit
	.export _loader_arch_loadaddr
_loader_arch_loadaddr:
arch:		.byte 0,0,0,0,0,0,0,0
version:	.byte 0,0,0,0,0,0,0,0	
; 	jmp _loader_appinit	

