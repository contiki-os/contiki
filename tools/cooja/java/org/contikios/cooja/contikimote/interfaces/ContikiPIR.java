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

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Collection;
import javax.swing.JButton;
import javax.swing.JPanel;
import org.jdom.Element;
import org.contikios.cooja.Mote;
import org.contikios.cooja.mote.memory.SectionMoteMemory;
import org.contikios.cooja.contikimote.ContikiMote;
import org.contikios.cooja.contikimote.ContikiMoteInterface;
import org.contikios.cooja.interfaces.PIR;
import org.contikios.cooja.mote.memory.VarMemory;

/**
 * Passive IR sensor mote interface.
 *
 * Contiki variables:
 * <ul>
 * <li>char simPirChanged (1=changed, else not changed)
 * <li>char simPirIsActive (1=active, else inactive)
 * </ul>
 * <p>
 *
 * Core interface:
 * <ul>
 * <li>pir_interface
 * </ul>
 * <p>
 *
 * This observable notifies if PIR triggers.
 *
 * @author Fredrik Osterlind
 */
public class ContikiPIR extends PIR implements ContikiMoteInterface {

  private ContikiMote mote;
  private VarMemory moteMem;

  /**
   * Creates an interface to the PIR at mote.
   *
   * @param mote Mote
   *
   * @see Mote
   * @see org.contikios.cooja.MoteInterfaceHandler
   */
  public ContikiPIR(Mote mote) {
    this.mote = (ContikiMote) mote;
    this.moteMem = new VarMemory(mote.getMemory());
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"pir_interface"};
  }

  /**
   * Simulates a change in the PIR sensor.
   */
  public void triggerChange() { 
    mote.getSimulation().invokeSimulationThread(new Runnable() {
      public void run() {
        doTriggerChange();
      }
    });
  }

  public void doTriggerChange() { 
    if (moteMem.getByteValueOf("simPirIsActive") == 1) {
      moteMem.setByteValueOf("simPirChanged", (byte) 1);

      mote.requestImmediateWakeup();
    }
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JButton clickButton = new JButton("Signal PIR");

    panel.add(clickButton);

    clickButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        triggerChange();
      }
    });

    return panel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}
