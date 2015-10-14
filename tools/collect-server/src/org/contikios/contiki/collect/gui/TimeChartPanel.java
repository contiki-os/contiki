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
 * TimeChartPanel
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 3 jul 2008
 * Updated : $Date: 2010/11/03 14:53:05 $
 *           $Revision: 1.1 $
 */

package org.contikios.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.util.Date;
import javax.swing.JPanel;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.data.time.Second;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.contikios.contiki.collect.CollectServer;
import org.contikios.contiki.collect.Node;
import org.contikios.contiki.collect.SensorData;
import org.contikios.contiki.collect.Visualizer;

/**
 *
 */
public abstract class TimeChartPanel extends JPanel implements Visualizer {

  private static final long serialVersionUID = -607864439709540641L;

  protected final CollectServer server;
  protected final String category;
  protected final String title;
  protected final TimeSeriesCollection timeSeries;
  protected final JFreeChart chart;
  protected final ChartPanel chartPanel;

  private Node[] selectedNodes;

  private double minValue;
  private double maxValue;
  private int rangeTick = 0;
  private boolean hasGlobalRange;
  private int maxItemCount;
  
  public TimeChartPanel(CollectServer server, String category, String title,
      String chartTitle, String timeAxisLabel, String valueAxisLabel) {
    super(new BorderLayout());
    this.server = server;
    this.category = category;
    this.title = title;
    this.timeSeries = new TimeSeriesCollection();
    this.chart = ChartFactory.createTimeSeriesChart(
        chartTitle, timeAxisLabel, valueAxisLabel, timeSeries,
        true, true, false
    );
    this.chartPanel = new ChartPanel(chart);
    this.chartPanel.setPreferredSize(new Dimension(500, 270));
    setBaseShapeVisible(true);
    setMaxItemCount(server.getDefaultMaxItemCount());
    add(chartPanel, BorderLayout.CENTER);
  }

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
          int groupSize = getGroupSize(node);
          if (groupSize > 1) {
            series.clear();
            updateSeries(series, node, groupSize);
          } else {
            series.addOrUpdate(new Second(new Date(data.getNodeTime())), getSensorDataValue(data));
          }
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
        // Reduce the number of items by grouping them and use the average for each group
        int groupSize = getGroupSize(node);
        if (groupSize > 1) {
          updateSeries(series, node, groupSize);
        } else {
          for (int i = 0, n = node.getSensorDataCount(); i < n; i++) {
            SensorData data = node.getSensorData(i);
            series.addOrUpdate(new Second(new Date(data.getNodeTime())), getSensorDataValue(data));
          }
        }
        timeSeries.addSeries(series);
      }
    }
  }

  protected int getGroupSize(Node node) {
    if (maxItemCount > 0) {
      int sensorDataCount = node.getSensorDataCount();
      if (sensorDataCount > maxItemCount) {
        int groupSize = sensorDataCount / maxItemCount;
        if (sensorDataCount / groupSize >= maxItemCount) {
          groupSize++;
        }
        return groupSize;
      }
    }
    return 1;
  }

  protected void updateSeries(TimeSeries series, Node node, int groupSize) {
    for (int i = 0, n = node.getSensorDataCount(); i < n; i += groupSize) {
      double value = 0.0;
      long time = 0L;
      for (int j = 0; j < groupSize; j++) {
        SensorData data = node.getSensorData(i);
        value += getSensorDataValue(data);
        time += data.getNodeTime() / 1000L;
      }
      series.addOrUpdate(new Second(new Date((time / groupSize) * 1000L)), value / groupSize);
    }
  }

  public boolean getBaseShapeVisible() {
    return ((XYLineAndShapeRenderer)this.chart.getXYPlot().getRenderer()).getBaseShapesVisible();
  }

  public void setBaseShapeVisible(boolean visible) {
    ((XYLineAndShapeRenderer)this.chart.getXYPlot().getRenderer()).setBaseShapesVisible(visible);
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

  /**
   * Returns the maximal number of chart items to display for each node.
   *
   * @return the maximal number of chart items to display for each node or <code>0</code>
   * for unlimited number of chart items.
   */
  public int getMaxItemCount() {
    return maxItemCount;
  }

  /**
   * Sets the maximal number of chart items to display for each node. Items will be
   * grouped and replaced by the average value when needed.
   *
   * @param maxItemCount - the maximal number of chart items to display for each node or
   * <code>0</code> for unlimited number (default)
   */
  public void setMaxItemCount(int maxItemCount) {
    this.maxItemCount = maxItemCount;
    if (isVisible()) {
      updateCharts();
    }
  }

  public void setVisible(boolean visible) {
    if (visible) {
      updateGlobalRange();
      updateCharts();
    } else {
      timeSeries.removeAllSeries();
    }
    super.setVisible(visible);
  }

  protected abstract double getSensorDataValue(SensorData data);

}
