#!/usr/bin/perl

# Try:
#  gcc -static -o test test.c -Wl,--print-map | perl parse-map.pl | unflatten -l 3 | dot -Tpng > map.png
#
# This won't show all edges!  An archive member is only listed the first
# time it needs to get included.  But this shows at least one reason why each
# archive member appears in the final object.

my $flag = 0;
my $line = "";
print "digraph map {\n";
print "rankdir = LR;";
while(<>)
{
    $flag++ if /^Archive member included because of file \(symbol\)$/;
    $flag++ if /^$/;
    next unless $flag == 2;

    chomp ($line .= $_);
    if ($line =~ /^(\S+)\s+(\S+)\s+\(([^)]+)\)$/s) {
	$line = "";
	my $archive_member = $1;
	my $because_file = $2;
	my $because_symbol = $3;
	$archive_member =~ s|.*/([^/]+)|$1|;
	$because_file =~ s|.*/([^/]+)|$1|;
	print "\"$because_file\" -> \"$archive_member\";\n";
    }
}
print "}\n";
