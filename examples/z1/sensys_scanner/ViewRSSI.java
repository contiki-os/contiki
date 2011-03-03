/*
 * Copyright (c) 2010, University of Luebeck, Germany.
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
 * Author: Carlo Alberto Boano <cboano@iti.uni-luebeck.de>, Joakim Eriksson, <joakime@sics.se>
 *
 */

import javax.swing.*;
import java.awt.*;
import java.io.*;

public class ViewRSSI extends JPanel{
  // Constants
  public static int TOTAL = 86; // Number of channel of 1 MHz sampled
  public static int DECREASE_RSSI = 2; // How many dBm the grey RSSI falls each sample
  public static int RSSI_MAX_VALUE = 100; // Maximum value obtainable from RSSI readings of CC2420.
  public static int MARGIN_BOTTOM = 20; // Margin from the bottom
  public static int MARGIN_RIGHT = 75; // Margin from the right
  public static int MARGIN_TOP = 12; // Margin from the top
  public static int INTERFERED_CHANNEL = 24; // Interfered channel
  
  public InputStream inputstr;
  private int[] rssi = new int[TOTAL];    // Array of current Noise floor values (black line)
  private int[] rssiMax = new int[TOTAL]; // Array with past Noise floor values (grey line)
  

  public ViewRSSI() {
  }

  public void paint(Graphics g) {
    Graphics2D g2 = (Graphics2D) g;
    int h = getHeight();
    int w = getWidth();    
    double factor = (h - (MARGIN_BOTTOM*1.0)) / RSSI_MAX_VALUE;
    double sSpacing = (w - MARGIN_RIGHT) / (TOTAL*1.0);
    int sWidth = (int) (sSpacing - 1);
    if (sWidth == 0) sWidth = 1;

    // Set white background in the plot
    g.setColor(Color.white);     
    g.fillRect(0, 0, w, h); 
    
    // Gradient example (ytics background)
    GradientPaint greytowhite = new GradientPaint(w-MARGIN_RIGHT,0,Color.WHITE,w, 0,Color.lightGray, false);
    g2.setPaint(greytowhite);
    g2.fillRect(w-MARGIN_RIGHT, 0, w, h);
    
    // Draw the light grey channels from 11 to 26
    double xpos = 10;	      
    for(int i=4;i<TOTAL-4;i++){    	
    	if (i==4+5*(INTERFERED_CHANNEL-11)) g.setColor(Color.cyan);    	
    	else g.setColor(Color.lightGray);
    	g.fillRect((int) (xpos+i*sSpacing), 0, (int) (sSpacing*3), h-MARGIN_BOTTOM);
    	i = i + 4;
    	g.setColor(Color.blue);	    	
    	g.drawString(String.valueOf(((i-8)/5)+11), (int) (xpos+(i-4)*sSpacing), MARGIN_TOP);
    }
    g.drawString(String.valueOf("Channel"), (int) (w-MARGIN_RIGHT+20), MARGIN_TOP);
    
    // Write the y-axis with dBm
    int base_dBm = -100; // The bottom corresponds to -100 dBm
    int ytics = 10; // How many tics on the y-axis
    for(int i=-ytics;i<=0;i++){    	    	
    	g.setColor(Color.red);
    	g.drawString(String.valueOf(((ytics+i)*(base_dBm/ytics))+"dBm"), (int) (w-MARGIN_RIGHT+20), (int) (h+i*(h/ytics))-MARGIN_BOTTOM);
    	// Dashed line for the ytics
    	final float dash1[] = {10.0f};
        final BasicStroke dashed = new BasicStroke(1.0f, BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER, 10.0f, dash1, 0.0f);        
    	g2.setStroke(dashed);
    	g2.drawLine(0, (int) (h+i*(h/ytics))-MARGIN_BOTTOM, (int) (w-MARGIN_RIGHT+7), (int) (h+i*(h/ytics))-MARGIN_BOTTOM);
    }
    
    // Write the x-axis with MHz     	    	
    g.setColor(Color.blue);	   
    int start_x_axis = 4;
    g.drawString(String.valueOf(2400+"MHz"), (int) xpos, (int) h-start_x_axis);
    g.drawString(String.valueOf(2442.5+"MHz"), (w-119)/2, (int) h-start_x_axis);
    g.drawString(String.valueOf(2485+"MHz"), w-119, (int) h-start_x_axis);
        
    // Draw the old RSSI in each MHz channel (grey)
    g.setColor(Color.gray);
    double xposition = xpos;
    for (int i = 0, n = rssi.length; i < n; i++) {
      int rssi = (int) (rssiMax[i] * factor);
      g.fillRect((int) xposition, h - MARGIN_BOTTOM - rssi, sWidth, rssi + 1);
      xposition += sSpacing;
    }

    // Draw the current RSSI in each MHz channel (black)
    g.setColor(Color.black);
    xposition = xpos;
    for (int i = 0, n = rssi.length; i < n; i++) {
      int rssiVal = (int) (rssi[i] * factor);
      g.fillRect((int) xposition, h - MARGIN_BOTTOM - rssiVal, sWidth, rssiVal + 1);
      xposition += sSpacing;
    }
  }

  public void handleInput() throws IOException {	
    BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
    while(true) {
      String line = reader.readLine();      
      if (line.startsWith("RSSI:")) {
		try {
		  String[] parts = line.substring(5).split(" ");
		  for (int i = 0, n = parts.length; i < n; i++) {
		    rssi[i] = Integer.parseInt(parts[i]); // An RSSI=0 corresponds to -100 dBm, an RSSI=100 corresponds to 0 dBm
		    if (rssi[i] > rssiMax[i]) rssiMax[i] = rssi[i];
		    else if (rssiMax[i] > 0) rssiMax[i] = rssiMax[i] - DECREASE_RSSI;
		  }
		} catch (Exception e) {		  
		  e.printStackTrace(); /* Report error, but do not fail... */
		}
		repaint();
      }
    }
  }

  
 public static void main(String[] args) throws IOException {
    JFrame win = new JFrame("RSSI Viewer");
    ViewRSSI panel;
    win.setBounds(10, 10, 590, 590);
    win.getContentPane().add(panel = new ViewRSSI());
    win.setVisible(true);
    win.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    panel.handleInput();
  }
 
}
