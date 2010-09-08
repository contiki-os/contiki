#!/usr/bin/perl

use strict;
use integer;

my $start = shift;
my $end = shift;

if (! $end) {
	print "Usage: $0 first-term-num last-term-num\n";
	die;
}

my $COLS = 4;

my $index = 0;

for (my $n=$start; $n <= $end; $n += 2) {
  #print "$n\n";

  open(OUT, ">/tmp/USB$n.ini");
  my $dev = "/dev/ttyUSB$n";
print OUT <<end
set line $dev
if failure { exit 1 "Unable top open $dev" }
set carrier-watch off
set flow-control none
set speed 115200
set serial 8n1
set key \\127 \\8
connect
quit
end
;
  close(OUT);
  my ($row, $col);
  $row = $index / $COLS;
  $col = $index % $COLS;
  my ($offx, $offy);
  $offx = $col * 300;
  $offy = $row * 300;
  my $cmd = "xterm -e 'kermit /tmp/USB$n.ini'&";
  #my $cmd = "xterm -geometry 40x20+$offx+$offy -e 'stty -F /dev/ttyUSB$n 115200 && cat /dev/ttyUSB$n'&";
#  my $cmd = "xterm -e 'stty -F /dev/ttyUSB$n 115200 && cat /dev/ttyUSB$n'&";
  print "$cmd\n";
  system($cmd);

  $index++;
}
