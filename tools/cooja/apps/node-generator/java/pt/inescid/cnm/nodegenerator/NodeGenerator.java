package pt.inescid.cnm.nodegenerator;

import java.io.File;
import java.lang.Math;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.Random;

import javax.swing.JFileChooser;
import javax.swing.JScrollPane;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.Mote;
import org.contikios.cooja.MoteType;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.TimeEvent;
import org.contikios.cooja.VisPlugin;
import org.contikios.cooja.dialogs.MessageList;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.util.StringUtils;

import pt.inescid.cnm.nodegenerator.MobileMote;

@ClassDescription("NodeGenerator")
@PluginType(PluginType.SIM_PLUGIN)
public class NodeGenerator extends VisPlugin {
  private static final long serialVersionUID = -1087396096570660083L;
  private static Logger logger = Logger.getLogger(NodeGenerator.class);
  
  private Simulation simulation;
  
  private double radius_increment;
  private double radius_max;
  private double radius_min;
  private double angle_min;
  private double angle_multiplier;
  private ArrayList<Double> radius;
  private ArrayList<Double> angles;
  private double random_offset;
  
  private String sink_mote_type;
  private String mote_type_1;
  private String mote_type_2;
  private double mote_type_ratio;
  private long generation_seed;
  
  private Random generator;
  
  private ArrayList<MobileMote> mobile_motes;
  
  private int id_counter;

  private MessageList log = new MessageList();

  public NodeGenerator(Simulation simulation, final Cooja gui) {
    super("NodeGenerator", gui, false);
    this.simulation = simulation;
    radius_increment = 20.0;
    radius_min = 30.0;
    radius_max = 90.0;
    angle_min = 90.0;
    angle_multiplier = 0.5;
    sink_mote_type = "sky1";
    mote_type_1 = "sky2";
    mote_type_2 = "sky2";
    mote_type_ratio = 0.1;
    random_offset = 20;
    generation_seed = 0;
  }
  
  private void generateNodes(){
    
    if(generation_seed == 0){
      generator = new Random();
    }else{
      generator = new Random(generation_seed);
    }
    
    //remove existing Motes before creating new ones
    Mote[] motes = simulation.getMotes();
    for (Mote m: motes) {
      simulation.removeMote(m);
    }
    
    id_counter = 0;
    
    mobile_motes = new ArrayList<MobileMote>();
    
    radius = new ArrayList<Double>();
    for(double r = radius_min; r <= radius_max; r += radius_increment){
      radius.add(r);
    }
    
    angles = new ArrayList<Double>();
    angles.add(angle_min);
    for(int i = 1; i < radius.size(); i++){
      angles.add(angles.get(i-1)*angle_multiplier);
    }
    
    MoteType sink_mt = simulation.getMoteType(sink_mote_type);
    MoteType mote_mt = simulation.getMoteType(mote_type_1);
    MoteType mob_mote_mt = null;
    if(mote_type_2 != null) mob_mote_mt = simulation.getMoteType(mote_type_2);
    
    createNodeAtPosition(sink_mt, 0, 0, false);
    for(int i = 0; i < radius.size(); i++){
      double r = radius.get(i);
      double angle_increment = angles.get(i);
      double angle_offset = generator.nextDouble()*angle_increment - angle_increment/2;
      for(double phi = angle_offset; phi <= 360 - angle_increment + angle_offset; phi += angle_increment){
        double posX = r * Math.cos(Math.toRadians(phi));
        double posY = r * Math.sin(Math.toRadians(phi));
        MoteType mt = null;
        boolean mobility;
        if(mob_mote_mt == null || generator.nextDouble() > mote_type_ratio){ 
          mt = mote_mt;
          mobility = false;
        }else{ 
          mt = mob_mote_mt;
          mobility = true;
        }
        createNodeAtPosition(mt,posX,posY,mobility);
      }
    }
  }
  
  private void createNodeAtPosition(MoteType mt, double posX, double posY, boolean is_mobile){
    double x = posX + generator.nextDouble()*random_offset - random_offset/2;
    double y = posY + generator.nextDouble()*random_offset - random_offset/2;
    Mote m = mt.generateMote(this.simulation);
    Position pos = m.getInterfaces().getPosition();
    pos.setCoordinates( x, y, pos.getZCoordinate());
    m.getInterfaces().getMoteID().setMoteID(++id_counter);
    this.simulation.addMote(m);
    if(is_mobile){
      MobileMote mm = new MobileMote(m,this);
      mobile_motes.add(mm);
    }
    return;
  }
  
  public Random getGenerator(){
    return generator;
  }
  
  public double getBoundary(){
    return radius_max;
  }
  
  public Simulation getSimulation(){
    return this.simulation;
  }
  
  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();

    return config;
  }
  
  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("radius_increment")) {
        radius_increment = Double.parseDouble(element.getText());
      }else if(name.equals("radius_max")){
        radius_max = Double.parseDouble(element.getText());
      }else if(name.equals("radius_min")){
        radius_min = Double.parseDouble(element.getText());
      }else if(name.equals("angle_min")){
        angle_min = Double.parseDouble(element.getText());
      }else if(name.equals("angle_multiplier")){
        angle_multiplier = Double.parseDouble(element.getText());
      }else if(name.equals("sink_mote_type")){
        sink_mote_type = element.getText();
      }else if(name.equals("mote_type")){
        mote_type_1 = element.getText();
        mote_type_ratio = 0;
      }else if(name.equals("mote_type_1")){
        mote_type_1 = element.getText();
      }else if(name.equals("mote_type_2")){
        mote_type_2 = element.getText();
      }else if(name.equals("mote_type_ratio")){
        mote_type_ratio = Double.parseDouble(element.getText());
      }else if(name.equals("random_offset")){
        random_offset = Double.parseDouble(element.getText());
      }else if(name.equals("generation_seed")){
        generation_seed = Long.valueOf(element.getText()).longValue();
      }
    }
    
    generateNodes();

    return true;
  }
}
