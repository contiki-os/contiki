#!/usr/bin/perl -w

use Device::SerialPort;
use Term::ReadKey;
use Getopt::Long;
use Time::HiRes qw(usleep);

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

if ($filename eq '') { die "you must specify a file with -f\n"; }

$ob->baudrate($baud);
$ob->parity('none');
$ob->databits(8);
$ob->stopbits(1);
$ob->handshake("rts");
$ob->read_const_time(1000); # 1 second per unfulfilled "read" call

<<<<<<< HEAD:mc1322x-load.pl
my $s = 0;

 SEND:
    do {
	
	my $c;
	my $count;
	my $ret = '';
	
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
	    ($count,$c) = $ob->read(1);
	    if ($count == 0) { 
		print '.';
		$ob->write(pack('C','0')); 
		next;
	    }
	    $ret .= $c;
	}
	print $ret . "\n";
	
	if (-e $filename) {
	    
	    my $size = -s $filename;
	    
	    print ("Size: $size bytes\n");
	    $ob->write(pack('V',$size));
	    
	    open(FILE, $filename) or die($!);
	    print "Sending $filename\n";
	    
	    my $i = 1;
	    while(read(FILE, $c, 1)) {
		print unpack('H',$c) . unpack('h',$c) if $verbose; 
		$i++;
		usleep(50); # this is as fast is it can go... 
		$ob->write($c);
	    }
	}

	if(-e $second) {$s=1; $filename = $second; next SEND; }
	
};

print "done.\n";

while(1) {
    print $ob->input;
}

$ob -> close or die "Close failed: $!\n";
ReadMode 0;
undef $ob;  # closes port AND frees memory in perl
exit;

