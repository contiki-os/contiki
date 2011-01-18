#!/usr/bin/perl -w

use Device::SerialPort;
use Term::ReadKey;
use Getopt::Long;
use Time::HiRes qw(usleep gettimeofday);

use strict;

my $filename = '';
my $second = '';
my $term = '/dev/ttyUSB0';
my $baud = '115200';
my $verbose;

GetOptions (
	    'terminal=s' => \$term, 
	    'baud=s' => \$baud,
    );

$| = 1;

# TODO: add help argument
#    print "Example usage: rftestrx2pcap.pl -t /dev/ttyS0 -b 9600\n";
#    exit;

my $ob = Device::SerialPort->new ($term) or die "Can't start $term\n";
    # next test will die at runtime unless $ob

$ob->baudrate($baud);
$ob->parity('none');
$ob->databits(8);
$ob->stopbits(1);
$ob->read_const_time(1000); # 1 second per unfulfilled "read" call

my $str = '';
my ($sec, $usec, $len);
my @frame;

my $magic = 0xa1b2c3d4;
my $major = 2;
my $minor = 4;
my $zone = 0;
my $sig = 0;
my $snaplen = 0xffff;
my $network = 230; # 802.15.4 no FCS

my $newpacket = 0;
my $len = 0;

print pack('LSSLLLL',($magic,$major,$minor,$zone,$sig,$snaplen,$network));

while(1) {
    my ($count, $c) = $ob->read(1);

    if (defined($count) && ($count != 0)) {
	$str .= $c;
	# match if ends in \n or \r and process line
	if(($str =~ /\n$/) ||
	   ($str =~ /\r$/)) {
	    if($str =~ /^rftest-rx --- len 0x(\w\w)/) {
		#new packet
		($sec, $usec) = gettimeofday;
		$len = hex($1);
		print STDERR "rftestline: $sec $usec $len $str";		
		$newpacket = 1;
	    } elsif($str =~ /^\w+/) {		
		# dataline, write out pcap entry
		$str =~ /(.+)/;
		chomp $str;
		my @data = split(' ',$1);

		# do header if this is a new packet
		if($newpacket == 1) {
		    $newpacket = 0;
		    print pack('LLLL',($sec,$usec,$len,$len));
		    print STDERR "new packet: $sec $usec $len " . ($len) . "\n\r";
		}

		# packet payload
		print STDERR "dataline: ";		
		print STDERR $str . "\n\r";

		foreach my $data (@data) {
		    print pack ('C',hex($data));
		}		
	    }
	    $str = '';
	}
    }
}

$ob -> close or die "Close failed: $!\n";
ReadMode 0;
undef $ob;  # closes port AND frees memory in perl
exit;

