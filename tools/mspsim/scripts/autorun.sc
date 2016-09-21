# autorun script for MSPSim
# - all commands will run after loaded firmware into MSPSim
#log CC2420 >log.txt
#printcalls >>log.txt

# Install and activate the plugin 'ContikiChecker'
# install ContikiChecker
# contikichecker

#start the nodegui service
service -f controlgui start
service -f nodegui start
service -f serialgui start
#service -f stackchart start
#rflistener output CC2420 >> rfdata.txt
stop
#start
