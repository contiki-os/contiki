import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;

import se.sics.cooja.GUI;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.ClassDescription;

import org.cooja.dbus.simulation.CDbus;

/**
 * Creates dbus functions to control externally
 *
 * @author Salvo 'LtWorf' Tomaselli
 */

@ClassDescription("DBus")
@PluginType(PluginType.SIM_PLUGIN)
public class DBus extends VisPlugin implements ActionListener {
    private CDbus connection;
    private Simulation simulation;

    private JButton btnStart = new JButton("Start");
    private JButton btnStop = new JButton("Stop");

	public DBus(Simulation simulation, final GUI gui) {
	    super("DBus", gui, false);

	    this.simulation = simulation;

        this.connection = new CDbus(simulation);

        btnStart.setEnabled(false);


        getContentPane().setLayout(new FlowLayout());
        getContentPane().add(btnStart);
        getContentPane().add(btnStop);

        btnStart.addActionListener(this);
        btnStop.addActionListener(this);
	}

    @Override
    public void actionPerformed(ActionEvent arg0) {
        if (arg0.getSource()==btnStart) {
            connection = new CDbus(simulation);
            btnStart.setEnabled(false);
            btnStop.setEnabled(true);


        } else if (arg0.getSource() == btnStop) {
            connection.disconnect();
            connection = null;
            btnStart.setEnabled(true);
            btnStop.setEnabled(false);
        }


    }
}