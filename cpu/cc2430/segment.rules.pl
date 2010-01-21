  $match_me = pop @ARGV; 
  #print "searching for $match_me\n";
  while (<>) {
    s/#.*$//; # filter out comments
    s/^\s*//; # filter out leading white space
    @F=split /\s+/; # split on white space
    if (($_ =~ m/\S\s+\S/) && ($match_me =~ m/$F[1]/)) { 
	print $F[0]."\n"; 
	exit; #return only first match
    }
  }
