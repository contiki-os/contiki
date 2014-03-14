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
 */

package org.contikios.cooja.mspmote.interfaces;

import java.awt.BorderLayout;
import java.awt.EventQueue;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.IOException;
import java.util.Collection;

import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTable;
import javax.swing.SwingUtilities;
import javax.swing.table.AbstractTableModel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.coffee.CoffeeFS;
import org.contikios.coffee.CoffeeFile;
import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.Mote;
import org.contikios.cooja.MoteInterface;
import org.contikios.cooja.dialogs.TableColumnAdjuster;

/**
 * Mote user interface to Coffee manager.
 * Requires users to manually update the filesystem before filesystem operations.
 *  
 * @author Fredrik Osterlind, Nicolas Tsiftes
 */
@ClassDescription("Coffee Filesystem")
public class SkyCoffeeFilesystem extends MoteInterface {
  private static Logger logger = Logger.getLogger(SkyCoffeeFilesystem.class);

  private Mote mote;

  private CoffeeFS coffeeFS = null;
  private CoffeeFile[] files = new CoffeeFile[0];

  private static final int COLUMN_NAME = 0;
  private static final int COLUMN_SIZE = 1;
  private static final int COLUMN_SAVE = 2;
  private static final int COLUMN_REMOVE = 3;
  private static final String[] COLUMN_NAMES = {
    "Filename",
    "Size",
    "Save",
    "Remove"
  };
  private JTable filesTable;

  public SkyCoffeeFilesystem(Mote mote) {
    this.mote = mote;

    /* Coffee configuration is already loaded statically */

    filesTable = new JTable(tableModel);
    filesTable.setFillsViewportHeight(true);
    filesTable.setFont(new Font("Monospaced", Font.PLAIN, 12));
    TableColumnAdjuster adjuster = new TableColumnAdjuster(filesTable);
    adjuster.setDynamicAdjustment(true);
    adjuster.packColumns();
  }

  public Collection<Element> getConfigXML() {
    return null;
  }
  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

  private void updateFS() {
    /* Create new filesystem instance */
    try {
      SkyFlash flash = mote.getInterfaces().getInterfaceOfType(SkyFlash.class);
      coffeeFS = new CoffeeFS(flash.m24p80);
    } catch (IOException e) {
      logger.fatal(e.getMessage(), e);
      coffeeFS = null;
    }

    files = coffeeFS.getFiles().values().toArray(new CoffeeFile[0]);
    for (CoffeeFile file : files) {
      file.getName();
      try {
        file.getLength();
      } catch (IOException e) {
      }
    }

    EventQueue.invokeLater(new Runnable() {
      public void run() {
        ((AbstractTableModel)filesTable.getModel()).fireTableDataChanged();
      }
    });
  }

  public JPanel getInterfaceVisualizer() {
    JPanel main = new JPanel(new BorderLayout());

    updateFS(); /* Update filesystem */
    if (coffeeFS == null) {
      main = new JPanel();
      main.add(new JLabel("Error when parsing Coffee filesystem"));
      return main;
    }

    /* Files listing */
    JPanel panel = new JPanel(new BorderLayout());
    panel.add(filesTable.getTableHeader(), BorderLayout.NORTH);
    panel.add(filesTable, BorderLayout.CENTER);
    main.add(panel, BorderLayout.CENTER);
    filesTable.repaint();


    /* Update (force) */
    Box box = Box.createHorizontalBox();
    JButton update = new JButton("Update filesystem");
    update.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        logger.info("Updating Coffee filesystem");
        updateFS();
      }
    });
    box.add(update);

    /* Insert */
    JButton insert = new JButton("Insert file");
    insert.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        
        JFileChooser fileChooser = new JFileChooser();
        int reply = fileChooser.showOpenDialog(Cooja.getTopParentContainer());
        if (reply != JFileChooser.APPROVE_OPTION) {
          return;
        }

        final File file = fileChooser.getSelectedFile();
        new Thread(new Runnable() {
          public void run() {
            logger.info("Adding file: " + file.getName());
            try {
              coffeeFS.insertFile(file);
            } catch (IOException e1) {
              logger.fatal("Coffee exception: " + e1.getMessage(), e1);
              return;
            }
            updateFS();
          }
        }).start();
      }
    });
    box.add(insert);

    main.add(box, BorderLayout.SOUTH);

    return main;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
  }

  private AbstractTableModel tableModel = new AbstractTableModel() {
    public String getColumnName(int col) {
      return COLUMN_NAMES[col].toString();
    }
    public int getRowCount() {
      return files.length;
    }
    public int getColumnCount() {
      return COLUMN_NAMES.length;
    }
    public Object getValueAt(int row, int col) {
      if (col == COLUMN_NAME) {
        return files[row].getName();
      }
      if (col == COLUMN_SIZE) {
        try {
          return files[row].getLength() + " bytes";
        } catch (IOException e) {
          return "? bytes";
        }
      }
      return new Boolean(false);
    }
    public boolean isCellEditable(int row, int col){
      return getColumnClass(col) == Boolean.class;
    }
    public void setValueAt(Object value, final int row, int col) {
      if (col == COLUMN_SAVE) {
        JFileChooser fc = new JFileChooser();
        int returnVal = fc.showSaveDialog(Cooja.getTopParentContainer());
        if (returnVal != JFileChooser.APPROVE_OPTION) {
          return;
        }

        final File saveFile = fc.getSelectedFile();
        if (saveFile.exists()) {
          String s1 = "Overwrite";
          String s2 = "Cancel";
          Object[] options = { s1, s2 };
          int n = JOptionPane.showOptionDialog(
              Cooja.getTopParentContainer(),
              "A file with the same name already exists.\nDo you want to remove it?",
              "Overwrite existing file?", JOptionPane.YES_NO_OPTION,
              JOptionPane.QUESTION_MESSAGE, null, options, s1);
          if (n != JOptionPane.YES_OPTION) {
            return;
          }
        }

        if (saveFile.exists() && !saveFile.canWrite()) {
          logger.fatal("No write access to file: " + saveFile);
          return;
        }

        new Thread(new Runnable() {
          public void run() {
            try {
              logger.info("Saving to file: " + saveFile.getName());
              boolean ok = coffeeFS.extractFile(files[row].getName(), saveFile);
              if (!ok) {
                logger.warn("Error when saving to file: " + saveFile.getName());
              }
            } catch (Exception e) {
              logger.fatal("Coffee exception: " + e.getMessage(), e);
            }
            updateFS();
          }
        }).start();
        return;
      }
      
      if (col == COLUMN_REMOVE) {
        int reply = JOptionPane.showConfirmDialog(
            Cooja.getTopParentContainer(),
            "Remove \"" + files[row].getName() + "\" from filesystem?");
        if (reply != JOptionPane.YES_OPTION) {
          return;
        }

        /* Remove file */
        new Thread(new Runnable() {
          public void run() {
            try {
              logger.info("Removing file: " + files[row].getName());
              coffeeFS.removeFile(files[row].getName());
            } catch (Exception e) {
              logger.fatal("Coffee exception: " + e.getMessage(), e);
            }
            updateFS();
          }
        }).start();
        return;
      }
    }
    public Class<?> getColumnClass(int c) {
      return getValueAt(0, c).getClass();
    }
  };
  
  public boolean extractFile(String coffeeFile, String diskFilename) {
    try {
      updateFS();
      return coffeeFS.extractFile(coffeeFile, new File(diskFilename));
    } catch (RuntimeException e) {
      logger.fatal("Error: " + e.getMessage(), e);
      return false;
    } catch (IOException e) {
      logger.fatal("Error: " + e.getMessage(), e);
      return false;
    }
  }
  
  public boolean insertFile(String diskFilename) {
    try {
      updateFS();
      return coffeeFS.insertFile(diskFilename) != null;
    } catch (RuntimeException e) {
      logger.fatal("Error: " + e.getMessage(), e);
      return false;
    } catch (IOException e) {
      logger.fatal("Error: " + e.getMessage(), e);
      return false;
    }
  }

}
