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
 * $Id: ContikiMoteID.java,v 1.8 2010/05/02 09:42:52 fros4943 Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.MoteID;

/**
 * Mote ID interface: 'node_id'.
 *
 * Contiki variables:
 * <ul>
 * <li>int simMoteID
 * <li>char simMoteIDChanged
 * </ul>
 *
 * This interface also seeds the Contiki random generator: 'random_init()'.
 *
 * Core interface:
 * <ul>
 * <li>moteid_interface
 * </ul>
 *
 * This observable notifies observers when the mote ID is set or altered.
 *
 * @author Fredrik Osterlind
 */
public class ContikiMoteID extends MoteID implements ContikiMoteInterface {
  private SectionMoteMemory moteMem = null;
  private static Logger logger = Logger.getLogger(ContikiMoteID.class);

  private int moteID = 0;

  private Mote mote;
  
  /**
   * Creates an interface to the mote ID at mote.
   *
   * @param mote Mote

   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiMoteID(Mote mote) {
    this.mote = mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"moteid_interface"};
  }

  public int getMoteID() {
    return moteID;
  }

  public void setMoteID(int newID) {
    moteID = newID;
    moteMem.setIntValueOf("simMoteID", moteID);
    moteMem.setByteValueOf("simMoteIDChanged", (byte) 1);
    moteMem.setIntValueOf("simRandomSeed", (int) (mote.getSimulation().getRandomSeed() + newID));
    setChanged();
    notifyObservers();
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JLabel idLabel = new JLabel();

    idLabel.setText("Mote ID: " + moteID);

    panel.add(idLabel);

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        idLabel.setText("Mote ID: " + moteID);
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

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    // Infinite boolean
    element = new Element("id");
    element.setText(Integer.toString(moteID));
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
