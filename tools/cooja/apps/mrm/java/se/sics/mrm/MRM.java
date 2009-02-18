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
 * $Id: MRM.java,v 1.8 2009/02/18 12:08:10 fros4943 Exp $
 */

package se.sics.mrm;

import java.util.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;
import se.sics.cooja.radiomediums.AbstractRadioMedium;

/**
 * This is the main class of the COOJA Multi-path Ray-tracing Medium (MRM)
 * package.
 *
 * MRM is meant to be an alternative to the simpler radio mediums available in
 * COOJA. It is packet based and uses a 2D ray-tracing approach to approximate
 * signal strength attenuations between simulated radios. Currently the
 * ray-tracing only supports reflections and refractions through homogeneous
 * obstacles.
 *
 * MRM provides a number of plugins for example a plugin for visualizing radio
 * environments, and a plugin for configuring the radio medium.
 *
 * Future work includes adding diffractions and scattering support.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Multi-path Ray-tracer Medium (MRM)")
public class MRM extends AbstractRadioMedium {
  private static Logger logger = Logger.getLogger(MRM.class);

  private ChannelModel currentChannelModel = null;
  private Simulation mySimulation = null;

  private Random random = null;

  /**
   * Notifies observers when this radio medium has changed settings.
   */
  private SettingsObservable settingsObservable = new SettingsObservable();

  /**
   * Creates a new Multi-path Ray-tracing Medium (MRM).
   */
  public MRM(Simulation simulation) {
    super(simulation);
    random = simulation.getRandomGenerator();

    // Create the channel model
    currentChannelModel = new ChannelModel();

    // Register temporary plugins
    simulation.getGUI().registerTemporaryPlugin(AreaViewer.class);
    simulation.getGUI().registerTemporaryPlugin(FormulaViewer.class);
  }

  public MRMRadioConnection createConnections(Radio sendingRadio) {
    Position sendingPosition = sendingRadio.getPosition();
    MRMRadioConnection newConnection = new MRMRadioConnection(sendingRadio);

    // Loop through all radios
    for (Radio listeningRadio: getRegisteredRadios()) {
      // Ignore sending radio and radios on different channels
      if (sendingRadio == listeningRadio) {
        continue;
      }
      if (sendingRadio.getChannel() >= 0 &&
          listeningRadio.getChannel() >= 0 &&
          sendingRadio.getChannel() != listeningRadio.getChannel()) {
        continue;
      }

      double listeningPositionX = listeningRadio.getPosition().getXCoordinate();
      double listeningPositionY = listeningRadio.getPosition().getYCoordinate();

      // Calculate probability of reception of listening radio
      double[] probData = currentChannelModel.getProbability(
          sendingPosition.getXCoordinate(),
          sendingPosition.getYCoordinate(),
          listeningPositionX,
          listeningPositionY,
          -Double.MAX_VALUE
      );

      //logger.info("Probability of reception is " + probData[0]);
      //logger.info("Signal strength at destination is " + probData[1]);
      if (random.nextFloat() < probData[0]) {
        // Check if this radio is able to receive transmission
        if (listeningRadio.isInterfered()) {
          // Keep interfering radio
          newConnection.addInterfered(listeningRadio, probData[1]);

        } else if (listeningRadio.isReceiving()) {
          newConnection.addInterfered(listeningRadio, probData[1]);

          // Start interfering radio
          listeningRadio.interfereAnyReception();

          // Update connection that is transmitting to this radio
          MRMRadioConnection existingConn = null;
          for (RadioConnection conn : getActiveConnections()) {
            for (Radio dstRadio : ((MRMRadioConnection) conn).getDestinations()) {
              if (dstRadio == listeningRadio) {
                existingConn = (MRMRadioConnection) conn;
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
          //logger.info("OK, creating connection and starting to transmit");
          newConnection.addDestination(listeningRadio, probData[1]);
          listeningRadio.signalReceptionStart();
        }
      } else if (probData[1] > currentChannelModel.getParameterDoubleValue("bg_noise_mean")) {
        // Interfere radio
        newConnection.addInterfered(listeningRadio, probData[1]);
        listeningRadio.interfereAnyReception();

        // TODO Radios always get interfered right now, should recalculate probability
//      if (maxInterferenceSignalStrength + SOME_RELEVANT_LIMIT > transmissionSignalStrength) {
//      // Recalculating probability of delivery
//      double[] probData = currentChannelModel.getProbability(
//          mySource.source.position.getXCoordinate(),
//          mySource.source.position.getYCoordinate(),
//          myDestination.position.getXCoordinate(),
//          myDestination.position.getYCoordinate(),
//          maxInterferenceSignalStrength);
//
//      if (new Random().nextFloat() >= probData[0]) {
//        return true;
//      }

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
      radio.setCurrentSignalStrength(currentChannelModel.getParameterDoubleValue(("bg_noise_mean")));
    }

    // Set signal strength on all OK transmissions
    for (RadioConnection conn : getActiveConnections()) {
//    ((MRMRadioConnection) conn).getSource().setCurrentSignalStrength(12345); // TODO Set signal strength on source?
      for (Radio dstRadio : ((MRMRadioConnection) conn).getDestinations()) {
        double signalStrength = ((MRMRadioConnection) conn).getDestinationSignalStrength(dstRadio);
        if (signalStrength > dstRadio.getCurrentSignalStrength()) {
          dstRadio.setCurrentSignalStrength(signalStrength);
        }
      }
    }

    // Set signal strength on all interferences
    for (RadioConnection conn : getActiveConnections()) {
      for (Radio interferedRadio : ((MRMRadioConnection) conn).getInterfered()) {
        double signalStrength = ((MRMRadioConnection) conn).getInterferenceSignalStrength(interferedRadio);
        if (signalStrength > interferedRadio.getCurrentSignalStrength()) {
          interferedRadio.setCurrentSignalStrength(signalStrength);
        }

        if (!interferedRadio.isInterfered()) {
          // Set to interfered again
          interferedRadio.interfereAnyReception();
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
    // Forwarding to current channel model
    return currentChannelModel.getConfigXML();
  }

  public boolean setConfigXML(Collection<Element> configXML,
      boolean visAvailable) {
    // Forwarding to current channel model
    return currentChannelModel.setConfigXML(configXML);
  }


  // -- MRM specific methods --

  /**
   * Adds an observer which is notified when this radio medium has
   * changed settings, such as added or removed radios.
   *
   * @param obs New observer
   */
  public void addSettingsObserver(Observer obs) {
    settingsObservable.addObserver(obs);
  }

  /**
   * Deletes an earlier registered setting observer.
   *
   * @param osb
   *          Earlier registered observer
   */
  public void deleteSettingsObserver(Observer obs) {
    settingsObservable.deleteObserver(obs);
  }

  /**
   * Returns position of given radio.
   *
   * @param radio Registered radio
   * @return Position of given radio
   */
  public Position getRadioPosition(Radio radio) {
    return radio.getPosition();
  }

  /**
   * @return Number of registered radios.
   */
  public int getRegisteredRadioCount() {
    return getRegisteredRadios().size();
  }

  /**
   * Returns radio at given index.
   *
   * @param index Index of registered radio.
   * @return Radio at given index
   */
  public Radio getRegisteredRadio(int index) {
    return getRegisteredRadios().get(index);
  }

  /**
   * Returns the current channel model object, responsible for
   * all probability and transmission calculations.
   *
   * @return Current channel model
   */
  public ChannelModel getChannelModel() {
    return currentChannelModel;
  }

  class SettingsObservable extends Observable {
    private void notifySettingsChanged() {
      setChanged();
      notifyObservers();
    }
  }

  class MRMRadioConnection extends RadioConnection {
    private Hashtable<Radio, Double> signalStrengths = new Hashtable<Radio, Double>();

    public MRMRadioConnection(Radio sourceRadio) {
      super(sourceRadio);
    }

    public void addDestination(Radio radio, double signalStrength) {
      signalStrengths.put(radio, signalStrength);
      addDestination(radio);
    }

    public void addInterfered(Radio radio, double signalStrength) {
      signalStrengths.put(radio, signalStrength);
      addInterfered(radio);
    }

    public double getDestinationSignalStrength(Radio radio) {
      return signalStrengths.get(radio);
    }

    public double getInterferenceSignalStrength(Radio radio) {
      return signalStrengths.get(radio);
    }
  }

}
