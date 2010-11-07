#!/usr/bin/perl
use strict;

my $bin = shift;
my $terms = shift;

if (! $terms) {
	print "Usage: $0 file.bin num-devices\n";
	die;
}

for (my $t=0; $t<$terms; $t++) {
	my $dev_num = 2 * $t + 1;
	my $ftdi_num = $terms - $t - 1;
	my $cmd = "mc1322x-load.pl -e -f $bin -t /dev/ttyUSB$dev_num  -c 'bbmc -l redbee-econotag -i $ftdi_num reset' &";
	print "$cmd\n";
	system($cmd);
}
