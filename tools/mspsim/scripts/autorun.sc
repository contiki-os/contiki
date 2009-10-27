# autorun script for MSPSim
# - all commands will run after loaded firmware into MSPSim
#log CC2420 >log.txt
#printcalls >>log.txt

# Install and activate the plugin 'ContikiChecker'
# install ContikiChecker
# contikichecker

#start the nodegui service
service controlgui start
service nodegui start
#service stackchart start
start
