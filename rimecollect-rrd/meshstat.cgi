#!/usr/bin/perl -w
		
# CGI script that creates a fill-out form
# and echoes back its values.

use CGI qw/:standard/;

# configs

# paths
my $meshpath = "/home/malvira/work";
my $wwwpath = "/var/www";
my $hostname = "localhost";

# aliases
my %aliases = ( 
    "2.0" => { 
	alias  =>  "Lower Door",
	ds=> {
	    "GPIO29" => "Lock",
	},
    },
    "4.0" => { 
	alias  =>  "Upper Door",
    },
    "1.0" => { 
	alias  =>  "Sink (Hotdog)",
    },
    );
			 
opendir(MESHDIR, $meshpath);
my @files = readdir(MESHDIR);

print header;
print start_html('Collect Mesh');


foreach my $file (@files) {
    next if $file !~ /([\d\.]+)\.rrd$/;
    my $addr = $1;
    print hr;
    print h1("$addr: $aliases{$addr}{'alias'}");
    my @info = split(/\n/,qx(rrdtool info $meshpath/$addr.rrd));

    my %ds;
    foreach my $info (@info) {
	next if $info !~ /ds\[([\w\d]+)\]/;
	$ds{$1}++;
    }
    
    foreach my $ds (keys(%ds)) {
	print h2("$ds: $aliases{$addr}{'ds'}{$ds}");
	qx(rrdtool graph $wwwpath/$addr-$ds.png --start end-60min DEF:$ds=$meshpath/$addr.rrd:$ds:LAST LINE2:$ds#00a000:\"$ds\");
	print img({src=>"http://$hostname/$addr-$ds.png"});
    }

}

print hr;

print end_html;

#/var/www/demo.png --title="Door" --start end-60min
#           --imginfo '<IMG SRC=http://localhost/demo.png>'
#    DEF:door=/home/malvira/work/2.0.rrd:GPIO29:LAST
#    LINE2:door#00a000:"Door lock">
