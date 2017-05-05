Non-intrusive monitoring of the RF-environment
==============================================


rf_environment runs clear channel assessment (CCA) test for over 
all 802.15.4 channels and reports stats per channel. The CCA test 
is run for different CCA thresholds from -60 to -190 dBm. CCA is a
non-destructive for the rf-environment it's just listens.

Best and worst channel is printed as average rf activity.

Originally developed for the Atmel avr-rss2 platform.
