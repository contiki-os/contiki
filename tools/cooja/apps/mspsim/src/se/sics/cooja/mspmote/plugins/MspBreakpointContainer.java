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
 * $Id: MspBreakpointContainer.java,v 1.1 2009/06/11 10:05:28 fros4943 Exp $
 */

package se.sics.cooja.mspmote.plugins;

import java.awt.event.ActionListener;
import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.WatchpointMote;
import se.sics.cooja.mspmote.MspMote;

/**
 * Breakpoint collection
 *
 * @author Fredrik Osterlind
 */
public class MspBreakpointContainer implements WatchpointMote {
  private static Logger logger = Logger.getLogger(MspBreakpointContainer.class);

  private Hashtable<File, Hashtable<Integer, Integer>> debuggingInfo = null;
  private MspMote mspMote;

  private ArrayList<MspBreakpoint> breakpoints = new ArrayList<MspBreakpoint>();
  private ArrayList<ActionListener> listeners = new ArrayList<ActionListener>();
  private MspBreakpoint lastTriggeredBreakpoint = null;

  /**
   * @param debuggingInfo Debugging information read from firmware file
   * @param mote Mote
   */
  public MspBreakpointContainer(MspMote mote, Hashtable<File, Hashtable<Integer, Integer>> debuggingInfo) {
    this.mspMote = mote;
    this.debuggingInfo = debuggingInfo;
  }
  
  /**
   * Add breakpoint at given address.
   *
   * @param address Executable address
   */
  public void addBreakpoint(Integer address) {
    addBreakpoint((File) null, (Integer) null, address);
  }

  /**
   * Add breakpoint at given address with given meta data.
   *
   * @param codeFile Source code file
   * @param lineNr Source code file line number
   * @param address Executable address
   * @return Added breakpoint
   */
  public MspBreakpoint addBreakpoint(File codeFile, int lineNr, Integer address) {
    MspBreakpoint bp = new MspBreakpoint(this, mspMote, address, codeFile, new Integer(lineNr));
    breakpoints.add(bp);

    /* Notify listeners */
    lastTriggeredBreakpoint = null;
    for (ActionListener listener: listeners) {
      listener.actionPerformed(null);
    }
    return bp;
  }

  /**
   * Remove breakpoint at given address.
   *
   * @param address Executable address
   */
  public MspBreakpoint removeBreakpoint(Integer address) {
    MspBreakpoint breakpointToRemove = null;
    for (MspBreakpoint breakpoint: breakpoints) {
      if (breakpoint.getExecutableAddress().intValue() == address.intValue()) {
        breakpointToRemove = breakpoint;
        break;
      }
    }
    if (breakpointToRemove == null) {
      return null;
    }

    breakpointToRemove.unregisterBreakpoint();
    breakpoints.remove(breakpointToRemove);

    /* Notify listeners */
    lastTriggeredBreakpoint = null;
    for (ActionListener listener: listeners) {
      listener.actionPerformed(null);
    }
    return breakpointToRemove;
  }

  /**
   * Checks if a breakpoint exists at given address.
   *
   * @param address Executable address
   * @return True if breakpoint exists, false otherwise
   */
  public boolean breakpointExists(Integer address) {
    if (address == null) {
      return false;
    }

    for (MspBreakpoint breakpoint: breakpoints) {
      if (breakpoint.getExecutableAddress().intValue() == address.intValue()) {
        return true;
      }
    }
    return false;
  }

  public boolean breakpointExists(File file, int lineNr) {
    for (MspBreakpoint breakpoint: breakpoints) {
      if (breakpoint.getCodeFile() == null) {
        continue;
      }
      if (breakpoint.getCodeFile().compareTo(file) != 0) {
        continue;
      }
      if (breakpoint.getLineNumber().intValue() != lineNr) {
        continue;
      }
      return true;
    }
    return false;
  }

  /**
   * @return All breakpoints
   */
  public MspBreakpoint[] getBreakpoints() {
    return breakpoints.toArray(new MspBreakpoint[0]);
  }

  public int getBreakpointsCount() {
    return breakpoints.size();
  }

  public void addWatchpointListener(ActionListener listener) {
    listeners.add(listener);
  }

  public void removeWatchpointListener(ActionListener listener) {
    listeners.remove(listener);
  }

  public ActionListener[] getWatchpointListeners() {
    return listeners.toArray(new ActionListener[0]);
  }
  
  protected void signalBreakpointTrigger(MspBreakpoint b) {
    if (b.stopsSimulation() && mspMote.getSimulation().isRunning()) {
      /* Stop simulation immediately */
      mspMote.getSimulation().stopSimulation();
      mspMote.stopNextInstruction();
    }

    /* Notify listeners */
    lastTriggeredBreakpoint = b;
    for (ActionListener listener: listeners) {
      listener.actionPerformed(null);
    }
  }

  public MspMote getMote() {
    return mspMote;
  }

  public MspBreakpoint getLastWatchpoint() {
    return lastTriggeredBreakpoint;
  }

  /**
   * Tries to calculate the executable address of given file.
   * Using debugging information from firmware file, 
   *
   * @param file Source code file
   * @param lineNr Source code file line number
   * @return Executable address or null if not found
   */
  public Integer getExecutableAddressOf(File file, int lineNr) {
    if (file == null || lineNr < 0 || debuggingInfo == null) {
      return null;
    }

    /* Match file */
    Hashtable<Integer, Integer> lineTable = debuggingInfo.get(file);
    if (lineTable == null) {
      Enumeration<File> fileEnum = debuggingInfo.keys();
      while (fileEnum.hasMoreElements()) {
        File f = fileEnum.nextElement();
        if (f != null && f.getName().equals(file.getName())) {
          lineTable = debuggingInfo.get(f);
          break;
        }
      }
    }
    if (lineTable == null) {
      return null;
    }

    /* Match line number */
    Integer address = lineTable.get(lineNr);
    if (address != null) {
      Enumeration<Integer> lineEnum = lineTable.keys();
      while (lineEnum.hasMoreElements()) {
        Integer l = lineEnum.nextElement();
        if (l != null && l.intValue() == lineNr) {
          /* Found line address */
          return lineTable.get(l);
        }
      }
    }
    
    return null;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    for (MspBreakpoint breakpoint: breakpoints) {
      element = new Element("breakpoint");
      element.addContent(breakpoint.getConfigXML());
      config.add(element);
    }

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("breakpoint")) {
        MspBreakpoint breakpoint = new MspBreakpoint(this, mspMote);
        if (!breakpoint.setConfigXML(element.getChildren(), visAvailable)) {
          logger.warn("Could not restore breakpoint: " + breakpoint);
        } else {
          breakpoints.add(breakpoint);
        }
      }
    }
    return true;
  }
}
