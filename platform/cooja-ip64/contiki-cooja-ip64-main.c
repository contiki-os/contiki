/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 *         COOJA Contiki mote main file.
 * \author
 *         Fredrik Osterlind <fros@sics.se>
 */

#include <jni.h>
#include <stdio.h>
#include <string.h>

#include "contiki.h"

#include "sys/clock.h"
#include "sys/etimer.h"
#include "sys/cooja_mt.h"
#include "sys/autostart.h"

#include "lib/random.h"
#include "lib/simEnvChange.h"

#include "net/rime/rime.h"
#include "net/netstack.h"
#include "net/ip/uip-nameserver.h"

#include "dev/serial-line.h"
#include "dev/cooja-radio.h"
#include "dev/button-sensor.h"
#include "dev/pir-sensor.h"
#include "dev/vib-sensor.h"

#include "sys/node-id.h"

#include "ip64.h"
#include "dev/slip.h"

/* JNI-defined functions, depends on the environment variable CLASSNAME */
#ifndef CLASSNAME
#error CLASSNAME is undefined, required by contiki-cooja-main.c
#endif /* CLASSNAME */
#define COOJA__QUOTEME(a,b,c) COOJA_QUOTEME(a,b,c)
#define COOJA_QUOTEME(a,b,c) a##b##c
#define COOJA_JNI_PATH Java_org_contikios_cooja_corecomm_
#define Java_org_contikios_cooja_corecomm_CLASSNAME_init COOJA__QUOTEME(COOJA_JNI_PATH,CLASSNAME,_init)
#define Java_org_contikios_cooja_corecomm_CLASSNAME_getMemory COOJA__QUOTEME(COOJA_JNI_PATH,CLASSNAME,_getMemory)
#define Java_org_contikios_cooja_corecomm_CLASSNAME_setMemory COOJA__QUOTEME(COOJA_JNI_PATH,CLASSNAME,_setMemory)
#define Java_org_contikios_cooja_corecomm_CLASSNAME_tick COOJA__QUOTEME(COOJA_JNI_PATH,CLASSNAME,_tick)
#define Java_org_contikios_cooja_corecomm_CLASSNAME_setReferenceAddress COOJA__QUOTEME(COOJA_JNI_PATH,CLASSNAME,_setReferenceAddress)

#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#define PRINT6ADDR(addr) printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])

/* Simulation mote interfaces */
SIM_INTERFACE_NAME(moteid_interface);
SIM_INTERFACE_NAME(vib_interface);
SIM_INTERFACE_NAME(rs232_interface);
SIM_INTERFACE_NAME(simlog_interface);
SIM_INTERFACE_NAME(beep_interface);
SIM_INTERFACE_NAME(radio_interface);
SIM_INTERFACE_NAME(button_interface);
SIM_INTERFACE_NAME(pir_interface);
SIM_INTERFACE_NAME(clock_interface);
SIM_INTERFACE_NAME(leds_interface);
SIM_INTERFACE_NAME(cfs_interface);
SIM_INTERFACES(&vib_interface, &moteid_interface, &rs232_interface, &simlog_interface, &beep_interface, &radio_interface, &button_interface, &pir_interface, &clock_interface, &leds_interface, &cfs_interface);
/* Example: manually add mote interfaces */
//SIM_INTERFACE_NAME(dummy_interface);
//SIM_INTERFACES(..., &dummy_interface);

/* Sensors */
SENSORS(&button_sensor, &pir_sensor, &vib_sensor);

/*
 * referenceVar is used for comparing absolute and process relative memory.
 * (this must not be static due to memory locations)
 */
long referenceVar;

/*
 * Contiki and rtimer threads.
 */
static struct cooja_mt_thread rtimer_thread;
static struct cooja_mt_thread process_run_thread;

#define MIN(a, b)   ( (a)<(b) ? (a) : (b) )

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
rtimer_thread_loop(void *data)
{
  while(1)
  {
    rtimer_arch_check();

    /* Return to COOJA */
    cooja_mt_yield();
  }
}
/*---------------------------------------------------------------------------*/
static void
set_mac_addr(void)
{
  linkaddr_t addr;
  int i;

  memset(&addr, 0, sizeof(linkaddr_t));
  for(i = 0; i < sizeof(uip_lladdr.addr); i += 2) {
    addr.u8[i + 1] = node_id & 0xff;
    addr.u8[i + 0] = node_id >> 8;
  }
  linkaddr_set_node_addr(&addr);
  printf("MAC address ");
  for(i = 0; i < sizeof(addr.u8) - 1; i++) {
    printf("%d.", addr.u8[i]);
  }
  printf("%d\n", addr.u8[i]);
}
/*---------------------------------------------------------------------------*/
void
contiki_init(void)
{
  int i;
  uint8_t addr[sizeof(uip_lladdr.addr)];
  uip_ipaddr_t ipaddr;
  uip_ds6_addr_t *lladdr;
  uip_ip4addr_t ipv4addr, netmask;

  /* Start process handler */
  process_init();

  /* Start Contiki processes */
  process_start(&etimer_process, NULL);
  process_start(&sensors_process, NULL);
  ctimer_init();

  /* Print startup information */
  printf(CONTIKI_VERSION_STRING " started. ");
  if(node_id > 0) {
    printf("Node id is set to %u.\n", node_id);
  } else {
    printf("Node id is not set.\n");
  }

  set_mac_addr();

  queuebuf_init();

  /* Initialize communication stack */
  netstack_init();
  printf("%s/%s/%s, channel check rate %lu Hz\n",
         NETSTACK_NETWORK.name, NETSTACK_MAC.name, NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1:
                         NETSTACK_RDC.channel_check_interval()));

  /* IPv6 CONFIGURATION */


  for(i = 0; i < sizeof(uip_lladdr.addr); i += 2) {
    addr[i + 1] = node_id & 0xff;
    addr[i + 0] = node_id >> 8;
  }
  linkaddr_copy(addr, &linkaddr_node_addr);
  memcpy(&uip_lladdr.addr, addr, sizeof(uip_lladdr.addr));

  process_start(&tcpip_process, NULL);

  printf("Tentative link-local IPv6 address ");

  lladdr = uip_ds6_get_link_local(-1);
  for(i = 0; i < 7; ++i) {
    printf("%02x%02x:", lladdr->ipaddr.u8[i * 2],
           lladdr->ipaddr.u8[i * 2 + 1]);
  }
  printf("%02x%02x\n", lladdr->ipaddr.u8[14],
         lladdr->ipaddr.u8[15]);

  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);
  printf("Tentative global IPv6 address ");
  for(i = 0; i < 7; ++i) {
    printf("%02x%02x:",
           ipaddr.u8[i * 2], ipaddr.u8[i * 2 + 1]);
  }
  printf("%02x%02x\n",
         ipaddr.u8[7 * 2], ipaddr.u8[7 * 2 + 1]);

  /* Start serial process */
  serial_line_init();

  /* Start autostart processes (defined in Contiki application) */
  print_processes(autostart_processes);
  autostart_start(autostart_processes);

  /* Start the SLIP */
  printf("Initiating SLIP with IP address is 172.16.0.2.\n");

  uip_ipaddr(&ipv4addr, 172, 16, 0, 2);
  uip_ipaddr(&netmask, 255, 255, 255, 0);
  ip64_set_ipv4_address(&ipv4addr, &netmask);

  rs232_set_input(slip_input_byte);
  log_set_putchar_with_slip(1);

  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;

  uip_ipaddr(&ip4addr, 8,8,8,8);
  ip64_addr_4to6(&ip4addr, &ip6addr);

  uip_nameserver_update((uip_ipaddr_t *)&ip6addr, UIP_NAMESERVER_INFINITE_LIFETIME);
}
/*---------------------------------------------------------------------------*/
static void
process_run_thread_loop(void *data)
{
  /* Yield once during bootup */
  simProcessRunValue = 1;
  cooja_mt_yield();

  contiki_init();

  while(1) {
    simProcessRunValue = process_run();
    while(simProcessRunValue-- > 0) {
      process_run();
    }
    simProcessRunValue = process_nevents();

    /* Check if we must stay awake */
    if(simDontFallAsleep) {
      simDontFallAsleep = 0;
      simProcessRunValue = 1;
    }

    /* Return to COOJA */
    cooja_mt_yield();
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Initialize a mote by starting processes etc.
 * \param env  JNI Environment interface pointer
 * \param obj  unused
 *
 *             This function initializes a mote by starting certain
 *             processes and setting up the environment.
 *
 *             This is a JNI function and should only be called via the
 *             responsible Java part (MoteType.java).
 */
JNIEXPORT void JNICALL
Java_org_contikios_cooja_corecomm_CLASSNAME_init(JNIEnv *env, jobject obj)
{
  /* Create rtimers and Contiki threads */
  cooja_mt_start(&rtimer_thread, &rtimer_thread_loop, NULL);
  cooja_mt_start(&process_run_thread, &process_run_thread_loop, NULL);
 }
/*---------------------------------------------------------------------------*/
/**
 * \brief      Get a segment from the process memory.
 * \param env      JNI Environment interface pointer
 * \param obj      unused
 * \param rel_addr Start address of segment
 * \param length   Size of memory segment
 * \param mem_arr  Byte array destination for the fetched memory segment
 * \return     Java byte array containing a copy of memory segment.
 *
 *             Fetches a memory segment from the process memory starting at
 *             (rel_addr), with size (length). This function does not perform
 *             ANY error checking, and the process may crash if addresses are
 *             not available/readable.
 *
 *             This is a JNI function and should only be called via the
 *             responsible Java part (MoteType.java).
 */
JNIEXPORT void JNICALL
Java_org_contikios_cooja_corecomm_CLASSNAME_getMemory(JNIEnv *env, jobject obj, jint rel_addr, jint length, jbyteArray mem_arr)
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
 * \param env      JNI Environment interface pointer
 * \param obj      unused
 * \param rel_addr Start address of segment
 * \param length   Size of memory segment
 * \param mem_arr  Byte array contaning new memory
 *
 *             Replaces a process memory segment with given byte array.
 *             This function does not perform ANY error checking, and the
 *             process may crash if addresses are not available/writable.
 *
 *             This is a JNI function and should only be called via the
 *             responsible Java part (MoteType.java).
 */
JNIEXPORT void JNICALL
Java_org_contikios_cooja_corecomm_CLASSNAME_setMemory(JNIEnv *env, jobject obj, jint rel_addr, jint length, jbyteArray mem_arr)
{
  jbyte *mem = (*env)->GetByteArrayElements(env, mem_arr, 0);
  memcpy((char *)(((long)rel_addr) + referenceVar),
         mem,
         length);
  (*env)->ReleaseByteArrayElements(env, mem_arr, mem, 0);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Let mote execute one "block" of code (tick mote).
 * \param env  JNI Environment interface pointer
 * \param obj  unused
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
Java_org_contikios_cooja_corecomm_CLASSNAME_tick(JNIEnv *env, jobject obj)
{
  simProcessRunValue = 0;

  /* Let all simulation interfaces act first */
  doActionsBeforeTick();

  /* Poll etimer process */
  if(etimer_pending()) {
    etimer_request_poll();
  }

  /* Let rtimers run.
   * Sets simProcessRunValue */
  cooja_mt_exec(&rtimer_thread);

  if(simProcessRunValue == 0) {
    /* Rtimers done: Let Contiki handle a few events.
     * Sets simProcessRunValue */
    cooja_mt_exec(&process_run_thread);
  }

  /* Let all simulation interfaces act before returning to java */
  doActionsAfterTick();

  /* Do we have any pending timers */
  simEtimerPending = etimer_pending();

  /* Save nearest expiration time */
  simEtimerNextExpirationTime = etimer_next_expiration_time();

}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Set the relative memory address of the reference variable.
 * \param env  JNI Environment interface pointer
 * \param obj  unused
 * \param addr Relative memory address
 *
 *             This is a JNI function and should only be called via the
 *             responsible Java part (MoteType.java).
 */
JNIEXPORT void JNICALL
Java_org_contikios_cooja_corecomm_CLASSNAME_setReferenceAddress(JNIEnv *env, jobject obj, jint addr)
{
  referenceVar = (((long)&referenceVar) - ((long)addr));
}
