#!/usr/bin/perl -w

use Device::SerialPort 0.05;
use Term::ReadKey;
use Getopt::Long;

use strict;

my $filename = '';
my $term = '/dev/ttyUSB0';
my $baud = '115200';
my $verbose;

GetOptions ('file=s' => \$filename, 
	    'terminal=s' => \$term, 
	    'verbose' => \$verbose, 
	    'baud=s' => \$baud);

$| = 1;

if($filename eq '') {
    print "Example usage: mc1322x-load.pl -f foo.bin -t /dev/ttyS0 -b 9600\n";
    print "       -f required: binary file to load\n";
    print "       -t default: /dev/ttyUSB0\n";
    print "       -b default: 115200\n";
    exit;
}
	
my $ob = Device::SerialPort->new ($term) or die "Can't start $term\n";
    # next test will die at runtime unless $ob

$baud = 115200 if (!defined($baud));

$ob->baudrate($baud);
$ob->parity('none');
$ob->databits(8);
$ob->stopbits(1);
$ob->handshake("rts");

my $c;

$ob->write(pack('C','0'));

my $ret = '';
until($ret eq 'CONNECT') {
    $c = $ob->input;
    $ret .= $c;
}
print $ret . "\n"; 


if (defined $filename) {

    my $size = -s $filename;

    print ("Size: $size bytes\n");
    $ob->write(pack('V',$size));

    open(FILE, $filename) or die($!);
    print "Sending $filename\n";

    my $i = 1;
    while(read(FILE, $c, 1)) {
	print unpack('H',$c) . unpack('h',$c) if $verbose; 
	print "\n" if ($verbose && ($i%4==0));
	$i++;
	select undef, undef, undef, 0.001;
	$ob->write($c);
    }
}

print "done.\n";

$ob -> close or die "Close failed: $!\n";
ReadMode 0;
undef $ob;  # closes port AND frees memory in perl
exit;

