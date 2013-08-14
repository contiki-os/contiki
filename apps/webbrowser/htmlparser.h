/*
 * Copyright (c) 2002, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Contiki desktop environment 
 *
 *
 */
#ifndef __HTMLPARSER_H__
#define __HTMLPARSER_H__

#include "contiki-net.h"

/* Callbacks. */
void htmlparser_link(char *text, unsigned char textlen, char *url);
void htmlparser_form(char *action);
void htmlparser_submitbutton(char *value,
			     char *name);
void htmlparser_inputfield(unsigned char type,
			   unsigned char size,
			   char *value,
			   char *name);
void htmlparser_newline(void);
void htmlparser_word(char *word, unsigned char wordlen);


#define HTMLPARSER_INPUTTYPE_NONE   0
#define HTMLPARSER_INPUTTYPE_TEXT   1
#define HTMLPARSER_INPUTTYPE_HIDDEN 2
#define HTMLPARSER_INPUTTYPE_SUBMIT 3
#define HTMLPARSER_INPUTTYPE_IMAGE  4
#define HTMLPARSER_INPUTTYPE_OTHER  5


/* Functions. */
void htmlparser_init(void);
void htmlparser_parse(char *data, uint16_t len);


#endif /* __HTMLPARSER_H__ */
