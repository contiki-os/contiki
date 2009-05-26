/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: RimeAddress.java,v 1.3 2009/05/26 14:24:21 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * Read-only interface to Rime address read from Contiki variable: rimeaddr_node_addr.
 * XXX Assuming Rime address is 2 bytes.
 *
 * @see #RIME_ADDR_LENGTH
 * @author Fredrik Osterlind
 */
@ClassDescription("Rime address")
public class RimeAddress extends MoteInterface {
  private static Logger logger = Logger.getLogger(RimeAddress.class);
  private AddressMemory moteMem;

  public static final int RIME_ADDR_LENGTH = 2;
  
  public RimeAddress(final Mote mote) {
    moteMem = (AddressMemory) mote.getMemory();

    /* Detect startup address (only zeroes) */
    TimeEvent updateWhenAddressReady = new TimeEvent(0) {
      public void execute(long t) {
        String addrString = getAddressString();
        addrString = addrString.replace(".", "");
        addrString = addrString.replace("0", "");
        if (!addrString.isEmpty()) {
          setChanged();
          notifyObservers();
          return;
        }

        /* Postpone until address has been set */
        mote.getSimulation().scheduleEvent(
            this,
            mote.getSimulation().getSimulationTime() + Simulation.MILLISECOND);
        return;
      }
    };
    updateWhenAddressReady.execute(0);
  }

  public String getAddressString() {
    String addrString = "";
    byte[] addr = moteMem.getByteArray("rimeaddr_node_addr", RIME_ADDR_LENGTH);
    for (int i=0; i < RIME_ADDR_LENGTH-1; i++) {
      addrString += (0xFF & addr[i]) + ".";
    }
    addrString += (0xFF & addr[RIME_ADDR_LENGTH-1]);
    return addrString;
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JLabel ipLabel = new JLabel();

    ipLabel.setText("Rime address: " + getAddressString());

    panel.add(ipLabel);

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        ipLabel.setText("Rime address: " + getAddressString());
      }
    });

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
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

  public double energyConsumption() {
    return 0;
  }
}
