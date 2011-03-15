#!/usr/bin/perl
use strict;
use Getopt::Long;

my $oui;
my $addr = "0x1e000";
my $iab;
my $term = "/dev/ttyUSB1";

GetOptions ('iab=s' => \$iab,
	    'oui=s' => \$oui,
	    'term=s' => \$term,
    ) or die 'bad options';

my $bin = shift;
my $address = shift;

if (!defined($address) || !defined($bin)) {
	print "Usage: $0 [--iab=a8c | --oui=abcdef | --term device] flasher.bin address\n";
	print "          iab is 12-bit and address is 28-bit e.g. --iab=a8c 1234567\n";
	print "          oui is 24-bit and address is 40-bit e.g. --oui=abcdef 123456789a\n";
	exit;
}

my $mac_h;
my $mac_l;

if(defined($iab)) {
    $iab = hex($iab);
    $address = hex($address);
    $mac_h = 0x0050C200 | ($iab >> 4);
    $mac_l = (($iab & 0xf) << 28) | $address;
} else {
    $address =~ /(.*?)(.{0,8})$/;
    my ($addr_h, $addr_l) = ($1, $2);
    if(!$addr_h) { $addr_h = 0 };
    $oui = hex($oui);
    $addr_l = hex($addr_l);
    $addr_h = hex($addr_h);
    $mac_h = ($oui << 8) | $addr_h;
    $mac_l = $addr_l;
}

printf("mach %x macl %x\n", $mac_h, $mac_l);

my @words;
for(my $i=0; $i<4; $i++) {
    push @words, ($mac_l >> ($i * 8)) & 0xff;
}
for(my $i=0; $i<4; $i++) {
    push @words, ($mac_h >> ($i * 8)) & 0xff;
}
reverse @words;
#foreach my $byte (@words) {
#    printf("%02X",$byte);
#}
#print "\n";

my $word1 = sprintf("%02X%02X%02X%02X",$words[4],$words[5],$words[6],$words[7]);
my $word2 = sprintf("%02X%02X%02X%02X",$words[0],$words[1],$words[2],$words[3]);

my $cmd = "mc1322x-load.pl -e -f $bin -z -t $term  -c 'bbmc -l redbee-econotag reset' $addr,0x$word1,0x$word2 &";
print "$cmd\n";
system($cmd);

