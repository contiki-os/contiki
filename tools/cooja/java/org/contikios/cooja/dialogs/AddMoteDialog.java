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
 */

package org.contikios.cooja.dialogs;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dialog;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.text.NumberFormat;
import java.text.ParseException;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;

import org.apache.log4j.Logger;

import org.contikios.cooja.Cooja;
import org.contikios.cooja.Mote;
import org.contikios.cooja.MoteType;
import org.contikios.cooja.Positioner;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.interfaces.MoteID;
import org.contikios.cooja.interfaces.Position;

/**
 * A dialog for adding motes.
 *
 * @author Fredrik Osterlind
 */
public class AddMoteDialog extends JDialog {

  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(AddMoteDialog.class);

  private AddMotesEventHandler myEventHandler = new AddMotesEventHandler();

  private final static int LABEL_WIDTH = 170;
  private final static int LABEL_HEIGHT = 15;

  private Vector<Mote> newMotes = null;

  private JButton addButton;

  private MoteType moteType = null;
  private Simulation simulation = null;

  private JFormattedTextField numberOfMotesField, startX, endX, startY, endY,
      startZ, endZ;
  private JComboBox positionDistributionBox;


  /**
   * Shows a dialog which enables a user to create and add motes of the given
   * type.
   *
   * @param parentContainer
   *          Parent container for dialog
   * @param simulation
   *          Simulation
   * @param moteType
   *          Mote type
   * @return New motes or null if aborted
   */
  public static Vector<Mote> showDialog(Container parentContainer,
      Simulation simulation, MoteType moteType) {

    AddMoteDialog myDialog = null;
    if (parentContainer instanceof Window) {
      myDialog = new AddMoteDialog((Window)parentContainer, simulation, moteType);
    } else if (parentContainer instanceof Dialog) {
      myDialog = new AddMoteDialog((Dialog)parentContainer, simulation, moteType);
    } else if (parentContainer instanceof Frame) {
      myDialog = new AddMoteDialog((Frame)parentContainer, simulation, moteType);
    } else {
      logger.fatal("Unknown parent container type: " + parentContainer);
      return null;
    }

    myDialog.setLocationRelativeTo(parentContainer);
    myDialog.checkSettings();

    if (myDialog != null) {
      myDialog.setVisible(true);
    }
    return myDialog.newMotes;
  }

  private AddMoteDialog(Frame frame, Simulation simulation, MoteType moteType) {
    super(frame, "Add motes (" + moteType.getDescription() + ")", ModalityType.APPLICATION_MODAL);
    setupDialog(simulation, moteType);
  }
  private AddMoteDialog(Window window, Simulation simulation, MoteType moteType) {
    super(window, "Add motes (" + moteType.getDescription() + ")", ModalityType.APPLICATION_MODAL);
    setupDialog(simulation, moteType);
  }
  private AddMoteDialog(Dialog dialog, Simulation simulation, MoteType moteType) {
    super(dialog, "Add motes (" + moteType.getDescription() + ")", ModalityType.APPLICATION_MODAL);
    setupDialog(simulation, moteType);
  }

  private void setupDialog(Simulation simulation, MoteType moteType) {
    this.moteType = moteType;
    this.simulation = simulation;

    JLabel label;
    JPanel mainPane = new JPanel();
    mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.Y_AXIS));
    JPanel smallPane;
    JFormattedTextField numberField;
    JButton button;
    JComboBox comboBox;
    NumberFormat integerFormat = NumberFormat.getIntegerInstance();
    NumberFormat doubleFormat = NumberFormat.getNumberInstance();

    // BOTTOM BUTTON PART
    JPanel buttonPane = new JPanel();
    buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.X_AXIS));
    buttonPane.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));

    buttonPane.add(Box.createHorizontalGlue());

    button = new JButton("Do not add motes");
    button.setActionCommand("cancel");
    button.addActionListener(myEventHandler);
    buttonPane.add(button);

    button = new JButton("Add motes");
    button.setActionCommand("add");
    button.addActionListener(myEventHandler);
    this.getRootPane().setDefaultButton(button);
    addButton = button;
    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
    buttonPane.add(button);

    // MAIN PART

    // Number of new motes
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Number of new motes");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    numberField = new JFormattedTextField(integerFormat);
    numberField.setFocusLostBehavior(JFormattedTextField.PERSIST);
    numberField.setValue(new Integer(1));
    numberField.setColumns(10);
    numberField.addFocusListener(myEventHandler);
    numberField.addPropertyChangeListener("value", myEventHandler);
    numberOfMotesField = numberField;

    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(numberField);

    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Position distribution
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Positioning");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    Vector<Class<? extends Positioner>> positioners = simulation.getCooja()
        .getRegisteredPositioners();
    String[] posDistributions = new String[positioners.size()];
    for (int i = 0; i < posDistributions.length; i++) {
      posDistributions[i] = Cooja.getDescriptionOf(positioners.get(i));
    }

    comboBox = new JComboBox(posDistributions);

    comboBox.setSelectedIndex(0);
    comboBox.addActionListener(myEventHandler);
    comboBox.addFocusListener(myEventHandler);
    positionDistributionBox = comboBox;
    label.setLabelFor(comboBox);

    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(comboBox);

    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Position interval X
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));

    label = new JLabel("Position interval");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));

    label = new JLabel("X ");
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));

    numberField = new JFormattedTextField(doubleFormat);
    numberField.setFocusLostBehavior(JFormattedTextField.PERSIST);
    numberField.setValue(new Double(0.0));
    numberField.setColumns(4);
    numberField.addFocusListener(myEventHandler);
    numberField.addPropertyChangeListener("value", myEventHandler);
    startX = numberField;
    smallPane.add(numberField);
    smallPane.add(Box.createHorizontalStrut(10));

    label = new JLabel("<->");
    label.setPreferredSize(new Dimension(LABEL_WIDTH / 4, LABEL_HEIGHT));
    smallPane.add(label);

    numberField = new JFormattedTextField(doubleFormat);
    numberField.setFocusLostBehavior(JFormattedTextField.PERSIST);
    numberField.setValue(new Double(100.0));
    numberField.setColumns(4);
    numberField.addFocusListener(myEventHandler);
    numberField.addPropertyChangeListener("value", myEventHandler);
    endX = numberField;
    smallPane.add(numberField);
    smallPane.add(Box.createHorizontalStrut(10));

    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Position interval Y
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));

    label = new JLabel("");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));

    label = new JLabel("Y ");
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));

    numberField = new JFormattedTextField(doubleFormat);
    numberField.setFocusLostBehavior(JFormattedTextField.PERSIST);
    numberField.setValue(new Double(0.0));
    numberField.setColumns(4);
    numberField.addFocusListener(myEventHandler);
    numberField.addPropertyChangeListener("value", myEventHandler);
    startY = numberField;
    smallPane.add(numberField);
    smallPane.add(Box.createHorizontalStrut(10));

    label = new JLabel("<->");
    label.setPreferredSize(new Dimension(LABEL_WIDTH / 4, LABEL_HEIGHT));
    smallPane.add(label);

    numberField = new JFormattedTextField(doubleFormat);
    numberField.setFocusLostBehavior(JFormattedTextField.PERSIST);
    numberField.setValue(new Double(100.0));
    numberField.setColumns(4);
    numberField.addFocusListener(myEventHandler);
    numberField.addPropertyChangeListener("value", myEventHandler);
    endY = numberField;
    smallPane.add(numberField);
    smallPane.add(Box.createHorizontalStrut(10));

    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Position interval Z
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));

    label = new JLabel("");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));

    label = new JLabel("Z ");
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));

    numberField = new JFormattedTextField(doubleFormat);
    numberField.setFocusLostBehavior(JFormattedTextField.PERSIST);
    numberField.setValue(new Double(0.0));
    numberField.setColumns(4);
    numberField.addFocusListener(myEventHandler);
    numberField.addPropertyChangeListener("value", myEventHandler);
    startZ = numberField;
    smallPane.add(numberField);
    smallPane.add(Box.createHorizontalStrut(10));

    label = new JLabel("<->");
    label.setPreferredSize(new Dimension(LABEL_WIDTH / 4, LABEL_HEIGHT));
    smallPane.add(label);

    numberField = new JFormattedTextField(doubleFormat);
    numberField.setFocusLostBehavior(JFormattedTextField.PERSIST);
    numberField.setValue(new Double(0.0));
    numberField.setColumns(4);
    numberField.addFocusListener(myEventHandler);
    numberField.addPropertyChangeListener("value", myEventHandler);
    endZ = numberField;
    smallPane.add(numberField);
    smallPane.add(Box.createHorizontalStrut(10));

    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    mainPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

    Container contentPane = getContentPane();
    contentPane.add(mainPane, BorderLayout.NORTH);
    contentPane.add(buttonPane, BorderLayout.SOUTH);

    pack();
  }

  private boolean checkSettings() {
    // Check settings
    boolean settingsOK = true;

    if (!checkSettings(startX, endX)) {
      settingsOK = false;
    }
    if (!checkSettings(startY, endY)) {
      settingsOK = false;
    }
    if (!checkSettings(startZ, endZ)) {
      settingsOK = false;
    }

    // Check number of new motes
    try {
      numberOfMotesField.commitEdit();
      if (((Number) numberOfMotesField.getValue()).intValue() < 0) {
	throw new ParseException("Malformed", 0);
      }
      numberOfMotesField.setBackground(Color.WHITE);
      numberOfMotesField.setToolTipText(null);
    } catch (ParseException e) {
      numberOfMotesField.setBackground(Color.RED);
      numberOfMotesField.setToolTipText("Must be >= 1");
      settingsOK = false;
    }

    addButton.setEnabled(settingsOK);

    return settingsOK;
  }

  private boolean checkSettings(JFormattedTextField start,
				JFormattedTextField end) {
    try {
      start.commitEdit();
      end.commitEdit();

      if (((Number) start.getValue()).doubleValue() <=
	  ((Number) end.getValue()).doubleValue()) {
	start.setBackground(Color.WHITE);
	start.setToolTipText(null);
	end.setBackground(Color.WHITE);
	end.setToolTipText(null);
	return true;
      }
    } catch (ParseException e) {
      // Malformed interval
    }
    start.setBackground(Color.RED);
    start.setToolTipText("Malformed interval");
    end.setBackground(Color.RED);
    end.setToolTipText("Malformed interval");
    return false;
  }

  private class AddMotesEventHandler
      implements
        ActionListener,
        FocusListener,
        PropertyChangeListener {
    public void propertyChange(PropertyChangeEvent e) {
      checkSettings();
    }
    public void focusGained(final FocusEvent e) {
      if (e.getSource() instanceof JFormattedTextField) {
        SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            ((JFormattedTextField) e.getSource()).selectAll();
          }
        });
      }
    }
    public void focusLost(FocusEvent e) {
      checkSettings();
    }
    public void actionPerformed(ActionEvent e) {
      if (e.getActionCommand().equals("cancel")) {
        newMotes = null;
        dispose();
      } else if (e.getActionCommand().equals("add")) {
        try {
	  // Validate input
	  if (!checkSettings()) {
	    return;
	  }

	  // Create new motes
          newMotes = new Vector<Mote>();
          int motesToAdd = ((Number) numberOfMotesField.getValue()).intValue();
          while (newMotes.size() < motesToAdd) {
            Mote newMote = moteType.generateMote(simulation);
            newMotes.add(newMote);
          }

          // Position new motes
          Class<? extends Positioner> positionerClass = null;
          for (Class<? extends Positioner> positioner : simulation.getCooja()
              .getRegisteredPositioners()) {
            if (Cooja.getDescriptionOf(positioner).equals(
                positionDistributionBox.getSelectedItem())) {
              positionerClass = positioner;
            }
          }

          Positioner positioner = Positioner.generateInterface(positionerClass,
              motesToAdd,
              ((Number) startX.getValue()).doubleValue(), ((Number) endX
                  .getValue()).doubleValue(), ((Number) startY.getValue())
                  .doubleValue(), ((Number) endY.getValue()).doubleValue(),
                  ((Number) startZ.getValue()).doubleValue(), ((Number) endZ
                      .getValue()).doubleValue());

          if (positioner == null) {
            logger.fatal("Could not create positioner");
            return;
          }

          for (int i = 0; i < newMotes.size(); i++) {
            Position newPosition = newMotes.get(i).getInterfaces().getPosition();
            if (newPosition != null) {
              double[] newPositionArray = positioner.getNextPosition();
              if (newPositionArray.length >= 3) {
                newPosition.setCoordinates(newPositionArray[0],
                    newPositionArray[1], newPositionArray[2]);
              } else if (newPositionArray.length >= 2) {
                newPosition.setCoordinates(newPositionArray[0],
                    newPositionArray[1], 0);
              } else if (newPositionArray.length >= 1) {
                newPosition.setCoordinates(newPositionArray[0], 0, 0);
              } else {
                newPosition.setCoordinates(0, 0, 0);
              }
            }
          }

          /* Set unique mote id's for all new motes
           * TODO ID should be provided differently; not rely on the unsafe MoteID interface */
          int nextMoteID = 1;
          for (Mote m: simulation.getMotes()) {
            int existing = m.getID();
            if (existing >= nextMoteID) {
              nextMoteID = existing + 1;
            }
          }
          for (Mote m: newMotes) {
            MoteID moteID = m.getInterfaces().getMoteID();
            if (moteID != null) {
              moteID.setMoteID(nextMoteID++);
            } else {
              logger.warn("Can't set mote ID (no mote ID interface): " + m);
            }
          }

          dispose();
        } catch (OutOfMemoryError e2) {
          newMotes = null;
          JOptionPane.showMessageDialog(
              AddMoteDialog.this,
              "Out of memory.\nException message: \"" + e2.getMessage() + "\"\n\n" +
              "Reduce number of nodes or start Cooja with more memory (\">ant run_bigmem\").",
              "Not enough heap memory.", JOptionPane.ERROR_MESSAGE
          );
        }
      }
    }
  }

}
