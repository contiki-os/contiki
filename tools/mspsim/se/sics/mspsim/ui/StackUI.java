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
 * -----------------------------------------------------------------
 *
 * StackUI
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 */

package se.sics.mspsim.ui;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;

import javax.swing.JPanel;

import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.RegisterMonitor;
import se.sics.mspsim.core.Memory.AccessMode;
import se.sics.mspsim.util.ComponentRegistry;
import se.sics.mspsim.util.MapTable;
import se.sics.mspsim.util.ServiceComponent;

public class StackUI extends JPanel implements ServiceComponent {

  private static final long serialVersionUID = 8648239617509299768L;

  private static final int STACK_FRAME = 1024;
  private int updateCyclePeriod = 2500;

  private final MSP430 cpu;
  private int heapStartAddress = -1;
  private int stackStartAddress = -1;
  private ChartPanel chartPanel;
  private LineChart minStackChart;
  private LineChart maxStackChart;
  private LineChart maxUsageStackChart;
  
//  private DotDiagram diagram;
  private int[] minData = new int[STACK_FRAME];
  private int[] maxData = new int[STACK_FRAME];
  private int[] maxUsageData = new int[STACK_FRAME];
  private int[] minCache = new int[STACK_FRAME];
  private int[] maxCache = new int[STACK_FRAME];
  private int[] maxUsageCache = new int[STACK_FRAME];
//  private String[] notes = new String[STACK_FRAME];

  private long lastCycles = 0;
  private int pos = 0;

  private boolean update = false;

  private Status status = Status.STOPPED;

  private ComponentRegistry registry;

  private ManagedWindow window;
  private RegisterMonitor registerMonitor;

  private String name;

  private boolean increasePos = false;
      
  public StackUI(MSP430 cpu) {
    this(cpu, 2500);
  }

  public StackUI(MSP430 cpu, int updateCyclePeriod) {
    super(new BorderLayout());
    this.updateCyclePeriod = updateCyclePeriod;
    this.cpu = cpu;

//    diagram = new DotDiagram(2);
//    diagram.setDotColor(0, Color.green);
//    diagram.setDotColor(1, Color.green);
//    diagram.addConstant(Color.red,
//        this.stackStartAddress - this.heapStartAddress);
//    diagram.setShowGrid(true);
//    add(diagram, BorderLayout.CENTER);
  }

  public void requestIncreasePos() {
    increasePos = true;
  }
  
  private void setup() {
      if (chartPanel != null) return;

      if (this.heapStartAddress < 0 && cpu.getDisAsm() != null) {
          MapTable mapTable = cpu.getDisAsm().getMap();
          if (mapTable != null) {
              this.heapStartAddress = mapTable.heapStartAddress;
              this.stackStartAddress = mapTable.stackStartAddress;
          }
      }
      if (this.stackStartAddress < 0) {
          // Did not find stack information in the firmware. Use CPU RAM
          // boundary as most compilers will use this as stack.
          this.stackStartAddress = cpu.config.ramStart + cpu.config.ramSize;
      }
      chartPanel = new ChartPanel();

      if (this.stackStartAddress > 0 && this.heapStartAddress > 0) {
          ConstantLineChart maxChart = new ConstantLineChart("Max", this.stackStartAddress - this.heapStartAddress);
          maxChart.setConfig("color", Color.red);
          chartPanel.addChart(maxChart);
      }

      minStackChart = new LineChart("Min Stack");
      minStackChart.setConfig("color", Color.green);
      chartPanel.addChart(minStackChart);

      maxStackChart = new LineChart("Max Stack");
      maxStackChart.setConfig("color", Color.green);
      chartPanel.addChart(maxStackChart);

      maxUsageStackChart = new LineChart("Max usage");
      maxUsageStackChart.setConfig("color", Color.blue);
      chartPanel.addChart(maxUsageStackChart);

      chartPanel.setAxisChart(maxStackChart);
      
      add(chartPanel, BorderLayout.CENTER);
      chartPanel.setMinimumSize(new Dimension(320, 200));
      setPreferredSize(new Dimension(320, 200));
      setSize(320, 200);
      setMinimumSize(new Dimension(320, 200));
      
      WindowManager wm = registry.getComponent(WindowManager.class);
      if (wm != null) {
          window = wm.createWindow("stackui");
          window.add(this);
      }

      registerMonitor = new RegisterMonitor.Adapter() {
          private int m = 0;
          public void notifyWriteBefore(int register, int data, AccessMode mode) {
              int size = stackStartAddress - data;
              if (minData[pos] > size) {
                  minData[pos] = size;
              }
              if (m < size) {
                m = size;
              }
              if (maxData[pos] < size) {
                  maxData[pos] = size;
              }
              if ((updateCyclePeriod > 0 && cpu.cpuCycles - lastCycles > updateCyclePeriod)
                  || increasePos) {
                increasePos = false;
                  lastCycles = cpu.cpuCycles;
//                    System.out.println("STACK UPDATE: " + type + "," + adr + "," + data + "," + pos);
                  
                  pos = (pos + 1) % minData.length;
                  minData[pos] = Integer.MAX_VALUE;
                  maxUsageData[pos] = m;
                  maxData[pos] = 0;
                  update = true;
                  repaint();
//                    this.notes[pos] = null;
//                    diagram.setData(0, this.minData, pos, this.minData.length);
//                    diagram.setDataWithNotes(1, this.maxData, notes, pos, this.maxData.length);
              }
          }
      };
  }

  public void paint(Graphics g) {
    if (update) {
      update = false;

      int p = pos;
      copy(this.minData, this.minCache, p);
      copy(this.maxData, this.maxCache, p);
      copy(this.maxUsageData, this.maxUsageCache, p);
      minStackChart.setData(this.minCache);
      maxStackChart.setData(this.maxCache);
      maxUsageStackChart.setData(this.maxUsageCache);
    }
    super.paint(g);
  }


  private void copy(int[] data1, int[] data2, int p) {
    if (p + 1 < data1.length) {
      System.arraycopy(data1, p + 1, data2, 0, data1.length - p - 1);
    }
    if (p > 0) {
      System.arraycopy(data1, 0, data2, data1.length - p, p);
    }
  }

  public Status getStatus() {
    return status;
  }

  public void init(String name, ComponentRegistry registry) {
    this.registry = registry;
    this.name = name;
  }

  public String getName() {
    return name;
  }

  public void start() {
    setup();
    status = Status.STARTED;
    cpu.addRegisterWriteMonitor(MSP430.SP, registerMonitor);
    window.setVisible(true);
  }

  public void stop() {
    status = Status.STOPPED;
    window.setVisible(false);
    if (registerMonitor != null) {
        cpu.removeRegisterWriteMonitor(MSP430.SP, registerMonitor);
    }
  }

}
