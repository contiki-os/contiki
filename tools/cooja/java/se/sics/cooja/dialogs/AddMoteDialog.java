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
 * $Id: AddMoteDialog.java,v 1.3 2007/01/10 14:59:07 fros4943 Exp $
 */

package se.sics.cooja.dialogs;

import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.text.*;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;

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

  private boolean settingsOK = true;
  private JButton addButton;

  private MoteType moteType = null;
  private Simulation simulation = null;

  private JFormattedTextField numberOfMotesField, startX, endX, startY, endY,
      startZ, endZ;
  private JComboBox positionDistributionBox, ipDistributionBox;

  
  /**
   * Shows a dialog which enables a user to create and add motes of the given
   * type.
   * 
   * @param parentFrame
   *          Parent frame for dialog
   * @param simulation
   *          Simulation
   * @param moteType
   *          Mote type
   * @return New motes or null if aborted
   */
  public static Vector<Mote> showDialog(Frame parentFrame,
      Simulation simulation, MoteType moteType) {

    AddMoteDialog myDialog = new AddMoteDialog(parentFrame, simulation, moteType);
    myDialog.setLocationRelativeTo(parentFrame);
    myDialog.checkSettings();

    if (myDialog != null) {
      myDialog.setVisible(true);
    }
    return myDialog.newMotes;
  }

  private AddMoteDialog(Frame frame, Simulation simulation, MoteType moteType) {
    super(frame, "Add motes (" + moteType.getDescription() + ")", true);
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

    button = new JButton("Cancel");
    button.setActionCommand("cancel");
    button.addActionListener(myEventHandler);
    buttonPane.add(button);

    button = new JButton("Create and Add");
    button.setEnabled(settingsOK);
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
    numberField.setValue(new Integer(1));
    numberField.setColumns(10);
    numberField.addPropertyChangeListener("value", myEventHandler);
    numberOfMotesField = numberField;

    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(numberField);

    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // IP address distribution
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("IP Addressing");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    Vector<Class<? extends IPDistributor>> ipDistributors = simulation.getGUI()
        .getRegisteredIPDistributors();
    String[] ipDistributions = new String[ipDistributors.size()];
    for (int i = 0; i < ipDistributions.length; i++)
      ipDistributions[i] = GUI.getDescriptionOf(ipDistributors.get(i));

    comboBox = new JComboBox(ipDistributions);

    comboBox.setSelectedIndex(0);
    comboBox.addActionListener(myEventHandler);
    comboBox.addFocusListener(myEventHandler);
    ipDistributionBox = comboBox;
    label.setLabelFor(comboBox);

    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(comboBox);

    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Position distribution
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Positioning");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    Vector<Class<? extends Positioner>> positioners = simulation.getGUI()
        .getRegisteredPositioners();
    String[] posDistributions = new String[positioners.size()];
    for (int i = 0; i < posDistributions.length; i++)
      posDistributions[i] = GUI.getDescriptionOf(positioners.get(i));

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
    numberField.setValue(new Double(0.0));
    numberField.setColumns(4);
    numberField.addPropertyChangeListener("value", myEventHandler);
    startX = numberField;
    smallPane.add(numberField);
    smallPane.add(Box.createHorizontalStrut(10));

    label = new JLabel("<->");
    label.setPreferredSize(new Dimension(LABEL_WIDTH / 4, LABEL_HEIGHT));
    smallPane.add(label);

    numberField = new JFormattedTextField(doubleFormat);
    numberField.setValue(new Double(100.0));
    numberField.setColumns(4);
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
    numberField.setValue(new Double(0.0));
    numberField.setColumns(4);
    numberField.addPropertyChangeListener("value", myEventHandler);
    startY = numberField;
    smallPane.add(numberField);
    smallPane.add(Box.createHorizontalStrut(10));

    label = new JLabel("<->");
    label.setPreferredSize(new Dimension(LABEL_WIDTH / 4, LABEL_HEIGHT));
    smallPane.add(label);

    numberField = new JFormattedTextField(doubleFormat);
    numberField.setValue(new Double(100.0));
    numberField.setColumns(4);
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
    numberField.setValue(new Double(0.0));
    numberField.setColumns(4);
    numberField.addPropertyChangeListener("value", myEventHandler);
    startZ = numberField;
    smallPane.add(numberField);
    smallPane.add(Box.createHorizontalStrut(10));

    label = new JLabel("<->");
    label.setPreferredSize(new Dimension(LABEL_WIDTH / 4, LABEL_HEIGHT));
    smallPane.add(label);

    numberField = new JFormattedTextField(doubleFormat);
    numberField.setValue(new Double(0.0));
    numberField.setColumns(4);
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

  private void checkSettings() {
    // Check settings
    settingsOK = true;

    // Check X interval
    if (((Number) startX.getValue()).doubleValue() > ((Number) endX.getValue())
        .doubleValue()) {
      startX.setBackground(Color.RED);
      startX.setToolTipText("Malformed interval");
      endX.setBackground(Color.RED);
      endX.setToolTipText("Malformed interval");
      settingsOK = false;
    } else {
      startX.setBackground(Color.WHITE);
      startX.setToolTipText(null);
      endX.setBackground(Color.WHITE);
      endX.setToolTipText(null);
    }

    // Check Y interval
    if (((Number) startY.getValue()).doubleValue() > ((Number) endY.getValue())
        .doubleValue()) {
      startY.setBackground(Color.RED);
      startY.setToolTipText("Malformed interval");
      endY.setBackground(Color.RED);
      endY.setToolTipText("Malformed interval");
      settingsOK = false;
    } else {
      startY.setBackground(Color.WHITE);
      startY.setToolTipText(null);
      endY.setBackground(Color.WHITE);
      endY.setToolTipText(null);
    }

    // Check Z interval
    if (((Number) startZ.getValue()).doubleValue() > ((Number) endZ.getValue())
        .doubleValue()) {
      startZ.setBackground(Color.RED);
      startZ.setToolTipText("Malformed interval");
      endZ.setBackground(Color.RED);
      endZ.setToolTipText("Malformed interval");
      settingsOK = false;
    } else {
      startZ.setBackground(Color.WHITE);
      startZ.setToolTipText(null);
      endZ.setBackground(Color.WHITE);
      endZ.setToolTipText(null);
    }

    // Check number of new motes
    if (((Number) numberOfMotesField.getValue()).intValue() < 1) {
      numberOfMotesField.setBackground(Color.RED);
      numberOfMotesField.setToolTipText("Must be >= 1");
      settingsOK = false;
    } else {
      numberOfMotesField.setBackground(Color.WHITE);
      numberOfMotesField.setToolTipText(null);
    }

    addButton.setEnabled(settingsOK);
  }

  private class AddMotesEventHandler
      implements
        ActionListener,
        FocusListener,
        PropertyChangeListener {
    public void propertyChange(PropertyChangeEvent e) {
      checkSettings();
    }
    public void focusGained(FocusEvent e) {
      // NOP
    }
    public void focusLost(FocusEvent e) {
      checkSettings();
    }
    public void actionPerformed(ActionEvent e) {
      if (e.getActionCommand().equals("cancel")) {
        newMotes = null;
        dispose();
      } else if (e.getActionCommand().equals("add")) {
        newMotes = new Vector<Mote>();

        // Create new motes
        int motesToAdd = ((Number) numberOfMotesField.getValue()).intValue();
        while (newMotes.size() < motesToAdd) {
          Mote newMote = moteType.generateMote(simulation);
          newMotes.add(newMote);
        }

        // Position new motes
        Class<? extends Positioner> positionerClass = null;
        for (Class<? extends Positioner> positioner : simulation.getGUI()
            .getRegisteredPositioners()) {
          if (GUI.getDescriptionOf(positioner).equals(
              (String) positionDistributionBox.getSelectedItem()))
            positionerClass = positioner;
        }

        Positioner positioner = Positioner.generateInterface(positionerClass,
            ((Number) numberOfMotesField.getValue()).intValue(),
            ((Number) startX.getValue()).doubleValue(), ((Number) endX
                .getValue()).doubleValue(), ((Number) startY.getValue())
                .doubleValue(), ((Number) endY.getValue()).doubleValue(),
            ((Number) startZ.getValue()).doubleValue(), ((Number) endZ
                .getValue()).doubleValue());

        if (positioner == null) {
          logger.fatal("Could not create positioner");
          dispose();
          return;
        }

        for (int i = 0; i < newMotes.size(); i++) {
          Position newPosition = newMotes.get(i).getInterfaces().getPosition();
          if (newPosition != null) {
            double[] newPositionArray = positioner.getNextPosition();
            if (newPositionArray.length >= 3)
              newPosition.setCoordinates(newPositionArray[0],
                  newPositionArray[1], newPositionArray[2]);
            else if (newPositionArray.length >= 2)
              newPosition.setCoordinates(newPositionArray[0],
                  newPositionArray[1], 0);
            else if (newPositionArray.length >= 1)
              newPosition.setCoordinates(newPositionArray[0], 0, 0);
            else
              newPosition.setCoordinates(0, 0, 0);
          }
        }

        // Set unique mote id's for all new motes
        int nextMoteID = 1;
        for (int i = 0; i < simulation.getMotesCount(); i++) {
          MoteID moteID = simulation.getMote(i).getInterfaces()
              .getMoteID();
          if (moteID != null && moteID.getMoteID() >= nextMoteID)
            nextMoteID = moteID.getMoteID() + 1;
        }

        for (int i = 0; i < newMotes.size(); i++) {
          MoteID moteID = newMotes.get(i).getInterfaces().getMoteID();
          if (moteID != null) {
            moteID.setMoteID(nextMoteID++);
          }
        }

        // IP address new motes
        Class<? extends IPDistributor> ipDistClass = null;
        for (Class<? extends IPDistributor> ipDistributor : simulation.getGUI()
            .getRegisteredIPDistributors()) {
          if (GUI.getDescriptionOf(ipDistributor).equals(
              (String) ipDistributionBox.getSelectedItem()))
            ipDistClass = ipDistributor;
        }

        IPDistributor ipDistributor = IPDistributor.generateIPDistributor(
            ipDistClass, newMotes);

        if (ipDistributor == null) {
          logger.fatal("Could not create IP distributor");
          dispose();
          return;
        }

        for (int i = 0; i < newMotes.size(); i++) {
          String newIPString = ipDistributor.getNextIPAddress();
          if (newMotes.get(i).getInterfaces().getIPAddress() != null)
            newMotes.get(i).getInterfaces().getIPAddress().setIPString(
                newIPString);
        }

        dispose();
      }
    }
  }

}
