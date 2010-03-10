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
 * $Id: contiki_template.c,v 1.17 2010/03/10 12:22:11 fros4943 Exp $
 */

/**
 * \defgroup coojacore COOJA Simulator Core
 * @{
 */

/**
 * \file
 *         C code template for generating Contiki main startup files from
 *         inside the COOJA Simulator. This file should not be compiled directly.
 * \author
 *         Fredrik Osterlind <fros@sics.se>
 */

/*---------------------------------------------------------------------------*/
/**
 * \page coojacore COOJA Simulator Core
 *
 * COOJA Simulator Core ("the Core") represents the entire C code part of COOJA
 * Simulator.
 * This part shortly consists of simulated sensors and actuators and code for
 * communicating with the Java part.
 *
 * Communication with the Core should always be handled via dedicated classes
 * (MoteType), and never directly. MoteType works as an interface giving access
 * to variable's values given their name (through CoreComm->Lib?).
 *
 */

#include <jni.h>
#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "contiki-conf.h"
#include "sys/clock.h"
#include "sys/etimer.h"
#include "sys/cooja_mt.h"
#include "dev/serial-line.h"
#include "dev/cooja-radio.h"
#include "lib/simEnvChange.h"
#include "lib/sensors.h"
#include "net/netstack.h"
#include "node-id.h"

#ifndef WITH_UIP
#define WITH_UIP 0
#endif
#if WITH_UIP
#include "net/uip.h"
#include "net/uip-fw.h"
#include "net/uip-fw-drv.h"
#include "net/uip-driver.h"
#include "dev/slip.h"
static struct uip_fw_netif wsn_if =
  {UIP_FW_NETIF(172,16,0,0, 255,255,0,0, uip_driver_send)};
static struct uip_fw_netif slip_if =
  {UIP_FW_NETIF(0,0,0,0, 0,0,0,0, slip_send)};
#endif /* WITH_UIP */

#ifndef WITH_UIP6
#define WITH_UIP6 0
#endif
#if WITH_UIP6
#include "net/uip-netif.h"
#define PRINT6ADDR(addr) printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#endif /* WITH_UIP6 */

PROCINIT(&etimer_process,&sensors_process);

/* Declare all sensors */
[SENSOR_DEFINITIONS]

/* Declare all simulation interfaces */
[INTERFACE_DEFINITIONS]


/* Create sensor array */
[SENSOR_ARRAY]

/* Create simulation interfaces array */
[INTERFACE_ARRAY]

/*
 * referenceVar is used for comparing absolute and process relative memory.
 * (this must not be static due to memory locations)
 */
long referenceVar;

/*
 * process_run() infinite loop.
 * Is yielded at least once per function call.
 */
static struct cooja_mt_thread process_run_thread;

/*---------------------------------------------------------------------------*/
static void
print_processes(struct process * const processes[])
{
  /*  const struct process * const * p = processes;*/
  printf("Starting");
  while(*processes != NULL) {
    printf(" '%s'", (*processes)->name);
    processes++;
  }
  putchar('\n');
}
/*---------------------------------------------------------------------------*/
static void
start_process_run_loop(void *data)
{
    /* Yield once during bootup */
    simProcessRunValue = 1;
    cooja_mt_yield();

    /* Initialize random generator */
    random_init(0);

    /* Start process handler */
    process_init();

    /* Start Contiki processes */
    procinit_init();

    /* Print startup information */
    printf(CONTIKI_VERSION_STRING " started. ");
    if(node_id > 0) {
      printf("Node id is set to %u.\n", node_id);
    } else {
      printf("Node id is not set.\n");
    }

    /* RIME CONFIGURATION */
    {
      int i;
      rimeaddr_t rimeaddr;

      /* Init Rime */
      ctimer_init();
      rimeaddr.u8[0] = node_id & 0xff;
      rimeaddr.u8[1] = node_id >> 8;
      rimeaddr_set_node_addr(&rimeaddr);
      printf("Rime address: ");
      for(i = 0; i < sizeof(rimeaddr_node_addr.u8) - 1; i++) {
        printf("%d.", rimeaddr_node_addr.u8[i]);
      }
      printf("%d\n", rimeaddr_node_addr.u8[i]);
    }

    queuebuf_init();

#if WITH_UIP
    /* IPv4 CONFIGURATION */
    {
      uip_ipaddr_t hostaddr, netmask;
      process_start(&tcpip_process, NULL);
      process_start(&uip_fw_process, NULL);
      process_start(&slip_process, NULL);
      uip_init();
      uip_fw_init();
      uip_ipaddr(&hostaddr, 172, 16, rimeaddr_node_addr.u8[1], rimeaddr_node_addr.u8[0]);
      uip_ipaddr(&netmask, 255,255,0,0);
      uip_sethostaddr(&hostaddr);
      uip_setnetmask(&netmask);
      uip_fw_register(&wsn_if);
      uip_fw_default(&slip_if);
      rs232_set_input(slip_input_byte);
      printf("IPv4 address: %d.%d.%d.%d\n", uip_ipaddr_to_quad(&hostaddr));
    }
#endif /* WITH_UIP */

#if WITH_UIP6
    /* IPv6 CONFIGURATION */
    {
      int i;
      uint8_t addr[sizeof(uip_lladdr.addr)];
      for (i=0; i < sizeof(uip_lladdr.addr); i++) {
        addr[i] = node_id & 0xff;
      }
      memcpy(&uip_lladdr.addr, addr, sizeof(uip_lladdr.addr));
      process_start(&tcpip_process, NULL);
      printf("IPv6 address: ");
      PRINT6ADDR(&uip_netif_physical_if.addresses[0].ipaddr);
      printf("\n");
    }
#endif /* WITH_UIP6 */

    /* Initialize communication stack */
    netstack_init();

    /* Start serial process */
    serial_line_init();

    /* Start autostart processes (defined in Contiki application) */
    print_processes(autostart_processes);
    autostart_start(autostart_processes);

    while(1)
	{
		simProcessRunValue = process_run();
		while(simProcessRunValue-- > 0) {
		  process_run();
		}
		simProcessRunValue = process_nevents();

		// Check if we must stay awake
		if(simDontFallAsleep) {
			simDontFallAsleep=0;
			simProcessRunValue = 1;
		}

		/* Return to COOJA */
		cooja_mt_yield();
	}
}

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
  /* Prepare thread that will do the process_run()-loop */
  cooja_mt_start(&process_run_thread, &start_process_run_loop, NULL);
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
/**
 * \brief      Let mote execute one "block" of code (tick mote).
 *
 *             Let mote defined by the active contiki processes and current
 *             process memory execute some program code. This code must not block
 *             or else this function will never return. A typical contiki
 *             process will return when it executes PROCESS_WAIT..() statements.
 *
 *             Before the control is left to contiki processes, any messages
 *             from the Java part are handled. These may for example be
 *             incoming network data. After the contiki processes return control,
 *             messages to the Java part are also handled (those which may need
 *             special attention).
 *
 *             This is a JNI function and should only be called via the
 *             responsible Java part (MoteType.java).
 */
JNIEXPORT void JNICALL
Java_se_sics_cooja_corecomm_[CLASS_NAME]_tick(JNIEnv *env, jobject obj)
{
  /* Let all simulation interfaces act first */
  doActionsBeforeTick();

  /* Poll etimer process */
  if (etimer_pending()) {
    etimer_request_poll();
  }

  /* Let Contiki handle a few events.
  	This call stores the process_run() return value */
  cooja_mt_exec(&process_run_thread);

  /* Let all simulation interfaces act before returning to java */
  doActionsAfterTick();

  /* Look for new e-timers */
  simEtimerPending = etimer_pending();

  /* Save nearest event timer expiration time */
  if (simEtimerPending) {
    simNextExpirationTime = etimer_next_expiration_time() - simCurrentTime;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Set the relative memory address of the reference variable.
 * \return     Relative memory address.
 *
 *             This is a JNI function and should only be called via the
 *             responsible Java part (MoteType.java).
 */
JNIEXPORT void JNICALL
Java_se_sics_cooja_corecomm_[CLASS_NAME]_setReferenceAddress(JNIEnv *env, jobject obj, jint addr)
{
  referenceVar = (((long)&referenceVar) - ((long)addr));
}

/** @} */
