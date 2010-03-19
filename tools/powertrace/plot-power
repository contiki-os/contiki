set key top right

set data style boxes
set pointsize 2

set ylabel "Radio duty cycle (%)"
set xlabel "Time (s)"
set terminal postscript eps enhanced "Helvetica" 16 lw 4 dl 5

set output "powertrace-power.eps"
set title "Average radio duty cycle (percent) over time"
plot [0:] [0:] 'powertrace-data' using ($1*10):($2*100) with lines notitle

set output "powertrace-node-power.eps"
set boxwidth 0.3
set title "Per-node radio duty cycle"
set ylabel "Radio duty cycle (%)"
set xlabel "Node number"
plot [-1:] [0:] 'powertrace-node-data' using 0:(100*$1) with boxes fs solid title "Total", \
'' using ($0 + 0.4):(100*$2) with boxes fs solid title "Idle", \
'' using 0:(100*$1):3 with labels center offset 0,1 notitle


set key top left
set boxwidth 0.6
set output "powertrace-sniff-power.eps"
set title "Per-channel radio usage"
set ylabel "Percent (%)"
set xlabel "Channel number"
plot [:] [0:] 'powertrace-sniff-data'  using 0:(100 * ($3 + $2)) with boxes title "Listen", \
'' using 0:(100 * $2):xticlabel(1) with boxes title "Transmission"
