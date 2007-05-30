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
 * $Id: Visualizer2D.java,v 1.11 2007/05/30 20:57:58 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import javax.swing.Timer;

import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;

/**
 * Visualizer2D is an abstract mote visualizer for simulations. All motes are
 * painted in the XY-plane, as seen from positive Z axis.
 * 
 * An implementation of this class must colorize the different motes, each mote
 * has two different colors; inner and outer.
 * 
 * By right-clicking the mouse on a mote a popup menu will be displayed. From
 * this menu mote plugins can be started. or the mote can be moved. Each
 * implementation may also register its own actions to be accessed from this
 * menu.
 * 
 * A Visualizer2D observers both the simulation and all mote positions.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("2D Mote Visualizer")
@PluginType(PluginType.SIM_PLUGIN)
public abstract class Visualizer2D extends VisPlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(Visualizer2D.class);

  private double factorXCoordToPixel;
  private double factorYCoordToPixel;
  private double smallestXCoord;
  private double smallestYCoord;

  private GUI myGUI = null;
  private Simulation simulation = null;
  private final JPanel canvas;
  private Visualizer2D myPlugin;

  private static final int CANVAS_BORDER_WIDTH = 25;
  private static final int MOTE_RADIUS = 8;

  private boolean moteIsBeingMoved = false;
  private long moteMoveBeginTime = -1;
  private Mote moteToMove = null;
  private Cursor moveCursor = new Cursor(Cursor.MOVE_CURSOR);

  private Observer simObserver = null; // Watches simulation changes
  private Observer posObserver = null; // Watches position changes

  private Observer moteHighligtObserver = null;
  private Mote highlightedMote = null;
  private Color highlightColor = Color.GRAY;
  private Timer highlightTimer = null;
  
  public interface MoteMenuAction {
    public boolean isEnabled(Mote mote);
    public String getDescription(Mote mote);
    public void doAction(Mote mote);
  }

  private class MoveMoteMenuAction implements MoteMenuAction {
    public boolean isEnabled(Mote mote) {
      return true;
    }
    public String getDescription(Mote mote) {
      return "Move " + mote;
    }
    public void doAction(Mote mote) {
      moteMoveBeginTime = -1;
      beginMoveRequest(mote);
    }
  };

  private class ButtonClickMoteMenuAction implements MoteMenuAction {
    public boolean isEnabled(Mote mote) {
      return mote.getInterfaces().getButton() != null
      && !mote.getInterfaces().getButton().isPressed();
    }
    public String getDescription(Mote mote) {
      return "Click button on " + mote;
    }
    public void doAction(Mote mote) {
      mote.getInterfaces().getButton().clickButton();
    }
  };

  private Vector<MoteMenuAction> menuActions = new Vector<MoteMenuAction>();

  /**
   * Registers as an simulation observer and initializes the canvas.
   * 
   * @param simulationToVisualize
   *          Simulation to visualize
   */
  public Visualizer2D(Simulation simulationToVisualize, GUI gui) {
    super("Visualizer2D", gui);

    myGUI = gui;
    myPlugin = this;

    // Set initial bounds of frame
    this.setBounds(150, 150, 300, 300);
    setVisible(true);

    simulation = simulationToVisualize;

    // Create "canvas" to paint on
    canvas = new JPanel() {
      private static final long serialVersionUID = 1L;
      public void paintComponent(Graphics g) {
        super.paintComponent(g);
        visualizeSimulation(g);
      }
    };
    canvas.setPreferredSize(new Dimension(getSize().width - 16,
        getSize().height - 38));
    canvas.setBorder(BorderFactory.createLineBorder(Color.GREEN, 2));
    canvas.setBackground(Color.WHITE);
    calculateTransformations();

    this.setContentPane(canvas);

    // Detect general simulation changes
    posObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        calculateTransformations();
        canvas.repaint();
      }
    };
    simulation.addObserver(simObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        canvas.setPreferredSize(new Dimension(getSize().width - 16,
            getSize().height - 38));

        // Register (or reregister) as observer on all mote positions
        for (int i = 0; i < simulation.getMotesCount(); i++) {
          Position posIntf = simulation.getMote(i).getInterfaces()
              .getPosition();
          if (posIntf != null) {
            posIntf.addObserver(posObserver);
          }
        }

        calculateTransformations();
        canvas.repaint();
      }
    });
    simObserver.update(null, null);

    // Detect mote highligts
    myGUI.addMoteHighligtObserver(moteHighligtObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (!(obj instanceof Mote))
          return;
        
        if (highlightTimer != null && highlightTimer.isRunning())
          highlightTimer.stop();
        
        highlightTimer = new Timer(100, null);
        highlightedMote = (Mote) obj;
        highlightTimer.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            // Decrease delay
            if (highlightTimer.getDelay() < 90) {
              highlightTimer.stop();
              highlightedMote = null;
              repaint();
              return;
            }

            // Toggle color
            if (highlightColor == Color.GRAY)
              highlightColor = Color.CYAN;
            else
              highlightColor = Color.GRAY; 
            highlightTimer.setDelay(highlightTimer.getDelay()-1);
            repaint();
          }
        });
        highlightTimer.start();          
      }
    });

    canvas.addMouseMotionListener(new MouseMotionListener() {
      public void mouseMoved(MouseEvent e) {
        myPlugin.handleMoveRequest(e.getPoint().x, e.getPoint().y, false);
      }
      public void mouseDragged(MouseEvent e) {
        myPlugin.handleMoveRequest(e.getPoint().x, e.getPoint().y, false);
      }
    });

    // Detect mouse events
    canvas.addMouseListener(new MouseListener() {
      public void mousePressed(MouseEvent e) {
        if (e.isPopupTrigger())
          myPlugin.handlePopupRequest(e.getPoint().x, e.getPoint().y);
        else if (SwingUtilities.isLeftMouseButton(e)){
          //myPlugin.handleMoveRequest(e.getPoint().x, e.getPoint().y, false);
          beginMoveRequest(e.getPoint().x, e.getPoint().y);
        }
      }
      public void mouseReleased(MouseEvent e) {
        if (e.isPopupTrigger())
          myPlugin.handlePopupRequest(e.getPoint().x, e.getPoint().y);
        else {
          myPlugin.handleMoveRequest(e.getPoint().x, e.getPoint().y, true);
        }
      }
      public void mouseEntered(MouseEvent e) {
        if (e.isPopupTrigger())
          myPlugin.handlePopupRequest(e.getPoint().x, e.getPoint().y);
      }
      public void mouseExited(MouseEvent e) {
        if (e.isPopupTrigger())
          myPlugin.handlePopupRequest(e.getPoint().x, e.getPoint().y);
      }
      public void mouseClicked(MouseEvent e) {
        if (e.isPopupTrigger())
          myPlugin.handlePopupRequest(e.getPoint().x, e.getPoint().y);
      }
    });

    // Detect component events
    addComponentListener(new ComponentListener() {
      public void componentMoved(ComponentEvent ce) {
        // NOP
      }
      public void componentShown(ComponentEvent ce) {
        // NOP
      }
      public void componentHidden(ComponentEvent ce) {
        // NOP
      }
      public void componentResized(ComponentEvent ce) {
        canvas.setPreferredSize(new Dimension(getSize().width - 16,
            getSize().height - 38));
        calculateTransformations();
        canvas.repaint();
      }
    });

    // Add menu action for moving motes
    addMoteMenuAction(new MoveMoteMenuAction());

    // Add menu action for clicking mote button
    addMoteMenuAction(new ButtonClickMoteMenuAction());
    
    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
  }

  /**
   * Add new mote menu action.
   * 
   * @see MoteMenuAction
   * @param menuAction Menu action
   */
  public void addMoteMenuAction(MoteMenuAction menuAction) {
    menuActions.add(menuAction);
  }

  private void handlePopupRequest(final int x, final int y) {
    final Vector<Mote> foundMotes = findMotesAtPosition(x, y);
    if (foundMotes == null || foundMotes.size() == 0)
      return;

    JPopupMenu pickMoteMenu = new JPopupMenu();
    pickMoteMenu.add(new JLabel("Select action:"));
    pickMoteMenu.add(new JSeparator());

    // Add 'show mote plugins'-actions
    for (final Mote mote : foundMotes) {
      final Point pos = new Point(canvas.getLocationOnScreen().x + x, canvas
          .getLocationOnScreen().y
          + y);

      pickMoteMenu.add(simulation.getGUI().createMotePluginsSubmenu(mote));
    }

    // Add the rest of the actions
    for (final MoteMenuAction menuAction : menuActions) {
      for (final Mote mote : foundMotes) {
        if (menuAction.isEnabled(mote)) {
          JMenuItem menuItem = new JMenuItem(menuAction.getDescription(mote));
          menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
              menuAction.doAction(mote);
            }
          });
          pickMoteMenu.add(menuItem);
        }
      }
    }

    // Show menu
    Point pos = new Point(canvas.getLocationOnScreen().x + x, canvas
        .getLocationOnScreen().y
        + y);
    pickMoteMenu.setLocation(pos.x, pos.y);
    pickMoteMenu.setInvoker(canvas);
    pickMoteMenu.setVisible(true);
  }

  private void beginMoveRequest(final int x, final int y) {
    final Vector<Mote> foundMotes = findMotesAtPosition(x, y);
    if (foundMotes == null || foundMotes.size() == 0)
      return;

    moteMoveBeginTime = System.currentTimeMillis();
    beginMoveRequest(foundMotes.get(0));
  }

  private void beginMoveRequest(Mote moteToMove) {
    moteIsBeingMoved = true;
    this.moteToMove = moteToMove;
    canvas.repaint();
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
      int returnValue = JOptionPane.showConfirmDialog(myPlugin, "Move mote to"
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
  protected Vector<Mote> findMotesAtPosition(int clickedX, int clickedY) {
    double xCoord = factorXPixelToCoord(clickedX);
    double yCoord = factorYPixelToCoord(clickedY);

    Vector<Mote> motesFound = new Vector<Mote>();

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
        motesFound.add(simulation.getMote(i));
      }
    }
    if (motesFound.size() == 0)
      return null;

    return motesFound;
  }

  /**
   * Get colors a certain mote should be painted with. May be overridden to get
   * a different color scheme.
   * 
   * Normally this method returns an array of two colors, one for the state
   * (outer circle), the other for the type (inner circle).
   * 
   * If this method only returns one color, the entire mote will be painted
   * using that.
   * 
   * @param mote
   *          Mote to paint
   * @return Color[] { Inner color, Outer color }
   */
  abstract public Color[] getColorOf(Mote mote);

  protected void visualizeSimulation(Graphics g) {
    for (int i = 0; i < simulation.getMotesCount(); i++) {
      Mote mote = simulation.getMote(i);
      Color moteColors[] = getColorOf(mote);
      Position motePos = mote.getInterfaces().getPosition();

      Point pixelCoord = transformPositionToPixel(motePos);
      int x = pixelCoord.x;
      int y = pixelCoord.y;

      if (mote == highlightedMote) {
        g.setColor(highlightColor);
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

    Position motePos = simulation.getMote(0).getInterfaces().getPosition();
    smallestXCoord = biggestXCoord = motePos.getXCoordinate();
    smallestYCoord = biggestYCoord = motePos.getYCoordinate();

    // Get extreme coordinates
    for (int i = 0; i < simulation.getMotesCount(); i++) {
      motePos = simulation.getMote(i).getInterfaces().getPosition();

      if (motePos.getXCoordinate() < smallestXCoord)
        smallestXCoord = motePos.getXCoordinate();

      if (motePos.getXCoordinate() > biggestXCoord)
        biggestXCoord = motePos.getXCoordinate();

      if (motePos.getYCoordinate() < smallestYCoord)
        smallestYCoord = motePos.getYCoordinate();

      if (motePos.getYCoordinate() > biggestYCoord)
        biggestYCoord = motePos.getYCoordinate();

    }

    if ((biggestXCoord - smallestXCoord) == 0) {
      factorXCoordToPixel = 1;
    } else
      factorXCoordToPixel = ((double) canvas.getPreferredSize().width - 2 * CANVAS_BORDER_WIDTH)
          / (biggestXCoord - smallestXCoord);

    if ((biggestYCoord - smallestYCoord) == 0) {
      factorYCoordToPixel = 1;
    } else
      factorYCoordToPixel = ((double) canvas.getPreferredSize().height - 2 * CANVAS_BORDER_WIDTH)
          / (biggestYCoord - smallestYCoord);
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
    return (int) ((yCoordinate - smallestYCoord) * factorYCoordToPixel)
        + CANVAS_BORDER_WIDTH;
  }
  private double factorXPixelToCoord(int xPixel) {
    return ((double) (xPixel - CANVAS_BORDER_WIDTH) / factorXCoordToPixel)
        + smallestXCoord;
  }
  private double factorYPixelToCoord(int yPixel) {
    return ((double) (yPixel - CANVAS_BORDER_WIDTH) / factorYCoordToPixel)
        + smallestYCoord;
  }

  public void closePlugin() {
    if (moteHighligtObserver != null) {
      myGUI.deleteMoteHighligtObserver(moteHighligtObserver);
    }

    if (simObserver != null) {
      simulation.deleteObserver(simObserver);

      for (int i = 0; i < simulation.getMotesCount(); i++) {
        Position posIntf = simulation.getMote(i).getInterfaces().getPosition();
        if (posIntf != null) {
          posIntf.deleteObserver(posObserver);
        }
      }
    }
  }

}
