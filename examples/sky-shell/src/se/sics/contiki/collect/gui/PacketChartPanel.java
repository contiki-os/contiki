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
 * $Id: PacketChartPanel.java,v 1.1 2010/09/06 22:42:29 nifi Exp $
 *
 * -----------------------------------------------------------------
 *
 * PacketChartPanel
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 6 sep 2010
 * Updated : $Date: 2010/09/06 22:42:29 $
 *           $Revision: 1.1 $
 */

package se.sics.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.util.Date;
import java.util.HashMap;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.data.time.Minute;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;

import se.sics.contiki.collect.CollectServer;
import se.sics.contiki.collect.Node;
import se.sics.contiki.collect.SensorData;
import se.sics.contiki.collect.Visualizer;

/**
 *
 */
public class PacketChartPanel extends JPanel implements Visualizer {

  private static final long serialVersionUID = -607864439709540641L;

  protected final CollectServer server;
  protected final String title;
  protected final TimeSeries series;

  protected final JFreeChart chart;
  protected final ChartPanel chartPanel;

  private Node[] selectedNodes;
  private HashMap<Node,Node> selectedMap = new HashMap<Node,Node>();

  public PacketChartPanel(CollectServer server, String title,
      String timeAxisLabel, String valueAxisLabel) {
    super(new BorderLayout());
    this.server = server;
    this.title = title;
    this.series = new TimeSeries("Received Packets", Minute.class);
    TimeSeriesCollection timeSeries = new TimeSeriesCollection(series);
    this.chart = ChartFactory.createTimeSeriesChart(
        "Received Packets", timeAxisLabel, valueAxisLabel, timeSeries,
        false, true, false
    );
    this.chartPanel = new ChartPanel(chart);
    this.chartPanel.setPreferredSize(new Dimension(500, 270));
    setBaseShapeVisible(false);
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
      this.selectedMap.clear();
      if (nodes != null) {
        for(Node node : nodes) {
          this.selectedMap.put(node, node);
        }
      }
      if (isVisible()) {
        updateCharts();
      }
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
      long minute = server.getSensorData(0).getSystemTime() / 60000;
      long lastMinute = minute;
      int count = 0;
      for(int i = 0; i < server.getSensorDataCount(); i++) {
        SensorData sd = server.getSensorData(i);
        if (selectedMap.get(sd.getNode()) != null) {
          if (sd.isDuplicate()) {
            duplicates++;
          } else {
            long min = sd.getSystemTime() / 60000;
            if (min != minute) {
              for(; lastMinute < minute - 1; lastMinute++) {
                series.add(new Minute(new Date(lastMinute * 60000L)), 0);
              }
              series.add(new Minute(new Date(minute * 60000L)), count);
              count = 0;
              lastMinute = minute + 1;
              minute = min;
            }
            count++;
          }
          total++;
        }
      }
      if (count > 0) {
        series.addOrUpdate(new Minute(new Date(minute * 60000L)), count);
      }
    }
    int nodes = selectedMap.size();
    if (nodes > 0) {
      chart.setTitle("Received " + total + " packets from " + nodes + " node"
          + (nodes > 1 ? "s" : "")
          + (duplicates > 0 ? (" (" + duplicates + " duplicates)") : ""));
    } else {
      chart.setTitle("Received Packets");
    }
  }

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
    if (visible) {
      updateCharts();
    } else {
      series.clear();
    }
    super.setVisible(visible);
  }

}
