/* Copyright (c) 2011 Red Hat Incorporated.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met: 

     Redistributions of source code must retain the above copyright 
     notice, this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

     The name of Red Hat Incorporated may not be used to endorse 
     or promote products derived from this software without specific 
     prior written permission.

   This software is provided by the copyright holders and contributors
   "AS IS" and any express or implied warranties, including, but not
   limited to, the implied warranties of merchantability and fitness for
   a particular purpose are disclaimed.  In no event shall Red Hat
   incorporated be liable for any direct, indirect, incidental, special,
   exemplary, or consequential damages (including, but not limited to,
   procurement of substitute goods or services; loss of use, data, or
   profits; or business interruption) however caused and on any theory of
   liability, whether in contract, strict liability, or tort (including
   negligence or otherwise) arising in any way out of the use of this
   software, even if advised of the possibility of such damage.  */

#define PROFILE_SUPPORT 1

#include "crt0.S"

	.global __mcount						
	.type	__mcount, @function
__mcount:								
	;; When a function is compiled for profiling, gcc creates code	
	;; like this at the start of each profiled function:		
	;;								
	;;      .global <func_name>					
	;; <func_name>:							
	;;      bsr __mcount						
	;;	<...function's prologue...>				
	;; 	<...function's body...>					
	;;								
	;; We must save all of the argument registers, extract the	
	;; address of <func_name>, call _mcount_internal to do the	
	;; real work and then restore the argument registers before	
	;; returning.							
									
	movw	ax, [sp]
	push	ax
	call	!!__mcount_internal
	pop	ax
	ret

	.size	__mcount, . - __mcount
