/*
 * Copyright (c) 2013, CETIC.
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

package be.cetic.cooja.plugins;

import java.io.IOException;
import java.util.Observable;
import java.util.Observer;
import java.util.Properties;
import java.util.Collection;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.io.File;
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.Plugin;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.RadioConnection;
import org.contikios.cooja.RadioMedium;
import org.contikios.cooja.RadioPacket;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.VisPlugin;
import org.contikios.cooja.interfaces.Radio;
import org.contikios.cooja.interfaces.Radio.RadioEvent;
//import org.contikios.cooja.plugins.analyzers.PacketAnalyser;
import org.contikios.cooja.plugins.analyzers.PcapExporter;
import org.contikios.cooja.ConvertedRadioPacket;

import org.contikios.cooja.util.StringUtils;

/**
 * Radio Logger which exports a pcap file only.
 * It was designed to support radio logging in COOJA's headless mode.
 * Based on Fredrik Osterlind's RadioLogger.java
 *
 * @author Laurent Deru
 */
@ClassDescription("Headless radio logger")
@PluginType(PluginType.SIM_PLUGIN)
public class RadioLoggerHeadless extends VisPlugin {
  private static final long serialVersionUID = -6927091711697081353L;

  private final Simulation simulation;
  private RadioMedium radioMedium;
  private Observer radioMediumObserver;
  private PcapExporter pcapExporter;
  private File pcapFile;
  
  private HashMap<Integer,byte[]> lastPacket;

  public RadioLoggerHeadless(final Simulation simulationToControl, final Cooja cooja) {
    super("Radio messages", cooja, false);
    System.err.println("Starting headless radio logger");
    try {
        pcapExporter = new PcapExporter();
    } catch (IOException e) {
        e.printStackTrace();
    }
    simulation = simulationToControl;
    radioMedium = simulation.getRadioMedium();
    lastPacket = new HashMap<Integer,byte[]>();

    radioMedium.addRadioTransmissionObserver(radioMediumObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        RadioConnection conn = radioMedium.getLastConnection();
        if (conn == null || conn.getSource().getLastEvent() != Radio.RadioEvent.TRANSMISSION_FINISHED) {
          return;
        }
        int moteId = conn.getSource().getMote().getID();
        byte[] data;
        if (conn.getSource().getLastPacketTransmitted() instanceof ConvertedRadioPacket) {
          data = ((ConvertedRadioPacket) conn.getSource().getLastPacketTransmitted()).getOriginalPacketData();
        } else {
          data = conn.getSource().getLastPacketTransmitted().getPacketData();
        }
        try {
          if(!(lastPacket.containsKey(moteId) && Arrays.equals(data,lastPacket.get(moteId)))){
            pcapExporter.exportPacketData(data);
            lastPacket.put(moteId,data);
          }
        } catch (IOException e) {
            System.err.println("Could not export pcap data");
            e.printStackTrace();
        }
      }
    });
  }

  public void closePlugin() {
    if (radioMediumObserver != null) {
      radioMedium.deleteRadioTransmissionObserver(radioMediumObserver);
    }
  }
  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    System.err.println("RadioLogger.setConfigXML()");
    for (Element element : configXML) {
      String name = element.getName();
      if (name.equals("pcap_file")) {
        pcapFile = simulation.getCooja().restorePortablePath(new File(element.getText()));
        try {
          pcapExporter.openPcap(pcapFile);
          } catch (IOException e) {
            e.printStackTrace();
          }
        }
      }
    return true;
  }

  public Collection<Element> getConfigXML() {
    System.err.println("RadioLogger.getConfigXML()");
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    if (pcapFile != null) {
      element = new Element("pcap_file");
      File file = simulation.getCooja().createPortablePath(pcapFile);
      element.setText(pcapFile.getPath().replaceAll("\\\\", "/"));
      element.setAttribute("EXPORT", "discard");
      config.add(element);
    }

    return config;
  }

}
