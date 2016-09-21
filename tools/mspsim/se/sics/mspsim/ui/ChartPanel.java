/**
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * This file is part of MSPSim.
 *
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * ChartPanel
 *
 * Authors : Adam Dunkels, Joakim Eriksson, Niclas Finne
 * Created : April 26 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.ui;
import java.awt.Color;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.util.Hashtable;

import javax.swing.BorderFactory;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.border.Border;

import se.sics.mspsim.util.ArrayUtils;

/**
 *
 */
public class ChartPanel extends JComponent {

  private static final long serialVersionUID = 7243446493049199594L;

  public static final String COLOR_TICK = "color.tick";

  private static final Border DEFAULT_BORDER =
    BorderFactory.createEmptyBorder(10,10,10,10);

  private static final Color[] DEFAULT_COLOR = {
    Color.blue, new Color(0xff008000), Color.red, Color.black
  };

  private static final Color LIGHT_GRAY = new Color(0xff909090);

  private Hashtable<String,Object> config = new Hashtable<String,Object>();
  private Chart[] charts = null;

  private Chart axisChart;

  /**
   *
   */
  public ChartPanel() {
    setForeground(Color.black);
    setBackground(Color.white);
    setBorder(DEFAULT_BORDER);
    setOpaque(true);
  }

  public Chart getAxisChart() {
    return axisChart;
  }

  public void setAxisChart(Chart chart) {
    this.axisChart = chart;
  }

  public synchronized void addChart(Chart chart) {
    charts = ArrayUtils.add(Chart.class, charts, chart);
  }

  public synchronized void removeChart(Chart chart) {
    charts = ArrayUtils.remove(charts, chart);
  }

  public Chart getChart(String name) {
    Chart[] charts = this.charts;
    if (charts != null) {
      for (int i = 0, n = charts.length; i < n; i++) {
        if (name.equals(charts[i].getName())) {
          return charts[i];
        }
      }
    }
    return null;
  }

  public Chart[] getCharts() {
    return charts;
  }

  public Object getConfig(String param) {
    return getConfig(param, null);
  }

  public Object getConfig(String param, Object defVal) {
    Object retVal = config.get(param);
    return retVal != null ? retVal : defVal;
  }

  public void setConfig(String param, Object value) {
    if (value != null) {
      config.put(param, value);
    } else {
      config.remove(param);
    }
  }

  private Color getDefaultColor(int index) {
    return DEFAULT_COLOR[index % DEFAULT_COLOR.length];
  }


  // -------------------------------------------------------------------
  // Paint handling
  // -------------------------------------------------------------------

  protected void paintComponent(Graphics g) {
    Graphics2D g2d = (Graphics2D) g.create();
    try {
      if (isOpaque()) {
        g2d.setColor(getBackground());
        g2d.fillRect(0, 0, getWidth(), getHeight());
      }
      paintCharts(g2d);
    } finally {
      g2d.dispose();
    }
  }

  protected void paintCharts(Graphics2D g) {
    Chart axisChart = this.axisChart;
    Color foreground = getForeground();
    Color tickColor = (Color) getConfig(COLOR_TICK, LIGHT_GRAY);
    FontMetrics fm = getFontMetrics(g.getFont());
    int fmHeight = fm.getAscent();
    Insets insets = getInsets();
    int height = getHeight();
    int width = getWidth();
    int leftInset = 40;
    height -= insets.top + insets.bottom + fmHeight + 2;
    width -= leftInset + insets.left + insets.right + 2;
    g.translate(leftInset + insets.left + 1, insets.top + 1);

    g.setColor(foreground);
    g.drawRect(-1, -1, width + 2, height + 2);

    Chart[] chs = getCharts();
    if (chs != null && chs.length > 0) {
      double totMaxY = Double.MIN_VALUE, totMinY = Double.MAX_VALUE; 
      double totMaxX = Double.MIN_VALUE, totMinX = Double.MAX_VALUE; 

      for (int i = 0, n = chs.length; i < n; i++) {
        Chart chart = chs[i];
        double maxY = chart.getMaxY();
        double minY = chart.getMinY();
        double minX = chart.getMinX();
        double maxX = chart.getMaxX();
        if (maxY >= minY && maxX > minX) {
          if (maxY > totMaxY) totMaxY = maxY;
          if (minY < totMinY) totMinY = minY;

          if (maxX > totMaxX) totMaxX = maxX;
          if (minX < totMinX) totMinX = minX;
        }
      }
      if (totMaxY >= totMinY && totMaxX > totMinX) {
        double yfac = (1.0 * height) / (totMaxY - totMinY);
        double xfac = (1.0 * width) / (totMaxX - totMinX);
        int zero = height;
        if (totMinY < 0) {
          zero += (int) (yfac * totMinY);
        }

        if (axisChart != null) {
//          double maxY = axisChart.getMaxY();
//          double minY = axisChart.getMinY();
//          double minX = axisChart.getMinX();
//          double maxX = axisChart.getMaxX();

//        if (maxY != minY && maxX != minX) {
          // Draw zero line
          g.setColor(tickColor);
          g.drawLine(0, zero, width, zero);
          g.setColor(foreground);
          g.drawString("0", -4 - fm.stringWidth("0"), zero + fmHeight / 2);

          double gridValue = getGridValue(totMinY, totMaxY, 10);
          for (double d = gridValue; d < totMaxY; d += gridValue) {
            int y = (int) (zero - yfac * d);
            String text = "" + (int) d;
            int tlen = fm.stringWidth(text);
            g.setColor(tickColor);
            g.drawLine(0, y, width, y);
            g.setColor(foreground);
            g.drawString(text, -4 - tlen, y + fmHeight / 2);
          }
          for (double d = gridValue; d < -totMinY; d += gridValue) {
            int y = (int) (zero + yfac * d);
            String text = "" + (int) -d;
            int tlen = fm.stringWidth(text);
            g.setColor(tickColor);
            g.drawLine(0, y, width, y);
            g.setColor(foreground);
            g.drawString(text, -4 - tlen, y + fmHeight / 2);
          }

          gridValue = getGridValue(totMinX, totMaxX, 10);
          for (double d = gridValue; d < totMaxX; d += gridValue) {
            int x = (int) (xfac * d);
            String text = "" + (int) d;
            int tlen = fm.stringWidth(text);
            g.setColor(tickColor);
            g.drawLine(x, 0, x, height);
            g.setColor(foreground);
            g.drawString(text, x - tlen / 2, height + fmHeight);
          }
        }

        for (int i = 0, n = chs.length; i < n; i++) {
          Chart chart = chs[i];
          double maxY = chart.getMaxY();
          double minY = chart.getMinY();
          double minX = chart.getMinX();
          double maxX = chart.getMaxX();

          if (maxY < minY || maxX <= minX) {
            // No data in this chart
          } else {
//          double yfac = (1.0 * height) / (maxY - minY);
//          double xfac = (1.0 * width) / (maxX - minX);

            g.setColor((Color) chart.getConfig("color", getDefaultColor(i)));
            chart.drawChart(g, xfac, yfac, width, height);
          }
        }
      }
    }
  }

  private double getGridValue(double totMinX, double totMaxX, int maxCount) {
    double diff = totMaxX - totMinX;
    if (diff <= 0) {
      return 1;
    }
    double d = Math.log(diff) / Math.log(10);
    int z = (int) d;
    if ((d - z) > 0.69) {
      z++;
    }
    if (z < 1) {
      return 1;
    }
    d = Math.pow(10, z - 1);
    if ((diff / (int) (1 * d)) <= maxCount) {
      return (int) (1 * d);
    }
    if ((diff / (int) (2.5 * d)) <= maxCount) {
      return (int) (2.5 * d);
    }
    return (int) (5 * d);
  }


  //-------------------------------------------------------------------
  // Test Main
  // -------------------------------------------------------------------

  public static void main(String[] args) {
    JFrame jf = new JFrame("test charts");
    jf.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    jf.setBounds(100,100,600,400);
    jf.setVisible(true);
    BarChart bc = new BarChart();
    LineChart lc = new LineChart();

    int[] data = new int[40];
    for (int i = 0, n = data.length; i < n; i++) {
      data[i] = (int) ((Math.random() * 300));
    }
    bc.setConfig("color", new Color(0xffb0b0b0));
    bc.setData(data);
    lc.setData(data);

    //    bc.setMinMax(0, 500);
    lc.setMinMax(-4096, 4096);

    ChartPanel chp = new ChartPanel();
    chp.addChart(lc);
    chp.addChart(bc);
    jf.getContentPane().add(chp);
    jf.setVisible(true);

    while(true) {
      try {
        Thread.sleep(100);
      } catch (Exception e) {
        // Ignore interrupts
      }

      int len = 1024;
      data = new int[len];
      for (int i = 0, n = data.length; i < n; i++) {
        data[i] = (int) (2500 - (Math.random() * 5000));
      }
      lc.setData(data);
      chp.repaint();
    }

  }

}
