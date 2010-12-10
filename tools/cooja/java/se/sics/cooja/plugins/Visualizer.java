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
 * $Id: Visualizer.java,v 1.19 2010/12/10 17:50:49 nifi Exp $
 */

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.MouseInfo;
import java.awt.Point;
import java.awt.Polygon;
import java.awt.RenderingHints;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.awt.dnd.DnDConstants;
import java.awt.dnd.DropTarget;
import java.awt.dnd.DropTargetDragEvent;
import java.awt.dnd.DropTargetDropEvent;
import java.awt.dnd.DropTargetEvent;
import java.awt.dnd.DropTargetListener;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.awt.geom.AffineTransform;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JButton;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JLabel;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JSeparator;
import javax.swing.SwingUtilities;
import javax.swing.Timer;
import javax.swing.plaf.basic.BasicInternalFrameUI;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.GUI.MoteRelation;
import se.sics.cooja.SimEventCentral.MoteCountListener;
import se.sics.cooja.interfaces.LED;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.SerialPort;
import se.sics.cooja.plugins.skins.AddressVisualizerSkin;
import se.sics.cooja.plugins.skins.AttributeVisualizerSkin;
import se.sics.cooja.plugins.skins.GridVisualizerSkin;
import se.sics.cooja.plugins.skins.IDVisualizerSkin;
import se.sics.cooja.plugins.skins.LEDVisualizerSkin;
import se.sics.cooja.plugins.skins.LogVisualizerSkin;
import se.sics.cooja.plugins.skins.MoteTypeVisualizerSkin;
import se.sics.cooja.plugins.skins.PositionVisualizerSkin;
import se.sics.cooja.plugins.skins.TrafficVisualizerSkin;
import se.sics.cooja.plugins.skins.UDGMVisualizerSkin;

/**
 * Simulation visualizer supporting visualization skins.
 * Motes are painted in the XY-plane, as seen from positive Z axis.
 *
 * Supports drag-n-drop motes, right-click popup menu, and visualization skins.
 *
 * Observes the simulation and all mote positions.
 *
 * @see #registerMoteMenuAction(Class)
 * @see #registerSimulationMenuAction(Class)
 * @see #registerVisualizerSkin(Class)
 * @see UDGMVisualizerSkin
 * @author Fredrik Osterlind
 */
@ClassDescription("Simulation visualizer")
@PluginType(PluginType.SIM_STANDARD_PLUGIN)
public class Visualizer extends VisPlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(Visualizer.class);

  public static final int MOTE_RADIUS = 8;
  private static final Color[] DEFAULT_MOTE_COLORS = { Color.WHITE };

  private GUI gui = null;
  private Simulation simulation = null;
  private final JPanel canvas;

  /* Viewport */
  private AffineTransform viewportTransform;
  public int resetViewport = 0;

  /* Actions: move motes, pan view, and zoom view */
  private boolean panning = false;
  private Position panningPosition = null; /* Panning start position */
  private boolean zooming = false;
  private double zoomStart = 0;
  private Position zoomingPosition = null; /* Zooming center position */
  private Point zoomingPixel = null; /* Zooming center pixel */
  private boolean moving = false;
  private Mote movedMote = null;
  public Mote clickedMote = null;
  private long moveStartTime = -1;
  private boolean moveConfirm;
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
    registerVisualizerSkin(TrafficVisualizerSkin.class);
    registerVisualizerSkin(PositionVisualizerSkin.class);
    registerVisualizerSkin(GridVisualizerSkin.class);
    registerVisualizerSkin(MoteTypeVisualizerSkin.class);
    registerVisualizerSkin(AttributeVisualizerSkin.class);
  }
  private ArrayList<VisualizerSkin> currentSkins = new ArrayList<VisualizerSkin>();

  /* Generic visualization */
  private MoteCountListener newMotesListener;
  private Observer posObserver = null;
  private Observer moteHighligtObserver = null;
  private ArrayList<Mote> highlightedMotes = new ArrayList<Mote>();
  private final static Color HIGHLIGHT_COLOR = Color.CYAN;
  private final static Color MOVE_COLOR = Color.WHITE;
  private Observer moteRelationsObserver = null;

  /* Popup menu */
  public static interface SimulationMenuAction {
    public boolean isEnabled(Visualizer visualizer, Simulation simulation);
    public String getDescription(Visualizer visualizer, Simulation simulation);
    public void doAction(Visualizer visualizer, Simulation simulation);
  }
  public static interface MoteMenuAction {
    public boolean isEnabled(Visualizer visualizer, Mote mote);
    public String getDescription(Visualizer visualizer, Mote mote);
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

    /* Register external skins */
    String[] skins = gui.getProjectConfig().getStringArrayValue(Visualizer.class, "SKINS");
    if (skins != null) {
      for (String skinClass: skins) {
        logger.info("Registering external visualizer skin: " + skinClass);
        Class<? extends VisualizerSkin> skin = gui.tryLoadClass(this, VisualizerSkin.class, skinClass);
        registerVisualizerSkin(skin);
      }
    }
    
    /* Main canvas */
    canvas = new JPanel() {
      private static final long serialVersionUID = 1L;
      public void paintComponent(Graphics g) {
        super.paintComponent(g);

        if (resetViewport > 0) {
          resetViewport();
          resetViewport--;
        }

        ((Graphics2D)g).setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        for (VisualizerSkin skin: currentSkins) {
          skin.paintBeforeMotes(g);
        }
        paintMotes(g);
        for (VisualizerSkin skin: currentSkins) {
          skin.paintAfterMotes(g);
        }
      }
    };
    canvas.setBackground(Color.WHITE);
    viewportTransform = new AffineTransform();

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
        repaint();
      }
    };
    simulation.getEventCentral().addMoteCountListener(newMotesListener = new MoteCountListener() {
      public void moteWasAdded(Mote mote) {
        Position pos = mote.getInterfaces().getPosition();
        if (pos != null) {
          pos.addObserver(posObserver);
          SwingUtilities.invokeLater(new Runnable() {
            public void run() {
              resetViewport = 1;
              repaint();
            }
          });
        }
      }
      public void moteWasRemoved(Mote mote) {
        Position pos = mote.getInterfaces().getPosition();
        if (pos != null) {
          pos.deleteObserver(posObserver);
          repaint();
        }
      }
    });
    for (Mote mote: simulation.getMotes()) {
      Position pos = mote.getInterfaces().getPosition();
      if (pos != null) {
        pos.addObserver(posObserver);
      }
    }

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
        handleMouseMove(e, false);
      }
      public void mouseDragged(MouseEvent e) {
        handleMouseMove(e, false);
      }
    });
    canvas.addMouseListener(new MouseAdapter() {
      public void mousePressed(MouseEvent e) {
        if (e.isPopupTrigger()) {
          handlePopupRequest(e.getPoint().x, e.getPoint().y);
          return;
        } 

        if (SwingUtilities.isLeftMouseButton(e)){
          handleMousePress(e);
        }
      }
      public void mouseReleased(MouseEvent e) {
        if (e.isPopupTrigger()) {
          handlePopupRequest(e.getPoint().x, e.getPoint().y);
          return;
        }

        handleMouseMove(e, true);
      }
      public void mouseClicked(MouseEvent e) {
        if (e.isPopupTrigger()) {
          handlePopupRequest(e.getPoint().x, e.getPoint().y);
        }

        handleMouseMove(e, true);
      }
    });

    /* Register mote menu actions */
    registerMoteMenuAction(MoveMoteMenuAction.class);
    registerMoteMenuAction(ButtonClickMoteMenuAction.class);
    registerMoteMenuAction(ShowLEDMoteMenuAction.class);
    registerMoteMenuAction(ShowSerialMoteMenuAction.class);
    registerMoteMenuAction(DeleteMoteMenuAction.class);

    /* Register simulation menu actions */
    registerSimulationMenuAction(ResetViewportAction.class);
    registerSimulationMenuAction(ToggleDecorationsMenuAction.class);

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
    resetViewport = 3; /* XXX Quick-fix */
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
            if (menuAction.isEnabled(this, mote)) {
              JMenuItem menuItem = new JMenuItem(menuAction.getDescription(this, mote));
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
        if (menuAction.isEnabled(this, simulation)) {
          JMenuItem menuItem = new JMenuItem(menuAction.getDescription(this, simulation));
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

  private void handleMousePress(MouseEvent mouseEvent) {
    int x = mouseEvent.getX();
    int y = mouseEvent.getY();
  	clickedMote = null;

    if (mouseEvent.isControlDown()) {
      /* Zoom */
      zooming = true;
      zoomingPixel = new Point(x, y);
      zoomingPosition = transformPixelToPosition(zoomingPixel);
      zoomStart = viewportTransform.getScaleX();
      return;
    }

    final Mote[] motes = findMotesAtPosition(x, y);
    if (mouseEvent.isShiftDown() ||
            (!mouseEvent.isAltDown() && (motes == null || motes.length == 0))) {
      /* No motes clicked or shift pressed: We should pan */
      panning = true;
      panningPosition = transformPixelToPosition(x, y);
      return;
    }

    if (motes != null && motes.length > 0) {
      /* One of the clicked motes should be moved */
    	clickedMote = motes[0];
      beginMoveRequest(motes[0], !mouseEvent.isAltDown(), !mouseEvent.isAltDown());
    }
  }

  private void beginMoveRequest(Mote moteToMove, boolean withTiming, boolean confirm) {
    if (withTiming) {
      moveStartTime = System.currentTimeMillis();
    } else {
      moveStartTime = -1;
    }
    moving = true;
    moveConfirm = confirm;
    movedMote = moteToMove;
    repaint();
  }

  private void handleMouseMove(MouseEvent e, boolean stop) {
    int x = e.getX();
    int y = e.getY();

    /* Panning */
    if (panning) {
      if (panningPosition == null || stop) {
        panning = false;
        return;
      }

      /* The current mouse position should correspond to where panning started */
      Position moved = transformPixelToPosition(x,y);
      viewportTransform.translate(
          moved.getXCoordinate() - panningPosition.getXCoordinate(), 
          moved.getYCoordinate() - panningPosition.getYCoordinate()
      );
      repaint();
      return;
    }

    /* Zooming */
    if (zooming) {
      if (zoomingPosition == null || zoomingPixel == null || stop) {
        zooming = false;
        return;
      }

      /* The zooming start pixel should correspond to the zooming center position */
      /* The current mouse position should correspond to where panning started */
      double zoomFactor = 1.0 + Math.abs((double) zoomingPixel.y - y)/100.0; 
      double newZoom = (zoomingPixel.y - y)>0?zoomStart*zoomFactor: zoomStart/zoomFactor;
      if (newZoom < 0.00001) {
        newZoom = 0.00001;
      }
      viewportTransform.setToScale(
          newZoom,
          newZoom
      );
      Position moved = transformPixelToPosition(zoomingPixel);
      viewportTransform.translate(
          moved.getXCoordinate() - zoomingPosition.getXCoordinate(), 
          moved.getYCoordinate() - zoomingPosition.getYCoordinate()
      );
      repaint();
      return;
    }

    /* Moving */
    if (moving) {
      if (!stop) {
        canvas.setCursor(moveCursor);
        return;
      }

      /* Restore cursor */
      canvas.setCursor(Cursor.getDefaultCursor());

      /* Move mote */
      if (moveStartTime < 0 || System.currentTimeMillis() - moveStartTime > 300) {
        Position newPos = transformPixelToPosition(x, y);
        if (moveConfirm) {
            String options[] = {"Yes", "Cancel"};
            int returnValue = JOptionPane.showOptionDialog(Visualizer.this,
                    "Move mote to" +
                    "\nX=" + newPos.getXCoordinate() + 
                    "\nY=" + newPos.getYCoordinate() + 
                    "\nZ=" + movedMote.getInterfaces().getPosition().getZCoordinate(),
                    "Move mote?",
                    JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE,
                    null, options, options[0]);
            moving = returnValue == JOptionPane.YES_OPTION;
        }
        if (moving) {
          movedMote.getInterfaces().getPosition().setCoordinates(
              newPos.getXCoordinate(),
              newPos.getYCoordinate(),
              movedMote.getInterfaces().getPosition().getZCoordinate()
          );
          repaint();
        }
      }

      moving = false;
      movedMote = null;
      repaint();
    }
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
    double xCoord = transformToPositionX(clickedX);
    double yCoord = transformToPositionY(clickedY);

    ArrayList<Mote> motes = new ArrayList<Mote>();

    // Calculate painted mote radius in coordinates
    double paintedMoteWidth = transformToPositionX(MOTE_RADIUS)
    - transformToPositionX(0);
    double paintedMoteHeight = transformToPositionY(MOTE_RADIUS)
    - transformToPositionY(0);

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

  public void paintMotes(Graphics g) {
    Mote[] allMotes = simulation.getMotes();

    /* Paint mote relations */
    MoteRelation[] relations = simulation.getGUI().getMoteRelations();
    for (MoteRelation r: relations) {
      Position sourcePos = r.source.getInterfaces().getPosition();
      Position destPos = r.dest.getInterfaces().getPosition();

      Point sourcePoint = transformPositionToPixel(sourcePos);
      Point destPoint = transformPositionToPixel(destPos);

      g.setColor(r.color == null ? Color.black : r.color);
      drawArrow(g, sourcePoint.x, sourcePoint.y, destPoint.x, destPoint.y, MOTE_RADIUS + 1);
    }

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

      if (mote == movedMote) {
        g.setColor(MOVE_COLOR);
        g.fillOval(x - MOTE_RADIUS, y - MOTE_RADIUS, 2 * MOTE_RADIUS,
            2 * MOTE_RADIUS);
      } else if (!highlightedMotes.isEmpty() && highlightedMotes.contains(mote)) {
        g.setColor(HIGHLIGHT_COLOR);
        g.fillOval(x - MOTE_RADIUS, y - MOTE_RADIUS, 2 * MOTE_RADIUS,
            2 * MOTE_RADIUS);
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
  }

  private Polygon arrowPoly = new Polygon();
  private void drawArrow(Graphics g, int xSource, int ySource, int xDest, int yDest, int delta) {
    double dx = xSource - xDest;
    double dy = ySource - yDest;
    double dir = Math.atan2(dx, dy);
    double len = Math.sqrt(dx * dx + dy * dy);
    dx /= len;
    dy /= len;
    len -= delta;
    xDest = xSource - (int) (dx * len);
    yDest = ySource - (int) (dy * len);
    g.drawLine(xDest, yDest, xSource, ySource);

    final int size = 8;
    arrowPoly.reset();
    arrowPoly.addPoint(xDest, yDest);
    arrowPoly.addPoint(xDest + xCor(size, dir + 0.5), yDest + yCor(size, dir + 0.5));
    arrowPoly.addPoint(xDest + xCor(size, dir - 0.5), yDest + yCor(size, dir - 0.5));
    arrowPoly.addPoint(xDest, yDest);
    g.fillPolygon(arrowPoly);
  }

  private int yCor(int len, double dir) {
    return (int)(0.5 + len * Math.cos(dir));
  }

  private int xCor(int len, double dir) {
    return (int)(0.5 + len * Math.sin(dir));
  }

  /**
   * Reset transform to show all motes.
   */
  protected void resetViewport() {
    Mote[] motes = simulation.getMotes();
    if (motes.length == 0) {
      /* No motes */
      viewportTransform.setToIdentity();
      return;
    }

    final double BORDER_SCALE_FACTOR = 1.1;
    double smallX, bigX, smallY, bigY, scaleX, scaleY;

    /* Init values */
    {
      Position pos = motes[0].getInterfaces().getPosition();
      smallX = bigX = pos.getXCoordinate();
      smallY = bigY = pos.getYCoordinate();
    }

    /* Extremes */
    for (Mote mote: motes) {
      Position pos = mote.getInterfaces().getPosition();
      smallX = Math.min(smallX, pos.getXCoordinate());
      bigX = Math.max(bigX, pos.getXCoordinate());
      smallY = Math.min(smallY, pos.getYCoordinate());
      bigY = Math.max(bigY, pos.getYCoordinate());
    }

    /* Scale viewport */
    if (smallX == bigX) {
      scaleX = 1;
    } else {
      scaleX = (bigX - smallX) / (double) (canvas.getWidth());
    }
    if (smallY == bigY) {
      scaleY = 1;
    } else {
      scaleY = (bigY - smallY) / (double) (canvas.getHeight());
    }

    viewportTransform.setToIdentity();
    double newZoom = (1.0/(BORDER_SCALE_FACTOR*Math.max(scaleX, scaleY)));
    viewportTransform.setToScale(
        newZoom,
        newZoom
    );

    /* Center visible motes */
    final double smallXfinal = smallX, bigXfinal = bigX, smallYfinal = smallY, bigYfinal = bigY;
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        Position viewMid =
          transformPixelToPosition(canvas.getWidth()/2, canvas.getHeight()/2);
        double motesMidX = (smallXfinal + bigXfinal) / 2.0;
        double motesMidY = (smallYfinal + bigYfinal) / 2.0;

        viewportTransform.translate(
            viewMid.getXCoordinate() - motesMidX, 
            viewMid.getYCoordinate() - motesMidY);
        canvas.repaint();
      }
    });
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
    return transformPositionToPixel(
        pos.getXCoordinate(), 
        pos.getYCoordinate(), 
        pos.getZCoordinate()
    );
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
    return new Point(transformToPixelX(x), transformToPixelY(y));
  }

  /**
   * @return Canvas
   */
  public JPanel getCurrentCanvas() {
    return canvas;
  }

  /**
   * Transforms a pixel coordinate to a real-world. Z-value will always be 0.
   *
   * @param pixelPos
   *          On-screen pixel coordinate
   * @return Real world coordinate (z=0).
   */
  public Position transformPixelToPosition(Point pixelPos) {
    return transformPixelToPosition(pixelPos.x, pixelPos.y);
  }
  public Position transformPixelToPosition(int x, int y) {
    Position position = new Position(null);
    position.setCoordinates(
        transformToPositionX(x),
        transformToPositionY(y), 
        0.0
    );
    return position;
  }

  private int transformToPixelX(double x) {
    return (int) (viewportTransform.getScaleX()*x + viewportTransform.getTranslateX());
  }
  private int transformToPixelY(double y) {
    return (int) (viewportTransform.getScaleY()*y + viewportTransform.getTranslateY());
  }
  private double transformToPositionX(int x) {
    return (x - viewportTransform.getTranslateX())/viewportTransform.getScaleX() ;
  }
  private double transformToPositionY(int y) {
    return (y - viewportTransform.getTranslateY())/viewportTransform.getScaleY() ;
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
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    /* Skins */
    for (VisualizerSkin skin: currentSkins) {
      element = new Element("skin");
      element.setText(skin.getClass().getName());
      config.add(element);
    }

    /* Viewport */
    element = new Element("viewport");
    double[] matrix = new double[6];
    viewportTransform.getMatrix(matrix);
    element.setText(
        matrix[0] + " " + 
        matrix[1] + " " + 
        matrix[2] + " " + 
        matrix[3] + " " + 
        matrix[4] + " " + 
        matrix[5]
    );
    config.add(element);

    /* Hide decorations */
    BasicInternalFrameUI ui = (BasicInternalFrameUI) getUI();
    if (ui.getNorthPane().getPreferredSize() == null ||
        ui.getNorthPane().getPreferredSize().height == 0) {
      element = new Element("hidden");
      config.add(element);
    }
    
    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("skin")) {
        String wanted = element.getText();
        for (Class<? extends VisualizerSkin> skinClass: visualizerSkins) {
          if (wanted.equals(skinClass.getName())
              /* Backwards compatibility */ 
              || wanted.equals(GUI.getDescriptionOf(skinClass))) {
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
      } else if (element.getName().equals("viewport")) {
        try {
          String[] matrix = element.getText().split(" ");
          viewportTransform.setTransform(
              Double.parseDouble(matrix[0]),
              Double.parseDouble(matrix[1]),
              Double.parseDouble(matrix[2]),
              Double.parseDouble(matrix[3]),
              Double.parseDouble(matrix[4]),
              Double.parseDouble(matrix[5])
          );
          resetViewport = 0;
        } catch (Exception e) {
          logger.warn("Bad viewport: " + e.getMessage());
          resetViewport();
        }
      } else if (element.getName().equals("hidden")) {
        BasicInternalFrameUI ui = (BasicInternalFrameUI) getUI();
        ui.getNorthPane().setPreferredSize(new Dimension(0,0));
        skinButton.setVisible(false);
      }
    }
    return true;
  }

  protected static class ButtonClickMoteMenuAction implements MoteMenuAction {
    public boolean isEnabled(Visualizer visualizer, Mote mote) {
      return mote.getInterfaces().getButton() != null
      && !mote.getInterfaces().getButton().isPressed();
    }
    public String getDescription(Visualizer visualizer, Mote mote) {
      return "Click button on " + mote;
    }
    public void doAction(Visualizer visualizer, Mote mote) {
      mote.getInterfaces().getButton().clickButton();
    }
  };

  protected static class DeleteMoteMenuAction implements MoteMenuAction {
    public boolean isEnabled(Visualizer visualizer, Mote mote) {
      return true;
    }
    public String getDescription(Visualizer visualizer, Mote mote) {
      return "Delete " + mote;
    }
    public void doAction(Visualizer visualizer, Mote mote) {
      mote.getSimulation().removeMote(mote);
    }
  };

  protected static class ShowLEDMoteMenuAction implements MoteMenuAction {
    public boolean isEnabled(Visualizer visualizer, Mote mote) {
      return mote.getInterfaces().getLED() != null;
    }
    public String getDescription(Visualizer visualizer, Mote mote) {
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
    public boolean isEnabled(Visualizer visualizer, Mote mote) {
      for (MoteInterface intf: mote.getInterfaces().getInterfaces()) {
        if (intf instanceof SerialPort) {
          return true;
        }
      }
      return false;
    }
    public String getDescription(Visualizer visualizer, Mote mote) {
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
    public boolean isEnabled(Visualizer visualizer, Mote mote) {
      return true;
    }
    public String getDescription(Visualizer visualizer, Mote mote) {
      return "Move " + mote;
    }
    public void doAction(Visualizer visualizer, Mote mote) {
      visualizer.beginMoveRequest(mote, false, true);
    }
  };

  protected static class ResetViewportAction implements SimulationMenuAction {
    public void doAction(Visualizer visualizer, Simulation simulation) {
      visualizer.resetViewport = 1;
      visualizer.repaint();
    }
    public String getDescription(Visualizer visualizer, Simulation simulation) {
      return "Reset viewport";
    }
    public boolean isEnabled(Visualizer visualizer, Simulation simulation) {
      return true;
    }
  };

  protected static class ToggleDecorationsMenuAction implements SimulationMenuAction {
    public void doAction(final Visualizer visualizer, Simulation simulation) {
      if (!(visualizer.getUI() instanceof BasicInternalFrameUI)) {
        return;
      }
      BasicInternalFrameUI ui = (BasicInternalFrameUI) visualizer.getUI();

      if (ui.getNorthPane().getPreferredSize() == null ||
          ui.getNorthPane().getPreferredSize().height == 0) {
        /* Restore window decorations */
        ui.getNorthPane().setPreferredSize(null);
        visualizer.skinButton.setVisible(true);
      } else {
        /* Hide window decorations */     
        ui.getNorthPane().setPreferredSize(new Dimension(0,0));
        visualizer.skinButton.setVisible(false);
      }
      visualizer.revalidate();
      SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          visualizer.repaint();
        }
      });
    }
    public String getDescription(Visualizer visualizer, Simulation simulation) {
      if (!(visualizer.getUI() instanceof BasicInternalFrameUI)) {
        return "Hide window decorations";
      }
      BasicInternalFrameUI ui = (BasicInternalFrameUI) visualizer.getUI();

      if (ui.getNorthPane().getPreferredSize() == null ||
          ui.getNorthPane().getPreferredSize().height == 0) {
        return "Restore window decorations";
      }
      return "Hide window decorations";
    }
    public boolean isEnabled(Visualizer visualizer, Simulation simulation) {
      if (!(visualizer.getUI() instanceof BasicInternalFrameUI)) {
        return false;
      }
      return true;
    }
  };
}
