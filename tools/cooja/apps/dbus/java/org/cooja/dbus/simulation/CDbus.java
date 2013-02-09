package org.cooja.dbus.simulation;

import org.freedesktop.dbus.DBusConnection;
import org.freedesktop.dbus.DBusSigHandler;
import org.freedesktop.dbus.DBusSignal;
import org.freedesktop.dbus.exceptions.DBusException;

import se.sics.cooja.Simulation;

public class CDbus {

	public static void main(String[] args) {
		new CDbus(null);
	}

	public CDbus(Simulation simulation) {
		Debug.debug("Creating DBus Connection");
	    DBusConnection conn = null;
	    try {
	         conn = DBusConnection.getConnection(DBusConnection.SESSION);
	    } catch (DBusException DBe) {
	         Debug.debug("Could not connect to bus",true);
	         return;
	    }

	    try {
	       conn.addSigHandler(DBusCommands.Start.class, new DBusStart(simulation));
	    } catch (DBusException DBe) {
	       conn.disconnect();
	       Debug.debug("DBus exception",true);
	       return;
	    }

	}

}



class DBusStart implements DBusSigHandler {

	private Simulation simulation;

	public DBusStart(Simulation s) {
		this.simulation=s;
	}

	@Override
	public void handle(DBusSignal s) {
		Debug.debug("suca");
		if (s instanceof DBusCommands.Start) {
			//this.simulation.startSimulation();
		}
	}

}