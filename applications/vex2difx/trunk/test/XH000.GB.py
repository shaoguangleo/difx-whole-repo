import os

isAstrid = 0
if 1:
    try:
        if os.getenv('ASTRIDVLBA') == '1':
            isAstrid = 1
    except:
        pass

if not isAstrid:
    from edu.nrao.evla.observe import Mark5C
    from edu.nrao.evla.observe import MatrixSwitch
    from edu.nrao.evla.observe import RDBE
    from edu.nrao.evla.observe import VLBALoIfSetup
    from edu.nrao.evla.observe import Parameters
    from edu.nrao.evla.observe import bbc

second = 1.0/86400.0

deltat2 = 1

obsCode = 'XH000'
stnCode = 'GB'
mjdStart = 56534 + 50400*second

# File written by vex2script version 0.24 vintage 20131209

dbe0 = RDBE(0, 'ddc', 'ddc_1501383.bin')
dbe0.setALC(1)
dbe0.setFormat('VDIF')
dbe0.setPSNMode(0)
dbe0.setPacket(0, 0, 28, 5032)
subarray.setDBE(dbe0)

recorder0 = Mark5C('-1')
recorder0.setMode('VDIF')
recorder0.setPSNMode(0)
recorder0.setPacket(0, 0, 28, 5032)
subarray.setRecorder(recorder0)

loif0 = VLBALoIfSetup() 
loif0.setIf('A', '20cm', 'R', 2400, 'L', 'NA', 0, '20cm', 0, 0, 791.5)
loif0.setIf('C', '20cm', 'L', 2400, 'L', 'NA', 0, '20cm', 0, 0, 791.5)
loif0.setPhaseCal(1)
loif0.setDBEParams(0, -1, -1, 10, 0)
loif0.setDBEParams(1, -1, -1, 10, 0)
loif0.setDBERemember(0, 1)
loif0.setDBERemember(1, 1)
channelSet0 = [ \
  bbc(0, 791.5, 32, 'L', 2, 0),   # IF A
  bbc(0, 759.5, 32, 'L', 2, 2),   # IF A
  bbc(1, 791.5, 32, 'L', 2, 1),   # IF C
  bbc(1, 759.5, 32, 'L', 2, 3)    # IF C
  ]

loif1 = VLBALoIfSetup() 
loif1.setIf('A', '4cm', 'R', 9100, 'L', 'NA', 0, '4cm', 0, 0, 715.5)
loif1.setIf('C', '4cm', 'L', 9100, 'L', 'NA', 0, '4cm', 0, 0, 715.5)
loif1.setPhaseCal(1)
loif1.setDBEParams(0, -1, -1, 10, 0)
loif1.setDBEParams(1, -1, -1, 10, 0)
loif1.setDBERemember(0, 1)
loif1.setDBERemember(1, 1)
channelSet1 = [ \
  bbc(0, 715.5, 32, 'L', 2, 0),   # IF A
  bbc(0, 683.5, 32, 'L', 2, 2),   # IF A
  bbc(1, 715.5, 32, 'L', 2, 1),   # IF C
  bbc(1, 683.5, 32, 'L', 2, 3)    # IF C
  ]

source0 = Source(0.513051528326295, 1.30397053367308)
source0.setName('0153+744')

# Setup Scan 
# changing to mode v18cm-512-4-2
subarray.setChannels(dbe0, channelSet0)
subarray.setVLBALoIfSetup(dbe0, loif0)
subarray.set4x4Switch('1A', 1)
subarray.set4x4Switch('1B', 3)
subarray.setSource(source0)
# Setup scan - run right away, but do not start recording
subarray.execute( array.time() + 2*second )

# Scan 0 = No0001
recorder0.setPacket(0, 0, 28, 5032)
subarray.setRecord(mjdStart + 0*second, mjdStart+840*second, 'No0001', obsCode, stnCode )
if array.time() < mjdStart + (840-10)*second:
  subarray.execute(mjdStart + 835*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+840*second) + ' since array.time is ' + str(array.time())

# Scan 1 = No0002
recorder0.setPacket(0, 0, 28, 5032)
subarray.setRecord(mjdStart + 900*second, mjdStart+1740*second, 'No0002', obsCode, stnCode )
if array.time() < mjdStart + (1740-10)*second:
  subarray.execute(mjdStart + 1735*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1740*second) + ' since array.time is ' + str(array.time())

# Scan 2 = No0003
recorder0.setPacket(0, 0, 28, 5032)
subarray.setRecord(mjdStart + 1800*second, mjdStart+2640*second, 'No0003', obsCode, stnCode )
if array.time() < mjdStart + (2640-10)*second:
  subarray.execute(mjdStart + 2635*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2640*second) + ' since array.time is ' + str(array.time())

# Scan 3 = No0004
recorder0.setPacket(0, 0, 28, 5032)
subarray.setRecord(mjdStart + 2700*second, mjdStart+3540*second, 'No0004', obsCode, stnCode )
if array.time() < mjdStart + (3540-10)*second:
  subarray.execute(mjdStart + 3535*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3540*second) + ' since array.time is ' + str(array.time())

# Scan 4 = No0005
# changing to mode v4cm-512-4-2
subarray.setChannels(dbe0, channelSet1)
subarray.setVLBALoIfSetup(dbe0, loif1)
subarray.set4x4Switch('1A', 1)
subarray.set4x4Switch('1B', 3)
recorder0.setPacket(0, 0, 28, 5032)
subarray.setRecord(mjdStart + 3600*second, mjdStart+4440*second, 'No0005', obsCode, stnCode )
if array.time() < mjdStart + (4440-10)*second:
  subarray.execute(mjdStart + 4435*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4440*second) + ' since array.time is ' + str(array.time())

# Scan 5 = No0006
recorder0.setPacket(0, 0, 28, 5032)
subarray.setRecord(mjdStart + 4500*second, mjdStart+5340*second, 'No0006', obsCode, stnCode )
if array.time() < mjdStart + (5340-10)*second:
  subarray.execute(mjdStart + 5335*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5340*second) + ' since array.time is ' + str(array.time())

# Scan 6 = No0007
recorder0.setPacket(0, 0, 28, 5032)
subarray.setRecord(mjdStart + 5400*second, mjdStart+6240*second, 'No0007', obsCode, stnCode )
if array.time() < mjdStart + (6240-10)*second:
  subarray.execute(mjdStart + 6235*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6240*second) + ' since array.time is ' + str(array.time())

# Scan 7 = No0008
recorder0.setPacket(0, 0, 28, 5032)
subarray.setRecord(mjdStart + 6300*second, mjdStart+7140*second, 'No0008', obsCode, stnCode )
if array.time() < mjdStart + (7140-10)*second:
  subarray.execute(mjdStart + 7135*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7140*second) + ' since array.time is ' + str(array.time())

array.wait(mjdStart + 7141*second)
