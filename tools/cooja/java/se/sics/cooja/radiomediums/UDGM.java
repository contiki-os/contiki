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
 * $Id: UDGM.java,v 1.15 2008/03/17 09:49:44 fros4943 Exp $
 */

package se.sics.cooja.radiomediums;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import org.jdom.Element;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.interfaces.ContikiRadio;
import se.sics.cooja.interfaces.*;
import se.sics.cooja.plugins.Visualizer2D;

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

  private static RadioMedium myRadioMedium;

  /* Signal strengths in dBm.
   * Approx. values measured on TmoteSky */
  public static final double SS_NOTHING = -100;

  public static final double SS_STRONG = -10;

  public static final double SS_WEAK = -95;

  private static double SUCCESS_RATIO_TX = 1.0;

  private static double SUCCESS_RATIO_RX = 1.0;

  // Maximum ranges (SS indicator 100)
  private static double TRANSMITTING_RANGE = 50;

  private static double INTERFERENCE_RANGE = 100;

  private Simulation mySimulation;

  private Random random = new Random();


  /**
   * Visualizes radio traffic in the UDGM. Allows a user to
   * change transmission ranges.
   *
   * Sending motes are blue, receiving motes are green and motes that hear noise
   * are painted red. Motes without radios are painted gray, and the rest are
   * white.
   *
   * @author Fredrik Osterlind
   */
  @ClassDescription("UDGM Visualizer")
  @PluginType(PluginType.SIM_PLUGIN)
  public static class VisUDGM extends Visualizer2D {
    private Mote selectedMote = null;

    private JSpinner transmissionSpinner = null;

    private JSpinner interferenceSpinner = null;

    private JSpinner successRatioTxSpinner = null;

    private JSpinner successRatioRxSpinner = null;

    private Observer radioMediumObserver;

    private class ChangeRangesMenuAction implements MoteMenuAction {
      public boolean isEnabled(Mote mote) {
        return true;
      }

      public String getDescription(Mote mote) {
        return "Change transmission ranges";
      }

      public void doAction(Mote mote) {
        transmissionSpinner.setVisible(true);
        interferenceSpinner.setVisible(true);
        repaint();
      }
    };

    private class ChangeSuccessRadioMenuAction implements MoteMenuAction {
      public boolean isEnabled(Mote mote) {
        return true;
      }

      public String getDescription(Mote mote) {
        return "Change transmission success ratio";
      }

      public void doAction(Mote mote) {
        successRatioTxSpinner.setVisible(true);
        successRatioRxSpinner.setVisible(true);
        repaint();
      }
    };

    public VisUDGM(Simulation sim, GUI gui) {
      super(sim, gui);
      setTitle("UDGM Visualizer");

      // Create spinners for changing ranges
      SpinnerNumberModel transmissionModel = new SpinnerNumberModel();
      transmissionModel.setValue(new Double(TRANSMITTING_RANGE));
      transmissionModel.setStepSize(new Double(1.0)); // 1m
      transmissionModel.setMinimum(new Double(0.0));

      SpinnerNumberModel interferenceModel = new SpinnerNumberModel();
      interferenceModel.setValue(new Double(INTERFERENCE_RANGE));
      interferenceModel.setStepSize(new Double(1.0)); // 1m
      interferenceModel.setMinimum(new Double(0.0));

      SpinnerNumberModel successRatioTxModel = new SpinnerNumberModel();
      successRatioTxModel.setValue(new Double(SUCCESS_RATIO_TX));
      successRatioTxModel.setStepSize(new Double(0.001)); // 0.1%
      successRatioTxModel.setMinimum(new Double(0.0));
      successRatioTxModel.setMaximum(new Double(1.0));

      SpinnerNumberModel successRatioRxModel = new SpinnerNumberModel();
      successRatioRxModel.setValue(new Double(SUCCESS_RATIO_RX));
      successRatioRxModel.setStepSize(new Double(0.001)); // 0.1%
      successRatioRxModel.setMinimum(new Double(0.0));
      successRatioRxModel.setMaximum(new Double(1.0));

      JSpinner.NumberEditor editor;
      transmissionSpinner = new JSpinner(transmissionModel);
      editor = new JSpinner.NumberEditor(transmissionSpinner, "0m");
      transmissionSpinner.setEditor(editor);
      interferenceSpinner = new JSpinner(interferenceModel);
      editor = new JSpinner.NumberEditor(interferenceSpinner, "0m");
      interferenceSpinner.setEditor(editor);
      successRatioTxSpinner = new JSpinner(successRatioTxModel);
      editor = new JSpinner.NumberEditor(successRatioTxSpinner, "0.0%");
      successRatioTxSpinner.setEditor(editor);
      successRatioRxSpinner = new JSpinner(successRatioRxModel);
      editor = new JSpinner.NumberEditor(successRatioRxSpinner, "0.0%");
      successRatioRxSpinner.setEditor(editor);


      ((JSpinner.DefaultEditor) transmissionSpinner.getEditor()).getTextField()
      .setColumns(5);
      ((JSpinner.DefaultEditor) interferenceSpinner.getEditor()).getTextField()
      .setColumns(5);
      ((JSpinner.DefaultEditor) successRatioTxSpinner.getEditor()).getTextField()
      .setColumns(5);
      ((JSpinner.DefaultEditor) successRatioRxSpinner.getEditor()).getTextField()
      .setColumns(5);
      transmissionSpinner.setToolTipText("Transmitting range (m)");
      interferenceSpinner.setToolTipText("Interference range (m)");
      successRatioTxSpinner.setToolTipText("Transmission success ratio (%)");
      successRatioRxSpinner.setToolTipText("Reception success ratio (%)");

      transmissionSpinner.addChangeListener(new ChangeListener() {
        public void stateChanged(ChangeEvent e) {
          TRANSMITTING_RANGE = ((SpinnerNumberModel) transmissionSpinner
              .getModel()).getNumber().doubleValue();
          repaint();
        }
      });

      interferenceSpinner.addChangeListener(new ChangeListener() {
        public void stateChanged(ChangeEvent e) {
          INTERFERENCE_RANGE = ((SpinnerNumberModel) interferenceSpinner
              .getModel()).getNumber().doubleValue();
          repaint();
        }
      });

      successRatioTxSpinner.addChangeListener(new ChangeListener() {
        public void stateChanged(ChangeEvent e) {
          SUCCESS_RATIO_TX = ((SpinnerNumberModel) successRatioTxSpinner
              .getModel()).getNumber().doubleValue();
          repaint();
        }
      });

      successRatioRxSpinner.addChangeListener(new ChangeListener() {
        public void stateChanged(ChangeEvent e) {
          SUCCESS_RATIO_RX = ((SpinnerNumberModel) successRatioRxSpinner
              .getModel()).getNumber().doubleValue();
          repaint();
        }
      });

      getCurrentCanvas().add(transmissionSpinner);
      getCurrentCanvas().add(interferenceSpinner);
      getCurrentCanvas().add(successRatioTxSpinner);
      getCurrentCanvas().add(successRatioRxSpinner);
      transmissionSpinner.setVisible(false);
      interferenceSpinner.setVisible(false);
      successRatioTxSpinner.setVisible(false);
      successRatioRxSpinner.setVisible(false);

      // Add mouse listener for selecting motes
      getCurrentCanvas().addMouseListener(new MouseListener() {
        public void mouseExited(MouseEvent e) {
          // Do nothing
        }

        public void mouseEntered(MouseEvent e) {
          // Do nothing
        }

        public void mouseReleased(MouseEvent e) {
          // Do nothing
        }

        public void mousePressed(MouseEvent e) {
          Vector<Mote> clickedMotes = findMotesAtPosition(e.getX(), e.getY());
          if (clickedMotes == null || clickedMotes.size() == 0) {
            selectedMote = null;
            transmissionSpinner.setVisible(false);
            interferenceSpinner.setVisible(false);
            successRatioTxSpinner.setVisible(false);
            successRatioRxSpinner.setVisible(false);
            repaint();
            return;
          }

          // Select one of the clicked motes
          if (clickedMotes.contains(selectedMote)) {
            int pos = clickedMotes.indexOf(selectedMote);
            if (pos < clickedMotes.size() - 1) {
              selectedMote = clickedMotes.get(pos + 1);
            } else {
              selectedMote = clickedMotes.firstElement();
            }
          } else {
            selectedMote = clickedMotes.firstElement();
          }
          repaint();
        }

        public void mouseClicked(MouseEvent e) {
        }
      });

      // Register change ranges and change success ratio action
      addMoteMenuAction(new ChangeRangesMenuAction());
      addMoteMenuAction(new ChangeSuccessRadioMenuAction());

      // Observe our own radio medium
      myRadioMedium
          .addRadioMediumObserver(radioMediumObserver = new Observer() {
            public void update(Observable obs, Object obj) {
              getCurrentCanvas().repaint();
            }
          });

    }

    public void closePlugin() {
      super.closePlugin();

      myRadioMedium.deleteRadioMediumObserver(radioMediumObserver);
    }

    public Color[] getColorOf(Mote mote) {
      Radio moteRadio = mote.getInterfaces().getRadio();
      if (moteRadio == null) {
        return new Color[] { Color.BLACK };
      }

      if (mote.getState() == Mote.State.DEAD) {
        return new Color[] { Color.BLACK };
      }

      if (selectedMote != null && mote == selectedMote) {
        return new Color[] { Color.CYAN };
      }

      if (moteRadio instanceof ContikiRadio && !((ContikiRadio) moteRadio).isOn()) {
        return new Color[] { Color.GRAY };
      }

      if (moteRadio.isTransmitting()) {
        return new Color[] { Color.BLUE };
      }

      if (moteRadio.isInterfered()) {
        return new Color[] { Color.RED };
      }

      if (moteRadio.isReceiving()) {
        return new Color[] { Color.GREEN };
      }

      return new Color[] { Color.WHITE };
    }

    public void visualizeSimulation(Graphics g) {

      // Paint transmission+interference areas for selected mote (if any)
      if (selectedMote != null) {
        Position motePos = selectedMote.getInterfaces().getPosition();

        Point pixelCoord = transformPositionToPixel(motePos);
        int x = pixelCoord.x;
        int y = pixelCoord.y;

        // Fetch current output power indicator (scale with as percent)
        if (selectedMote.getInterfaces().getRadio() != null) {
          Radio selectedRadio = selectedMote.getInterfaces().getRadio();
          double moteInterferenceRange = INTERFERENCE_RANGE
              * (selectedRadio.getCurrentOutputPowerIndicator() / selectedRadio.getOutputPowerIndicatorMax());
          double moteTransmissionRange = TRANSMITTING_RANGE
              * (selectedRadio.getCurrentOutputPowerIndicator() / selectedRadio.getOutputPowerIndicatorMax());

          Point translatedZero = transformPositionToPixel(0.0, 0.0, 0.0);
          Point translatedInterference = transformPositionToPixel(
              moteInterferenceRange, moteInterferenceRange, 0.0);
          Point translatedTransmission = transformPositionToPixel(
              moteTransmissionRange, moteTransmissionRange, 0.0);

          translatedInterference.x = Math.abs(translatedInterference.x
              - translatedZero.x);
          translatedInterference.y = Math.abs(translatedInterference.y
              - translatedZero.y);
          translatedTransmission.x = Math.abs(translatedTransmission.x
              - translatedZero.x);
          translatedTransmission.y = Math.abs(translatedTransmission.y
              - translatedZero.y);

          // Interference
          g.setColor(Color.DARK_GRAY);
          g.fillOval(x - translatedInterference.x,
              y - translatedInterference.y, 2 * translatedInterference.x,
              2 * translatedInterference.y);

          // Transmission
          g.setColor(Color.GREEN);
          g.fillOval(x - translatedTransmission.x,
              y - translatedTransmission.y, 2 * translatedTransmission.x,
              2 * translatedTransmission.y);
        }
      }

      // Let parent paint motes
      super.visualizeSimulation(g);

      // Paint just finished connections
      RadioConnection[] conns;
      if (myRadioMedium != null
          && (conns = myRadioMedium.getLastTickConnections()) != null) {
        for (RadioConnection conn : conns) {
          if (conn != null) {
            Point sourcePoint = transformPositionToPixel(conn.getSource()
                .getPosition());

            // Paint destinations
            for (Radio destRadio : conn.getDestinations()) {
              Position destPos = destRadio.getPosition();
              Point destPoint = transformPositionToPixel(destPos);

              g.setColor(Color.BLACK);
              g
                  .drawLine(sourcePoint.x, sourcePoint.y, destPoint.x,
                      destPoint.y);

            }
          }
        }
      }
    }
  }

  public UDGM(Simulation simulation) {
    super(simulation);

    // Register this radio medium's plugins
    simulation.getGUI().registerTemporaryPlugin(VisUDGM.class);

    myRadioMedium = this;
    mySimulation = simulation;
    random.setSeed(simulation.getRandomSeed());
  }

  public RadioConnection createConnections(Radio sendingRadio) {
    Position sendingPosition = sendingRadio.getPosition();

    RadioConnection newConnection = new RadioConnection(sendingRadio);

    // Fetch current output power indicator (scale with as percent)
    double moteTransmissionRange = TRANSMITTING_RANGE
        * (sendingRadio.getCurrentOutputPowerIndicator() / sendingRadio.getOutputPowerIndicatorMax());
    double moteInterferenceRange = INTERFERENCE_RANGE
        * (sendingRadio.getCurrentOutputPowerIndicator() / sendingRadio.getOutputPowerIndicatorMax());

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
      if (sendingRadio.getChannel() != listeningRadio.getChannel()) {
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
        double distFactor = dist/TRANSMITTING_RANGE;
        double signalStrength = SS_STRONG + distFactor*(SS_WEAK - SS_STRONG);
        dstRadio.setCurrentSignalStrength(signalStrength);
      }
    }

    // Set signal strength on all interferences
    for (RadioConnection conn : getActiveConnections()) {
      for (Radio intfRadio : conn.getInterfered()) {
        double dist = conn.getSource().getPosition().getDistanceTo(intfRadio.getPosition());
        double distFactor = dist/TRANSMITTING_RANGE;
        double signalStrength = SS_STRONG + distFactor*(SS_WEAK - SS_STRONG);
        intfRadio.setCurrentSignalStrength(signalStrength);

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

  public boolean setConfigXML(Collection<Element> configXML,
      boolean visAvailable) {
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
    random.setSeed(mySimulation.getRandomSeed());
    return true;
  }

}
