/*
 * Example Contiki test script (JavaScript).
 * A Contiki test script acts on mote output, such as via printf()'s.
 * The script may operate on the following variables:
 *  Mote mote, int id, String msg
 */

/* Wait until node has booted */
WAIT_UNTIL(msg.startsWith('Starting'));
log.log("Mote started\n");
mymote = mote; /* store mote reference */

/* Wait 3 seconds (3000ms) */
GENERATE_MSG(3000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

/* Write command to serial port */
log.log("Writing 'ls' to mote serial port\n");
write(mymote, "ls");

/* Read replies */
while (true) {
  YIELD();
  if (mote == mymote) {
    log.log("Mote replied: " + msg + "\n");
  }
}