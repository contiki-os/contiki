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
 * $Id: ContikiButton.java,v 1.16 2010/02/05 08:49:18 fros4943 Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.awt.event.*;
import java.util.Collection;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMote;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.Button;

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
  private SectionMoteMemory moteMem;
  private ContikiMote mote;

  private static Logger logger = Logger.getLogger(ContikiButton.class);

  /**
   * Creates an interface to the button at mote.
   *
   * @param mote Mote
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiButton(Mote mote) {
    this.mote = (ContikiMote) mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"button_interface"};
  }

  private TimeEvent pressButtonEvent = new MoteTimeEvent(mote, 0) {
    public void execute(long t) {
      doPressButton();
    }
  };

  private TimeEvent releaseButtonEvent = new MoteTimeEvent(mote, 0) {
    public void execute(long t) {
      /* Wait until button change is handled by Contiki */
      if (moteMem.getByteValueOf("simButtonChanged") != 0) {
        /* Postpone button release */
        mote.getSimulation().scheduleEvent(releaseButtonEvent, t + Simulation.MILLISECOND);
        return;
      }

      /*logger.info("Releasing button at: " + t);*/
      doReleaseButton();
    }
  };

  /**
   * Clicks button: Presses and immediately releases button.
   */
  public void clickButton() {
    mote.getSimulation().invokeSimulationThread(new Runnable() {
      public void run() {
        mote.getSimulation().scheduleEvent(pressButtonEvent, mote.getSimulation().getSimulationTime());
        mote.getSimulation().scheduleEvent(releaseButtonEvent, mote.getSimulation().getSimulationTime() + Simulation.MILLISECOND);
      }      
    });
  }

  public void pressButton() {
    mote.getSimulation().invokeSimulationThread(new Runnable() {
      public void run() {
        mote.getSimulation().scheduleEvent(pressButtonEvent, mote.getSimulation().getSimulationTime());
      }      
    });
  }

  public void releaseButton() {
    mote.getSimulation().invokeSimulationThread(new Runnable() {
      public void run() {
        mote.getSimulation().scheduleEvent(releaseButtonEvent, mote.getSimulation().getSimulationTime());
      }      
    });
  }

  private void doReleaseButton() {
    moteMem.setByteValueOf("simButtonIsDown", (byte) 0);

    if (moteMem.getByteValueOf("simButtonIsActive") == 1) {
      moteMem.setByteValueOf("simButtonChanged", (byte) 1);

      /* If mote is inactive, wake it up */
      mote.requestImmediateWakeup();

      setChanged();
      notifyObservers();
    }
  }

  private void doPressButton() {
    moteMem.setByteValueOf("simButtonIsDown", (byte) 1);

    if (moteMem.getByteValueOf("simButtonIsActive") == 1) {
      moteMem.setByteValueOf("simButtonChanged", (byte) 1);

      /* If mote is inactive, wake it up */
      mote.requestImmediateWakeup();

      setChanged();
      notifyObservers();
    }
  }

  public boolean isPressed() {
    return moteMem.getByteValueOf("simButtonIsDown") == 1;
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JButton clickButton = new JButton("Click button");

    panel.add(clickButton);

    clickButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        clickButton();
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
