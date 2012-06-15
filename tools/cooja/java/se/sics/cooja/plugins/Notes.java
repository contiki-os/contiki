/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: Notes.java,v 1.1 2010/03/11 22:11:10 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.Collection;

import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.SwingUtilities;
import javax.swing.plaf.basic.BasicInternalFrameUI;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;

@ClassDescription("Notes")
@PluginType(PluginType.SIM_STANDARD_PLUGIN)
public class Notes extends VisPlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(Visualizer.class);

  private JTextArea notes = new JTextArea("Enter notes here");
  private boolean decorationsVisible = true;

  public Notes(Simulation simulation, GUI gui) {
    super("Notes", gui);

    add(BorderLayout.CENTER, new JScrollPane(notes));

    /* Popup menu */
    if (Notes.this.getUI() instanceof BasicInternalFrameUI) {
      final JPopupMenu popup = new JPopupMenu();
      JMenuItem headerMenuItem = new JMenuItem("Toggle decorations");
      headerMenuItem.setEnabled(true);
      headerMenuItem.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          setDecorationsVisible(!decorationsVisible);
        }
      });
      popup.add(headerMenuItem);
      notes.addMouseListener(new MouseAdapter() {
        public void mousePressed(MouseEvent e) {
          if (e.isPopupTrigger()) {
            popup.show(Notes.this, e.getX(), e.getY());
          }
        }
        public void mouseReleased(MouseEvent e) {
          if (e.isPopupTrigger()) {
            popup.show(Notes.this, e.getX(), e.getY());
          }
        }
        public void mouseClicked(MouseEvent e) {
          if (e.isPopupTrigger()) {
            popup.show(Notes.this, e.getX(), e.getY());
          }
        }
      });
    }


    /* XXX HACK: here we set the position and size of the window when it appears on a blank simulation screen. */
    this.setLocation(680, 0);
    this.setSize(gui.getDesktopPane().getWidth() - 680, 160);
  }

  public String getNotes() {
    return notes.getText();
  }

  public void setNotes(String text) {
    this.notes.setText(text);
  }

  private void setDecorationsVisible(boolean visible) {
    if (!(Notes.this.getUI() instanceof BasicInternalFrameUI)) {
      return;
    }
    BasicInternalFrameUI ui = (BasicInternalFrameUI) Notes.this.getUI();

    if (visible) {
      ui.getNorthPane().setPreferredSize(null);
    } else {
      ui.getNorthPane().setPreferredSize(new Dimension(0,0));
    }

    Notes.this.revalidate();
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        Notes.this.repaint();
      }
    });

    decorationsVisible = visible;
  }

  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    element = new Element("notes");
    element.setText(notes.getText());
    config.add(element);

    element = new Element("decorations");
    element.setText("" + decorationsVisible);
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("notes")) {
        notes.setText(element.getText());
      }
      if (element.getName().equals("decorations")) {
        decorationsVisible = Boolean.parseBoolean(element.getText());
        setDecorationsVisible(decorationsVisible);
      }
    }
    return true;
  }
}
