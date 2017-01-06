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
import javax.swing.JCheckBox;
import javax.swing.JFileChooser;
import javax.swing.JButton;
import javax.swing.JTextArea;
import javax.swing.SwingConstants;
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
import org.contikios.cooja.radiomediums.TerrainLOSMedium; 

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import java.io.File;

/**
 * Visualizer skin for configuring the TerrainLOS radio medium.
 *
 * Allows a user to change the collective TX/interference ranges, the TX/RX
 * success ratio, terrain filepath, east/south width, east/south offset, and
 * whether to output a dag.
 * 
 * Also displays the active terrain being used as a gray-scale map, white is
 * the highest elevation and black is the lowest elevation, and the visible 
 * locations from the highlighted mote.
 *
 * To also see radio traffic, this skin can be combined with {@link
 * TrafficVisualizerSkin}.
 *
 * @see TrafficVisualizerSkin
 * @see TerrainLOSMedium
 * @author Sam Mansfield
 * @author Sidharth Gilela
 * @author Fredrik Osterlind
 * @author Enrico Joerns
 */
@ClassDescription("Radio environment (TerrainLOSMedium)")
@SupportedArguments(radioMediums = {TerrainLOSMedium.class})
public class TerrainLOSVisualizerSkin implements VisualizerSkin {

  private static final Logger logger = Logger.getLogger(TerrainLOSVisualizerSkin.class);

  private static final Color COLOR_TX = new Color(0, 255, 0, 100);
  private static final Color COLOR_INT = new Color(50, 50, 50, 100);

  private Simulation simulation = null;
  private Visualizer visualizer = null;
  private TerrainLOSMedium radioMedium = null;

  private JInternalFrame rrFrame;
  private Box ratioRX, ratioTX, rangeTX, rangeINT, eastWidth, southWidth, eastOffset, southOffset, 
          outputDAG, terrainFP;
 
  /* Used to change the terrain filepath */
  private String filepath = "";

  @Override
  public void setActive(Simulation simulation, Visualizer vis) {
    if (!(simulation.getRadioMedium() instanceof TerrainLOSMedium)) {
      logger.fatal("Cannot activate TerrainLOSMedium skin for unknown radio medium: " + 
          simulation.getRadioMedium());
      return;
    }
    this.simulation = simulation;
    this.visualizer = vis;
    this.radioMedium = (TerrainLOSMedium) simulation.getRadioMedium();

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

    SpinnerNumberModel eastWidthModel = new SpinnerNumberModel();
    eastWidthModel.setValue(new Double(radioMedium.map.EAST_WIDTH));
    eastWidthModel.setStepSize(new Double(1.0)); // 1deg
    eastWidthModel.setMinimum(new Double(0.0));
    eastWidthModel.setMaximum(new Double(3600.0));

    SpinnerNumberModel southWidthModel = new SpinnerNumberModel();
    southWidthModel.setValue(new Double(radioMedium.map.SOUTH_WIDTH));
    southWidthModel.setStepSize(new Double(1.0)); // 1deg
    southWidthModel.setMinimum(new Double(0.0));
    southWidthModel.setMaximum(new Double(3600.0));

    SpinnerNumberModel eastOffsetModel = new SpinnerNumberModel();
    eastOffsetModel.setValue(new Double(radioMedium.map.EAST_OFFSET));
    eastOffsetModel.setStepSize(new Double(1.0)); // 1deg
    eastOffsetModel.setMinimum(new Double(0.0));
    eastOffsetModel.setMaximum(new Double(3600.0));

    SpinnerNumberModel southOffsetModel = new SpinnerNumberModel();
    southOffsetModel.setValue(new Double(radioMedium.map.SOUTH_OFFSET));
    southOffsetModel.setStepSize(new Double(1.0)); // 1deg
    southOffsetModel.setMinimum(new Double(0.0));
    southOffsetModel.setMaximum(new Double(3600.0));

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
    
    final JSpinner eastWidthSpinner = new JSpinner(eastWidthModel);
    editor = new JSpinner.NumberEditor(eastWidthSpinner, "0deg");
    eastWidthSpinner.setEditor(editor);
    
    final JSpinner southWidthSpinner = new JSpinner(southWidthModel);
    editor = new JSpinner.NumberEditor(southWidthSpinner, "0deg");
    southWidthSpinner.setEditor(editor);
    
    final JSpinner eastOffsetSpinner = new JSpinner(eastOffsetModel);
    editor = new JSpinner.NumberEditor(eastOffsetSpinner, "0deg");
    eastOffsetSpinner.setEditor(editor);
    
    final JSpinner southOffsetSpinner = new JSpinner(southOffsetModel);
    editor = new JSpinner.NumberEditor(southOffsetSpinner, "0deg");
    southOffsetSpinner.setEditor(editor);

    final JCheckBox outputDagVar = new JCheckBox("true");
		
    JButton terrainFPButton = new JButton("Select File");
    terrainFPButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ae) {
        JFileChooser fileChooser = new JFileChooser();
        int returnValue = fileChooser.showOpenDialog(null);
        if (returnValue == JFileChooser.APPROVE_OPTION) {
          File selectedFile = fileChooser.getSelectedFile();
          filepath = selectedFile.getAbsolutePath();
          radioMedium.setTerrainFilepath(filepath);
        }
      }
    });

    ((JSpinner.DefaultEditor) interferenceRangeSpinner.getEditor()).getTextField().setColumns(5);
    ((JSpinner.DefaultEditor) successRatioTxSpinner.getEditor()).getTextField().setColumns(5);
    ((JSpinner.DefaultEditor) successRatioRxSpinner.getEditor()).getTextField().setColumns(5);
    ((JSpinner.DefaultEditor) eastWidthSpinner.getEditor()).getTextField().setColumns(5);
    ((JSpinner.DefaultEditor) southWidthSpinner.getEditor()).getTextField().setColumns(5);	
    ((JSpinner.DefaultEditor) eastOffsetSpinner.getEditor()).getTextField().setColumns(5);
    ((JSpinner.DefaultEditor) southOffsetSpinner.getEditor()).getTextField().setColumns(5);

    txRangeSpinner.setToolTipText("Transmitting range (m)");
    interferenceRangeSpinner.setToolTipText("Interference range (m)");
    successRatioTxSpinner.setToolTipText("Transmission success ratio (%)");
    successRatioRxSpinner.setToolTipText("Reception success ratio (%)");
    eastWidthSpinner.setToolTipText("East width (deg)");
    southWidthSpinner.setToolTipText("South width (deg)");
    eastOffsetSpinner.setToolTipText("East offset (deg)");
    southOffsetSpinner.setToolTipText("South offset (deg)");
    outputDagVar.setToolTipText("Output DAG (t/f)");

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

    eastWidthSpinner.addChangeListener(new ChangeListener() {
      @Override
      public void stateChanged(ChangeEvent e) {
        radioMedium.setEastWidth(((SpinnerNumberModel) eastWidthSpinner.getModel())
                .getNumber().intValue());
        visualizer.repaint();
      }
    });

    southWidthSpinner.addChangeListener(new ChangeListener() {
      @Override
      public void stateChanged(ChangeEvent e) {
        radioMedium.setSouthWidth(((SpinnerNumberModel) southWidthSpinner.getModel())
                .getNumber().intValue());
        visualizer.repaint();
      }
    });

    eastOffsetSpinner.addChangeListener(new ChangeListener() {
      @Override
      public void stateChanged(ChangeEvent e) {
        radioMedium.setEastOffset(((SpinnerNumberModel) eastOffsetSpinner.getModel())
                .getNumber().intValue());
        visualizer.repaint();
      }
    });
	
    southOffsetSpinner.addChangeListener(new ChangeListener() {
      @Override
      public void stateChanged(ChangeEvent e) {
        radioMedium.setSouthOffset(((SpinnerNumberModel) southOffsetSpinner.getModel())
                .getNumber().intValue());
        visualizer.repaint();
      }
    });

    outputDagVar.addItemListener(new ItemListener() {
      public void itemStateChanged(ItemEvent e) {         
        if (e.getStateChange()==1){
          radioMedium.setOutputDAG(true);
        }	
      }           
    });

    /* Register menu actions */
    visualizer.registerSimulationMenuAction(RangeMenuAction.class);

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
    eastWidth = Box.createHorizontalBox();
    eastWidth.add(new JLabel("East width:"));
    eastWidth.add(Box.createHorizontalStrut(5));
    eastWidth.add(eastWidthSpinner);
    southWidth = Box.createHorizontalBox();
    southWidth.add(new JLabel("South width:"));
    southWidth.add(Box.createHorizontalStrut(5));
    southWidth.add(southWidthSpinner);
    eastOffset = Box.createHorizontalBox();
    eastOffset.add(new JLabel("East offset:"));
    eastOffset.add(Box.createHorizontalStrut(5));
    eastOffset.add(eastOffsetSpinner);
    southOffset = Box.createHorizontalBox();
    southOffset.add(new JLabel("South offset:"));
    southOffset.add(Box.createHorizontalStrut(5));
    southOffset.add(southOffsetSpinner);
    outputDAG = Box.createHorizontalBox();
    outputDAG.add(new JLabel("Output DAG:"));
    outputDAG.add(Box.createHorizontalStrut(5));
    outputDAG.add(outputDagVar);
    terrainFP = Box.createHorizontalBox();
    terrainFP.add(new JLabel("Terrain filepath:"));
    terrainFP.add(Box.createHorizontalStrut(5));
    terrainFP.add(terrainFPButton);
		
    rangeTX.setVisible(false);
    rangeINT.setVisible(false);
    ratioTX.setVisible(false);
    ratioRX.setVisible(false);
    eastWidth.setVisible(false);
    southWidth.setVisible(false);
    eastOffset.setVisible(false);
    southOffset.setVisible(false);
    outputDAG.setVisible(false);
    terrainFP.setVisible(false);

    main.add(rangeTX);
    main.add(rangeINT);
    main.add(ratioTX);
    main.add(ratioRX);
    main.add(eastWidth);
    main.add(southWidth);
    main.add(eastOffset);
    main.add(southOffset);
    main.add(outputDAG);
    main.add(terrainFP);

    rrFrame = new JInternalFrame("TerrainLOSMedium", false, true);
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
        eastWidth.setVisible(false);
        southWidth.setVisible(false);
        eastOffset.setVisible(false);
        southOffset.setVisible(false);
        outputDAG.setVisible(false);
        terrainFP.setVisible(false);
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
  }

  @Override
  public Color[] getColorOf(Mote mote) {
    return null;
  }
  
  private Color heightToColor(double height) {
    double colorVal = 255.0*((height - this.radioMedium.map.min)/
        (this.radioMedium.map.max - this.radioMedium.map.min));
    
    if(colorVal > 255.0)
      colorVal = 255.0;
    if(colorVal < 0) 
      colorVal = 0;
    
    return new Color((int) colorVal, (int) colorVal, (int) colorVal, 100);
  }

  private double distance(double srcX, double srcY, double destX, double destY) {
    return Math.hypot(srcX - destX, srcY - destY);
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

      /* Fetch current output power indicator (scale with as percent) */
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
              = visualizer.transformPositionToPixel(moteInterferenceRange, 
                  moteInterferenceRange, 0.0);
      Point translatedTransmission
              = visualizer.transformPositionToPixel(moteTransmissionRange, 
                  moteTransmissionRange, 0.0);
      Point translatedInterferenceMax
              = visualizer.transformPositionToPixel(radioMedium.INTERFERENCE_RANGE, 
                  radioMedium.INTERFERENCE_RANGE, 0.0);
      Point translatedTransmissionMax
              = visualizer.transformPositionToPixel(radioMedium.TRANSMITTING_RANGE, 
                  radioMedium.TRANSMITTING_RANGE, 0.0);
      Point eastSouthPoint
              = visualizer.transformPositionToPixel(radioMedium.map.EAST_WIDTH*33, 
                  radioMedium.map.SOUTH_WIDTH*33, 0.0);
      Point degUnit = visualizer.transformPositionToPixel(33.0, 33.0, 0.0);

      translatedInterference.x = Math.abs(translatedInterference.x - translatedZero.x);
      translatedInterference.y = Math.abs(translatedInterference.y - translatedZero.y);
      translatedTransmission.x = Math.abs(translatedTransmission.x - translatedZero.x);
      translatedTransmission.y = Math.abs(translatedTransmission.y - translatedZero.y);
      translatedInterferenceMax.x = Math.abs(translatedInterferenceMax.x - translatedZero.x);
      translatedInterferenceMax.y = Math.abs(translatedInterferenceMax.y - translatedZero.y);
      translatedTransmissionMax.x = Math.abs(translatedTransmissionMax.x - translatedZero.x);
      translatedTransmissionMax.y = Math.abs(translatedTransmissionMax.y - translatedZero.y);
      eastSouthPoint.x = Math.abs(eastSouthPoint.x - translatedZero.x);
      eastSouthPoint.y = Math.abs(eastSouthPoint.y - translatedZero.y);
      degUnit.x = Math.abs(degUnit.x - translatedZero.x);
      degUnit.y = Math.abs(degUnit.y - translatedZero.y);

      /* Draw border around terrain */
      g.drawRect(translatedZero.x, translatedZero.y, eastSouthPoint.x, eastSouthPoint.y);

      /* Draw the gray-scale terrain */
      for(int i = 0; i < this.radioMedium.map.EAST_WIDTH; i++) {
        for(int j = 0; j < this.radioMedium.map.SOUTH_WIDTH; j++) {
          /* The gray-scale map is relaitve to the max and min height of the terrain.
           * The maximum height will be white, the minimum height will be black.
           */
          g.setColor(heightToColor(this.radioMedium.map.h[i][j]));
          
          Point coordinate = visualizer.transformPositionToPixel(i*33, j*33, 0.0);
          /* Each index in map.h corresponds to 33m */
          g.drawRect(coordinate.x, coordinate.y, degUnit.x, degUnit.y);
          g.fillRect(coordinate.x, coordinate.y, degUnit.x, degUnit.y);
        }
      }	
      
      double moteX = motePos.getXCoordinate();
      double moteY = motePos.getYCoordinate();

      /* Draw the places visible by the selected mote */
      if(moteX > 0 &&  moteX < 34*this.radioMedium.map.EAST_WIDTH &&
          moteY > 0 &&  moteY < 34*this.radioMedium.map.SOUTH_WIDTH) {
        
        this.radioMedium.map.calculateLOS(this.radioMedium.map.convertLocToCord((int) moteX), 
                         this.radioMedium.map.convertLocToCord((int) moteY));

        for(int i = 0; i < this.radioMedium.map.EAST_WIDTH; i++) {
          for(int j = 0; j < this.radioMedium.map.SOUTH_WIDTH; j++) {
            if(distance(moteX, moteY, i*33, j*33) < Math.max(radioMedium.TRANSMITTING_RANGE,
                radioMedium.INTERFERENCE_RANGE)) {
              Point coordinate = 
                visualizer.transformPositionToPixel(i*33, j*33, 0.0);
           
              /* isThereLOS is based on meters, so we must multiply
               * the cord (i, j) by 33 to convert from degrees. 
               */
              if(this.radioMedium.map.isThereLOS(i, j)) {
                g.setColor(new Color(255, 220, 100, 80));
                /* Each index in map.h corresponds to 33m */
                g.drawRect(coordinate.x, coordinate.y, degUnit.x, degUnit.y);
                g.fillRect(coordinate.x, coordinate.y, degUnit.x, degUnit.y);
              }
            }
          }
        }
      } 

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
                = ((TerrainLOSMedium) simulation.getRadioMedium()).getSuccessProbability(selectedRadio, m.getInterfaces().getRadio());
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
      return "Change components";
    }

    @Override
    public void doAction(Visualizer visualizer, Simulation simulation) {
      VisualizerSkin[] skins = visualizer.getCurrentSkins();
      for (VisualizerSkin skin : skins) {
        if (skin instanceof TerrainLOSVisualizerSkin) {
          TerrainLOSVisualizerSkin vskin = ((TerrainLOSVisualizerSkin) skin);
          vskin.rangeTX.setVisible(true);
          vskin.rangeINT.setVisible(true);
          vskin.ratioTX.setVisible(true);
          vskin.ratioRX.setVisible(true);	
          vskin.eastWidth.setVisible(true);
          vskin.southWidth.setVisible(true);
          vskin.eastOffset.setVisible(true);
          vskin.southOffset.setVisible(true);
          vskin.outputDAG.setVisible(true);
          vskin.terrainFP.setVisible(true);	
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
