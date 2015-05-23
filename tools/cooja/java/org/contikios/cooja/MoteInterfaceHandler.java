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
 */

package org.contikios.cooja;

import java.util.*;

import org.apache.log4j.Logger;
import org.contikios.cooja.interfaces.*;

/**
 * The mote interface handler holds all interfaces for a specific mote.
 * Interfaces should be polled via this class when the mote is ticked.
 *
 * @see #doActiveActionsAfterTick()
 * @see #doActiveActionsBeforeTick()
 * @see #doPassiveActionsAfterTick()
 * @see #doPassiveActionsBeforeTick()
 *
 * @author Fredrik Osterlind
 */
public class MoteInterfaceHandler {
  private static Logger logger = Logger.getLogger(MoteInterfaceHandler.class);

  private ArrayList<MoteInterface> moteInterfaces = new ArrayList<MoteInterface>();

  /* Cached interfaces */
  private Battery myBattery;
  private Beeper myBeeper;
  private Button myButton;
  private Clock myClock;
  private IPAddress myIPAddress;
  private RimeAddress myRimeAddress;
  private LED myLED;
  private Log myLog;
  private MoteID myMoteID;
  private PIR myPIR;
  private Position myPosition;
  private Radio myRadio;
  private PolledBeforeActiveTicks[] polledBeforeActive = null;
  private PolledAfterActiveTicks[] polledAfterActive = null;
  private PolledBeforeAllTicks[] polledBeforeAll = null;
  private PolledAfterAllTicks[] polledAfterAll = null;

  /**
   * Creates new empty mote interface handler.
   */
  public MoteInterfaceHandler() {
  }

  /**
   * Creates new mote interface handler. All given interfaces are created.
   *
   * @param mote Mote
   * @param interfaceClasses Mote interface classes
   */
  public MoteInterfaceHandler(Mote mote, Class<? extends MoteInterface>[] interfaceClasses) {
    for (Class<? extends MoteInterface> interfaceClass : interfaceClasses) {
      MoteInterface intf = MoteInterface.generateInterface(interfaceClass, mote);

      if (intf != null) {
        addInterface(intf);
      } else {
        logger.fatal("Could not load interface: " + interfaceClass);
      }
    }
  }

  /**
   * Returns interface of given type. Returns the first interface found that
   * is either of the given class or of a subclass.
   *
   * Usage: getInterfaceOfType(Radio.class)
   *
   * @param interfaceType Class of interface to return
   * @return Mote interface, or null if no interface exists of given type
   */
  public <N extends MoteInterface> N getInterfaceOfType(Class<N> interfaceType) {
    for (MoteInterface intf : moteInterfaces) {
      if (interfaceType.isInstance(intf)) {
        return interfaceType.cast(intf);
      }
    }

    return null;
  }

  /**
   * Returns the first interface with a class name that ends with the given arguments.
   * Example: mote.getInterfaces().get("Temperature");
   * 
   * @param <N>
   * @param classname
   * @return
   */
  public MoteInterface get(String classname) {
    for (MoteInterface intf : moteInterfaces) {
      if (intf.getClass().getName().endsWith(classname)) {
        return intf;
      }
    }
    return null;
  }

  /**
   * Returns the battery interface (if any).
   *
   * @return Battery interface
   */
  public Battery getBattery() {
    if (myBattery == null) {
      myBattery = getInterfaceOfType(Battery.class);
    }
    return myBattery;
  }

  /**
   * Returns the beeper interface (if any).
   *
   * @return Beeper interface
   */
  public Beeper getBeeper() {
    if (myBeeper == null) {
      myBeeper = getInterfaceOfType(Beeper.class);
    }
    return myBeeper;
  }

  /**
   * Returns the button interface (if any).
   *
   * @return Button interface
   */
  public Button getButton() {
    if (myButton == null) {
      myButton = getInterfaceOfType(Button.class);
    }
    return myButton;
  }

  /**
   * Returns the clock interface (if any).
   *
   * @return Clock interface
   */
  public Clock getClock() {
    if (myClock == null) {
      myClock = getInterfaceOfType(Clock.class);
    }
    return myClock;
  }

  /**
   * Returns the IP address interface (if any).
   *
   * @return IP Address interface
   */
  public IPAddress getIPAddress() {
    if (myIPAddress == null) {
      myIPAddress = getInterfaceOfType(IPAddress.class);
    }
    return myIPAddress;
  }

  /**
   * @return Rime address interface
   */
  public RimeAddress getRimeAddress() {
    if (myRimeAddress == null) {
      myRimeAddress = getInterfaceOfType(RimeAddress.class);
    }
    return myRimeAddress;
  }

  /**
   * Returns the LED interface (if any).
   *
   * @return LED interface
   */
  public LED getLED() {
    if (myLED == null) {
      myLED = getInterfaceOfType(LED.class);
    }
    return myLED;
  }

  /**
   * Returns the log interface (if any).
   *
   * @return Log interface
   */
  public Log getLog() {
    if (myLog == null) {
      myLog = getInterfaceOfType(Log.class);
    }
    return myLog;
  }

  /**
   * Returns the mote ID interface (if any).
   *
   * @return Mote ID interface
   */
  public MoteID getMoteID() {
    if (myMoteID == null) {
      myMoteID = getInterfaceOfType(MoteID.class);
    }
    return myMoteID;
  }

  /**
   * Returns the PIR interface (if any).
   *
   * @return PIR interface
   */
  public PIR getPIR() {
    if (myPIR == null) {
      myPIR = getInterfaceOfType(PIR.class);
    }
    return myPIR;
  }

  /**
   * Returns the position interface (if any).
   *
   * @return Position interface
   */
  public Position getPosition() {
    if (myPosition == null) {
      myPosition = getInterfaceOfType(Position.class);
    }
    return myPosition;
  }

  /**
   * Returns the radio interface (if any).
   *
   * @return Radio interface
   */
  public Radio getRadio() {
    if (myRadio == null) {
      myRadio = getInterfaceOfType(Radio.class);
    }
    return myRadio;
  }

  /**
   * Polls active interfaces before mote tick.
   */
  public void doActiveActionsBeforeTick() {
    if (polledBeforeActive == null) {
      ArrayList<PolledBeforeActiveTicks> intfs = new ArrayList<PolledBeforeActiveTicks>();
      for (MoteInterface intf: moteInterfaces) {
        if (intf instanceof PolledBeforeActiveTicks) {
          intfs.add((PolledBeforeActiveTicks)intf);
        }
      }
      polledBeforeActive = intfs.toArray(new PolledBeforeActiveTicks[intfs.size()]);
    }

    for (PolledBeforeActiveTicks element : polledBeforeActive) {
      element.doActionsBeforeTick();
    }
  }

  /**
   * Polls active interfaces after mote tick.
   */
  public void doActiveActionsAfterTick() {
    if (polledAfterActive == null) {
      ArrayList<PolledAfterActiveTicks> intfs = new ArrayList<PolledAfterActiveTicks>();
      for (MoteInterface intf: moteInterfaces) {
        if (intf instanceof PolledAfterActiveTicks) {
          intfs.add((PolledAfterActiveTicks)intf);
        }
      }
      polledAfterActive = intfs.toArray(new PolledAfterActiveTicks[intfs.size()]);
    }

    for (PolledAfterActiveTicks element : polledAfterActive) {
      element.doActionsAfterTick();
    }
  }

  /**
   * Polls passive interfaces before mote tick.
   */
  public void doPassiveActionsBeforeTick() {
    if (polledBeforeAll == null) {
      ArrayList<PolledBeforeAllTicks> intfs = new ArrayList<PolledBeforeAllTicks>();
      for (MoteInterface intf: moteInterfaces) {
        if (intf instanceof PolledBeforeAllTicks) {
          intfs.add((PolledBeforeAllTicks)intf);
        }
      }
      polledBeforeAll = intfs.toArray(new PolledBeforeAllTicks[intfs.size()]);
    }

    for (PolledBeforeAllTicks element : polledBeforeAll) {
      element.doActionsBeforeTick();
    }
  }

  /**
   * Polls passive interfaces after mote tick.
   */
  public void doPassiveActionsAfterTick() {
    if (polledAfterAll == null) {
      ArrayList<PolledAfterAllTicks> intfs = new ArrayList<PolledAfterAllTicks>();
      for (MoteInterface intf: moteInterfaces) {
        if (intf instanceof PolledAfterAllTicks) {
          intfs.add((PolledAfterAllTicks)intf);
        }
      }
      polledAfterAll = intfs.toArray(new PolledAfterAllTicks[intfs.size()]);
    }

    for (PolledAfterAllTicks element : polledAfterAll) {
      element.doActionsAfterTick();
    }
  }

  /**
   * @return Mote interfaces
   */
  public Collection<MoteInterface> getInterfaces() {
    return moteInterfaces;
  }

  /**
   * Add mote interface.
   *
   * @param intf Mote interface
   * @see PolledBeforeActiveTicks
   * @see PolledBeforeAllTicks
   * @see PolledAfterActiveTicks
   * @see PolledAfterAllTicks
   */
  public void addInterface(MoteInterface intf) {
    moteInterfaces.add(intf);

    polledBeforeActive = null;
    polledAfterActive = null;
    polledBeforeAll = null;
    polledAfterAll = null;
  }

  public String toString() {
    return "Mote interfaces handler (" + moteInterfaces.size() + " mote interfaces)";
  }
}
