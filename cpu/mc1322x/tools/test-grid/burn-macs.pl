#!/usr/bin/perl
use strict;

my $bin = shift;
my $terms = shift;

my $addr = "0x1e000";
my $iab = 0xa8c; # Redwire, LLC's IAB

my $mac_h;
my $mac_l;

if(defined($iab)) {
    $mac_h = 0x0050C200 | ($iab >> 4);
    $mac_l = ($iab & 0xf) << 28;
}

if (! $terms) {
	print "Usage: $0 flasher.bin num-devices\n";
	die;
}

for (my $t=0; $t<$terms; $t++) {
	my $dev_num = 2 * $t + 1;
	$mac_l |= $dev_num;
	#stupid 32-bit thing...
	my $mac;
	printf("mac_h %x\n", $mac_h);
	printf("mac_l %x\n", $mac_l);
	my @words;
	for(my $i=0; $i<4; $i++) {
	    push @words, ($mac_h >> ($i * 8)) & 0xff;
	}
	for(my $i=0; $i<4; $i++) {
	    push @words, ($mac_l >> ($i * 8)) & 0xff;
	}
	reverse @words;
	foreach my $byte (@words) {
	    printf("%02X",$byte);
	}
	print "\n";

	my $word1 = sprintf("%02X%02X%02X%02X",$words[4],$words[5],$words[6],$words[7]);
	my $word2 = sprintf("%02X%02X%02X%02X",$words[0],$words[1],$words[2],$words[3]);

	my $ftdi_num = $terms - $t - 1;
	my $cmd = "mc1322x-load.pl -e -f $bin -z -t /dev/ttyUSB$dev_num  -c 'bbmc -l redbee-econotag -i $ftdi_num reset' $addr,0x$word1,0x$word2 &";
	print "$cmd\n";
	system($cmd);
}
