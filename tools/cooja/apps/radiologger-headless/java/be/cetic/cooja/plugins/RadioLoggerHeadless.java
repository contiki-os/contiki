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

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Plugin;
import se.sics.cooja.PluginType;
import se.sics.cooja.RadioConnection;
import se.sics.cooja.RadioMedium;
import se.sics.cooja.RadioPacket;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.interfaces.Radio;
//import se.sics.cooja.plugins.analyzers.PacketAnalyser;
import se.sics.cooja.plugins.analyzers.PcapExporter;

import se.sics.cooja.util.StringUtils;

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

  public RadioLoggerHeadless(final Simulation simulationToControl, final GUI gui) {
    super("Radio messages", gui, false);
    System.err.println("Starting headless radio logger");
    try {
        pcapExporter = new PcapExporter();
    } catch (IOException e) {
        e.printStackTrace();
    }
    simulation = simulationToControl;
    radioMedium = simulation.getRadioMedium();

    radioMedium.addRadioMediumObserver(radioMediumObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        RadioConnection conn = radioMedium.getLastConnection();
        if (conn == null) {
          return;
        }
        RadioPacket radioPacket = conn.getSource().getLastPacketTransmitted();
        //PacketAnalyser.Packet packet = new PacketAnalyser.Packet(radioPacket.getData(), PacketAnalyser.MAC_LEVEL);
        try {
            //pcapExporter.exportPacketData(packet.getPayload());
            pcapExporter.exportPacketData(radioPacket.getPacketData());
        } catch (IOException e) {
            System.err.println("Could not export PCap data");
            e.printStackTrace();
        }
      }
    });
  }

  public void closePlugin() {
    if (radioMediumObserver != null) {
      radioMedium.deleteRadioMediumObserver(radioMediumObserver);
    }
  }
}
