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
 * $Id: ContikiButton.java,v 1.1 2006/08/21 12:13:05 fros4943 Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.awt.event.*;
import java.util.Collection;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.Button;

/**
 * This class represents a button.
 * 
 * It needs read/write access to the following core variables:
 * <ul>
 * <li>char simButtonIsDown (1=down, else up)
 * <li>char simButtonChanged (1=changed, else not changed)
 * <li>char simButtonIsActive (1=active, else inactive)
 * </ul>
 * Dependency core interfaces are:
 * <ul>
 * <li>button_interface
 * </ul>
 * <p>
 * This observable notifies observers when the button changes state (between
 * pressed and released).
 * 
 * @author Fredrik Osterlind
 */
public class ContikiButton extends Button implements ContikiMoteInterface {
  private SectionMoteMemory moteMem;
  private Mote mote;

  private boolean shouldBeReleased = false;
  private static Logger logger = Logger.getLogger(ContikiButton.class);

  private final boolean RAISES_EXTERNAL_INTERRUPT;

  /**
   * Creates an interface to the button at mote.
   * 
   * @param mote
   *          Button's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiButton(Mote mote) {
    // Read class configurations of this mote type
    RAISES_EXTERNAL_INTERRUPT = mote.getType().getConfig()
        .getBooleanValue(ContikiButton.class, "EXTERNAL_INTERRUPT_bool");

    this.mote = mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"button_interface"};
  }

  /**
   * Clicks button. Button will be down for one tick, and then released.
   */
  public void clickButton() {
    pressButton();
    shouldBeReleased = true;
  }

  public void releaseButton() {
    moteMem.setByteValueOf("simButtonIsDown", (byte) 0);

    if (moteMem.getByteValueOf("simButtonIsActive") == 1) {
      moteMem.setByteValueOf("simButtonChanged", (byte) 1);

      // If mote is inactive, wake it up
      if (RAISES_EXTERNAL_INTERRUPT)
        mote.setState(Mote.STATE_ACTIVE);

      setChanged();
      notifyObservers();
    }
  }

  public void pressButton() {
    moteMem.setByteValueOf("simButtonIsDown", (byte) 1);

    if (moteMem.getByteValueOf("simButtonIsActive") == 1) {
      moteMem.setByteValueOf("simButtonChanged", (byte) 1);

      // If mote is inactive, wake it up
      if (RAISES_EXTERNAL_INTERRUPT)
        mote.setState(Mote.STATE_ACTIVE);

      setChanged();
      notifyObservers();
    }
  }

  public boolean isPressed() {
    return moteMem.getByteValueOf("simButtonIsDown") == 1;
  }
  
  public void doActionsBeforeTick() {
    // Nothing to do
  }

  public void doActionsAfterTick() {
    // If a button is pressed and should be clicked, release it now
    if (shouldBeReleased) {
      releaseButton();
      shouldBeReleased = false;
    }
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

  public double energyConsumptionPerTick() {
    return 0.0;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML) {
  }

}
