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

import java.awt.Dimension;
import java.awt.Toolkit;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.Mote;
import se.sics.cooja.SectionMoteMemory;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.Beeper;
import se.sics.cooja.interfaces.PolledAfterActiveTicks;

/**
 * Beeper mote interface.
 *
 * Contiki variables:
 * <ul>
 * <li>char simBeeped (1=on, else off)
 * </ul>
 * <p>
 *
 * Core interface:
 * <ul>
 * <li>beep_interface
 * </ul>
 * <p>
 *
 * This observable is changed and notifies observers when the mote beeps.
 *
 * @author Fredrik Osterlind
 */
public class ContikiBeeper extends Beeper implements ContikiMoteInterface, PolledAfterActiveTicks {
  private Mote mote = null;
  private SectionMoteMemory moteMem = null;
  private static Logger logger = Logger.getLogger(ContikiBeeper.class);

  /**
   * Creates an interface to the beeper at mote.
   *
   * @param mote
   *          Beeper's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiBeeper(Mote mote) {
    this.mote = mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();
  }

  public boolean isBeeping() {
    return moteMem.getByteValueOf("simBeeped") == 1;
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"beep_interface"};
  }

  public void doActionsAfterTick() {
    if (moteMem.getByteValueOf("simBeeped") == 1) {
      this.setChanged();
      this.notifyObservers(mote);

      moteMem.setByteValueOf("simBeeped", (byte) 0);
    }
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    final JLabel statusLabel = new JLabel("Last beep at time: ?");
    panel.add(statusLabel);

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (!isBeeping()) {
          return;
        }

        long currentTime = mote.getSimulation().getSimulationTime();
        statusLabel.setText("Last beep at time: " + currentTime);

        /* Beep on speakers */
        Toolkit.getDefaultToolkit().beep();
      }
    });

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

    panel.setMinimumSize(new Dimension(140, 60));
    panel.setPreferredSize(new Dimension(140, 60));

    return panel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }

    this.deleteObserver(observer);
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}
