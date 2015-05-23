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
 */

package org.contikios.cooja.interfaces;

import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.MoteInterface;
import org.contikios.cooja.mote.memory.MemoryInterface;
import org.contikios.cooja.mote.memory.MemoryInterface.SegmentMonitor;
import org.contikios.cooja.mote.memory.VarMemory;

/**
 * Read-only interface to Rime address read from Contiki variable: linkaddr_node_addr.
 * XXX Assuming Rime address is 2 bytes.
 *
 * @see #RIME_ADDR_LENGTH
 * @author Fredrik Osterlind
 */
@ClassDescription("Rime address")
public class RimeAddress extends MoteInterface {
  private static Logger logger = Logger.getLogger(RimeAddress.class);
  private VarMemory moteMem;

  public static final int RIME_ADDR_LENGTH = 2;

  private SegmentMonitor memMonitor = null;

  public RimeAddress(final Mote mote) {
    moteMem = new VarMemory(mote.getMemory());
    if (hasRimeAddress()) {
      memMonitor = new SegmentMonitor() {
        @Override
        public void memoryChanged(MemoryInterface memory, SegmentMonitor.EventType type, long address) {
          if (type != SegmentMonitor.EventType.WRITE) {
            return;
          }
          setChanged();
          notifyObservers();
        }
      };
      /* TODO XXX Timeout? */
      moteMem.addVarMonitor(SegmentMonitor.EventType.WRITE, "linkaddr_node_addr", memMonitor);
    }
  }

  public boolean hasRimeAddress() {
    return moteMem.variableExists("linkaddr_node_addr");
  }

  public String getAddressString() {
    if (!hasRimeAddress()) {
      return null;
    }

    String addrString = "";
    byte[] addr = moteMem.getByteArray("linkaddr_node_addr", RIME_ADDR_LENGTH);
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

  public void removed() {
    super.removed();
    if (memMonitor != null) {
      moteMem.removeVarMonitor("linkaddr_node_addr", memMonitor);
    }
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }
}
