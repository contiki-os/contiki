package se.sics.cooja.plugins;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;

import org.freedesktop.dbus

/**
 * Creates dbus functions to control externally
 *
 * @author Salvo 'LtWorf' Tomaselli
 */
@ClassDescription("DBus control")
@PluginType(PluginType.SIM_STANDARD_PLUGIN)
public class DBus extends VisPlugin {

	private Simulation simulation;

	public DBus(Simulation simulation, GUI gui) {
		    super("Simulation control", gui);
		    this.simulation = simulation;
	}

}
