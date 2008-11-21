lowAlpha = "BA\n";
highAlpha = "JIHGFEDCBA\n";
lowCount = "10\n";
highCount = "9876543210\n";

/* Filter messages */
if (msg.equals(lowCount)) {
  log.log("Count is low\n");
} else if (msg.equals(lowAlpha)) {
  log.log("Alpha is low\n");
} else if (msg.equals(highCount)) {
  log.log("Count is high\n");
} else if (msg.equals(highAlpha)) {
  log.log("Alpha is high\n");
} else {
  /* Ignore all other messages */
  return;
}

/* Remember messages */
count = global.get(msg);
if (count == null) {
  count = 0;
}
count++;
global.put(msg, count);

/* Wait during test */
count = global.get(lowAlpha);
if (count == null || count < 5) return;
count = global.get(highAlpha);
if (count == null || count < 5) return;
count = global.get(lowCount);
if (count == null || count < 5) return;
count = global.get(highCount);
if (count == null || count < 5) return;

log.testOK(); /* We are done! */
