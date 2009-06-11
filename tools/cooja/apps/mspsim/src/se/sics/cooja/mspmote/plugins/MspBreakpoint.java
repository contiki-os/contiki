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
 * $Id: MspBreakpoint.java,v 1.1 2009/06/11 10:05:28 fros4943 Exp $
 */

package se.sics.cooja.mspmote.plugins;

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.Vector;

import org.jdom.Element;

import se.sics.cooja.Watchpoint;
import se.sics.cooja.mspmote.MspMote;
import se.sics.mspsim.core.CPUMonitor;

/**
 * Breakpoint.
 * Contains meta data such source code file and line number.
 *
 * @author Fredrik Osterlind
 */
public class MspBreakpoint implements Watchpoint {
  private MspBreakpointContainer breakpoints;
  private MspMote mspMote;

  private CPUMonitor cpuMonitor = null;

  private boolean stopsSimulation = true;

  private Integer address = null; /* Binary address */

  private File codeFile = null; /* Source code, may be null*/
  private Integer lineNr = null; /* Source code line number, may be null */

  public MspBreakpoint(MspBreakpointContainer breakpoints, MspMote mote) {
    this.breakpoints = breakpoints;
    this.mspMote = mote;

  }

  public MspBreakpoint(MspBreakpointContainer breakpoints, MspMote mote, Integer address) {
    this(breakpoints, mote);
    this.address = address;

    createMonitor();
  }

  public MspBreakpoint(MspBreakpointContainer breakpoints, MspMote mote, Integer address, File codeFile, Integer lineNr) {
    this(breakpoints, mote, address);
    this.codeFile = codeFile;
    this.lineNr = lineNr;
  }

  /**
   * @return MSP mote
   */
  public MspMote getMote() {
    return mspMote;
  }

  /**
   * @return Executable address
   */
  public Integer getExecutableAddress() {
    return address;
  }
  
  /**
   * @return Source code file
   */
  public File getCodeFile() {
    return codeFile;
  }

  /**
   * @return Source code file line number
   */
  public Integer getLineNumber() {
    return lineNr;
  }

  public boolean stopsSimulation() {
    return stopsSimulation;
  }

  public void setStopsSimulation(boolean stops) {
    stopsSimulation = stops;
  }

  private void createMonitor() {
    cpuMonitor = new CPUMonitor() {
      public void cpuAction(int type, int adr, int data) {
        breakpoints.signalBreakpointTrigger(MspBreakpoint.this);
      }
    };
    mspMote.getCPU().setBreakPoint(address, cpuMonitor);
  }
  
  public void unregisterBreakpoint() {
    mspMote.getCPU().setBreakPoint(address, null);
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    element = new Element("address");
    element.setText(address.toString());
    config.add(element);

    element = new Element("stops");
    element.setText("" + stopsSimulation);
    config.add(element);

    if (codeFile != null) {
      element = new Element("codefile");
      File file = mspMote.getSimulation().getGUI().createPortablePath(codeFile);
      element.setText(file.getPath().replaceAll("\\\\", "/"));
      config.add(element);
    }

    if (lineNr != null) {
      element = new Element("line");
      element.setText(lineNr.toString());
      config.add(element);
    }

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    /* Already knows mote and breakpoints */
    
    for (Element element : configXML) {
      if (element.getName().equals("codefile")) {
        File file = new File(element.getText());
        file = mspMote.getSimulation().getGUI().restorePortablePath(file);

        try {
          codeFile = file.getCanonicalFile();
        } catch (IOException e) {
        }

        if (codeFile == null || !codeFile.exists()) {
          return false;
        }
      } else if (element.getName().equals("line")) {
        lineNr = Integer.parseInt(element.getText());
      } else if (element.getName().equals("address")) {
        address = Integer.parseInt(element.getText());
      } else if (element.getName().equals("stops")) {
        stopsSimulation = Boolean.parseBoolean(element.getText());
      }
    }

    if (address == null) {
      return false;
    }

    createMonitor();
    return true;
  }


  public String getDescription() {
    if (codeFile != null) {
      return codeFile.getPath() + ":" + lineNr + " (0x" + Integer.toHexString(address.intValue()) + ")";
    }
    return "0x" + Integer.toHexString(address.intValue());
  }
}
