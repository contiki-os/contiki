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
 * $Id: IPAddress.java,v 1.11 2010/12/07 10:40:08 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.AddressMemory;
import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.MoteTimeEvent;
import se.sics.cooja.Simulation;
import se.sics.cooja.TimeEvent;

/**
 * Read-only interface to IPv4 or IPv6 address.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("IP Address")
public class IPAddress extends MoteInterface {
  private static Logger logger = Logger.getLogger(IPAddress.class);
  private AddressMemory moteMem;

	private static final int IPv6_MAX_ADDRESSES = 4;
  private int ipv6NetworkInterfaceAddressOffset;
  private int ipv6AddressStructSize;
	private boolean ipv6IsGlobal = false;
	private int ipv6AddressIndex = -1;

  public IPAddress(final Mote mote) {
    moteMem = (AddressMemory) mote.getMemory();

    /* ipV6: Struct sizes and offsets */
  	int intLength = moteMem.getIntegerLength();
  	int longLength = 4;
  	ipv6NetworkInterfaceAddressOffset /* net/uip-ds6.h:uip_ds6_netif_t */ =
  		4 /* link_mtu */ + 
  		align(1, intLength) /* cur_hop_limit */ + 
  		4 /* base_reachable_time */ +
  		4 /* reachable_time */ +
  		4 /* retrans_timer */ +
  		align(1, intLength) /* maxdadns */;
  	ipv6AddressStructSize /* net/uip-ds6.h:uip_ds6_addr_t */ =
  		align(1, 2) /* isused */ + 
  		16 /* ipaddr, aligned on 16 bits */ + 
  		align(3, intLength) /* state + type + isinfinite */ + 
  		2*longLength /* vlifetime */ + 
  		2*longLength /* dadtimer */ +
  		align(1, intLength) /* dadnscount */;
  	ipv6AddressStructSize = align(ipv6AddressStructSize, intLength);

    /* Detect startup IP (only zeroes) */
    TimeEvent updateWhenAddressReady = new MoteTimeEvent(mote, 0) {
      public void execute(long t) {
        if (!isVersion4() && !isVersion6()) {
          return;
        }

        String ipString = getIPString();
        ipString = ipString.replace(".", "");
        ipString = ipString.replace(":", "");
        ipString = ipString.replace("0", "");
        if (!ipString.isEmpty()) {
          setChanged();
          notifyObservers();
          return;
        }

        /* Postpone until IP has been set */
        mote.getSimulation().scheduleEvent(
            this,
            mote.getSimulation().getSimulationTime() + Simulation.MILLISECOND);
        return;
      }
    };
    updateWhenAddressReady.execute(0);
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
      for (int i=0; i < 3; i++) {
        ipString += (0xFF & ip[i]) + ".";
      }
      ipString += (0xFF & ip[3]);
      return ipString;
    } else if (isVersion6()) {
    	String ipString = getUncompressedIPv6Address();
    	if (ipString != null) {
      	ipString = compressIPv6Address(ipString);
    	}
      return ipString;
    }
    return null;
  }

  public String compressIPv6Address(String ipString) {
    while (ipString.contains(":0000:")) {
    	ipString = ipString.replaceAll(":0000:", "::");
    }
    while (ipString.contains("0000")) {
    	ipString = ipString.replaceAll("0000", "0");
    }
    while (ipString.contains(":::")) {
      ipString = ipString.replaceAll(":::", "::");
    }
    while (ipString.contains(":0")) {
      ipString = ipString.replaceAll(":0", ":");
    }
    if (ipString.endsWith(":")) {
    	ipString = ipString + "0";
    }
    return ipString;
  }

  public String getUncompressedIPv6Address() {
  	byte[] ip = null;

  	/* TODO No need to copy the entire array! */
  	byte[] structData = moteMem.getByteArray("uip_ds6_if",
  			ipv6NetworkInterfaceAddressOffset+IPv6_MAX_ADDRESSES*ipv6AddressStructSize);

  	ipv6AddressIndex = -1;
  	for (int addressIndex=0; addressIndex < IPv6_MAX_ADDRESSES; addressIndex++) {
  		int offset = ipv6NetworkInterfaceAddressOffset+addressIndex*ipv6AddressStructSize;
    	byte isUsed = structData[offset];
    	if (isUsed == 0) {
    		continue;
    	}
      byte[] addressData = new byte[16];
      System.arraycopy(
      		structData, offset+2/* ipaddr offset */,
      		addressData, 0, 16);
      
      if (addressData[0] == (byte)0xFE && addressData[1] == (byte)0x80) {
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

    String ipString = "";
    int i=0;
    while (i < 14) {
      int val = (0xFF&ip[i+1]) + ((0xFF&ip[i])<<8);
      ipString += hex16ToString(val) + ":";
      i+=2;
    }
    int val = (0xFF&ip[15]) + ((0xFF&ip[14])<<8);
    ipString += hex16ToString(val);
    return ipString;
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
    return moteMem.variableExists("uip_ds6_if");
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JLabel ipLabel = new JLabel();

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (isVersion4()) {
          ipLabel.setText("IPv4 address: " + getIPString());
        } else if (isVersion6()) {
          ipLabel.setText((ipv6IsGlobal?"Global":"Local") + 
          		" IPv6 address(#" + ipv6AddressIndex + "): " + getIPString());
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

  private static String hex16ToString(int data) {
    String s = Integer.toString(data & 0xffff, 16);
    if (s.length() < 4) {
      s = "0000".substring(0, 4 - s.length()) + s;
    }
    return s;
  }
  
  private static int align(int bytes, int alignment) {
  	int size = (bytes/alignment)*alignment;
  	while (size < bytes) {
  		size += alignment;
  	}
  	return size;
  }

}
