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
 * This file is part of the Contiki desktop environment for the C64.
 *
 * $Id: www.h,v 1.4 2008/11/09 10:58:06 adamdunkels Exp $
 *
 */
#ifndef __WWW_H__
#define __WWW_H__

#ifndef WWW_CONF_WEBPAGE_WIDTH
#define WWW_CONF_WEBPAGE_WIDTH 80
#endif
#ifndef WWW_CONF_WEBPAGE_HEIGHT
#define WWW_CONF_WEBPAGE_HEIGHT 20
#endif
#ifndef WWW_CONF_HISTORY_SIZE
#define WWW_CONF_HISTORY_SIZE 40
#endif
#ifndef WWW_CONF_MAX_URLLEN
#define WWW_CONF_MAX_URLLEN 200
#endif
#ifndef WWW_CONF_MAX_NUMPAGEWIDGETS
#define WWW_CONF_MAX_NUMPAGEWIDGETS 20
#endif
#ifndef WWW_CONF_RENDERSTATE
#define WWW_CONF_RENDERSTATE 1
#endif
#ifndef WWW_CONF_FORMS
#define WWW_CONF_FORMS 1
#endif
#ifndef WWW_CONF_MAX_FORMACTIONLEN
#define WWW_CONF_MAX_FORMACTIONLEN 200
#endif
#ifndef WWW_CONF_MAX_FORMNAMELEN
#define WWW_CONF_MAX_FORMNAMELEN 200
#endif
#ifndef WWW_CONF_MAX_INPUTNAMELEN
#define WWW_CONF_MAX_INPUTNAMELEN 200
#endif
#ifndef WWW_CONF_MAX_INPUTVALUELEN
#define WWW_CONF_MAX_INPUTVALUELEN 200
#endif

PROCESS_NAME(www_process);

#endif /* __WWW_H__ */
