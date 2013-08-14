/*
 * Copyright (c) 2010, Swedish Institute of Computer Science
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
 */

/**
 * \file
 *	Database configuration options.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef DB_OPTIONS_H
#define DB_OPTIONS_H

#include "contiki-conf.h"

/* Features. Include only what is needed in order to save space. */
#ifndef DB_FEATURE_JOIN
#define DB_FEATURE_JOIN			1
#endif /* DB_FEATURE_JOIN */

#ifndef DB_FEATURE_REMOVE
#define DB_FEATURE_REMOVE		1
#endif /* DB_FEATURE_REMOVE */

#ifndef DB_FEATURE_FLOATS
#define DB_FEATURE_FLOATS		0
#endif /* DB_FEATURE_FLOATS */

#ifndef DB_FEATURE_COFFEE
#define DB_FEATURE_COFFEE		1
#endif /* DB_FEATURE_COFFEE */

#ifndef DB_FEATURE_INTEGRITY
#define DB_FEATURE_INTEGRITY		0
#endif /* DB_FEATURE_INTEGRITY */


/* Configuration parameters that may be trimmed to save space. */
#ifndef DB_ERROR_BUF_SIZE
#define DB_ERROR_BUF_SIZE		50
#endif /* DB_ERROR_BUF_SIZE */

#ifndef DB_INDEX_POOL_SIZE
#define DB_INDEX_POOL_SIZE		3
#endif /* DB_INDEX_POOL_SIZE */

#ifndef DB_RELATION_POOL_SIZE
#define DB_RELATION_POOL_SIZE		5
#endif /* DB_RELATION_POOL_SIZE */

#ifndef DB_ATTRIBUTE_POOL_SIZE
#define DB_ATTRIBUTE_POOL_SIZE		16
#endif /* DB_ATTRIBUTE_POOL_SIZE */

#ifndef DB_MAX_ATTRIBUTES_PER_RELATION
#define DB_MAX_ATTRIBUTES_PER_RELATION	6
#endif /* DB_MAX_ATTRIBUTES_PER_RELATION */

#ifndef DB_MAX_ELEMENT_SIZE
#define DB_MAX_ELEMENT_SIZE		16
#endif /* DB_MAX_ELEMENT_SIZE */


#ifndef DB_VM_BYTECODE_SIZE
#define DB_VM_BYTECODE_SIZE		128
#endif /* DB_VM_BYTECODE_SIZE */

/* Language options. */
#ifndef AQL_MAX_QUERY_LENGTH
#define AQL_MAX_QUERY_LENGTH        	128
#endif /* AQL_MAX_QUERY_LENGTH */

#ifndef AQL_MAX_VALUE_LENGTH
#define AQL_MAX_VALUE_LENGTH        	DB_MAX_ELEMENT_SIZE
#endif /* AQL_MAX_VALUE_LENGTH */

#ifndef AQL_RELATION_LIMIT
#define AQL_RELATION_LIMIT     		3
#endif /* AQL_RELATION_LIMIT */

#ifndef AQL_ATTRIBUTE_LIMIT
#define AQL_ATTRIBUTE_LIMIT    		5
#endif /* AQL_ATTRIBUTE_LIMIT */


/* Physical storage options. Changing these may cause compatibility problems. */
#ifndef DB_COFFEE_RESERVE_SIZE
#define DB_COFFEE_RESERVE_SIZE          (128 * 1024UL)
#endif /* DB_COFFEE_RESERVE_SIZE */

#ifndef DB_MAX_CHAR_SIZE_PER_ROW
#define DB_MAX_CHAR_SIZE_PER_ROW	64
#endif /* DB_MAX_CHAR_SIZE_PER_ROW */

#ifndef DB_MAX_FILENAME_LENGTH
#define DB_MAX_FILENAME_LENGTH		16
#endif /* DB_MAX_FILENAME_LENGTH */

#ifndef ATTRIBUTE_NAME_LENGTH
#define ATTRIBUTE_NAME_LENGTH		12
#endif /* ATTRIBUTE_NAME_LENGTH */

#ifndef RELATION_NAME_LENGTH
#define RELATION_NAME_LENGTH		10
#endif /* RELATION_NAME_LENGTH */

#ifndef RESULT_RELATION
#define RESULT_RELATION			"db-result"
#endif /* RESULT_RELATION */

#ifndef TEMP_RELATION
#define TEMP_RELATION			"db-temp"
#endif /* TEMP_RELATION */

/* Index options. */
#ifndef DB_INDEX_COST
#define DB_INDEX_COST			64
#endif /* DB_INDEX_COST */

#ifndef DB_MEMHASH_INDEX_LIMIT
#define DB_MEMHASH_INDEX_LIMIT  	1
#endif /* DB_MEMHASH_INDEX_LIMIT */

#ifndef DB_MEMHASH_TABLE_SIZE
#define DB_MEMHASH_TABLE_SIZE		61
#endif /* DB_MEMHASH_TABLE_SIZE */

#ifndef DB_HEAP_INDEX_LIMIT
#define DB_HEAP_INDEX_LIMIT		1
#endif /* DB_HEAP_INDEX_LIMIT */

#ifndef DB_HEAP_CACHE_LIMIT
#define DB_HEAP_CACHE_LIMIT		1
#endif /* DB_HEAP_CACHE_LIMIT */


/* Propositional Logic Engine options. */
#ifndef PLE_MAX_NAME_LENGTH
#define PLE_MAX_NAME_LENGTH		ATTRIBUTE_NAME_LENGTH
#endif /* PLE_MAX_NAME_LENGTH */

#ifndef PLE_MAX_VARIABLE_ID
#define PLE_MAX_VARIABLE_ID		AQL_ATTRIBUTE_LIMIT - 1
#endif /* PLE_MAX_VARIABLE_ID */

#ifndef PLE_USE_FLOATS
#define PLE_USE_FLOATS			DB_FEATURE_FLOATS
#endif /* PLE_USE_FLOATS */


#endif /* !DB_OPTIONS_H */
