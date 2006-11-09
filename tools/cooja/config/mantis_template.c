/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: mantis_template.c,v 1.1 2006/11/09 19:32:53 fros4943 Exp $
 */

/**
 * \file
 *         C code template for generating Mantis source code files from COOJA
 *         Simulator. This file should not be compiled directly.
 * \author
 *         Fredrik Osterlind <fros@sics.se>
 */

#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "mos.h"
#include "sem.h"
#include "msched.h"

extern int main(); /* in mantis/src/mos/sys/main.c */
extern uint32_t cooja_time; /* in mantis/src/mos/sys/main.c */
extern pthread_cond_t cooja_cv; /* in mantis/src/mos/sys/main.c */
extern mos_thread_t threads[MAX_THREADS]; /* in mantis/src/mos/sys/main.c */

/*
 * referenceVar is used for comparing absolute and process relative memory.
 * (this must not be static due to memory locations)
 */
int referenceVar;

int dataVar = 1;
int i;

/*---------------------------------------------------------------------------*/
/**
 * \brief      Initialize a mote by starting processes etc.
 *
 *             This function initializes a mote by starting certain
 *             processes and setting up the environment.
 *
 *             This is a JNI function and should only be called via the
 *             responsible Java part (MoteType.java).
 */
JNIEXPORT void JNICALL
Java_se_sics_cooja_corecomm_[CLASS_NAME]_init(JNIEnv *env, jobject obj)
{
  main();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Get a segment from the process memory.
 * \param start Start address of segment
 * \param length Size of memory segment
 * \return     Java byte array containing a copy of memory segment.
 *
 *             Fetches a memory segment from the process memory starting at
 *             (start), with size (length). This function does not perform
 *             ANY error checking, and the process may crash if addresses are
 *             not available/readable.
 *
 *             This is a JNI function and should only be called via the
 *             responsible Java part (MoteType.java).
 */
JNIEXPORT void JNICALL
Java_se_sics_cooja_corecomm_[CLASS_NAME]_getMemory(JNIEnv *env, jobject obj, jint start, jint length, jbyteArray mem_arr)
{
  (*env)->SetByteArrayRegion(env, mem_arr, 0, (size_t) length, (jbyte *) start);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Replace a segment of the process memory with given byte array.
 * \param start Start address of segment
 * \param length Size of memory segment
 * \param mem_arr Byte array contaning new memory
 *
 *             Replaces a process memory segment with given byte array.
 *             This function does not perform ANY error checking, and the
 *             process may crash if addresses are not available/writable.
 *
 *             This is a JNI function and should only be called via the
 *             responsible Java part (MoteType.java).
 */
JNIEXPORT void JNICALL
Java_se_sics_cooja_corecomm_[CLASS_NAME]_setMemory(JNIEnv *env, jobject obj, jint start, jint length, jbyteArray mem_arr)
{
  jbyte *mem = (*env)->GetByteArrayElements(env, mem_arr, 0);
  memcpy((void *) start, mem, length);
  (*env)->ReleaseByteArrayElements(env, mem_arr, mem, 0);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Let mote execute one "block" of code (tick mote).
 *
 *             This is a JNI function and should only be called via the
 *             responsible Java part (MoteType.java).
 */
JNIEXPORT void JNICALL
Java_se_sics_cooja_corecomm_[CLASS_NAME]_tick(JNIEnv *env, jobject obj)
{
   int readyexist = 0;
	
   /* Ugly hack, setting all ready threads to BLOCKED just to discover which ones wake up */
   for(i=0; i<MAX_THREADS; i++) {
   	if (threads[i].state == READY) {
   	  threads[i].state = BLOCKED;
   	}
   }

  cooja_time += 1; // increase time
  //printf("CORE> tick, time is now %i\n", cooja_time);

   /* Signal all threads to start working again */
   for(i=0; i<MAX_THREADS; i++) {
   	  if (threads[i].state == BLOCKED) {
		mos_thread_resume(&threads[i]);
   	  }
   }

  /* Wait until all threads are blocked again. ??!?!!
   * For now we just assume all threads will be done within 1ms. */
   do {
   	 	readyexist=0;
	   for(i=0; i<MAX_THREADS; i++) {
	   	if (threads[i].state == READY) {
	   		readyexist = 1;
	   		//printf("waiting until thread finished...: %p\n", &threads[i]);
	   		//fflush(stdout);
	   		usleep(1);
	   	}
	   }
   } while (readyexist);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Get the absolute memory address of a special variable.
 * \return     Absolute memory address.
 *
 *             Returns the absolute memory address of a special variable
 *             "referenceVar". By comparing this address with the relative
 *             address (from the map file) for referenceVar, an runtime offset
 *             can be calculated.
 *
 *             This is a JNI function and should only be called via the
 *             responsible Java part (MoteType.java).
 */
JNIEXPORT jint JNICALL
Java_se_sics_cooja_corecomm_[CLASS_NAME]_getReferenceAbsAddr(JNIEnv *env, jobject obj)
{
  return (jint) &referenceVar;
}
