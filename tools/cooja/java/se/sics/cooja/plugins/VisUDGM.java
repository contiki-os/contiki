package se.sics.cooja.plugins;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.Observable;
import java.util.Observer;
import java.util.Vector;

import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.PluginType;
import se.sics.cooja.RadioConnection;
import se.sics.cooja.Simulation;
import se.sics.cooja.contikimote.interfaces.ContikiRadio;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.plugins.Visualizer2D;
import se.sics.cooja.radiomediums.UDGM;

/**
 * Visualizes radio traffic in the UDGM radio medium.
 * Allows a user to change the collective TX/interference ranges,
 * and the TX/RX success ratio.
 *
 * Sending motes are blue, receiving motes are green and motes that hear noise
 * are painted red. Motes without radios are painted gray, and the rest are
 * white.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("UDGM Visualizer")
@PluginType(PluginType.SIM_PLUGIN)
public class VisUDGM extends Visualizer2D {
  private JSpinner txRangeSpinner = null;
  private JSpinner interferenceRangeSpinner = null;
  private JSpinner successRatioTxSpinner = null;
  private JSpinner successRatioRxSpinner = null;

  private UDGM radioMedium = null;
  private Observer radioMediumObserver;

  private Mote selectedMote = null;

  private class RangeMenuAction implements MoteMenuAction {
    public boolean isEnabled(Mote mote) {
      return true;
    }

    public String getDescription(Mote mote) {
      return "Change transmission ranges";
    }

    public void doAction(Mote mote) {
      txRangeSpinner.setVisible(true);
      interferenceRangeSpinner.setVisible(true);
      repaint();
    }
  };

  private class SuccessRatioMenuAction implements MoteMenuAction {
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

    radioMedium = (UDGM) sim.getRadioMedium();

    // Create spinners for changing ranges
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
        repaint();
      }
    });

    interferenceRangeSpinner.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e) {
        radioMedium.INTERFERENCE_RANGE = ((SpinnerNumberModel)
            interferenceRangeSpinner.getModel()).getNumber().doubleValue();
        repaint();
      }
    });

    successRatioTxSpinner.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e) {
        radioMedium.SUCCESS_RATIO_TX = ((SpinnerNumberModel)
            successRatioTxSpinner.getModel()).getNumber().doubleValue();
        repaint();
      }
    });

    successRatioRxSpinner.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e) {
        radioMedium.SUCCESS_RATIO_RX = ((SpinnerNumberModel)
            successRatioRxSpinner.getModel()).getNumber().doubleValue();
        repaint();
      }
    });

    getCurrentCanvas().add(txRangeSpinner);
    getCurrentCanvas().add(interferenceRangeSpinner);
    getCurrentCanvas().add(successRatioTxSpinner);
    getCurrentCanvas().add(successRatioRxSpinner);

    txRangeSpinner.setVisible(false);
    interferenceRangeSpinner.setVisible(false);
    successRatioTxSpinner.setVisible(false);
    successRatioRxSpinner.setVisible(false);

    /* Enable user to select mote by mouse click */
    getCurrentCanvas().addMouseListener(new MouseListener() {
      public void mouseExited(MouseEvent e) { }
      public void mouseEntered(MouseEvent e) { }
      public void mouseReleased(MouseEvent e) { }
      public void mouseClicked(MouseEvent e) { }

      public void mousePressed(MouseEvent e) {
        Vector<Mote> clickedMotes = findMotesAtPosition(e.getX(), e.getY());
        if (clickedMotes == null || clickedMotes.size() == 0) {
          selectedMote = null;
          txRangeSpinner.setVisible(false);
          interferenceRangeSpinner.setVisible(false);
          successRatioTxSpinner.setVisible(false);
          successRatioRxSpinner.setVisible(false);
          repaint();
          return;
        }

        /* Several motes may have been clicked: select another one */
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
    });

    // Register change ranges and change success ratio action
    addMoteMenuAction(new RangeMenuAction());
    addMoteMenuAction(new SuccessRatioMenuAction());

    // Observe radio medium
    radioMedium.addRadioMediumObserver(radioMediumObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        getCurrentCanvas().repaint();
      }
    });
  }

  public void closePlugin() {
    super.closePlugin();
    radioMedium.deleteRadioMediumObserver(radioMediumObserver);
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

    /* Paint transmission and interference range for select mote */
    if (selectedMote != null) {
      Position motePos = selectedMote.getInterfaces().getPosition();

      Point pixelCoord = transformPositionToPixel(motePos);
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

        Point translatedZero = transformPositionToPixel(0.0, 0.0, 0.0);
        Point translatedInterference = transformPositionToPixel(moteInterferenceRange, moteInterferenceRange, 0.0);
        Point translatedTransmission = transformPositionToPixel(moteTransmissionRange, moteTransmissionRange, 0.0);

        translatedInterference.x = Math.abs(translatedInterference.x - translatedZero.x);
        translatedInterference.y = Math.abs(translatedInterference.y - translatedZero.y);
        translatedTransmission.x = Math.abs(translatedTransmission.x - translatedZero.x);
        translatedTransmission.y = Math.abs(translatedTransmission.y - translatedZero.y);

        // Interference
        g.setColor(Color.DARK_GRAY);
        g.fillOval(
            x - translatedInterference.x,
            y - translatedInterference.y,
            2 * translatedInterference.x,
            2 * translatedInterference.y);

        // Transmission
        g.setColor(Color.GREEN);
        g.fillOval(
            x - translatedTransmission.x,
            y - translatedTransmission.y,
            2 * translatedTransmission.x,
            2 * translatedTransmission.y);
      }
    }

    // Let parent paint motes
    super.visualizeSimulation(g);

    /* Paint active connections in black */
    RadioConnection[] conns = radioMedium.getActiveConnections();
    if (conns != null) {
      g.setColor(Color.BLACK);
      for (RadioConnection conn : conns) {
        Point sourcePoint = transformPositionToPixel(conn.getSource().getPosition());
        for (Radio destRadio : conn.getDestinations()) {
          Position destPos = destRadio.getPosition();
          Point destPoint = transformPositionToPixel(destPos);
          g.drawLine(sourcePoint.x, sourcePoint.y, destPoint.x, destPoint.y);
        }
      }
    }

    /* Paint past connections in gray */
    conns = radioMedium.getLastTickConnections();
    if (conns != null) {
      g.setColor(Color.GRAY);
      for (RadioConnection conn : conns) {
        Point sourcePoint = transformPositionToPixel(conn.getSource().getPosition());
        for (Radio dest : conn.getDestinations()) {
          Position destPos = dest.getPosition();
          Point destPoint = transformPositionToPixel(destPos);
          g.drawLine(sourcePoint.x, sourcePoint.y, destPoint.x, destPoint.y);
        }
      }
    }

  }
}
