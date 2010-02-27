#!/usr/bin/perl -w
use strict;
my $verbose = 1;

####
# Feed data on stdin from a RIME collect sink
#
# Enters data into  rimeaddr.rrd
# 
# Creates rimeaddr.rrd from a template if data shows up from a source and 
# rimeaddr.rrd doesn't exist 
# 
# default template is read from default.rrdtmpl
#
# if rimeaddr.rrdtmpl exisits, that is used instead.

####
#
# Templates are shell scripts that create the desired rrd
#

####
# Data messages are in the form of:
#
# Sink got message from 1.0, seqno 109, hops 0: len 12 'GPIO29-High'
#
#

my $datapattern = 'Sink got message from ([\d\.]+), seqno \d+, hops \d+: len \d+ \'([\w\d]+-[\w\d]+)\'';

sub rrdcreate {
    my ($newrrd_filename, $tmpl_filename) = @_;
    open FILE, "$tmpl_filename" or die $!;
    my $tmpl = <FILE>;
    chomp $tmpl;
    print "using template $tmpl found in $tmpl_filename\n" if $verbose;
    `rrdtool create $newrrd_filename $tmpl`;
}

while(<>) {

    next if($_ !~ /$datapattern/);
    print("rimeaddr $1 data $2\n") if $verbose;

    my ($ds,$data) = split(/-/,$2);
    print("ds: $ds, data: $data\n") if $verbose;

    if(-e "$1.rrd") {
	# an rrd already exists for this device 
        # do an update
	`rrdtool update $1.rrd -t $ds N:$data`
    } else {
	# an rrd for this device doesn't exist yet
	# find a template and make it
	my $tmpl = "DS:speed:COUNTER:600:U:U RRA:AVERAGE:0.5:6:10";
	print "creating new rrd $1.rrd...   " if $verbose;
	if(-e "$1.rrdtmpl") {
	    rrdcreate("$1.rrd","$1.rrdtmpl");
	    `rrdtool update $1.rrd -t $ds N:$data`
	} elsif(-e "default.rrdtmpl") {
	    rrdcreate("$1.rrd","default.rrdtmpl");
	    `rrdtool update $1.rrd -t $ds N:$data`
	} else {
	    print "can't create rrd for $1: no template found\n";
	}
    }
}
