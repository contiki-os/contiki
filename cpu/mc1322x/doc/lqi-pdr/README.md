lqi-pdr
=======

    grep count 1000pkt-64len.txt | cut -d ' '  -f 2,5 | sed 's/ /,/g' | sort -n > 1000pkt-64len.csv
    asy plot.asy
    gv plot.eps
