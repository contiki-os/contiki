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
 *	.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef DB_TYPES_H
#define DB_TYPES_H

enum db_result {
  DB_FINISHED = 3,
  DB_GOT_ROW = 2,
  DB_OK = 1,
  DB_LIMIT_ERROR = -1,
  DB_ALLOCATION_ERROR = -2,
  DB_STORAGE_ERROR = -3,
  DB_PARSING_ERROR = -4,
  DB_NAME_ERROR = -5,
  DB_RELATIONAL_ERROR = -6,
  DB_TYPE_ERROR = -7,
  DB_IMPLEMENTATION_ERROR = -8,
  DB_INDEX_ERROR = -9,
  DB_BUSY_ERROR = -10,
  DB_INCONSISTENCY_ERROR = -11,
  DB_ARGUMENT_ERROR = -12
};

typedef enum db_result db_result_t;
typedef int db_storage_id_t;

#define DB_ERROR(result_code)	((result_code) < DB_OK)
#define DB_SUCCESS(result_code) !DB_ERROR(result_code)

#endif /* !DB_TYPES_H */
