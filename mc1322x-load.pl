#!/usr/bin/perl -w

use Device::SerialPort 0.05;
use Term::ReadKey;
use Getopt::Long;

use strict;

my $filename = '';
my $second = '';
my $term = '/dev/ttyUSB0';
my $baud = '115200';
my $verbose;

GetOptions ('file=s' => \$filename,
	    'secondfile=s' => \$second,
	    'terminal=s' => \$term, 
	    'verbose' => \$verbose, 
	    'baud=s' => \$baud);

$| = 1;

if($filename eq '') {
    print "Example usage: mc1322x-load.pl -f foo.bin -t /dev/ttyS0 -b 9600\n";
    print "          or : mc1322x-load.pl -f flasher.bin -s flashme.bin\n";
    print "       -f required: binary file to load\n";
    print "       -s optional: secondary binary file to send\n";
    print "       -t default: /dev/ttyUSB0\n";
    print "       -b default: 115200\n";
    exit;
}
	
my $ob = Device::SerialPort->new ($term) or die "Can't start $term\n";
    # next test will die at runtime unless $ob

if(($filename eq '')) die "you must specify a file with -f\n";

$ob->baudrate($baud);
$ob->parity('none');
$ob->databits(8);
$ob->stopbits(1);
$ob->handshake("rts");

my $s = 0;

 SEND:
    do {
	
	my $c;

	if($s == 1) { print "performing secondary send\n"; }
	
	$ob->write(pack('C','0'));
	
	my $ret = '';
	my $test;

	if($s == 1) { 
	    $test = 'ready'; 
	} else {
	    $test = 'CONNECT';
	}

	until($ret eq $test) {
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

	if(-e $second) {$s=1; $filename = $second; continue SEND; }
	
}

print "done.\n";

$ob -> close or die "Close failed: $!\n";
ReadMode 0;
undef $ob;  # closes port AND frees memory in perl
exit;

