/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: XMLCommandHandler.java,v 1.1 2009/02/26 13:47:38 fros4943 Exp $
 */

package se.sics.chakana;

import java.io.File;
import java.io.IOException;
import java.io.StringReader;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.util.Collection;
import javax.swing.JFrame;
import javax.swing.JInternalFrame;

import org.apache.log4j.Logger;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import se.sics.chakana.EventpointEvaluator.EventpointException;
import se.sics.chakana.eventpoints.*;
import se.sics.cooja.AddressMemory;
import se.sics.cooja.CoreComm;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteMemory;
import se.sics.cooja.RadioMedium;
import se.sics.cooja.Simulation;
import se.sics.cooja.Mote.State;
import se.sics.cooja.MoteType.MoteTypeCreationException;
import se.sics.cooja.dialogs.MessageList;
import se.sics.cooja.plugins.VisState;
import se.sics.cooja.plugins.Visualizer2D;
import se.sics.cooja.radiomediums.UDGM;

/**
 * Handles incoming XML commands.
 * Each command handling call blocks during handling and returns a reply to the client.
 * 
 * A command must always be of the form:
 * <command>...</command>
 * 
 * @author Fredrik Osterlind
 */
public class XMLCommandHandler {
  public static final boolean DEBUG_OUTPUT = true;
  
  private static Logger logger = Logger.getLogger(XMLCommandHandler.class);

  public static final String XML_OK = "<ok/>";
  public static final String XML_OK_START = "<ok>";
  public static final String XML_OK_END = "</ok>";

  public static final String XML_INFO_START = "<info>";
  public static final String XML_INFO_END = "</info>";
  public static final String XML_ERROR_START = "<error>";
  public static final String XML_ERROR_END = "</error>";
  
  public static final String XML_COMMAND_NAME = "command";
  public static final String XML_COMMAND_END = "</command>";

  public static final String XML_EVENTPOINT_START = "<eventpoint>";
  public static final String XML_EVENTPOINT_END = "</eventpoint>";
  
  public static final String XML_ID_NAME = "id";
  public static final String XML_TYPE_NAME = "type";
  public static final String XML_MOTE_NAME = "mote";
  public static final String XML_VARIABLE_NAME = "variable";
  public static final String XML_MOTECOUNT_NAME = "motecount";
  public static final String XML_VALUE_NAME = "value";
  public static final String XML_TIME_NAME = "time";
  public static final String XML_SIZE_NAME = "size";
  public static final String XML_ADDRESS_NAME = "address";
  public static final String XML_VISIBLE_NAME = "visible";
  public static final String XML_TRIGGERON_NAME = "triggeron";
  public static final String XML_WATCHPOINT_INT = "int";
  public static final String XML_WATCHPOINT_VARIABLE = "variable";
  public static final String XML_WATCHPOINT_ADDRESS = "address";
  public static final String XML_TIMEPOINT_SIMULATION = "time";
  public static final String XML_TIMEPOINT_REAL = "realtime";
  public static final String XML_EVENTPOINT_RADIOMEDIUM = "radiomedium";

  private GUI myGUI;
  private EventpointEvaluator myEvaluator;
  private ChakanaPlugin myParent;

  public enum Command {
    CREATE_SIM,
    CONF_SIM,
    CONF_PLUGINS,
    CONTROL_SIM,
    SET_GUI,
    ADD_EVENTPOINT,
    READ_MEMORY,
    WRITE_MEMORY,
    CLEAR_EVENTPOINTS,
    DELETE_EVENTPOINT,
    KILL_NODES,
    GET_INFO,
    CUSTOM_COMMAND,
    EXIT_COOJA;
  }
  
  public enum ControlSimulationCommand {
    RESUME;
  }
  
  /**
   * TODO Document
   * 
   * @param gui
   * @param parent
   * @param evaluator
   */
  public XMLCommandHandler(GUI gui, ChakanaPlugin parent, EventpointEvaluator evaluator) {
    myGUI = gui;
    myEvaluator = evaluator;
    myParent = parent;
  }
  
  /**
   * @param command
   * @return
   */
  public boolean containsEntireCommand(String command) {
    return command.contains(XMLCommandHandler.XML_COMMAND_END); // TODO Ugly trick
  }

  /**
   * Handle given command (XML format).
   * Method blocks until given command has been handled.
   * 
   * @param command
   *          Command to handle
   * @return Reply to client in XML format
   */
  public String handleCommand(String command) {
    try {
      SAXBuilder builder = new SAXBuilder();
      Document doc = builder.build(new StringReader(command));
      Element root = doc.getRootElement();

      if (!root.getName().equals(XML_COMMAND_NAME)) {
        return createErrorMessage("Malformed command: " + root.getName());
      }

      String cmdTypeString = root.getAttributeValue("value");
      Command cmdType;
      try {      
        cmdType = Command.valueOf(cmdTypeString);
      } catch (IllegalArgumentException e) {
        return createErrorMessage("Unknown command: " + cmdTypeString);
      }

      Collection<Element> cmdInfo = root.getChildren();

      logger.debug("Command type: " + cmdType);
      switch (cmdType) {
      case CREATE_SIM:  
        return createSimulation(cmdInfo);
      case CONF_SIM:
        return configureSimulation(cmdInfo);
      case CONF_PLUGINS:
        return configurePlugins(cmdInfo);
      case CONTROL_SIM:  
        return controlSimulation(cmdInfo);
      case ADD_EVENTPOINT:  
        return addEventpoint(cmdInfo);
      case SET_GUI:  
        return configureGUI(cmdInfo);
      case READ_MEMORY:  
        return readMemory(cmdInfo);
      case WRITE_MEMORY:  
        return writeMemory(cmdInfo);
      case CLEAR_EVENTPOINTS:  
        return clearEventpoints();
      case DELETE_EVENTPOINT:  
        return deleteEventpoint(cmdInfo);
      case KILL_NODES:  
        return killNodes(cmdInfo);
      case GET_INFO:  
        return getInfo(cmdInfo);
      case CUSTOM_COMMAND:  
        return handleCustomCommand(cmdInfo);
      case EXIT_COOJA:  
        return shutdownCOOJA(cmdInfo);
      default: 
        return createErrorMessage("Unknown command: " + cmdType);
      }
    } catch (JDOMException e) {
      logger.fatal("Command parsing exception: " + e);
      return createErrorMessage("Invalid command syntax: " + e.getMessage());
    } catch (IOException e) {
      logger.fatal("Command parsing exception: " + e);
      return createErrorMessage("Invalid command syntax: " + e.getMessage());
    }
  }

  /**
   * Handle create new simulation command.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String createSimulation(Collection<Element> arguments) {
    Simulation simulation = new Simulation(myGUI);
    simulation.setTitle("[chakana - no title]");
    simulation.setDelayTime(0);
    simulation.setSimulationTime(0);
    simulation.setTickTime(1);

    try {
      RadioMedium radioMedium = RadioMedium.generateRadioMedium(UDGM.class,
          simulation);
      simulation.setRadioMedium(radioMedium);
    } catch (Exception e) {
      return createErrorMessage("Create simulation: Could not create radio medium: " + e.getMessage());
    }

    // Let simulation parse command arguments
    myGUI.setSimulation(simulation);
    try {
      boolean success = simulation.setConfigXML(arguments, false);
    } catch (Exception e) {
      logger.fatal("Error when configuring simulation: " + e);
      if (DEBUG_OUTPUT) {
        if (e instanceof MoteTypeCreationException) {
          MessageList compilationOutput = ((MoteTypeCreationException) e).getCompilationOutput();
          if (compilationOutput != null) {
            logger.info("Compilation output:");
            for(int i = 0; i < compilationOutput.getModel().getSize(); i++) {
              logger.info(compilationOutput.getModel().getElementAt(i));
            }
          }
          StackTraceElement[] stackTrace = e.getStackTrace();
          if (stackTrace != null) {
            logger.info("Stack trace:");
            for(StackTraceElement element: stackTrace) {
              logger.info(element);
            }
          }
          GUI.showErrorDialog(new JFrame(""), "Set DEBUG_OUTPUT to false to disable this frame", e, false); // XXX Graphical component
        }
      }
      return createErrorMessage("Create simulation: Could not configure simulation: " + e.getMessage());
    }

    return XML_OK;
  }

  /**
   * Handle configure simulation command.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String configureSimulation(Collection<Element> arguments) {
    Simulation simulation = myGUI.getSimulation();

    // Let simulation parse command arguments
    try {
      simulation.setConfigXML(arguments, false);
    } catch (Exception e) {
      logger.fatal("Error when configuring simulation: " + e);
      e.printStackTrace();
      return createErrorMessage("Could not configure simulation: " + e.getMessage());
    }
    return XML_OK;
  }

  /**
   * Handle configure plugins command.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String configurePlugins(Collection<Element> arguments) {
    Simulation simulation = myGUI.getSimulation();

    // Let GUI parse command arguments
    try {
      myGUI.setPluginsConfigXML(arguments, simulation, false);
    } catch (Exception e) {
      logger.fatal("Error when configuring plugins: " + e);
      return createErrorMessage("Could not configure plugins: " + e.getMessage());
    }

    return XML_OK;
  }

  /**
   * Handle control simulation command.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String controlSimulation(Collection<Element> arguments) {
    String reply = "";

    for (Element element : arguments) {
      ControlSimulationCommand cmdType;
      try {      
        cmdType = ControlSimulationCommand.valueOf(element.getName());
      } catch (IllegalArgumentException e) {
        return createErrorMessage("Unknown simulation control command: " + element.getName());
      }
      
      switch (cmdType) {
      case RESUME:
        // TODO Check performance degradation using try-blocks
        try {
          myEvaluator.resumeSimulation();
          reply += createEventpointMessage(myEvaluator);
        } catch (EventpointException e) {
          return createErrorMessage("Exception during simulation: " + e);
        }
        break;
      default: 
        return createErrorMessage("Unknown simulation control command");
      }
    }
    return reply;
  }

  /**
   * Handle exit COOJA command.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String shutdownCOOJA(Collection<Element> arguments) {
    myParent.performCOOJAShutdown();
    logger.info("Shutdown requested");
    return XML_OK;
  }

  /**
   * Fetch mote variable value as specified by arguments.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String readMemory(Collection<Element> arguments) {
    Simulation simulation = myGUI.getSimulation();

    String type = null;
    String mote = null;
    String variable = null;
    String size = null;
    String address = null;

    for (Element element : arguments) {
      if (element.getName().equals(XML_TYPE_NAME)) {
        type = element.getText();
      } else if (element.getName().equals(XML_MOTE_NAME)) {
        mote = element.getText();
      } else if (element.getName().equals(XML_VARIABLE_NAME)) {
        variable = element.getText();
      } else if (element.getName().equals(XML_SIZE_NAME)) {
        size = element.getText();
      } else if (element.getName().equals(XML_ADDRESS_NAME)) {
        address = element.getText();
      } else {
        return createErrorMessage("Unknown read memory parameter: " + element.getName());
      }
    }

    if (type == null)
      return createErrorMessage("No read memory type specified");
    if (mote == null)
      return createErrorMessage("No mote specified");
 
    if (mote == null)
      return createErrorMessage("No mote ID specified");
    int moteNr = Integer.parseInt(mote);
    if (moteNr < 0 || simulation.getMotesCount() <= moteNr) {
      return createErrorMessage("Bad mote ID specified: " + moteNr);
    }
    MoteMemory memory = simulation.getMote(moteNr).getMemory();
    
    // Read integer variable
    if (type.equals("int")) {
      if (variable == null)
        return createErrorMessage("No variable name specified");
      if (variable.contains(" "))
        return createErrorMessage("Variable name must not contain spaces: " + variable);
      
      if (!(memory instanceof AddressMemory))
        return createErrorMessage("Can't read mote memory variables (not address memory)");
      
      if (!((AddressMemory) memory).variableExists(variable)) {
        return createErrorMessage("Variable does not exist: " + variable);
      }
      
      int val = ((AddressMemory) memory).getIntValueOf(variable);
      return XML_OK_START + val + XML_OK_END;
    } else if (type.equals("variable")) {
      if (variable == null)
        return createErrorMessage("No variable name specified");
      if (variable.contains(" "))
        return createErrorMessage("Variable name must not contain spaces: " + variable);
      
      if (!(memory instanceof AddressMemory))
        return createErrorMessage("Can't read mote memory variables (not address memory)");
      
      if (!((AddressMemory) memory).variableExists(variable)) {
        return createErrorMessage("Variable does not exist: " + variable);
      }
      
      if (size == null)
        return createErrorMessage("No size specified");
      int sizeParsed = Integer.parseInt(size);
      if (sizeParsed < 0) {
        return createErrorMessage("Bad size specified: " + sizeParsed);
      }

      byte[] val = ((AddressMemory) memory).getByteArray(variable, sizeParsed);
      String ret = "";
      for (byte b: val)
        ret += (int) (0xff&b) + " ";
      ret = ret.trim();
      return XML_OK_START + ret + XML_OK_END;
    } else if (type.equals("address")) {
      
      if (size == null)
        return createErrorMessage("No size specified");
      int sizeParsed = Integer.parseInt(size);
      if (sizeParsed < 0) {
        return createErrorMessage("Bad size specified: " + sizeParsed);
      }

      int addressParsed = Integer.parseInt(address);
      if (addressParsed < 0) {
        return createErrorMessage("Bad start address specified: " + addressParsed);
      }

      byte[] val = memory.getMemorySegment(addressParsed, sizeParsed);
      String ret = "";
      for (byte b: val)
        ret += (int) (0xff&b) + " ";
      ret = ret.trim();
      return XML_OK_START + ret + XML_OK_END;
    }

    return createErrorMessage("Bad read memory type specified: " + type);
  }

  /**
   * Write mote variable value as specified by arguments.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String writeMemory(Collection<Element> arguments) {
    Simulation simulation = myGUI.getSimulation();

    String type = null;
    String mote = null;
    String variable = null;
    String size = null;
    String address = null;
    String value = null;

    for (Element element : arguments) {
      if (element.getName().equals(XML_TYPE_NAME)) {
        type = element.getText();
      } else if (element.getName().equals(XML_MOTE_NAME)) {
        mote = element.getText();
      } else if (element.getName().equals(XML_VARIABLE_NAME)) {
        variable = element.getText();
      } else if (element.getName().equals(XML_SIZE_NAME)) {
        size = element.getText();
      } else if (element.getName().equals(XML_ADDRESS_NAME)) {
        address = element.getText();
      } else if (element.getName().equals(XML_VALUE_NAME)) {
        value = element.getText();
      } else {
        return createErrorMessage("Unknown write memory parameter: " + element.getName());
      }
    }

    if (type == null)
      return createErrorMessage("No write memory type specified");
    if (mote == null)
      return createErrorMessage("No mote specified");
 
    if (mote == null)
      return createErrorMessage("No mote ID specified");
    int moteNr = Integer.parseInt(mote);
    if (moteNr < 0 || simulation.getMotesCount() <= moteNr) {
      return createErrorMessage("Bad mote ID specified: " + moteNr);
    }
    MoteMemory memory = simulation.getMote(moteNr).getMemory();
    
    // Write integer variable
    if (type.equals("int")) {
      if (variable == null)
        return createErrorMessage("No variable name specified");
      if (variable.contains(" "))
        return createErrorMessage("Variable name must not contain spaces: " + variable);
      
      if (!(memory instanceof AddressMemory))
        return createErrorMessage("Can't read mote memory variables (not address memory)");
      
      if (!((AddressMemory) memory).variableExists(variable)) {
        return createErrorMessage("Variable does not exist: " + variable);
      }

      if (value == null)
        return createErrorMessage("No value specified");
      if (value.contains(" "))
        return createErrorMessage("Integer value must not contain spaces: " + value);

      int val;
      try {
        val = Integer.parseInt(value);
      } catch (NumberFormatException e) {
        return createErrorMessage("Bad integer value specified: " + e);
      }
      
      ((AddressMemory) memory).setIntValueOf(variable, val);
      return XML_OK;
    } else if (type.equals("variable")) {
      if (variable == null)
        return createErrorMessage("No variable name specified");
      if (variable.contains(" "))
        return createErrorMessage("Variable name must not contain spaces: " + variable);
      
      if (!(memory instanceof AddressMemory))
        return createErrorMessage("Can't read mote memory variables (not address memory)");
      
      if (!((AddressMemory) memory).variableExists(variable)) {
        return createErrorMessage("Variable does not exist: " + variable);
      }
      
      if (size == null)
        return createErrorMessage("No size specified");
      int sizeParsed = Integer.parseInt(size);
      if (sizeParsed < 0) {
        return createErrorMessage("Bad size specified: " + sizeParsed);
      }

      if (value == null)
        return createErrorMessage("No value specified");
      String[] bytesParsed = value.split(" ");
      if (bytesParsed.length != sizeParsed)
        return createErrorMessage("Number of bytes and specified size does not match: " + bytesParsed.length + "!=" + sizeParsed);

      byte[] val = new byte[bytesParsed.length];
      for (int i=0; i < sizeParsed; i++) {
        val[i] = Byte.parseByte(bytesParsed[i]);
      }

      ((AddressMemory) memory).setByteArray(variable, val);
      return XML_OK;
    } else if (type.equals("address")) {
      
      if (size == null)
        return createErrorMessage("No size specified");
      int sizeParsed = Integer.parseInt(size);
      if (sizeParsed < 0) {
        return createErrorMessage("Bad size specified: " + sizeParsed);
      }

      int addressParsed = Integer.parseInt(address);
      if (addressParsed < 0) {
        return createErrorMessage("Bad start address specified: " + addressParsed);
      }

      if (value == null)
        return createErrorMessage("No value specified");
      String[] bytesParsed = value.split(" ");
      if (bytesParsed.length != sizeParsed)
        return createErrorMessage("Number of bytes and specified size does not match: " + bytesParsed.length + "!=" + sizeParsed);

      byte[] val = new byte[bytesParsed.length];
      for (int i=0; i < sizeParsed; i++) {
        val[i] = Byte.parseByte(bytesParsed[i]);
      }

      memory.setMemorySegment(addressParsed, val);
      return XML_OK;
    }

    return createErrorMessage("Bad write memory type specified: " + type);
  }

  /**
   * Handle add new eventpoint command.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String addEventpoint(Collection<Element> arguments) {
    Simulation simulation = myGUI.getSimulation();

    String type = null;
    String mote = null;
    String variable = null;
    String time = null;
    String size = null;
    String triggeron = null;
    String address = null;
    String count = null;

    for (Element element : arguments) {
      if (element.getName().equals(XML_TYPE_NAME)) {
        type = element.getText();
      } else if (element.getName().equals(XML_MOTE_NAME)) {
        mote = element.getText();
      } else if (element.getName().equals(XML_VARIABLE_NAME)) {
        variable = element.getText();
      } else if (element.getName().equals(XML_TIME_NAME)) {
        time = element.getText();
      } else if (element.getName().equals(XML_SIZE_NAME)) {
        size = element.getText();
      } else if (element.getName().equals(XML_ADDRESS_NAME)) {
        address = element.getText();
      } else if (element.getName().equals(XML_TRIGGERON_NAME)) {
        triggeron = element.getText();
      } else if (element.getName().equals("count")) {
        count = element.getText();
      } else {
        return createErrorMessage("Unknown eventpoint parameter: " + element.getName());
      }
    }

    logger.debug("Eventpoint type: " + type);
    if (type == null)
      return createErrorMessage("No eventpoint type specified");

    // Integer variable watchpoint
    if (type.equals(XML_WATCHPOINT_INT)) {
      if (variable == null)
        return createErrorMessage("No variable name specified");
      if (variable.contains(" "))
        return createErrorMessage("Variable name must not contain spaces: " + variable);
      if (mote == null)
        return createErrorMessage("No mote ID specified");
      int moteNr = Integer.parseInt(mote);
      if (moteNr < 0 || simulation.getMotesCount() <= moteNr) {
        return createErrorMessage("Bad mote ID specified: " + moteNr);
      }
      Mote moteObject = simulation.getMote(moteNr);

      MoteMemory memory = simulation.getMote(moteNr).getMemory();
      
      if (!(memory instanceof AddressMemory))
        return createErrorMessage("Can't write mote memory variables (not address memory)");
      
      if (!((AddressMemory) memory).variableExists(variable)) {
        return createErrorMessage("Variable does not exist: " + variable);
      }

      Eventpoint newEventpoint = new IntegerWatchpoint(moteObject, variable);
      myEvaluator.addEventpoint(newEventpoint);
      return createOkMessage(newEventpoint, simulation.getSimulationTime());
    }

    // Variable watchpoint
    if (type.equals(XML_WATCHPOINT_VARIABLE)) {
      if (variable == null)
        return createErrorMessage("No variable name specified");
      if (variable.contains(" "))
        return createErrorMessage("Variable name must not contain spaces: " + variable);
      if (mote == null)
        return createErrorMessage("No mote ID specified");
      if (size == null)
        return createErrorMessage("No size specified");
      int sizeParsed = Integer.parseInt(size);
      if (sizeParsed < 0) {
        return createErrorMessage("Bad size specified: " + sizeParsed);
      }
      int moteNr = Integer.parseInt(mote);
      if (moteNr < 0 || simulation.getMotesCount() <= moteNr) {
        return createErrorMessage("Bad mote ID specified: " + moteNr);
      }
      Mote moteObject = simulation.getMote(moteNr);

      MoteMemory memory = simulation.getMote(moteNr).getMemory();
      
      if (!(memory instanceof AddressMemory))
        return createErrorMessage("Can't write mote memory variables (not address memory)");
      
      if (!((AddressMemory) memory).variableExists(variable)) {
        return createErrorMessage("Variable does not exist: " + variable);
      }

      Eventpoint newEventpoint = new VariableWatchpoint(moteObject, variable, sizeParsed);
      myEvaluator.addEventpoint(newEventpoint);
      return createOkMessage(newEventpoint, simulation.getSimulationTime());
    }


    // Memory area watchpoint
    if (type.equals(XML_WATCHPOINT_ADDRESS)) {
      if (mote == null)
        return createErrorMessage("No mote ID specified");
      if (size == null)
        return createErrorMessage("No size specified");
      int sizeParsed = Integer.parseInt(size);
      if (sizeParsed < 0) {
        return createErrorMessage("Bad size specified: " + sizeParsed);
      }
      int addressParsed = Integer.parseInt(address);
      if (addressParsed < 0) {
        return createErrorMessage("Bad start address specified: " + addressParsed);
      }
      int moteNr = Integer.parseInt(mote);
      if (moteNr < 0 || simulation.getMotesCount() <= moteNr) {
        return createErrorMessage("Bad mote ID specified: " + moteNr);
      }
      Mote moteObject = simulation.getMote(moteNr);
      MoteMemory memory = simulation.getMote(moteNr).getMemory();
      
      Eventpoint newEventpoint = new Watchpoint(moteObject, addressParsed, sizeParsed);
      myEvaluator.addEventpoint(newEventpoint);
      return createOkMessage(newEventpoint, simulation.getSimulationTime());
    }

    // Simulation timepoint
    if (type.equals(XML_TIMEPOINT_SIMULATION)) {
      if (time == null)
        return createErrorMessage("No time specified");
      int timeParsed = Integer.parseInt(time);
      if (timeParsed < 0) {
        return createErrorMessage("Bad time specified: " + timeParsed);
      }

      Eventpoint newEventpoint = new SimulationTimepoint(simulation, timeParsed);
      myEvaluator.addEventpoint(newEventpoint);
      return createOkMessage(newEventpoint, simulation.getSimulationTime());
    }

    // Real timepoint
    if (type.equals(XML_TIMEPOINT_REAL)) {
      if (time == null)
        return createErrorMessage("No time specified");
      long timeParsed = Long.parseLong(time);
      if (timeParsed < 0) {
        return createErrorMessage("Bad time specified: " + timeParsed);
      }

      Eventpoint newEventpoint = new RealTimepoint(timeParsed);
      myEvaluator.addEventpoint(newEventpoint);
      return createOkMessage(newEventpoint, simulation.getSimulationTime());
    }

    // Radio medium event point
    if (type.equals(XML_EVENTPOINT_RADIOMEDIUM)) {
      int countInt;
      try {
        countInt = Integer.parseInt(count);
      } catch (NumberFormatException e) {
        return createErrorMessage("Bad count specified: " + e);
      }

      if (triggeron == null || triggeron.equals("all")) {
        Eventpoint newEventpoint = new RadioMediumEventpoint(simulation.getRadioMedium(), countInt);
        myEvaluator.addEventpoint(newEventpoint);
        return createOkMessage(newEventpoint, simulation.getSimulationTime());
      } else if (triggeron.equals("completed")) {
        Eventpoint newEventpoint = new TransmissionRadioMediumEventpoint(simulation.getRadioMedium(), countInt);
        myEvaluator.addEventpoint(newEventpoint);
        return createOkMessage(newEventpoint, simulation.getSimulationTime());
      } else {
        return createErrorMessage("Bad trigger on parameter: " + triggeron);
      }
    }

    return createErrorMessage("Bad eventpoint type specified: " + type);
  }

  /**
   * Handle configure GUI command.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String configureGUI(Collection<Element> arguments) {
    String visible = null;

    for (Element element : arguments) {
      if (element.getName().equals(XML_VISIBLE_NAME)) {
        visible = element.getText();
      } else {
        return createErrorMessage("Unknown GUI configure parameter: " + element.getName());
      }
    }
    
    boolean shouldBeVisible = Boolean.parseBoolean(visible);
    if (myGUI.isVisualized() != shouldBeVisible) {
      myGUI.setVisualized(shouldBeVisible);
    }
    return XML_OK;
  }

  /**
   * Handle delete eventpoint command.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String deleteEventpoint(Collection<Element> arguments) {
    String id = null;

    for (Element element : arguments) {
      if (element.getName().equals(XML_ID_NAME)) {
        id = element.getText();
      } else {
        return createErrorMessage("Unknown eventpoint parameter: " + element.getName());
      }
    }

    int idInt;
    try {
      idInt = Integer.parseInt(id);
    } catch (NumberFormatException e) {
      return createErrorMessage("Bad eventpoint ID specified: " + e);
    }

    Eventpoint eventpoint = myEvaluator.getEventpoint(idInt);
    if (eventpoint == null)
      return createErrorMessage("No eventpoint with ID " + idInt + " exists");

    myEvaluator.deleteEventpoint(idInt);
    return XML_OK;
  }

  /**
   * Handle kill nodes command.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String killNodes(Collection<Element> arguments) {
    String lowId = null;
    String highId = null;

    for (Element element : arguments) {
      if (element.getName().equals("lowmote")) {
        lowId = element.getText();
      } else if (element.getName().equals("highmote")) {
        highId = element.getText();
      } else {
        return createErrorMessage("Unknown eventpoint parameter: " + element.getName());
      }
    }

    int lowIdInt;
    int highIdInt;
    try {
      lowIdInt = Integer.parseInt(lowId);
      highIdInt = Integer.parseInt(highId);
    } catch (NumberFormatException e) {
      return createErrorMessage("Bad mote interval specified: " + e);
    }

    if (lowIdInt > highIdInt)
      return createErrorMessage("Bad mote interval specified: Low ID must be <= than high ID");
      
    if (lowIdInt < 0)
      return createErrorMessage("Bad mote interval specified: Low ID >= 0");
      
    if (myGUI.getSimulation() == null)
      return createErrorMessage("No simulation exists");
      
    if (myGUI.getSimulation().getMotesCount() < highIdInt)
      return createErrorMessage("Bad mote interval specified: Only " + myGUI.getSimulation().getMotesCount() + " motes exist");

    for (int pos=lowIdInt; pos <= highIdInt; pos++) {
      Mote mote = myGUI.getSimulation().getMote(pos);
      mote.setState(State.DEAD);
    }
    return XML_OK;
  }

  /**
   * Handle get info command.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String getInfo(Collection<Element> arguments) {
    Simulation simulation = myGUI.getSimulation();
    String type = null;
    String mote = null;
    String variable = null;

    for (Element element : arguments) {
      if (element.getName().equals(XML_TYPE_NAME)) {
        type = element.getText();
      } else if (element.getName().equals(XML_MOTE_NAME)) {
        mote = element.getText();
      } else if (element.getName().equals(XML_VARIABLE_NAME)) {
        variable = element.getText();
      } else {
        return createErrorMessage("Unknown info parameter: " + element.getName());
      }
    }

    if (type.equals("motestate")) {
      if (mote == null)
        return createErrorMessage("No mote ID specified");
      int moteNr = Integer.parseInt(mote);
      if (moteNr < 0 || simulation.getMotesCount() <= moteNr) {
        return createErrorMessage("Bad mote ID specified: " + moteNr);
      }

      State state = simulation.getMote(moteNr).getState();
      if (state == State.DEAD)
        return XML_OK_START + 0 + XML_OK_END;
      return XML_OK_START + 1 + XML_OK_END;

    } else if (type.equals("time")) {
      return XML_OK_START + myGUI.getSimulation().getSimulationTime() + XML_OK_END;
    } else if (type.equals(XML_MOTECOUNT_NAME)) {
      return XML_OK_START + myGUI.getSimulation().getMotesCount() + XML_OK_END;
    } else if (type.equals("var_address")) {
      if (variable == null)
        return createErrorMessage("No variable name specified");
      if (variable.contains(" "))
        return createErrorMessage("Variable name must not contain spaces: " + variable);
      if (mote == null)
        return createErrorMessage("No mote ID specified");
      int moteNr = Integer.parseInt(mote);
      if (moteNr < 0 || simulation.getMotesCount() <= moteNr) {
        return createErrorMessage("Bad mote ID specified: " + moteNr);
      }
      Mote moteObject = simulation.getMote(moteNr);
      MoteMemory memory = simulation.getMote(moteNr).getMemory();
        
      if (!(memory instanceof AddressMemory))
        return createErrorMessage("Can't read mote memory variable address (not address memory)");

      if (!((AddressMemory) memory).variableExists(variable)) {
        return createErrorMessage("Variable does not exist: " + variable);
      }

      int address = ((AddressMemory) memory).getVariableAddress(variable);
      return XML_OK_START + address + XML_OK_END;
    }

    return createErrorMessage("Unknown type: " + type);
  }


  /**
   * Handle custom command.
   * 
   * @param arguments Command arguments
   * @return Reply to client (XML format)
   */
  private String handleCustomCommand(Collection<Element> arguments) {
    Simulation simulation = myGUI.getSimulation();
    String action = null;
    int id = -1;
    int visible = -1;
    int hide = -1;

    for (Element element : arguments) {
      if (element.getName().equals("action")) {
        action = element.getText();
      } else if (element.getName().equals("id")) {
        String idString = element.getText();
        id = Integer.parseInt(idString);
      } else if (element.getName().equals("hide")) {
        String hideString = element.getText();
        hide = Integer.parseInt(hideString);
      } else if (element.getName().equals("visible")) {
        String visibleString = element.getText();
        visible = Integer.parseInt(visibleString);
      } else {
        return createErrorMessage("Unknown info parameter: " + element.getName());
      }
    }
    
    if (action.equals("CLICK_SINK")) {
      simulation.getMote(0).getInterfaces().getButton().clickButton();
      return XML_OK;
    }

    if (action.equals("SHOW_FIRE")) {
      if (!simulation.getGUI().isVisualized())
        return XML_OK;

      JInternalFrame[] allPlugins = simulation.getGUI().getDesktopPane().getAllFrames();

      try {
        Class<Visualizer2D> pluginClass = (Class<Visualizer2D>) simulation.getGUI().tryLoadClass(this, Visualizer2D.class, "se.sics.runes.RunesVis");
        Class[] parameterTypes = new Class[1];
        parameterTypes[0] = Boolean.TYPE;
        Method method = pluginClass.getMethod("showFire", parameterTypes);
        Object[] parameterArguments = new Object[1];
        parameterArguments[0] = new Boolean(visible==1?true:false);

        for (JInternalFrame plugin: allPlugins) {
          if (plugin.getClass() == pluginClass) {
            method.invoke(plugin, parameterArguments);
          }
        }

      } catch (Exception e) {
        return createErrorMessage("Exception: " + e.getMessage());
      }
      return XML_OK;
    }

    if (action.equals("RESET_COLORS")) {
      if (!simulation.getGUI().isVisualized())
        return XML_OK;

      JInternalFrame[] allPlugins = simulation.getGUI().getDesktopPane().getAllFrames();

      try {
        Class<Visualizer2D> pluginClass = (Class<Visualizer2D>) simulation.getGUI().tryLoadClass(this, Visualizer2D.class, "se.sics.runes.RunesVis");
        Class[] parameterTypes = new Class[0];
        Method method = pluginClass.getMethod("resetColors", parameterTypes);
        Object[] parameterArguments = new Object[0];

        for (JInternalFrame plugin: allPlugins) {
          if (plugin.getClass() == pluginClass) {
            method.invoke(plugin, parameterArguments);
          }
        }

      } catch (Exception e) {
        return createErrorMessage("Exception: " + e.getMessage());
      }
      return XML_OK;
    }

    if (action.equals("SHOW_TRUCK")) {
      if (!simulation.getGUI().isVisualized())
        return XML_OK;

      JInternalFrame[] allPlugins = simulation.getGUI().getDesktopPane().getAllFrames();

      try {
        Class<Visualizer2D> pluginClass = (Class<Visualizer2D>) simulation.getGUI().tryLoadClass(this, Visualizer2D.class, "se.sics.runes.RunesVis");

        if (hide == 0) {
          Class[] parameterTypes = new Class[0];
          Method method = pluginClass.getMethod("showTruck", parameterTypes);
          Object[] parameterArguments = new Object[0];

          for (JInternalFrame plugin: allPlugins) {
            if (plugin.getClass() == pluginClass) {
              method.invoke(plugin, parameterArguments);
            }
          }
        } else {
          Class[] parameterTypes = new Class[0];
          Method method = pluginClass.getMethod("hideTruck", parameterTypes);
          Object[] parameterArguments = new Object[0];

          for (JInternalFrame plugin: allPlugins) {
            if (plugin.getClass() == pluginClass) {
              method.invoke(plugin, parameterArguments);
            }
          }
        }

      } catch (Exception e) {
        return createErrorMessage("Exception: " + e.getMessage());
      }
      return XML_OK;
    }

    if (action.equals("INDICATE_MOTE")) {
      if (!simulation.getGUI().isVisualized())
        return XML_OK;

      JInternalFrame[] allPlugins = simulation.getGUI().getDesktopPane().getAllFrames();

      try {
        Class<Visualizer2D> pluginClass = (Class<Visualizer2D>) simulation.getGUI().tryLoadClass(this, Visualizer2D.class, "se.sics.runes.RunesVis");
        Class[] parameterTypes = new Class[1];
        parameterTypes[0] = Integer.TYPE;
        Method method = pluginClass.getMethod("indicateMote", parameterTypes);
        Object[] parameterArguments = new Object[1];
        parameterArguments[0] = new Integer(id);

        for (JInternalFrame plugin: allPlugins) {
          if (plugin.getClass() == pluginClass) {
            method.invoke(plugin, parameterArguments);
          }
        }

      } catch (Exception e) {
        return createErrorMessage("Exception: " + e.getMessage());
      }
      return XML_OK;
    }


    return createErrorMessage("Unknown action: " + action);
  }

  /**
   * Handle clear eventpoints command.
   * 
   * @return Reply to client (XML format)
   */
  private String clearEventpoints() {
    myEvaluator.clearAllEventpoints();
    return XML_OK;
  }
  
  public static String createErrorMessage(String message) {
    return XML_ERROR_START + message + XML_ERROR_END;
  }

  public static String createInfoMessage(String message) {
    return XML_INFO_START + message + XML_INFO_END;
  }
  
  public static String createOkMessage(Eventpoint eventpoint, int time) {
    return XML_OK_START + eventpoint.getID() + XML_OK_END;
  }
  
  public static String createEventpointMessage(EventpointEvaluator evaluator) {
    return XML_EVENTPOINT_START +
    evaluator.getLastTriggeredEventpointID() +
    XML_INFO_START + 
    evaluator.getTriggeredEventpoint().getMessage() +
    XML_INFO_END + 
    XML_EVENTPOINT_END;
  }
}
