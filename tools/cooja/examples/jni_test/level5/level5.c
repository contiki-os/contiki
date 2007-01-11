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
 * $Id: level5.c,v 1.2 2007/01/11 14:27:26 fros4943 Exp $
 */

#include <jni.h>
#include <stdio.h>
#include <string.h>

int ref_var;

int initialized_counter=1;
int uninitialized_counter;

JNIEXPORT void JNICALL
Java_Level5_doCount(JNIEnv *env, jobject obj)
{
 fprintf(stderr, ">> DATA_counter=\t%i\tBSS_counter=\t%i\n", ++initialized_counter, ++uninitialized_counter);
 fflush(stderr);
}
JNIEXPORT jint JNICALL
Java_Level5_getRefAddress(JNIEnv *env, jobject obj)
{
  return (jint) &ref_var;
}

JNIEXPORT jbyteArray JNICALL
Java_Level5_getMemory(JNIEnv *env, jobject obj, jint start, jint length)
{
  jbyteArray ret=(*env)->NewByteArray(env, length);
  (*env)->SetByteArrayRegion(env, ret, 0, (size_t) length, (jbyte *) start);

  return (ret);
}

JNIEXPORT void JNICALL
Java_Level5_setMemory(JNIEnv *env, jobject obj, jint start, jint length, jbyteArray mem_arr)
{
  jbyte *mem = (*env)->GetByteArrayElements(env, mem_arr, 0);
  memcpy((void *) start, mem, length);
  (*env)->ReleaseByteArrayElements(env, mem_arr, mem, 0);
}
