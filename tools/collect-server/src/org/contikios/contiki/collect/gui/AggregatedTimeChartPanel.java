/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * -----------------------------------------------------------------
 *
 * PacketChartPanel
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 6 sep 2010
 */

package org.contikios.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.data.time.Minute;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;

import org.contikios.contiki.collect.CollectServer;
import org.contikios.contiki.collect.Node;
import org.contikios.contiki.collect.SensorData;
import org.contikios.contiki.collect.Visualizer;

/**
 *
 */
public abstract class AggregatedTimeChartPanel<T> extends JPanel implements Visualizer {

  private static final long serialVersionUID = 2100788758213434540L;

  protected final CollectServer server;
  protected final String category;
  protected final String title;
  protected final TimeSeries series;

  protected final JFreeChart chart;
  protected final ChartPanel chartPanel;

  private Node[] selectedNodes;
  private HashMap<Node,T> selectedMap = new HashMap<Node,T>();

  public AggregatedTimeChartPanel(CollectServer server, String category, String title,
      String timeAxisLabel, String valueAxisLabel) {
    super(new BorderLayout());
    this.server = server;
    this.category = category;
    this.title = title;
    this.series = new TimeSeries(title, Minute.class);
    TimeSeriesCollection timeSeries = new TimeSeriesCollection(series);
    this.chart = ChartFactory.createTimeSeriesChart(
        title, timeAxisLabel, valueAxisLabel, timeSeries,
        false, true, false
    );
    this.chartPanel = new ChartPanel(chart);
    this.chartPanel.setPreferredSize(new Dimension(500, 270));
    setBaseShapeVisible(false);
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
    if (isVisible()) {
      updateSelected(nodes);
    }
  }

  private void updateSelected(Node[] nodes) {
    if (this.selectedNodes != nodes) {
      this.selectedNodes = nodes;
      this.selectedMap.clear();
      if (nodes != null) {
        for(Node node : nodes) {
          this.selectedMap.put(node, createState(node));
        }
      }
      updateCharts();
    }
  }

  @Override
  public void nodeDataReceived(SensorData data) {
    if (isVisible() && selectedMap.get(data.getNode()) != null) {
      updateCharts();
    }
  }

  @Override
  public void clearNodeData() {
    if (isVisible()) {
      updateCharts();
    }
  }

  private void updateCharts() {
    int duplicates = 0;
    int total = 0;
    series.clear();
    if (this.selectedNodes != null && server.getSensorDataCount() > 0) {
      long minute = server.getSensorData(0).getNodeTime() / 60000;
      long lastMinute = minute;
      int count = 0;
      clearState(selectedMap);
      for(int i = 0; i < server.getSensorDataCount(); i++) {
        SensorData sd = server.getSensorData(i);
        T nodeState = selectedMap.get(sd.getNode());
        if (nodeState != null) {
          if (sd.isDuplicate()) {
            duplicates++;
          } else {
            long min = sd.getNodeTime() / 60000;
            if (min != minute) {
              if (lastMinute < minute) {
                series.add(new Minute(new Date(lastMinute * 60000L)), 0);
                if (lastMinute < minute - 1) {
                  series.add(new Minute(new Date((minute - 1) * 60000L)), 0);
                }
              }
              series.add(new Minute(new Date(minute * 60000L)), getTotalDataValue(count));
              count = 0;
              lastMinute = minute + 1;
              minute = min;
            }
            count += getSensorDataValue(sd, nodeState);
          }
          total++;
        }
      }
    }
    chart.setTitle(getTitle(selectedMap.size(), total, duplicates));
  }

  protected String getTitle(int nodeCount, int dataCount, int duplicateCount) {
    return title;
  }

  protected abstract T createState(Node node);

  protected void clearState(Map<Node,T> map) {
  }

  protected int getTotalDataValue(int value) {
    return value;
  }

  protected abstract int getSensorDataValue(SensorData sd, T nodeState);

  public boolean getBaseShapeVisible() {
    return ((XYLineAndShapeRenderer)this.chart.getXYPlot().getRenderer()).getBaseShapesVisible();
  }

  public void setBaseShapeVisible(boolean visible) {
    ((XYLineAndShapeRenderer)this.chart.getXYPlot().getRenderer()).setBaseShapesVisible(visible);
  }

  public double getRangeMinimumSize() {
    return chart.getXYPlot().getRangeAxis().getAutoRangeMinimumSize();
  }

  public void setRangeMinimumSize(double size) {
    chart.getXYPlot().getRangeAxis().setAutoRangeMinimumSize(size);
  }

  public void setVisible(boolean visible) {
    updateSelected(visible ? server.getSelectedNodes() : null);
    super.setVisible(visible);
  }

}
