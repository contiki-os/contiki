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

package se.sics.cooja.positioners;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.text.NumberFormat;
import java.util.Random;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;

import se.sics.cooja.*;

/**
 * Asks for user input for every mote added.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Manual positioning")
public class ManualPositioner extends Positioner {
  private int nodesLeft = 0;
  private boolean skipRemainder = false;
  private double lastX, lastY, lastZ;

  private double startX, endX, startY, endY, startZ, endZ;
  private Random random = new Random(); /* Do not use main random generator for setup */

  public ManualPositioner(int totalNumberOfMotes,
      double startX, double endX,
      double startY, double endY,
      double startZ, double endZ) {
    this.startX = startX;
    this.endX = endX;
    this.startY = startY;
    this.endY = endY;
    this.startZ = startZ;
    this.endZ = endZ;

    nodesLeft = totalNumberOfMotes;

    if (totalNumberOfMotes > 10) {
      int value = JOptionPane.showConfirmDialog(
          GUI.getTopParentContainer(),
          "Do you really want to manually enter the positions of " + totalNumberOfMotes + " new motes ?\n",
          "Manually enter mote positions?",
          JOptionPane.YES_NO_OPTION,
          JOptionPane.WARNING_MESSAGE);
      if (value != JOptionPane.YES_OPTION) {
        throw new RuntimeException("Aborted by user");
      }
    }
  }

  public double[] getNextPosition() {
    /* Generate the rest randomly? */
    if (skipRemainder) {
      return new double[] {
          startX + random.nextDouble()*(endX - startX),
          startY + random.nextDouble()*(endY - startY),
          startZ + random.nextDouble()*(endZ - startZ)
      };
    }

    /* Wait for user input */
    PositionDialog dialog = new PositionDialog(nodesLeft--);
    dialog.xField.setValue(lastX);
    dialog.yField.setValue(lastY);
    dialog.zField.setValue(lastZ);
    dialog.setLocationRelativeTo(GUI.getTopParentContainer());
    dialog.pack();
    dialog.setModal(true);
    dialog.setResizable(false);
    dialog.setVisible(true);
    if (dialog.shouldSkipRemainder) {
      skipRemainder = true;
    }

    if (skipRemainder) {
      return new double[] {
          startX + random.nextDouble()*(endX - startX),
          startY + random.nextDouble()*(endY - startY),
          startZ + random.nextDouble()*(endZ - startZ)
      };
    }

    lastX = ((Number) dialog.xField.getValue()).doubleValue();
    lastY = ((Number) dialog.yField.getValue()).doubleValue();
    lastZ = ((Number) dialog.zField.getValue()).doubleValue();
    return new double[] { lastX, lastY, lastZ };
  }

  class PositionDialog extends JDialog {
    private NumberFormat doubleFormat = NumberFormat.getNumberInstance();

    public boolean shouldSkipRemainder = false;
    public JFormattedTextField xField, yField, zField;
    public PositionDialog(int mote) {
      JButton button;
      JFormattedTextField numberField;

      setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
      setTitle("Motes left: " + mote);

      JPanel panel = new JPanel();
      panel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
      panel.setLayout(new GridLayout(3, 3));
      panel.add(new JLabel("X:"));
      panel.add(new JLabel("Y:"));
      panel.add(new JLabel("Z:"));

      FocusListener focusListener = new FocusListener() {
        public void focusGained(FocusEvent e) {
          final JFormattedTextField source = ((JFormattedTextField)e.getSource());
          SwingUtilities.invokeLater(
              new Runnable() {
                public void run() {
                  source.selectAll();
                }
              }
          );
        }
        public void focusLost(FocusEvent e) {
        }
      };

      numberField = new JFormattedTextField(doubleFormat);
      numberField.setValue(new Double(0.0));
      numberField.setColumns(5);
      numberField.addFocusListener(focusListener);
      panel.add(numberField);
      xField = numberField;
      numberField = new JFormattedTextField(doubleFormat);
      numberField.setValue(new Double(0.0));
      numberField.setColumns(5);
      numberField.addFocusListener(focusListener);
      panel.add(numberField);
      yField = numberField;
      numberField = new JFormattedTextField(doubleFormat);
      numberField.setValue(new Double(0.0));
      numberField.setColumns(5);
      numberField.addFocusListener(focusListener);
      panel.add(numberField);
      zField = numberField;

      panel.add(Box.createHorizontalStrut(10));

      JPanel buttons = new JPanel();
      buttons.setLayout(new BoxLayout(buttons, BoxLayout.X_AXIS));
      buttons.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));
      buttons.add(Box.createHorizontalGlue());

      button = new JButton("Next");
      button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          dispose();
        }
      });
      buttons.add(button);
      getRootPane().setDefaultButton(button);

      button = new JButton("Skip remainder (random)");
      button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          shouldSkipRemainder = true;
          dispose();
        }
      });
      buttons.add(button);

      add(BorderLayout.CENTER, panel);
      add(BorderLayout.SOUTH, buttons);
    }
  }

}
