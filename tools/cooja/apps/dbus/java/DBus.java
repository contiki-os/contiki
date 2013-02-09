import se.sics.cooja.GUI;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;


public class DBus extends VisPlugin {

	private Simulation simulation;

	public DBus(Simulation simulation, final GUI gui) {
	    super("DBus", gui, false);
	    this.simulation = simulation;


	}

}
