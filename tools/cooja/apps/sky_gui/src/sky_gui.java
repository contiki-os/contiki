/*
 * Contiki Sky GUI Plugin
 *
 * Copyright (c) 2014, Mariorosario Prist
 * Author: Mariorosario Prist <prist.mario@gmail.com>, Federico Giuggioloni <federico.giuggioloni@gmail.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \addtogroup Contiki Sky GUI Plugin
 *
 * @{
 */

/**
 * \file sky_gui.java
 * \brief Main program for the Contiki Sky GUI Plugin. It is used to communicate with
 * the Sky Mote ADC pins, user Button, reset Button and Leds.
 * A useful Joystick functionality permits to move the mote in every position.
 * \author Mariorosario Prist <prist.mario@gmail.com>
 * \author Federico Giuggioloni <federico.giuggioloni@gmail.com>
 * \date 2014-09-28
 */

import java.awt.event.MouseMotionListener;
import java.awt.event.ActionListener;
import java.awt.event.MouseListener;
import java.awt.event.ItemListener;
import java.awt.event.ActionEvent;
import java.awt.event.MouseEvent;
import java.awt.event.ItemEvent;
import java.awt.image.BufferedImage;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.awt.Point;
import java.awt.Color;
import java.awt.Font;
import java.util.Observable;
import java.util.Hashtable;
import java.io.IOException;
import java.util.Observer;
import java.io.File;

import javax.swing.JComponent;
import javax.imageio.ImageIO;
import javax.swing.BoxLayout;
import javax.swing.ImageIcon;
import javax.swing.JCheckBox;
import javax.swing.JButton;
import javax.swing.JSlider;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.Box;

import org.apache.log4j.Priority;
import org.apache.log4j.Logger;
import org.contikios.cooja.mspmote.interfaces.MspSerial;
import org.contikios.cooja.mspmote.MspMoteTimeEvent;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.mspmote.SkyMote;
import org.contikios.cooja.MotePlugin;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.VisPlugin;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.Mote;

import se.sics.mspsim.platform.sky.SkyNode;
import se.sics.mspsim.core.PortListener;
import se.sics.mspsim.core.ADCInput;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.IOUnit;
import se.sics.mspsim.core.ADC12;

/**
 *
 * Sky Mote Graphic User Interface Plugin. Plugin to communicate with the Sky Mote
 * ADC pins, user Button, reset Button and Leds. A usefull Joystick functionality
 * permits to move the mote in every position.
 *
 */
@ClassDescription("Sky Graphics User Interface")
@PluginType(PluginType.MOTE_PLUGIN)
public class sky_gui extends VisPlugin implements MotePlugin {

  private static final long serialVersionUID = -7547521368759404109L;
  private static final int FLOW_LAYOUT_HORIZONTAL_GAP = 0;
  private static final int FLOW_LAYOUT_VERTICAL_GAP = 0;

  private static Logger logger = Logger.getLogger(sky_gui.class);

  private Simulation mSimulation;
  private Observer mObserver;
  private SkyMote skyMote;
  private Mote mote;

  private String lastLogMessage = "";
  protected Position mMotePosition;
  private MspSerial mSerialPort;
  private LEDView mLedView;

  private final JCustomCheckBox adcChecks[] = new JCustomCheckBox[8];
  private final JSlider adcSliders[] = new JSlider[8];
  private final JLabel adcLabels[] = new JLabel[8];
  private boolean mChecks[] = new boolean[8];
  private int adcValues[] = new int[8];
  private int speedMove = 40;
  private MspMoteTimeEvent releaseButtonEvent;
  private MspMoteTimeEvent pressButtonEvent;

  private Dimension mMiddleColumnDim = new Dimension(100, 230);
  private Dimension mSliderDim = new Dimension(100, 30);
  private Dimension mSerialDim = new Dimension(374, 100);
  private Dimension mColumnDim = new Dimension(137, 230);
  private Dimension mTotalDim = new Dimension(374, 380);
  private Dimension mCheckboxDim = new Dimension(20, 30);
  private Dimension mTopDim = new Dimension(374, 33);

  LEDModel mLedModel = new LEDModel();

  /*---------------------------------------------------------------------------*/
  /**
   * Connection between Slider and ADC Input.
   *
   */
  protected class ADCClient implements ADCInput {
    byte mPin;

    public ADCClient(byte pin) {
      mPin = pin;
    }
    /**
     *
     */
    @Override
    public int nextData() {
      return adcValues[mPin];
    }
  }
  /*---------------------------------------------------------------------------*/
  /**
   * Class Constructor.
   *
   * @param mote
   * @param simulation
   * @param gui
   */
  public sky_gui(Mote mote, Simulation simulation, final Cooja gui) {
    super("Sky GUI (" + mote + ")", gui, false);

    this.mote = mote;
    skyMote = (SkyMote)mote;
    mSimulation = skyMote.getSimulation();

    if(Cooja.isVisualized()) {
      this.getContentPane().setSize(300, 300);
      this.setResizable(false);
      getContentPane().setLayout(new BorderLayout());
      getContentPane().add(getLeftInterface(), BorderLayout.LINE_START);
      getContentPane().add(getRightInterface(), BorderLayout.LINE_END);

      try {
        getContentPane().add(getCenterImage(), BorderLayout.CENTER);
      } catch(IOException e) {
        e.printStackTrace();
      }

      getContentPane().add(getTopInterface(), BorderLayout.PAGE_START);
      getContentPane().add(getBottomInterface(), BorderLayout.PAGE_END);
    }

    pressButtonEvent = new MspMoteTimeEvent((SkyMote)mote, 0) {
      public void execute(long t) {
        skyMote.skyNode.getButton().setPressed(true);
      }
    };

    releaseButtonEvent = new MspMoteTimeEvent((SkyMote)mote, 0) {
      public void execute(long t) {
        skyMote.skyNode.getButton().setPressed(false);
      }
    };

    getContentPane().setSize(mTotalDim);
    getContentPane().setPreferredSize(mTotalDim);
    pack();

    logger.debug("Starting to listen on port " + (18000 + mote.getID()));
  }
  /*---------------------------------------------------------------------------*/
  /**
   *
   * Connection between ADC pin and Slider.
   *
   */
  protected class ADCret implements ADCInput {
    private JSlider myslider;

    ADCret(JSlider slider) {
      myslider = slider;
    }
    /**
     *
     */
    public int nextData() {
      int value = 1023;

      if(myslider == null) {
        return value;
      } else {
        value = myslider.getValue();
      }
      return value;
    }
  }
  /*---------------------------------------------------------------------------*/
  /**
   *
   * Sky Mote GUI Left side.
   *
   * @return JPanel
   */
  public JPanel getLeftInterface() {
    JPanel panel = new JPanel();
    panel.setLayout(new GridLayout(5, 1, 0, 0));

    panel.add(getLedPanel());
    panel.add(getAdcPanel(7, true));
    panel.add(getAdcPanel(6, true));
    panel.add(getAdcPanel(3, true));
    panel.add(getAdcPanel(2, true));

    panel.setPreferredSize(mColumnDim);
    panel.setSize(mColumnDim);

    return panel;
  }
  /*---------------------------------------------------------------------------*/
  /**
   *
   * Sky Mote GUI Right side.
   *
   * @return JPanel
   */
  public JPanel getRightInterface() {
    JPanel panel = new JPanel();
    panel.setLayout(new GridLayout(5, 1, 0, 0));

    panel.add(getButtonsPanel());
    panel.add(getAdcPanel(4, false));
    panel.add(getAdcPanel(5, false));
    panel.add(getAdcPanel(0, false));
    panel.add(getAdcPanel(1, false));

    panel.setSize(mColumnDim);
    panel.setPreferredSize(mColumnDim);

    return panel;
  }
  /*---------------------------------------------------------------------------*/
  /**
   *
   * Sky Mote GUI Bottom side.
   *
   * @return JPanel
   */
  public JPanel getBottomInterface() {
    mSerialPort = (MspSerial)skyMote.getInterfaces().getLog();

    mSerialPort.addSerialDataObserver(new Observer() {
                                        @Override
                                        public void update(Observable arg0, Object arg1) {
                                          char lastData = (char)mSerialPort.getLastSerialData();
                                          if(lastData == '\n') {
                                            lastLogMessage = "";
                                          } else {
                                            lastLogMessage += lastData;
                                          }
                                        }
                                      });

    JPanel serialInterface = mSerialPort.getInterfaceVisualizer();
    serialInterface.setPreferredSize(mSerialDim);
    serialInterface.setSize(mSerialDim);

    return serialInterface;
  }
  /*---------------------------------------------------------------------------*/
  /**
   *
   * Sky Mote GUI Center side.
   *
   * @return JPanel
   * @throws IOException
   *    File not found
   */
  public JPanel getCenterImage() throws IOException {
    JPanel panel = new JPanel();
    panel.setLayout(new FlowLayout(FlowLayout.CENTER,
                                   FLOW_LAYOUT_HORIZONTAL_GAP, FLOW_LAYOUT_VERTICAL_GAP));

    BufferedImage myPicture = ImageIO.read(new File(
                                             "../apps/sky_gui/img/sky.png"));
    JLabel picLabel = new JLabel(new ImageIcon(myPicture));
    picLabel.setAlignmentX(JLabel.CENTER_ALIGNMENT);
    picLabel.setAlignmentY(JLabel.CENTER_ALIGNMENT);
    panel.add(picLabel);

    panel.setSize(mMiddleColumnDim);
    panel.setPreferredSize(mMiddleColumnDim);

    return panel;
  }
  /*---------------------------------------------------------------------------*/
  /**
   *
   * Sky Mote GUI Top side.
   *
   * @return JPanel
   */
  public JPanel getTopInterface() {
    mMotePosition = skyMote.getInterfaces().getPosition();

    JPanel panel = new JPanel();
    panel.setLayout(new FlowLayout(FlowLayout.CENTER,
                                   FLOW_LAYOUT_HORIZONTAL_GAP, FLOW_LAYOUT_VERTICAL_GAP));

    JLabel jLabel = new JLabel("Joystick");
    jLabel.setFont(jLabel.getFont().deriveFont(40));
    panel.add(jLabel);
    panel.add(Box.createRigidArea(new Dimension(10, 0)));
    Joystick levetta = new Joystick();
    panel.add(levetta);

    panel.setSize(mTopDim);
    panel.setPreferredSize(mTopDim);

    return panel;
  }
  /*---------------------------------------------------------------------------*/
  /**
   * Get ADC Panel. It contains the checkbox, label and slider associated to
   * the ADC pin.
   *
   * @param pin - ADC pin.
   * @param left - Left or Right position (true or false) in the Sky Mote GUI.
   *
   * @return JPanel
   */
  public JPanel getAdcPanel(int pin, boolean left) {
    int FPS_MIN = 0;
    int FPS_MAX = 1024;
    int FPS_INIT = 0;
    int align = left ? FlowLayout.RIGHT : FlowLayout.LEFT;

    JPanel panel = new JPanel();
    panel.setLayout(new FlowLayout(align, FLOW_LAYOUT_HORIZONTAL_GAP,
                                   FLOW_LAYOUT_VERTICAL_GAP));

    JPanel labelPanel = new JPanel();
    labelPanel.setLayout(new FlowLayout(align, FLOW_LAYOUT_HORIZONTAL_GAP,
                                        FLOW_LAYOUT_VERTICAL_GAP));

    adcChecks[pin] = new JCustomCheckBox(pin);
    adcChecks[pin].addItemListener(mChangeListener);

    adcSliders[pin] = new JSlider(FPS_MIN, FPS_MAX, FPS_INIT);

    Hashtable<Integer, JLabel> labelTable = new Hashtable<Integer, JLabel>();
    labelTable.put(new Integer(FPS_MIN), new JLabel("0V"));
    labelTable.put(new Integer(FPS_MAX / 2), new JLabel("1.8V"));
    labelTable.put(new Integer(FPS_MAX), new JLabel("3.3V"));
    labelTable.get(new Integer(FPS_MIN)).setFont(new Font("Serif", Font.ITALIC, 8));
    labelTable.get(new Integer(FPS_MAX / 2)).setFont(new Font("Serif", Font.ITALIC, 8));
    labelTable.get(new Integer(FPS_MAX)).setFont(new Font("Serif", Font.ITALIC, 8));

    adcSliders[pin].setLabelTable(labelTable);
    adcSliders[pin].setPaintLabels(true);
    Font font = new Font("Serif", Font.ITALIC, 8);
    adcSliders[pin].setFont(font);

    if(pin == 4) {
      adcLabels[pin] = new JLabel("PAR ADC " + pin);
    } else if(pin == 5) {
      adcLabels[pin] = new JLabel("TRS ADC " + pin);
    } else {
      adcLabels[pin] = new JLabel("ADC " + pin);
    }

    adcSliders[pin].setPreferredSize(mSliderDim);
    adcChecks[pin].setPreferredSize(mCheckboxDim);
    adcSliders[pin].setSize(mSliderDim);
    adcChecks[pin].setSize(mCheckboxDim);

    if(left) {
      panel.add(adcSliders[pin]);
      panel.add(adcChecks[pin]);
      labelPanel.add(adcLabels[pin]);
    } else {
      panel.add(adcChecks[pin]);
      labelPanel.add(adcLabels[pin]);
      panel.add(adcSliders[pin]);
    }

    managePin(pin);

    JPanel ret = new JPanel();
    ret.setLayout(new BoxLayout(ret, BoxLayout.Y_AXIS));
    ret.setPreferredSize(new Dimension(80, 100));
    ret.add(labelPanel);
    ret.add(panel);
    return ret;
  }
  /*---------------------------------------------------------------------------*/
  /**
   * Get a Leds Interface Panel.
   *
   * @return JPanel
   */
  public JPanel getLedPanel() {
    JPanel panel = new JPanel();

    panel.setLayout(new FlowLayout(FlowLayout.CENTER,
                                   FLOW_LAYOUT_HORIZONTAL_GAP, FLOW_LAYOUT_VERTICAL_GAP));
    panel.setPreferredSize(new Dimension(100, 115));

    mLedView = new LEDView(new Color(255, 0, 0), new Color(0, 255, 0),
                           new Color(0, 0, 255));

    IOUnit unit = skyMote.getCPU().getIOUnit("Port 5");
    ((IOPort)unit).addPortListener(mLedView.getPortListener());

    mObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        mLedView.repaint();
      }
    };

    mLedModel.addObserver(mObserver);
    panel.add(mLedView);

    return panel;
  }
  /*---------------------------------------------------------------------------*/
  /**
   *
   * Get Buttons Panel.
   *
   * @return JPanel
   */
  public JPanel getButtonsPanel() {
    JPanel panel = new JPanel();

    panel.setLayout(new FlowLayout(FlowLayout.CENTER,
                                   FLOW_LAYOUT_HORIZONTAL_GAP, FLOW_LAYOUT_VERTICAL_GAP));

    panel.add(getButton(new ActionListener() {
                          @Override
                          public void actionPerformed(ActionEvent arg0) {
                            premiBottone();
                          }
                        }, "Button"));

    panel.add(getButton(new ActionListener() {
                          @Override
                          public void actionPerformed(ActionEvent arg0) {
                            skyMote.getCPU().reset();
                          }
                        }, "Reset"));

    return panel;
  }
  /*---------------------------------------------------------------------------*/
  /**
   * Get Button Panel.
   *
   * @param listener
   *
   * @param text - Button Text.
   *
   * @return JPanel
   *
   */
  public JPanel getButton(ActionListener listener, String text) {
    JPanel panel = new JPanel();
    panel.setLayout(new FlowLayout(FlowLayout.LEFT,
                                   FLOW_LAYOUT_HORIZONTAL_GAP, FLOW_LAYOUT_VERTICAL_GAP));

    JButton button = new JButton(text);
    button.addActionListener(listener);
    panel.add(button);

    return panel;
  }
  /*---------------------------------------------------------------------------*/
  /**
   * Event of Button Press.
   */
  public void premiBottone() {
    skyMote.getInterfaces().getButton().clickButton();
  }
  /*---------------------------------------------------------------------------*/
  /**
   *
   * Checkbox ItemListener. Activate or Deactivate ADC Slider.
   *
   */
  private ItemListener mChangeListener = new ItemListener() {

    @Override
    public void itemStateChanged(ItemEvent arg0) {
      switch(arg0.getStateChange()) {
      case ItemEvent.SELECTED:
      case ItemEvent.DESELECTED:
        JCustomCheckBox check = (JCustomCheckBox)arg0.getSource();

        int n = check.getPosition();

        mChecks[n] = !mChecks[n];

        managePin(n);
        break;
      }
    }
  };
  /*---------------------------------------------------------------------------*/
  /**
   *
   * Joystick Class used to move the Sky Mote.
   *
   */
  private class Joystick extends JComponent {

    private static final long serialVersionUID = -2507869745481402032L;

    private final Dimension mDimensions = new Dimension(30, 30);
    private static final double SMALL_RATIO = 0.75;

    private Point mMouseBase = new Point(mDimensions.width / 2,
                                         mDimensions.height / 2);

    private boolean mMousePressed = false;
    private boolean mMouseDragged = false;
    private int currX = mMouseBase.x;
    private int currY = mMouseBase.y;
    private int mOvalHalfWidth = 7;

    public Joystick() {
      super();

      setPreferredSize(mDimensions);
      setSize(mDimensions);
      addMouseMotionListener(mLevettaMotionListener);
      addMouseListener(mLevettaListener);
    }
    /**
     *
     */
    private Runnable mUpdateCycle = new Runnable() {

      @Override
      public void run() {
        while(mMousePressed) {
          repaint();
          try {
            Thread.sleep(10);
          } catch(InterruptedException e) {
            e.printStackTrace();
          }
        }
      }
    };

    private MouseListener mLevettaListener = new MouseListener() {

      @Override
      public void mouseReleased(MouseEvent e) {
        mMousePressed = false;
        mMouseDragged = false;
        currX = mMouseBase.x;
        currY = mMouseBase.y;

        repaint();
      }
      /**
       *
       */
      @Override
      public void mousePressed(MouseEvent e) {
        mMousePressed = true;

        Thread thread = new Thread(mUpdateCycle);
        thread.start();
      }
      /**
       *
       */
      @Override
      public void mouseExited(MouseEvent e) {
      }
      /**
       *
       */
      @Override
      public void mouseEntered(MouseEvent e) {
      }
      /**
       *
       */
      @Override
      public void mouseClicked(MouseEvent e) {
      }
    };

    private MouseMotionListener mLevettaMotionListener = new MouseMotionListener() {
      /**
       *
       */
      @Override
      public void mouseMoved(MouseEvent e) {
      }
      /**
       *
       */
      @Override
      public void mouseDragged(MouseEvent e) {
        if(!mMouseDragged) {
        }

        currX = e.getX();
        currY = e.getY();

        mMouseDragged = true;
      }
    };
    /**
     *
     */
    @Override
    public void paintComponent(Graphics g) {
      super.paintComponent(g);

      /* Get the X and Y movement */
      double xIncr = currX - mMouseBase.getX();
      double yIncr = currY - mMouseBase.getY();

      /* Set new position */
      double x = mMotePosition.getXCoordinate() + xIncr / speedMove;
      double y = mMotePosition.getYCoordinate() + yIncr / speedMove;

      mMotePosition.setCoordinates(x, y, 0);

      g.setColor(Color.LIGHT_GRAY);
      g.fillOval(0, 0, mDimensions.width, mDimensions.height);

      g.setColor(Color.BLACK);
      g.drawLine(0, (int)mMouseBase.getY(), mDimensions.width,
                 (int)mMouseBase.getY());

      g.setColor(Color.BLACK);
      g.drawLine((int)mMouseBase.getX(), 0, (int)mMouseBase.getY(),
                 mDimensions.height);

      g.setColor(Color.BLACK);
      g.drawLine((int)mMouseBase.getX(), (int)mMouseBase.getY(), currX,
                 currY);

      g.fillOval(currX - mOvalHalfWidth, currY - mOvalHalfWidth,
                 mOvalHalfWidth * 2, mOvalHalfWidth * 2);

      g.setColor(Color.GRAY);
      g.drawLine((int)mMouseBase.getX(), (int)mMouseBase.getY(), currX,
                 currY);

      g.fillOval((int)(currX - (mOvalHalfWidth * SMALL_RATIO)),
                 (int)(currY - (mOvalHalfWidth * SMALL_RATIO)),
                 (int)(mOvalHalfWidth * SMALL_RATIO) * 2,
                 (int)(mOvalHalfWidth * SMALL_RATIO) * 2);
    }
  }
  /*---------------------------------------------------------------------------*/
  /**
   * Set the ADC input in relation to the Checkbox checked.
   *
   * @param n - Pin to control.
   */
  public void managePin(int n) {
    ADC12 adc = (ADC12)skyMote.getCPU().getIOUnit("ADC12");

    if(mChecks[n]) {
      logger.log(Priority.DEBUG, "ADC input Connected to the pin " + n);

      adc.setADCInput(n, new ADCret(adcSliders[n]));

      showSlider(n);
    } else {
      adc.setADCInput(n, null);
      hideSlider(n);
    }
  }
  /*---------------------------------------------------------------------------*/
  /**
   *
   * Hide ADC Slider.
   *
   * @param n - ADC Slider to hide.
   */
  public void hideSlider(int n) {
    adcSliders[n].setEnabled(false);
    adcSliders[n].setVisible(false);
  }
  /*---------------------------------------------------------------------------*/
  /**
   *
   * Show ADC Slider.
   *
   * @param n - ADC Slider to show.
   */
  public void showSlider(int n) {
    adcSliders[n].setEnabled(true);
    adcSliders[n].setVisible(true);
  }
  /*---------------------------------------------------------------------------*/
  /**
   * Observe and Update Led Interface.
   *
   */
  public class LEDModel extends Observable {
    public void ledUpdate() {
      setChanged();
      notifyObservers();
    }
  }
  /*---------------------------------------------------------------------------*/
  /**
   * Custom Led Component.
   *
   */
  public class LEDView extends JComponent {
    private static final long serialVersionUID = 795919797134270088L;
    private Dimension mDimensions;
    private Color darkColors[];
    private boolean leds[];
    private Color colors[];

    PortListener portListener = new PortListener() {

      @Override
      public void portWrite(IOPort source, int data) {
        leds[0] = (data & SkyNode.BLUE_LED) == 0;
        leds[1] = (data & SkyNode.GREEN_LED) == 0;
        leds[2] = (data & SkyNode.RED_LED) == 0;

        mLedModel.ledUpdate();
      }
    };

    public PortListener getPortListener() {
      return portListener;
    }
    /**
     *
     */
    public LEDView(int numberOfLeds) {
      super();

      leds = new boolean[numberOfLeds];
      colors = new Color[numberOfLeds];
      darkColors = new Color[numberOfLeds];

      for(int i = 0; i < numberOfLeds; i++) {
        leds[i] = false;
        colors[i] = new Color(255, 255, 255);
        darkColors[i] = new Color(0, 0, 0);
      }

      mDimensions = new Dimension(120, 30);

      setSize(mDimensions);
      setPreferredSize(mDimensions);
      setVisible(true);
      setEnabled(true);
    }
    /**
     *
     */
    public LEDView() {
      this(3);
    }
    /**
     *
     */
    public LEDView(Color ... cols) {
      this(3);

      for(int i = 0; i < cols.length; i++) {
        colors[i] = cols[i];

        int dr = Math.max(cols[i].getRed() - 155, 0);
        int dg = Math.max(cols[i].getGreen() - 155, 0);
        int db = Math.max(cols[i].getBlue() - 155, 0);

        darkColors[i] = new Color(dr, dg, db);
      }
    }
    /**
     *
     */
    public boolean isLedOn(int which) {
      return leds[which];
    }
    /**
     *
     */
    @Override
    public void paintComponent(Graphics g) {
      super.paintComponent(g);

      Color current;
      int ledRadius = 20;
      int xIncrement = 30;
      int currentX = (getWidth() - leds.length * xIncrement) / 2;
      int currentY = getHeight() / 2 - ledRadius / 2;

      for(int i = 0; i < leds.length; i++) {
        if(leds[i]) {
          current = colors[i];
        } else {
          current = darkColors[i];
        }

        g.setColor(current);
        g.fillOval(currentX, currentY, ledRadius, ledRadius);
        g.setColor(Color.BLACK);
        g.drawOval(currentX, currentY, ledRadius, ledRadius);

        currentX = currentX + xIncrement;
      }
    }
  }
  /*---------------------------------------------------------------------------*/
  /**
   * Custom CheckBox Component to manage ADC Pin.
   *
   */
  private class JCustomCheckBox extends JCheckBox {

    private static final long serialVersionUID = 5663754769371544564L;
    private int position = 0;

    public JCustomCheckBox(int posizione) {
      super();
      this.position = posizione;
      setSelected(false);
    }
    public int getPosition() {
      return position;
    }
  }
  /*---------------------------------------------------------------------------*/
  @Override
  public void closePlugin() {
    super.closePlugin();

    if(mObserver != null) {
      mLedModel.deleteObserver(mObserver);
    }
  }
  /*---------------------------------------------------------------------------*/
  @Override
  public Mote getMote() {
    return mote;
  }
}
/*---------------------------------------------------------------------------*/

/** @} */
