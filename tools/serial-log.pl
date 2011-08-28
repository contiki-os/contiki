#!/usr/bin/perl -w
#Log serial port to terminal and file with timestamps
#D.Kopf 8/28/2001
#

use Device::SerialPort;
use Term::ReadKey;
use Getopt::Long;
use Time::HiRes qw(usleep);

use strict;

my $term = '/dev/com1';
my $baud = '57600';
my $rts = 'none';
my $logfile = 'none';
my $ssss = '';

GetOptions ('terminal=s' => \$term, 
	'baud=s' => \$baud,
	'rts=s' => \$rts,
	'seconds!' => \$ssss,
	'logfile:s' =>\$logfile
) or goto printhelp;
goto bypass;
printhelp:
	print "Example serial-log.pl -t /dev/ttyS0 -b 57600 -r none -l logfile\n";
	print "       -t, --terminal\n";
	print "       -b, --baud\n";
	print "       -r,--rts [none|rts] flow control\n";
	print "       -s,--seconds Display ssss instead of hh:mm:ss\n";	
	print "       -l,--logfile outputfile (.log will be added, -l defaults to serial.log)\n";
	print "\n";
	print  "The default is equivalent to serial-log.pl -t /dev/com1 -b 57600 -r none\n";
	exit;
bypass:

my $ob = Device::SerialPort->new ($term) or die "Can't start $term\n";
    # next test will die at runtime unless $ob

$ob->baudrate($baud);
$ob->parity('none');
$ob->databits(8);
$ob->stopbits(1);
if($rts eq 'rts') {
	$ob->handshake('rts');
} else {
	$ob->handshake('none');
}
$ob->read_const_time(1000); # 1 second per unfulfilled "read" call
$ob->rts_active(1);

my $c; my $count;
my ($hh,$mm,$ss,$mday,$mon,$year,$wday,$yday,$isdst,$theTime,$seconds,$startseconds);
#use POSIX qw(strftime);

($ss,$mm,$hh,$mday,$mon,$year,$wday,$yday,$isdst) = localtime;
$theTime = sprintf "on %02d/%02d/%04d at %02d:%02d:%02d\n", $mon+1, $mday, $year+1900, $hh, $mm, $ss;
if($logfile ne 'none') {
	if($logfile eq '') {$logfile = 'serial';}
	$logfile ="${logfile}.log";
	$c=1;
	open LOGFILE, "$logfile" or do {$c=0;};
	if ($c) {
		print "File $logfile exists. Restart, append, or quit (r,a,q) (q) ? ";
		#$| = 1;               # force a flush after print
		$_ = <STDIN>;chomp;
		if ($_ eq 'r') {
			open LOGFILE, ">$logfile" or die $!;
			print "Restarting $logfile $theTime\n";
			print LOGFILE "serial-log.pl logging to $logfile started $theTime\n";
		} else {
			if ($_ eq 'a') {
				open LOGFILE, ">>$logfile" or die $!;
				print "Appending to $logfile $theTime\n";
				print LOGFILE "serial-log.pl appending to $logfile $theTime\n";
			} else {
				print "Quitting, file unchanged.\n";
				exit;
			}
		}
	} else {
		open LOGFILE, ">$logfile" or die $!;
		print LOGFILE "serial-log.pl logging to $logfile started $theTime\n";
		print "Starting $logfile $theTime\n";

	}
	LOGFILE->autoflush(1);
} else {
	$logfile='';
	print  "Serial logging of $term started $theTime\n";
}
if($ssss ne '') {
	$startseconds=$hh*3600+$mm*60+$ss;
}

$theTime='';

while(1) {
	($count, $c) = $ob->read(1);
	if (defined($count) && ($count != 0)) {
		if ($theTime eq '') {
			($ss,$mm,$hh) = localtime(time);
			if ($ssss ne '') {
				$seconds = $hh*3600+$mm*60+$ss-$startseconds;
				if ($seconds < 0) {$seconds+=24*60*60;}
				$theTime = sprintf("%04d ",$seconds);
			} else {
				$theTime = sprintf("%02d:%02s:%02d ", $hh,$mm,$ss);
				#$theTime = strftime "%H:%M:%S ", localtime;
			}
			print $theTime ;
			if ($logfile)  {LOGFILE->autoflush(1);print LOGFILE $theTime;}
		}
		print $c ;
		if ($logfile) { print LOGFILE $c;}
		if ($c eq "\n") {$theTime=''};
	}
}
if ($logfile) {close LOGFILE or die $!}
$ob -> close or die "Serial port close failed: $!\n";
ReadMode 0;
undef $ob;  # closes port AND frees memory in perl
exit;
