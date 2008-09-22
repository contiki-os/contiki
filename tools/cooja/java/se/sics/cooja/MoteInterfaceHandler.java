/*
 * Copyright (c) 2006, Swedish Institute of Computer Science. All rights
 * reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * $Id: MoteInterfaceHandler.java,v 1.3 2008/09/22 16:18:22 joxe Exp $
 */

package se.sics.cooja;

import java.util.*;
import org.apache.log4j.Logger;

import se.sics.cooja.interfaces.*;

/**
 * A mote interface handler holds all interfaces for a specific mote. Even
 * though an interface handler strictly does not need any interfaces at all, a
 * position interface is highly recommended. (A lot of plugins depend on a mote
 * position, for example a typical visualizer.)
 * 
 * Interfaces are divided into active and passive interfaces. Active interfaces
 * are only polled if the mote is active, while passive interfaces are polled in
 * all states except dead state.
 * 
 * Interfaces should be polled via this class when the mote is ticked.
 * 
 * @author Fredrik Osterlind
 */
public class MoteInterfaceHandler {
  private static Logger logger = Logger.getLogger(MoteInterfaceHandler.class);

  private Battery myBattery;

  private Beeper myBeeper;

  private Button myButton;

  private Clock myClock;

  private IPAddress myIPAddress;

  private LED myLED;

  private Log myLog;

  private MoteID myMoteID;

  private PIR myPIR;

  private Position myPosition;

  private Radio myRadio;

  private Vector<MoteInterface> myActiveInterfaces = new Vector<MoteInterface>();
  private MoteInterface[] activeCache = null;
  
  private Vector<MoteInterface> myPassiveInterfaces = new Vector<MoteInterface>();

  /**
   * Creates a new empty mote interface handler.
   */
  public MoteInterfaceHandler() {
  }

  /**
   * Creates a new mote interface handler. All given interfaces are loaded.
   * 
   * @param mote
   *          The mote holding this interface handler
   * @param allInterfaces
   *          Simulation interfaces to load
   */
  public MoteInterfaceHandler(Mote mote,
      Vector<Class<? extends MoteInterface>> allInterfaces) {

    // Load all interfaces
    for (Class<? extends MoteInterface> interfaceClass : allInterfaces) {
      boolean isPassive = false;

      // Check if interface is active or passive
      if (PassiveMoteInterface.class.isAssignableFrom(interfaceClass))
        isPassive = true;

      // Load interface
      MoteInterface loadedInterface = MoteInterface.generateInterface(
          interfaceClass, mote);

      if (loadedInterface != null)
        if (isPassive)
          addPassiveInterface(loadedInterface);
        else
          addActiveInterface(loadedInterface);
      else
        logger.warn("Interface could not be loaded: " + interfaceClass);
    }
  }

  /**
   * Get an interface (active or passive) of the given type. Returns the first
   * loaded interface found, that is either of the given class or of a subclass.
   * 
   * For example, if the current radio interface is wanted, this method would be
   * called like the following: getInterfaceOfType(Radio.class)
   * 
   * @param interfaceType
   *          Type of interface to return
   * @return Interface or null if no interface loaded of given type
   */
  public <N extends MoteInterface> N getInterfaceOfType(Class<N> interfaceType) {

    Enumeration<? extends MoteInterface> allActive = myActiveInterfaces
        .elements();
    while (allActive.hasMoreElements()) {
      N nextInterface = (N) allActive.nextElement();
      if (interfaceType.isAssignableFrom(nextInterface.getClass()))
        return nextInterface;
    }
    Enumeration<? extends MoteInterface> allPassive = myPassiveInterfaces
        .elements();
    while (allPassive.hasMoreElements()) {
      N nextInterface = (N) allPassive.nextElement();
      if (interfaceType.isAssignableFrom(nextInterface.getClass()))
        return nextInterface;
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
   * @return IPAddress interface
   */
  public IPAddress getIPAddress() {
    if (myIPAddress == null) {
      myIPAddress = getInterfaceOfType(IPAddress.class);
    }
    return myIPAddress;
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
   * Polls all active interfaces. This method should be called during a mote
   * tick before the mote software is executed.
   */
  public void doActiveActionsBeforeTick() {
	  // Assuming only one caller!!!
    if (activeCache == null) {
	  activeCache = (MoteInterface[]) myActiveInterfaces.toArray(new MoteInterface[myActiveInterfaces.size()]);
	}
//    for (int i = 0; i < myActiveInterfaces.size(); i++)
//      myActiveInterfaces.get(i).doActionsBeforeTick();
    for (int i = 0, n = activeCache.length; i < n; i++) {
      activeCache[i].doActionsBeforeTick();
    }
  }

  /**
   * Polls all active interfaces. This method should be called during a mote
   * tick after the mote software has executed.
   */
  public void doActiveActionsAfterTick() {
    for (int i = 0; i < myActiveInterfaces.size(); i++)
      myActiveInterfaces.get(i).doActionsAfterTick();
  }

  /**
   * Polls all passive interfaces. This method should be called during a mote
   * tick before the mote software is executed.
   */
  public void doPassiveActionsBeforeTick() {
    for (int i = 0; i < myPassiveInterfaces.size(); i++)
      myPassiveInterfaces.get(i).doActionsBeforeTick();
  }

  /**
   * Polls all passive interfaces. This method should be called during a mote
   * tick after the mote software has executed.
   */
  public void doPassiveActionsAfterTick() {
    for (int i = 0; i < myPassiveInterfaces.size(); i++)
      myPassiveInterfaces.get(i).doActionsAfterTick();
  }

  /**
   * Returns all passive mote interfaces.
   * 
   * @return All passive mote interface
   */
  public Vector<MoteInterface> getAllPassiveInterfaces() {
    return myPassiveInterfaces;
  }

  /**
   * Returns all active mote interfaces.
   * 
   * @return All active mote interface
   */
  public Vector<MoteInterface> getAllActiveInterfaces() {
    return myActiveInterfaces;
  }

  /**
   * Add an active interface to corresponding mote. An active interface is only
   * allowed to act if the mote is in active state. However, since interfaces
   * may awaken a sleeping mote up via external interrupts, most of the
   * interfaces should be active.
   * 
   * For example a button interface should be active. When the button is
   * pressed, the interface will wake the mote up (simulated external
   * interrupt), and then that button will be allowed to act before next tick.
   * 
   * A passive interface is an interface which will always act if the mote is
   * not dead. For example a battery should always be allowed to act since a
   * mote needs energy even if it is in sleep mode.
   * 
   * @see #addPassiveInterface(MoteInterface)
   * @param newInterface
   *          New interface
   */
  public void addActiveInterface(MoteInterface newInterface) {
    myActiveInterfaces.add(newInterface);
    activeCache = null;
  }

  /**
   * Add a passive interface to corresponding mote. For explanation of passive
   * vs active interfaces, see addActiveInterface(MoteInterface).
   * 
   * @see #addActiveInterface(MoteInterface)
   * @param newInterface
   *          New interface
   */
  public void addPassiveInterface(MoteInterface newInterface) {
    myPassiveInterfaces.add(newInterface);
  }

}
