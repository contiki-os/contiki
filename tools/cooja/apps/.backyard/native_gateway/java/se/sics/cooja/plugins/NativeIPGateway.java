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
 * $Id: NativeIPGateway.java,v 1.12 2010/01/15 10:55:21 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JScrollPane;

import jpcap.JpcapCaptor;
import jpcap.JpcapSender;
import jpcap.NetworkInterface;
import jpcap.packet.EthernetPacket;
import jpcap.packet.IPPacket;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.GUI.RunnableInEDT;
import se.sics.cooja.Mote;
import se.sics.cooja.MotePlugin;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.SupportedArguments;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.dialogs.CompileContiki;
import se.sics.cooja.dialogs.MessageList;
import se.sics.cooja.interfaces.IPAddress;
import se.sics.cooja.interfaces.SerialPort;

@ClassDescription("Open Native IP Gateway")
@PluginType(PluginType.MOTE_PLUGIN)
@SupportedArguments(moteInterfaces = {IPAddress.class})
public class NativeIPGateway extends VisPlugin implements MotePlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(NativeIPGateway.class);

  private final static int IP_HEADER_LEN = 20;

  private final static byte SLIP_END = (byte)0300;
  private final static byte SLIP_ESC = (byte)0333;
  private final static byte SLIP_ESC_END = (byte)0334;
  private final static byte SLIP_ESC_ESC = (byte)0335;

  private final static int LABEL_WIDTH = 170;
  private final static int LABEL_HEIGHT = 20;

  private Mote mote;
  private SerialPort serialPort = null;
  private boolean registeredGateway = false;

  private ArrayList<NetworkInterfaceW> networkInterfacesAll;

  private NetworkInterfaceW networkInterface = null;
  private NetworkInterfaceW loopbackInterface = null;
  private Thread captureThread = null;
  private JpcapCaptor captor = null;
  private JpcapSender sender = null;
  private String moteIP = null;
  private byte[] networkInterfaceMAC = new byte[6];

  private boolean shutdownCaptureThread = false;

  private int inPkts = 0, outPkts = 0;
  private int inBytes = 0, outBytes = 0;

  private JLabel gatewayLabel = null;
  private JLabel interfaceLabel = null;
  private JLabel macLabel = null;
  private JLabel ipLabel = null;
  private JLabel inLabel = null;
  private JLabel outLabel = null;
  private JComboBox selectNICComboBox;
  private JCheckBox autoRegisterRoutes;

  private final boolean ON_WINDOWS;

  private final String NETMASK = "255.255.0.0";
  private String restoreRoutesCmd = null;

  private Process tunProcess = null;
  private Thread shutdownHook = null;
  private final static String TUNNEL_APP_TARGET = "minimal-net";
  private boolean shouldDisableLoopbackForwarding = false;
  private boolean shouldEnableRPFilter = false;

  private SlipState readSlipState = SlipState.STATE_OK;
  private int readSlipLength = 0;
  private final int READ_SLIP_BUFFER_SIZE = 2048;
  private byte[] readSlipBuffer = new byte[READ_SLIP_BUFFER_SIZE];

  public NativeIPGateway(Mote mote, Simulation simulation, final GUI gui) {
    super("Native IP Gateway (" + mote + ")", gui, false);
    this.mote = mote;

    /* Native OS - plugin depends on platform specific commands */
    String osName = System.getProperty("os.name").toLowerCase();
    if (osName.startsWith("win")) {
      ON_WINDOWS = true;
    } else {
      ON_WINDOWS = false;
    }

    /* Mote serial port */
    serialPort = (SerialPort) mote.getInterfaces().getLog();
    if (serialPort == null) {
      throw new RuntimeException("No mote serial port");
    }

    /* Mote IP address */
    if (mote.getInterfaces().getIPAddress() == null) {
      throw new RuntimeException("No IP address interface found (need IP address)");
    }
    moteIP = mote.getInterfaces().getIPAddress().getIPString();
    mote.getInterfaces().getIPAddress().addObserver(new Observer() {
      public void update(Observable obs, Object obj) {
        moteIP = NativeIPGateway.this.mote.getInterfaces().getIPAddress().getIPString();
        try {
          if (captor != null) {
            String[] ipSplit = moteIP.split("\\.");
            String filter = "ip and dst net " + ipSplit[0] + "." + ipSplit[1];
            if (autoRegisterRoutes.isSelected()) {
              updateNativeRoute();
            }
            captor.setFilter(filter, true);
            if (ipLabel != null) {
              ipLabel.setText(moteIP);
            }
          }
        } catch (IOException e) {
          logger.fatal("Error when updating native route: " + e.getMessage(), e);
        }
      }
    });

    /* Observe serial port for outgoing IP packets */
    serialPort.addSerialDataObserver(new Observer() {
      public void update(Observable obs, Object obj) {
        readSlipAccumulated(serialPort.getLastSerialData());
      }
    });

    autoRegisterRoutes = new JCheckBox("", true);
    autoRegisterRoutes.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (autoRegisterRoutes.isSelected()) {
          updateNativeRoute();
        }
      }
    });

    /* Create tunnel interface to capture packet on (default) */
    createTunInterface();

    /* Configure loopback interface */
    configureLoopbackInterface();

    /* Network interfaces list */
    NetworkInterface[] intfs = JpcapCaptor.getDeviceList();
    if (intfs == null || intfs.length == 0) {
      throw new RuntimeException("No network interfaces found");
    }
    networkInterfacesAll = new ArrayList<NetworkInterfaceW>();
    for (NetworkInterface i: intfs) {
      networkInterfacesAll.add(new NetworkInterfaceW(i));
    }

    selectNICComboBox = new JComboBox();

    NetworkInterfaceW tunnelInterface = null;
    for (NetworkInterfaceW intf : networkInterfacesAll) {
      if (!ON_WINDOWS && intf.intf.name.equals("lo")) {
        loopbackInterface = intf;
      }
      if (intf.intf.name != null && intf.intf.name.equals("tap0")) {
        tunnelInterface = intf;
      }
      if (intf.intf.description != null && intf.intf.description.contains("VMware Virtual Ethernet Adapter")) {
        tunnelInterface = intf;
      }

      selectNICComboBox.addItem(intf);
    }
    selectNICComboBox.addItemListener(new ItemListener() {
      public void itemStateChanged(ItemEvent e) {
        if (e.getStateChange() != ItemEvent.SELECTED) {
          return;
        }

        /* Detect selected network interface */
        NetworkInterfaceW intf =
          (NetworkInterfaceW) ((JComboBox)e.getSource()).getSelectedItem();
        if (networkInterface == intf) {
          /* Already selected */
          return;
        }

        /* Activate network interface */
        startCapturingPackets(intf);
      }
    });

    /* GUI components */
    if (GUI.isVisualized()) {
      JPanel mainPane = new JPanel();
      mainPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
      mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.Y_AXIS));

      ipLabel = addInfo(mainPane, "Mote IP Address:", moteIP);
      ipLabel.setToolTipText(null);

      addComponent(mainPane, "Route to/Capture on: ", selectNICComboBox);
      addComponent(mainPane, "Auto-register native route: ", autoRegisterRoutes);

      mainPane.add(Box.createVerticalStrut(10));
      interfaceLabel = addInfo(mainPane, "Network Interface:", "?");
      gatewayLabel = addInfo(mainPane, "Network Gateway:", "?");
      macLabel = addInfo(mainPane, "Network MAC: ", "?");
      mainPane.add(Box.createVerticalStrut(10));
      inLabel = addInfo(mainPane, "Packets to simulation:", "0");
      inLabel.setToolTipText(null);
      outLabel = addInfo(mainPane, "Packets from simulation:", "0");
      outLabel.setToolTipText(null);

      getContentPane().add(BorderLayout.CENTER,
          new JScrollPane(mainPane,
              JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
              JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED));

      pack();
      setSize(getWidth()+10, getHeight()+10);
    }

    /* Start capturing network traffic for simulated network */
    if (tunnelInterface != null) {
      startCapturingPackets(tunnelInterface);
    } else {
      startCapturingPackets(networkInterfacesAll.get(0));
    }
  }

  /**
   * Network Interface Wrapper
   */
  private class NetworkInterfaceW {
    NetworkInterface intf;
    public NetworkInterfaceW(NetworkInterface intf) {
      this.intf = intf;
    }
    public String toString() {
      if (ON_WINDOWS && intf.name != null && intf.description != null) {
        return intf.description + ": " + intf.name;
      } else if (intf.name != null && intf.description != null) {
        return intf.name + ": " + intf.description;
      }
      if (intf.name != null) {
        return intf.name;
      }
      if (intf.description != null) {
        return intf.description;
      }
      return "Unknown";
    }
  }

  private void startCapturingPackets(NetworkInterfaceW intf) {
    /* Wait for old capture thread to exit */
    if (captureThread != null &&
        captureThread.isAlive()) {
      shutdownCaptureThread = true;
      try {
        captureThread.join();
      } catch (InterruptedException e) {
        logger.fatal("Error while capturing packets: " + e.getMessage(), e);
      }
    }
    captureThread = null;

    /* Start listening on selected network interface */
    networkInterface = intf;

    /* Capture thread for incoming IP packets */
    captureThread = new Thread() {
      public void run() {

        /*logger.info("Capture thread started");*/
        try {
          captor = JpcapCaptor.openDevice(networkInterface.intf, 65535, true, 20);
          captor.setNonBlockingMode(false);
          captor.setPacketReadTimeout(20);
          String[] ipSplit = moteIP.split("\\.");
          if (ipSplit.length != 4) {
            logger.fatal("Bad mote IP address: " + moteIP);
          }
          String filter = "ip and dst net " + ipSplit[0] + "." + ipSplit[1];
          captor.setFilter(filter, true);
        } catch (IOException e) {
          logger.fatal("Error when creating captor instance: " + e.getMessage(), e);
          return;
        }

        while (!shutdownCaptureThread) {
          IPPacket packet = (IPPacket) captor.getPacket();
          if (packet == null) {
            continue;
          }

          if (!moteIP.equals("0.0.0.0")) {
            handleIncomingPacket(packet);
          }
        }

        /*logger.info("Capture thread terminated");*/
      }
    };
    shutdownCaptureThread = false;
    captureThread.start();

    /* Prepare packet sender */
    try {
      if (sender != null) {
        sender.close();
        sender = null;
      }

      if (sender == null) {
        if (loopbackInterface != null) {
          sender = JpcapSender.openDevice(loopbackInterface.intf);
        } else {
          sender = JpcapSender.openDevice(networkInterface.intf);
        }
      }
    } catch (IOException e) {
      if (!e.getMessage().contains("The operation completed successfully")) {
        logger.fatal("Can not send packets on this network interface: " + e.getMessage());
        if (sender != null) {
          sender.close();
        }
        sender = null;
      }
    }

    System.arraycopy(networkInterface.intf.mac_address, 0, networkInterfaceMAC, 0, 6);

    if (autoRegisterRoutes.isSelected()) {
      updateNativeRoute();
    }

    /* Update GUI */
    if (GUI.isVisualized()) {
      interfaceLabel.setText("" + networkInterface);
      interfaceLabel.setToolTipText(networkInterface.intf.description);
      if (networkInterface.intf.addresses.length > 0) {
        gatewayLabel.setText(networkInterface.intf.addresses[0].address.getHostAddress());
        gatewayLabel.setToolTipText(networkInterface.intf.addresses[0].address.getHostAddress());
      } else {
        gatewayLabel.setText("[no gateway]");
        gatewayLabel.setToolTipText("");
      }
      macLabel.setText(Integer.toHexString(networkInterfaceMAC[0]&0xFF) +
          ":" + Integer.toHexString(networkInterfaceMAC[1]&0xFF) +
          ":" + Integer.toHexString(networkInterfaceMAC[2]&0xFF) +
          ":" + Integer.toHexString(networkInterfaceMAC[3]&0xFF) +
          ":" + Integer.toHexString(networkInterfaceMAC[4]&0xFF) +
          ":" + Integer.toHexString(networkInterfaceMAC[5]&0xFF));
      macLabel.setToolTipText(Integer.toHexString(networkInterfaceMAC[0]&0xFF) +
          ":" + Integer.toHexString(networkInterfaceMAC[1]&0xFF) +
          ":" + Integer.toHexString(networkInterfaceMAC[2]&0xFF) +
          ":" + Integer.toHexString(networkInterfaceMAC[3]&0xFF) +
          ":" + Integer.toHexString(networkInterfaceMAC[4]&0xFF) +
          ":" + Integer.toHexString(networkInterfaceMAC[5]&0xFF));

      selectNICComboBox.setSelectedItem(networkInterface);
    }
  }

  private void configureLoopbackInterface() {
    if (ON_WINDOWS) {
      /* Nothing to configure */
    } else {
      configureLoopbackInterfaceLinux();
    }
  }

  private void configureLoopbackInterfaceLinux() {
    enableLoopbackForwardingLinux();
    disableLoopbackRPFilterLinux();
  }

  private void enableLoopbackForwardingLinux() {
    try {
      File forwardingFile = new File("/proc/sys/net/ipv4/conf/lo/forwarding");
      if (!forwardingFile.exists() || !forwardingFile.canWrite()) {
        logger.warn("No access to " + forwardingFile.getPath());
        return;
      }

      Process process = Runtime.getRuntime().exec("cat " + forwardingFile.getPath());
      process.waitFor();
      char forwardingValue = (char) process.getInputStream().read();
      /*logger.debug(forwardingFile.getPath() + " has value: " + forwardingValue);*/
      if (forwardingValue != '1' && forwardingValue != '0') {
        logger.fatal("Unknown value in " + forwardingFile.getPath() + ": " + forwardingValue);
        return;
      }

      if (forwardingValue == '1') {
        logger.info("Forwarding already enabled on loopback interface. No action.");
        return;
      }

      process = Runtime.getRuntime().exec(new String[] { "bash", "-c", "echo 1 > " + forwardingFile.getPath() });
      process.waitFor();
      logger.info("Enabled forwarding on loopback interface.");

      shouldDisableLoopbackForwarding = true;
    } catch (Exception e) {
      logger.fatal("Error when enabling forwarding: " + e.getMessage(), e);
    }
  }

  private void disableLoopbackForwardingLinux() {
    try {
      File forwardingFile = new File("/proc/sys/net/ipv4/conf/lo/forwarding");
      if (!forwardingFile.exists() || !forwardingFile.canWrite()) {
        logger.warn("No access to " + forwardingFile.getPath());
        return;
      }

      Process process = Runtime.getRuntime().exec(new String[] { "bash", "-c", "echo 0 > " + forwardingFile.getPath() });
      process.waitFor();
      logger.info("Disabled forwarding on loopback interface.");
    } catch (Exception e) {
      logger.fatal("Error when disabling forwarding: " + e.getMessage(), e);
    }
  }

  private void disableLoopbackRPFilterLinux() {
    try {
      File filterFile = new File("/proc/sys/net/ipv4/conf/lo/rp_filter");
      if (!filterFile.exists() || !filterFile.canWrite()) {
        logger.warn("No access to " + filterFile.getPath());
        return;
      }

      Process process = Runtime.getRuntime().exec("cat " + filterFile.getPath());
      process.waitFor();
      char filterValue = (char) process.getInputStream().read();
      /*logger.debug(filterFile.getPath() + " has value: " + filterValue);*/
      if (filterValue != '1' && filterValue != '0') {
        logger.fatal("Unknown value in " + filterFile.getPath() + ": " + filterValue);
        return;
      }

      if (filterValue == '0') {
        logger.info("RP filter already disabled on loopback interface. No action.");
        return;
      }

      process = Runtime.getRuntime().exec(new String[] { "bash", "-c", "echo 0 > " + filterFile.getPath() });
      process.waitFor();
      logger.info("Disabled RP filter on loopback interface.");

      shouldEnableRPFilter = true;
    } catch (Exception e) {
      logger.fatal("Error when disabling loopback RP filter: " + e.getMessage(), e);
    }
  }

  private void enableLoopbackRPFilterLinux() {
    try {
      File filterFile = new File("/proc/sys/net/ipv4/conf/lo/rp_filter");
      if (!filterFile.exists() || !filterFile.canWrite()) {
        logger.warn("No access to " + filterFile.getPath());
        return;
      }

      Process process = Runtime.getRuntime().exec(new String[] { "bash", "-c", "echo 1 > " + filterFile.getPath() });
      process.waitFor();
      logger.info("Enabled RP filter on loopback interface.");
    } catch (Exception e) {
      logger.fatal("Error when enabling loopback RP filter: " + e.getMessage(), e);
    }
  }

  private void createTunInterface() {
    if (ON_WINDOWS) {
      /*logger.warn("Cannot create tunnel network interface on Windows. Try using VMware interfaces.");*/
    } else {
      createTunInterfaceLinux();
    }
  }

  private void createTunInterfaceLinux() {
    /* Show progress bar while compiling */
    final JDialog progressDialog;
    if (GUI.isVisualized()) {
      progressDialog = new JDialog(
          (Window)GUI.getTopParentContainer(),
          "Starting Native IP Gateway plugin"
      );
    } else {
      progressDialog = null;
    }
    final MessageList output = new MessageList();
    if (GUI.isVisualized()) {
      new RunnableInEDT<Boolean>() {
        public Boolean work() {
          JProgressBar progressBar = new JProgressBar(0, 100);
          progressBar.setValue(0);
          progressBar.setString("Compiling hello-world.minimal-net...");
          progressBar.setStringPainted(true);
          progressBar.setIndeterminate(true);
          progressDialog.getContentPane().add(BorderLayout.NORTH, progressBar);
          progressDialog.getContentPane().add(BorderLayout.CENTER, new JScrollPane(output));
          progressDialog.setSize(350, 150);
          progressDialog.setLocationRelativeTo(GUI.getTopParentContainer());
          progressDialog.setVisible(true);
          GUI.setProgressMessage("Compiling hello-world.minimal-net (Native IP Gateway)");
          return true;
        }
      }.invokeAndWait();
    }

    try {
      /* Create tunnel interface by starting any Contiki minimal-net application.
       * We use the hello-world application.
       *
       * The Contiki node should have the IP address 192.168.1.2. */
      File tunContikiAppDir =
        new File(GUI.getExternalToolsSetting("PATH_CONTIKI"), "examples/hello-world");
      File tunContikiApp = new File(tunContikiAppDir, "hello-world." + TUNNEL_APP_TARGET);
      /*logger.info("Creating tap0 via " + tunContikiAppDir + "/" + tunContikiApp);*/
      Process p = CompileContiki.compile(
          GUI.getExternalToolsSetting("PATH_MAKE") + " " + tunContikiApp.getName()  + " TARGET=" + TUNNEL_APP_TARGET,
          null,
          null /* Do not observe output firmware file */,
          tunContikiAppDir,
          null,
          null,
          output,
          true
      );
      if (p.exitValue() != 0) {
        if (GUI.isVisualized()) {
          progressDialog.setVisible(false);
          progressDialog.dispose();
        }
        throw new Exception("Compile failed: " + tunContikiApp.getPath());
      }

      logger.info("> " + tunContikiApp.getName());
      tunProcess = Runtime.getRuntime().exec(new String[] { "./" + tunContikiApp.getName() }, null, tunContikiAppDir);

      /* Shutdown hook: kill minimal-net process */
      shutdownHook = new Thread(new Runnable() {
        public void run() {
          if (tunProcess == null) {
            return;
          }
          tunProcess.destroy();
          tunProcess = null;
        }
      });
      Runtime.getRuntime().addShutdownHook(shutdownHook);

      /* Waiting some time - otherwise pcap may not discover the new interface */
      Thread.sleep(250);

      logger.info("Created tap0 via " + tunContikiApp.getAbsolutePath());
    } catch (Exception e) {
      logger.fatal("Error when creating tap0: " + e.getMessage());
      logger.fatal("Try using an already existing network interface");
    }

    /* Hide progress bar */
    if (GUI.isVisualized()) {
      new RunnableInEDT<Boolean>() {
        public Boolean work() {
          progressDialog.setVisible(false);
          progressDialog.dispose();
          return true;
        }
      }.invokeAndWait();
    }
  }


  private void deleteTunInterface() {
    if (ON_WINDOWS) {
      /*deleteTunInterfaceWindows();*/
    } else {
      deleteTunInterfaceLinux();
    }
  }

  private void deleteTunInterfaceLinux() {
    if (tunProcess == null) {
      return;
    }
    try {
      tunProcess.destroy();
      logger.debug("Closed tap0 process");
    } catch (Exception e) {
      logger.fatal("Error when deleting tap0: " + e.getMessage());
    }
  }

  private void updateNativeRoute() {
    if (mote.getInterfaces().getIPAddress().getIPString().equals("0.0.0.0")) {
      /*logger.info("Not adding route, mote has no IP: " +
          mote.getInterfaces().getIPAddress().getIPString());*/
      return;
    }

    if (networkInterface.intf.addresses.length <= 0) {
      return;
    }

    if (ON_WINDOWS) {
      updateNativeRouteWindows();
    } else {
      updateNativeRouteLinux();
    }
  }

  private void updateNativeRouteWindows() {
    if (restoreRoutesCmd != null) {
      /*logger.info("Deleting old route: '" + restoreRoutesCmd + "'");*/
      try {
        logger.info("> " + restoreRoutesCmd);
        Process process = Runtime.getRuntime().exec(restoreRoutesCmd);
        process.waitFor();
      } catch (Exception e) {
        logger.fatal("Error when updating native route: " + e.getMessage(), e);
      }
      restoreRoutesCmd = null;
    }

    String moteNetIP =
      mote.getInterfaces().getIPAddress().getIPString().split("\\.")[0] + "." +
      mote.getInterfaces().getIPAddress().getIPString().split("\\.")[1] + "." +
      "0.0";
    /*logger.info("Simulation IP net : " + moteNetIP);*/

    String gatewayIP =
      (0xFF&networkInterface.intf.addresses[0].address.getAddress()[0]) + "." +
      (0xFF&networkInterface.intf.addresses[0].address.getAddress()[1]) + "." +
      (0xFF&networkInterface.intf.addresses[0].address.getAddress()[2]) + "." +
      "254"; /* Non-existing gateway - just make the packets go away */
    /*logger.info("Gateway IP: " + gatewayIP);*/

    /*logger.info("Netmask: " + NETMASK);*/


    try {
      logger.info("Registering route to simulated network");
      String cmd = "route add " + moteNetIP + " mask " + NETMASK + " " + gatewayIP;
      logger.info("> " + cmd);
      Process process = Runtime.getRuntime().exec(cmd);
      process.waitFor();
      restoreRoutesCmd = "route delete " + moteNetIP;
    } catch (Exception e) {
      logger.fatal("Error when adding route: " + e.getMessage(), e);
    }
  }

  private void updateNativeRouteLinux() {
    String moteNetIP =
      mote.getInterfaces().getIPAddress().getIPString().split("\\.")[0] + "." +
      mote.getInterfaces().getIPAddress().getIPString().split("\\.")[1] + "." +
      "0.0";
    /*logger.info("Simulation IP net : " + moteNetIP);*/

    String gatewayIP =
      (0xFF&networkInterface.intf.addresses[0].address.getAddress()[0]) + "." +
      (0xFF&networkInterface.intf.addresses[0].address.getAddress()[1]) + "." +
      (0xFF&networkInterface.intf.addresses[0].address.getAddress()[2]) + "." +
      "2";
    /*logger.info("Gateway IP: " + gatewayIP);*/

    /*logger.info("Netmask: " + NETMASK);*/

    try {
      logger.info("Registering route to simulated network");

      restoreRoutesCmd = "route del -net " + moteNetIP + " netmask 255.255.0.0";
      logger.info("> " + restoreRoutesCmd);
      Process process = Runtime.getRuntime().exec(restoreRoutesCmd);
      process.waitFor();

      String cmd = "route add -net " + moteNetIP + " netmask " + NETMASK + " gw " + gatewayIP;
      logger.info("> " + cmd);
      process = Runtime.getRuntime().exec(cmd);
      process.waitFor();

      cmd = "arp -s " + gatewayIP + " -Ds " + networkInterface.intf.name;
      logger.info("> " + cmd);
      process = Runtime.getRuntime().exec(cmd);
      process.waitFor();
    } catch (Exception e) {
      logger.fatal("Error when adding route: " + e.getMessage(), e);
    }
  }

  private void handleIncomingPacket(IPPacket packet) {
    if (!registeredGateway) {
      /* Make mote register as gateway (only needed once) */
      writeAsSlip("?IPA".getBytes(), serialPort);
      registeredGateway = true;
    }

    /*logger.debug("tot length : " + packet.len);
    logger.debug("ip proto: " + packet.protocol);
    logger.debug("ip length : " + packet.length);
    logger.debug("ip header length : " + packet.header.length);
    logger.debug("ip data length : " + packet.data.length);*/

    /* Send IP packet data (without captured non-IP header) */
    int offset = packet.len - packet.length;
    byte[] packetData = new byte[packet.header.length + packet.data.length - offset];
    System.arraycopy(packet.header, offset, packetData, 0, packet.header.length-offset);
    System.arraycopy(packet.data, 0, packetData, packet.header.length-offset, packet.data.length);
    writeAsSlip(packetData, serialPort);

    inPkts++;
    inBytes += packet.len;

    /* Update GUI */
    if (GUI.isVisualized()) {
      inLabel.setText(inPkts + " (" + inBytes + " bytes)");
    }
  }

  private void handleOutgoingPacket(byte[] packetData) {

    /* Sanity check outgoing data */
    if (packetData.length < IP_HEADER_LEN) {
      /*logger.warn("Ignoring small packet:\n" + StringUtils.hexDump(packetData));*/
      return;
    }
    if (packetData[0] != 0x45) {
      /*logger.warn("Ignoring bad header:\n" + StringUtils.hexDump(packetData));*/
      return;
    }

    if (sender == null) {
      logger.warn("No sender instance, dropping outgoing packet");
      return;
    }

    /* Create packet */
    jpcap.packet.Packet packet = new jpcap.packet.Packet();
    packet.header = new byte[0];
    packet.data = packetData;
    packet.len = packetData.length;

    /* Link ethernet frame */
    EthernetPacket ether = new EthernetPacket();
    ether.frametype = EthernetPacket.ETHERTYPE_IP;
    ether.dst_mac = networkInterfaceMAC;
    ether.src_mac = networkInterfaceMAC;

    if (loopbackInterface != null) {
      /* Use zeroed destination MAC (loopback) */
      ether.dst_mac = new byte[6];
      ether.dst_mac[0] = 0x0;
      ether.dst_mac[1] = 0x0;
      ether.dst_mac[2] = 0x0;
      ether.dst_mac[3] = 0x0;
      ether.dst_mac[4] = 0x0;
      ether.dst_mac[5] = 0x0;
    }
    packet.datalink = ether;

    /*logger.info("Sending packet (" + packet.len + " bytes) to native network: " + sender);*/
    sender.sendPacket(packet);
    outPkts++;
    outBytes += packet.len;

    /* Update GUI */
    if (GUI.isVisualized()) {
      outLabel.setText(outPkts + " (" + outBytes + " bytes)");
    }
  }

  /**
   * Wraps packet as SLIP.
   *
   * @param packet Packet data
   */
  private static void writeAsSlip(byte[] packet, SerialPort serialPort) {
    serialPort.writeByte(SLIP_END);

    for (byte b: packet) {
      if (b == SLIP_END) {
        serialPort.writeByte(SLIP_ESC);
        serialPort.writeByte(SLIP_ESC_END);
      } else if (b == SLIP_ESC) {
        serialPort.writeByte(SLIP_ESC);
        serialPort.writeByte(SLIP_ESC_ESC);
      } else {
        serialPort.writeByte(b);
      }
    }

    serialPort.writeByte(SLIP_END);
  }

  public enum SlipState {
    STATE_OK,
    STATE_ESC,
    STATE_RUBBISH
  }

  private boolean readSlipAccumulated(byte b) {
    switch (readSlipState) {

    case STATE_RUBBISH:
      readSlipLength = 0;
      if(b == SLIP_END) {
        readSlipState = SlipState.STATE_OK;
      }
      return false;

    case STATE_ESC:
      if(b == SLIP_ESC_END) {
        b = SLIP_END;
      } else if (b == SLIP_ESC_ESC) {
        b = SLIP_ESC;
      } else {
        readSlipState = SlipState.STATE_RUBBISH;
        readSlipLength = 0;
        return false;
      }
      readSlipState = SlipState.STATE_OK;
      break;

    case STATE_OK:
      if(b == SLIP_ESC) {
        readSlipState = SlipState.STATE_ESC;
        return false;
      } else if (b == SLIP_END) {
        /* Handle outgoing packet */
        byte[] packet = new byte[readSlipLength];
        System.arraycopy(readSlipBuffer, 0, packet, 0, readSlipLength);
        handleOutgoingPacket(packet);
        readSlipLength = 0;
        return true;
      }

    }

    /* Receive byte */
    readSlipBuffer[readSlipLength] = b;
    readSlipLength++;
    if (readSlipLength >= READ_SLIP_BUFFER_SIZE) {
      readSlipState = SlipState.STATE_RUBBISH;
      readSlipLength = 0;
    }

    return false;
  }

  private JLabel addInfo(JPanel pane, String desc, String value) {
    JPanel smallPane = new JPanel(new BorderLayout());
    JLabel label = new JLabel(desc);
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(value);
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    label.setToolTipText(value);
    smallPane.add(BorderLayout.CENTER, label);
    pane.add(smallPane);
    pane.add(Box.createRigidArea(new Dimension(0,10)));

    return label;
  }

  private Component addComponent(JPanel pane, String desc, Component comp) {
    JPanel smallPane = new JPanel(new BorderLayout());
    JLabel label = new JLabel(desc);
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.WEST, label);
    comp.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.CENTER, comp);
    pane.add(smallPane);
    pane.add(Box.createRigidArea(new Dimension(0,10)));

    return comp;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("network_interface")) {
        boolean ok = false;
        for (NetworkInterfaceW intf: networkInterfacesAll) {
          if (intf.intf.name.equals(element.getText())) {
            startCapturingPackets(intf);
            ok = true;
            break;
          }
        }
        if (!ok) {
          logger.warn("Network interface not available: " + element.getText());
          logger.warn("Instead capturing on default network interface: " + networkInterface);
        }
      } else if (element.getName().equals("register_routes")) {
        autoRegisterRoutes.setSelected(Boolean.parseBoolean(element.getText()));
      } else {
        return false;
      }
    }

    return true;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    element = new Element("network_interface");
    element.setText(networkInterface.intf.name);
    config.add(element);

    element = new Element("register_routes");
    element.setText("" + autoRegisterRoutes.isSelected());
    config.add(element);

    return config;
  }

  public void closePlugin() {
    if (sender != null) {
      sender.close();
    }

    shutdownCaptureThread = true;

    if (shouldDisableLoopbackForwarding) {
      disableLoopbackForwardingLinux();
    }
    if (shouldEnableRPFilter) {
      enableLoopbackRPFilterLinux();
    }

    if (restoreRoutesCmd != null) {
      /*logger.info("Deleting old route: '" + restoreRoutesCmd + "'");*/
      try {
        logger.info("> " + restoreRoutesCmd);
        Process process = Runtime.getRuntime().exec(restoreRoutesCmd);
        process.waitFor();
      } catch (Exception e) {
        logger.fatal("Error when deleting route: " + e.getMessage(), e);
      }
      restoreRoutesCmd = null;
    }

    deleteTunInterface();

    if (shutdownHook != null) {
      Runtime.getRuntime().removeShutdownHook(shutdownHook);
      shutdownHook = null;
    }
  }

  public Mote getMote() {
    return mote;
  }

}
