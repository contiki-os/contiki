/*
 * Example Contiki test script (JavaScript).
 * A Contiki test script acts on mote output, such as via printf()'s.
 * The script may operate on the following variables:
 *  Mote mote, int id, String msg
 *
 * This script logs everything.
 */

TIMEOUT(3000000); /* msec; 1500000 = 25 minutes; 3000000 usato per test paper WCNC; 4200000 per test con periodo 180 */

while (true) {
  log.log(time + ":" + id + ":" + msg + "\n");
  YIELD();
}
