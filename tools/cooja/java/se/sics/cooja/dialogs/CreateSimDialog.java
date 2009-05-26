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
 * $Id: CreateSimDialog.java,v 1.16 2009/05/26 14:25:07 fros4943 Exp $
 */

package se.sics.cooja.dialogs;

import java.awt.*;
import java.awt.event.*;
import java.text.*;
import java.util.Random;
import java.util.Vector;
import javax.swing.*;

import org.apache.log4j.Logger;

import se.sics.cooja.*;

/**
 * A dialog for creating and configuring a simulation.
 *
 * @author Fredrik Osterlind
 */
public class CreateSimDialog extends JDialog {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(CreateSimDialog.class);

  private AddSimEventHandler myEventHandler = new AddSimEventHandler();

  private final static int LABEL_WIDTH = 170;
  private final static int LABEL_HEIGHT = 25;

  private Simulation mySimulation = null;
  private GUI myGUI = null;

  private CreateSimDialog myDialog;

  private JFormattedTextField delayTime, simulationTime, tickTime;
  private JFormattedTextField randomSeed, tickLists, delayedStartup;
  private JCheckBox randomSeedGenerated;

  private JTextField title;
  private JComboBox radioMediumBox;

  private JButton cancelButton;

  /**
   * Shows a dialog for configuring a simulation.
   *
   * @param parentContainer Parent container for dialog
   * @param simulationToConfigure Simulation to configure
   * @return True if simulation configured correctly
   */
  public static boolean showDialog(Container parentContainer, Simulation simulationToConfigure) {
    final CreateSimDialog myDialog;
    if (parentContainer instanceof Window) {
      myDialog = new CreateSimDialog((Window) parentContainer, simulationToConfigure.getGUI());
    } else if (parentContainer instanceof Dialog) {
      myDialog = new CreateSimDialog((Dialog) parentContainer, simulationToConfigure.getGUI());
    } else if (parentContainer instanceof Frame) {
      myDialog = new CreateSimDialog((Frame) parentContainer, simulationToConfigure.getGUI());
    } else {
      logger.fatal("Unknown parent container type: " + parentContainer);
      return false;
    }

    myDialog.setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
    myDialog.addWindowListener(new WindowListener() {
      public void windowDeactivated(WindowEvent e) {
      }

      public void windowIconified(WindowEvent e) {
      }

      public void windowDeiconified(WindowEvent e) {
      }

      public void windowOpened(WindowEvent e) {
      }

      public void windowClosed(WindowEvent e) {
      }

      public void windowActivated(WindowEvent e) {
      }

      public void windowClosing(WindowEvent e) {
        myDialog.cancelButton.doClick();
      }
    });

    myDialog.mySimulation = simulationToConfigure;

    // Set title
    if (simulationToConfigure.getTitle() != null) {
      // Title already preset
      myDialog.title.setText(simulationToConfigure.getTitle());
    } else {
      // Suggest title
      myDialog.title.setText("My simulation");
    }

    // Set delay time
    myDialog.delayTime.setValue(new Integer(simulationToConfigure.getDelayTime()));

    // Set simulation time
    myDialog.simulationTime.setValue(new Long(simulationToConfigure.getSimulationTime()/Simulation.MILLISECOND));

    // Select radio medium
    if (simulationToConfigure.getRadioMedium() != null) {
      Class<? extends RadioMedium> radioMediumClass =
        simulationToConfigure.getRadioMedium().getClass();

      String currentDescription = GUI.getDescriptionOf(radioMediumClass);

      for (int i=0; i < myDialog.radioMediumBox.getItemCount(); i++) {
        String menuDescription = (String) myDialog.radioMediumBox.getItemAt(i);
        if (menuDescription.equals(currentDescription)) {
          myDialog.radioMediumBox.setSelectedIndex(i);
          break;
        }
      }
    }

    // Set random seed
    if (simulationToConfigure.getRandomSeedGenerated()) {
      myDialog.randomSeedGenerated.setSelected(true);
      myDialog.randomSeed.setEnabled(false);
      myDialog.randomSeed.setText("[autogenerated]");
    } else {
      myDialog.randomSeed.setEnabled(true);
      myDialog.randomSeed.setValue(new Long(simulationToConfigure.getRandomSeed()));
    }

    // Set delayed mote startup time (ms)
    myDialog.delayedStartup.setValue(new Long(simulationToConfigure.getDelayedMoteStartupTime()/Simulation.MILLISECOND));


    // Set position and focus of dialog
    myDialog.setLocationRelativeTo(parentContainer);
    myDialog.title.requestFocus();
    myDialog.title.select(0, myDialog.title.getText().length());

    // Dispose on escape key
    InputMap inputMap = myDialog.getRootPane().getInputMap(JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
    inputMap.put(KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0, false), "dispose");
    AbstractAction cancelAction = new AbstractAction(){
      public void actionPerformed(ActionEvent e) {
        myDialog.cancelButton.doClick();
      }
    };
    myDialog.getRootPane().getActionMap().put("dispose", cancelAction);

    myDialog.setVisible(true);

    if (myDialog.mySimulation != null) {
      // Simulation configured correctly
      return true;
    }
    return false;
  }

  private CreateSimDialog(Dialog dialog, GUI gui) {
    super(dialog, "Create new simulation", ModalityType.APPLICATION_MODAL);
    setupDialog(gui);
  }
  private CreateSimDialog(Window window, GUI gui) {
    super(window, "Create new simulation", ModalityType.APPLICATION_MODAL);
    setupDialog(gui);
  }
  private CreateSimDialog(Frame frame, GUI gui) {
    super(frame, "Create new simulation", ModalityType.APPLICATION_MODAL);
    setupDialog(gui);
  }

  private void setupDialog(GUI gui) {
    myDialog = this;
    myGUI = gui;

    Box vertBox = Box.createVerticalBox();

    JLabel label;
    JTextField textField;
    Box horizBox;
    JButton button;
    JComboBox comboBox;
    JFormattedTextField numberField;
    NumberFormat integerFormat = NumberFormat.getIntegerInstance();


    // BOTTOM BUTTON PART
    Box buttonBox = Box.createHorizontalBox();
    buttonBox.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));

    buttonBox.add(Box.createHorizontalGlue());

    cancelButton = new JButton("Cancel");
    cancelButton.setActionCommand("cancel");
    cancelButton.addActionListener(myEventHandler);
    buttonBox.add(cancelButton);

    button = new JButton("Create");
    button.setActionCommand("create");
    button.addActionListener(myEventHandler);
    buttonBox.add(Box.createHorizontalStrut(5));
    myDialog.rootPane.setDefaultButton(button);
    buttonBox.add(button);


    // MAIN PART

    // Title
    horizBox = Box.createHorizontalBox();
    horizBox.setMaximumSize(new Dimension(Integer.MAX_VALUE,LABEL_HEIGHT));
    horizBox.setAlignmentX(Component.LEFT_ALIGNMENT);
    label = new JLabel("Simulation title");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));

    textField = new JTextField();
    textField.setText("[no title]");
    textField.setColumns(25);
    title = textField;

    horizBox.add(label);
    horizBox.add(Box.createHorizontalStrut(10));
    horizBox.add(textField);

    vertBox.add(horizBox);
    vertBox.add(Box.createRigidArea(new Dimension(0,5)));

    // Radio Medium selection
    horizBox = Box.createHorizontalBox();
    horizBox.setMaximumSize(new Dimension(Integer.MAX_VALUE,LABEL_HEIGHT));
    horizBox.setAlignmentX(Component.LEFT_ALIGNMENT);
    label = new JLabel("Radio Medium");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));

    Vector<String> radioMediumDescriptions = new Vector<String>();
    for (Class<? extends RadioMedium> radioMediumClass: gui.getRegisteredRadioMediums()) {
      String description = GUI.getDescriptionOf(radioMediumClass);
      radioMediumDescriptions.add(description);
    }

    comboBox = new JComboBox(radioMediumDescriptions);

    comboBox.setSelectedIndex(0);
    radioMediumBox = comboBox;
    label.setLabelFor(comboBox);

    horizBox.add(label);
    horizBox.add(Box.createHorizontalStrut(10));
    horizBox.add(comboBox);
    horizBox.setToolTipText("Determines the radio surroundings behaviour");

    vertBox.add(horizBox);
    vertBox.add(Box.createRigidArea(new Dimension(0,5)));


/*    // Radio Medium Logging selection
    smallPane = Box.createHorizontalBox();
    smallPane.setMaximumSize(new Dimension(Integer.MAX_VALUE,LABEL_HEIGHT));
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    logCheckBox = new JCheckBox("Log all radio traffic?");
    logCheckBox.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));

    textField = new JTextField();
    textField.setText("[filename]");
    textField.setColumns(25);
    logFilename = textField;

    smallPane.add(logCheckBox);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(textField);


    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,5)));
*/


    // -- Advanced settings --
    Box advancedBox = Box.createVerticalBox();
    advancedBox.setBorder(BorderFactory.createTitledBorder("Advanced settings"));

    // Start time
    horizBox = Box.createHorizontalBox();
    horizBox.setMaximumSize(new Dimension(Integer.MAX_VALUE,LABEL_HEIGHT));
    horizBox.setAlignmentX(Component.LEFT_ALIGNMENT);
    label = new JLabel("Simulation start time (ms)");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));

    numberField = new JFormattedTextField(integerFormat);
    numberField.setValue(new Integer(0));
    numberField.setColumns(4);
    numberField.setEnabled(false); /* Disabled: Almost never used */
    simulationTime = numberField;

    horizBox.add(label);
    horizBox.add(Box.createHorizontalStrut(150));
    horizBox.add(numberField);
    horizBox.setToolTipText("Initial value of simulated time");

    advancedBox.add(horizBox);
    advancedBox.add(Box.createRigidArea(new Dimension(0,5)));

    // Delayed startup
    horizBox = Box.createHorizontalBox();
    horizBox.setMaximumSize(new Dimension(Integer.MAX_VALUE,LABEL_HEIGHT));
    horizBox.setAlignmentX(Component.LEFT_ALIGNMENT);
    label = new JLabel("Random startup (max, ms)");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));

    numberField = new JFormattedTextField(integerFormat);
    numberField.setValue(new Integer(10000));
    numberField.setColumns(4);
    delayedStartup = numberField;

    horizBox.add(label);
    horizBox.add(Box.createHorizontalStrut(150));
    horizBox.add(numberField);
    horizBox.setToolTipText("Maximum mote startup delay (random interval: [0, time])");

    advancedBox.add(horizBox);
    advancedBox.add(Box.createVerticalStrut(5));

    advancedBox.add(Box.createVerticalStrut(5));

    // Delay time
    horizBox = Box.createHorizontalBox();
    horizBox.setMaximumSize(new Dimension(Integer.MAX_VALUE,LABEL_HEIGHT));
    horizBox.setAlignmentX(Component.LEFT_ALIGNMENT);
    label = new JLabel("Delay time (ms)");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));

    numberField = new JFormattedTextField(integerFormat);
    numberField.setValue(new Integer(100));
    numberField.setColumns(4);
    delayTime = numberField;

    horizBox.add(label);
    horizBox.add(Box.createHorizontalStrut(150));
    horizBox.add(numberField);
    horizBox.setToolTipText("Delay between each simulated millisecond. Controls simulation speed.");

    advancedBox.add(horizBox);
    advancedBox.add(Box.createVerticalStrut(5));

    advancedBox.add(Box.createVerticalStrut(5));

    // Random seed
    horizBox = Box.createHorizontalBox();
    horizBox.setMaximumSize(new Dimension(Integer.MAX_VALUE,LABEL_HEIGHT));
    horizBox.setAlignmentX(Component.LEFT_ALIGNMENT);
    label = new JLabel("Main random seed");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));

    numberField = new JFormattedTextField(integerFormat);
    numberField.setValue(new Integer(123456));
    numberField.setColumns(4);
    randomSeed = numberField;

    randomSeedGenerated = new JCheckBox();
    randomSeedGenerated.setToolTipText("Autmatically generate random seed at simulation load");
    randomSeedGenerated.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (((JCheckBox)e.getSource()).isSelected()) {
          randomSeed.setEnabled(false);
          randomSeed.setText("[autogenerated]");
        } else {
          randomSeed.setEnabled(true);
          randomSeed.setValue(new Integer(123456));
        }
      }

    });
    horizBox.add(label);
    horizBox.add(Box.createHorizontalStrut(150));
    horizBox.add(numberField);
    horizBox.add(randomSeedGenerated);
    horizBox.setToolTipText("Simulation random seed. Controls the random behavior such as mote startup delays, node positions etc.");

    advancedBox.add(horizBox);
    advancedBox.add(Box.createVerticalStrut(5));

    vertBox.add(advancedBox);
    vertBox.add(Box.createVerticalGlue());

    vertBox.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));

    Container contentPane = getContentPane();
    contentPane.add(vertBox, BorderLayout.CENTER);
    contentPane.add(buttonBox, BorderLayout.SOUTH);

    pack();
  }

  private class AddSimEventHandler implements ActionListener {
    public void actionPerformed(ActionEvent e) {
      if (e.getActionCommand().equals("cancel")) {
        mySimulation = null;
        dispose();
      } else if (e.getActionCommand().equals("create")) {
        mySimulation.setDelayTime(((Number) delayTime.getValue()).intValue());
        mySimulation.setSimulationTime(((Number) simulationTime.getValue()).intValue());
        mySimulation.setTitle(title.getText());

        String currentRadioMediumDescription = (String) radioMediumBox.getSelectedItem();
        for (Class<? extends RadioMedium> radioMediumClass: myGUI.getRegisteredRadioMediums()) {
          String radioMediumDescription = GUI.getDescriptionOf(radioMediumClass);

          if (currentRadioMediumDescription.equals(radioMediumDescription)) {
            try {
              RadioMedium radioMedium = RadioMedium.generateRadioMedium(radioMediumClass, mySimulation);
              mySimulation.setRadioMedium(radioMedium);
            } catch (Exception ex) {
              logger.fatal("Exception when creating radio medium: " + ex);
              ex.printStackTrace();
              mySimulation.setRadioMedium(null);
            }
            break;
          }
        }

        if (randomSeedGenerated.isSelected()) {
          mySimulation.setRandomSeedGenerated(true);
          mySimulation.setRandomSeed(new Random().nextLong());
        } else {
          mySimulation.setRandomSeedGenerated(false);
          mySimulation.setRandomSeed(((Number) randomSeed.getValue()).longValue());
        }

        mySimulation.setDelayedMoteStartupTime((int) ((Number) delayedStartup.getValue()).intValue()*Simulation.MILLISECOND);

        dispose();
      }
    }
  }

}
