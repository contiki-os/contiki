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
 * $Id: CreateSimDialog.java,v 1.5 2007/03/22 23:06:26 fros4943 Exp $
 */

package se.sics.cooja.dialogs;

import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.text.*;
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
  private final static int LABEL_HEIGHT = 15;
  
  private Simulation mySimulation = null;
  private GUI myGUI = null;
  
  private CreateSimDialog myDialog;
  
  private JFormattedTextField delayTime, simulationTime, tickTime;
  private JTextField title;
  private JComboBox radioMediumBox;
  
  private JTextField logFilename;
  private JCheckBox logCheckBox;
  private JButton cancelButton;
  
  /**
   * Shows a dialog for configuring a simulation.
   *
   * @param parentFrame Parent frame for dialog
   * @param simulationToConfigure Simulation to configure
   * @return True if simulation configured correctly
   */
  public static boolean showDialog(Frame parentFrame, Simulation simulationToConfigure) {
    final CreateSimDialog myDialog = new CreateSimDialog(parentFrame, simulationToConfigure.getGUI());

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
      myDialog.title.setText("[enter simulation title]");
    }

    // Set delay time
    myDialog.delayTime.setValue(new Integer(simulationToConfigure.getDelayTime()));

    // Set simulation time
    myDialog.simulationTime.setValue(new Integer(simulationToConfigure.getSimulationTime()));

    // Set tick time
    myDialog.tickTime.setValue(new Integer(simulationToConfigure.getTickTime()));

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
    
    // Set position and focus of dialog
    myDialog.setLocationRelativeTo(parentFrame);
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
  
  private CreateSimDialog(Frame frame, GUI gui) {
    super(frame, "Create new simulation", true);
    
    myDialog = this;
    myGUI = gui;
    
    JPanel mainPane = new JPanel();
    mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.Y_AXIS));
    
    JLabel label;
    JTextField textField;
    JPanel smallPane;
    JButton button;
    JComboBox comboBox;
    JFormattedTextField numberField;
    NumberFormat integerFormat = NumberFormat.getIntegerInstance();
    
    
    // BOTTOM BUTTON PART
    JPanel buttonPane = new JPanel();
    buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.X_AXIS));
    buttonPane.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));
    
    buttonPane.add(Box.createHorizontalGlue());
    
    cancelButton = new JButton("Cancel");
    cancelButton.setActionCommand("cancel");
    cancelButton.addActionListener(myEventHandler);
    buttonPane.add(cancelButton);
    
    button = new JButton("Create");
    button.setActionCommand("create");
    button.addActionListener(myEventHandler);
    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
    myDialog.rootPane.setDefaultButton(button);
    buttonPane.add(button);
    
    
    // MAIN PART
    
    // Title
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Simulation title");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    
    textField = new JTextField();
    textField.setText("[no title]");
    textField.setColumns(25);
    title = textField;
    
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(textField);
    
    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,5)));
    
    // Radio Medium selection
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
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
    
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(comboBox);
    
    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,5)));
    
    
    // Radio Medium Logging selection
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
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
    
    
    // Delay time
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Delay time (ms)");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    
    numberField = new JFormattedTextField(integerFormat);
    numberField.setValue(new Integer(100));
    numberField.setColumns(4);
    delayTime = numberField;
    
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(150));
    smallPane.add(numberField);
    
    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,5)));
    
    // Simulation start time
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Simulation start time (ms)");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    
    numberField = new JFormattedTextField(integerFormat);
    numberField.setValue(new Integer(0));
    numberField.setColumns(4);
    simulationTime = numberField;
    
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(150));
    smallPane.add(numberField);
    
    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,5)));
    
    // Tick time
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Tick time (ms)");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    
    numberField = new JFormattedTextField(integerFormat);
    numberField.setValue(new Integer(1));
    numberField.setColumns(4);
    tickTime = numberField;
    
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(150));
    smallPane.add(numberField);
    
    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,5)));
    
    
    mainPane.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
    
    Container contentPane = getContentPane();
    contentPane.add(mainPane, BorderLayout.NORTH);
    contentPane.add(buttonPane, BorderLayout.SOUTH);
    
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
        mySimulation.setTickTime(((Number) tickTime.getValue()).intValue());
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
              mySimulation.setRadioMedium(null);
            }
            break;
          }
        }
        
        if (logCheckBox.isSelected()) {
          ConnectionLogger connLogger = new ConnectionLogger(new File(logFilename.getText()));
          mySimulation.getRadioMedium().setConnectionLogger(connLogger);
        }
        
        dispose();
      }
    }
  }
  
}
