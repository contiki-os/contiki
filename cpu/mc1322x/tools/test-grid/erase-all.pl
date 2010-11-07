#!/usr/bin/perl
use strict;

my $terms = shift;

if (! $terms) {
	print "Usage: num-devices\n";
	die;
}

for (my $t=0; $t<$terms; $t++) {
	my $dev_num = 2 * $t + 1;
	my $ftdi_num = $terms - $t - 1;
	my $cmd = "bbmc -l redbee-econotag -i $ftdi_num erase &";
	print "$cmd\n";
	system($cmd);
}
