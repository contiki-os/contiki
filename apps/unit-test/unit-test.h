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
 *	A tool for unit testing Contiki software.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include "sys/rtimer.h"

typedef enum unit_test_result {
  unit_test_failure = 0,
  unit_test_success = 1
} unit_test_result_t;

/**
 * The unit_test structure describes the results of a unit test. Each 
 * registered unit test statically allocates an object of this type.
 */
typedef struct unit_test {
  const char * const descr;
  const char * const test_file;
  unit_test_result_t result;
  unsigned exit_line;
  rtimer_clock_t start;
  rtimer_clock_t end;
} unit_test_t;

/**
 * Register a unit test.
 *
 * This macro allocates unit test descriptor, which is a structure of 
 * type unit_test_t. The descriptor contains information about a unit 
 * test, and the results from the last execution of the test.
 *
 * \param name The name of the unit test.
 * \param descr A string that briefly describes the unit test.
 */
#define UNIT_TEST_REGISTER(name, descr) static unit_test_t unit_test_##name = {descr, __FILE__, unit_test_success, 0, 0, 0}

/**
 * Define a unit test.
 *
 * This macro defines the function that will be executed when conducting 
 * a unit test. The name that is passed as a parameter must have been 
 * registered with the UNIT_TEST_REGISTER() macro.
 *
 * The function defined by this macro must start with a call to the
 * UNIT_TEST_BEGIN() macro, and end with a call to the UNIT_TEST_END() 
 * macro.
 *
 * The standard test function template produced by this macro will 
 * ensure that the unit test keeps track of the result, the time taken to 
 * execute (in rtimer ticks), and the exit point of the test. The 
 * latter corresponds to the line number at which the test was 
 * determined to be a success or failure.
 *
 * \param name The name of the unit test.
 */
#define UNIT_TEST(name) unit_test_result_t unit_test_function_##name(unit_test_t *utp)

/**
 * Mark the starting point of the unit test function.
 */
#define UNIT_TEST_BEGIN() do {                                                \
                            utp->start = RTIMER_NOW();                        \
                            utp->result = unit_test_success;                  \
                          } while(0)

/**
 * Mark the ending point of the unit test function.
 */
#define UNIT_TEST_END() UNIT_TEST_SUCCEED();                                  \
                        unit_test_end:                                        \
                          utp->end = RTIMER_NOW();                            \
                          return utp->result;

/*
 * The test result is printed with a function that is selected by 
 * defining UNIT_TEST_PRINT_FUNCTION, which must be of the type
 * "void (*)(const unit_test_t *)". The default selection is 
 * unit_test_print_report, which is available in unit-test.c.
 */
#ifndef UNIT_TEST_PRINT_FUNCTION
#define UNIT_TEST_PRINT_FUNCTION unit_test_print_report
#endif /* !UNIT_TEST_PRINT_FUNCTION */

/**
 * Print a report of the execution of a unit test.
 *
 * \param name The name of the unit test.
 */
#define UNIT_TEST_PRINT_REPORT(name) UNIT_TEST_PRINT_FUNCTION(&unit_test_##name)

/**
 * Execute a unit test and print a report on the results.
 *
 * \param name The name of the unit test.
 */
#define UNIT_TEST_RUN(name)  do {                                             \
                               unit_test_function_##name(&unit_test_##name);  \
                               UNIT_TEST_PRINT_REPORT(name);                  \
                             } while(0)

/**
 * Report that a unit test succeeded.
 *
 * This macro is useful for writing tests that can succeed earlier than 
 * the last execution point of the test, which is specified by a call to 
 * the UNIT_TEST_END() macro.
 *
 * Tests can usually be written without calls to UNIT_TEST_SUCCEED(), 
 * since it is implicitly called at the end of the test---unless 
 * UNIT_TEST_FAIL() has been called.
 * 
 */
#define UNIT_TEST_SUCCEED() do {                                              \
                              utp->exit_line = __LINE__;                      \
                              goto unit_test_end;                             \
                            } while(0)

/**
 * Report that a unit test failed.
 *
 * This macro is used to signal that a unit test failed to execute. The 
 * line number at which this macro was called is stored in the unit test 
 * descriptor.
 */
#define UNIT_TEST_FAIL() do {                                                 \
                           utp->exit_line = __LINE__;                         \
                           utp->result = unit_test_failure;                   \
                           goto unit_test_end;                                \
                         } while(0)

/**
 * Report failure if an expression is false.
 *
 * \param expr The expression to evaluate.
 */
#define UNIT_TEST_ASSERT(expr) do {                                           \
                                 if(!(expr)) {                                \
                                   UNIT_TEST_FAIL();                          \
                                 }                                            \
                               } while(0)

/**
 * Obtain the result of a certain unit test.
 *
 * If the unit test has not yet been executed, this macro returns 
 * unit_test_failed. Otherwise it returns the result of the last
 * execution of the unit test.
 *
 * \param name The name of the unit test.
 */
#define UNIT_TEST_RESULT(name) (unit_test_##name.result)

/* The default print function. */
void unit_test_print_report(const unit_test_t *utp);

#endif /* !UNIT_TEST_H */
