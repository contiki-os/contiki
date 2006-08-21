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
 * $Id: StandardRadioMedium.java,v 1.1 2006/08/21 12:13:13 fros4943 Exp $
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
import se.sics.cooja.interfaces.*;
import se.sics.cooja.plugins.Visualizer2D;

/**
 * Standard radio medium has two different range parameters; one for
 * transmitting and one for interfering other transmissions.
 * 
 * Radio data are analysed when all registered motes have ticked once (this
 * medium is registered as a tick observer). For every mote in transmission
 * range of a sending mote, current listen status is changed from hearing
 * nothing to hearing packet, or hearing packet to hearing noise. This way, if a
 * mote hears two packets during one tick loop, it will receive none (noise). If
 * a mote is in the interference range, the current listen status will be set to
 * hears noise immediately.
 * 
 * This radio medium registers a dynamic visual plugin: Radio Medium - Standard.
 * Via this plugin ranges can be viewed and changed. Active connection may also
 * be viewed.
 * 
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Standard Radio")
public class StandardRadioMedium extends RadioMedium {
  private static Logger logger = Logger.getLogger(StandardRadioMedium.class);

  private Vector<Position> registeredPositions = new Vector<Position>();
  private Vector<Radio> registeredRadios = new Vector<Radio>();

  private Vector<Position> sendingPositions = new Vector<Position>();
  private Vector<Radio> sendingRadios = new Vector<Radio>();

  private static RadioMedium myRadioMedium;

  /**
   * Visualizes radio traffic in the current radio medium. Allows a user to
   * change transmission ranges.
   * 
   * Sending motes are blue, receiving motes are green and motes that hear noise
   * are painted red. Motes without radios are painted gray, and the rest are
   * white.
   * 
   * @author Fredrik Osterlind
   */
  @ClassDescription("Radio Medium - Standard")
  @VisPluginType(VisPluginType.SIM_PLUGIN)
  public static class RadioMediumSimPlugin extends Visualizer2D {
    private Mote selectedMote = null;

    private JSpinner transmissionSpinner = null;
    private JSpinner interferenceSpinner = null;

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

    public RadioMediumSimPlugin(Simulation sim) {
      super(sim);
      setTitle("Radio Medium - Standard");

      // Create spinners for changing ranges
      SpinnerNumberModel transmissionModel = new SpinnerNumberModel();
      transmissionModel.setValue(new Double(TRANSMITTING_RANGE));
      transmissionModel.setStepSize(new Double(1.0)); // 1m
      transmissionModel.setMinimum(new Double(0.0));

      SpinnerNumberModel interferenceModel = new SpinnerNumberModel();
      interferenceModel.setValue(new Double(INTERFERENCE_RANGE));
      interferenceModel.setStepSize(new Double(1.0)); // 1m
      interferenceModel.setMinimum(new Double(0.0));

      transmissionSpinner = new JSpinner(transmissionModel);
      interferenceSpinner = new JSpinner(interferenceModel);

      ((JSpinner.DefaultEditor) transmissionSpinner.getEditor()).getTextField()
          .setColumns(5);
      ((JSpinner.DefaultEditor) interferenceSpinner.getEditor()).getTextField()
          .setColumns(5);
      transmissionSpinner.setToolTipText("Transmitting range");
      interferenceSpinner.setToolTipText("Interference range");

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

      getCurrentCanvas().add(transmissionSpinner);
      getCurrentCanvas().add(interferenceSpinner);
      transmissionSpinner.setVisible(false);
      interferenceSpinner.setVisible(false);

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
            repaint();
            return;
          }

          // Select one of the clicked motes
          if (clickedMotes.contains(selectedMote)) {
            int pos = clickedMotes.indexOf(selectedMote);
            if (pos < clickedMotes.size() - 1)
              selectedMote = clickedMotes.get(pos + 1);
            else
              selectedMote = clickedMotes.firstElement();
          } else {
            selectedMote = clickedMotes.firstElement();
          }
          repaint();
        }
        public void mouseClicked(MouseEvent e) {
        }
      });

      // Register change ranges action
      addMoteMenuAction(new ChangeRangesMenuAction());

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
      if (moteRadio == null)
        return new Color[]{Color.GRAY};

      if (selectedMote != null && mote == selectedMote)
        return new Color[]{Color.CYAN};

      if (moteRadio.getSendState() == Radio.SENT_SOMETHING)
        return new Color[]{Color.BLUE};

      if (moteRadio.getListenState() == Radio.HEARS_PACKET)
        return new Color[]{Color.GREEN};

      if (moteRadio.getListenState() == Radio.HEARS_NOISE)
        return new Color[]{Color.RED};

      return new Color[]{Color.WHITE};
    }

    public void visualizeSimulation(Graphics g) {

      // Paint transmission+interference areas for selected mote (if any)
      if (selectedMote != null) {
        Position motePos = selectedMote.getInterfaces().getPosition();

        Point pixelCoord = transformPositionToPixel(motePos);
        int x = pixelCoord.x;
        int y = pixelCoord.y;

        Point translatedZero = transformPositionToPixel(0.0, 0.0, 0.0);
        Point translatedInterference = transformPositionToPixel(
            INTERFERENCE_RANGE, INTERFERENCE_RANGE, 0.0);
        Point translatedTransmission = transformPositionToPixel(
            TRANSMITTING_RANGE, TRANSMITTING_RANGE, 0.0);

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
        g.fillOval(x - translatedInterference.x, y - translatedInterference.y,
            2 * translatedInterference.x, 2 * translatedInterference.y);

        // Transmission
        g.setColor(Color.GREEN);
        g.fillOval(x - translatedTransmission.x, y - translatedTransmission.y,
            2 * translatedTransmission.x, 2 * translatedTransmission.y);

      }

      // Let parent paint motes
      super.visualizeSimulation(g);

      // Paint last tick connections
      if (myRadioMedium != null
          && myRadioMedium.getLastTickConnections() != null) {
        for (RadioConnection conn : myRadioMedium.getLastTickConnections()) {
          if (conn != null) {
            Point sourcePoint = transformPositionToPixel(conn
                .getSourcePosition());

            // Paint destinations
            for (Position destPos : conn.getDestinationPositons()) {
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

  public StandardRadioMedium() {
    // Register this radio medium's plugins
    GUI.currentGUI.registerTemporaryPlugin(RadioMediumSimPlugin.class);
    myRadioMedium = this;
  }

  private boolean isTickObserver = false;

  private static double TRANSMITTING_RANGE = 20; // 20m
  private static double INTERFERENCE_RANGE = 40; // 40m

  private static boolean RECEIVE_MY_OWN_PACKETS = false;
  private static boolean DETECT_MY_OWN_PACKETS = true;

  private class RadioMediumObservable extends Observable {
    private void transferredData() {
      setChanged();
      notifyObservers();
    }
  }
  private RadioMediumObservable radioMediumObservable = new RadioMediumObservable();

  private RadioConnection[] lastTickConnections = null;

  private ConnectionLogger myLogger = null;

  private Observer radioDataObserver = new Observer() {
    public void update(Observable radio, Object obj) {
      if (((Radio) radio).getSendState() == Radio.SENT_SOMETHING) {
        if (!sendingPositions.contains((Position) obj)) {
          sendingPositions.add((Position) obj);
          sendingRadios.add((Radio) radio);
        }
      }
    }
  };

  private Observer tickObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      RadioConnection[] oldTickConnections = lastTickConnections;
      lastTickConnections = null;
      if (sendingPositions.size() > 0) {
        final int numberSending = sendingPositions.size();
        lastTickConnections = new RadioConnection[numberSending];

        // Loop through all sending radios
        for (int sendNr = 0; sendNr < numberSending; sendNr++) {
          Radio sendingRadio = sendingRadios.get(sendNr);
          byte[] dataToSend = sendingRadio.getLastPacketSent();

          lastTickConnections[sendNr] = new RadioConnection();
          lastTickConnections[sendNr].setSource(sendingRadios.get(sendNr),
              sendingPositions.get(sendNr), dataToSend);

          // Set sending radio unable to receive any more data
          if (RECEIVE_MY_OWN_PACKETS) {
            sendingRadio.receivePacket(dataToSend);
            sendingRadio.advanceListenState();
          } else if (DETECT_MY_OWN_PACKETS) {
            sendingRadio.setListenState(Radio.HEARS_NOISE);
          }

          // Loop through all radios that are listening
          for (int listenNr = 0; listenNr < registeredPositions.size(); listenNr++) {

            // If not the sending radio..
            if (sendingRadios.get(sendNr) != registeredRadios.get(listenNr)) {
              Radio listeningRadio = registeredRadios.get(listenNr);

              double distance = sendingPositions.get(sendNr).getDistanceTo(
                  registeredPositions.get(listenNr));

              if (distance <= TRANSMITTING_RANGE) {
                lastTickConnections[sendNr].addDestination(registeredRadios
                    .get(listenNr), registeredPositions.get(listenNr),
                    dataToSend);

                // If close enough to transmit ok..
                if (listeningRadio.getListenState() == Radio.HEARS_PACKET) {
                  // .. but listening radio already received a packet
                  listeningRadio.advanceListenState();
                } else if (listeningRadio.getListenState() == Radio.HEARS_NOISE) {
                  // .. but listening radio heard interference
                  listeningRadio.advanceListenState();
                } else {
                  // .. send packet
                  listeningRadio.receivePacket(dataToSend);
                  listeningRadio.setListenState(Radio.HEARS_PACKET);
                }
              } else if (distance <= INTERFERENCE_RANGE) {
                // If close enough to sabotage other transmissions..
                if (listeningRadio.getListenState() == Radio.HEARS_PACKET) {
                  // .. and listening radio already received a packet
                  listeningRadio.advanceListenState();
                } else {
                  // .. and listening radio has done nothing sofar
                  listeningRadio.setListenState(Radio.HEARS_NOISE);
                }
              }
              // else too far away
            }
          }
        }
        sendingPositions.clear();
        sendingRadios.clear();

        if (myLogger != null) {
          for (RadioConnection conn : lastTickConnections)
            myLogger.logConnection(conn);
        }

      }
      if (lastTickConnections != oldTickConnections)
        radioMediumObservable.transferredData();
    }
  };

  public void registerMote(Mote mote, Simulation sim) {
    registerRadioInterface(mote.getInterfaces().getRadio(), mote
        .getInterfaces().getPosition(), sim);
  }

  public void unregisterMote(Mote mote, Simulation sim) {
    unregisterRadioInterface(mote.getInterfaces().getRadio(), sim);
  }

  public void registerRadioInterface(Radio radio, Position position,
      Simulation sim) {
    if (radio != null && position != null) {
      if (!isTickObserver) {
        sim.addTickObserver(tickObserver);
        isTickObserver = true;
      }

      registeredPositions.add(position);
      registeredRadios.add(radio);
      radio.addObserver(radioDataObserver);
    } // else logger.warn("Standard Radio Medium not registering mote");
  }

  public void unregisterRadioInterface(Radio radio, Simulation sim) {
    for (int i = 0; i < registeredRadios.size(); i++) {
      if (registeredRadios.get(i).equals(radio)) {
        registeredRadios.remove(i);
        registeredPositions.remove(i);
        radio.deleteObserver(radioDataObserver);
        return;
      }
    }
    logger.warn("Could not find radio: " + radio + " to unregister");
  }

  public void addRadioMediumObserver(Observer observer) {
    radioMediumObservable.addObserver(observer);
  }

  public void deleteRadioMediumObserver(Observer observer) {
    radioMediumObservable.deleteObserver(observer);
  }

  public RadioConnection[] getLastTickConnections() {
    return lastTickConnections;
  }

  public void setConnectionLogger(ConnectionLogger connection) {
    myLogger = connection;
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

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML) {
    for (Element element : configXML) {
      if (element.getName().equals("transmitting_range")) {
        TRANSMITTING_RANGE = Double.parseDouble(element.getText());
      }

      if (element.getName().equals("interference_range")) {
        INTERFERENCE_RANGE = Double.parseDouble(element.getText());
      }
    }
    return true;
  }

}
