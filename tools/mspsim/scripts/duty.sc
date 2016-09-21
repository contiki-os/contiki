echo "#!type line" | window duty
duty 10 "MSP430 Core.active" CC2420.listen CC2420.transmit "Tmote Sky.0" | window duty
echo "#!set 0 label CPU" | window duty
echo "#!set 1 label Listen" | window duty
echo "#!set 2 label Transmit" | window duty
echo "#!set 3 label LEDS" | window duty
echo "#!title Duty Cycle" | window duty