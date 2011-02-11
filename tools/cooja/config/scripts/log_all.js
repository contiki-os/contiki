/*
 * Example Contiki test script (JavaScript).
 * A Contiki test script acts on mote output, such as via printf()'s.
 * The script may operate on the following variables:
 *  Mote mote, int id, String msg
 */

TIMEOUT(60000);

while (true) {
  log.log(time + ":" + id + ":" + msg + "\n");
  YIELD();
}
