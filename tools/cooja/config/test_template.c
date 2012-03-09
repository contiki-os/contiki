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
 * $Id: test_template.c,v 1.1 2009/02/20 16:59:52 fros4943 Exp $
 */

#include <jni.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* This is a stripped version of platform/cooja/contiki-cooja-main.c, used by
 * Cooja's Contiki Mote Configuration Wizard */

const struct simInterface *simInterfaces[] = {NULL};

long referenceVar; /* Placed somewhere in the BSS section */

/* Variables with known memory addresses */
int var1=1;
int var2=2;
int var3=2;
int arr1[10];
int arr2[10];
int uvar1;
int uvar2;

JNIEXPORT void JNICALL
Java_se_sics_cooja_corecomm_[CLASS_NAME]_init(JNIEnv *env, jobject obj)
{
 }
/*---------------------------------------------------------------------------*/
JNIEXPORT void JNICALL
Java_se_sics_cooja_corecomm_[CLASS_NAME]_getMemory(JNIEnv *env, jobject obj, jint rel_addr, jint length, jbyteArray mem_arr)
{
  (*env)->SetByteArrayRegion(
      env,
      mem_arr,
      0,
      (size_t) length,
      (jbyte *) (((long)rel_addr) + referenceVar)
  );

}
/*---------------------------------------------------------------------------*/
JNIEXPORT void JNICALL
Java_se_sics_cooja_corecomm_[CLASS_NAME]_setMemory(JNIEnv *env, jobject obj, jint rel_addr, jint length, jbyteArray mem_arr)
{
  jbyte *mem = (*env)->GetByteArrayElements(env, mem_arr, 0);
  memcpy(
      (char*) (((long)rel_addr) + referenceVar),
      mem,
      length);
  (*env)->ReleaseByteArrayElements(env, mem_arr, mem, 0);
}
/*---------------------------------------------------------------------------*/
JNIEXPORT void JNICALL
Java_se_sics_cooja_corecomm_[CLASS_NAME]_tick(JNIEnv *env, jobject obj)
{
  ++var1;
  ++uvar1;
}
/*---------------------------------------------------------------------------*/
JNIEXPORT void JNICALL
Java_se_sics_cooja_corecomm_[CLASS_NAME]_setReferenceAddress(JNIEnv *env, jobject obj, jint addr)
{
  referenceVar = (((long)&referenceVar) - ((long)addr));
}
