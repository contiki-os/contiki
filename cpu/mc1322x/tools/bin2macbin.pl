#!/usr/bin/perl

use strict;
use File::Copy;
use Getopt::Long;
use File::Basename;

my $cmd;
my $oui;
my $iab;

GetOptions ('iab=s' => \$iab,
	    'oui=s' => \$oui,
    ) or die 'bad options';

my $address = shift;
my $infile = shift;
my $outfile = shift;

if (!defined($address) || !defined($infile)) {
	print "Usage: $0 [--iab=a8c | --oui=abcdef ] address infile [outfile]\n";
	print "          iab is 12-bit and address is 28-bit e.g. --iab=a8c 1234567\n";
	print "          oui is 24-bit and address is 40-bit e.g. --oui=abcdef 123456789a\n";
	print "\n";
	print "          if outfile is not specified, for infile foo.bin outfile will be\n";
	print "          foo-[macaddress].bin e.g:\n";
        print "          for --iab=a8c 1234567 foo.bin -> foo-0050c2a8c1234567.bin\n";
        print "          for --oui=abcdef 123456789a foo.bin -> foo-abcdef123456789a.bin\n";
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

if(!defined($outfile))
{
    my $basename = basename($infile,(".bin"));
    $outfile = sprintf("-%08x%08x.bin",$mac_h, $mac_l);
    $outfile = $basename . $outfile;
    print "outfile $outfile\n";
}

copy($infile, $outfile) or die("Couldn't copy $infile to $outfile");
$cmd = sprintf("echo -n -e '\\x%02X\\x%02X\\x%02X\\x%02X\\x%02X\\x%02X\\x%02X\\x%02X' | dd of=$outfile bs=1 seek=122872 conv=notrunc",
	       $words[7],$words[6],$words[5],$words[4],
	       $words[3],$words[2],$words[1],$words[0]);

print "$cmd\n";
system("bash -c \"$cmd\"");


