#!/usr/bin/perl -w
		
# CGI script that creates a fill-out form
# and echoes back its values.

use CGI qw/:standard/;

# configs

# paths
my $meshpath = "/home/malvira/work";
my $wwwpath = "/var/www";
my $hostname = "hotdog.redwirellc.com";

# aliases
my %aliases = ( 
    "2.0" => { 
	alias  =>  "Lower Door",
	ds=> {
	    "GPIO29" => "Lock (0 - locked, 1 - unlocked)",
	},
    },
    "4.0" => { 
	alias  =>  "Upper Door",
    },
    "1.0" => { 
	alias  =>  "Hotdog (datasink)",
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

    my %dses;
    foreach my $info (@info) {
	next if $info !~ /ds\[([\w\d]+)\]\.([\w\d_]+)\s+=\s+([\w\d]+)/;
	$dses{$1}{$2} = $3;
    }

    my $lastupdate = qx(rrdtool lastupdate $meshpath/$addr.rrd);
    $lastupdate =~ /([\w\d]+)\s+(\d+):\s+([\w\d]+)/;
    print localtime($2) . " $1 $3<br>";
    
    foreach my $ds (keys(%dses)) {
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
