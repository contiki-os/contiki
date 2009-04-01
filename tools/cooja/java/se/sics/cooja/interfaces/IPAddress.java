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
 * $Id: IPAddress.java,v 1.3 2009/04/01 17:43:17 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import java.util.Vector;

import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * IPv4 or IPv6 address.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("IP Address")
public abstract class IPAddress extends MoteInterface {
  private static Logger logger = Logger.getLogger(IPAddress.class);
  private AddressMemory moteMem;
  private static boolean warnedNotImplemented = false;

  public IPAddress(final Mote mote) {
    moteMem = (AddressMemory) mote.getMemory();

    /* Detect startup IP (only zeroes) */
    TimeEvent updateWhenAddressReady = new TimeEvent(0) {
      public void execute(long t) {
        if (!isVersion4() && !isVersion6()) {
          return;
        }

        String ipString = getIPString();
        ipString = ipString.replace(".", "");
        ipString = ipString.replace("0", "");
        if (!ipString.isEmpty()) {
          setChanged();
          notifyObservers();
          return;
        }

        /* Postpone until IP has been set */
        mote.getSimulation().scheduleEvent(
            this,
            mote.getSimulation().getSimulationTime() + 1);
        return;
      }
    };
    updateWhenAddressReady.execute(0);
  }

  /**
   * Returns IP address string.
   * Supports both IPv4 and IPv6 addresses.
   *
   * @see #setIPString(String)
   * @return IP address string
   */
  public String getIPString() {
    if (isVersion4()) {
      String ipString = "";
      byte[] ip = moteMem.getByteArray("uip_hostaddr", 4);
      for (int i=0; i < 3; i++) {
        ipString += (0xFF & ip[i]) + ".";
      }
      ipString += ip[3];
      return ipString;
    } else if (isVersion6()) {
      String ipString = "";

      /* XXX Assuming fixed offset in struct uip_netif (uip-netif.h) */
      int offset =
        4*4 /* 4 uint32_t */ + 2*moteMem.getIntegerLength() /* 2 uint8_t */;
      byte[] tmp = moteMem.getByteArray(
          "uip_netif_physical_if",
          offset + 16);
      byte[] ip = new byte[16];
      System.arraycopy(tmp, offset, ip, 0, 16);

      int i=0;
      while (i < 14) {
        int val = (0xFF&ip[i+1]) + ((0xFF&ip[i])<<8);
        ipString += hex16ToString(val) + ".";
        i+=2;
      }
      int val = (0xFF&ip[15]) + ((0xFF&ip[14])<<8);
      ipString += hex16ToString(val);

      return ipString;
    }
    return null;
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
    return moteMem.variableExists("uip_netif_physical_if");
  }

  /**
   * Set IPv4 address: a.b.c.d.
   *
   * @param a First byte
   * @param b Second byte
   * @param c Third byte
   * @param d Fourth byte
   */
  public void setIPv4Address(int a, int b, int c, int d) {
    /* TODO Implement */
    if (!warnedNotImplemented) {
      warnedNotImplemented = true;
      logger.warn("Changing IP addresses is no longer supported.");
      logger.warn("The IP address is instead determined by Contiki, for example based on the MAC/Rime address, or the node ID.");
    }
  }

  /**
   * Set IPv6 address.
   *
   * @param address 16 element IP address array
   */
  public void setIPv6Address(int[] address) {
    /* TODO Implement */
    if (!warnedNotImplemented) {
      warnedNotImplemented = true;
      logger.warn("Changing IP addresses is no longer supported.");
      logger.warn("The IP address is instead determined by Contiki, for example based on the MAC/Rime address, or the node ID.");
    }
  }

  /**
   * Set IP address.
   * Supports both IPv4 and IPv6 addresses.
   *
   * @see #getIPString()
   * @param ipString IP address string
   */
  public void setIPString(String ipString) {
    /* TODO Implement */
    if (!warnedNotImplemented) {
      warnedNotImplemented = true;
      logger.warn("Changing IP addresses is no longer supported.");
      logger.warn("The IP address is instead determined by Contiki, for example based on the MAC/Rime address, or the node ID.");
    }
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JLabel ipLabel = new JLabel();

    if (isVersion4()) {
      ipLabel.setText("IPv4 address: " + getIPString());
    } else if (isVersion6()) {
      ipLabel.setText("IPv6 address: " + getIPString());
    } else {
      ipLabel.setText("Unknown IP");
    }

    panel.add(ipLabel);

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (isVersion4()) {
          ipLabel.setText("IPv4 address: " + getIPString());
        } else if (isVersion6()) {
          ipLabel.setText("IPv6 address: " + getIPString());
        } else {
          ipLabel.setText("Unknown IP");
        }
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
    Vector<Element> config = new Vector<Element>();
    Element element;

    // Infinite boolean
    element = new Element("ip");
    element.setText(getIPString());
    config.add(element);

    return config;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("ip")) {
        setIPString(element.getText());
      }
    }
  }

  private static String hex16ToString(int data) {
    String str16 = "0000000000000000";
    String s = Integer.toString(data & 0xffff, 16);
    if (s.length() < 4) {
      s = str16.substring(0, 4 - s.length()) + s;
    }
    return s;
  }
}
