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
 * $Id: TimeChartPanel.java,v 1.1 2008/07/09 23:18:07 nifi Exp $
 *
 * -----------------------------------------------------------------
 *
 * PowerPanel
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 3 jul 2008
 * Updated : $Date: 2008/07/09 23:18:07 $
 *           $Revision: 1.1 $
 */

package se.sics.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.util.Date;
import javax.swing.JPanel;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.data.time.Second;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import se.sics.contiki.collect.CollectServer;
import se.sics.contiki.collect.Node;
import se.sics.contiki.collect.SensorData;
import se.sics.contiki.collect.Visualizer;

/**
 *
 */
public abstract class TimeChartPanel extends JPanel implements Visualizer {

  private static final long serialVersionUID = -607864439709540641L;

  protected final CollectServer server;
  protected final String title;
  protected final TimeSeriesCollection timeSeries;
  protected final JFreeChart chart;
  protected final ChartPanel chartPanel;

  private Node[] selectedNodes;

  private double minValue;
  private double maxValue;
  private int rangeTick = 0;
  private boolean hasGlobalRange;

  public TimeChartPanel(CollectServer server, String title,
      String chartTitle, String timeAxisLabel, String valueAxisLabel) {
    super(new BorderLayout());
    this.server = server;
    this.title = title;
    this.timeSeries = new TimeSeriesCollection();
    this.chart = ChartFactory.createTimeSeriesChart(
        chartTitle, timeAxisLabel, valueAxisLabel, timeSeries,
        true, true, false
    );
    this.chartPanel = new ChartPanel(chart);
    this.chartPanel.setPreferredSize(new Dimension(500, 270));
    add(chartPanel, BorderLayout.CENTER);
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
  public void nodeAdded(Node node) {
    // Ignore
  }

  @Override
  public void nodesSelected(Node[] nodes) {
    if (this.selectedNodes != nodes) {
      this.selectedNodes = nodes;
      if (isVisible()) {
        updateCharts();
      }
    }
  }

  @Override
  public void nodeDataReceived(SensorData data) {
    if (hasGlobalRange) {
      boolean update = false;
      if (minValue > maxValue) {
        update = true;
      } else {
        double value = getSensorDataValue(data);
        if (value < minValue) {
          minValue = value;
          update = true;
        }
        if (value > maxValue) {
          maxValue = value;
          update = true;
        }
      }
      if (update && isVisible()) {
        updateGlobalRange();
      }
    }
    if (isVisible() && selectedNodes != null && selectedNodes.length == timeSeries.getSeriesCount()) {
      Node node = data.getNode();
      for (int i = 0, n = selectedNodes.length; i < n; i++) {
        if (node == selectedNodes[i]) {
          TimeSeries series = timeSeries.getSeries(i);
          series.addOrUpdate(new Second(new Date(data.getTime())), getSensorDataValue(data));
          chartPanel.repaint();
          break;
        }
      }
    }
  }

  @Override
  public void clearNodeData() {
    if (isVisible()) {
      updateCharts();
    }
  }

  private void updateCharts() {
    timeSeries.removeAllSeries();
    if (this.selectedNodes != null) {
      for(Node node: this.selectedNodes) {
        TimeSeries series = new TimeSeries(node.getName(), Second.class);
        for (int i = 0, n = node.getSensorDataCount(); i < n; i++) {
          SensorData data = node.getSensorData(i);
          series.addOrUpdate(new Second(new Date(data.getTime())), getSensorDataValue(data));
        }
        timeSeries.addSeries(series);
      }
    }
  }

  public int getRangeTick() {
    return rangeTick;
  }

  public void setRangeTick(int rangeTick) {
    this.rangeTick = rangeTick;
  }

  public double getRangeMinimumSize() {
    return chart.getXYPlot().getRangeAxis().getAutoRangeMinimumSize();
  }

  public void setRangeMinimumSize(double size) {
    chart.getXYPlot().getRangeAxis().setAutoRangeMinimumSize(size);
  }

  public boolean hasGlobalRange() {
    return hasGlobalRange;
  }

  public void setGlobalRange(boolean hasGlobalRange) {
    if (this.hasGlobalRange != hasGlobalRange) {
      this.hasGlobalRange = hasGlobalRange;
      if (hasGlobalRange) {
        minValue = Double.MAX_VALUE;
        maxValue = Double.MIN_NORMAL;
        if (isVisible()) {
          updateGlobalRange();
        }
      } else {
        chart.getXYPlot().getRangeAxis().setAutoRange(true);
      }
    }
  }

  private void updateGlobalRange() {
    if (hasGlobalRange) {
      if (minValue > maxValue) {
        for (int i = 0, n = server.getSensorDataCount(); i < n; i++) {
          double value = getSensorDataValue(server.getSensorData(i));
          if (value < minValue) minValue = value;
          if (value > maxValue) maxValue = value;
        }
      }
      if (minValue < maxValue) {
        double minSize = getRangeMinimumSize();
        double min = minValue;
        double max = maxValue;
        if (max - min < minSize) {
          double d = (minSize - (max - min)) / 2;
          min -= d;
          max += d;
        }
        if (rangeTick > 0) {
          min = ((int) (min - rangeTick / 2) / rangeTick) * rangeTick;
//          max = ((int) (max + rangeTick / 2) / rangeTick) * rangeTick;
        }
        chart.getXYPlot().getRangeAxis().setRange(min, max);
      }
    }
  }

  protected abstract double getSensorDataValue(SensorData data);

  public void setVisible(boolean visible) {
    if (visible) {
      updateGlobalRange();
      updateCharts();
    } else {
      timeSeries.removeAllSeries();
    }
    super.setVisible(visible);
  }

}
