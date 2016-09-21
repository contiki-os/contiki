/**
 * 
 */
package se.sics.mspsim.cli;

import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.net.CC2420PacketHandler;
import se.sics.jipv6.mac.IEEE802154Handler;
import se.sics.jipv6.mac.LoWPANHandler;
import se.sics.jipv6.core.IPStack;
import se.sics.jipv6.tunnel.TSPClient;
import se.sics.mspsim.util.ComponentRegistry;
import se.sics.mspsim.util.Utils;

/**
 * @author joakim
 *
 */
public class NetCommands implements CommandBundle {

  private IPStack ipStack;
  public void setupCommands(final ComponentRegistry registry, CommandHandler handler) {
    handler.registerCommand("ipstack", new BasicLineCommand("setup 802.15.4/IP stack", "") {
      CC2420PacketHandler listener;
      public int executeCommand(CommandContext context) {
        MSP430 cpu = registry.getComponent(MSP430.class);
        listener = new CC2420PacketHandler(cpu);
        listener.setOutput(context.out);
        IEEE802154Handler ieeeHandler = new IEEE802154Handler();
        listener.addUpperLayerHandler(0, ieeeHandler);
        ieeeHandler.setLowerLayerHandler(listener);
        ipStack = new IPStack();
        byte[] macAddr = new byte[] {0x2,0x12,0x74,0x00,0x11,0x11,0x12,0x12};
        ipStack.setLinkLayerAddress(macAddr);
        ipStack.setRouter(true);
        LoWPANHandler lowpanHandler = new LoWPANHandler();
        lowpanHandler.setIPStack(ipStack);
        ieeeHandler.addUpperLayerHandler(0, lowpanHandler);
        lowpanHandler.setLowerLayerHandler(ieeeHandler);
        ipStack.setLinkLayerHandler(lowpanHandler);
        context.err.print("IP Stack started");
        return 0;
      }
      
      public void lineRead(String line) {
        if (listener != null) {
          byte[] data = Utils.hexconv(line);
          for (int i = 0; i < data.length; i++) {
            // Currently it will auto print when packet is ready...
            listener.receivedByte(data[i]);
          }
        }
      }
    });

    handler.registerCommand("tspstart", new BasicCommand("starts a TSP tunnel", "<server> <user> <password>") {
      public int executeCommand(CommandContext context) {
        TSPClient tunnel = TSPClient.startTSPTunnel(ipStack, context.getArgument(0),
            context.getArgument(1), context.getArgument(2));
        ipStack.setTunnel(tunnel);
        if (tunnel != null) {
          context.out.print("TSP Tunnel started");
          return 0;
        } else {
          context.out.print("TSP Tunnel failed");
          return 1;
        }
      }
    });
  }
}
