/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: MapPanel.java,v 1.1 2008/07/09 23:18:07 nifi Exp $
 *
 * -----------------------------------------------------------------
 *
 * MapPanel
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 3 jul 2008
 * Updated : $Date: 2008/07/09 23:18:07 $
 *           $Revision: 1.1 $
 */

package se.sics.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.geom.Line2D;
import java.util.Hashtable;
import java.util.logging.Logger;
import javax.swing.ImageIcon;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.Timer;
import javax.swing.plaf.basic.BasicGraphicsUtils;
import se.sics.contiki.collect.CollectServer;
import se.sics.contiki.collect.Link;
import se.sics.contiki.collect.Node;
import se.sics.contiki.collect.SensorData;
import se.sics.contiki.collect.Visualizer;

/**
 *
 */
public class MapPanel extends JPanel implements Visualizer, ActionListener, MouseListener, MouseMotionListener {

  private static final long serialVersionUID = -8256619482599309425L;

  private static final Logger log =
    Logger.getLogger(MapPanel.class.getName());

  private static final boolean VISUAL_DRAG = true;

  private static final int FADE_COUNT = 20;
  private static final int AGE_COUNT = 200;

  private static final Color[] OTHER_COLOR = new Color[FADE_COUNT];
  private static final Color[] LINK_COLOR = new Color[AGE_COUNT];

  static {
    for (int i = 0; i < FADE_COUNT; i++) {
      OTHER_COLOR[i] = new Color(0xe0,0xe0,0x00,0xFF
                               - ((i * 255) / (FADE_COUNT - 1)));
    }

    for (int i = 0, n = AGE_COUNT; i < n; i++) {
      LINK_COLOR[i] = new Color(0x40 + i / 2, 0x40 + i / 2, 0xf0, 0xff - i);
    }
  }

  private static final int delta = 7;

  public static final int SHOW_BLINK = 40;
  public static final int TOTAL_SHOW = 600;

  private Timer timer = new Timer(200, this);
  private boolean hasPendingEvents = false;
  private int ticker = 0;

  private JPopupMenu popupMenu;
//  private MapNode popupNode;
  private JMenuItem hideItem;
  private JMenuItem resetNetworkItem;

  private Hashtable<String,MapNode> nodeTable = new Hashtable<String,MapNode>();
  private MapNode[] nodeList;

  private MapNode selectedNode;
  private MapNode[] selectedMapNodes;
  private Node[] selectedNodes;
  private MapNode draggedNode;
  private long draggedTime;

  private ImageIcon mapImage;
  private String mapName;

  private final CollectServer server;

  private boolean hideNetwork = false;


  public MapPanel(CollectServer server) {
    super(new BorderLayout());
    this.server = server;
    setPreferredSize(new Dimension(300, 200));

    popupMenu = new JPopupMenu(getTitle());
//    popupMenu.addSeparator();
    hideItem = createMenuItem(popupMenu, "Hide Network");
    resetNetworkItem = createMenuItem(popupMenu, "Reset Network");

    addMouseListener(this);
    if (VISUAL_DRAG) {
      addMouseMotionListener(this);
    }
    setBackground(Color.white);
  }

  public String getMapBackground() {
    return mapName;
  }

  public boolean setMapBackground(String image) {
    if (image == null) {
      mapImage = null;
      mapName = null;
      return true;
    }

    ImageIcon ii = new ImageIcon(image);
    if (ii.getIconWidth() <= 0 || ii.getIconHeight() <= 0) {
      log.warning("could not find image '" + image + '\'');
      return false;
    }
    mapImage = ii;
    mapName = image;
    setPreferredSize(new Dimension(ii.getIconWidth(), ii.getIconHeight()));
    repaint();
    return true;
  }

  private JMenuItem createMenuItem(JPopupMenu menu, String title) {
    JMenuItem item = new JMenuItem(title);
    item.addActionListener(this);
    menu.add(item);
    return item;
  }

  public void setVisible(boolean visible) {
    if (visible) {
      clear();
      timer.start();
    } else {
      timer.stop();
    }
    super.setVisible(visible);
  }

  public void clear() {
    draggedNode = null;
    hasPendingEvents = false;
  }


  // -------------------------------------------------------------------
  // Node handling
  // -------------------------------------------------------------------

  public Node getNode(String id) {
    MapNode node = nodeTable.get(id);
    return node != null ? node.node : null;
  }

  public MapNode getMapNode(String id) {
    return nodeTable.get(id);
  }

  private MapNode addMapNode(Node nd) {
    MapNode node = nodeTable.get(nd.getID());
    if (node == null) {
      node = new MapNode(this, nd);
      synchronized (this) {
        nodeTable.put("" + nd.getID(), node);
        nodeList = nodeTable.values().toArray(new MapNode[nodeTable.size()]);
      }
    }
    return node;
  }


  // -------------------------------------------------------------------
  // Visualizer
  // -------------------------------------------------------------------

  @Override
  public String getTitle() {
    return "Sensor Map";
  }

  @Override
  public Component getPanel() {
    return this;
  }

  @Override
  public void nodesSelected(Node[] nodes) {
    if (selectedNodes != nodes) {
      if (selectedMapNodes != null) {
        for(MapNode node : selectedMapNodes) {
          node.isSelected = false;
        }
      }
      selectedNodes = nodes;
      if (nodes == null || nodes.length == 0) {
        selectedNode = null;
        selectedMapNodes = null;
      } else {
        selectedMapNodes = new MapNode[nodes.length];
        for (int i = 0, n = nodes.length; i < n; i++) {
          selectedMapNodes[i] = addMapNode(nodes[i]);
          selectedMapNodes[i].isSelected = true;
        }
        selectedNode = selectedMapNodes[0];
      }
      repaint();
    }
  }

  @Override
  public void nodeAdded(Node nd) {
    addMapNode(nd);
    repaint();
  }

  @Override
  public void nodeDataReceived(SensorData sensorData) {
    repaint();
  }

  @Override
  public void clearNodeData() {
    // Ignore
  }


  // -------------------------------------------------------------------
  // Graphics
  // -------------------------------------------------------------------

  public void paint(Graphics g) {
    Graphics2D g2d = (Graphics2D) g;
    int lx = 10;
    super.paint(g);
    long time = System.currentTimeMillis();
    if (mapImage != null) {
      mapImage.paintIcon(this, g, 0, 0);
    }
    MapNode[] nodes = nodeList;
    if (nodes != null) {
      Line2D line = new Line2D.Double();
      for (int i = 0, m = nodes.length; i < m; i++) {
        MapNode n = nodes[i];
        int x, y;
        if (n.node.hasLocation()) {
          x = n.node.getX();
          y = n.node.getY();
        } else {
          x = lx;
          y = 10;
          lx += 30;
        }

        if (!hideNetwork) {
          FontMetrics fm = g.getFontMetrics();
          int fnHeight = fm.getHeight();
          int fnDescent = fm.getDescent();
          for (int j = 0, mu = n.node.getLinkCount(); j < mu; j++) {
            Link link = n.node.getLink(j);
            if (link.node.hasLocation()) {
              long age = (time - link.getLastActive()) / 100;
              int x2 = link.node.getX();
              int y2 = link.node.getY();
              if (n.isSelected) {
                if (age > LINK_COLOR.length) {
                  age = 100;
                } else {
                  age -= 50;
                }
              }
              line.setLine(x, y, x2, y2);
              if (age < LINK_COLOR.length) {
                g.setColor(age < 0 ? LINK_COLOR[0] : LINK_COLOR[(int) age]);
              } else {
                g.setColor(LINK_COLOR[LINK_COLOR.length - 1]);
              }
              g2d.draw(line);
//              g.setColor(Color.lightGray);
              int xn1, xn2, yn1, yn2;
              if (x <= x2) {
                xn1 = x; xn2 = x2;
                yn1 = y; yn2 = y2;
              } else {
                xn1 = x2; xn2 = x;
                yn1 = y2; yn2 = y;
              }
              int dx = xn1 + (xn2 - xn1) / 2 + 4;
              int dy = yn1 + (yn2 - yn1) / 2 - fnDescent;
              if (yn2 < yn1) {
                dy += fnHeight - fnDescent;
              }
              g.drawString("ETX:" + (((int)(link.getETX() * 100 + 0.5)) / 100.0), dx, dy);
            }
          }
        }

        n.paint(g, x, y);

        g.setColor(Color.black);
        if (n.isSelected) {
          BasicGraphicsUtils.drawDashedRect(g, x - delta, y - delta, 2 * delta, 2 * delta);
        }
        if (selectedNode != null && selectedNode.message != null) {
          g.drawString(selectedNode.message, 10, 10);
        }
      }
    }
  }


  // -------------------------------------------------------------------
  // ActionListener
  // -------------------------------------------------------------------

  public void actionPerformed(ActionEvent e) {
    Object source = e.getSource();
    if (source == timer) {
      ticker++;
      if (hasPendingEvents) {
        boolean repaint = false;
        hasPendingEvents = false;
        MapNode[] nodes = nodeList;
        if (nodes != null) {
          long time = System.currentTimeMillis();
          for (int i = 0, n = nodes.length; i < n; i++) {
            if (nodes[i].tick(time)) {
              repaint = true;
            }
          }
        }
        if (repaint) {
          hasPendingEvents = true;
          repaint();
        }
      } else if ((ticker % 10) == 0) {
        repaint();
      }
    } else if (source == hideItem) {
      hideNetwork = !hideNetwork;
      if (!hideNetwork) hideItem.setText("Hide Network");
      else hideItem.setText("Show Network");
      repaint();

    } else if (source == resetNetworkItem) {
      MapNode[] nodes = nodeList;
      if (nodes != null) {
        for (int i = 0, m = nodes.length; i < m; i++) {
          MapNode n = nodes[i];
          n.node.clearLinks();
        }
        repaint();
      }
    }
  }


  // -------------------------------------------------------------------
  // Mouselistener
  // -------------------------------------------------------------------

  private MapNode getNodeAt(int mx, int my) {
    int lx = 10;
    MapNode[] nodes = nodeList;
    if (nodes != null) {
      for (int i = 0, m = nodes.length; i < m; i++) {
        MapNode n = nodes[i];
        int x, y;
        if (n.node.hasLocation()) {
          x = n.node.getX();
          y = n.node.getY();
        } else {
          x = lx;
          y = 10;
          lx += 30;
        }
        if (mx >= (x - delta)
            && mx <= (x + delta)
            && my >= (y - delta)
            && my <= (y + delta)) {
          return n;
        }
      }
    }
    return null;
  }

  public void mouseClicked(MouseEvent e) {
    int mx = e.getX();
    int my = e.getY();
    if (e.getButton() == MouseEvent.BUTTON1) {
      MapNode node = getNodeAt(mx, my);
      if (node != selectedNode) {
        server.selectNodes(node == null ? null : new Node[] { node.node });
      }
    }
    showPopup(e);
  }

  public void mousePressed(MouseEvent e) {
    if (e.getButton() == MouseEvent.BUTTON1) {
      MapNode aNode = getNodeAt(e.getX(), e.getY());
      if (aNode != selectedNode) {
        server.selectNodes(aNode != null ? new Node[] { aNode.node } : null);
      }
      draggedNode = aNode;
      draggedTime = System.currentTimeMillis();

    } else if (selectedNode != null) {
      selectedNode = draggedNode = null;
      server.selectNodes(null);
    }
    showPopup(e);
  }

  public void mouseReleased(MouseEvent e) {
    if (draggedNode != null && e.getButton() == MouseEvent.BUTTON1) {
      if ((!VISUAL_DRAG || (draggedTime > 0)) &&
          (System.currentTimeMillis() - draggedTime) < 300) {
        // Do not drag if mouse is only moved during click

      } else {
        draggedNode.node.setLocation(e.getX(), e.getY());
        server.updateNodeLocation(draggedNode.node);
        draggedNode = null;
        repaint();
      }
    }

    showPopup(e);
  }

  private void showPopup(MouseEvent e) {
    if (e.isPopupTrigger()
        && (e.getModifiers() & (MouseEvent.SHIFT_MASK|MouseEvent.CTRL_MASK)) == 0) {
//      popupNode = getNodeAt(e.getX(), e.getY());
//      nodeItem.setEnabled(popupNode != null);
      popupMenu.show(this, e.getX(), e.getY());
    }
  }

  public void mouseEntered(MouseEvent e) {
  }

  public void mouseExited(MouseEvent e) {
  }


  // -------------------------------------------------------------------
  // MouseMotion
  // -------------------------------------------------------------------

  public void mouseDragged(MouseEvent e) {
    if (!VISUAL_DRAG || draggedNode == null) {
      // Do nothing

    } else if (draggedTime > 0) {
      if ((System.currentTimeMillis() - draggedTime) > 300) {
        // No mouse click, time to drag the node
        draggedTime = -1;
      }

    } else {
      draggedNode.node.setLocation(e.getX(), e.getY());
      repaint();
    }
  }

  public void mouseMoved(MouseEvent e) {
  }


  // -------------------------------------------------------------------
  // MapNode
  // -------------------------------------------------------------------

  private static class MapNode {

    public final Node node;
    public boolean isSelected;
    public String message = null;

    private int tick = 0;

    MapNode(MapPanel panel, Node node) {
      this.node = node;
    }

    boolean tick(long time) {
      boolean r = false;
      if (tick > 0) {
        tick--;
        r = true;
      }

      for (int i = 0, n = node.getLinkCount(); i < n; i++) {
        Link link = node.getLink(i);
        long age = (time - link.getLastActive()) / 100;
        if (age < 200) {
          r = true;
          break;
        }
      }
      return r;
    }

    public void paint(Graphics g, int x, int y) {
      if (tick > (TOTAL_SHOW - SHOW_BLINK)) {
        if ((tick & 4) == 0) {
          // Hide circle
        } else {
          int index = FADE_COUNT - tick - 1;
          if (index < 0) {
            index = 0;
          }
          final int d = 8;
          g.setColor(OTHER_COLOR[index]);
          g.fillOval(x - d, y - d, d * 2 + 1, d * 2 + 1);
        }
      }

      if (tick < (TOTAL_SHOW - SHOW_BLINK) && tick > 0) {
        g.setColor(Color.red);
        int height = 13 * tick / TOTAL_SHOW;
        g.fillRect(x - 6, 5 + y - height, 2, height);
      }

      g.setColor(Color.black);
      final int od = 3;
      g.drawString(node.getID(), x + od * 2 + 3, y + 4);
      g.fillOval(x - od, y - od, od * 2 + 1, od * 2 + 1);
    }

  } // end of inner class MapNode

}
