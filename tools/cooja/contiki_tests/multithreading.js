TIMEOUT(60000, log.log("nrLowAlpha: " + nrLowAlpha + "\nnrHighAlpha: " + nrHighAlpha + "\nnrLowCount: " + nrLowCount + "\nnrHighCount: " + nrHighCount + "\n"));

lowAlpha = "BA";
highAlpha = "JIHGFEDCBA";
lowCount = "10";
highCount = "9876543210";

nrLowAlpha = 0;
nrHighAlpha = 0;
nrLowCount = 0;
nrHighCount = 0;

while (true) {

  if (msg.equals(lowCount)) {
    //log.log("Count is low\n");
    nrLowCount++;
  } else if (msg.equals(lowAlpha)) {
    //log.log("Alpha is low\n");
    nrLowAlpha++;
  } else if (msg.equals(highCount)) {
    //log.log("Count is high\n");
    nrHighCount++;
  } else if (msg.equals(highAlpha)) {
    //log.log("Alpha is high\n");
    nrHighAlpha++;
  }

  if (nrLowCount >= 5 &&
      nrLowAlpha >= 5 &&
      nrLowCount >= 5 &&
      nrHighCount >= 5 &&
      nrHighAlpha >= 5) {
     log.testOK();
  }
  YIELD();
}

