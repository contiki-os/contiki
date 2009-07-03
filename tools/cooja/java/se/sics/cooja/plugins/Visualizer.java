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
 * $Id: Visualizer.java,v 1.9 2009/07/03 14:06:20 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.*;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.awt.dnd.DnDConstants;
import java.awt.dnd.DropTarget;
import java.awt.dnd.DropTargetDragEvent;
import java.awt.dnd.DropTargetDropEvent;
import java.awt.dnd.DropTargetEvent;
import java.awt.dnd.DropTargetListener;
import java.awt.event.*;
import java.io.File;
import java.io.IOException;
import java.util.*;
import java.util.List;

import javax.swing.*;
import javax.swing.Timer;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.GUI.MoteRelation;
import se.sics.cooja.SimEventCentral.MoteCountListener;
import se.sics.cooja.interfaces.*;
import se.sics.cooja.plugins.skins.AddressVisualizerSkin;
import se.sics.cooja.plugins.skins.IDVisualizerSkin;
import se.sics.cooja.plugins.skins.LEDVisualizerSkin;
import se.sics.cooja.plugins.skins.LogVisualizerSkin;

/**
 * Simulation visualizer supporting visualization skins.
 * Motes are painted in the XY-plane, as seen from positive Z axis.
 *
 * Supports drag-n-drop motes, right-click popup menu, and visualization skins.
 *
 * Observes the simulation and all mote positions.
 *
 * @see #registerMoteMenuAction(MoteMenuAction)
 * @see #registerSimulationMenuAction(SimulationMenuAction)
 * @see #registerVisualizerSkin(Class)
 * @see UDGMVisualizerSkin
 * @author Fredrik Osterlind
 */
@ClassDescription("Simulation visualizer")
@PluginType(PluginType.SIM_STANDARD_PLUGIN)
public class Visualizer extends VisPlugin {

  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(Visualizer.class);

  private static final int CANVAS_BORDER_WIDTH = 25;
  public static final int MOTE_RADIUS = 8;

  private static final Color[] DEFAULT_MOTE_COLORS = { Color.WHITE };

  private GUI gui = null;
  private Simulation simulation = null;
  private final JPanel canvas;

  /* Transformation */
  private double factorXCoordToPixel;
  private double factorYCoordToPixel;
  private double smallestXCoord;
  private double smallestYCoord;

  /* Moving motes */
  private boolean moteIsBeingMoved = false;
  private long moteMoveBeginTime = -1;
  private Mote moteToMove = null;
  private Cursor moveCursor = new Cursor(Cursor.MOVE_CURSOR);

  /* Visualizer skins */
  private final JButton skinButton = new JButton("Select visualizer skins");
  private static ArrayList<Class<? extends VisualizerSkin>> visualizerSkins =
    new ArrayList<Class<? extends VisualizerSkin>>();
  static {
    /* Register default visualizer skins */
    registerVisualizerSkin(IDVisualizerSkin.class);
    registerVisualizerSkin(AddressVisualizerSkin.class);
    registerVisualizerSkin(LogVisualizerSkin.class);
    registerVisualizerSkin(LEDVisualizerSkin.class);
  }
  private ArrayList<VisualizerSkin> currentSkins = new ArrayList<VisualizerSkin>();

  /* Generic visualization */
  private MoteCountListener newMotesListener;
  private Observer posObserver = null;
  private Observer moteHighligtObserver = null;
  private Vector<Mote> highlightedMotes = new Vector<Mote>();
  private final static Color HIGHLIGHT_COLOR = Color.CYAN;
  private Observer moteRelationsObserver = null;

  /* Popup menu */
  public static interface SimulationMenuAction {
    public boolean isEnabled(Simulation simulation);
    public String getDescription(Simulation simulation);
    public void doAction(Visualizer visualizer, Simulation simulation);
  }

  public static interface MoteMenuAction {
    public boolean isEnabled(Mote mote);
    public String getDescription(Mote mote);
    public void doAction(Visualizer visualizer, Mote mote);
  }

  private ArrayList<Class<? extends SimulationMenuAction>> simulationMenuActions =
    new ArrayList<Class<? extends SimulationMenuAction>>();
  private ArrayList<Class<? extends MoteMenuAction>> moteMenuActions =
    new ArrayList<Class<? extends MoteMenuAction>>();

  public Visualizer(Simulation simulation, GUI gui) {
    super("Simulation Visualizer", gui);
    this.gui = gui;
    this.simulation = simulation;

    /* Main canvas */
    canvas = new JPanel() {
      private static final long serialVersionUID = 1L;
      public void paintComponent(Graphics g) {
        super.paintComponent(g);
        paintSkinGeneric(g);
        for (VisualizerSkin skin: currentSkins) {
          skin.paintSkin(g);
        }
      }
    };
    canvas.setBackground(Color.WHITE);
    calculateTransformations();

    /* Skin selector */
    skinButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        Point mouse = MouseInfo.getPointerInfo().getLocation();
        JCheckBoxMenuItem item;
        JPopupMenu skinPopupMenu = new JPopupMenu();

        for (Class<? extends VisualizerSkin> skinClass: visualizerSkins) {
          String description = GUI.getDescriptionOf(skinClass);
          item = new JCheckBoxMenuItem(description, false);
          item.putClientProperty("skinclass", skinClass);

          /* Select skin if active */
          for (VisualizerSkin skin: currentSkins) {
            if (skin.getClass() == skinClass) {
              item.setSelected(true);
              break;
            }
          }

          item.addItemListener(new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
              JCheckBoxMenuItem menuItem = ((JCheckBoxMenuItem)e.getItem());
              if (menuItem == null) {
                logger.fatal("No menu item");
                return;
              }

              Class<VisualizerSkin> skinClass =
                (Class<VisualizerSkin>) menuItem.getClientProperty("skinclass");
              if (skinClass == null) {
                logger.fatal("Unknown visualizer skin class: " + skinClass);
                return;
              }

              if (menuItem.isSelected()) {
                /* Create and activate new skin */
                generateAndActivateSkin(skinClass);
              } else {
                /* Deactivate skin */
                VisualizerSkin skinToDeactivate = null;
                for (VisualizerSkin skin: currentSkins) {
                  if (skin.getClass() == skinClass) {
                    skinToDeactivate = skin;
                    break;
                  }
                }
                if (skinToDeactivate == null) {
                  logger.fatal("Unknown visualizer skin to deactivate: " + skinClass);
                  return;
                }
                skinToDeactivate.setInactive();
                repaint();
                currentSkins.remove(skinToDeactivate);
                skinButton.setText("Select visualizer skins " +
                    "(" + currentSkins.size() + "/" + visualizerSkins.size() + ")");
              }
            }
          });

          skinPopupMenu.add(item);
        }

        skinPopupMenu.setLocation(mouse);
        skinPopupMenu.setInvoker(skinButton);
        skinPopupMenu.setVisible(true);
      }
    });
    this.add(BorderLayout.NORTH, skinButton);
    this.add(BorderLayout.CENTER, canvas);

    /* Observe simulation and mote positions */
    posObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        calculateTransformations();
        repaint();
      }
    };
    simulation.getEventCentral().addMoteCountListener(newMotesListener = new MoteCountListener() {
      public void moteWasAdded(Mote mote) {
        Position pos = mote.getInterfaces().getPosition();
        if (pos != null) {
          pos.addObserver(posObserver);
        }
        calculateTransformations();
        repaint();
      }
      public void moteWasRemoved(Mote mote) {
        Position pos = mote.getInterfaces().getPosition();
        if (pos != null) {
          pos.deleteObserver(posObserver);
        }
        calculateTransformations();
        repaint();
      }
    });
    for (Mote mote: simulation.getMotes()) {
      Position pos = mote.getInterfaces().getPosition();
      if (pos != null) {
        pos.addObserver(posObserver);
      }
    }
    calculateTransformations();
    repaint();

    /* Observe mote highlights */
    gui.addMoteHighlightObserver(moteHighligtObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (!(obj instanceof Mote)) {
          return;
        }

        final Timer timer = new Timer(100, null);
        final Mote mote = (Mote) obj;
        timer.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            /* Count down */
            if (timer.getDelay() < 90) {
              timer.stop();
              highlightedMotes.remove(mote);
              repaint();
              return;
            }

            /* Toggle highlight state */
            if (highlightedMotes.contains(mote)) {
              highlightedMotes.remove(mote);
            } else {
              highlightedMotes.add(mote);
            }
            timer.setDelay(timer.getDelay()-1);
            repaint();
          }
        });
        timer.start();
      }
    });

    /* Observe mote relations */
    gui.addMoteRelationsObserver(moteRelationsObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        repaint();
      }
    });

    /* Popup menu */
    canvas.addMouseMotionListener(new MouseMotionListener() {
      public void mouseMoved(MouseEvent e) {
        handleMoveRequest(e.getPoint().x, e.getPoint().y, false);
      }
      public void mouseDragged(MouseEvent e) {
        handleMoveRequest(e.getPoint().x, e.getPoint().y, false);
      }
    });
    canvas.addMouseListener(new MouseListener() {
      public void mousePressed(MouseEvent e) {
        if (e.isPopupTrigger()) {
          handlePopupRequest(e.getPoint().x, e.getPoint().y);
        } else if (SwingUtilities.isLeftMouseButton(e)){
          //handleMoveRequest(e.getPoint().x, e.getPoint().y, false);
          beginMoveRequest(e.getPoint().x, e.getPoint().y);
        }
      }
      public void mouseReleased(MouseEvent e) {
        if (e.isPopupTrigger()) {
          handlePopupRequest(e.getPoint().x, e.getPoint().y);
        } else {
          handleMoveRequest(e.getPoint().x, e.getPoint().y, true);
        }
      }
      public void mouseEntered(MouseEvent e) {
        if (e.isPopupTrigger()) {
          handlePopupRequest(e.getPoint().x, e.getPoint().y);
        }
      }
      public void mouseExited(MouseEvent e) {
        if (e.isPopupTrigger()) {
          handlePopupRequest(e.getPoint().x, e.getPoint().y);
        }
      }
      public void mouseClicked(MouseEvent e) {
        if (e.isPopupTrigger()) {
          handlePopupRequest(e.getPoint().x, e.getPoint().y);
        }
      }
    });

    addComponentListener(new ComponentAdapter() {
      public void componentResized(ComponentEvent ce) {
        calculateTransformations();
        repaint();
      }
    });

    /* Register mote menu actions */
    registerMoteMenuAction(MoveMoteMenuAction.class);
    registerMoteMenuAction(ButtonClickMoteMenuAction.class);
    registerMoteMenuAction(ShowLEDMoteMenuAction.class);
    registerMoteMenuAction(ShowSerialMoteMenuAction.class);
    registerMoteMenuAction(DeleteMoteMenuAction.class);

    /* Register simulation menu actions */
    /* ... */

    /* Drag and drop files to motes */
    DropTargetListener dTargetListener = new DropTargetListener() {
      public void dragEnter(DropTargetDragEvent dtde) {
        if (acceptOrRejectDrag(dtde)) {
          dtde.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
        } else {
          dtde.rejectDrag();
        }
      }
      public void dragExit(DropTargetEvent dte) {
      }
      public void dropActionChanged(DropTargetDragEvent dtde) {
        if (acceptOrRejectDrag(dtde)) {
          dtde.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
        } else {
          dtde.rejectDrag();
        }
      }
      public void dragOver(DropTargetDragEvent dtde) {
        if (acceptOrRejectDrag(dtde)) {
          dtde.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
        } else {
          dtde.rejectDrag();
        }
      }
      public void drop(DropTargetDropEvent dtde) {
        Transferable transferable = dtde.getTransferable();

        /* Only accept single files */
        File file = null;
        if (!transferable.isDataFlavorSupported(DataFlavor.javaFileListFlavor)) {
          dtde.rejectDrop();
          return;
        }

        dtde.acceptDrop(DnDConstants.ACTION_COPY_OR_MOVE);

        try {
          List<Object> transferList = Arrays.asList(
              transferable.getTransferData(DataFlavor.javaFileListFlavor)
          );
          if (transferList.size() != 1) {
            return;
          }
          List<File> list = (List<File>) transferList.get(0);
          if (list.size() != 1) {
            return;
          }
          file = list.get(0);
        }
        catch (Exception e) {
          return;
        }

        if (file == null || !file.exists()) {
          return;
        }

        handleDropFile(file, dtde.getLocation());
      }
      private boolean acceptOrRejectDrag(DropTargetDragEvent dtde) {
        Transferable transferable = dtde.getTransferable();

        /* Make sure one, and only one, mote exists under mouse pointer */
        Point point = dtde.getLocation();
        Mote[] motes = findMotesAtPosition(point.x, point.y);
        if (motes == null || motes.length != 1) {
          return false;
        }

        /* Only accept single files */
        File file;
        if (!transferable.isDataFlavorSupported(DataFlavor.javaFileListFlavor)) {
          return false;
        }
        try {
          List<Object> transferList = Arrays.asList(
              transferable.getTransferData(DataFlavor.javaFileListFlavor)
          );
          if (transferList.size() != 1) {
            return false;
          }
          List<File> list = (List<File>) transferList.get(0);
          if (list.size() != 1) {
            return false;
          }
          file = list.get(0);
        } catch (UnsupportedFlavorException e) {
          return false;
        } catch (IOException e) {
          return false;
        }

        /* Extract file extension */
        return isDropFileAccepted(file);
      }
    };
    canvas.setDropTarget(
        new DropTarget(canvas, DnDConstants.ACTION_COPY_OR_MOVE, dTargetListener, true, null)
    );

    this.setSize(300, 300);
    setLocation(gui.getDesktopPane().getWidth() - getWidth(), 0);
    setVisible(true);
  }

  private void generateAndActivateSkin(Class<? extends VisualizerSkin> skinClass) {
    for (VisualizerSkin skin: currentSkins) {
      if (skinClass == skin.getClass()) {
        logger.warn("Selected skin already active: " + skinClass);
        return;
      }
    }

    /* Create and activate new skin */
    try {
      VisualizerSkin newSkin = skinClass.newInstance();
      newSkin.setActive(Visualizer.this.simulation, Visualizer.this);
      currentSkins.add(newSkin);
    } catch (InstantiationException e1) {
      e1.printStackTrace();
    } catch (IllegalAccessException e1) {
      e1.printStackTrace();
    }

    skinButton.setText("Select visualizer skins " +
        "(" + currentSkins.size() + "/" + visualizerSkins.size() + ")");
    repaint();
  }

  public VisualizerSkin[] getCurrentSkins() {
    VisualizerSkin[] skins = new VisualizerSkin[currentSkins.size()];
    return currentSkins.toArray(skins);
  }

  /**
   * Register simulation menu action.
   *
   * @see SimulationMenuAction
   * @param menuAction Menu action
   */
  public void registerSimulationMenuAction(Class<? extends SimulationMenuAction> menuAction) {
    if (simulationMenuActions.contains(menuAction)) {
      return;
    }

    simulationMenuActions.add(menuAction);
  }

  public void unregisterSimulationMenuAction(Class<? extends SimulationMenuAction> menuAction) {
    simulationMenuActions.remove(menuAction);
  }

  /**
   * Register mote menu action.
   *
   * @see MoteMenuAction
   * @param menuAction Menu action
   */
  public void registerMoteMenuAction(Class<? extends MoteMenuAction> menuAction) {
    if (moteMenuActions.contains(menuAction)) {
      return;
    }

    moteMenuActions.add(menuAction);
  }

  public void unregisterMoteMenuAction(Class<? extends MoteMenuAction> menuAction) {
    moteMenuActions.remove(menuAction);
  }

  public static void registerVisualizerSkin(Class<? extends VisualizerSkin> skin) {
    if (visualizerSkins.contains(skin)) {
      return;
    }
    visualizerSkins.add(skin);
  }

  public static void unregisterVisualizerSkin(Class<? extends VisualizerSkin> skin) {
    visualizerSkins.remove(skin);
  }


  private void handlePopupRequest(final int x, final int y) {

    JPopupMenu menu = new JPopupMenu();
    menu.add(new JLabel("Select action:"));

    /* Mote specific actions */
    final Mote[] motes = findMotesAtPosition(x, y);
    if (motes != null && motes.length > 0) {
      menu.add(new JSeparator());

      /* Add registered mote actions */
      for (final Mote mote : motes) {
        menu.add(simulation.getGUI().createMotePluginsSubmenu(mote));
        for (Class<? extends MoteMenuAction> menuActionClass: moteMenuActions) {
          try {
            final MoteMenuAction menuAction = menuActionClass.newInstance();
            if (menuAction.isEnabled(mote)) {
              JMenuItem menuItem = new JMenuItem(menuAction.getDescription(mote));
              menuItem.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                  menuAction.doAction(Visualizer.this, mote);
                }
              });
              menu.add(menuItem);
            }
          } catch (InstantiationException e1) {
            logger.fatal("Error: " + e1.getMessage(), e1);
          } catch (IllegalAccessException e1) {
            logger.fatal("Error: " + e1.getMessage(), e1);
          }
        }
      }

    }

    /* Simulation specific actions */
    menu.add(new JSeparator());
    for (Class<? extends SimulationMenuAction> menuActionClass: simulationMenuActions) {
      try {
        final SimulationMenuAction menuAction = menuActionClass.newInstance();
        if (menuAction.isEnabled(simulation)) {
          JMenuItem menuItem = new JMenuItem(menuAction.getDescription(simulation));
          menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
              menuAction.doAction(Visualizer.this, simulation);
            }
          });
          menu.add(menuItem);
        }
      } catch (InstantiationException e1) {
        logger.fatal("Error: " + e1.getMessage(), e1);
      } catch (IllegalAccessException e1) {
        logger.fatal("Error: " + e1.getMessage(), e1);
      }
    }

    /* Show menu */
    menu.setLocation(new Point(
        canvas.getLocationOnScreen().x + x,
        canvas.getLocationOnScreen().y + y));
    menu.setInvoker(canvas);
    menu.setVisible(true);
  }

  private void beginMoveRequest(final int x, final int y) {
    final Mote[] motes = findMotesAtPosition(x, y);
    if (motes == null || motes.length == 0) {
      return;
    }

    moteMoveBeginTime = System.currentTimeMillis();
    beginMoveRequest(motes[0]);
  }

  private void beginMoveRequest(Mote moteToMove) {
    moteIsBeingMoved = true;
    this.moteToMove = moteToMove;
    repaint();
  }

  private void handleMoveRequest(final int x, final int y,
      boolean wasJustReleased) {

    if (!moteIsBeingMoved) {
      return;
    }

    if (!wasJustReleased) {
      // Still moving mote
      canvas.setCursor(moveCursor);
      return;
    }

    // Stopped moving mote
    canvas.setCursor(Cursor.getDefaultCursor());
    moteIsBeingMoved = false;

    Position newXYValues = transformPixelToPositon(new Point(x, y));

    if (moteMoveBeginTime <= 0 || System.currentTimeMillis() - moteMoveBeginTime > 300) {
      int returnValue = JOptionPane.showConfirmDialog(Visualizer.this, "Move mote to"
          + "\nX=" + newXYValues.getXCoordinate() + "\nY="
          + newXYValues.getYCoordinate() + "\nZ="
          + moteToMove.getInterfaces().getPosition().getZCoordinate());

      if (returnValue == JOptionPane.OK_OPTION) {
        moteToMove.getInterfaces().getPosition().setCoordinates(
            newXYValues.getXCoordinate(), newXYValues.getYCoordinate(),
            moteToMove.getInterfaces().getPosition().getZCoordinate());
      }
    }
    moteMoveBeginTime = -1;
    moteToMove = null;
    repaint();
  }

  /**
   * Returns all motes at given position.
   *
   * @param clickedX
   *          X coordinate
   * @param clickedY
   *          Y coordinate
   * @return All motes at given position
   */
  public Mote[] findMotesAtPosition(int clickedX, int clickedY) {
    double xCoord = factorXPixelToCoord(clickedX);
    double yCoord = factorYPixelToCoord(clickedY);

    ArrayList<Mote> motes = new ArrayList<Mote>();

    // Calculate painted mote radius in coordinates
    double paintedMoteWidth = factorXPixelToCoord(MOTE_RADIUS)
    - factorXPixelToCoord(0);
    double paintedMoteHeight = factorYPixelToCoord(MOTE_RADIUS)
    - factorYPixelToCoord(0);

    for (int i = 0; i < simulation.getMotesCount(); i++) {
      Position pos = simulation.getMote(i).getInterfaces().getPosition();

      // Transform to unit circle before checking if mouse hit this mote
      double distanceX = Math.abs(xCoord - pos.getXCoordinate())
      / paintedMoteWidth;
      double distanceY = Math.abs(yCoord - pos.getYCoordinate())
      / paintedMoteHeight;

      if (distanceX * distanceX + distanceY * distanceY <= 1) {
        motes.add(simulation.getMote(i));
      }
    }
    if (motes.size() == 0) {
      return null;
    }

    Mote[] motesArr = new Mote[motes.size()];
    return motes.toArray(motesArr);
  }

  public void paintSkinGeneric(Graphics g) {
    Mote[] allMotes = simulation.getMotes();
    for (Mote mote: allMotes) {

      /* Use the first skin's non-null mote colors */
      Color moteColors[] = null;
      for (VisualizerSkin skin: currentSkins) {
        moteColors = skin.getColorOf(mote);
        if (moteColors != null) {
          break;
        }
      }
      if (moteColors == null) {
        moteColors = DEFAULT_MOTE_COLORS;
      }

      Position motePos = mote.getInterfaces().getPosition();

      Point pixelCoord = transformPositionToPixel(motePos);
      int x = pixelCoord.x;
      int y = pixelCoord.y;

      if (!highlightedMotes.isEmpty() && highlightedMotes.contains(mote)) {
        g.setColor(HIGHLIGHT_COLOR);
        g.fillOval(x - MOTE_RADIUS, y - MOTE_RADIUS, 2 * MOTE_RADIUS,
            2 * MOTE_RADIUS);
      } else if (mote == moteToMove) {
        // Don't fill mote
      } else if (moteColors.length >= 2) {
        g.setColor(moteColors[0]);
        g.fillOval(x - MOTE_RADIUS, y - MOTE_RADIUS, 2 * MOTE_RADIUS,
            2 * MOTE_RADIUS);

        g.setColor(moteColors[1]);
        g.fillOval(x - MOTE_RADIUS / 2, y - MOTE_RADIUS / 2, MOTE_RADIUS,
            MOTE_RADIUS);

      } else if (moteColors.length >= 1) {
        g.setColor(moteColors[0]);
        g.fillOval(x - MOTE_RADIUS, y - MOTE_RADIUS, 2 * MOTE_RADIUS,
            2 * MOTE_RADIUS);
      }

      g.setColor(Color.BLACK);
      g.drawOval(x - MOTE_RADIUS, y - MOTE_RADIUS, 2 * MOTE_RADIUS,
          2 * MOTE_RADIUS);
    }

    /* Paint mote relations */
    MoteRelation[] relations = simulation.getGUI().getMoteRelations();
    for (MoteRelation r: relations) {
      Position sourcePos = r.source.getInterfaces().getPosition();
      Position destPos = r.dest.getInterfaces().getPosition();

      Point sourcePoint = transformPositionToPixel(sourcePos);
      Point destPoint = transformPositionToPixel(destPos);

      Point middlePoint = new Point(
          (destPoint.x*9 + sourcePoint.x*1)/10,
          (destPoint.y*9 + sourcePoint.y*1)/10
      );

      /* "Arrow body" is painted gray */
      g.setColor(Color.LIGHT_GRAY);
      g.drawLine(sourcePoint.x, sourcePoint.y, middlePoint.x, middlePoint.y);

      /* "Arrow head" is painted black */
      g.setColor(Color.BLACK);
      g.drawLine(middlePoint.x, middlePoint.y, destPoint.x, destPoint.y);
    }
  }


  /**
   * Recalculate size of canvas and factors for transforming between real and
   * pixel coordinates. This method is called every time this frame is resized
   * or created.
   */
  protected void calculateTransformations() {
    if (simulation.getMotesCount() == 0) {
      smallestXCoord = 0;
      smallestYCoord = 0;
      factorXCoordToPixel = 1;
      factorYCoordToPixel = 1;
      return;
    }

    double biggestXCoord, biggestYCoord;

    Position pos = simulation.getMote(0).getInterfaces().getPosition();
    smallestXCoord = biggestXCoord = pos.getXCoordinate();
    smallestYCoord = biggestYCoord = pos.getYCoordinate();

    // Get extreme coordinates
    Mote[] motes = simulation.getMotes();
    for (Mote mote: motes) {
      pos = mote.getInterfaces().getPosition();

      smallestXCoord = Math.min(smallestXCoord, pos.getXCoordinate());
      smallestYCoord = Math.min(smallestYCoord, pos.getYCoordinate());
      biggestXCoord = Math.max(biggestXCoord, pos.getXCoordinate());
      biggestYCoord = Math.max(biggestYCoord, pos.getYCoordinate());
    }

    if ((biggestXCoord - smallestXCoord) == 0) {
      factorXCoordToPixel = 1;
    } else {
      factorXCoordToPixel = ((double)
          canvas.getSize().width - 2
          * CANVAS_BORDER_WIDTH)
          / (biggestXCoord - smallestXCoord);
    }

    if ((biggestYCoord - smallestYCoord) == 0) {
      factorYCoordToPixel = 1;
    } else {
      factorYCoordToPixel = ((double)
          canvas.getSize().height - 2
          * CANVAS_BORDER_WIDTH)
          / (biggestYCoord - smallestYCoord);
    }
  }

  /**
   * Transforms a real-world position to a pixel which can be painted onto the
   * current sized canvas.
   *
   * @param pos
   *          Real-world position
   * @return Pixel coordinates
   */
  public Point transformPositionToPixel(Position pos) {
    return new Point(factorXCoordToPixel(pos.getXCoordinate()),
        factorYCoordToPixel(pos.getYCoordinate()));
  }

  /**
   * Transforms real-world coordinates to a pixel which can be painted onto the
   * current sized canvas.
   *
   * @param x Real world X
   * @param y Real world Y
   * @param z Real world Z (ignored)
   * @return Pixel coordinates
   */
  public Point transformPositionToPixel(double x, double y, double z) {
    return new Point(factorXCoordToPixel(x), factorYCoordToPixel(y));
  }

  /**
   * Transforms a pixel coordinate to a real-world. Z-value will always be 0.
   *
   * @param pixelPos
   *          On-screen pixel coordinate
   * @return Real world coordinate (z=0).
   */
  public Position transformPixelToPositon(Point pixelPos) {
    Position dummyPosition = new Position(null);
    dummyPosition.setCoordinates(factorXPixelToCoord(pixelPos.x),
        factorYPixelToCoord(pixelPos.y), 0.0);
    return dummyPosition;
  }

  /**
   * @return The current canvas to paint on
   */
  public JPanel getCurrentCanvas() {
    return canvas;
  }

  private int factorXCoordToPixel(double xCoordinate) {
    return (int) ((xCoordinate - smallestXCoord) * factorXCoordToPixel + CANVAS_BORDER_WIDTH);
  }
  private int factorYCoordToPixel(double yCoordinate) {
    return (int) ((yCoordinate - smallestYCoord) * factorYCoordToPixel) + CANVAS_BORDER_WIDTH;
  }
  private double factorXPixelToCoord(int xPixel) {
    return ((xPixel - CANVAS_BORDER_WIDTH) / factorXCoordToPixel) + smallestXCoord;
  }
  private double factorYPixelToCoord(int yPixel) {
    return ((yPixel - CANVAS_BORDER_WIDTH) / factorYCoordToPixel) + smallestYCoord;
  }

  public void closePlugin() {
    for (VisualizerSkin skin: currentSkins) {
      skin.setInactive();
    }
    currentSkins.clear();
    if (moteHighligtObserver != null) {
      gui.deleteMoteHighlightObserver(moteHighligtObserver);
    }
    if (moteRelationsObserver != null) {
      gui.deleteMoteRelationsObserver(moteRelationsObserver);
    }

    simulation.getEventCentral().removeMoteCountListener(newMotesListener);
    for (Mote mote: simulation.getMotes()) {
      Position pos = mote.getInterfaces().getPosition();
      if (pos != null) {
        pos.deleteObserver(posObserver);
      }
    }
  }

  protected boolean isDropFileAccepted(File file) {
    return true; /* TODO */
  }

  protected void handleDropFile(File file, Point point) {
    logger.fatal("Drag and drop not implemented: " + file);
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    for (VisualizerSkin skin: currentSkins) {
      element = new Element("skin");
      element.setText(GUI.getDescriptionOf(skin.getClass()));
      config.add(element);
    }

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("skin")) {
        String wanted = element.getText();
        for (Class<? extends VisualizerSkin> skinClass: visualizerSkins) {
          if (wanted.equals(GUI.getDescriptionOf(skinClass))) {
            final Class<? extends VisualizerSkin> skin = skinClass;
            SwingUtilities.invokeLater(new Runnable() {
              public void run() {
                generateAndActivateSkin(skin);
              }
            });
            wanted = null;
            break;
          }
        }
        if (wanted != null) {
          logger.warn("Could not load skin: " + element.getText());
        }
      }
    }
    return true;
  }

  protected static class ButtonClickMoteMenuAction implements MoteMenuAction {
    public boolean isEnabled(Mote mote) {
      return mote.getInterfaces().getButton() != null
      && !mote.getInterfaces().getButton().isPressed();
    }
    public String getDescription(Mote mote) {
      return "Click button on " + mote;
    }
    public void doAction(Visualizer visualizer, Mote mote) {
      mote.getInterfaces().getButton().clickButton();
    }
  };

  protected static class DeleteMoteMenuAction implements MoteMenuAction {
    public boolean isEnabled(Mote mote) {
      return true;
    }
    public String getDescription(Mote mote) {
      return "Delete " + mote;
    }
    public void doAction(Visualizer visualizer, Mote mote) {
      mote.getSimulation().removeMote(mote);
    }
  };

  protected static class ShowLEDMoteMenuAction implements MoteMenuAction {
    public boolean isEnabled(Mote mote) {
      return mote.getInterfaces().getLED() != null;
    }
    public String getDescription(Mote mote) {
      return "Show LEDs on " + mote;
    }
    public void doAction(Visualizer visualizer, Mote mote) {
      Simulation simulation = mote.getSimulation();
      LED led = mote.getInterfaces().getLED();
      if (led == null) {
        return;
      }

      /* Extract description (input to plugin) */
      String desc = GUI.getDescriptionOf(mote.getInterfaces().getLED());

      MoteInterfaceViewer viewer =
        (MoteInterfaceViewer) simulation.getGUI().tryStartPlugin(
            MoteInterfaceViewer.class,
            simulation.getGUI(),
            simulation,
            mote);
      if (viewer == null) {
        return;
      }
      viewer.setSelectedInterface(desc);
      viewer.pack();
    }
  };

  protected static class ShowSerialMoteMenuAction implements MoteMenuAction {
    public boolean isEnabled(Mote mote) {
      for (MoteInterface intf: mote.getInterfaces().getInterfaces()) {
        if (intf instanceof SerialPort) {
          return true;
        }
      }
      return false;
    }
    public String getDescription(Mote mote) {
      return "Show serial port on " + mote;
    }
    public void doAction(Visualizer visualizer, Mote mote) {
      Simulation simulation = mote.getSimulation();
      SerialPort serialPort = null;
      for (MoteInterface intf: mote.getInterfaces().getInterfaces()) {
        if (intf instanceof SerialPort) {
          serialPort = (SerialPort) intf;
          break;
        }
      }

      if (serialPort == null) {
        return;
      }

      /* Extract description (input to plugin) */
      String desc = GUI.getDescriptionOf(serialPort);

      MoteInterfaceViewer viewer =
        (MoteInterfaceViewer) simulation.getGUI().tryStartPlugin(
            MoteInterfaceViewer.class,
            simulation.getGUI(),
            simulation,
            mote);
      if (viewer == null) {
        return;
      }
      viewer.setSelectedInterface(desc);
      viewer.pack();
    }
  };

  protected static class MoveMoteMenuAction implements MoteMenuAction {
    public boolean isEnabled(Mote mote) {
      return true;
    }
    public String getDescription(Mote mote) {
      return "Move " + mote;
    }
    public void doAction(Visualizer visualizer, Mote mote) {
      visualizer.moteMoveBeginTime = -1;
      visualizer.beginMoveRequest(mote);
    }
  };
}
