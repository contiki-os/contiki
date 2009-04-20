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
 * $Id: UDGMVisualizerSkin.java,v 1.6 2009/04/20 18:09:54 fros4943 Exp $
 */

package se.sics.cooja.plugins.skins;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JLabel;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.RadioConnection;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.VisualizerSkin;
import se.sics.cooja.plugins.Visualizer.SimulationMenuAction;
import se.sics.cooja.radiomediums.UDGM;

/**
 * Visualizer skin for Unit Disk Graph radio medium.
 *
 * Allows a user to change the collective TX/interference ranges,
 * and the TX/RX success ratio.
 *
 * Transmitting motes are painted blue. XXXXXXXX
 * Receiving motes are painted green.
 * Interfered motes are painted red.
 * Motes without radios are painted gray.
 * All other motes are painted white.
 *
 * @see UDGM
 * @author Fredrik Osterlind
 */
@ClassDescription("Radio environment (UDGM)")
public class UDGMVisualizerSkin implements VisualizerSkin {
  private static Logger logger = Logger.getLogger(UDGMVisualizerSkin.class);

  private static final boolean DRAW_ARROWS = true;
  
  private Simulation simulation = null;
  private Visualizer visualizer = null;
  private UDGM radioMedium = null;
  private Observer radioMediumObserver;

  private JLabel txCounter = null;
  private JLabel rxCounter = null;
  private JLabel interferedCounter = null;

  private JSpinner txRangeSpinner = null;
  private JSpinner interferenceRangeSpinner = null;
  private JSpinner successRatioTxSpinner = null;
  private JSpinner successRatioRxSpinner = null;

  private Mote selectedMote = null;

  private MouseListener selectMoteMouseListener = new MouseListener() {
    public void mouseExited(MouseEvent e) { }
    public void mouseEntered(MouseEvent e) { }
    public void mouseReleased(MouseEvent e) { }
    public void mouseClicked(MouseEvent e) { }

    public void mousePressed(MouseEvent e) {
      Mote[] motes = visualizer.findMotesAtPosition(e.getX(), e.getY());
      if (motes == null || motes.length == 0) {
        selectedMote = null;
        txRangeSpinner.setVisible(false);
        interferenceRangeSpinner.setVisible(false);
        successRatioTxSpinner.setVisible(false);
        successRatioRxSpinner.setVisible(false);
        visualizer.repaint();
        return;
      }

      selectedMote = motes[0];
      visualizer.repaint();
    }
  };

  public void setActive(Simulation simulation, Visualizer vis) {
    if (!(simulation.getRadioMedium() instanceof UDGM)) {
      logger.fatal("Cannot activate UDGM skin for unknown radio medium: " + simulation.getRadioMedium());
      return;
    }
    this.simulation = simulation;
    this.visualizer = vis;
    this.radioMedium = (UDGM) simulation.getRadioMedium();

    /* Spinner GUI components */
    SpinnerNumberModel transmissionModel = new SpinnerNumberModel();
    transmissionModel.setValue(new Double(radioMedium.TRANSMITTING_RANGE));
    transmissionModel.setStepSize(new Double(1.0)); // 1m
    transmissionModel.setMinimum(new Double(0.0));

    SpinnerNumberModel interferenceModel = new SpinnerNumberModel();
    interferenceModel.setValue(new Double(radioMedium.INTERFERENCE_RANGE));
    interferenceModel.setStepSize(new Double(1.0)); // 1m
    interferenceModel.setMinimum(new Double(0.0));

    SpinnerNumberModel successRatioTxModel = new SpinnerNumberModel();
    successRatioTxModel.setValue(new Double(radioMedium.SUCCESS_RATIO_TX));
    successRatioTxModel.setStepSize(new Double(0.001)); // 0.1%
    successRatioTxModel.setMinimum(new Double(0.0));
    successRatioTxModel.setMaximum(new Double(1.0));

    SpinnerNumberModel successRatioRxModel = new SpinnerNumberModel();
    successRatioRxModel.setValue(new Double(radioMedium.SUCCESS_RATIO_RX));
    successRatioRxModel.setStepSize(new Double(0.001)); // 0.1%
    successRatioRxModel.setMinimum(new Double(0.0));
    successRatioRxModel.setMaximum(new Double(1.0));

    JSpinner.NumberEditor editor;
    txRangeSpinner = new JSpinner(transmissionModel);
    editor = new JSpinner.NumberEditor(txRangeSpinner, "0m");
    txRangeSpinner.setEditor(editor);
    interferenceRangeSpinner = new JSpinner(interferenceModel);
    editor = new JSpinner.NumberEditor(interferenceRangeSpinner, "0m");
    interferenceRangeSpinner.setEditor(editor);
    successRatioTxSpinner = new JSpinner(successRatioTxModel);
    editor = new JSpinner.NumberEditor(successRatioTxSpinner, "0.0%");
    successRatioTxSpinner.setEditor(editor);
    successRatioRxSpinner = new JSpinner(successRatioRxModel);
    editor = new JSpinner.NumberEditor(successRatioRxSpinner, "0.0%");
    successRatioRxSpinner.setEditor(editor);

    ((JSpinner.DefaultEditor) txRangeSpinner.getEditor()).getTextField().setColumns(5);
    ((JSpinner.DefaultEditor) interferenceRangeSpinner.getEditor()).getTextField().setColumns(5);
    ((JSpinner.DefaultEditor) successRatioTxSpinner.getEditor()).getTextField().setColumns(5);
    ((JSpinner.DefaultEditor) successRatioRxSpinner.getEditor()).getTextField().setColumns(5);
    txRangeSpinner.setToolTipText("Transmitting range (m)");
    interferenceRangeSpinner.setToolTipText("Interference range (m)");
    successRatioTxSpinner.setToolTipText("Transmission success ratio (%)");
    successRatioRxSpinner.setToolTipText("Reception success ratio (%)");

    txRangeSpinner.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e) {
        radioMedium.TRANSMITTING_RANGE = ((SpinnerNumberModel)
            txRangeSpinner.getModel()).getNumber().doubleValue();
        visualizer.repaint();
      }
    });

    interferenceRangeSpinner.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e) {
        radioMedium.INTERFERENCE_RANGE = ((SpinnerNumberModel)
            interferenceRangeSpinner.getModel()).getNumber().doubleValue();
        visualizer.repaint();
      }
    });

    successRatioTxSpinner.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e) {
        radioMedium.SUCCESS_RATIO_TX = ((SpinnerNumberModel)
            successRatioTxSpinner.getModel()).getNumber().doubleValue();
        visualizer.repaint();
      }
    });

    successRatioRxSpinner.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e) {
        radioMedium.SUCCESS_RATIO_RX = ((SpinnerNumberModel)
            successRatioRxSpinner.getModel()).getNumber().doubleValue();
        visualizer.repaint();
      }
    });

    txRangeSpinner.setVisible(false);
    interferenceRangeSpinner.setVisible(false);
    successRatioTxSpinner.setVisible(false);
    successRatioRxSpinner.setVisible(false);

    /* Register mouse listener */
    visualizer.getCurrentCanvas().addMouseListener(selectMoteMouseListener);

    /* Register menu actions */
    visualizer.registerSimulationMenuAction(RangeMenuAction.class);
    visualizer.registerSimulationMenuAction(SuccessRatioMenuAction.class);

    /* Add (currently invisible) spinners */
    txCounter = new JLabel("TX: " + radioMedium.COUNTER_TX);
    rxCounter = new JLabel("RX: " + radioMedium.COUNTER_RX);
    interferedCounter = new JLabel("INT: " +  + radioMedium.COUNTER_INTERFERED);
    visualizer.getCurrentCanvas().add(txCounter);
    visualizer.getCurrentCanvas().add(rxCounter);
    visualizer.getCurrentCanvas().add(interferedCounter);

    visualizer.getCurrentCanvas().add(txRangeSpinner);
    visualizer.getCurrentCanvas().add(interferenceRangeSpinner);
    visualizer.getCurrentCanvas().add(successRatioTxSpinner);
    visualizer.getCurrentCanvas().add(successRatioRxSpinner);

    /* Start observing radio medium */
    radioMedium.addRadioMediumObserver(radioMediumObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (txCounter != null) {
          txCounter.setText("TX: " + radioMedium.COUNTER_TX);
        }
        if (rxCounter != null) {
          rxCounter.setText("RX: " + radioMedium.COUNTER_RX);
        }
        if (interferedCounter != null) {
          interferedCounter.setText("INT: " +  + radioMedium.COUNTER_INTERFERED);
        }

        visualizer.repaint();
      }
    });
  }

  public void setInactive() {
    if (simulation == null) {
      /* Skin was never activated */
      return;
    }

    /* Remove mouse listener */
    visualizer.getCurrentCanvas().removeMouseListener(selectMoteMouseListener);

    /* Stop observing radio medium */
    radioMedium.deleteRadioMediumObserver(radioMediumObserver);
    radioMediumObserver = null;
    radioMedium = null;

    visualizer.getCurrentCanvas().remove(txCounter);
    visualizer.getCurrentCanvas().remove(rxCounter);
    visualizer.getCurrentCanvas().remove(interferedCounter);

    /* Remove spinners */
    visualizer.getCurrentCanvas().remove(txRangeSpinner);
    visualizer.getCurrentCanvas().remove(interferenceRangeSpinner);
    visualizer.getCurrentCanvas().remove(successRatioTxSpinner);
    visualizer.getCurrentCanvas().remove(successRatioRxSpinner);

    /* Unregister menu actions */
    visualizer.unregisterSimulationMenuAction(RangeMenuAction.class);
    visualizer.unregisterSimulationMenuAction(SuccessRatioMenuAction.class);
  }

  public Color[] getColorOf(Mote mote) {
    if (simulation == null) {
      /* Skin was never activated */
      return null;
    }

    if (mote.getState() == Mote.State.DEAD) {
      return null;
    }

    Radio moteRadio = mote.getInterfaces().getRadio();
    if (moteRadio == null) {
      return null;
    }

    if (selectedMote != null && mote == selectedMote) {
      return new Color[] { Color.CYAN };
    }

    if (!moteRadio.isReceiverOn()) {
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

    return null;
  }

  private static Color COLOR_INTERFERENCE = new Color(
      Color.DARK_GRAY.getRed(),
      Color.DARK_GRAY.getGreen(),
      Color.DARK_GRAY.getBlue(),
      100
  );
  private static Color COLOR_TX = new Color(
      Color.GREEN.getRed(),
      Color.GREEN.getGreen(),
      Color.GREEN.getBlue(),
      100
  );
  public void paintSkin(Graphics g) {
    if (simulation == null) {
      /* Skin was never activated */
      return;
    }

    /* Paint transmission and interference range for select mote */
    if (selectedMote != null) {
      Graphics2D g2 = (Graphics2D) g;
      Position motePos = selectedMote.getInterfaces().getPosition();

      Point pixelCoord = visualizer.transformPositionToPixel(motePos);
      int x = pixelCoord.x;
      int y = pixelCoord.y;

      // Fetch current output power indicator (scale with as percent)
      if (selectedMote.getInterfaces().getRadio() != null) {
        Radio selectedRadio = selectedMote.getInterfaces().getRadio();
        double moteInterferenceRange =
          radioMedium.INTERFERENCE_RANGE
          * ((double) selectedRadio.getCurrentOutputPowerIndicator()
              / (double) selectedRadio.getOutputPowerIndicatorMax());
        double moteTransmissionRange =
          radioMedium.TRANSMITTING_RANGE
          * ((double) selectedRadio.getCurrentOutputPowerIndicator()
              / (double) selectedRadio.getOutputPowerIndicatorMax());

        Point translatedZero = visualizer.transformPositionToPixel(0.0, 0.0, 0.0);
        Point translatedInterference = visualizer.transformPositionToPixel(moteInterferenceRange, moteInterferenceRange, 0.0);
        Point translatedTransmission = visualizer.transformPositionToPixel(moteTransmissionRange, moteTransmissionRange, 0.0);

        translatedInterference.x = Math.abs(translatedInterference.x - translatedZero.x);
        translatedInterference.y = Math.abs(translatedInterference.y - translatedZero.y);
        translatedTransmission.x = Math.abs(translatedTransmission.x - translatedZero.x);
        translatedTransmission.y = Math.abs(translatedTransmission.y - translatedZero.y);

        /* Interference range */
        g.setColor(COLOR_INTERFERENCE);
        g.fillOval(
            x - translatedInterference.x,
            y - translatedInterference.y,
            2 * translatedInterference.x,
            2 * translatedInterference.y);

        /* Transmission range */
        g.setColor(COLOR_TX);
        g.fillOval(
            x - translatedTransmission.x,
            y - translatedTransmission.y,
            2 * translatedTransmission.x,
            2 * translatedTransmission.y);
      }
    }

    /* Paint active connections in black */
    RadioConnection[] conns = radioMedium.getActiveConnections();
    if (conns != null) {
      g.setColor(Color.BLACK);
      for (RadioConnection conn : conns) {
        Radio source = conn.getSource();
        Point sourcePoint = visualizer.transformPositionToPixel(source.getPosition());
        for (Radio destRadio : conn.getDestinations()) {
          Position destPos = destRadio.getPosition();
          Point destPoint = visualizer.transformPositionToPixel(destPos);
          g.drawLine(sourcePoint.x, sourcePoint.y, destPoint.x, destPoint.y);

          /* Draw arrows */
          if (DRAW_ARROWS) {
            Point centerPoint = new Point(
                destPoint.x/2 + sourcePoint.x/2,
                destPoint.y/2 + sourcePoint.y/2
            );
            int startAngle = (int) (-180 * Math.atan2(destPoint.y - sourcePoint.y, destPoint.x - sourcePoint.x)/Math.PI - 90);
            g.drawArc(centerPoint.x-5, centerPoint.y-5, 10, 10, startAngle, 180);
          }
        }
      }
    }

    /* Paint past connections in gray */
    /*conns = radioMedium.getLastTickConnections();
    if (conns != null) {
      g.setColor(Color.GRAY);
      for (RadioConnection conn : conns) {
        Point sourcePoint = visualizer.transformPositionToPixel(conn.getSource().getPosition());
        for (Radio dest : conn.getDestinations()) {
          Position destPos = dest.getPosition();
          Point destPoint = visualizer.transformPositionToPixel(destPos);
          g.drawLine(sourcePoint.x, sourcePoint.y, destPoint.x, destPoint.y);
        }
      }
    }*/
  }

  public static class RangeMenuAction implements SimulationMenuAction {
    public boolean isEnabled(Simulation simulation) {
      return true;
    }

    public String getDescription(Simulation simulation) {
      return "Change transmission ranges";
    }

    public void doAction(Visualizer visualizer, Simulation simulation) {
      VisualizerSkin[] skins = visualizer.getCurrentSkins();
      for (VisualizerSkin skin: skins) {
        if (skin instanceof UDGMVisualizerSkin) {
          ((UDGMVisualizerSkin)skin).txRangeSpinner.setVisible(true);
          ((UDGMVisualizerSkin)skin).interferenceRangeSpinner.setVisible(true);
          visualizer.repaint();
        }
      }
    }
  };

  public static class SuccessRatioMenuAction implements SimulationMenuAction {
    public boolean isEnabled(Simulation simulation) {
      return true;
    }

    public String getDescription(Simulation simulation) {
      return "Change TX/RX success ratio";
    }

    public void doAction(Visualizer visualizer, Simulation simulation) {
      VisualizerSkin[] skins = visualizer.getCurrentSkins();
      for (VisualizerSkin skin: skins) {
        if (skin instanceof UDGMVisualizerSkin) {
          ((UDGMVisualizerSkin)skin).successRatioTxSpinner.setVisible(true);
          ((UDGMVisualizerSkin)skin).successRatioRxSpinner.setVisible(true);
          visualizer.repaint();
        }
      }
    }
  };

  public Visualizer getVisualizer() {
    return visualizer;
  }
}
