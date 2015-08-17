/*****************************************************************************
 *
 * MODULE:             Debug tracing
 *
 * COMPONENT:          dbg.h
 *
 * AUTHOR:             MRR
 *
 * DESCRIPTION:        Provide printf style debug tracing
 *
 * $HeadURL: http://svn/sware/Projects/Cortex/Modules/DBG/Tags/CORTEX_DBG_1v2_RC1/Include/dbg.h $
 *
 * $Revision: 21882 $
 *
 * $LastChangedBy: mwild $
 *
 * $LastChangedDate: 2009-11-13 13:30:19 +0000 (Fri, 13 Nov 2009) $
 *
 * $Id: dbg.h 21882 2009-11-13 13:30:19Z mwild $
 *
 *****************************************************************************
 *
 * This software is owned by Jennic and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on Jennic products. You, and any third parties must reproduce
 * the copyright and warranty notice and any other legend of ownership on each
 * copy or partial copy of the software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS". JENNIC MAKES NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * ACCURACY OR LACK OF NEGLIGENCE. JENNIC SHALL NOT, IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT LIMITED TO, SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON WHATSOEVER.
 *
 * Copyright Jennic Ltd. 2007 All rights reserved
 *
 ****************************************************************************/

#ifndef DBG_H_
#define DBG_H_

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

#include "common.h"

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef __GNUC__

#ifdef DBG_VERBOSE
/* note - a warning will be issued if ASSERTION evaluates to FALSE at
 * compile time */
#define DBG_vAssert(STREAM, ASSERTION)                              \
    do {                                                            \
        const bool_t bAssertion = (ASSERTION);                      \
        typedef char DBG_STATIC_ASSERT_FAILED[bAssertion ? 0 : -1]; \
        bool_t bEval = ((STREAM) && !bAssertion) ?                  \
                dbg_vAssertImpl(__FILE__,                           \
                                __LINE__,                           \
                                #ASSERTION), TRUE : FALSE;          \
        bEval = bEval;                                              \
    } while(0)
#else
/* note - a warning will be issued if ASSERTION evaluates to FALSE at
 * compile time */
#define DBG_vAssert(STREAM, ASSERTION)                              \
    do {                                                            \
        const bool_t bAssertion = (ASSERTION);                      \
        typedef char DBG_STATIC_ASSERT_FAILED[bAssertion ? 0 : -1]; \
        bool_t bEval = ((STREAM) && !bAssertion) ?                  \
                dbg_vAssertImpl("",                                 \
                                0,                                  \
                                #ASSERTION), TRUE : FALSE;          \
        bEval = bEval;                                              \
    } while(0)
#endif

/* need to write this in such a way that FORMAT doesn't get linked in
 * if STREAM isn't enabled, but all the ARGS still get evaluated in
 * case they may cause side effects
 * (may still need work to make this portable to non GCC compilers) */
#ifdef DBG_VERBOSE
#define DBG_vPrintf(STREAM, FORMAT, ARGS...)                        \
    do {                                                            \
        const char *pccFormatIn = (FORMAT);                         \
        const char *pccFormat = (STREAM) ?                          \
            pccFormatIn : NULL;                                     \
        dbg_vPrintfImpl(__FILE__, __LINE__, pccFormat, ## ARGS);    \
    } while(0)
#else
#define DBG_vPrintf(STREAM, FORMAT, ARGS...)                        \
    do {                                                            \
        const char *pccFormatIn = (FORMAT);                         \
        const char *pccFormat = (STREAM) ?                          \
            pccFormatIn : NULL;                                     \
        (STREAM) ?                                                  \
            dbg_vPrintfImpl("",0, pccFormat, ## ARGS) :             \
            dbg_vPrintfNullImpl("",0, pccFormat, ## ARGS);          \
    } while(0)
#endif

/* define empty functions as inline, so they
 * 1) don't take up any space
 * 2) don't attempt to link to the library functions */
#define DBG_INLINE static inline __attribute__((always_inline))

/* set up printf style warnings */
#define DBG_FORMAT_PRINTF(A, B)
//__attribute__((format (__printf__, A, B)))

#else

#ifdef DBG_VERBOSE
/* note - a warning will be issued if ASSERTION evaluates to FALSE at
 * compile time */
#define DBG_vAssert(STREAM, ASSERTION)                              \
    do {                                                            \
        const bool_t bAssertion = (ASSERTION);                      \
        bool_t bEval = ((STREAM) && !bAssertion) ?                  \
                dbg_vAssertImpl(__FILE__,                           \
                                __LINE__,                           \
                                #ASSERTION), TRUE : FALSE;          \
        bEval = bEval;                                              \
    } while(0)
#else
/* note - a warning will be issued if ASSERTION evaluates to FALSE at
 * compile time */
#define DBG_vAssert(STREAM, ASSERTION)                              \
    do {                                                            \
        const bool_t bAssertion = (ASSERTION);                      \
        bool_t bEval = ((STREAM) && !bAssertion) ?                  \
                dbg_vAssertImpl("",                                 \
                                0,                                  \
                                #ASSERTION), TRUE : FALSE;          \
        bEval = bEval;                                              \
    } while(0)
#endif

/* need to write this in such a way that FORMAT doesnt get linked in
 * if STREAM isnt enabled, but all the ARGS still get evaluated in
 * case they may cause side effects
 * (may still need work to make this portable to non GCC compilers) */
#ifdef DBG_VERBOSE
#define DBG_vPrintf                                                \
        dbg_vSetFileLine(__FILE__, __LINE__);                      \
        dbg_vPrintfImpl2
#else
#define DBG_vPrintf                                                \
        dbg_vSetFileLine("",0);                                    \
        dbg_vPrintfImpl2
#endif

#define DBG_INLINE static
#define DBG_FORMAT_PRINTF(A, B)
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef struct
{
    void (*prInitHardwareCb)(void);
    void (*prPutchCb)       (char c);
    void (*prFlushCb)       (void);
    void (*prFailedAssertCb)(void);

} tsDBG_FunctionTbl;


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

#if defined(DBG_ENABLE) || defined(DBG_C_)

PUBLIC void DBG_vInit(tsDBG_FunctionTbl *psFunctionTbl);

PUBLIC void DBG_vReset(void);

void DBG_vDumpStack(void);

#ifndef __GNUC__
PUBLIC void dbg_vSetFileLine(const char   *pcFileName,
                             uint32_t      u32LineNumber);

DBG_FORMAT_PRINTF(3, 4)
PUBLIC void dbg_vPrintfImpl2(bool_t bStream,
                             const char   *pcFormat,
                             ...);
#endif

DBG_FORMAT_PRINTF(3, 4)
PUBLIC void dbg_vPrintfImpl(const char    *Private_pcSrcFileName,
                            uint32_t       Private_u32SrcLineNumber,
                            const char    *pcFormat,
                            ...);

PUBLIC void dbg_vAssertImpl(const char    *Private_pcSrcFileName,
                            uint32_t       Private_u32SrcLineNumber,
                            const char    *pcAssertionString);
#else /* defined(DBG_ENABLE) || defined(DBG_C_) */

/* need to make sure the parameters are still evaluated in a safe scope */
#define DBG_vInit(psFunctionTbl) ((void)(psFunctionTbl))

#define DBG_vReset() ((void)0)

#define DBG_vDumpStack() ((void)0)

#ifndef __GNUC__
DBG_INLINE
PUBLIC void dbg_vSetFileLine(const char   *pcFileName,
                             uint32_t      u32LineNumber){}

DBG_FORMAT_PRINTF(3, 4)
DBG_INLINE
PUBLIC void dbg_vPrintfImpl2(bool_t bStream,
                             const char   *pcFormat,
                             ...){}
#endif

DBG_FORMAT_PRINTF(3, 4)
DBG_INLINE
PUBLIC void dbg_vPrintfImpl(const char    *Private_pcSrcFileName,
                            uint32_t       Private_u32SrcLineNumber,
                            const char    *pcFormat,
                            ...){}

DBG_INLINE
PUBLIC void dbg_vAssertImpl(const char    *Private_pcSrcFileName,
                            uint32_t       Private_u32SrcLineNumber,
                            const char    *pcAssertionString){}
#endif /* defined(DBG_ENABLE) || defined(DBG_C_) */

DBG_INLINE
PUBLIC void dbg_vPrintfNullImpl(const char    *Private_pcSrcFileName,
                            uint32_t       Private_u32SrcLineNumber,
                            const char    *pcFormat,
                            ...){}

DBG_INLINE
PUBLIC void dbg_vAssertNullImpl(const char    *Private_pcSrcFileName,
                            uint32_t       Private_u32SrcLineNumber,
                            const char    *pcAssertionString){}

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

extern const char DBG_gszVERSION[];

#if defined __cplusplus
};
#endif

#endif /*DBG_H_*/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
