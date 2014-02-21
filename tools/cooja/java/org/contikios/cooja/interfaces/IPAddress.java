/*
 * Copyright (c) 2014, TU Braunschweig.
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
import java.util.LinkedList;
import java.util.List;
import java.util.Observable;
import java.util.Observer;
import javax.swing.JLabel;
import javax.swing.JPanel;
import org.apache.log4j.Logger;
import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.MoteInterface;
import org.contikios.cooja.mote.memory.MemoryInterface;
import org.contikios.cooja.mote.memory.MemoryInterface.SegmentMonitor;
import org.contikios.cooja.mote.memory.MemoryLayout;
import org.contikios.cooja.mote.memory.VarMemory;
import org.contikios.cooja.util.IPUtils;
import org.jdom.Element;

/**
 * Read-only interface to IPv4 or IPv6 address.
 *
 * @author Fredrik Osterlind
 * @author Enrico Joerns
 */
@ClassDescription("IP Addresses")
public class IPAddress extends MoteInterface {

  private static final Logger logger = Logger.getLogger(IPAddress.class);
  private static final int IPv6_MAX_ADDRESSES = 4;
  private static final int MONITORED_SIZE = 150;
  
  private enum IPv {
    NONE,
    IPv4,
    IPv6
  }

  private final IPv ipVersion;

  private final VarMemory moteMem;
  private final MemoryLayout memLayout;
  private IPContainer localIPAddr = null;

  private final SegmentMonitor memMonitor;

  private List<IPContainer> ipList = new LinkedList<>();

  private int ipv6_addr_size = 0;
  private int ipv6_addr_list_offset = 0;

  public IPAddress(final Mote mote) {
    moteMem = new VarMemory(mote.getMemory());
    memLayout = mote.getMemory().getLayout();

    /* If the ip memory sections changed, we recalculate addresses
     * and notify our observers.*/
    memMonitor = new MemoryInterface.SegmentMonitor() {
      int accessCount = 0;
      long lastAccess = 0;
      @Override
      public void memoryChanged(MemoryInterface memory, SegmentMonitor.EventType type, long address) {
        if (type != SegmentMonitor.EventType.WRITE) {
          return;
        }

        /* XXX Quick & Dirty IPv4 update handle */
        if (ipVersion == IPv.IPv4) {
          updateIPAddresses();
          setChanged();
          notifyObservers();
          return;
        }

        /* Wait until size and offsest values are set initially,
         * then add memory monitor for each ip field */
        if ((ipv6_addr_list_offset == 0) || (ipv6_addr_size == 0)) {
          ipv6_addr_list_offset = moteMem.getByteValueOf("uip_ds6_netif_addr_list_offset");
          ipv6_addr_size = moteMem.getByteValueOf("uip_ds6_addr_size");
          /* If the variables just updated, add the final ip listeners */
          if ((ipv6_addr_list_offset != 0) && (ipv6_addr_size != 0)) {
            /* Add monitor for each IP region */
            for (int i = 0; i < IPv6_MAX_ADDRESSES; i++) {
              long addr_of_ip = moteMem.getVariableAddress("uip_ds6_if") // start address of interface
                      + ipv6_addr_list_offset // offset to ip address region
                      + i * ipv6_addr_size // offset to ith ip address 
                      + 1 + memory.getLayout().getPaddingBytesFor(
                              MemoryLayout.DataType.INT8,
                              MemoryLayout.DataType.INT16); // skip 'isused'
              moteMem.addMemoryMonitor(
                      EventType.WRITE,
                      addr_of_ip,
                      16, /* Size of ip address in byte */
                      memMonitor);
            }
            /* Initial scan for IP address */
            updateIPAddresses();
            if (ipList.size() > 0) {
              setChanged();
              notifyObservers();
            }
            /** @TODO: Remove other listeners? */
          }
        } else {

          /** Note: works when 'isused' bit is set first
           * and address region is written sequentially */

          /* check for sequential reading of 16 byte block */
          if (address == lastAccess + 1) {
            accessCount++;
            lastAccess = address;
            if (accessCount == 16) {
              updateIPAddresses();
              setChanged();
              notifyObservers();
              lastAccess = 0;
            }
          }
          else {
            /* Check if ip write was interrupted unexpectedly last time */
            if (lastAccess != 0) {
              updateIPAddresses();
              setChanged();
              notifyObservers();
            }
            accessCount = 1;
            lastAccess = address;
          }
        }
      }
    };

    /* Determine IP version an add MemoryMonitors */
    if (moteMem.variableExists("uip_hostaddr")) {
      logger.debug("IPv4 detected");
      ipVersion = IPv.IPv4;
      moteMem.addVarMonitor(
              SegmentMonitor.EventType.WRITE,
              "uip_hostaddr",
              memMonitor);
    } else if (moteMem.variableExists("uip_ds6_netif_addr_list_offset")
            && moteMem.variableExists("uip_ds6_addr_size")
            && moteMem.variableExists("uip_ds6_if")) {
      logger.debug("IPv6 detected");
      ipVersion = IPv.IPv6;
      moteMem.addVarMonitor(
              SegmentMonitor.EventType.WRITE,
              "uip_ds6_netif_addr_list_offset",
              memMonitor);
      moteMem.addVarMonitor(
              SegmentMonitor.EventType.WRITE,
              "uip_ds6_addr_size",
              memMonitor);
    } else {
      ipVersion = IPv.NONE;
    }

    // initially look for IPs we already have
    updateIPAddresses();
  }

  /**
   * Returns true if any IP stack (Ipv4/6) is supported by mote
   * @return true if either IPv4 or IPv6 was detected
   */
  public boolean hasIP() {
    return !(ipVersion == IPv.NONE);
  }

  /**
   * Get local IP of mote.
   * @return local IP or null if not existing
   */
  public IPContainer getLocalIP() {
    return localIPAddr;
  }

  /**
   * Returns IP address string.
   * Supports both IPv4 and IPv6 addresses.
   *
   * @param idx
   * @return IP address string
   */
  public IPContainer getIP(int idx) {
    try {
      return ipList.get(idx);
    } catch (IndexOutOfBoundsException ex) {
      logger.warn("Invalid IP index " + idx);
      return null;
    }
  }

  /**
   * Rereads IP addresses from memory and updates localIP entry.
   */
  private void updateIPAddresses() {
    ipList.clear();
    if (ipVersion == IPv.IPv4) {
      addIPv4Addresses();
      localIPAddr = ipList.get(0);
    }
    else if (ipVersion == IPv.IPv6) {
      addIPv6Addresses();
      /* look for local ip addr */
      for (IPContainer c : ipList) {
        if (!c.isGlobal) {
          localIPAddr = c;
        }
      }
    }
  }

  /**
   * Rereads IPv4 addresses from memory.
   */
  private void addIPv4Addresses() {
    ipList.add(new IPContainer(0, moteMem.getByteArray("uip_hostaddr", 4), true));
  }

  /**
   * Rereads IPv6 addresses from memory.
   */
  private void addIPv6Addresses() {

    /* IpV6: Struct sizes and offsets */
    int ipv6NetworkInterfaceAddressOffset = moteMem.getByteValueOf("uip_ds6_netif_addr_list_offset");
    int ipv6AddressStructSize = moteMem.getByteValueOf("uip_ds6_addr_size");
    /* check if addresses were not set yet */
    if (ipv6NetworkInterfaceAddressOffset == 0 || ipv6AddressStructSize == 0) {
      return;
    }

    byte[] structData = moteMem.getByteArray(
            moteMem.getVariableAddress("uip_ds6_if") + ipv6NetworkInterfaceAddressOffset,
            IPv6_MAX_ADDRESSES * ipv6AddressStructSize);
    
    for (int addressIndex = 0; addressIndex < IPv6_MAX_ADDRESSES; addressIndex++) {
      int offset = addressIndex * ipv6AddressStructSize;
      byte isUsed = structData[offset];
      if (isUsed == 0) {
        continue;
      }
      byte[] addressData = new byte[16];
      System.arraycopy(
              structData, offset + 1 + memLayout.getPaddingBytesFor(
                      MemoryLayout.DataType.INT8,
                      MemoryLayout.DataType.INT16),/* ipaddr offset */
              addressData, 0, 16);

      if (((addressData[0] & (byte) 0xFF) == (byte) 0xFE) && ((addressData[1] & (byte) 0xFF) == (byte) 0x80)) {
        ipList.add(new IPContainer(addressIndex, addressData, false));
      }
      else {
        ipList.add(new IPContainer(addressIndex, addressData, true));
      }

    }
  }

  // -- MoteInterface overrides

  @Override
  public void removed() {
    super.removed();
    if (memMonitor != null) {
      if (ipVersion == IPv.IPv4) {
        moteMem.removeVarMonitor("uip_hostaddr",memMonitor);
      }
      else if (ipVersion == IPv.IPv6) {
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
        StringBuilder ipStr = new StringBuilder();
        ipStr.append("<html>");
        for (IPContainer ipc: ipList) {
          if (ipVersion == IPv.IPv4) {
            ipStr.append("IPv4 address: ")
                    .append(ipc.toString())
                    .append("<br>");
          }
          else if (ipVersion == IPv.IPv6) {
            ipStr.append(ipc.isGlobal() ? "Global" : "Local")
                    .append(" IPv6 address(#")
                    .append(ipc.getAddID())
                    .append("): ")
                    .append(ipc.toString())
                    .append("<br>");
          }
          else {
            ipStr.append("Unknown IP<br>");
          }
        }
        ipStr.append("</html>");
        ipLabel.setText(ipStr.toString());
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

  /**
   * Holds a single IP address.
   * 
   * Note: The compressed IP version is already computed in constructor
   */
  public class IPContainer {

    private boolean isGlobal = false;
    private final byte[] ip;
    private final int addrIdx;
    private final String cprString;

    public IPContainer(int addidx, byte[] ip, boolean global) {
      this.addrIdx = addidx;
      this.ip = ip;
      this.isGlobal = global;
      if (ipVersion == IPv.IPv4) {
        cprString = IPUtils.getIPv4AddressString(ip);
      } else if (ipVersion == IPv.IPv6) {
        cprString = IPUtils.getCompressedIPv6AddressString(ip);
      } else {
        cprString = "";
      }
      /* logger.info("Added new IP: " + cprString); */
    }

    public int getAddID() {
      return addrIdx;
    }

    public boolean isGlobal() {
      return isGlobal;
    }

    public byte[] getIP() {
      return ip;
    }
    
    @Override
    public String toString() {
      return cprString;
    }
    
    public String toUncompressedString() {
      if (ipVersion == IPv.IPv4) {
        return cprString;
      }
      else if (ipVersion == IPv.IPv6) {
        return IPUtils.getUncompressedIPv6AddressString(ip);
      }
      else {
        return "";
      }
    }
  }
}
