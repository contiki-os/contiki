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
 *
 * -----------------------------------------------------------------
 *
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne, Fredrik Osterlind
 * Created : 2006-06-14
 * Updated : $Date: 2009/11/13 14:27:46 $
 *           $Revision: 1.15 $
 */

package se.sics.cooja.dialogs;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.StringSelection;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.io.PrintStream;
import java.util.ArrayList;

import javax.swing.DefaultListCellRenderer;
import javax.swing.DefaultListModel;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JList;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JSeparator;
import javax.swing.ListModel;
import javax.swing.ListSelectionModel;

import se.sics.cooja.GUI;

public class MessageList extends JList {

  public static final int NORMAL = 0;
  public static final int WARNING = 1;
  public static final int ERROR = 2;

  private Color[] foregrounds = new Color[] { null, Color.red };
  private Color[] backgrounds = new Color[] { null, null };

  private JPopupMenu popup = null;
  private boolean hideNormal = false;

  private int max = -1;
  
  public MessageList() {
    super.setModel(new MessageModel());
    setCellRenderer(new MessageRenderer());
    setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
  }

  /**
   * @param max Maximum number of messages
   */
  public MessageList(int max) {
    this();
    this.max = max;
  }

  public Color getForeground(int type) {
    Color c = type > 0 && type <= foregrounds.length
      ? foregrounds[type - 1] : null;
    return c == null ? getForeground() : c;
  }

  public void setForeground(int type, Color color) {
    if (type > 0 && type <= foregrounds.length) {
      foregrounds[type - 1] = color;
    } else if (type == NORMAL) {
      setForeground(color);
    }
  }

  public Color getBackground(int type) {
    Color c = type > 0 && type <= backgrounds.length
      ? backgrounds[type - 1] : null;
    return c == null ? getBackground() : c;
  }

  public void setBackground(int type, Color color) {
    if (type > 0 && type <= backgrounds.length) {
      backgrounds[type - 1] = color;
    } else if (type == NORMAL) {
      setBackground(color);
    }
  }

  public PrintStream getInputStream(final int type) {
    try {
      PipedInputStream input = new PipedInputStream();
      PipedOutputStream output = new PipedOutputStream(input);
      final BufferedReader stringInput = new BufferedReader(new InputStreamReader(input));

      Thread readThread = new Thread(new Runnable() {
        public void run() {
          String readLine;
          try {
            while ((readLine = stringInput.readLine()) != null) {
              addMessage(readLine, type);
            }
          } catch (IOException e) {
            // Occurs when write end closes pipe - die quietly
          }
        }

      });
      readThread.start();

      return new PrintStream(output);
    } catch (Exception e) {
      System.out.println("Exception: "+ e);
      return null;
    }
  }

  public void addMessage(String message) {
    addMessage(message, NORMAL);
  }

  private ArrayList<MessageContainer> messages = new ArrayList<MessageContainer>();

  public MessageContainer[] getMessages() {
    return messages.toArray(new MessageContainer[0]);
  }

  private void updateModel() {
    boolean scroll = getLastVisibleIndex() >= getModel().getSize() - 2;

    while (messages.size() > getModel().getSize()) {
      ((DefaultListModel) getModel()).addElement(messages.get(getModel().getSize()));
    }
    while (max > 0 && getModel().getSize() > max) {
      ((DefaultListModel) getModel()).removeElementAt(0);
      messages.remove(0);
    }

    if (scroll) {
      ensureIndexIsVisible(getModel().getSize() - 1);
    }
  }

  public void addMessage(final String message, final int type) {
    GUI.setProgressMessage(message, type);

    MessageContainer msg = new MessageContainer(message, type);
    messages.add(msg);

    java.awt.EventQueue.invokeLater(new Runnable() {
      public void run() {
        updateModel();
      }
    });
  }

  public void clearMessages() {
    messages.clear();
    ((DefaultListModel) getModel()).clear();
  }

  public void setModel(ListModel model) {
    throw new IllegalArgumentException("changing model not permitted");
  }

  public void addPopupMenuItem(JMenuItem item, boolean withDefaults) {
    if (popup == null) {
      popup = new JPopupMenu();
      addMouseListener(new MouseAdapter() {
        public void mouseClicked(MouseEvent e) {
          if (e.isPopupTrigger()) {
            popup.show(MessageList.this, e.getX(), e.getY());
          }
        }
        public void mousePressed(MouseEvent e) {
          if (e.isPopupTrigger()) {
            popup.show(MessageList.this, e.getX(), e.getY());
          }
        }
        public void mouseReleased(MouseEvent e) {
          if (e.isPopupTrigger()) {
            popup.show(MessageList.this, e.getX(), e.getY());
          }
        }
      });

      JMenuItem headerMenuItem = new JMenuItem("Output:");
      headerMenuItem.setEnabled(false);
      popup.add(headerMenuItem);
      popup.add(new JSeparator());

      if (withDefaults) {
        /* Create default menu items */
        final JMenuItem hideNormalMenuItem = new JCheckBoxMenuItem("Hide normal output");
        hideNormalMenuItem.setEnabled(true);
        hideNormalMenuItem.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            MessageList.this.hideNormal = hideNormalMenuItem.isSelected();
            ((MessageModel)getModel()).updateList();
          }
        });
        popup.add(hideNormalMenuItem);

        JMenuItem consoleOutputMenuItem = new JMenuItem("Output to console");
        consoleOutputMenuItem.setEnabled(true);
        consoleOutputMenuItem.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            MessageContainer[] messages = getMessages();
            System.out.println("\nCOMPILATION OUTPUT:\n");
            for (MessageContainer msg: messages) {
              if (hideNormal && msg.type == NORMAL) {
                continue;
              }
              System.out.println(msg);
            }
            System.out.println();
          }
        });
        popup.add(consoleOutputMenuItem);

        JMenuItem clipboardMenuItem = new JMenuItem("Copy to clipboard");
        clipboardMenuItem.setEnabled(true);
        clipboardMenuItem.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

            StringBuilder sb = new StringBuilder();
            MessageContainer[] messages = getMessages();
            for (MessageContainer msg: messages) {
              if (hideNormal && msg.type == NORMAL) {
                continue;
              }
              sb.append(msg + "\n");
            }

            StringSelection stringSelection = new StringSelection(sb.toString());
            clipboard.setContents(stringSelection, null);
          }
        });
        popup.add(clipboardMenuItem);

        popup.add(new JSeparator());
      }
    }

    if (item == null) {
      return;
    }

    popup.add(item);
  }

  // -------------------------------------------------------------------
  // MessageContainer
  // -------------------------------------------------------------------

  public static class MessageContainer {
    public final int type;
    public final String message;

    public MessageContainer(String message, int type) {
      this.message = message;
      this.type = type;
    }

    public String toString() {
      return message;
    }

  } // end of inner class MessageContainer


  // -------------------------------------------------------------------
  // Renderer for messages
  // -------------------------------------------------------------------

  private class MessageModel extends DefaultListModel {
    public void updateList() {
      fireContentsChanged(this, 0, getSize());
    }
  }

  private class MessageRenderer extends DefaultListCellRenderer {
    private Dimension nullDimension = new Dimension(0,0);
    public Component getListCellRendererComponent(
        JList list,
	Object value,
        int index,
        boolean isSelected,
        boolean cellHasFocus)
    {
      super.getListCellRendererComponent(list, value, index, isSelected,
					 cellHasFocus);
      MessageContainer msg = (MessageContainer) value;

      if (hideNormal && msg.type == NORMAL && index != MessageList.this.getModel().getSize()-1) {
        setPreferredSize(nullDimension);
        return this;
      }

      setPreferredSize(null);
      setForeground(((MessageList) list).getForeground(msg.type));
      setBackground(((MessageList) list).getBackground(msg.type));
      return this;
    }

  } // end of inner class MessageRenderer

} // end of MessagList
