import sys
import re

import subprocess
import multiprocessing
from multiprocessing import Pool, Lock

flashProgram = 'c:\\Jennic\\Tools\\flashprogrammer\\FlashCLI.exe'

def serialdump(port):
  cmd=['start','make','TARGET=jn5168','serialdump',port]
  rv = subprocess.call(cmd, shell=True)

def serialdumpPorts(comStr):
  ttyPorts = []
  for port in comStr:
    portNum=int(port.replace('COM',''))-1
    ttyPorts.append('ttyS'+str(portNum))
          
  p = Pool()
  p.map(serialdump, ttyPorts)
  p.close()

def list_mote():
  #There is no COM0 in windows. We use this to trigger an error message that lists all valid COM ports
  cmd=[flashProgram,'-c','COM0']
  proc = subprocess.Popen(cmd, shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,)
  stdout_value, stderr_value = proc.communicate('through stdin to stdout')
  com_str = (stderr_value)
  #print '\tpass through:', repr(stdout_value)
  #print '\tstderr      :', com_str
  
  ## Extract COM ports from output:
  ## Example com_str: "Available ports: ['COM15', 'COM10']" 
  res = re.compile('\[((?:\'COM\d+\'.?.?)+)\]').search(com_str)
  #res = re.compile('.*\[(.*)\].*').match(com_str)
  
  ports = []
  if res:
    portsStr=str(res.group(1))
    #print portsStr
    ports=portsStr.replace('\'', '').replace(',','').split()
  return ports

def extractInformation(inputStr):
    stdout_value=(inputStr)
    portStr=''
    macStr=''
    info=''
    programSuccess=''
    
    #print 'output: ', inputStr
    
    res = re.compile('Connecting to device on (COM\d+)').search(stdout_value) 
    if res:
      portStr=str(res.group(1))
      
    ### extracting the following information    
    '''
    Devicelabel:           JN516x, BL 0x00080006
    FlashLabel:            Internal Flash (256K)
    Memory:                0x00008000 bytes RAM, 0x00040000 bytes Flash
    ChipPartNo:            8
    ChipRevNo:             1
    ROM Version:           0x00080006
    MAC Address:           00:15:8D:00:00:35:DD:FB
    ZB License:            0x00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00
    User Data:             00:00:00:00:00:00:00:00
    FlashMID:              0xCC
    FlashDID:              0xEE
    MacLocation:           0x00000010
    Sector Length:         0x08000
    Bootloader Version:    0x00080006
    '''
    
    res = re.compile('(Devicelabel.*\sFlashLabel.*\sMemory.*\sChipPartNo.*\sChipRevNo.*\sROM Version.*\sMAC Address.*\sZB License.*\sUser Data.*\sFlashMID.*\sFlashDID.*\sMacLocation.*\sSector Length.*\sBootloader Version\:\s+0x\w{8})').search(stdout_value) 
    if res:
      info=str(res.group(1))
      
    res = re.compile('MAC Address\:\s+((?:\w{2}\:?){8})').search(stdout_value)
    if res:
      macStr=str(res.group(1))
    
    res = re.compile('(Program\ssuccessfully\swritten\sto\sflash)').search(stdout_value)
    if res:
      programSuccess=str(res.group(1))
      
    return [portStr, macStr, info, programSuccess] 
        
def program_motes(motes, firmwareFile):
  for m in motes:
    cmd=[flashProgram,'-c',m, '-B', '1000000', '-s', '-w', '-f', firmwareFile]
    proc = subprocess.Popen(cmd, shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,)
    stdout_value, stderr_value = proc.communicate('through stdin to stdout')   
    [portStr, macStr, info, programSuccess]=extractInformation(stdout_value)               
    print portStr, macStr, programSuccess
      
    errors = (stderr_value)
    if errors != '':
    	print 'Errors:', errors  

def motes_info(motes, macOnly):
  if macOnly:
    print "Listing Mac addresses:"
  else:
    print "Listing motes info:" 
  for m in motes:
    cmd=[flashProgram,'-c',m, '-B', '1000000']
    proc = subprocess.Popen(cmd, shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,)
    stdout_value, stderr_value = proc.communicate('through stdin to stdout')   
    [portStr, macStr, info, programSuccess]=extractInformation(stdout_value)  
    errors = (stderr_value)
    
    if macOnly:
      print portStr, macStr
    else:
      print portStr, '\n', info, '\n'    
          
    if errors != '':
      print 'Errors:', errors                    

def main():
   if len(sys.argv) > 2:
   	flashProgram=sys.argv[1]

   motes=list_mote()
   if motes:
    motes.sort()
   print 'Found %d JN5168 motes at:' %(len(motes))
   motesStr=''
   for m in motes:
    motesStr += "%s " %(str(m))
   print motesStr 
   firmwareFile='#'	

   if len(sys.argv) > 2:   		
    firmwareFile=sys.argv[2]
   elif len(sys.argv) > 1:
   	firmwareFile=sys.argv[1]		   
   
   if firmwareFile not in ['#', '!', '?', '%']:    	
   	print '\nBatch programming all connected motes...\n'
   	program_motes(motes, firmwareFile)
   elif firmwareFile == '?' or firmwareFile == '!':
    displayMacList=(firmwareFile == '!')
    motes_info(motes,displayMacList)          
   elif firmwareFile == '%':
    print '\nLogging from all connected motes...\n'
    serialdumpPorts(motes)         
   else:
    print '\nNo firmware file specified.\n'

if __name__ == '__main__':
  multiprocessing.freeze_support()         
  main()
