package se.sics.mrm;

import java.awt.*;
import java.awt.event.*;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.text.NumberFormat;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * This plugin allows a user to reconfigure current radio channel parameters.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("MRM - Formula Viewer")
@PluginType(PluginType.SIM_PLUGIN)
public class FormulaViewer extends se.sics.cooja.VisPlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(FormulaViewer.class);

  private Simulation currentSimulation;
  private MRM currentRadioMedium;
  private ChannelModel currentChannelModel;

  private static Dimension labelDimension = new Dimension(240, 20);
  private static NumberFormat doubleFormat = NumberFormat.getNumberInstance();
  private static NumberFormat integerFormat = NumberFormat.getIntegerInstance();
  
  private Vector<JFormattedTextField> allIntegerParameters = new Vector<JFormattedTextField>();
  private Vector<JFormattedTextField> allDoubleParameters = new Vector<JFormattedTextField>();
  private Vector<JCheckBox> allBooleanParameters = new Vector<JCheckBox>();
  
  private JPanel areaGeneral;
  private JPanel areaTransmitter;
  private JPanel areaReceiver;
  private JPanel areaRayTracer;
  private JPanel areaShadowing;

  /**
   * Creates a new formula viewer.
   * 
   * @param simulationToVisualize Simulation which holds the MRM channel model.
   */
  public FormulaViewer(Simulation simulationToVisualize, GUI gui) {
    super("MRM - Formula Viewer", gui);
    
    currentSimulation = simulationToVisualize;
    currentRadioMedium = (MRM) currentSimulation.getRadioMedium();
    currentChannelModel = currentRadioMedium.getChannelModel();

    // -- Create and add GUI components --
    JPanel allComponents = new JPanel();
    allComponents.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
    allComponents.setLayout(new BoxLayout(allComponents, BoxLayout.Y_AXIS));

    JScrollPane scrollPane = new JScrollPane(allComponents);
    scrollPane.setPreferredSize(new Dimension(500,400));
    scrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
    scrollPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
    setContentPane(scrollPane);

    JPanel collapsableArea;
    
    // General parameters
    collapsableArea = createCollapsableArea("General parameters", allComponents);
    areaGeneral = collapsableArea;

    addBooleanParameter(
        "apply_random",
        currentChannelModel.getParameterDescription("apply_random"),
        collapsableArea,
        currentChannelModel.getParameterBooleanValue("apply_random")
    );

    addDoubleParameter(
        "snr_threshold",
        currentChannelModel.getParameterDescription("snr_threshold"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("snr_threshold")
    );

    addDoubleParameter(
        "bg_noise_mean",
        currentChannelModel.getParameterDescription("bg_noise_mean"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("bg_noise_mean")
    );

    addDoubleParameter(
        "bg_noise_var",
        currentChannelModel.getParameterDescription("bg_noise_var"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("bg_noise_var")
    );

    addDoubleParameter(
        "system_gain_mean",
        currentChannelModel.getParameterDescription("system_gain_mean"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("system_gain_mean")
    );
    
    addDoubleParameter(
        "system_gain_var",
        currentChannelModel.getParameterDescription("system_gain_var"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("system_gain_var")
    );
    
    addDoubleParameter(
        "wavelength",
        currentChannelModel.getParameterDescription("wavelength"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("wavelength")
    );

    // Transmitter parameters
    collapsableArea = createCollapsableArea("Transmitter parameters", allComponents);
    areaTransmitter = collapsableArea;

    addDoubleParameter(
        "tx_power",
        currentChannelModel.getParameterDescription("tx_power"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("tx_power")
    );
    
    addDoubleParameter(
        "tx_antenna_gain",
        currentChannelModel.getParameterDescription("tx_antenna_gain"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("tx_antenna_gain")
    );

    // Receiver parameters
    collapsableArea = createCollapsableArea("Receiver parameters", allComponents);
    areaReceiver = collapsableArea;

    addDoubleParameter(
        "rx_sensitivity",
        currentChannelModel.getParameterDescription("rx_sensitivity"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("rx_sensitivity")
    );

    addDoubleParameter(
        "rx_antenna_gain",
        currentChannelModel.getParameterDescription("rx_antenna_gain"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("rx_antenna_gain")
    );
    
    // Ray Tracer parameters
    collapsableArea = createCollapsableArea("Ray Tracer parameters", allComponents);
    areaRayTracer = collapsableArea;

    addBooleanParameter(
        "rt_disallow_direct_path",
        currentChannelModel.getParameterDescription("rt_disallow_direct_path"),
        collapsableArea,
        currentChannelModel.getParameterBooleanValue("rt_disallow_direct_path")
    );

    addBooleanParameter(
        "rt_ignore_non_direct",
        currentChannelModel.getParameterDescription("rt_ignore_non_direct"),
        collapsableArea,
        currentChannelModel.getParameterBooleanValue("rt_ignore_non_direct")
    );

    addBooleanParameter(
        "rt_fspl_on_total_length",
        currentChannelModel.getParameterDescription("rt_fspl_on_total_length"),
        collapsableArea,
        currentChannelModel.getParameterBooleanValue("rt_fspl_on_total_length")
    );

    addIntegerParameter(
        "rt_max_rays",
        currentChannelModel.getParameterDescription("rt_max_rays"),
        collapsableArea,
        currentChannelModel.getParameterIntegerValue("rt_max_rays")
    );

    addIntegerParameter(
        "rt_max_refractions",
        currentChannelModel.getParameterDescription("rt_max_refractions"),
        collapsableArea,
        currentChannelModel.getParameterIntegerValue("rt_max_refractions")
    );

    addIntegerParameter(
        "rt_max_reflections",
        currentChannelModel.getParameterDescription("rt_max_reflections"),
        collapsableArea,
        currentChannelModel.getParameterIntegerValue("rt_max_reflections")
    );

    addIntegerParameter(
        "rt_max_diffractions",
        currentChannelModel.getParameterDescription("rt_max_diffractions"),
        collapsableArea,
        currentChannelModel.getParameterIntegerValue("rt_max_diffractions")
    );
    
/*    addBooleanParameter(
        "rt_use_scattering",
        currentChannelModel.getParameterDescription("rt_use_scattering"),
        collapsableArea,
        currentChannelModel.getParameterBooleanValue("rt_use_scattering")
    );
*/
    addDoubleParameter(
        "rt_refrac_coefficient",
        currentChannelModel.getParameterDescription("rt_refrac_coefficient"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("rt_refrac_coefficient")
    );

    addDoubleParameter(
        "rt_reflec_coefficient",
        currentChannelModel.getParameterDescription("rt_reflec_coefficient"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("rt_reflec_coefficient")
    );

    addDoubleParameter(
        "rt_diffr_coefficient",
        currentChannelModel.getParameterDescription("rt_diffr_coefficient"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("rt_diffr_coefficient")
    );
    
/*    addDoubleParameter(
        "rt_scatt_coefficient",
        currentChannelModel.getParameterDescription("rt_scatt_coefficient"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("rt_scatt_coefficient")
    );
*/    
    // Shadowing parameters
    collapsableArea = createCollapsableArea("Shadowing parameters", allComponents);
    areaShadowing = collapsableArea;

    addDoubleParameter(
        "obstacle_attenuation",
        currentChannelModel.getParameterDescription("obstacle_attenuation"),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue("obstacle_attenuation")
    );

    
    
    // Add channel model observer responsible to keep all GUI components synched
    currentChannelModel.addSettingsObserver(channelModelSettingsObserver);
    
    // Set initial size etc.
    pack();
    setVisible(true);
    
    // Tries to select this plugin
    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
    
  }

  /**
   * Creates a new collapsable area which may be used for holding model parameters.
   * @param title Title of area
   * @param contentPane Where this area should be added
   * @return New empty collapsable area
   */
  private JPanel createCollapsableArea(String title, Container contentPane) {
    // Create panels
    JPanel holdingPanel = new JPanel() {
      public Dimension getMaximumSize() {
        return new Dimension(super.getMaximumSize().width, getPreferredSize().height);
      }
    };
    holdingPanel.setLayout(new BoxLayout(holdingPanel, BoxLayout.Y_AXIS));

    final JPanel collapsableArea = new JPanel() {
      public Dimension getMaximumSize() {
        return new Dimension(super.getMaximumSize().width, getPreferredSize().height);
      }
    };
    collapsableArea.setLayout(new BoxLayout(collapsableArea, BoxLayout.Y_AXIS));
    collapsableArea.setVisible(false);
    
    JPanel titlePanel = new JPanel(new BorderLayout()) {
      public Dimension getMaximumSize() {
        return new Dimension(super.getMaximumSize().width, getPreferredSize().height);
      }
    };
    
    titlePanel.add(BorderLayout.WEST, new JLabel(title));
    JCheckBox collapseCheckBox = new JCheckBox("show settings", false);
    collapseCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (((JCheckBox) e.getSource()).isSelected())
          collapsableArea.setVisible(true);
        else
          collapsableArea.setVisible(false);
      }
    });
    collapsableArea.putClientProperty("my_checkbox", collapseCheckBox);

    titlePanel.add(BorderLayout.EAST, collapseCheckBox);
    
    collapsableArea.setBorder(
        BorderFactory.createLineBorder(Color.LIGHT_GRAY)
    );   
    collapsableArea.setAlignmentY(Component.TOP_ALIGNMENT);
    
    holdingPanel.add(titlePanel);
    holdingPanel.add(collapsableArea);

    contentPane.add(holdingPanel);
    return collapsableArea;
  }
  
  /**
   * Creates and adds a panel with a label and a 
   * text field which accepts doubles.
   * 
   * @param id Identifier of new parameter
   * @param description Description of new parameter
   * @param contentPane Where to add created panel
   * @param initialValue Initial value
   * @return Text field in created panel
   */
  private JFormattedTextField addDoubleParameter(String id, String description, Container contentPane, double initialValue) {
    JPanel panel = new JPanel();
    JLabel label;
    JFormattedTextField textField;
    
    panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
    panel.setAlignmentY(Component.TOP_ALIGNMENT);
    panel.add(Box.createHorizontalStrut(10));
    panel.add(label = new JLabel(description));
    label.setPreferredSize(labelDimension);
    panel.add(Box.createHorizontalGlue());
    panel.add(textField = new JFormattedTextField(doubleFormat));
    textField.setValue(new Double(initialValue));
    textField.setColumns(4);
    textField.putClientProperty("id", id);
    textField.addPropertyChangeListener("value", new PropertyChangeListener() {
      public void propertyChange(PropertyChangeEvent e) {
        Object sourceObject = e.getSource();
        Double newValue = ((Number) e.getNewValue()).doubleValue();
        String id = (String) ((JFormattedTextField) sourceObject).getClientProperty("id");
        currentChannelModel.setParameterValue(id, newValue);
      }
    });
    allDoubleParameters.add(textField);
    
    contentPane.add(panel);

    return textField;
  }
  
  /**
   * Creates and adds a panel with a label and a 
   * text field which accepts integers.
   * 
   * @param id Identifier of new parameter
   * @param description Description of new parameter
   * @param contentPane Where to add created panel
   * @param initialValue Initial value
   * @return Text field in created panel
   */
  private JFormattedTextField addIntegerParameter(String id, String description, Container contentPane, int initialValue) {
    JPanel panel = new JPanel();
    JLabel label;
    JFormattedTextField textField;
    
    panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
    panel.setAlignmentY(Component.TOP_ALIGNMENT);
    panel.add(Box.createHorizontalStrut(10));
    panel.add(label = new JLabel(description));
    label.setPreferredSize(labelDimension);
    panel.add(Box.createHorizontalGlue());
    panel.add(textField = new JFormattedTextField(integerFormat));
    textField.setValue(new Double(initialValue));
    textField.setColumns(4);
    textField.putClientProperty("id", id);
    textField.addPropertyChangeListener("value", new PropertyChangeListener() {
      public void propertyChange(PropertyChangeEvent e) {
        Object sourceObject = e.getSource();
        Integer newValue = ((Number) e.getNewValue()).intValue();
        String id = (String) ((JFormattedTextField) sourceObject).getClientProperty("id");
        currentChannelModel.setParameterValue(id, newValue);
      }
    });

    allIntegerParameters.add(textField);

    contentPane.add(panel);

    return textField;
  }
  
  /**
   * Creates and adds a panel with a label and a 
   * boolean checkbox.
   * 
   * @param id Identifier of new parameter
   * @param description Description of new parameter
   * @param contentPane Where to add created panel
   * @param initialValue Initial value
   * @return Checkbox in created panel
   */
  private JCheckBox addBooleanParameter(String id, String description, Container contentPane, boolean initialValue) {
    JPanel panel = new JPanel();
    JLabel label;
    JCheckBox checkBox;
    
    panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
    panel.setAlignmentY(Component.TOP_ALIGNMENT);
    panel.add(Box.createHorizontalStrut(10));
    panel.add(label = new JLabel(description));
    label.setPreferredSize(labelDimension);
    panel.add(Box.createHorizontalGlue());
    panel.add(checkBox = new JCheckBox());
    checkBox.setSelected(initialValue);
    checkBox.putClientProperty("id", id);
    checkBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        JCheckBox source = (JCheckBox) e.getSource();
        currentChannelModel.setParameterValue(
            (String) source.getClientProperty("id"),
            new Boolean(source.isSelected())
        );
      }
    });

    allBooleanParameters.add(checkBox);

    contentPane.add(panel);

    return checkBox;
  }
  
  /**
   * Creates and adds a panel with a description label.
   * 
   * @param description Description of new parameter
   * @param contentPane Where to add created panel
   * @return Created label
   */
  private JLabel addLabelParameter(String description, Container contentPane) {
    JPanel panel = new JPanel();
    JLabel label;
    
    panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
    panel.setAlignmentY(Component.TOP_ALIGNMENT);
    panel.add(Box.createHorizontalStrut(10));
    panel.add(label = new JLabel(description));
    label.setPreferredSize(labelDimension);
    panel.add(Box.createHorizontalGlue());

    contentPane.add(panel);

    return label;
  }
  
  /**
   * Listens to settings changes in the channel model.
   * If it changes, all GUI parameters are updated accordingly.
   */
  private Observer channelModelSettingsObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      // Update all integers
      for (int i=0; i < allIntegerParameters.size(); i++) {
        JFormattedTextField textField = (JFormattedTextField) allIntegerParameters.get(i);
        String id = (String) textField.getClientProperty("id");
        textField.setValue(currentChannelModel.getParameterValue(id));
      }

      // Update all doubles
      for (int i=0; i < allDoubleParameters.size(); i++) {
        JFormattedTextField textField = (JFormattedTextField) allDoubleParameters.get(i);
        String id = (String) textField.getClientProperty("id");
        textField.setValue(currentChannelModel.getParameterValue(id));
      }

      // Update all booleans
      for (int i=0; i < allBooleanParameters.size(); i++) {
        JCheckBox checkBox = (JCheckBox) allBooleanParameters.get(i);
        String id = (String) checkBox.getClientProperty("id");
        checkBox.setSelected(currentChannelModel.getParameterBooleanValue(id));
      }

      repaint();
    }
  };
  
  public void closePlugin() {
    // Remove the channel model observer
    if (currentChannelModel != null && channelModelSettingsObserver != null) {
      currentChannelModel.deleteSettingsObserver(channelModelSettingsObserver);
    } else {
      logger.fatal("Can't remove channel model observer: " + channelModelSettingsObserver);
    }
  }
  
  /**
   * Returns XML elements representing the current configuration.
   * 
   * @see #setConfigXML(Collection)
   * @return XML element collection 
   */
  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    element = new Element("show_general");
    element.setText(Boolean.toString(areaGeneral.isVisible()));
    config.add(element);
    element = new Element("show_transmitter");
    element.setText(Boolean.toString(areaTransmitter.isVisible()));
    config.add(element);
    element = new Element("show_receiver");
    element.setText(Boolean.toString(areaReceiver.isVisible()));
    config.add(element);
    element = new Element("show_raytracer");
    element.setText(Boolean.toString(areaRayTracer.isVisible()));
    config.add(element);
    element = new Element("show_shadowing");
    element.setText(Boolean.toString(areaShadowing.isVisible()));
    config.add(element);
    return config;
  }

  /**
   * Sets the configuration depending on the given XML elements.
   * 
   * @see #getConfigXML()
   * @param configXML
   *          Config XML elements
   * @return True if config was set successfully, false otherwise
   */
  public boolean setConfigXML(Collection<Element> configXML) {
    for (Element element : configXML) {
      if (element.getName().equals("show_general")) {
        JCheckBox checkBox = (JCheckBox) areaGeneral.getClientProperty("my_checkbox");
        checkBox.setSelected(Boolean.parseBoolean(element.getText()));
        checkBox.getActionListeners()[0].actionPerformed(new ActionEvent(checkBox,
            ActionEvent.ACTION_PERFORMED, ""));
      } else if (element.getName().equals("show_transmitter")) {
        JCheckBox checkBox = (JCheckBox) areaTransmitter.getClientProperty("my_checkbox");
        checkBox.setSelected(Boolean.parseBoolean(element.getText()));
        checkBox.getActionListeners()[0].actionPerformed(new ActionEvent(checkBox,
            ActionEvent.ACTION_PERFORMED, ""));
      } else if (element.getName().equals("show_receiver")) {
        JCheckBox checkBox = (JCheckBox) areaReceiver.getClientProperty("my_checkbox");
        checkBox.setSelected(Boolean.parseBoolean(element.getText()));
        checkBox.getActionListeners()[0].actionPerformed(new ActionEvent(checkBox,
            ActionEvent.ACTION_PERFORMED, ""));
      } else if (element.getName().equals("show_raytracer")) {
        JCheckBox checkBox = (JCheckBox) areaRayTracer.getClientProperty("my_checkbox");
        checkBox.setSelected(Boolean.parseBoolean(element.getText()));
        checkBox.getActionListeners()[0].actionPerformed(new ActionEvent(checkBox,
            ActionEvent.ACTION_PERFORMED, ""));
      } else if (element.getName().equals("show_shadowing")) {
        JCheckBox checkBox = (JCheckBox) areaShadowing.getClientProperty("my_checkbox");
        checkBox.setSelected(Boolean.parseBoolean(element.getText()));
        checkBox.getActionListeners()[0].actionPerformed(new ActionEvent(checkBox,
            ActionEvent.ACTION_PERFORMED, ""));
      }
    }
    return true;    
  }

}
