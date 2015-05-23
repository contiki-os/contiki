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

package org.contikios.cooja.contikimote.interfaces;

import org.apache.log4j.Logger;

import org.contikios.cooja.*;
import org.contikios.cooja.contikimote.ContikiMote;
import org.contikios.cooja.contikimote.ContikiMoteInterface;
import org.contikios.cooja.interfaces.Button;
import org.contikios.cooja.mote.memory.VarMemory;

/**
 * Button mote interface.
 *
 * Contiki variables:
 * <ul>
 * <li>char simButtonIsDown (1=down, else up)
 * <li>char simButtonChanged (1=changed, else not changed)
 * <li>char simButtonIsActive (1=active, else inactive)
 * </ul>
 *
 * Core interface:
 * <ul>
 * <li>button_interface
 * </ul>
 * <p>
 *
 * This observable notifies when the button is pressed or released.
 *
 * @author Fredrik Osterlind
 */
public class ContikiButton extends Button implements ContikiMoteInterface {
  private final VarMemory moteMem;
  private final ContikiMote mote;

  private static final Logger logger = Logger.getLogger(ContikiButton.class);

  /**
   * Creates an interface to the button at mote.
   *
   * @param mote Mote
   * @see Mote
   * @see org.contikios.cooja.MoteInterfaceHandler
   */
  public ContikiButton(Mote mote) {
    super(mote);
    this.mote = (ContikiMote) mote;
    this.moteMem = new VarMemory(mote.getMemory());
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"button_interface"};
  }

  @Override
  protected void doReleaseButton() {
    moteMem.setByteValueOf("simButtonIsDown", (byte) 0);

    if (moteMem.getByteValueOf("simButtonIsActive") == 1) {
      moteMem.setByteValueOf("simButtonChanged", (byte) 1);

      /* If mote is inactive, wake it up */
      mote.requestImmediateWakeup();

      setChanged();
      notifyObservers();
    }
  }

  @Override
  protected void doPressButton() {
    moteMem.setByteValueOf("simButtonIsDown", (byte) 1);

    if (moteMem.getByteValueOf("simButtonIsActive") == 1) {
      moteMem.setByteValueOf("simButtonChanged", (byte) 1);

      /* If mote is inactive, wake it up */
      mote.requestImmediateWakeup();

      setChanged();
      notifyObservers();
    }
  }

  @Override
  public boolean isPressed() {
    return moteMem.getByteValueOf("simButtonIsDown") == 1;
  }

}
