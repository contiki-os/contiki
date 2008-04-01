/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: MspMoteID.java,v 1.3 2008/04/01 08:07:58 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import java.util.Vector;

import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.AddressMemory.UnknownVariableException;
import se.sics.cooja.interfaces.MoteID;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteMemory;

/**
 * @author Fredrik Osterlind
 */
public class MspMoteID extends MoteID {
  private static Logger logger = Logger.getLogger(MspMoteID.class);

  private MspMote mote;
  private MspMoteMemory moteMem = null;

  public static final boolean GENERATE_ID_HEADER = true;

  /**
   * Creates an interface to the mote ID at mote.
   *
   * @param mote
   *          Mote ID's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public MspMoteID(Mote mote) {
    this.mote = (MspMote) mote;
    this.moteMem = (MspMoteMemory) mote.getMemory();
  }

  public int getMoteID() {
    try {
      return moteMem.getIntValueOf("node_id");
    } catch (UnknownVariableException e) {
      logger.fatal("Contiki variable 'node_id' not found");
    }
    return -1;
  }

  public void setMoteID(int newID) {
    /* Write node ID to flash */
    if (GENERATE_ID_HEADER) {
      SkyFlash flash = mote.getInterfaces().getInterfaceOfType(SkyFlash.class);
      flash.writeIDheader(newID);
    }

    try {
      moteMem.setIntValueOf("node_id", newID);
    } catch (UnknownVariableException e) {
      logger.fatal("Contiki variable 'node_id' not found");
    }

    setChanged();
    notifyObservers();
  }

  public void doActionsBeforeTick() {
  }

  public void doActionsAfterTick() {
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JLabel idLabel = new JLabel();

    idLabel.setText("Mote ID: " + getMoteID());

    panel.add(idLabel);

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        idLabel.setText("Mote ID: " + getMoteID());
      }
    });

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

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

  public double energyConsumptionPerTick() {
    return 0.0;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    // Infinite boolean
    element = new Element("id");
    element.setText(Integer.toString(getMoteID()));
    config.add(element);

    return config;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("id")) {
        setMoteID(Integer.parseInt(element.getText()));
      }
    }
  }

}
