package org.cooja.dbus.simulation;

import org.freedesktop.dbus.DBusConnection;
import org.freedesktop.dbus.exceptions.DBusException;

import se.sics.cooja.Simulation;

/**
 * The class creates the session dbus connection
 * and then listens to the calls to manipulate the
 * simulation.
 *
 *
 * @author Salvo 'LtWorf' Tomaselli
 *
 */
public class CDbus {
    DBusConnection conn = null;

	public CDbus(Simulation simulation) {
		Debug.debug("Creating DBus Connection");


	    try {
	         conn = DBusConnection.getConnection(DBusConnection.SESSION);


	    } catch (DBusException DBe) {
	         Debug.debug("Could not connect to bus",true);
	         return;
	    }

	    try {
	        conn.requestBusName("org.cooja.dbus.simulation");
	        conn.exportObject("/org/cooja/dbus/simulation/SimControl", new SimControlActuator(simulation));

	    } catch (DBusException DBe) {
	        DBe.printStackTrace();
	        conn.disconnect();
	        Debug.debug("DBus exception",true);
	        return;
	    }

	}

	public void disconnect() {
	    conn.disconnect();
	}

}



class SimControlActuator implements SimControl {

	private Simulation simulation;

	public SimControlActuator(Simulation s) {
		this.simulation=s;
	}

    @Override
    public boolean isRemote() {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public synchronized void Step() {
        simulation.stepMillisecondSimulation();
    }

    @Override
    public synchronized void Start() {
        simulation.startSimulation();
    }

    @Override
    public synchronized void Stop() {
        simulation.stopSimulation();
    }

    @Override
    public synchronized void setSimulationTime(long time) {
        simulation.setSimulationTime(time);
    }

    @Override
    public synchronized long getSimulationTime() {
        return simulation.getSimulationTime();
    }

    @Override
    public synchronized void setSpeedLimit(double limit) {
        simulation.setSpeedLimit(limit);
    }

    @Override
    public synchronized double getSpeedLimit() {
        return simulation.getSpeedLimit();
    }

    @Override
    public synchronized void setTitle(String title) {
        simulation.setTitle(title);
    }

    @Override
    public synchronized String getTitle() {
        return simulation.getTitle();
    }

    @Override
    public synchronized void Reload() {
        simulation.getGUI().reloadCurrentSimulation(simulation.isRunning());
    }

}