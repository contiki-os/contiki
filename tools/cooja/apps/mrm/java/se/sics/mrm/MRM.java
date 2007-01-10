package se.sics.mrm;

import java.util.*;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;

/**
 * This is the main class of the COOJA Multi-path Ray-tracing Medium (MRM)
 * package.
 * 
 * MRM is meant to be a replacement for the simpler radio mediums available in
 * COOJA. It is packet based and uses a 2D ray-tracing approach to approximate
 * signal strength attenuations between simulated radios. Currently the
 * ray-tracing only supports reflections and refractions through homogeneous
 * obstacles.
 * 
 * MRM provides a number of plugins for example a plugin for visualizing radio
 * environments, and a plugin for configuring the radio medium.
 * 
 * When a radio transmits data the area of interference around it will be
 * occupied for a time depending on the length of the packet sent. If the entire
 * transmission is completed without any interference the packet will be
 * delivered, otherwise nothing will be delivered.
 * 
 * Future work includes adding diffractions and scattering support.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Multi-path Ray-tracer Medium (MRM)")
public class MRM extends RadioMedium {
  private static Logger logger = Logger.getLogger(MRM.class);

  private ChannelModel currentChannelModel = null;
  private Observer simulationObserver = null;
  private Simulation mySimulation = null;
  
  // Registered members of radio medium
  private Vector<MRMMember> registeredMembers = new Vector<MRMMember>();

  private Vector<RadioTransmission> allTransmissions = new Vector<RadioTransmission>();
  private Vector<RadioTransfer> allTransfers = new Vector<RadioTransfer>();
  private Vector<RadioInterference> allInterferences = new Vector<RadioInterference>();

  
  /**
   * Notifies observers when this radio medium is starting or has finished a packet delivery.
   */
  private TransmissionsObservable radioActivityObservable = new TransmissionsObservable();
  
  /**
   * Notifies observers when this radio medium has changed settings.
   */
  private SettingsObservable settingsObservable = new SettingsObservable();
  
  /**
   * Listens to all registered radios, and fetches any new incoming radio packets.
   */
  private Observer radioObserver = new Observer() {
    public void update(Observable radio, Object obj) {
      Radio sendingRadio = (Radio) radio;
      if (sendingRadio.getLastEvent() != Radio.RadioEvent.TRANSMISSION_STARTED)
        return;

      // Locate corresponding member
      MRMMember sendingMember = null;
      for (MRMMember member: registeredMembers) {
        if (member.radio == radio) {
          sendingMember = member;
          break;
        }
      }
      if (sendingMember == null) {
        logger.fatal("MRM: Could not locate radio member - is radio registered? " + radio);
        return;
      }

      // Check that member is not receiving data
      if (sendingMember.isListeningOnTransmission()) {
        logger.fatal("MRM: Radio is trying to send data but is currently receiving! This must be fixed in Contiki!");
        return;
      }

      // Check that member is not already sending data
      for (RadioTransmission transmission: allTransmissions) {
        if (transmission.source == sendingMember) {
          logger.fatal("MRM: Radio is trying to send data but is already sending! This must be fixed in Contiki!");
          return;
        }
      }
      
      int transmissionEndTime = sendingRadio.getTransmissionEndTime();
      
      // Create transmission
      byte[] packetToSend = sendingRadio.getLastPacketTransmitted();
      RadioTransmission transmission = new RadioTransmission(sendingMember, transmissionEndTime, packetToSend);
      allTransmissions.add(transmission);
      radioActivityObservable.notifyRadioActivityChanged(); // Need to notify observers
      
      double sendingX = sendingMember.position.getXCoordinate();
      double sendingY = sendingMember.position.getYCoordinate();
      Random random = new Random();

      // Calculate how the other radios will be affected by this packet
      for (MRMMember member: registeredMembers) {
        // Ignore this sending radio
        if (member != sendingMember) {
          double receivingX = member.position.getXCoordinate();
          double receivingY = member.position.getYCoordinate();
          
          double[] probData = currentChannelModel.getProbability(sendingX, sendingY, receivingX, receivingY, -Double.MAX_VALUE);
          
          //logger.info("Probability of reception is " + probData[0]);
          //logger.info("Signal strength at destination is " + probData[1]);
          if (random.nextFloat() < probData[0]) {
            // Connection successful (if not interfered later)
            //logger.info("OK, creating connection and starting to transmit");
            tryCreateTransmission(transmission, member, probData[1]);
          } else if (probData[1] > 100) { // TODO Impossible value, what should it be?!
            // Transmission is only interference at destination
            tryCreateInterference(transmission, member, probData[1]);
          } else {
            //logger.info("Signal to low to be considered interference");
          }
        }
      }  
            
    }
  };
  
  
  /**
   * Creates a new Multi-path Ray-tracing Medium (MRM).
   */
  public MRM(Simulation simulation) {

    // Create the channel model
    currentChannelModel = new ChannelModel();

    // Register temporary plugins
    logger.info("Registering MRM plugins");
    simulation.getGUI().registerTemporaryPlugin(AreaViewer.class);
    simulation.getGUI().registerTemporaryPlugin(FormulaViewer.class);
  }

  // -- Radio Medium standard methods --

  public void registerMote(Mote mote, Simulation sim) {
    registerRadioInterface(mote.getInterfaces().getRadio(), mote.getInterfaces().getPosition(), sim);
  }

  public void unregisterMote(Mote mote, Simulation sim) {
    unregisterRadioInterface(mote.getInterfaces().getRadio(), sim);
  }

  public void registerRadioInterface(Radio radio, Position position, Simulation sim) {
    if (radio == null || position == null) {
      logger.fatal("Could not register radio: " + radio + " @ " + position);
      return;
    }
    
    // If we are not already tick observering simulation, we should be
    if (simulationObserver == null) {
      mySimulation = sim;
      simulationObserver = new Observer() {
        public void update(Observable obs, Object obj) {
          // Check if any transmission is active in the radio medium
          if (allTransmissions.isEmpty())
            return;
            
          Vector<RadioTransmission> uncompletedTransmissions = new Vector<RadioTransmission>();
          Vector<RadioTransmission> completedTransmissions = new Vector<RadioTransmission>();
          
          // Check if any transmission has completed
          for (RadioTransmission transmission: allTransmissions) {
            if (transmission.isCompleted()) {
              completedTransmissions.add(transmission);
            } else {
              uncompletedTransmissions.add(transmission);
            }
          }
          
          if (completedTransmissions.isEmpty())
            // Nothing to do
            return;

          // At least one transmission has completed - deliver data for associated transfers
          for (RadioTransmission transmission: completedTransmissions) {
            // Unregister interferences of this transmission source
            Vector<RadioInterference> intfToUnregister = new Vector<RadioInterference>();
            for (RadioInterference interference: allInterferences) {
              if (interference.mySource == transmission) {
                intfToUnregister.add(interference);
              }
            }
            for (RadioInterference interference: intfToUnregister)
              unregisterInterference(interference);
            
            // Deliver data and unregister transmission
            Vector<RadioTransfer> transToUnregister = new Vector<RadioTransfer>();
            for (RadioTransfer transfer: allTransfers) {
              if (transfer.mySource == transmission) {
                if (!transfer.interferenceDestroyedConnection()) {
                  // Don't interfer connection
                } else {
                  transfer.myDestination.radio.interferReception(0);
                }
                transToUnregister.add(transfer);
              }
            }
            for (RadioTransfer transfer: transToUnregister)
              unregisterTransmission(transfer);
          }
          
          allTransmissions = uncompletedTransmissions;
          radioActivityObservable.notifyRadioActivityChanged(); // Need to notify observers
          
          // Update all radio signal strengths
          for (MRMMember member: registeredMembers) {
            member.updateHeardSignalStrength();
          }
        }
      };
      
      sim.addTickObserver(simulationObserver);
    }

    // Save both radio and its position, and also register us as an observer to the radio
    MRMMember member = new MRMMember(radio, position);
    registeredMembers.add(member);
    radio.addObserver(radioObserver);
    radio.setCurrentSignalStrength(currentChannelModel.getParameterDoubleValue(("bg_noise_mean")));
    
    // Settings have changed - notify observers
    settingsObservable.notifySettingsChanged();
  }
  
  public void unregisterRadioInterface(Radio radioToRemove, Simulation sim) {
    // Find corresponding radio member and remove it
    MRMMember memberToRemove = null;
    for (MRMMember member: registeredMembers) {
      if (member.radio == radioToRemove) {
        memberToRemove = member;
        break;
      }
    }

    if (memberToRemove != null) {
      registeredMembers.remove(memberToRemove);
    } else
      logger.warn("MRM: Could not unregister radio: " + radioToRemove);
    
    // Settings have changed - notify observers
    settingsObservable.notifySettingsChanged();
  }
  
  public void addRadioMediumObserver(Observer observer) {
    // Add radio traffic observer to this radio medium
    radioActivityObservable.addObserver(observer);
  }

  public Observable getRadioMediumObservable() {
    return radioActivityObservable;
  }

  public void deleteRadioMediumObserver(Observer observer) {
    // Remove observer from this radio medium
    radioActivityObservable.deleteObserver(observer);
  }
  
  public RadioConnection[] getLastTickConnections() {
    logger.fatal("MRM: getLastTickConnections() not implemented");
    return null;
  }

  public void setConnectionLogger(ConnectionLogger connection) {
    logger.fatal("MRM: setConnectionLogger() not implemented");
  }

  public Collection<Element> getConfigXML() {
    // Just forwarding to current channel model
    return currentChannelModel.getConfigXML();
  }
  
  public boolean setConfigXML(Collection<Element> configXML,
      boolean visAvailable) {
    // Just forwarding to current channel model
    return currentChannelModel.setConfigXML(configXML);
  }

  
  // -- Radio Medium specific methods --

  /**
   * Adds an observer which is notified when this radio medium has
   * changed settings, such as added or removed radios.
   * 
   * @param obs New observer
   */
  public void addSettingsObserver(Observer obs) {
    settingsObservable.addObserver(obs);
  }

  /**
   * Deletes an earlier registered setting observer.
   * 
   * @param osb
   *          Earlier registered observer
   */
  public void deleteSettingsObserver(Observer obs) {
    settingsObservable.deleteObserver(obs);
  }
  
  /**
   * Returns position of given radio.
   * 
   * @param radio Registered radio
   * @return Position of given radio
   */
  public Position getRadioPosition(Radio radio) {
    // Find radio, and return its position
    for (MRMMember member: registeredMembers) {
      if (member.radio == radio) {
        return member.position;
      }
    }
    logger.fatal("MRM: Given radio is not registered!");
    return null;
  }
  
  /**
   * @return Number of registered radios.
   */
  public int getRegisteredRadioCount() {
    return registeredMembers.size();
  }

  /**
   * Returns radio at given index.
   * 
   * @param index Index of registered radio.
   * @return Radio at given index
   */
  public Radio getRegisteredRadio(int index) {
    return registeredMembers.get(index).radio;
  }

  /**
   * Returns the current channel model object, responsible for
   * all probability and transmission calculations.
   * 
   * @return Current channel model
   */
  public ChannelModel getChannelModel() {
    return currentChannelModel;
  }

  
  
  /**
   * Tries to create a new transmission between given transmission and
   * destination. The given signal strength should be the incoming signal
   * strength at the destination. This value will be used after the transmission
   * is completed in order to compare the connection with any interference.
   * 
   * @param source
   * @param destination
   * @param signalStrength
   * @return
   */
  public void tryCreateTransmission(RadioTransmission source, MRMMember destination, double signalStrength) {
    // Check if destination is already listening to a connection
    if (destination.isListeningOnTransmission()) {
      RadioInterference newInterference = new RadioInterference(source, destination, signalStrength);
      destination.heardInterferences.add(newInterference);
      
      registerInterference(newInterference);
      return;
    }

    // Create new transmission
    RadioTransfer newTransmission = new RadioTransfer(source, destination, signalStrength);
    destination.heardTransmission = newTransmission;

    registerTransmission(newTransmission);
  }
  
  public void tryCreateInterference(RadioTransmission source, MRMMember destination, double signalStrength) {
    RadioInterference newInterference = new RadioInterference(source, destination, signalStrength);
    destination.heardInterferences.add(newInterference);
    
    registerInterference(newInterference);
  }
  
  
  /**
   * ARM radio transmission.
   */
  class RadioTransmission {
    MRMMember source = null;
    int endTime = 0;
    byte[] dataToTransfer = null;

    public RadioTransmission(MRMMember source, int endTime, byte[] dataToTransfer) {
      this.source = source;
      
      this.endTime = endTime;
      
      this.dataToTransfer = dataToTransfer;
    }
    
    /**
     * @return True if no longer transmitting.
     */
    public boolean isCompleted() {
      return mySimulation.getSimulationTime() >= endTime;
    }

  
  }
  
  /**
   * ARM radio interference
   */
  class RadioInterference {
    RadioTransmission mySource;
    MRMMember myDestination;
    double interferenceSignalStrength;
    
    public RadioInterference(RadioTransmission transmission, MRMMember destination, double signalStrength) {
      this.mySource = transmission;
      this.myDestination = destination;
      this.interferenceSignalStrength = signalStrength;
    }
    
    /**
     * @return True if interference is no more.
     */
    public boolean isOld() {
      return mySource.isCompleted();
    }

  }

  /**
   * ARM radio transfers
   */
  class RadioTransfer {
    RadioTransmission mySource;
    MRMMember myDestination;
    double transmissionSignalStrength;
    double maxInterferenceSignalStrength;
    
    public RadioTransfer(RadioTransmission source, MRMMember destination, double signalStrength) {
      this.mySource = source;
      this.myDestination = destination;
      this.transmissionSignalStrength = signalStrength;
      maxInterferenceSignalStrength = -Double.MAX_VALUE;
      
      destination.radio.receivePacket(source.dataToTransfer, source.endTime);
      destination.radio.setCurrentSignalStrength(signalStrength);
    }
    
    public void addInterference(double signalStrength) {
      if (signalStrength > maxInterferenceSignalStrength) {
        maxInterferenceSignalStrength = signalStrength;
      }
      myDestination.radio.setCurrentSignalStrength(Math.max(
          maxInterferenceSignalStrength, transmissionSignalStrength));
    }
    
    /**
     * @return True if transmission is completed.
     */
    public boolean isOld() {
      return mySource.isCompleted();
    }
    
    /**
     * @return True if interference destroyed transmission
     */
    public boolean interferenceDestroyedConnection() {
      if (maxInterferenceSignalStrength + 30 > transmissionSignalStrength) {
        // Recalculating probability of delivery
        double[] probData = currentChannelModel.getProbability(
            mySource.source.position.getXCoordinate(),
            mySource.source.position.getYCoordinate(),
            myDestination.position.getXCoordinate(),
            myDestination.position.getYCoordinate(),
            maxInterferenceSignalStrength);
//        logger.info("Transfer was interfered, recalculating probability of success: " + probData[0]);
        
        if (new Random().nextFloat() >= probData[0]) {
          return true;
        }
      }
        
      return false;
    }
  }

  /**
   * Inner class used for keeping track transceivers.
   */
  class MRMMember {
    Radio radio = null;
    Position position = null;

    private RadioTransfer heardTransmission;
    private Vector<RadioInterference> heardInterferences = new Vector<RadioInterference>();
    double currentSignalStrength = -Double.MAX_VALUE;
    
    public MRMMember(Radio radio, Position position) {
      this.radio = radio;
      this.position = position;
    }
    
    public boolean isListeningOnTransmission() {
      if (heardTransmission == null)
        return false;
      
      if (heardTransmission.isOld()) {
        heardTransmission = null;
      }
      
      return heardTransmission != null;
    }
    
    /**
     * Calculates current incoming signal strength at this radio.
     * Observe, does not alter any transmissions!
     */
    public void updateHeardSignalStrength() {
      double maxSignalStrength = -Double.MAX_VALUE;

      // Get maximum interference and also update interference list
      Vector<RadioInterference> newInterferences = new Vector<RadioInterference>();
      for (RadioInterference interference: heardInterferences) {
        if (!interference.isOld()) {
          newInterferences.add(interference);
          maxSignalStrength = Math.max(maxSignalStrength,
              interference.interferenceSignalStrength);
        }
      }
      heardInterferences = newInterferences;

      if (heardTransmission != null && !heardTransmission.isOld()) {
        maxSignalStrength = Math.max(maxSignalStrength,
            heardTransmission.transmissionSignalStrength);
      } else
        heardTransmission = null;

      // Noise level
      maxSignalStrength = Math.max(maxSignalStrength, currentChannelModel
          .getParameterDoubleValue("bg_noise_mean"));
      
      currentSignalStrength = maxSignalStrength;
      radio.setCurrentSignalStrength(currentSignalStrength);
    }
    
  }

  
  public void registerInterference(RadioInterference interference) {
    allInterferences.add(interference);
    
    updateInterferences();
    radioActivityObservable.notifyRadioActivityChanged(); // Need to notify observers
  }
  
  public void registerTransmission(RadioTransfer transmission) {
    allTransfers.add(transmission);

    updateInterferences();
    radioActivityObservable.notifyRadioActivityChanged(); // Need to notify observers
  }
  
  public void unregisterInterference(RadioInterference interference) {
    updateInterferences();

    allInterferences.remove(interference);
    radioActivityObservable.notifyRadioActivityChanged(); // Need to notify observers
  }
  
  public void unregisterTransmission(RadioTransfer transmission) {
    updateInterferences();

    allTransfers.remove(transmission);
    radioActivityObservable.notifyRadioActivityChanged(); // Need to notify observers
  }

  private void updateInterferences() {
    // We need to check impact of interferences on transmissions
    for (RadioTransfer transmission: allTransfers) {
      for (RadioInterference interference: allInterferences) {
        if (interference.myDestination == transmission.myDestination) {
          transmission.addInterference(interference.interferenceSignalStrength);
        }
      }
    }
  }
  
  class TransmissionsObservable extends Observable {
    private void notifyRadioActivityChanged() {
      setChanged();
      notifyObservers();
    }
  }

  class SettingsObservable extends Observable {
    private void notifySettingsChanged() {
      setChanged();
      notifyObservers();
    }
  }

  /**
   * @return Current active transmissions
   */
  public Vector<RadioTransmission> getCurrentTransmissions() {
    return allTransmissions;
  }
  
  /**
   * @return Current active transmissions
   */
  public RadioTransmission[] getCurrentTransmissionsArray() {
    return allTransmissions.toArray(new RadioTransmission[0]);
  }
  
  /**
   * @return Current active interferences
   */
  public Vector<RadioInterference> getCurrentInterferences() {
    return allInterferences;
  }

  /**
   * @return Current active interferences
   */
  public RadioInterference[] getCurrentInterferencesArray() {
    return allInterferences.toArray(new RadioInterference[0]);
  }  
  
  /**
   * @return Current active transfers
   */
  public Vector<RadioTransfer> getCurrentTransfers() {
    return allTransfers;
  }
  
  /**
   * @return Current active transfers
   */
  public RadioTransfer[] getCurrentTransfersArray() {
    return allTransfers.toArray(new RadioTransfer[0]);
  }
  

  
  
}
