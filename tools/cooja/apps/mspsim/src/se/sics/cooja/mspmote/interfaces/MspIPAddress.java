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
 * $Id: MspIPAddress.java,v 1.1 2008/12/03 13:11:20 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import javax.swing.JLabel;
import javax.swing.JPanel;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.Mote;
import se.sics.cooja.TimeEvent;
import se.sics.cooja.AddressMemory.UnknownVariableException;
import se.sics.cooja.interfaces.IPAddress;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteMemory;

public class MspIPAddress extends IPAddress {
  private static Logger logger = Logger.getLogger(MspIPAddress.class);

  private MspMote mote;
  private MspMoteMemory moteMem = null;

  public MspIPAddress(Mote mote) {
    this.mote = (MspMote) mote;
    this.moteMem = (MspMoteMemory) mote.getMemory();

    TimeEvent updateWhenAddressReady = new TimeEvent(0) {
      public void execute(int t) {
        if (getIPString().equals("0.0.0.0")) {
          MspIPAddress.this.mote.getSimulation().scheduleEvent(
              this,
              MspIPAddress.this.mote.getSimulation().getSimulationTime() + 10);
          return;
        }
        setChanged();
        notifyObservers();
      }
    };
    mote.getSimulation().scheduleEvent(
        updateWhenAddressReady,
        mote.getSimulation().getSimulationTime() + 10
    );
  }

  public String getIPString() {
    try {
      byte[] addr = moteMem.getByteArray("uip_hostaddr", 4);
      return
      (addr[0]&0xFF) + "." +
      (addr[1]&0xFF) + "." +
      (addr[2]&0xFF) + "." +
      (addr[3]&0xFF);
    } catch (UnknownVariableException e) {
      logger.fatal("IP Address not available on mote: " + e.getMessage());
    }
    return "?.?.?.?";
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
    try {
      byte[] addr = new byte[4];

      addr[0] = (byte) a;
      addr[1] = (byte) b;
      addr[2] = (byte) c;
      addr[3] = (byte) d;
      moteMem.setByteArray("uip_hostaddr", addr);
    } catch (UnknownVariableException e) {
      logger.fatal("IP Address not available on mote: " + e.getMessage());
    }

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

  public double energyConsumption() {
    return 0;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}
