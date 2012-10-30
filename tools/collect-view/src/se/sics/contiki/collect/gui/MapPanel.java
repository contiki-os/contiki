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
 *
 * -----------------------------------------------------------------
 *
 * MapPanel
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 3 jul 2008
 * Updated : $Date: 2010/11/23 16:21:48 $
 *           $Revision: 1.4 $
 */

package se.sics.contiki.collect.gui;
import java.awt.Color;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GridLayout;
import java.awt.Polygon;
import java.awt.RenderingHints;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Properties;
import java.util.logging.Logger;

import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JFormattedTextField;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JSlider;
import javax.swing.Timer;
import javax.swing.border.LineBorder;
import javax.swing.border.TitledBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.plaf.basic.BasicGraphicsUtils;

import se.sics.contiki.collect.CollectServer;
import se.sics.contiki.collect.Configurable;
import se.sics.contiki.collect.Link;
import se.sics.contiki.collect.Node;
import se.sics.contiki.collect.SensorData;
import se.sics.contiki.collect.Visualizer;

/**
 *
 */
public class MapPanel extends JPanel implements Configurable, Visualizer, ActionListener, MouseListener, MouseMotionListener {

  private static final long serialVersionUID = -8256619482599309425L;

  private static final Logger log =
    Logger.getLogger(MapPanel.class.getName());

  private static final boolean VISUAL_DRAG = true;

  private static final Color LINK_COLOR = new Color(0x40, 0x40, 0xf0, 0xff);

  private static final int delta = 7;

  private final CollectServer server;
  private final String category;
  private final boolean isMap;
  private String title;

  private Timer timer;

  private JPopupMenu popupMenu;
  private JCheckBoxMenuItem layoutItem;
  private JCheckBoxMenuItem lockedItem;
  private JMenuItem shakeItem;
//  private JCheckBoxMenuItem dragItem;
  private JCheckBoxMenuItem backgroundItem;
  private JCheckBoxMenuItem showNetworkItem;
  private JCheckBoxMenuItem configItem;
  private JMenuItem resetNetworkItem;
  private MapNode popupNode;

  private Hashtable<String,MapNode> nodeTable = new Hashtable<String,MapNode>();
  private MapNode[] nodeList = new MapNode[0];
  private boolean updateNodeList;

  private MapNode selectedNode;
  private ArrayList<MapNode> selectedMapNodes = new ArrayList<MapNode>();
  private Node[] selectedNodes;
  private MapNode draggedNode;
  private long draggedTime;

  private ImageIcon mapImage;
  private String mapName;
  private boolean showBackground;

  private int layoutRepel = 100;
  private int layoutAttract = 50;
  private int layoutGravity = 1;

  private double etxFactor = 1.0;

  private boolean isLayoutActive = true;
  private boolean hideNetwork = false;

  protected JPanel configPanel;

  public MapPanel(CollectServer server, String title, String category, boolean isMap) {
    super(null);
    this.server = server;
    this.title = title;
    this.category = category;
    this.isMap = isMap;
    setPreferredSize(new Dimension(300, 200));

    popupMenu = new JPopupMenu(getTitle());
    if (!isMap) {
      layoutItem = createCheckBoxMenuItem(popupMenu, "Update Layout", isLayoutActive);
      popupMenu.add(layoutItem);

      lockedItem = createCheckBoxMenuItem(popupMenu, "Fixed Node Position", false);
      shakeItem = createMenuItem(popupMenu, "Shake Nodes");
      popupMenu.addSeparator();
    }

    showNetworkItem = createCheckBoxMenuItem(popupMenu, "Show Network Info", true);
    resetNetworkItem = createMenuItem(popupMenu, "Reset Network");
    popupMenu.addSeparator();
    if (isMap) {
      backgroundItem = createCheckBoxMenuItem(popupMenu, "Show Background", false);
      backgroundItem.setEnabled(false);
    } else {
      configItem = createCheckBoxMenuItem(popupMenu, "Show Layout Settings", false);
    }

//    popupMenu.addSeparator();
//    dragItem = new JCheckBoxMenuItem("Visible Drag", true);
//    popupMenu.add(dragItem);

    setBackground(Color.white);
    addMouseListener(this);
    addMouseMotionListener(this);
    
    if (!isMap) {
      timer = new Timer(100, this);

      configPanel = new JPanel(new GridLayout(0, 1));
      configPanel.setBorder(LineBorder.createBlackLineBorder());

      JSlider slider = new JSlider(JSlider.HORIZONTAL, 0, 1000, 1000 - layoutAttract);
      slider.setBorder(new TitledBorder("Attract Factor: " + (1000 - layoutAttract)));
      slider.addChangeListener(new ChangeListener() {
        public void stateChanged(ChangeEvent e) {
          JSlider slider = (JSlider)e.getSource();
          layoutAttract = 1000 - slider.getValue();
          ((TitledBorder)slider.getBorder()).setTitle("Attract Factor: " + slider.getValue());
        }
      });
      configPanel.add(slider);

      slider = new JSlider(JSlider.HORIZONTAL, 0, 1000, layoutRepel);
      slider.setBorder(new TitledBorder("Repel Range: " + layoutRepel));
      slider.addChangeListener(new ChangeListener() {
        public void stateChanged(ChangeEvent e) {
          JSlider slider = (JSlider)e.getSource();
          layoutRepel = slider.getValue();
          ((TitledBorder)slider.getBorder()).setTitle("Repel Range: " + layoutRepel);
        }
      });
      configPanel.add(slider);

      slider = new JSlider(JSlider.HORIZONTAL, 0, 100, layoutGravity);
      slider.setBorder(new TitledBorder("Gravity: " + layoutGravity));
      slider.addChangeListener(new ChangeListener() {
        public void stateChanged(ChangeEvent e) {
          JSlider slider = (JSlider)e.getSource();
          layoutGravity = slider.getValue();
          ((TitledBorder)slider.getBorder()).setTitle("Gravity: " + layoutGravity);
        }
      });
      configPanel.add(slider);

      final JFormattedTextField etxField = new JFormattedTextField(new Double(etxFactor));
      etxField.setBorder(BorderFactory.createTitledBorder("ETX factor"));
      etxField.setColumns(5);
      etxField.addPropertyChangeListener("value", new PropertyChangeListener() {
        public void propertyChange(PropertyChangeEvent evt) {
          etxFactor = ((Number)etxField.getValue()).doubleValue();
          repaint();
        }
      });
      configPanel.add(etxField);

      configPanel.setVisible(false);
      add(configPanel);

      addComponentListener(new ComponentAdapter() {
        public void componentResized(ComponentEvent ev) {
          if (configPanel.isVisible()) {
            updateConfigLayout();
          }
        }
      });
    }
  }

  public String getMapBackground() {
    return isMap ? mapName : null;
  }

  public boolean setMapBackground(String image) {
    if (!isMap) {
      return false;
    }
    if (image == null) {
      mapImage = null;
      mapName = null;
      backgroundItem.setEnabled(false);
      backgroundItem.setSelected(false);
      showBackground = false;
      repaint();
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
    showBackground = true;
    backgroundItem.setEnabled(true);
    backgroundItem.setSelected(true);
    repaint();
    return true;
  }

  private JCheckBoxMenuItem createCheckBoxMenuItem(JPopupMenu menu, String title, boolean isSelected) {
    JCheckBoxMenuItem item = new JCheckBoxMenuItem(title, isSelected);
    item.addActionListener(this);
    menu.add(item);
    return item;
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
      if (timer != null) {
        timer.start();
      }
    } else {
      if (timer != null) {
        timer.stop();
      }
    }
    super.setVisible(visible);
  }

  public void clear() {
    setCursor(Cursor.getDefaultCursor());
    draggedNode = null;
    updateSelected();
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
    String id = nd.getID();
    MapNode node = nodeTable.get(id);
    if (node == null) {
      node = new MapNode(this, nd);
      node.y = 10 + (int) (Math.random() * (Math.max(100, getHeight()) - 20));
      node.x = 10 + (int) (Math.random() * (Math.max(100, getWidth()) - 30)); 

      String location = server.getConfig(isMap ? id : ("collect.map." + id));
      if (location != null) {
        try {
          String[] pos = location.split(",");
          node.x = Integer.parseInt(pos[0].trim());
          node.y = Integer.parseInt(pos[1].trim());
          node.hasFixedLocation = !isMap;
        } catch (Exception e) {
          System.err.println("could not parse node location: " + location);
          e.printStackTrace();
        }
      }

      nodeTable.put(id, node);
      updateNodeList = true;
    }
    return node;
  }

  private MapNode[] getNodeList() {
    if (updateNodeList) {
      synchronized (nodeTable) {
        updateNodeList = false;
        nodeList = nodeTable.values().toArray(new MapNode[nodeTable.size()]);
      }
    }
    return nodeList;
  }


  // -------------------------------------------------------------------
  // Visualizer
  // -------------------------------------------------------------------

  @Override
  public String getCategory() {
    return category;
  }

  @Override
  public String getTitle() {
    return title;
  }

  @Override
  public Component getPanel() {
    return this;
  }

  @Override
  public void nodesSelected(Node[] nodes) {
    if (selectedNodes != nodes) {
      selectedNodes = nodes;
      if (isVisible()) {
        updateSelected();
      }
    }
  }

  private void updateSelected() {
    if (selectedMapNodes.size() > 0) {
      for(MapNode node : selectedMapNodes) {
        node.isSelected = false;
      }
      selectedMapNodes.clear();
    }

    if (selectedNodes == null || selectedNodes.length == 0) {
      selectedNode = null;
    } else {
      for (Node node : selectedNodes) {
        MapNode mapNode = addMapNode(node);
        selectedMapNodes.add(mapNode);
        mapNode.isSelected = true;
      }
      selectedNode = selectedMapNodes.get(0);
    }

    repaint();
  }

  @Override
  public void nodeAdded(Node nd) {
    addMapNode(nd);
    if (isVisible()) {
      repaint();
    }
  }

  @Override
  public void nodeDataReceived(SensorData sensorData) {
    if (isVisible()) {
      repaint();
    }
  }

  @Override
  public void clearNodeData() {
    nodeTable.clear();
    updateNodeList = true;
    nodesSelected(null);
    if (isVisible()) {
      repaint();
    }
  }


  // -------------------------------------------------------------------
  // Graphics
  // -------------------------------------------------------------------

  @Override
  protected void paintComponent(Graphics g) {
    Graphics2D g2d = (Graphics2D) g;
    FontMetrics fm = g.getFontMetrics();
    int fnHeight = fm.getHeight();
    int fnDescent = fm.getDescent();
    int width = getWidth();
    int height = getHeight();

    g.setColor(getBackground());
    g.fillRect(0, 0, width, height);
    if (showBackground && isMap) {
      mapImage.paintIcon(this, g, 0, 0);
    }
    g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
        RenderingHints.VALUE_ANTIALIAS_ON);

    // Display legend
    if (!hideNetwork) {
      int legendWidth = fm.stringWidth("ETX");
      g.setColor(Color.black);
      g.drawString("ETX", width - legendWidth - 10, 10 + fnHeight - fnDescent);
      g.drawRect(width - legendWidth - 30, 8, legendWidth + 24, fnHeight + 4);
      g.setColor(LINK_COLOR);
      g2d.drawLine(width - legendWidth - 25, 10 + fnHeight / 2,
          width - legendWidth - 15, 10 + fnHeight / 2);
    }

    MapNode[] nodes = getNodeList();
    if (!isMap || !hideNetwork) {
      g.setColor(LINK_COLOR);
      for (MapNode n : nodes) {
        for (int j = 0, mu = n.node.getLinkCount(); j < mu; j++) {
          Link link = n.node.getLink(j);
          MapNode linkNode = addMapNode(link.node);
          int x2 = linkNode.x;
          int y2 = linkNode.y;
          g2d.drawLine(n.x, n.y, x2, y2);
          drawArrow(g, n.x, n.y, x2, y2, 3);
          if (!hideNetwork) {
            int xn1, xn2, yn1, yn2;
            if (n.x <= x2) {
              xn1 = n.x; xn2 = x2;
              yn1 = n.y; yn2 = y2;
            } else {
              xn1 = x2; xn2 = n.x;
              yn1 = y2; yn2 = n.y;
            }
            int dx = xn1 + (xn2 - xn1) / 2 + 4;
            int dy = yn1 + (yn2 - yn1) / 2 - fnDescent;
            if (yn2 < yn1) {
              dy += fnHeight - fnDescent;
            }
            g.drawString(
                Double.toString(((int) (link.getETX() * etxFactor * 100 + 0.5)) / 100.0),
                dx, dy);
          }
        }
      }
    }

    for (MapNode n : nodes) {
      n.paint(g, n.x, n.y);

      g.setColor(Color.black);
      if (n.isSelected) {
        BasicGraphicsUtils.drawDashedRect(g, n.x - delta, n.y - delta,
            2 * delta, 2 * delta);
      }
      if (selectedNode != null && selectedNode.message != null) {
        g.drawString(selectedNode.message, 10, 10);
      }
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


  // -------------------------------------------------------------------
  // ActionListener
  // -------------------------------------------------------------------

  public void actionPerformed(ActionEvent e) {
    Object source = e.getSource();
    if (!isMap && source == timer) {
      if (isLayoutActive) {
        updateNodeLayout();
        repaint();
      }

    } else if (!isMap && source == lockedItem) {
      if (popupNode != null) {
        boolean wasFixed = popupNode.hasFixedLocation;
        popupNode.hasFixedLocation = lockedItem.isSelected();
        if (wasFixed && !popupNode.hasFixedLocation) {
          server.removeConfig("collect.map." + popupNode.node.getID());
        } else if (!wasFixed && popupNode.hasFixedLocation) {
          server.setConfig("collect.map." + popupNode.node.getID(),
                           "" + popupNode.x + ',' + popupNode.y);
        }
        repaint();
      }

    } else if (!isMap && source == layoutItem) {
      isLayoutActive = layoutItem.isSelected();

    } else if (!isMap && source == shakeItem) {
      for(MapNode n : getNodeList()) {
        if (!n.hasFixedLocation) {
          n.x += Math.random() * 100 - 50;
          n.y += Math.random() * 100 - 50;
        }
      }

    } else if (!isMap && source == configItem) {
      if (configItem.isSelected()) {
        configPanel.setSize(getPreferredSize());
        configPanel.validate();
        updateConfigLayout();
        configPanel.setVisible(true);
      } else {
        configPanel.setVisible(false);
      }
      repaint();
      
    } else if (source == showNetworkItem) {
      hideNetwork = !showNetworkItem.isSelected();
      repaint();

    } else if (source == resetNetworkItem) {
      for(MapNode n : getNodeList()) {
        n.node.clearLinks();
      }
      repaint();
    } else if (isMap && source == backgroundItem) {
      showBackground = mapImage != null && backgroundItem.isSelected();
      repaint();
    }
  }

  private void updateNodeLayout() {
    MapNode[] nodes = getNodeList();
    for (MapNode n : nodes) {

      // Attract connected nodes
      for(int i = 0, jn = n.node.getLinkCount(); i < jn; i++) {
        Link link = n.node.getLink(i);
        MapNode n2 = addMapNode(link.node);
        double vx = n2.x - n.x;
        double vy = n2.y - n.y;
        double dist = Math.sqrt(vx * vx + vy * vy);
        dist = dist == 0 ? 0.00001 : dist;
        double etx = link.getETX() * etxFactor;
        if (etx > 5) etx = 5;
        double factor = (etx * layoutAttract - dist) / (dist * 3);
        double dx = factor * vx;
        double dy = factor * vy;

        n2.dx += dx;
        n2.dy += dy;
        n.dx -= dx;
        n.dy -= dy;
      }

      // Repel nodes that are too close
      double dx = 0, dy = 0;
      for (MapNode n2 : nodes) {
        if (n == n2) {
          continue;
        }
        double vx = n.x - n2.x;
        double vy = n.y - n2.y;
        double dist = vx * vx + vy * vy;
        if (dist == 0) {
          dx += Math.random() * 5;
          dy += Math.random() * 5;
        } else if (dist < layoutRepel * layoutRepel) {
          dx += vx / dist;
          dy += vy / dist;
        }
      }
      double dist = dx * dx + dy * dy;
      if (dist > 0) {
        dist = Math.sqrt(dist) / 2;
        n.dx += dx / dist;
        n.dy += dy / dist;
      }

      n.dy += layoutGravity;
    }

    // Update the node positions
    int width = getWidth();
    int height = getHeight();
    for(MapNode n : nodes) {
      if (!n.hasFixedLocation && n != draggedNode) {
        n.x += Math.max(-5, Math.min(5, n.dx));
        n.y += Math.max(-5, Math.min(5, n.dy));
        if (n.x < 0) {
          n.x = 0;
        } else if (n.x > width) {
          n.x = width;
        }
        if (n.y < 0) {
          n.y = 0;
        } else if (n.y > height) {
          n.y = height;
        }
      }
      n.dx /= 2;
      n.dy /= 2;
    }
  }

  private void updateConfigLayout() {
    configPanel.setLocation(getWidth() - configPanel.getWidth() - 10,
        getHeight() - configPanel.getHeight() - 10);
  }


  // -------------------------------------------------------------------
  // Mouselistener
  // -------------------------------------------------------------------

  private MapNode getNodeAt(int mx, int my) {
    for(MapNode n : getNodeList()) {
      if (mx >= (n.x - delta)
          && mx <= (n.x + delta)
          && my >= (n.y - delta)
          && my <= (n.y + delta)) {
        return n;
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
      if (draggedTime < 0) {
        setCursor(Cursor.getDefaultCursor());
        draggedTime = 0;
      }
      selectedNode = draggedNode = null;
      server.selectNodes(null);
    }
    showPopup(e);
  }

  public void mouseReleased(MouseEvent e) {
    if (draggedNode != null && e.getButton() == MouseEvent.BUTTON1) {
      if ((draggedTime > 0) &&
          (System.currentTimeMillis() - draggedTime) < 300) {
        // Do not drag if mouse is only moved during click

      } else {
        draggedNode.x = e.getX();
        draggedNode.y = e.getY();
        setCursor(Cursor.getDefaultCursor());
        if (!isMap && draggedNode.hasFixedLocation) {
          /* Update fixed location */
          server.setConfig("collect.map." + draggedNode.node.getID(),
                           "" + draggedNode.x + ',' + draggedNode.y);
        }
        draggedTime = 0;
        draggedNode = null;
        repaint();
      }
    }

    showPopup(e);
  }

  private void showPopup(MouseEvent e) {
    if (e.isPopupTrigger()
        && (e.getModifiers() & (MouseEvent.SHIFT_MASK|MouseEvent.CTRL_MASK)) == 0) {
      popupNode = getNodeAt(e.getX(), e.getY());
      if (!isMap) {
        lockedItem.setEnabled(popupNode != null);
        lockedItem.setSelected(popupNode != null ? popupNode.hasFixedLocation : false);
      }
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
    if (draggedNode == null) {
      // Do nothing

    } else if (draggedTime > 0) {
      if ((System.currentTimeMillis() - draggedTime) > 300) {
        // No mouse click, time to drag the node
        draggedTime = -1;
        setCursor(Cursor.getPredefinedCursor(Cursor.MOVE_CURSOR));
      }

    } else if (VISUAL_DRAG /* && dragItem.isSelected() */) {
      draggedNode.x = e.getX();
      draggedNode.y = e.getY();
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
    public int x;
    public int y;
    public double dx;
    public double dy;
    public boolean hasFixedLocation;
    public boolean isSelected;
    public String message;

    MapNode(MapPanel panel, Node node) {
      this.node = node;
    }

    public void paint(Graphics g, int x, int y) {
      final int od = 3;
      g.setColor(Color.black);
      g.drawString(node.getID(), x + od * 2 + 3, y + 4);
      if (hasFixedLocation) {
        g.setColor(Color.red);
      }
      g.fillOval(x - od, y - od, od * 2 + 1, od * 2 + 1);
    }

  } // end of inner class MapNode


  @Override
  public void updateConfig(Properties config) {
    if (isMap) {
      for (MapNode n : getNodeList()) {
        config.put(n.node.getID(), "" + n.x + ',' + n.y);
      }
    }
  }

}
