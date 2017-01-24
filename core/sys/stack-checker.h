/*
 * Copyright (c) 2017, University of Bristol - http://www.bris.ac.uk/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *         Stack checker library header file.
 * \author
 *         Atis Elsts <atis.elsts@bristol.ac.uk>
 */

/** \addtogroup sys
 * @{ */

/**
 * \defgroup stack Stack checker library
 *
 * Basic support for stack guards and stack overflow detection.
 * On startup, fills the area between the stack and the heap with a known pattern.
 * During execution, the fill can be checked in order to find out
 * the extent to which the stack has been used.
 * 
 *
 * @{
 */

#ifndef STACK_CHECK_H_
#define STACK_CHECK_H_

/* If this is disabled, the functions are no-ops */
#ifdef STACK_CHECK_CONF_ENABLED
#define STACK_CHECK_ENABLED STACK_CHECK_CONF_ENABLED
#else
#define STACK_CHECK_ENABLED 1 /* Enable by default */
#endif

/*---------------------------------------------------------------------------*/
#if STACK_CHECK_ENABLED
/*---------------------------------------------------------------------------*/

/**
 * \brief      Initialize the stack area with a known pattern
 *
 *             This function initializes the memory between the stack and heap
 *             areas. The function should be called by the system
 *             during boot-up.
 */
void stack_check_init(void);

/**
 * \brief      Calculate the maximal stack usage so far.
 *
 *             This function relies on the assumption that the stack memory
 *             that has been reserved by functions and local variables
 *             is actually overwritten with new contents. If the stack is
 *             just reserved, but not used, the function will fail to detect
 *             that usage.
 *             In addition, this function can warn if the stack memory range
 *             has been completely used, but it cannot detect
 *             and warn if stack overflow has already taken place.
 */
uint16_t stack_check_get_usage(void);

/**
 * \brief      Calculate the maximal permitted stack usage.
 *
 *             This function returns the number of bytes between the origin
 *             of the stack and the end of heap.
 */
uint16_t stack_check_get_reserved_size(void);

/**
 * \brief      The origin point from which the stack grows (an optional #define)
 *
 */
#ifdef STACK_CONF_ORIGIN
#define STACK_ORIGIN STACK_CONF_ORIGIN
#endif

/*---------------------------------------------------------------------------*/
#else /* STACK_CHECK_ENABLED */
/*---------------------------------------------------------------------------*/

/* Stack check functionality disabled */
#define stack_check_init()
#define stack_check_get_usage() 0
#define stack_check_get_reserved_size() 0

/*---------------------------------------------------------------------------*/
#endif /* STACK_CHECK_ENABLED */
/*---------------------------------------------------------------------------*/

#endif /* STACK_CHECK_H_ */

/** @} */
/** @} */
