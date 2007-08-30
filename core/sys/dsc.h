/**
 * \file
 * Declaration of the DSC program description structure.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 */

/**
 * \addtogroup loader
 * @{
 */

/**
 * \page dsc The program description structure
 *
 * The Contiki DSC structure is used for describing programs. It
 * includes a string describing the program, the name of the program
 * file on disk (or a pointer to the programs initialization function
 * for systems without disk support), a bitmap icon and a text version
 * of the same icon.
 *
 * The DSC is saved into a file which can be loaded by programs such
 * as the "Directory" application which reads all DSC files on disk
 * and presents the icons and descriptions in a window.
 *
 */

/*
 * Copyright (c) 2003, Adam Dunkels.
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
 * $Id: dsc.h,v 1.3 2007/08/30 14:39:17 matsutsuka Exp $
 *
 */
#ifndef __DSC_H__
#define __DSC_H__

#include "ctk/ctk.h"

/**
 * The DSC program description structure.
 *
 * The DSC structure is used for describing a Contiki program. It
 * includes a short textual description of the program, either the
 * name of the program on disk, or a pointer to the init() function,
 * and an icon for the program.
 */
struct dsc {
  char *description; /**< A text string containing a one-line
			description of the program */
  
#if WITH_LOADER_ARCH
  char *prgname;     /**< The name of the program on disk. */
#else /* WITH_LOADER_ARCH */
  struct process *process; /**< A pointer to the program's process. */
#endif /* WITH_LOADER_ARCH */
  
#if CTK_CONF_ICONS  
  struct ctk_icon *icon;  /**< A pointer to the ctk_icon structure for
			     the DSC. */
#endif /* CTK_CONF_ICONS */
 
#if WITH_LOADER_ARCH
  void *loadaddr;         /**< The loading address of the DSC. Used by
			     the LOADER_UNLOAD() function when
			     deallocating the memory allocated for the
			     DSC when loading it. */
#endif /* WITH_LOADER_ARCH */
};

/**
 * Intantiating macro for the DSC structure.
 *
 * \param dscname The name of the C variable which is to contain the
 * DSC.
 *
 * \param description A one-line text describing the program.
 *
 * \param prgname The name of the program on disk.
 *
 * \param initfunc A pointer to the initialization function of the
 * program.
 *
 * \param icon A pointer to the CTK icon.
 */
#if WITH_LOADER_ARCH
#if CTK_CONF_ICONS
#define DSC(dscname, description, prgname, process, icon) \
        CLIF const struct dsc dscname = {description, prgname, icon}
#else /* CTK_CONF_ICONS */
#define DSC(dscname, description, prgname, process, icon) \
        CLIF const struct dsc dscname = {description, prgname}
#endif /* CTK_CONF_ICONS */
#else /* WITH_LOADER_ARCH */
#if CTK_CONF_ICONS
#define DSC(dscname, description, prgname, process, icon) \
    PROCESS_NAME(process); \
    const struct dsc dscname = {description, &process, icon}
#else /* CTK_CONF_ICONS */
#define DSC(dscname, description, prgname, process, icon) \
    PROCESS_NAME(process); \
    const struct dsc dscname = {description, &process}
#endif /* CTK_CONF_ICONS */
#endif /* WITH_LOADER_ARCH */

#define DSC_HEADER(name) extern struct dsc name

#ifndef NULL
#define NULL 0
#endif /* NULL */

/** @} */

#endif /* _DSC_H__ */
