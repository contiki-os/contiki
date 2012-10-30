/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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

package se.sics.cooja.contikimote.interfaces;

import org.apache.log4j.Logger;
import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.IPAddress;

/**
 * uIP IP address.
 *
 * Contiki variables (currently not activated):
 * <ul>
 * <li>char simIP[4], or char simIP[16]
 * <li>int simIPv4
 * <li>int simIPv6
 * <li>char simIPChanged (1 if new IP should be set)
 * </ul>
 * <p>
 *
 * Core interface:
 * <ul>
 * <li>ip_interface
 * </ul>
 *
 * This observable notifies when the IP address is set.
 * Note that this mote interface does not detect if Contiki changes IP address at run-time.
 *
 * @author Fredrik Osterlind
 */
public class ContikiIPAddress extends IPAddress implements ContikiMoteInterface {
  private static Logger logger = Logger.getLogger(ContikiIPAddress.class);

  /**
   * Creates an interface to the IP address at mote.
   *
   * @param mote IP address' mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiIPAddress(final Mote mote) {
    super(mote);
  }

  public static String[] getCoreInterfaceDependencies() {
    /*return new String[]{"ip_interface"};*/
    return null;
  }
}
