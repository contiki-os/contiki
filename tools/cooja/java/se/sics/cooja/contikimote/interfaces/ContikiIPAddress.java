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
 * $Id: ContikiIPAddress.java,v 1.3 2008/10/28 10:12:43 fros4943 Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.IPAddress;

/**
 * uIP IP address.
 *
 * Contiki variables:
 * <ul>
 * <li>char simIPa
 * <li>char simIPb
 * <li>char simIPc
 * <li>char simIPd
 * <li>char simIPChanged (1 if new IP should be set)
 * </ul>
 * <p>
 *
 * The new IP will be "simIPa.simIPb.simIPc.simIPd".
 * Note that this mote interface does not detect if Contiki changes IP address.
 *
 * Core interface:
 * <ul>
 * <li>ip_interface
 * </ul>
 *
 * This observable notifies when the IP address is set or altered.
 *
 * @author Fredrik Österlind
 */
public class ContikiIPAddress extends IPAddress implements ContikiMoteInterface {
  private SectionMoteMemory moteMem = null;
  private static Logger logger = Logger.getLogger(ContikiIPAddress.class);

  /**
   * Creates an interface to the IP address at mote.
   *
   * @param mote
   *          IP address' mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiIPAddress(Mote mote) {
    this.moteMem = (SectionMoteMemory) mote.getMemory();
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"ip_interface"};
  }

  public String getIPString() {
    return
    (int) moteMem.getByteValueOf("simIPa")
    + "." +
    (int) moteMem.getByteValueOf("simIPb")
    + "." +
    (int) moteMem.getByteValueOf("simIPc")
    + "." +
    (int) moteMem.getByteValueOf("simIPd");
  }

  public void setIPString(String ipAddress) {
    String[] ipArray = ipAddress.split("\\.");
    if (ipArray.length < 4) {
      logger.warn("Could not set ip address (" + ipAddress + ")");
    } else {
      setIPNumber((char) Integer.parseInt(ipArray[0]), (char) Integer
          .parseInt(ipArray[1]), (char) Integer.parseInt(ipArray[2]),
          (char) Integer.parseInt(ipArray[3]));
    }
  }

  public void setIPNumber(char a, char b, char c, char d) {
    moteMem.setByteValueOf("simIPa", (byte) a);
    moteMem.setByteValueOf("simIPb", (byte) b);
    moteMem.setByteValueOf("simIPc", (byte) c);
    moteMem.setByteValueOf("simIPd", (byte) d);
    moteMem.setByteValueOf("simIPChanged", (byte) 1);

    setChanged();
    notifyObservers();
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JLabel ipLabel = new JLabel();

    ipLabel.setText("IPv4 address: " + getIPString());

    panel.add(ipLabel);

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        ipLabel.setText("IPv4 address: " + getIPString());
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
    // Virtual interface, does not require any energy
    return 0.0;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    // Infinite boolean
    element = new Element("ipv4address");
    element.setText(getIPString());
    config.add(element);

    return config;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("ipv4address")) {
        setIPString(element.getText());
      }
    }
  }

}
