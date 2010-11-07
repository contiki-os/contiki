#!/usr/bin/perl
use strict;

my $bin = shift;
my $terms = shift;

my $addr = "0x1e000";
my $company_id;
my $iab = 0xabc; # Redwire, LLC's IAB

if(defined($iab)) {
    $company_id = (0x0050C2 << 12) | $iab;
}

if (! $terms) {
	print "Usage: $0 flasher.bin num-devices\n";
	die;
}

for (my $t=0; $t<$terms; $t++) {
	my $dev_num = 2 * $t + 1;
	my $mac;
	if(defined($iab)) {
	    $mac = ($company_id << 28) | $dev_num;
	} else {
	    $mac = ($company_id << 40) | $dev_num;
	}
	my $ftdi_num = $terms - $t - 1;
	my $cmd = "mc1322x-load.pl -e -f $bin -z -t /dev/ttyUSB$dev_num  -c 'bbmc -l redbee-econotag -i $ftdi_num reset' $addr," . sprintf("0x%08X,0x%08X\n", ($mac >> 32), ($mac & 0xffffffff));
	print "$cmd\n";
	system($cmd);
}
