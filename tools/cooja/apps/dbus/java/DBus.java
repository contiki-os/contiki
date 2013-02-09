import se.sics.cooja.GUI;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.ClassDescription;

import org.cooja.dbus.simulation.CDbus;
import org.freedesktop.dbus.*;
import org.freedesktop.dbus.exceptions.DBusException;

/**
 * Creates dbus functions to control externally
 *
 * @author Salvo 'LtWorf' Tomaselli
 */

@ClassDescription("DBus")
@PluginType(PluginType.SIM_PLUGIN)
public class DBus extends VisPlugin {
	public DBus(Simulation simulation, final GUI gui) {
	    super("DBus", gui, false);
	    CDbus connection = new CDbus(simulation);

	    	}
}