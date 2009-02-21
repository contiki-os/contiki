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
 * $Id: UDGM.java,v 1.21 2009/02/21 09:49:51 fros4943 Exp $
 */

package se.sics.cooja.radiomediums;

import java.util.*;
import org.jdom.Element;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;
import se.sics.cooja.plugins.VisUDGM;

/**
 * The Unit Disk Graph medium has two different range parameters; one for
 * transmitting and one for interfering other transmissions.
 *
 * The radio medium supports both byte and packet radios.
 *
 * The radio medium registers a visualizer plugin. Via this plugin the current
 * radio states and range parameters can be viewed and changed.
 *
 * The registered radios' signal strengths are updated whenever the radio medium
 * changes. There are three fixed levels: no surrounding traffic heard, noise
 * heard and data heard.
 *
 * The radio output power indicator (0-100) is used in a very simple way; the
 * total transmission (and interfering) range is multiplied with [power_ind]%.
 *
 * @see #SS_STRONG
 * @see #SS_WEAK
 * @see #SS_NOTHING
 *
 * @see VisUDGM
 * @author Fredrik Osterlind
 */
@ClassDescription("Unit Disk Graph Medium (UDGM)")
public class UDGM extends AbstractRadioMedium {
  private static Logger logger = Logger.getLogger(UDGM.class);

  /* Signal strengths in dBm.
   * Approx. values measured on TmoteSky */
  public static final double SS_NOTHING = -100;
  public static final double SS_STRONG = -10;
  public static final double SS_WEAK = -95;

  public double SUCCESS_RATIO_TX = 1.0; /* Success ratio of TX. If this fails, no radios receive the packet */
  public double SUCCESS_RATIO_RX = 1.0; /* Success ratio of RX. If this fails, a single radio does not receive the packet */
  public double TRANSMITTING_RANGE = 50; /* Transmission range. */
  public double INTERFERENCE_RANGE = 100; /* Interference range. Ignored if below transmission range. */

  private Simulation mySimulation;

  private Random random = null;

  public UDGM(Simulation simulation) {
    super(simulation);

    /* Register visualizer plugin */
    simulation.getGUI().registerTemporaryPlugin(VisUDGM.class);

    mySimulation = simulation;
    random = mySimulation.getRandomGenerator();
  }

  public RadioConnection createConnections(Radio sendingRadio) {
    Position sendingPosition = sendingRadio.getPosition();

    RadioConnection newConnection = new RadioConnection(sendingRadio);

    // Fetch current output power indicator (scale with as percent)
    double moteTransmissionRange = TRANSMITTING_RANGE
        * ((double) sendingRadio.getCurrentOutputPowerIndicator() / (double) sendingRadio.getOutputPowerIndicatorMax());
    double moteInterferenceRange = INTERFERENCE_RANGE
        * ((double) sendingRadio.getCurrentOutputPowerIndicator() / (double) sendingRadio.getOutputPowerIndicatorMax());

    /* Fail transmission randomly (affects all receiving nodes) */
    if (SUCCESS_RATIO_TX < 1.0 && random.nextDouble() > SUCCESS_RATIO_TX) {
      return newConnection;
    }

    // Loop through all radios
    for (int listenNr = 0; listenNr < getRegisteredRadios().size(); listenNr++) {
      Radio listeningRadio = getRegisteredRadios().get(listenNr);
      Position listeningRadioPosition = listeningRadio.getPosition();

      // Ignore sending radio and radios on different channels
      if (sendingRadio == listeningRadio) {
        continue;
      }
      if (sendingRadio.getChannel() >= 0 &&
          listeningRadio.getChannel() >= 0 &&
          sendingRadio.getChannel() != listeningRadio.getChannel()) {
        continue;
      }

      double distance = sendingPosition.getDistanceTo(listeningRadioPosition);

      if (distance <= moteTransmissionRange) {
        // Check if this radio is able to receive transmission
        if (listeningRadio.isInterfered()) {
          // Keep interfering radio
          newConnection.addInterfered(listeningRadio);

        } else if (listeningRadio.isReceiving() ||
            (SUCCESS_RATIO_RX < 1.0 && random.nextDouble() > SUCCESS_RATIO_RX)) {
          newConnection.addInterfered(listeningRadio);

          // Start interfering radio
          listeningRadio.interfereAnyReception();

          // Update connection that is transmitting to this radio
          RadioConnection existingConn = null;
          for (RadioConnection conn : getActiveConnections()) {
            for (Radio dstRadio : conn.getDestinations()) {
              if (dstRadio == listeningRadio) {
                existingConn = conn;
                break;
              }
            }
          }
          if (existingConn != null) {
            // Change radio from receiving to interfered
            existingConn.removeDestination(listeningRadio);
            existingConn.addInterfered(listeningRadio);

          }
        } else {
          // Radio OK to receive
          newConnection.addDestination(listeningRadio);
          listeningRadio.signalReceptionStart();
        }
      } else if (distance <= moteInterferenceRange) {
        // Interfere radio
        newConnection.addInterfered(listeningRadio);
        listeningRadio.interfereAnyReception();
      }
    }

    return newConnection;
  }

  public void updateSignalStrengths() {
    // // Save old signal strengths
    // double[] oldSignalStrengths = new double[registeredRadios.size()];
    // for (int i = 0; i < registeredRadios.size(); i++) {
    // oldSignalStrengths[i] = registeredRadios.get(i)
    // .getCurrentSignalStrength();
    // }

    // Reset signal strength on all radios
    for (Radio radio : getRegisteredRadios()) {
      radio.setCurrentSignalStrength(SS_NOTHING);
    }

    // Set signal strength on all OK transmissions
    for (RadioConnection conn : getActiveConnections()) {
      conn.getSource().setCurrentSignalStrength(SS_STRONG);
      for (Radio dstRadio : conn.getDestinations()) {
        double dist = conn.getSource().getPosition().getDistanceTo(dstRadio.getPosition());

        double maxTxDist = TRANSMITTING_RANGE
        * ((double) conn.getSource().getCurrentOutputPowerIndicator() / (double) conn.getSource().getOutputPowerIndicatorMax());
        double distFactor = dist/maxTxDist;

        double signalStrength = SS_STRONG + distFactor*(SS_WEAK - SS_STRONG);
        dstRadio.setCurrentSignalStrength(signalStrength);
      }
    }

    // Set signal strength on all interferences
    for (RadioConnection conn : getActiveConnections()) {
      for (Radio intfRadio : conn.getInterfered()) {
        double dist = conn.getSource().getPosition().getDistanceTo(intfRadio.getPosition());

        double maxTxDist = TRANSMITTING_RANGE
        * ((double) conn.getSource().getCurrentOutputPowerIndicator() / (double) conn.getSource().getOutputPowerIndicatorMax());
        double distFactor = dist/maxTxDist;

        if (distFactor < 1) {
          double signalStrength = SS_STRONG + distFactor*(SS_WEAK - SS_STRONG);
          intfRadio.setCurrentSignalStrength(signalStrength);
        } else {
          intfRadio.setCurrentSignalStrength(SS_WEAK);
        }

        if (!intfRadio.isInterfered()) {
          // Set to interfered again
          intfRadio.interfereAnyReception();
        }
      }
    }

    // // Fetch new signal strengths
    // double[] newSignalStrengths = new double[registeredRadios.size()];
    // for (int i = 0; i < registeredRadios.size(); i++) {
    // newSignalStrengths[i] = registeredRadios.get(i)
    // .getCurrentSignalStrength();
    // }
    //
    // // Compare new and old signal strengths
    // for (int i = 0; i < registeredRadios.size(); i++) {
    // if (oldSignalStrengths[i] != newSignalStrengths[i])
    // logger.warn("Signal strengths changed on radio[" + i + "]: "
    // + oldSignalStrengths[i] + " -> " + newSignalStrengths[i]);
    // }
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    // Transmitting range
    element = new Element("transmitting_range");
    element.setText(Double.toString(TRANSMITTING_RANGE));
    config.add(element);

    // Interference range
    element = new Element("interference_range");
    element.setText(Double.toString(INTERFERENCE_RANGE));
    config.add(element);

    /* Transmission success probability */
    element = new Element("success_ratio_tx");
    element.setText("" + SUCCESS_RATIO_TX);
    config.add(element);

    /* Reception success probability */
    element = new Element("success_ratio_rx");
    element.setText("" + SUCCESS_RATIO_RX);
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("transmitting_range")) {
        TRANSMITTING_RANGE = Double.parseDouble(element.getText());
      }

      if (element.getName().equals("interference_range")) {
        INTERFERENCE_RANGE = Double.parseDouble(element.getText());
      }

      /* Backwards compatibility */
      if (element.getName().equals("success_ratio")) {
        SUCCESS_RATIO_TX = Double.parseDouble(element.getText());
        logger.warn("Loading old COOJA Config, XML element \"sucess_ratio\" parsed at \"sucess_ratio_tx\"");
      }

      if (element.getName().equals("success_ratio_tx")) {
        SUCCESS_RATIO_TX = Double.parseDouble(element.getText());
      }

      if (element.getName().equals("success_ratio_rx")) {
        SUCCESS_RATIO_RX = Double.parseDouble(element.getText());
      }
    }
    return true;
  }

}
