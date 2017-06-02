Non-intrusive monitoring of the RF-environment
==============================================

rf_environment runs the clear channel assessment (CCA) test over 
all 802.15.4 channels and reports stats per channel. The CCA test 
is run for different CCA thresholds from -60dBm to -90dBm. CCA is
a non-destructive for the rf-environment as it's just listens.
Best and worst channel is printed as average rf activity.
See example below from Uppsala Kungs-Vaksalagatan. 2017-05-08
and Electrum Stockholm. Originally developed for the Atmel avr-rss2 
platform.

Probability for not passing a CCA check in percent per channel.
3-minute samples. Of course this just snapshots to illustrate 
functionality

<pre>
             Chan:  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26
             ---------------------------------------------------------------------
cca_thresh=-82dBm   0   0   7   0   1   2   2   0   0   0   0   6  30   5   0   1 Best=11 Worst=23 Ave=3.09 
cca_thresh=-80dBm   0   0   1   0   1   0   3   1   0   0   1  16  15   1   2   0 Best=11 Worst=22 Ave=2.31 
cca_thresh=-78dBm   0   0   4  10   0   2   2   2   1   2   0  12  23   4   1   1 Best=11 Worst=23 Ave=3.65 
cca_thresh=-76dBm   0   0  12   8   4   0   6   4  10   3   1  24  15   0   1   1 Best=11 Worst=22 Ave=5.37 
cca_thresh=-74dBm   0   1   1   2   1   0   4   1   2   1   2  10  16  22   5   1 Best=11 Worst=24 Ave=3.96 
cca_thresh=-72dBm   0   1   3   3   3   0   2   1   1   4   2   5   3   8   5   3 Best=11 Worst=24 Ave=2.26 
cca_thresh=-70dBm   0   0   5   3   3   3   1   5   9  26  60  77  53  35  27   8 Best=11 Worst=22 Ave=19.40
cca_thresh=-68dBm   0   1   9  10   1   2   1   3   0   4  59  32  60  37  24   3 Best=11 Worst=23 Ave=14.89
cca_thresh=-66dBm   0   2   3   2   1   2   2   1   5  15  50  64  77  49  16   5 Best=11 Worst=23 Ave=17.87
cca_thresh=-64dBm   1   3   0   1   1   2   1   1   6  18  19  31  62  47  25   3 Best=13 Worst=23 Ave=13.35
cca_thresh=-62dBm   0   0   3   6   2   5   2   0  23  43  37  25  18  32  27  25 Best=11 Worst=20 Ave=15.14
cca_thresh=-60dBm   2   2   3   3   2   3   1   8  34  37  40  49  72  55   9   9 Best=17 Worst=23 Ave=20.17
cca_thresh=-90dBm   0   1  11  10   4   8   2   1  10  22  15  17  22  18   3   9 Best=11 Worst=20 Ave=9.06 
cca_thresh=-88dBm   0   0  17  37   2   3   2   5  12  18  24  43  13  28   6   3 Best=11 Worst=22 Ave=12.90
cca_thresh=-86dBm   0   3  12   2   0   3   3   4  12  11  17  13  42  19  17  10 Best=11 Worst=23 Ave=10.05
cca_thresh=-84dBm   0   0   3   3   0   3   2   4  12  11  14  13  23   9  11  15 Best=11 Worst=23 Ave=7.33 
cca_thresh=-82dBm   0   2  30  24   2   4   2   6   3  11   4  10   8   3   4   1 Best=11 Worst=13 Ave=6.66 
cca_thresh=-80dBm   0   1   9   3   0   1   6   6  15   0   0   8  11   4   3   3 Best=11 Worst=19 Ave=4.05 
cca_thresh=-78dBm   0   1   3   2   0   1   7   8   1   0   4  13   6   3   1   1 Best=11 Worst=22 Ave=2.79 
cca_thresh=-76dBm   0   0   1   7   1   8  11  10  21   1   2  10  28   3   0   1 Best=11 Worst=23 Ave=6.15 
cca_thresh=-74dBm   0   3   2   2   0   1   6   4   8   0   3   5   8   9   0   0 Best=11 Worst=24 Ave=2.77 
cca_thresh=-72dBm   0   0   0   3   1   2   2   2   1   1   3   7  11   9   1   1 Best=11 Worst=23 Ave=2.40 
cca_thresh=-70dBm   0   1  11   2   1   2   4   1   4   4  13  31   7   1   1   1 Best=11 Worst=22 Ave=4.69 
cca_thresh=-68dBm   0   0  13  26   1   5   7   8   3   1   1  20  43   7   1   0 Best=11 Worst=23 Ave=8.21 
cca_thresh=-66dBm   1   2  13   9   1   3   3   1   3  16  11  22   9   7   0   1 Best=25 Worst=22 Ave=5.79 
cca_thresh=-64dBm   0   1   6   2   1   2   2   0   3   8   4   8  14   1   4   1 Best=11 Worst=23 Ave=3.10 
cca_thresh=-62dBm   0   1   4   0   1   2   1   3   1   1   9  16  22   7   1   1 Best=11 Worst=23 Ave=3.91 
cca_thresh=-60dBm   0   1   2   0   1   0   1   1  15  33   4   5  26   2   0   0 Best=11 Worst=20 Ave=5.34 
cca_thresh=-90dBm   0   1   0   2   2   0   1   1   0   1   6  12  24  10   2   1 Best=11 Worst=23 Ave=3.64 
cca_thresh=-88dBm   0   0   0   2   0   1   0   2   0   2   2   9  10   0   0   1 Best=11 Worst=23 Ave=1.54 

Electrum Kista Stockolm (KTH/SICS etc)

cca_thresh=-62dBm   2  17  26   2   2   1  14  18  17   4   9   6   5  31  47   2 Best=16 Worst=25 Ave=12.07
cca_thresh=-60dBm   7   8  13   2   2   1   6   6   7   1  11  32  16  11   1   2 Best=16 Worst=22 Ave=7.54 
cca_thresh=-90dBm   1   9  13   3   2   1  31  32  10   2   2  12   7  10   1   2 Best=11 Worst=18 Ave=8.17 
cca_thresh=-88dBm   4   8  10   2   2   1   5  12   6   3   5   8   2   9   1   2 Best=16 Worst=18 Ave=4.45 
cca_thresh=-86dBm  11   9  10   2   2   1   5  16  21   2   5   5   2   3   1   4 Best=25 Worst=19 Ave=5.88 
cca_thresh=-84dBm   4   9   7   5   5   1   6  46  16   3  16   2  13   5   1   2 Best=25 Worst=18 Ave=8.38 
cca_thresh=-82dBm  15   9  14   2   2   1   7  22  14   1  15   2  10  10   1  19 Best=25 Worst=18 Ave=8.68 
cca_thresh=-80dBm   1   8  16   3   2   1  14  23   6   1  10  22   5   7   3   4 Best=11 Worst=18 Ave=7.38 
cca_thresh=-78dBm  16   9  25   3   3   1   9   8  12   2   7   2   3   2   1   2 Best=16 Worst=13 Ave=6.13 
cca_thresh=-76dBm  12   9  23   4   4   0  36   9  10   2  32  14   7   4   1   3 Best=16 Worst=17 Ave=10.13
cca_thresh=-74dBm   8  24   9   8   5   0  16   6  10   2   3  31  27  18   1   3 Best=16 Worst=22 Ave=10.32
cca_thresh=-72dBm   4   7  18   4   2   0   6  11   7   1   3   6   3   9   1   2 Best=16 Worst=13 Ave=4.80 

</pre>