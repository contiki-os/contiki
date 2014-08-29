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
 * Read-only interface to IPv4 or IPv6 address.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("IP Address")
public class IPAddress extends MoteInterface {

  private static Logger logger = Logger.getLogger(IPAddress.class);
  private final VarMemory moteMem;

  private static final int IPv6_MAX_ADDRESSES = 4;
  private boolean ipv6IsGlobal = false;
  private int ipv6AddressIndex = -1;

  private static final int MONITORED_SIZE = 150;
  private SegmentMonitor memMonitor;

  public IPAddress(final Mote mote) {
    moteMem = new VarMemory(mote.getMemory());

    memMonitor = new MemoryInterface.SegmentMonitor() {
      @Override
      public void memoryChanged(MemoryInterface memory, SegmentMonitor.EventType type, long address) {
        if (type != SegmentMonitor.EventType.WRITE) {
          return;
        }
        setChanged();
        notifyObservers();
      }
    };
    if (isVersion4()) {
      moteMem.addVarMonitor(SegmentMonitor.EventType.WRITE, "uip_hostaddr", memMonitor);
    } else if (isVersion6()) {
      moteMem.addVarMonitor(SegmentMonitor.EventType.WRITE, "uip_ds6_netif_addr_list_offset", memMonitor);
      moteMem.addVarMonitor(SegmentMonitor.EventType.WRITE, "uip_ds6_addr_size", memMonitor);
      moteMem.addVarMonitor(SegmentMonitor.EventType.WRITE, "uip_ds6_if", memMonitor);
    }
  }

  /**
   * Returns IP address string.
   * Supports both IPv4 and IPv6 addresses.
   *
   * @return IP address string
   */
  public String getIPString() {
    if (isVersion4()) {
      String ipString = "";
      byte[] ip = moteMem.getByteArray("uip_hostaddr", 4);
      for (int i = 0; i < 3; i++) {
        ipString += (0xFF & ip[i]) + ".";
      }
      ipString += (0xFF & ip[3]);
      return ipString;
    } else if (isVersion6()) {
      String ipString = getUncompressedIPv6Address();
      return compressIPv6Address(ipString);
    }
    return null;
  }

  public static String compressIPv6Address(String ipString) {
    if (ipString.contains(":0000:0000:0000:0000:")) {
      ipString = ipString.replace(":0000:0000:0000:0000:", "::");
    } else if (ipString.contains(":0000:0000:0000:")) {
      ipString = ipString.replace(":0000:0000:0000:", "::");
    } else if (ipString.contains(":0000:0000:")) {
      ipString = ipString.replace(":0000:0000:", "::");
    } else if (ipString.contains(":0000:")) {
      ipString = ipString.replace(":0000:", "::");
    }
    while (ipString.contains(":0")) {
      ipString = ipString.replaceAll(":0", ":");
    }
    return ipString;
  }

  public byte[] getIPv6Address() {
    byte[] ip = null;

    /* IpV6: Struct sizes and offsets */
    int ipv6NetworkInterfaceAddressOffset = moteMem.getByteValueOf("uip_ds6_netif_addr_list_offset");
    int ipv6AddressStructSize = moteMem.getByteValueOf("uip_ds6_addr_size");
    if (ipv6NetworkInterfaceAddressOffset == 0 || ipv6AddressStructSize == 0) {
      return null;
    }

    /* TODO No need to copy the entire array! */
    byte[] structData = moteMem.getByteArray(
            "uip_ds6_if",
            ipv6NetworkInterfaceAddressOffset + IPv6_MAX_ADDRESSES * ipv6AddressStructSize);

    ipv6AddressIndex = -1;
    for (int addressIndex = 0; addressIndex < IPv6_MAX_ADDRESSES; addressIndex++) {
      int offset = ipv6NetworkInterfaceAddressOffset + addressIndex * ipv6AddressStructSize;
      byte isUsed = structData[offset];
      if (isUsed == 0) {
        continue;
      }
      byte[] addressData = new byte[16];
      System.arraycopy(
              structData, offset + 2/* ipaddr offset */,
              addressData, 0, 16);

      if (addressData[0] == (byte) 0xFE && addressData[1] == (byte) 0x80) {
        ipv6IsGlobal = false;
      } else {
        ipv6IsGlobal = true;
      }

      ip = addressData;
      ipv6AddressIndex = addressIndex;
      if (ipv6IsGlobal) {
        break;
      }
    }
    if (ip == null) {
      ip = new byte[16];
      ipv6AddressIndex = -1;
    }
    return ip;
  }

  public static String getUncompressedIPv6AddressString(byte[] ip) {
    StringBuilder sb = new StringBuilder();
    for (int i = 0; i < 14; i += 2) {
      sb.append(String.format("%02x%02x:", 0xFF & ip[i + 0], 0xFF & ip[i + 1]));
    }
    sb.append(String.format("%02x%02x", 0xFF & ip[14], 0xFF & ip[15]));
    return sb.toString();
  }

  public String getUncompressedIPv6Address() {
    byte[] ip = getIPv6Address();
    if (ip == null) {
      return "";
    }
    return getUncompressedIPv6AddressString(ip);
  }

  /**
   * @return True if mote has an IPv4 address
   */
  public boolean isVersion4() {
    return moteMem.variableExists("uip_hostaddr");
  }

  /**
   * @return True if mote has an IPv6 address
   */
  public boolean isVersion6() {
    return moteMem.variableExists("uip_ds6_netif_addr_list_offset")
            && moteMem.variableExists("uip_ds6_addr_size")
            && moteMem.variableExists("uip_ds6_if");
  }

  @Override
  public void removed() {
    super.removed();
    if (memMonitor != null) {
      if (isVersion4()) {
        moteMem.removeVarMonitor("rimeaddr_node_addr", memMonitor);
      } else if (isVersion6()) {
        moteMem.removeVarMonitor("uip_ds6_netif_addr_list_offset", memMonitor);
        moteMem.removeVarMonitor("uip_ds6_addr_size", memMonitor);
        moteMem.removeVarMonitor("uip_ds6_if", memMonitor);
      }
    }
  }

  @Override
  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JLabel ipLabel = new JLabel();

    Observer observer;
    this.addObserver(observer = new Observer() {
      @Override
      public void update(Observable obs, Object obj) {
        if (isVersion4()) {
          ipLabel.setText("IPv4 address: " + getIPString());
        } else if (isVersion6()) {
          ipLabel.setText((ipv6IsGlobal ? "Global" : "Local")
                  + " IPv6 address(#" + ipv6AddressIndex + "): " + getIPString());
        } else {
          ipLabel.setText("Unknown IP");
        }
      }
    });
    observer.update(null, null);

    panel.add(ipLabel);
    panel.putClientProperty("intf_obs", observer);

    return panel;
  }

  @Override
  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }
    this.deleteObserver(observer);
  }

  @Override
  public Collection<Element> getConfigXML() {
    return null;
  }

  @Override
  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }
}
