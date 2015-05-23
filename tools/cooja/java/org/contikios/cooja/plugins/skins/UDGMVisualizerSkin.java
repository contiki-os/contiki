/*
 * Copyright (c) 2009-2013, Swedish Institute of Computer Science, TU Braunscheig
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
package org.contikios.cooja.plugins.skins;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.geom.Area;
import java.awt.geom.Ellipse2D;
import java.beans.PropertyVetoException;
import java.util.Set;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JInternalFrame;
import javax.swing.JLabel;
import javax.swing.JLayeredPane;
import javax.swing.JPanel;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.SwingUtilities;
import static javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.InternalFrameAdapter;
import javax.swing.event.InternalFrameEvent;

import org.apache.log4j.Logger;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.SupportedArguments;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.interfaces.Radio;
import org.contikios.cooja.plugins.Visualizer;
import org.contikios.cooja.plugins.Visualizer.SimulationMenuAction;
import org.contikios.cooja.plugins.VisualizerSkin;
import org.contikios.cooja.radiomediums.UDGM;

/**
 * Visualizer skin for configuring the Unit Disk Graph radio medium (UDGM).
 *
 * Allows a user to change the collective TX/interference ranges, and the TX/RX
 * success ratio.
 *
 * To also see radio traffic, this skin can be combined with {@link
 * TrafficVisualizerSkin}.
 *
 * @see TrafficVisualizerSkin
 * @see UDGM
 * @author Fredrik Osterlind
 * @author Enrico Joerns
 */
@ClassDescription("Radio environment (UDGM)")
@SupportedArguments(radioMediums = {UDGM.class})
public class UDGMVisualizerSkin implements VisualizerSkin {

  private static final Logger logger = Logger.getLogger(UDGMVisualizerSkin.class);

  private static final Color COLOR_TX = new Color(0, 255, 0, 100);
  private static final Color COLOR_INT = new Color(50, 50, 50, 100);

  private Simulation simulation = null;
  private Visualizer visualizer = null;
  private UDGM radioMedium = null;

  private JInternalFrame rrFrame;
  private Box ratioRX, ratioTX, rangeTX, rangeINT;

  @Override
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
    final JSpinner txRangeSpinner = new JSpinner(transmissionModel);
    editor = new JSpinner.NumberEditor(txRangeSpinner, "0m");
    txRangeSpinner.setEditor(editor);
    final JSpinner interferenceRangeSpinner = new JSpinner(interferenceModel);
    editor = new JSpinner.NumberEditor(interferenceRangeSpinner, "0m");
    interferenceRangeSpinner.setEditor(editor);
    final JSpinner successRatioTxSpinner = new JSpinner(successRatioTxModel);
    editor = new JSpinner.NumberEditor(successRatioTxSpinner, "0.0%");
    successRatioTxSpinner.setEditor(editor);
    final JSpinner successRatioRxSpinner = new JSpinner(successRatioRxModel);
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
      @Override
      public void stateChanged(ChangeEvent e) {
        radioMedium.setTxRange(((SpinnerNumberModel) txRangeSpinner.getModel())
                .getNumber().doubleValue());
        visualizer.repaint();
      }
    });

    interferenceRangeSpinner.addChangeListener(new ChangeListener() {
      @Override
      public void stateChanged(ChangeEvent e) {
        radioMedium.setInterferenceRange(((SpinnerNumberModel) interferenceRangeSpinner.getModel())
                .getNumber().doubleValue());
        visualizer.repaint();
      }
    });

    successRatioTxSpinner.addChangeListener(new ChangeListener() {
      @Override
      public void stateChanged(ChangeEvent e) {
        radioMedium.SUCCESS_RATIO_TX = ((SpinnerNumberModel) successRatioTxSpinner.getModel())
                .getNumber().doubleValue();
        visualizer.repaint();
      }
    });

    successRatioRxSpinner.addChangeListener(new ChangeListener() {
      @Override
      public void stateChanged(ChangeEvent e) {
        radioMedium.SUCCESS_RATIO_RX = ((SpinnerNumberModel) successRatioRxSpinner.getModel())
                .getNumber().doubleValue();
        visualizer.repaint();
      }
    });

    /* Register menu actions */
    visualizer.registerSimulationMenuAction(RangeMenuAction.class);
    visualizer.registerSimulationMenuAction(SuccessRatioMenuAction.class);

    /* UI components */
    JPanel main = new JPanel();
    main.setLayout(new BoxLayout(main, BoxLayout.Y_AXIS));
    main.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));

    rangeTX = Box.createHorizontalBox();
    rangeTX.add(new JLabel("TX range:"));
    rangeTX.add(Box.createHorizontalStrut(5));
    rangeTX.add(txRangeSpinner);
    rangeINT = Box.createHorizontalBox();
    rangeINT.add(new JLabel("INT range:"));
    rangeINT.add(Box.createHorizontalStrut(5));
    rangeINT.add(interferenceRangeSpinner);
    ratioTX = Box.createHorizontalBox();
    ratioTX.add(new JLabel("TX ratio:"));
    ratioTX.add(Box.createHorizontalStrut(5));
    ratioTX.add(successRatioTxSpinner);
    ratioRX = Box.createHorizontalBox();
    ratioRX.add(new JLabel("RX ratio:"));
    ratioRX.add(Box.createHorizontalStrut(5));
    ratioRX.add(successRatioRxSpinner);

    rangeTX.setVisible(false);
    rangeINT.setVisible(false);
    ratioTX.setVisible(false);
    ratioRX.setVisible(false);

    main.add(rangeTX);
    main.add(rangeINT);
    main.add(ratioTX);
    main.add(ratioRX);

    rrFrame = new JInternalFrame("UDGM", false, true);
    rrFrame.setVisible(false);
    rrFrame.setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
    rrFrame.addInternalFrameListener(new InternalFrameAdapter() {
      @Override
      public void internalFrameClosing(InternalFrameEvent ife) {
        super.internalFrameClosed(ife);
        rangeTX.setVisible(false);
        rangeINT.setVisible(false);
        ratioTX.setVisible(false);
        ratioRX.setVisible(false);
        rrFrame.setVisible(false);
      }
    });

    rrFrame.getContentPane().add(BorderLayout.CENTER, main);
    rrFrame.pack();
  }

  @Override
  public void setInactive() {
    if (simulation == null) {
      /* Skin was never activated */
      return;
    }

    /* Remove spinners etc */
    visualizer.getCurrentCanvas().remove(rrFrame);

    /* Unregister menu actions */
    visualizer.unregisterSimulationMenuAction(RangeMenuAction.class);
    visualizer.unregisterSimulationMenuAction(SuccessRatioMenuAction.class);
  }

  @Override
  public Color[] getColorOf(Mote mote) {
    return null;
  }

  @Override
  public void paintBeforeMotes(Graphics g) {
    Set<Mote> selectedMotes = visualizer.getSelectedMotes();
    if (simulation == null || selectedMotes == null) {
      return;
    }

    Area intRangeArea = new Area();
    Area intRangeMaxArea = new Area();
    Area trxRangeArea = new Area();
    Area trxRangeMaxArea = new Area();

    for (Mote selectedMote : selectedMotes) {
      if (selectedMote.getInterfaces().getRadio() == null) {
        continue;
      }

      /* Paint transmission and interference range for selected mote */
      Position motePos = selectedMote.getInterfaces().getPosition();

      Point pixelCoord = visualizer.transformPositionToPixel(motePos);
      int x = pixelCoord.x;
      int y = pixelCoord.y;

      // Fetch current output power indicator (scale with as percent)
      Radio selectedRadio = selectedMote.getInterfaces().getRadio();
      double moteInterferenceRange
              = radioMedium.INTERFERENCE_RANGE
              * ((double) selectedRadio.getCurrentOutputPowerIndicator()
              / (double) selectedRadio.getOutputPowerIndicatorMax());
      double moteTransmissionRange
              = radioMedium.TRANSMITTING_RANGE
              * ((double) selectedRadio.getCurrentOutputPowerIndicator()
              / (double) selectedRadio.getOutputPowerIndicatorMax());

      Point translatedZero = visualizer.transformPositionToPixel(0.0, 0.0, 0.0);
      Point translatedInterference
              = visualizer.transformPositionToPixel(moteInterferenceRange, moteInterferenceRange, 0.0);
      Point translatedTransmission
              = visualizer.transformPositionToPixel(moteTransmissionRange, moteTransmissionRange, 0.0);
      Point translatedInterferenceMax
              = visualizer.transformPositionToPixel(radioMedium.INTERFERENCE_RANGE, radioMedium.INTERFERENCE_RANGE, 0.0);
      Point translatedTransmissionMax
              = visualizer.transformPositionToPixel(radioMedium.TRANSMITTING_RANGE, radioMedium.TRANSMITTING_RANGE, 0.0);

      translatedInterference.x = Math.abs(translatedInterference.x - translatedZero.x);
      translatedInterference.y = Math.abs(translatedInterference.y - translatedZero.y);
      translatedTransmission.x = Math.abs(translatedTransmission.x - translatedZero.x);
      translatedTransmission.y = Math.abs(translatedTransmission.y - translatedZero.y);
      translatedInterferenceMax.x = Math.abs(translatedInterferenceMax.x - translatedZero.x);
      translatedInterferenceMax.y = Math.abs(translatedInterferenceMax.y - translatedZero.y);
      translatedTransmissionMax.x = Math.abs(translatedTransmissionMax.x - translatedZero.x);
      translatedTransmissionMax.y = Math.abs(translatedTransmissionMax.y - translatedZero.y);

      /* Interference range */
      intRangeArea.add(new Area(new Ellipse2D.Double(
              x - translatedInterference.x,
              y - translatedInterference.y,
              2 * translatedInterference.x,
              2 * translatedInterference.y)));

      /* Interference range (MAX) */
      trxRangeArea.add(new Area(new Ellipse2D.Double(
              x - translatedTransmission.x,
              y - translatedTransmission.y,
              2 * translatedTransmission.x,
              2 * translatedTransmission.y)));

      intRangeMaxArea.add(new Area(new Ellipse2D.Double(
              x - translatedInterferenceMax.x,
              y - translatedInterferenceMax.y,
              2 * translatedInterferenceMax.x,
              2 * translatedInterferenceMax.y)));

      /* Transmission range (MAX) */
      trxRangeMaxArea.add(new Area(new Ellipse2D.Double(
              x - translatedTransmissionMax.x,
              y - translatedTransmissionMax.y,
              2 * translatedTransmissionMax.x,
              2 * translatedTransmissionMax.y)));

    }

    Graphics2D g2d = (Graphics2D) g;

    g2d.setColor(COLOR_INT);
    g2d.fill(intRangeArea);
    g.setColor(Color.GRAY);
    g2d.draw(intRangeMaxArea);

    g.setColor(COLOR_TX);
    g2d.fill(trxRangeArea);
    g.setColor(Color.GRAY);
    g2d.draw(trxRangeMaxArea);

    FontMetrics fm = g.getFontMetrics();
    g.setColor(Color.BLACK);

    /* Print transmission success probabilities only if single mote is selected */
    if (selectedMotes.size() == 1) {
      Mote selectedMote = selectedMotes.toArray(new Mote[0])[0];
      Radio selectedRadio = selectedMote.getInterfaces().getRadio();
      for (Mote m : simulation.getMotes()) {
        if (m == selectedMote) {
          continue;
        }
        double prob
                = ((UDGM) simulation.getRadioMedium()).getSuccessProbability(selectedRadio, m.getInterfaces().getRadio());
        if (prob == 0.0d) {
          continue;
        }
        String msg = (((int) (1000 * prob)) / 10.0) + "%";
        Position pos = m.getInterfaces().getPosition();
        Point pixel = visualizer.transformPositionToPixel(pos);
        int msgWidth = fm.stringWidth(msg);
        g.drawString(msg, pixel.x - msgWidth / 2, pixel.y + 2 * Visualizer.MOTE_RADIUS + 3);
      }
    }

  }

  @Override
  public void paintAfterMotes(Graphics g) {
  }

  public static class RangeMenuAction implements SimulationMenuAction {

    @Override
    public boolean isEnabled(Visualizer visualizer, Simulation simulation) {
      return true;
    }

    @Override
    public String getDescription(Visualizer visualizer, Simulation simulation) {
      return "Change transmission ranges";
    }

    @Override
    public void doAction(Visualizer visualizer, Simulation simulation) {
      VisualizerSkin[] skins = visualizer.getCurrentSkins();
      for (VisualizerSkin skin : skins) {
        if (skin instanceof UDGMVisualizerSkin) {
          UDGMVisualizerSkin vskin = ((UDGMVisualizerSkin) skin);
          vskin.rangeTX.setVisible(true);
          vskin.rangeINT.setVisible(true);
          vskin.updateRatioRangeFrame();
        }
      }
    }
  };

  public static class SuccessRatioMenuAction implements SimulationMenuAction {

    @Override
    public boolean isEnabled(Visualizer visualizer, Simulation simulation) {
      return true;
    }

    @Override
    public String getDescription(Visualizer visualizer, Simulation simulation) {
      return "Change TX/RX success ratio";
    }

    @Override
    public void doAction(Visualizer visualizer, Simulation simulation) {
      VisualizerSkin[] skins = visualizer.getCurrentSkins();
      for (VisualizerSkin skin : skins) {
        if (skin instanceof UDGMVisualizerSkin) {
          UDGMVisualizerSkin vskin = ((UDGMVisualizerSkin) skin);
          vskin.ratioTX.setVisible(true);
          vskin.ratioRX.setVisible(true);
          vskin.updateRatioRangeFrame();
        }
      }
    }
  };

  private void updateRatioRangeFrame() {
    if (rrFrame.getDesktopPane() == null) {
      visualizer.getDesktopPane().add(rrFrame);
    }
    rrFrame.pack();
    /* Place frame at the upper right corner of the visualizer canvas */
    Point visCanvasPos = SwingUtilities.convertPoint(
            visualizer.getCurrentCanvas(),
            visualizer.getCurrentCanvas().getLocation(),
            visualizer.getDesktopPane());
    rrFrame.setLocation(
            visCanvasPos.x + visualizer.getCurrentCanvas().getWidth() - rrFrame.getWidth(),
            visCanvasPos.y);
    /* Try to place on top with focus */
    rrFrame.setLayer(JLayeredPane.MODAL_LAYER);
    rrFrame.setVisible(true);
    rrFrame.moveToFront();
    try {
      rrFrame.setSelected(true);
    }
    catch (PropertyVetoException ex) {
      logger.warn("Failed getting focus");
    }
  }

  @Override
  public Visualizer getVisualizer() {
    return visualizer;
  }
}
