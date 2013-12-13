from edu.nrao.evla.observe import Mark5C
from edu.nrao.evla.observe import ESSR
from edu.nrao.evla.observe import MatrixSwitch
from edu.nrao.evla.observe import RDBE
from edu.nrao.evla.observe import VLBALoIfSetup
from edu.nrao.evla.observe import Parameters
from edu.nrao.evla.observe import bbc

second = 1.0/86400.0

deltat2 = 1

obsCode = 'BN048C'
stnCode = 'LA'
mjdStart = 56459 + 77475*second

# File written by vex2script version 0.24 vintage 20131209

dbe0 = RDBE(0, 'ddc', 'ddc_1411380.bin')
dbe0.setALC(1)
dbe0.setFormat('Mark5B')
dbe0.setPSNMode(0)
dbe0.setPacket(0, 0, 36, 5008)
subarray.setDBE(dbe0)

recorder0 = Mark5C('-1')
recorder0.setMode('Mark5B')
recorder0.setPSNMode(0)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecorder(recorder0)

# XCUBE init
essr = ESSR()          # constructor
essr.setMode(4)        # one in, one out
essr.setRoute(2, 4)    # route incoming traffic from input 1 to output 1
essr.setPace(4, 5)     # set port 4 packet pacing to 5
subarray.setESSR(essr)

loif0 = VLBALoIfSetup() 
loif0.setIf('A', '13cm', 'R', 2900, 'L', 'NA', 0, '13cm', 768)
loif0.setIf('B', '4cm', 'R', 7900, 'U', 'NA', 0, '4cm', 768)
loif0.setPhaseCal(5)
loif0.setDBEParams(0, -1, -1, 10, 0)
loif0.setDBEParams(1, -1, -1, 10, 0)
loif0.setDBERemember(0, 1)
loif0.setDBERemember(1, 1)
channelSet0 = [ \
  bbc(1, 768, 128, 'L', 2, 0), \  # IF B
  bbc(0, 768, 128, 'L', 2, 0), \  # IF A
  bbc(1, 640, 128, 'L', 2, 0), \  # IF B
  bbc(0, 640, 128, 'L', 2, 0) \  # IF A
  ]

loif1 = VLBALoIfSetup() 
loif1.setIf('D', '2cm', 'L', 14400, 'U', 'NA', 0, '2cm', 896)
loif1.setPhaseCal(5)
loif1.setDBEParams(0, -1, -1, 10, 0)
loif1.setDBERemember(0, 1)
channelSet1 = [ \
  bbc(0, 896, 128, 'U', 2, 0), \  # IF D
  bbc(0, 768, 128, 'U', 2, 0), \  # IF D
  bbc(0, 640, 128, 'U', 2, 0), \  # IF D
  bbc(0, 512, 128, 'U', 2, 0) \  # IF D
  ]

loif2 = VLBALoIfSetup() 
loif2.setIf('D', '1cm', 'L', 23000, 'U', 'NA', 14100, '1cm', 896)
loif2.setPhaseCal(5)
loif2.setDBEParams(0, -1, -1, 10, 0)
loif2.setDBERemember(0, 1)
channelSet2 = [ \
  bbc(0, 896, 128, 'U', 2, 0), \  # IF D
  bbc(0, 768, 128, 'U', 2, 0), \  # IF D
  bbc(0, 640, 128, 'U', 2, 0), \  # IF D
  bbc(0, 512, 128, 'U', 2, 0) \  # IF D
  ]

loif3 = VLBALoIfSetup() 
loif3.setIf('C', '7mm', 'L', 42400, 'U', 'NA', 34800, '7mm', 896)
loif3.setPhaseCal(5)
loif3.setDBEParams(0, -1, -1, 10, 0)
loif3.setDBERemember(0, 1)
channelSet3 = [ \
  bbc(0, 896, 128, 'U', 2, 0), \  # IF C
  bbc(0, 768, 128, 'U', 2, 0), \  # IF C
  bbc(0, 640, 128, 'U', 2, 0), \  # IF C
  bbc(0, 512, 128, 'U', 2, 0) \  # IF C
  ]

source0 = Source(2.47422339445842, 0.681361276928833)
source0.setName('J0927+3902')

source1 = Source(2.88634331150412, 0.682000172785076)
source1.setName('J1101+3904')

source2 = Source(2.89923288444375, 0.666869939921883)
source2.setName('J1104+3812')

# Setup Scan 
# changing to mode trdbe.sx
subarray.setChannels(dbe0, channelSet0)
subarray.setVLBALoIfSetup(dbe0, loif0)
subarray.set4x4Switch('1A', 1)
subarray.set4x4Switch('1B', 2)
subarray.setSource(source0)
# Setup scan - run right away, but do not start recording
subarray.execute( array.time() + 2*second )

# Scan 0 = No0001
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 0*second, mjdStart+119*second, 'No0001', obsCode, stnCode )
if array.time() < mjdStart + (119-10)*second:
  subarray.execute(mjdStart + 114*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+119*second) + ' since array.time is ' + str(array.time())

# Scan 1 = No0002
# changing to mode rdbe.1cm
subarray.setChannels(dbe0, channelSet2)
subarray.setVLBALoIfSetup(dbe0, loif2)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 134*second, mjdStart+239*second, 'No0002', obsCode, stnCode )
if array.time() < mjdStart + (239-10)*second:
  subarray.execute(mjdStart + 234*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+239*second) + ' since array.time is ' + str(array.time())

# Scan 2 = No0003
# changing to mode rdbe.2cm
subarray.setChannels(dbe0, channelSet1)
subarray.setVLBALoIfSetup(dbe0, loif1)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 254*second, mjdStart+359*second, 'No0003', obsCode, stnCode )
if array.time() < mjdStart + (359-10)*second:
  subarray.execute(mjdStart + 354*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+359*second) + ' since array.time is ' + str(array.time())

# Scan 3 = No0004
# changing to mode rdbe.7mm
subarray.setChannels(dbe0, channelSet3)
subarray.setVLBALoIfSetup(dbe0, loif3)
subarray.set4x4Switch('1A', 3)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 374*second, mjdStart+478*second, 'No0004', obsCode, stnCode )
if array.time() < mjdStart + (478-10)*second:
  subarray.execute(mjdStart + 473*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+478*second) + ' since array.time is ' + str(array.time())

# Scan 4 = No0006
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 512*second, mjdStart+528*second, 'No0006', obsCode, stnCode )
if array.time() < mjdStart + (528-10)*second:
  subarray.execute(mjdStart + 523*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+528*second) + ' since array.time is ' + str(array.time())

# Scan 5 = No0007
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 537*second, mjdStart+553*second, 'No0007', obsCode, stnCode )
if array.time() < mjdStart + (553-10)*second:
  subarray.execute(mjdStart + 548*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+553*second) + ' since array.time is ' + str(array.time())

# Scan 6 = No0008
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 562*second, mjdStart+578*second, 'No0008', obsCode, stnCode )
if array.time() < mjdStart + (578-10)*second:
  subarray.execute(mjdStart + 573*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+578*second) + ' since array.time is ' + str(array.time())

# Scan 7 = No0009
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 587*second, mjdStart+603*second, 'No0009', obsCode, stnCode )
if array.time() < mjdStart + (603-10)*second:
  subarray.execute(mjdStart + 598*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+603*second) + ' since array.time is ' + str(array.time())

# Scan 8 = No0010
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 612*second, mjdStart+628*second, 'No0010', obsCode, stnCode )
if array.time() < mjdStart + (628-10)*second:
  subarray.execute(mjdStart + 623*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+628*second) + ' since array.time is ' + str(array.time())

# Scan 9 = No0011
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 637*second, mjdStart+653*second, 'No0011', obsCode, stnCode )
if array.time() < mjdStart + (653-10)*second:
  subarray.execute(mjdStart + 648*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+653*second) + ' since array.time is ' + str(array.time())

# Scan 10 = No0012
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 662*second, mjdStart+678*second, 'No0012', obsCode, stnCode )
if array.time() < mjdStart + (678-10)*second:
  subarray.execute(mjdStart + 673*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+678*second) + ' since array.time is ' + str(array.time())

# Scan 11 = No0013
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 687*second, mjdStart+703*second, 'No0013', obsCode, stnCode )
if array.time() < mjdStart + (703-10)*second:
  subarray.execute(mjdStart + 698*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+703*second) + ' since array.time is ' + str(array.time())

# Scan 12 = No0014
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 712*second, mjdStart+728*second, 'No0014', obsCode, stnCode )
if array.time() < mjdStart + (728-10)*second:
  subarray.execute(mjdStart + 723*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+728*second) + ' since array.time is ' + str(array.time())

# Scan 13 = No0015
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 737*second, mjdStart+753*second, 'No0015', obsCode, stnCode )
if array.time() < mjdStart + (753-10)*second:
  subarray.execute(mjdStart + 748*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+753*second) + ' since array.time is ' + str(array.time())

# Scan 14 = No0016
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 762*second, mjdStart+777*second, 'No0016', obsCode, stnCode )
if array.time() < mjdStart + (777-10)*second:
  subarray.execute(mjdStart + 772*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+777*second) + ' since array.time is ' + str(array.time())

# Scan 15 = No0017
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 786*second, mjdStart+802*second, 'No0017', obsCode, stnCode )
if array.time() < mjdStart + (802-10)*second:
  subarray.execute(mjdStart + 797*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+802*second) + ' since array.time is ' + str(array.time())

# Scan 16 = No0018
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 811*second, mjdStart+827*second, 'No0018', obsCode, stnCode )
if array.time() < mjdStart + (827-10)*second:
  subarray.execute(mjdStart + 822*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+827*second) + ' since array.time is ' + str(array.time())

# Scan 17 = No0019
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 836*second, mjdStart+852*second, 'No0019', obsCode, stnCode )
if array.time() < mjdStart + (852-10)*second:
  subarray.execute(mjdStart + 847*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+852*second) + ' since array.time is ' + str(array.time())

# Scan 18 = No0020
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 861*second, mjdStart+877*second, 'No0020', obsCode, stnCode )
if array.time() < mjdStart + (877-10)*second:
  subarray.execute(mjdStart + 872*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+877*second) + ' since array.time is ' + str(array.time())

# Scan 19 = No0021
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 886*second, mjdStart+902*second, 'No0021', obsCode, stnCode )
if array.time() < mjdStart + (902-10)*second:
  subarray.execute(mjdStart + 897*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+902*second) + ' since array.time is ' + str(array.time())

# Scan 20 = No0022
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 911*second, mjdStart+927*second, 'No0022', obsCode, stnCode )
if array.time() < mjdStart + (927-10)*second:
  subarray.execute(mjdStart + 922*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+927*second) + ' since array.time is ' + str(array.time())

# Scan 21 = No0023
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 936*second, mjdStart+952*second, 'No0023', obsCode, stnCode )
if array.time() < mjdStart + (952-10)*second:
  subarray.execute(mjdStart + 947*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+952*second) + ' since array.time is ' + str(array.time())

# Scan 22 = No0024
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 961*second, mjdStart+977*second, 'No0024', obsCode, stnCode )
if array.time() < mjdStart + (977-10)*second:
  subarray.execute(mjdStart + 972*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+977*second) + ' since array.time is ' + str(array.time())

# Scan 23 = No0025
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 986*second, mjdStart+1002*second, 'No0025', obsCode, stnCode )
if array.time() < mjdStart + (1002-10)*second:
  subarray.execute(mjdStart + 997*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1002*second) + ' since array.time is ' + str(array.time())

# Scan 24 = No0026
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1011*second, mjdStart+1027*second, 'No0026', obsCode, stnCode )
if array.time() < mjdStart + (1027-10)*second:
  subarray.execute(mjdStart + 1022*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1027*second) + ' since array.time is ' + str(array.time())

# Scan 25 = No0027
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1036*second, mjdStart+1052*second, 'No0027', obsCode, stnCode )
if array.time() < mjdStart + (1052-10)*second:
  subarray.execute(mjdStart + 1047*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1052*second) + ' since array.time is ' + str(array.time())

# Scan 26 = No0028
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1061*second, mjdStart+1077*second, 'No0028', obsCode, stnCode )
if array.time() < mjdStart + (1077-10)*second:
  subarray.execute(mjdStart + 1072*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1077*second) + ' since array.time is ' + str(array.time())

# Scan 27 = No0029
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1086*second, mjdStart+1102*second, 'No0029', obsCode, stnCode )
if array.time() < mjdStart + (1102-10)*second:
  subarray.execute(mjdStart + 1097*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1102*second) + ' since array.time is ' + str(array.time())

# Scan 28 = No0030
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1111*second, mjdStart+1126*second, 'No0030', obsCode, stnCode )
if array.time() < mjdStart + (1126-10)*second:
  subarray.execute(mjdStart + 1121*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1126*second) + ' since array.time is ' + str(array.time())

# Scan 29 = No0031
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1135*second, mjdStart+1151*second, 'No0031', obsCode, stnCode )
if array.time() < mjdStart + (1151-10)*second:
  subarray.execute(mjdStart + 1146*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1151*second) + ' since array.time is ' + str(array.time())

# Scan 30 = No0032
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1160*second, mjdStart+1176*second, 'No0032', obsCode, stnCode )
if array.time() < mjdStart + (1176-10)*second:
  subarray.execute(mjdStart + 1171*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1176*second) + ' since array.time is ' + str(array.time())

# Scan 31 = No0033
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1185*second, mjdStart+1201*second, 'No0033', obsCode, stnCode )
if array.time() < mjdStart + (1201-10)*second:
  subarray.execute(mjdStart + 1196*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1201*second) + ' since array.time is ' + str(array.time())

# Scan 32 = No0034
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1210*second, mjdStart+1226*second, 'No0034', obsCode, stnCode )
if array.time() < mjdStart + (1226-10)*second:
  subarray.execute(mjdStart + 1221*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1226*second) + ' since array.time is ' + str(array.time())

# Scan 33 = No0035
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1235*second, mjdStart+1251*second, 'No0035', obsCode, stnCode )
if array.time() < mjdStart + (1251-10)*second:
  subarray.execute(mjdStart + 1246*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1251*second) + ' since array.time is ' + str(array.time())

# Scan 34 = No0036
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1260*second, mjdStart+1276*second, 'No0036', obsCode, stnCode )
if array.time() < mjdStart + (1276-10)*second:
  subarray.execute(mjdStart + 1271*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1276*second) + ' since array.time is ' + str(array.time())

# Scan 35 = No0037
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1285*second, mjdStart+1301*second, 'No0037', obsCode, stnCode )
if array.time() < mjdStart + (1301-10)*second:
  subarray.execute(mjdStart + 1296*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1301*second) + ' since array.time is ' + str(array.time())

# Scan 36 = No0038
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1310*second, mjdStart+1326*second, 'No0038', obsCode, stnCode )
if array.time() < mjdStart + (1326-10)*second:
  subarray.execute(mjdStart + 1321*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1326*second) + ' since array.time is ' + str(array.time())

# Scan 37 = No0039
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1335*second, mjdStart+1351*second, 'No0039', obsCode, stnCode )
if array.time() < mjdStart + (1351-10)*second:
  subarray.execute(mjdStart + 1346*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1351*second) + ' since array.time is ' + str(array.time())

# Scan 38 = No0040
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1360*second, mjdStart+1376*second, 'No0040', obsCode, stnCode )
if array.time() < mjdStart + (1376-10)*second:
  subarray.execute(mjdStart + 1371*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1376*second) + ' since array.time is ' + str(array.time())

# Scan 39 = No0041
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1385*second, mjdStart+1401*second, 'No0041', obsCode, stnCode )
if array.time() < mjdStart + (1401-10)*second:
  subarray.execute(mjdStart + 1396*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1401*second) + ' since array.time is ' + str(array.time())

# Scan 40 = No0042
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1410*second, mjdStart+1426*second, 'No0042', obsCode, stnCode )
if array.time() < mjdStart + (1426-10)*second:
  subarray.execute(mjdStart + 1421*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1426*second) + ' since array.time is ' + str(array.time())

# Scan 41 = No0043
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1435*second, mjdStart+1451*second, 'No0043', obsCode, stnCode )
if array.time() < mjdStart + (1451-10)*second:
  subarray.execute(mjdStart + 1446*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1451*second) + ' since array.time is ' + str(array.time())

# Scan 42 = No0044
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1460*second, mjdStart+1476*second, 'No0044', obsCode, stnCode )
if array.time() < mjdStart + (1476-10)*second:
  subarray.execute(mjdStart + 1471*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1476*second) + ' since array.time is ' + str(array.time())

# Scan 43 = No0045
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1485*second, mjdStart+1500*second, 'No0045', obsCode, stnCode )
if array.time() < mjdStart + (1500-10)*second:
  subarray.execute(mjdStart + 1495*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1500*second) + ' since array.time is ' + str(array.time())

# Scan 44 = No0046
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1509*second, mjdStart+1525*second, 'No0046', obsCode, stnCode )
if array.time() < mjdStart + (1525-10)*second:
  subarray.execute(mjdStart + 1520*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1525*second) + ' since array.time is ' + str(array.time())

# Scan 45 = No0047
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1534*second, mjdStart+1550*second, 'No0047', obsCode, stnCode )
if array.time() < mjdStart + (1550-10)*second:
  subarray.execute(mjdStart + 1545*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1550*second) + ' since array.time is ' + str(array.time())

# Scan 46 = No0048
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1559*second, mjdStart+1575*second, 'No0048', obsCode, stnCode )
if array.time() < mjdStart + (1575-10)*second:
  subarray.execute(mjdStart + 1570*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1575*second) + ' since array.time is ' + str(array.time())

# Scan 47 = No0049
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1584*second, mjdStart+1600*second, 'No0049', obsCode, stnCode )
if array.time() < mjdStart + (1600-10)*second:
  subarray.execute(mjdStart + 1595*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1600*second) + ' since array.time is ' + str(array.time())

# Scan 48 = No0050
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1609*second, mjdStart+1625*second, 'No0050', obsCode, stnCode )
if array.time() < mjdStart + (1625-10)*second:
  subarray.execute(mjdStart + 1620*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1625*second) + ' since array.time is ' + str(array.time())

# Scan 49 = No0051
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1634*second, mjdStart+1650*second, 'No0051', obsCode, stnCode )
if array.time() < mjdStart + (1650-10)*second:
  subarray.execute(mjdStart + 1645*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1650*second) + ' since array.time is ' + str(array.time())

# Scan 50 = No0052
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1659*second, mjdStart+1675*second, 'No0052', obsCode, stnCode )
if array.time() < mjdStart + (1675-10)*second:
  subarray.execute(mjdStart + 1670*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1675*second) + ' since array.time is ' + str(array.time())

# Scan 51 = No0053
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1684*second, mjdStart+1700*second, 'No0053', obsCode, stnCode )
if array.time() < mjdStart + (1700-10)*second:
  subarray.execute(mjdStart + 1695*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1700*second) + ' since array.time is ' + str(array.time())

# Scan 52 = No0054
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1709*second, mjdStart+1725*second, 'No0054', obsCode, stnCode )
if array.time() < mjdStart + (1725-10)*second:
  subarray.execute(mjdStart + 1720*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1725*second) + ' since array.time is ' + str(array.time())

# Scan 53 = No0055
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1734*second, mjdStart+1750*second, 'No0055', obsCode, stnCode )
if array.time() < mjdStart + (1750-10)*second:
  subarray.execute(mjdStart + 1745*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1750*second) + ' since array.time is ' + str(array.time())

# Scan 54 = No0056
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1759*second, mjdStart+1775*second, 'No0056', obsCode, stnCode )
if array.time() < mjdStart + (1775-10)*second:
  subarray.execute(mjdStart + 1770*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1775*second) + ' since array.time is ' + str(array.time())

# Scan 55 = No0057
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1784*second, mjdStart+1800*second, 'No0057', obsCode, stnCode )
if array.time() < mjdStart + (1800-10)*second:
  subarray.execute(mjdStart + 1795*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1800*second) + ' since array.time is ' + str(array.time())

# Scan 56 = No0058
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1809*second, mjdStart+1825*second, 'No0058', obsCode, stnCode )
if array.time() < mjdStart + (1825-10)*second:
  subarray.execute(mjdStart + 1820*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1825*second) + ' since array.time is ' + str(array.time())

# Scan 57 = No0059
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1834*second, mjdStart+1850*second, 'No0059', obsCode, stnCode )
if array.time() < mjdStart + (1850-10)*second:
  subarray.execute(mjdStart + 1845*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1850*second) + ' since array.time is ' + str(array.time())

# Scan 58 = No0060
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1859*second, mjdStart+1874*second, 'No0060', obsCode, stnCode )
if array.time() < mjdStart + (1874-10)*second:
  subarray.execute(mjdStart + 1869*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1874*second) + ' since array.time is ' + str(array.time())

# Scan 59 = No0061
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1883*second, mjdStart+1899*second, 'No0061', obsCode, stnCode )
if array.time() < mjdStart + (1899-10)*second:
  subarray.execute(mjdStart + 1894*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1899*second) + ' since array.time is ' + str(array.time())

# Scan 60 = No0062
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1908*second, mjdStart+1924*second, 'No0062', obsCode, stnCode )
if array.time() < mjdStart + (1924-10)*second:
  subarray.execute(mjdStart + 1919*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1924*second) + ' since array.time is ' + str(array.time())

# Scan 61 = No0063
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1933*second, mjdStart+1949*second, 'No0063', obsCode, stnCode )
if array.time() < mjdStart + (1949-10)*second:
  subarray.execute(mjdStart + 1944*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1949*second) + ' since array.time is ' + str(array.time())

# Scan 62 = No0064
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1958*second, mjdStart+1974*second, 'No0064', obsCode, stnCode )
if array.time() < mjdStart + (1974-10)*second:
  subarray.execute(mjdStart + 1969*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1974*second) + ' since array.time is ' + str(array.time())

# Scan 63 = No0065
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1983*second, mjdStart+1999*second, 'No0065', obsCode, stnCode )
if array.time() < mjdStart + (1999-10)*second:
  subarray.execute(mjdStart + 1994*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1999*second) + ' since array.time is ' + str(array.time())

# Scan 64 = No0066
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2008*second, mjdStart+2024*second, 'No0066', obsCode, stnCode )
if array.time() < mjdStart + (2024-10)*second:
  subarray.execute(mjdStart + 2019*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2024*second) + ' since array.time is ' + str(array.time())

# Scan 65 = No0067
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2033*second, mjdStart+2049*second, 'No0067', obsCode, stnCode )
if array.time() < mjdStart + (2049-10)*second:
  subarray.execute(mjdStart + 2044*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2049*second) + ' since array.time is ' + str(array.time())

# Scan 66 = No0068
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2058*second, mjdStart+2074*second, 'No0068', obsCode, stnCode )
if array.time() < mjdStart + (2074-10)*second:
  subarray.execute(mjdStart + 2069*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2074*second) + ' since array.time is ' + str(array.time())

# Scan 67 = No0069
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2083*second, mjdStart+2099*second, 'No0069', obsCode, stnCode )
if array.time() < mjdStart + (2099-10)*second:
  subarray.execute(mjdStart + 2094*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2099*second) + ' since array.time is ' + str(array.time())

# Scan 68 = No0070
# changing to mode rdbe.1cm
subarray.setChannels(dbe0, channelSet2)
subarray.setVLBALoIfSetup(dbe0, loif2)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2105*second, mjdStart+2129*second, 'No0070', obsCode, stnCode )
if array.time() < mjdStart + (2129-10)*second:
  subarray.execute(mjdStart + 2124*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2129*second) + ' since array.time is ' + str(array.time())

# Scan 69 = No0071
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2138*second, mjdStart+2159*second, 'No0071', obsCode, stnCode )
if array.time() < mjdStart + (2159-10)*second:
  subarray.execute(mjdStart + 2154*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2159*second) + ' since array.time is ' + str(array.time())

# Scan 70 = No0072
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2168*second, mjdStart+2189*second, 'No0072', obsCode, stnCode )
if array.time() < mjdStart + (2189-10)*second:
  subarray.execute(mjdStart + 2184*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2189*second) + ' since array.time is ' + str(array.time())

# Scan 71 = No0073
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2198*second, mjdStart+2219*second, 'No0073', obsCode, stnCode )
if array.time() < mjdStart + (2219-10)*second:
  subarray.execute(mjdStart + 2214*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2219*second) + ' since array.time is ' + str(array.time())

# Scan 72 = No0074
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2228*second, mjdStart+2248*second, 'No0074', obsCode, stnCode )
if array.time() < mjdStart + (2248-10)*second:
  subarray.execute(mjdStart + 2243*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2248*second) + ' since array.time is ' + str(array.time())

# Scan 73 = No0075
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2257*second, mjdStart+2278*second, 'No0075', obsCode, stnCode )
if array.time() < mjdStart + (2278-10)*second:
  subarray.execute(mjdStart + 2273*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2278*second) + ' since array.time is ' + str(array.time())

# Scan 74 = No0076
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2287*second, mjdStart+2308*second, 'No0076', obsCode, stnCode )
if array.time() < mjdStart + (2308-10)*second:
  subarray.execute(mjdStart + 2303*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2308*second) + ' since array.time is ' + str(array.time())

# Scan 75 = No0077
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2317*second, mjdStart+2338*second, 'No0077', obsCode, stnCode )
if array.time() < mjdStart + (2338-10)*second:
  subarray.execute(mjdStart + 2333*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2338*second) + ' since array.time is ' + str(array.time())

# Scan 76 = No0078
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2347*second, mjdStart+2368*second, 'No0078', obsCode, stnCode )
if array.time() < mjdStart + (2368-10)*second:
  subarray.execute(mjdStart + 2363*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2368*second) + ' since array.time is ' + str(array.time())

# Scan 77 = No0079
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2377*second, mjdStart+2398*second, 'No0079', obsCode, stnCode )
if array.time() < mjdStart + (2398-10)*second:
  subarray.execute(mjdStart + 2393*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2398*second) + ' since array.time is ' + str(array.time())

# Scan 78 = No0080
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2407*second, mjdStart+2428*second, 'No0080', obsCode, stnCode )
if array.time() < mjdStart + (2428-10)*second:
  subarray.execute(mjdStart + 2423*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2428*second) + ' since array.time is ' + str(array.time())

# Scan 79 = No0081
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2437*second, mjdStart+2458*second, 'No0081', obsCode, stnCode )
if array.time() < mjdStart + (2458-10)*second:
  subarray.execute(mjdStart + 2453*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2458*second) + ' since array.time is ' + str(array.time())

# Scan 80 = No0082
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2467*second, mjdStart+2488*second, 'No0082', obsCode, stnCode )
if array.time() < mjdStart + (2488-10)*second:
  subarray.execute(mjdStart + 2483*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2488*second) + ' since array.time is ' + str(array.time())

# Scan 81 = No0083
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2497*second, mjdStart+2518*second, 'No0083', obsCode, stnCode )
if array.time() < mjdStart + (2518-10)*second:
  subarray.execute(mjdStart + 2513*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2518*second) + ' since array.time is ' + str(array.time())

# Scan 82 = No0084
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2527*second, mjdStart+2548*second, 'No0084', obsCode, stnCode )
if array.time() < mjdStart + (2548-10)*second:
  subarray.execute(mjdStart + 2543*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2548*second) + ' since array.time is ' + str(array.time())

# Scan 83 = No0085
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2557*second, mjdStart+2578*second, 'No0085', obsCode, stnCode )
if array.time() < mjdStart + (2578-10)*second:
  subarray.execute(mjdStart + 2573*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2578*second) + ' since array.time is ' + str(array.time())

# Scan 84 = No0086
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2587*second, mjdStart+2607*second, 'No0086', obsCode, stnCode )
if array.time() < mjdStart + (2607-10)*second:
  subarray.execute(mjdStart + 2602*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2607*second) + ' since array.time is ' + str(array.time())

# Scan 85 = No0087
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2616*second, mjdStart+2637*second, 'No0087', obsCode, stnCode )
if array.time() < mjdStart + (2637-10)*second:
  subarray.execute(mjdStart + 2632*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2637*second) + ' since array.time is ' + str(array.time())

# Scan 86 = No0088
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2646*second, mjdStart+2667*second, 'No0088', obsCode, stnCode )
if array.time() < mjdStart + (2667-10)*second:
  subarray.execute(mjdStart + 2662*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2667*second) + ' since array.time is ' + str(array.time())

# Scan 87 = No0089
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2676*second, mjdStart+2697*second, 'No0089', obsCode, stnCode )
if array.time() < mjdStart + (2697-10)*second:
  subarray.execute(mjdStart + 2692*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2697*second) + ' since array.time is ' + str(array.time())

# Scan 88 = No0090
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2706*second, mjdStart+2727*second, 'No0090', obsCode, stnCode )
if array.time() < mjdStart + (2727-10)*second:
  subarray.execute(mjdStart + 2722*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2727*second) + ' since array.time is ' + str(array.time())

# Scan 89 = No0091
# changing to mode rdbe.2cm
subarray.setChannels(dbe0, channelSet1)
subarray.setVLBALoIfSetup(dbe0, loif1)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2733*second, mjdStart+2787*second, 'No0091', obsCode, stnCode )
if array.time() < mjdStart + (2787-10)*second:
  subarray.execute(mjdStart + 2782*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2787*second) + ' since array.time is ' + str(array.time())

# Scan 90 = No0092
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2796*second, mjdStart+2847*second, 'No0092', obsCode, stnCode )
if array.time() < mjdStart + (2847-10)*second:
  subarray.execute(mjdStart + 2842*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2847*second) + ' since array.time is ' + str(array.time())

# Scan 91 = No0093
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2856*second, mjdStart+2907*second, 'No0093', obsCode, stnCode )
if array.time() < mjdStart + (2907-10)*second:
  subarray.execute(mjdStart + 2902*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2907*second) + ' since array.time is ' + str(array.time())

# Scan 92 = No0094
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2916*second, mjdStart+2966*second, 'No0094', obsCode, stnCode )
if array.time() < mjdStart + (2966-10)*second:
  subarray.execute(mjdStart + 2961*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2966*second) + ' since array.time is ' + str(array.time())

# Scan 93 = No0095
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2975*second, mjdStart+3026*second, 'No0095', obsCode, stnCode )
if array.time() < mjdStart + (3026-10)*second:
  subarray.execute(mjdStart + 3021*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3026*second) + ' since array.time is ' + str(array.time())

# Scan 94 = No0096
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3035*second, mjdStart+3086*second, 'No0096', obsCode, stnCode )
if array.time() < mjdStart + (3086-10)*second:
  subarray.execute(mjdStart + 3081*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3086*second) + ' since array.time is ' + str(array.time())

# Scan 95 = No0097
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3095*second, mjdStart+3146*second, 'No0097', obsCode, stnCode )
if array.time() < mjdStart + (3146-10)*second:
  subarray.execute(mjdStart + 3141*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3146*second) + ' since array.time is ' + str(array.time())

# Scan 96 = No0098
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3155*second, mjdStart+3206*second, 'No0098', obsCode, stnCode )
if array.time() < mjdStart + (3206-10)*second:
  subarray.execute(mjdStart + 3201*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3206*second) + ' since array.time is ' + str(array.time())

# Scan 97 = No0099
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3215*second, mjdStart+3266*second, 'No0099', obsCode, stnCode )
if array.time() < mjdStart + (3266-10)*second:
  subarray.execute(mjdStart + 3261*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3266*second) + ' since array.time is ' + str(array.time())

# Scan 98 = No0100
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3275*second, mjdStart+3325*second, 'No0100', obsCode, stnCode )
if array.time() < mjdStart + (3325-10)*second:
  subarray.execute(mjdStart + 3320*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3325*second) + ' since array.time is ' + str(array.time())

# Scan 99 = No0101
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3334*second, mjdStart+3385*second, 'No0101', obsCode, stnCode )
if array.time() < mjdStart + (3385-10)*second:
  subarray.execute(mjdStart + 3380*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3385*second) + ' since array.time is ' + str(array.time())

# Scan 100 = No0102
# changing to mode trdbe.sx
subarray.setChannels(dbe0, channelSet0)
subarray.setVLBALoIfSetup(dbe0, loif0)
subarray.set4x4Switch('1A', 1)
subarray.set4x4Switch('1B', 2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3391*second, mjdStart+3445*second, 'No0102', obsCode, stnCode )
if array.time() < mjdStart + (3445-10)*second:
  subarray.execute(mjdStart + 3440*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3445*second) + ' since array.time is ' + str(array.time())

# Scan 101 = No0103
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3454*second, mjdStart+3505*second, 'No0103', obsCode, stnCode )
if array.time() < mjdStart + (3505-10)*second:
  subarray.execute(mjdStart + 3500*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3505*second) + ' since array.time is ' + str(array.time())

# Scan 102 = No0104
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3514*second, mjdStart+3565*second, 'No0104', obsCode, stnCode )
if array.time() < mjdStart + (3565-10)*second:
  subarray.execute(mjdStart + 3560*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3565*second) + ' since array.time is ' + str(array.time())

# Scan 103 = No0105
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3574*second, mjdStart+3625*second, 'No0105', obsCode, stnCode )
if array.time() < mjdStart + (3625-10)*second:
  subarray.execute(mjdStart + 3620*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3625*second) + ' since array.time is ' + str(array.time())

# Scan 104 = No0106
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3634*second, mjdStart+3684*second, 'No0106', obsCode, stnCode )
if array.time() < mjdStart + (3684-10)*second:
  subarray.execute(mjdStart + 3679*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3684*second) + ' since array.time is ' + str(array.time())

# Scan 105 = No0107
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3693*second, mjdStart+3744*second, 'No0107', obsCode, stnCode )
if array.time() < mjdStart + (3744-10)*second:
  subarray.execute(mjdStart + 3739*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3744*second) + ' since array.time is ' + str(array.time())

# Scan 106 = No0108
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3753*second, mjdStart+3804*second, 'No0108', obsCode, stnCode )
if array.time() < mjdStart + (3804-10)*second:
  subarray.execute(mjdStart + 3799*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3804*second) + ' since array.time is ' + str(array.time())

# Scan 107 = No0109
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3813*second, mjdStart+3864*second, 'No0109', obsCode, stnCode )
if array.time() < mjdStart + (3864-10)*second:
  subarray.execute(mjdStart + 3859*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3864*second) + ' since array.time is ' + str(array.time())

# Scan 108 = No0110
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3873*second, mjdStart+3924*second, 'No0110', obsCode, stnCode )
if array.time() < mjdStart + (3924-10)*second:
  subarray.execute(mjdStart + 3919*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3924*second) + ' since array.time is ' + str(array.time())

# Scan 109 = No0111
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3933*second, mjdStart+3984*second, 'No0111', obsCode, stnCode )
if array.time() < mjdStart + (3984-10)*second:
  subarray.execute(mjdStart + 3979*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+3984*second) + ' since array.time is ' + str(array.time())

# Scan 110 = No0112
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 3993*second, mjdStart+4044*second, 'No0112', obsCode, stnCode )
if array.time() < mjdStart + (4044-10)*second:
  subarray.execute(mjdStart + 4039*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4044*second) + ' since array.time is ' + str(array.time())

# Scan 111 = No0113
subarray.setSource(source0)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4089*second, mjdStart+4163*second, 'No0113', obsCode, stnCode )
if array.time() < mjdStart + (4163-10)*second:
  subarray.execute(mjdStart + 4158*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4163*second) + ' since array.time is ' + str(array.time())

# Scan 112 = No0114
# changing to mode rdbe.1cm
subarray.setChannels(dbe0, channelSet2)
subarray.setVLBALoIfSetup(dbe0, loif2)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4169*second, mjdStart+4283*second, 'No0114', obsCode, stnCode )
if array.time() < mjdStart + (4283-10)*second:
  subarray.execute(mjdStart + 4278*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4283*second) + ' since array.time is ' + str(array.time())

# Scan 113 = No0115
# changing to mode rdbe.2cm
subarray.setChannels(dbe0, channelSet1)
subarray.setVLBALoIfSetup(dbe0, loif1)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4289*second, mjdStart+4403*second, 'No0115', obsCode, stnCode )
if array.time() < mjdStart + (4403-10)*second:
  subarray.execute(mjdStart + 4398*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4403*second) + ' since array.time is ' + str(array.time())

# Scan 114 = No0116
# changing to mode rdbe.7mm
subarray.setChannels(dbe0, channelSet3)
subarray.setVLBALoIfSetup(dbe0, loif3)
subarray.set4x4Switch('1A', 3)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4409*second, mjdStart+4522*second, 'No0116', obsCode, stnCode )
if array.time() < mjdStart + (4522-10)*second:
  subarray.execute(mjdStart + 4517*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4522*second) + ' since array.time is ' + str(array.time())

# Scan 115 = No0118
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4557*second, mjdStart+4572*second, 'No0118', obsCode, stnCode )
if array.time() < mjdStart + (4572-10)*second:
  subarray.execute(mjdStart + 4567*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4572*second) + ' since array.time is ' + str(array.time())

# Scan 116 = No0119
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4582*second, mjdStart+4597*second, 'No0119', obsCode, stnCode )
if array.time() < mjdStart + (4597-10)*second:
  subarray.execute(mjdStart + 4592*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4597*second) + ' since array.time is ' + str(array.time())

# Scan 117 = No0120
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4607*second, mjdStart+4622*second, 'No0120', obsCode, stnCode )
if array.time() < mjdStart + (4622-10)*second:
  subarray.execute(mjdStart + 4617*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4622*second) + ' since array.time is ' + str(array.time())

# Scan 118 = No0121
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4632*second, mjdStart+4647*second, 'No0121', obsCode, stnCode )
if array.time() < mjdStart + (4647-10)*second:
  subarray.execute(mjdStart + 4642*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4647*second) + ' since array.time is ' + str(array.time())

# Scan 119 = No0122
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4657*second, mjdStart+4672*second, 'No0122', obsCode, stnCode )
if array.time() < mjdStart + (4672-10)*second:
  subarray.execute(mjdStart + 4667*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4672*second) + ' since array.time is ' + str(array.time())

# Scan 120 = No0123
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4682*second, mjdStart+4697*second, 'No0123', obsCode, stnCode )
if array.time() < mjdStart + (4697-10)*second:
  subarray.execute(mjdStart + 4692*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4697*second) + ' since array.time is ' + str(array.time())

# Scan 121 = No0124
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4707*second, mjdStart+4722*second, 'No0124', obsCode, stnCode )
if array.time() < mjdStart + (4722-10)*second:
  subarray.execute(mjdStart + 4717*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4722*second) + ' since array.time is ' + str(array.time())

# Scan 122 = No0125
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4732*second, mjdStart+4747*second, 'No0125', obsCode, stnCode )
if array.time() < mjdStart + (4747-10)*second:
  subarray.execute(mjdStart + 4742*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4747*second) + ' since array.time is ' + str(array.time())

# Scan 123 = No0126
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4757*second, mjdStart+4772*second, 'No0126', obsCode, stnCode )
if array.time() < mjdStart + (4772-10)*second:
  subarray.execute(mjdStart + 4767*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4772*second) + ' since array.time is ' + str(array.time())

# Scan 124 = No0127
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4782*second, mjdStart+4796*second, 'No0127', obsCode, stnCode )
if array.time() < mjdStart + (4796-10)*second:
  subarray.execute(mjdStart + 4791*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4796*second) + ' since array.time is ' + str(array.time())

# Scan 125 = No0128
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4806*second, mjdStart+4821*second, 'No0128', obsCode, stnCode )
if array.time() < mjdStart + (4821-10)*second:
  subarray.execute(mjdStart + 4816*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4821*second) + ' since array.time is ' + str(array.time())

# Scan 126 = No0129
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4831*second, mjdStart+4846*second, 'No0129', obsCode, stnCode )
if array.time() < mjdStart + (4846-10)*second:
  subarray.execute(mjdStart + 4841*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4846*second) + ' since array.time is ' + str(array.time())

# Scan 127 = No0130
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4856*second, mjdStart+4871*second, 'No0130', obsCode, stnCode )
if array.time() < mjdStart + (4871-10)*second:
  subarray.execute(mjdStart + 4866*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4871*second) + ' since array.time is ' + str(array.time())

# Scan 128 = No0131
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4881*second, mjdStart+4896*second, 'No0131', obsCode, stnCode )
if array.time() < mjdStart + (4896-10)*second:
  subarray.execute(mjdStart + 4891*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4896*second) + ' since array.time is ' + str(array.time())

# Scan 129 = No0132
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4906*second, mjdStart+4921*second, 'No0132', obsCode, stnCode )
if array.time() < mjdStart + (4921-10)*second:
  subarray.execute(mjdStart + 4916*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4921*second) + ' since array.time is ' + str(array.time())

# Scan 130 = No0133
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4931*second, mjdStart+4946*second, 'No0133', obsCode, stnCode )
if array.time() < mjdStart + (4946-10)*second:
  subarray.execute(mjdStart + 4941*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4946*second) + ' since array.time is ' + str(array.time())

# Scan 131 = No0134
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4956*second, mjdStart+4971*second, 'No0134', obsCode, stnCode )
if array.time() < mjdStart + (4971-10)*second:
  subarray.execute(mjdStart + 4966*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4971*second) + ' since array.time is ' + str(array.time())

# Scan 132 = No0135
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 4981*second, mjdStart+4996*second, 'No0135', obsCode, stnCode )
if array.time() < mjdStart + (4996-10)*second:
  subarray.execute(mjdStart + 4991*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+4996*second) + ' since array.time is ' + str(array.time())

# Scan 133 = No0136
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5006*second, mjdStart+5021*second, 'No0136', obsCode, stnCode )
if array.time() < mjdStart + (5021-10)*second:
  subarray.execute(mjdStart + 5016*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5021*second) + ' since array.time is ' + str(array.time())

# Scan 134 = No0137
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5031*second, mjdStart+5046*second, 'No0137', obsCode, stnCode )
if array.time() < mjdStart + (5046-10)*second:
  subarray.execute(mjdStart + 5041*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5046*second) + ' since array.time is ' + str(array.time())

# Scan 135 = No0138
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5056*second, mjdStart+5071*second, 'No0138', obsCode, stnCode )
if array.time() < mjdStart + (5071-10)*second:
  subarray.execute(mjdStart + 5066*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5071*second) + ' since array.time is ' + str(array.time())

# Scan 136 = No0139
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5081*second, mjdStart+5096*second, 'No0139', obsCode, stnCode )
if array.time() < mjdStart + (5096-10)*second:
  subarray.execute(mjdStart + 5091*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5096*second) + ' since array.time is ' + str(array.time())

# Scan 137 = No0140
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5106*second, mjdStart+5121*second, 'No0140', obsCode, stnCode )
if array.time() < mjdStart + (5121-10)*second:
  subarray.execute(mjdStart + 5116*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5121*second) + ' since array.time is ' + str(array.time())

# Scan 138 = No0141
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5131*second, mjdStart+5145*second, 'No0141', obsCode, stnCode )
if array.time() < mjdStart + (5145-10)*second:
  subarray.execute(mjdStart + 5140*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5145*second) + ' since array.time is ' + str(array.time())

# Scan 139 = No0142
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5155*second, mjdStart+5170*second, 'No0142', obsCode, stnCode )
if array.time() < mjdStart + (5170-10)*second:
  subarray.execute(mjdStart + 5165*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5170*second) + ' since array.time is ' + str(array.time())

# Scan 140 = No0143
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5180*second, mjdStart+5195*second, 'No0143', obsCode, stnCode )
if array.time() < mjdStart + (5195-10)*second:
  subarray.execute(mjdStart + 5190*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5195*second) + ' since array.time is ' + str(array.time())

# Scan 141 = No0144
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5205*second, mjdStart+5220*second, 'No0144', obsCode, stnCode )
if array.time() < mjdStart + (5220-10)*second:
  subarray.execute(mjdStart + 5215*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5220*second) + ' since array.time is ' + str(array.time())

# Scan 142 = No0145
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5230*second, mjdStart+5245*second, 'No0145', obsCode, stnCode )
if array.time() < mjdStart + (5245-10)*second:
  subarray.execute(mjdStart + 5240*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5245*second) + ' since array.time is ' + str(array.time())

# Scan 143 = No0146
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5255*second, mjdStart+5270*second, 'No0146', obsCode, stnCode )
if array.time() < mjdStart + (5270-10)*second:
  subarray.execute(mjdStart + 5265*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5270*second) + ' since array.time is ' + str(array.time())

# Scan 144 = No0147
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5280*second, mjdStart+5295*second, 'No0147', obsCode, stnCode )
if array.time() < mjdStart + (5295-10)*second:
  subarray.execute(mjdStart + 5290*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5295*second) + ' since array.time is ' + str(array.time())

# Scan 145 = No0148
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5305*second, mjdStart+5320*second, 'No0148', obsCode, stnCode )
if array.time() < mjdStart + (5320-10)*second:
  subarray.execute(mjdStart + 5315*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5320*second) + ' since array.time is ' + str(array.time())

# Scan 146 = No0149
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5330*second, mjdStart+5345*second, 'No0149', obsCode, stnCode )
if array.time() < mjdStart + (5345-10)*second:
  subarray.execute(mjdStart + 5340*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5345*second) + ' since array.time is ' + str(array.time())

# Scan 147 = No0150
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5355*second, mjdStart+5370*second, 'No0150', obsCode, stnCode )
if array.time() < mjdStart + (5370-10)*second:
  subarray.execute(mjdStart + 5365*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5370*second) + ' since array.time is ' + str(array.time())

# Scan 148 = No0151
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5380*second, mjdStart+5395*second, 'No0151', obsCode, stnCode )
if array.time() < mjdStart + (5395-10)*second:
  subarray.execute(mjdStart + 5390*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5395*second) + ' since array.time is ' + str(array.time())

# Scan 149 = No0152
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5405*second, mjdStart+5420*second, 'No0152', obsCode, stnCode )
if array.time() < mjdStart + (5420-10)*second:
  subarray.execute(mjdStart + 5415*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5420*second) + ' since array.time is ' + str(array.time())

# Scan 150 = No0153
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5430*second, mjdStart+5445*second, 'No0153', obsCode, stnCode )
if array.time() < mjdStart + (5445-10)*second:
  subarray.execute(mjdStart + 5440*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5445*second) + ' since array.time is ' + str(array.time())

# Scan 151 = No0154
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5455*second, mjdStart+5470*second, 'No0154', obsCode, stnCode )
if array.time() < mjdStart + (5470-10)*second:
  subarray.execute(mjdStart + 5465*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5470*second) + ' since array.time is ' + str(array.time())

# Scan 152 = No0155
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5480*second, mjdStart+5495*second, 'No0155', obsCode, stnCode )
if array.time() < mjdStart + (5495-10)*second:
  subarray.execute(mjdStart + 5490*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5495*second) + ' since array.time is ' + str(array.time())

# Scan 153 = No0156
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5505*second, mjdStart+5519*second, 'No0156', obsCode, stnCode )
if array.time() < mjdStart + (5519-10)*second:
  subarray.execute(mjdStart + 5514*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5519*second) + ' since array.time is ' + str(array.time())

# Scan 154 = No0157
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5529*second, mjdStart+5544*second, 'No0157', obsCode, stnCode )
if array.time() < mjdStart + (5544-10)*second:
  subarray.execute(mjdStart + 5539*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5544*second) + ' since array.time is ' + str(array.time())

# Scan 155 = No0158
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5554*second, mjdStart+5569*second, 'No0158', obsCode, stnCode )
if array.time() < mjdStart + (5569-10)*second:
  subarray.execute(mjdStart + 5564*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5569*second) + ' since array.time is ' + str(array.time())

# Scan 156 = No0159
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5579*second, mjdStart+5594*second, 'No0159', obsCode, stnCode )
if array.time() < mjdStart + (5594-10)*second:
  subarray.execute(mjdStart + 5589*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5594*second) + ' since array.time is ' + str(array.time())

# Scan 157 = No0160
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5604*second, mjdStart+5619*second, 'No0160', obsCode, stnCode )
if array.time() < mjdStart + (5619-10)*second:
  subarray.execute(mjdStart + 5614*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5619*second) + ' since array.time is ' + str(array.time())

# Scan 158 = No0161
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5629*second, mjdStart+5644*second, 'No0161', obsCode, stnCode )
if array.time() < mjdStart + (5644-10)*second:
  subarray.execute(mjdStart + 5639*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5644*second) + ' since array.time is ' + str(array.time())

# Scan 159 = No0162
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5654*second, mjdStart+5669*second, 'No0162', obsCode, stnCode )
if array.time() < mjdStart + (5669-10)*second:
  subarray.execute(mjdStart + 5664*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5669*second) + ' since array.time is ' + str(array.time())

# Scan 160 = No0163
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5679*second, mjdStart+5694*second, 'No0163', obsCode, stnCode )
if array.time() < mjdStart + (5694-10)*second:
  subarray.execute(mjdStart + 5689*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5694*second) + ' since array.time is ' + str(array.time())

# Scan 161 = No0164
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5704*second, mjdStart+5719*second, 'No0164', obsCode, stnCode )
if array.time() < mjdStart + (5719-10)*second:
  subarray.execute(mjdStart + 5714*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5719*second) + ' since array.time is ' + str(array.time())

# Scan 162 = No0165
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5729*second, mjdStart+5744*second, 'No0165', obsCode, stnCode )
if array.time() < mjdStart + (5744-10)*second:
  subarray.execute(mjdStart + 5739*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5744*second) + ' since array.time is ' + str(array.time())

# Scan 163 = No0166
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5754*second, mjdStart+5769*second, 'No0166', obsCode, stnCode )
if array.time() < mjdStart + (5769-10)*second:
  subarray.execute(mjdStart + 5764*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5769*second) + ' since array.time is ' + str(array.time())

# Scan 164 = No0167
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5779*second, mjdStart+5794*second, 'No0167', obsCode, stnCode )
if array.time() < mjdStart + (5794-10)*second:
  subarray.execute(mjdStart + 5789*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5794*second) + ' since array.time is ' + str(array.time())

# Scan 165 = No0168
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5804*second, mjdStart+5819*second, 'No0168', obsCode, stnCode )
if array.time() < mjdStart + (5819-10)*second:
  subarray.execute(mjdStart + 5814*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5819*second) + ' since array.time is ' + str(array.time())

# Scan 166 = No0169
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5829*second, mjdStart+5844*second, 'No0169', obsCode, stnCode )
if array.time() < mjdStart + (5844-10)*second:
  subarray.execute(mjdStart + 5839*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5844*second) + ' since array.time is ' + str(array.time())

# Scan 167 = No0170
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5854*second, mjdStart+5869*second, 'No0170', obsCode, stnCode )
if array.time() < mjdStart + (5869-10)*second:
  subarray.execute(mjdStart + 5864*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5869*second) + ' since array.time is ' + str(array.time())

# Scan 168 = No0171
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5879*second, mjdStart+5893*second, 'No0171', obsCode, stnCode )
if array.time() < mjdStart + (5893-10)*second:
  subarray.execute(mjdStart + 5888*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5893*second) + ' since array.time is ' + str(array.time())

# Scan 169 = No0172
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5903*second, mjdStart+5918*second, 'No0172', obsCode, stnCode )
if array.time() < mjdStart + (5918-10)*second:
  subarray.execute(mjdStart + 5913*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5918*second) + ' since array.time is ' + str(array.time())

# Scan 170 = No0173
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5928*second, mjdStart+5943*second, 'No0173', obsCode, stnCode )
if array.time() < mjdStart + (5943-10)*second:
  subarray.execute(mjdStart + 5938*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5943*second) + ' since array.time is ' + str(array.time())

# Scan 171 = No0174
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5953*second, mjdStart+5968*second, 'No0174', obsCode, stnCode )
if array.time() < mjdStart + (5968-10)*second:
  subarray.execute(mjdStart + 5963*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5968*second) + ' since array.time is ' + str(array.time())

# Scan 172 = No0175
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 5978*second, mjdStart+5993*second, 'No0175', obsCode, stnCode )
if array.time() < mjdStart + (5993-10)*second:
  subarray.execute(mjdStart + 5988*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+5993*second) + ' since array.time is ' + str(array.time())

# Scan 173 = No0176
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6003*second, mjdStart+6018*second, 'No0176', obsCode, stnCode )
if array.time() < mjdStart + (6018-10)*second:
  subarray.execute(mjdStart + 6013*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6018*second) + ' since array.time is ' + str(array.time())

# Scan 174 = No0177
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6028*second, mjdStart+6043*second, 'No0177', obsCode, stnCode )
if array.time() < mjdStart + (6043-10)*second:
  subarray.execute(mjdStart + 6038*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6043*second) + ' since array.time is ' + str(array.time())

# Scan 175 = No0178
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6053*second, mjdStart+6068*second, 'No0178', obsCode, stnCode )
if array.time() < mjdStart + (6068-10)*second:
  subarray.execute(mjdStart + 6063*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6068*second) + ' since array.time is ' + str(array.time())

# Scan 176 = No0179
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6078*second, mjdStart+6093*second, 'No0179', obsCode, stnCode )
if array.time() < mjdStart + (6093-10)*second:
  subarray.execute(mjdStart + 6088*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6093*second) + ' since array.time is ' + str(array.time())

# Scan 177 = No0180
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6103*second, mjdStart+6118*second, 'No0180', obsCode, stnCode )
if array.time() < mjdStart + (6118-10)*second:
  subarray.execute(mjdStart + 6113*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6118*second) + ' since array.time is ' + str(array.time())

# Scan 178 = No0181
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6128*second, mjdStart+6143*second, 'No0181', obsCode, stnCode )
if array.time() < mjdStart + (6143-10)*second:
  subarray.execute(mjdStart + 6138*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6143*second) + ' since array.time is ' + str(array.time())

# Scan 179 = No0182
# changing to mode rdbe.1cm
subarray.setChannels(dbe0, channelSet2)
subarray.setVLBALoIfSetup(dbe0, loif2)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6149*second, mjdStart+6173*second, 'No0182', obsCode, stnCode )
if array.time() < mjdStart + (6173-10)*second:
  subarray.execute(mjdStart + 6168*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6173*second) + ' since array.time is ' + str(array.time())

# Scan 180 = No0183
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6183*second, mjdStart+6203*second, 'No0183', obsCode, stnCode )
if array.time() < mjdStart + (6203-10)*second:
  subarray.execute(mjdStart + 6198*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6203*second) + ' since array.time is ' + str(array.time())

# Scan 181 = No0184
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6213*second, mjdStart+6233*second, 'No0184', obsCode, stnCode )
if array.time() < mjdStart + (6233-10)*second:
  subarray.execute(mjdStart + 6228*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6233*second) + ' since array.time is ' + str(array.time())

# Scan 182 = No0185
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6243*second, mjdStart+6262*second, 'No0185', obsCode, stnCode )
if array.time() < mjdStart + (6262-10)*second:
  subarray.execute(mjdStart + 6257*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6262*second) + ' since array.time is ' + str(array.time())

# Scan 183 = No0186
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6273*second, mjdStart+6292*second, 'No0186', obsCode, stnCode )
if array.time() < mjdStart + (6292-10)*second:
  subarray.execute(mjdStart + 6287*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6292*second) + ' since array.time is ' + str(array.time())

# Scan 184 = No0187
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6303*second, mjdStart+6322*second, 'No0187', obsCode, stnCode )
if array.time() < mjdStart + (6322-10)*second:
  subarray.execute(mjdStart + 6317*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6322*second) + ' since array.time is ' + str(array.time())

# Scan 185 = No0188
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6333*second, mjdStart+6352*second, 'No0188', obsCode, stnCode )
if array.time() < mjdStart + (6352-10)*second:
  subarray.execute(mjdStart + 6347*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6352*second) + ' since array.time is ' + str(array.time())

# Scan 186 = No0189
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6363*second, mjdStart+6382*second, 'No0189', obsCode, stnCode )
if array.time() < mjdStart + (6382-10)*second:
  subarray.execute(mjdStart + 6377*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6382*second) + ' since array.time is ' + str(array.time())

# Scan 187 = No0190
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6393*second, mjdStart+6412*second, 'No0190', obsCode, stnCode )
if array.time() < mjdStart + (6412-10)*second:
  subarray.execute(mjdStart + 6407*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6412*second) + ' since array.time is ' + str(array.time())

# Scan 188 = No0191
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6423*second, mjdStart+6442*second, 'No0191', obsCode, stnCode )
if array.time() < mjdStart + (6442-10)*second:
  subarray.execute(mjdStart + 6437*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6442*second) + ' since array.time is ' + str(array.time())

# Scan 189 = No0192
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6453*second, mjdStart+6472*second, 'No0192', obsCode, stnCode )
if array.time() < mjdStart + (6472-10)*second:
  subarray.execute(mjdStart + 6467*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6472*second) + ' since array.time is ' + str(array.time())

# Scan 190 = No0193
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6483*second, mjdStart+6502*second, 'No0193', obsCode, stnCode )
if array.time() < mjdStart + (6502-10)*second:
  subarray.execute(mjdStart + 6497*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6502*second) + ' since array.time is ' + str(array.time())

# Scan 191 = No0194
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6513*second, mjdStart+6532*second, 'No0194', obsCode, stnCode )
if array.time() < mjdStart + (6532-10)*second:
  subarray.execute(mjdStart + 6527*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6532*second) + ' since array.time is ' + str(array.time())

# Scan 192 = No0195
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6543*second, mjdStart+6562*second, 'No0195', obsCode, stnCode )
if array.time() < mjdStart + (6562-10)*second:
  subarray.execute(mjdStart + 6557*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6562*second) + ' since array.time is ' + str(array.time())

# Scan 193 = No0196
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6573*second, mjdStart+6592*second, 'No0196', obsCode, stnCode )
if array.time() < mjdStart + (6592-10)*second:
  subarray.execute(mjdStart + 6587*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6592*second) + ' since array.time is ' + str(array.time())

# Scan 194 = No0197
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6603*second, mjdStart+6621*second, 'No0197', obsCode, stnCode )
if array.time() < mjdStart + (6621-10)*second:
  subarray.execute(mjdStart + 6616*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6621*second) + ' since array.time is ' + str(array.time())

# Scan 195 = No0198
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6632*second, mjdStart+6651*second, 'No0198', obsCode, stnCode )
if array.time() < mjdStart + (6651-10)*second:
  subarray.execute(mjdStart + 6646*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6651*second) + ' since array.time is ' + str(array.time())

# Scan 196 = No0199
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6662*second, mjdStart+6681*second, 'No0199', obsCode, stnCode )
if array.time() < mjdStart + (6681-10)*second:
  subarray.execute(mjdStart + 6676*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6681*second) + ' since array.time is ' + str(array.time())

# Scan 197 = No0200
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6692*second, mjdStart+6711*second, 'No0200', obsCode, stnCode )
if array.time() < mjdStart + (6711-10)*second:
  subarray.execute(mjdStart + 6706*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6711*second) + ' since array.time is ' + str(array.time())

# Scan 198 = No0201
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6722*second, mjdStart+6741*second, 'No0201', obsCode, stnCode )
if array.time() < mjdStart + (6741-10)*second:
  subarray.execute(mjdStart + 6736*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6741*second) + ' since array.time is ' + str(array.time())

# Scan 199 = No0202
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6752*second, mjdStart+6771*second, 'No0202', obsCode, stnCode )
if array.time() < mjdStart + (6771-10)*second:
  subarray.execute(mjdStart + 6766*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6771*second) + ' since array.time is ' + str(array.time())

# Scan 200 = No0203
# changing to mode rdbe.2cm
subarray.setChannels(dbe0, channelSet1)
subarray.setVLBALoIfSetup(dbe0, loif1)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6777*second, mjdStart+6831*second, 'No0203', obsCode, stnCode )
if array.time() < mjdStart + (6831-10)*second:
  subarray.execute(mjdStart + 6826*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6831*second) + ' since array.time is ' + str(array.time())

# Scan 201 = No0204
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6842*second, mjdStart+6891*second, 'No0204', obsCode, stnCode )
if array.time() < mjdStart + (6891-10)*second:
  subarray.execute(mjdStart + 6886*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6891*second) + ' since array.time is ' + str(array.time())

# Scan 202 = No0205
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6902*second, mjdStart+6951*second, 'No0205', obsCode, stnCode )
if array.time() < mjdStart + (6951-10)*second:
  subarray.execute(mjdStart + 6946*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+6951*second) + ' since array.time is ' + str(array.time())

# Scan 203 = No0206
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 6962*second, mjdStart+7010*second, 'No0206', obsCode, stnCode )
if array.time() < mjdStart + (7010-10)*second:
  subarray.execute(mjdStart + 7005*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7010*second) + ' since array.time is ' + str(array.time())

# Scan 204 = No0207
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7021*second, mjdStart+7070*second, 'No0207', obsCode, stnCode )
if array.time() < mjdStart + (7070-10)*second:
  subarray.execute(mjdStart + 7065*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7070*second) + ' since array.time is ' + str(array.time())

# Scan 205 = No0208
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7081*second, mjdStart+7130*second, 'No0208', obsCode, stnCode )
if array.time() < mjdStart + (7130-10)*second:
  subarray.execute(mjdStart + 7125*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7130*second) + ' since array.time is ' + str(array.time())

# Scan 206 = No0209
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7141*second, mjdStart+7190*second, 'No0209', obsCode, stnCode )
if array.time() < mjdStart + (7190-10)*second:
  subarray.execute(mjdStart + 7185*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7190*second) + ' since array.time is ' + str(array.time())

# Scan 207 = No0210
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7201*second, mjdStart+7250*second, 'No0210', obsCode, stnCode )
if array.time() < mjdStart + (7250-10)*second:
  subarray.execute(mjdStart + 7245*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7250*second) + ' since array.time is ' + str(array.time())

# Scan 208 = No0211
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7261*second, mjdStart+7310*second, 'No0211', obsCode, stnCode )
if array.time() < mjdStart + (7310-10)*second:
  subarray.execute(mjdStart + 7305*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7310*second) + ' since array.time is ' + str(array.time())

# Scan 209 = No0212
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7321*second, mjdStart+7369*second, 'No0212', obsCode, stnCode )
if array.time() < mjdStart + (7369-10)*second:
  subarray.execute(mjdStart + 7364*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7369*second) + ' since array.time is ' + str(array.time())

# Scan 210 = No0213
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7381*second, mjdStart+7429*second, 'No0213', obsCode, stnCode )
if array.time() < mjdStart + (7429-10)*second:
  subarray.execute(mjdStart + 7424*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7429*second) + ' since array.time is ' + str(array.time())

# Scan 211 = No0214
# changing to mode trdbe.sx
subarray.setChannels(dbe0, channelSet0)
subarray.setVLBALoIfSetup(dbe0, loif0)
subarray.set4x4Switch('1A', 1)
subarray.set4x4Switch('1B', 2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7435*second, mjdStart+7489*second, 'No0214', obsCode, stnCode )
if array.time() < mjdStart + (7489-10)*second:
  subarray.execute(mjdStart + 7484*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7489*second) + ' since array.time is ' + str(array.time())

# Scan 212 = No0215
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7501*second, mjdStart+7549*second, 'No0215', obsCode, stnCode )
if array.time() < mjdStart + (7549-10)*second:
  subarray.execute(mjdStart + 7544*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7549*second) + ' since array.time is ' + str(array.time())

# Scan 213 = No0216
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7561*second, mjdStart+7609*second, 'No0216', obsCode, stnCode )
if array.time() < mjdStart + (7609-10)*second:
  subarray.execute(mjdStart + 7604*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7609*second) + ' since array.time is ' + str(array.time())

# Scan 214 = No0217
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7621*second, mjdStart+7669*second, 'No0217', obsCode, stnCode )
if array.time() < mjdStart + (7669-10)*second:
  subarray.execute(mjdStart + 7664*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7669*second) + ' since array.time is ' + str(array.time())

# Scan 215 = No0218
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7681*second, mjdStart+7728*second, 'No0218', obsCode, stnCode )
if array.time() < mjdStart + (7728-10)*second:
  subarray.execute(mjdStart + 7723*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7728*second) + ' since array.time is ' + str(array.time())

# Scan 216 = No0219
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7740*second, mjdStart+7788*second, 'No0219', obsCode, stnCode )
if array.time() < mjdStart + (7788-10)*second:
  subarray.execute(mjdStart + 7783*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7788*second) + ' since array.time is ' + str(array.time())

# Scan 217 = No0220
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7800*second, mjdStart+7848*second, 'No0220', obsCode, stnCode )
if array.time() < mjdStart + (7848-10)*second:
  subarray.execute(mjdStart + 7843*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7848*second) + ' since array.time is ' + str(array.time())

# Scan 218 = No0221
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7860*second, mjdStart+7908*second, 'No0221', obsCode, stnCode )
if array.time() < mjdStart + (7908-10)*second:
  subarray.execute(mjdStart + 7903*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7908*second) + ' since array.time is ' + str(array.time())

# Scan 219 = No0222
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7920*second, mjdStart+7968*second, 'No0222', obsCode, stnCode )
if array.time() < mjdStart + (7968-10)*second:
  subarray.execute(mjdStart + 7963*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7968*second) + ' since array.time is ' + str(array.time())

# Scan 220 = No0223
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7980*second, mjdStart+8028*second, 'No0223', obsCode, stnCode )
if array.time() < mjdStart + (8028-10)*second:
  subarray.execute(mjdStart + 8023*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8028*second) + ' since array.time is ' + str(array.time())

# Scan 221 = No0224
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8041*second, mjdStart+8087*second, 'No0224', obsCode, stnCode )
if array.time() < mjdStart + (8087-10)*second:
  subarray.execute(mjdStart + 8082*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8087*second) + ' since array.time is ' + str(array.time())

# Scan 222 = No0225
subarray.setSource(source0)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8198*second, mjdStart+8207*second, 'No0225', obsCode, stnCode )
if array.time() < mjdStart + (8207-10)*second:
  subarray.execute(mjdStart + 8202*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8207*second) + ' since array.time is ' + str(array.time())

# Scan 223 = No0226
# changing to mode rdbe.1cm
subarray.setChannels(dbe0, channelSet2)
subarray.setVLBALoIfSetup(dbe0, loif2)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8213*second, mjdStart+8327*second, 'No0226', obsCode, stnCode )
if array.time() < mjdStart + (8327-10)*second:
  subarray.execute(mjdStart + 8322*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8327*second) + ' since array.time is ' + str(array.time())

# Scan 224 = No0227
# changing to mode rdbe.2cm
subarray.setChannels(dbe0, channelSet1)
subarray.setVLBALoIfSetup(dbe0, loif1)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8333*second, mjdStart+8446*second, 'No0227', obsCode, stnCode )
if array.time() < mjdStart + (8446-10)*second:
  subarray.execute(mjdStart + 8441*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8446*second) + ' since array.time is ' + str(array.time())

# Scan 225 = No0228
# changing to mode rdbe.7mm
subarray.setChannels(dbe0, channelSet3)
subarray.setVLBALoIfSetup(dbe0, loif3)
subarray.set4x4Switch('1A', 3)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8452*second, mjdStart+8566*second, 'No0228', obsCode, stnCode )
if array.time() < mjdStart + (8566-10)*second:
  subarray.execute(mjdStart + 8561*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8566*second) + ' since array.time is ' + str(array.time())

# Scan 226 = No0230
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8605*second, mjdStart+8616*second, 'No0230', obsCode, stnCode )
if array.time() < mjdStart + (8616-10)*second:
  subarray.execute(mjdStart + 8611*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8616*second) + ' since array.time is ' + str(array.time())

# Scan 227 = No0231
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8630*second, mjdStart+8641*second, 'No0231', obsCode, stnCode )
if array.time() < mjdStart + (8641-10)*second:
  subarray.execute(mjdStart + 8636*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8641*second) + ' since array.time is ' + str(array.time())

# Scan 228 = No0232
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8655*second, mjdStart+8666*second, 'No0232', obsCode, stnCode )
if array.time() < mjdStart + (8666-10)*second:
  subarray.execute(mjdStart + 8661*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8666*second) + ' since array.time is ' + str(array.time())

# Scan 229 = No0233
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8680*second, mjdStart+8691*second, 'No0233', obsCode, stnCode )
if array.time() < mjdStart + (8691-10)*second:
  subarray.execute(mjdStart + 8686*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8691*second) + ' since array.time is ' + str(array.time())

# Scan 230 = No0234
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8705*second, mjdStart+8716*second, 'No0234', obsCode, stnCode )
if array.time() < mjdStart + (8716-10)*second:
  subarray.execute(mjdStart + 8711*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8716*second) + ' since array.time is ' + str(array.time())

# Scan 231 = No0235
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8730*second, mjdStart+8741*second, 'No0235', obsCode, stnCode )
if array.time() < mjdStart + (8741-10)*second:
  subarray.execute(mjdStart + 8736*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8741*second) + ' since array.time is ' + str(array.time())

# Scan 232 = No0236
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8755*second, mjdStart+8766*second, 'No0236', obsCode, stnCode )
if array.time() < mjdStart + (8766-10)*second:
  subarray.execute(mjdStart + 8761*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8766*second) + ' since array.time is ' + str(array.time())

# Scan 233 = No0237
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8780*second, mjdStart+8791*second, 'No0237', obsCode, stnCode )
if array.time() < mjdStart + (8791-10)*second:
  subarray.execute(mjdStart + 8786*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8791*second) + ' since array.time is ' + str(array.time())

# Scan 234 = No0238
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8806*second, mjdStart+8815*second, 'No0238', obsCode, stnCode )
if array.time() < mjdStart + (8815-10)*second:
  subarray.execute(mjdStart + 8810*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8815*second) + ' since array.time is ' + str(array.time())

# Scan 235 = No0239
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8829*second, mjdStart+8840*second, 'No0239', obsCode, stnCode )
if array.time() < mjdStart + (8840-10)*second:
  subarray.execute(mjdStart + 8835*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8840*second) + ' since array.time is ' + str(array.time())

# Scan 236 = No0240
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8855*second, mjdStart+8865*second, 'No0240', obsCode, stnCode )
if array.time() < mjdStart + (8865-10)*second:
  subarray.execute(mjdStart + 8860*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8865*second) + ' since array.time is ' + str(array.time())

# Scan 237 = No0241
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8880*second, mjdStart+8890*second, 'No0241', obsCode, stnCode )
if array.time() < mjdStart + (8890-10)*second:
  subarray.execute(mjdStart + 8885*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8890*second) + ' since array.time is ' + str(array.time())

# Scan 238 = No0242
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8905*second, mjdStart+8915*second, 'No0242', obsCode, stnCode )
if array.time() < mjdStart + (8915-10)*second:
  subarray.execute(mjdStart + 8910*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8915*second) + ' since array.time is ' + str(array.time())

# Scan 239 = No0243
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8930*second, mjdStart+8940*second, 'No0243', obsCode, stnCode )
if array.time() < mjdStart + (8940-10)*second:
  subarray.execute(mjdStart + 8935*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8940*second) + ' since array.time is ' + str(array.time())

# Scan 240 = No0244
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8955*second, mjdStart+8965*second, 'No0244', obsCode, stnCode )
if array.time() < mjdStart + (8965-10)*second:
  subarray.execute(mjdStart + 8960*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8965*second) + ' since array.time is ' + str(array.time())

# Scan 241 = No0245
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8980*second, mjdStart+8990*second, 'No0245', obsCode, stnCode )
if array.time() < mjdStart + (8990-10)*second:
  subarray.execute(mjdStart + 8985*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8990*second) + ' since array.time is ' + str(array.time())

# Scan 242 = No0246
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9005*second, mjdStart+9015*second, 'No0246', obsCode, stnCode )
if array.time() < mjdStart + (9015-10)*second:
  subarray.execute(mjdStart + 9010*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9015*second) + ' since array.time is ' + str(array.time())

# Scan 243 = No0247
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9030*second, mjdStart+9040*second, 'No0247', obsCode, stnCode )
if array.time() < mjdStart + (9040-10)*second:
  subarray.execute(mjdStart + 9035*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9040*second) + ' since array.time is ' + str(array.time())

# Scan 244 = No0248
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9055*second, mjdStart+9065*second, 'No0248', obsCode, stnCode )
if array.time() < mjdStart + (9065-10)*second:
  subarray.execute(mjdStart + 9060*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9065*second) + ' since array.time is ' + str(array.time())

# Scan 245 = No0249
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9080*second, mjdStart+9090*second, 'No0249', obsCode, stnCode )
if array.time() < mjdStart + (9090-10)*second:
  subarray.execute(mjdStart + 9085*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9090*second) + ' since array.time is ' + str(array.time())

# Scan 246 = No0250
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9106*second, mjdStart+9115*second, 'No0250', obsCode, stnCode )
if array.time() < mjdStart + (9115-10)*second:
  subarray.execute(mjdStart + 9110*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9115*second) + ' since array.time is ' + str(array.time())

# Scan 247 = No0251
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9131*second, mjdStart+9140*second, 'No0251', obsCode, stnCode )
if array.time() < mjdStart + (9140-10)*second:
  subarray.execute(mjdStart + 9135*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9140*second) + ' since array.time is ' + str(array.time())

# Scan 248 = No0252
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9156*second, mjdStart+9164*second, 'No0252', obsCode, stnCode )
if array.time() < mjdStart + (9164-10)*second:
  subarray.execute(mjdStart + 9159*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9164*second) + ' since array.time is ' + str(array.time())

# Scan 249 = No0253
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9180*second, mjdStart+9189*second, 'No0253', obsCode, stnCode )
if array.time() < mjdStart + (9189-10)*second:
  subarray.execute(mjdStart + 9184*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9189*second) + ' since array.time is ' + str(array.time())

# Scan 250 = No0254
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9205*second, mjdStart+9214*second, 'No0254', obsCode, stnCode )
if array.time() < mjdStart + (9214-10)*second:
  subarray.execute(mjdStart + 9209*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9214*second) + ' since array.time is ' + str(array.time())

# Scan 251 = No0255
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9230*second, mjdStart+9239*second, 'No0255', obsCode, stnCode )
if array.time() < mjdStart + (9239-10)*second:
  subarray.execute(mjdStart + 9234*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9239*second) + ' since array.time is ' + str(array.time())

# Scan 252 = No0256
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9255*second, mjdStart+9264*second, 'No0256', obsCode, stnCode )
if array.time() < mjdStart + (9264-10)*second:
  subarray.execute(mjdStart + 9259*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9264*second) + ' since array.time is ' + str(array.time())

# Scan 253 = No0257
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9280*second, mjdStart+9289*second, 'No0257', obsCode, stnCode )
if array.time() < mjdStart + (9289-10)*second:
  subarray.execute(mjdStart + 9284*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9289*second) + ' since array.time is ' + str(array.time())

# Scan 254 = No0258
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9306*second, mjdStart+9314*second, 'No0258', obsCode, stnCode )
if array.time() < mjdStart + (9314-10)*second:
  subarray.execute(mjdStart + 9309*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9314*second) + ' since array.time is ' + str(array.time())

# Scan 255 = No0259
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9330*second, mjdStart+9339*second, 'No0259', obsCode, stnCode )
if array.time() < mjdStart + (9339-10)*second:
  subarray.execute(mjdStart + 9334*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9339*second) + ' since array.time is ' + str(array.time())

# Scan 256 = No0260
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9356*second, mjdStart+9364*second, 'No0260', obsCode, stnCode )
if array.time() < mjdStart + (9364-10)*second:
  subarray.execute(mjdStart + 9359*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9364*second) + ' since array.time is ' + str(array.time())

# Scan 257 = No0261
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9381*second, mjdStart+9389*second, 'No0261', obsCode, stnCode )
if array.time() < mjdStart + (9389-10)*second:
  subarray.execute(mjdStart + 9384*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9389*second) + ' since array.time is ' + str(array.time())

# Scan 258 = No0262
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9406*second, mjdStart+9414*second, 'No0262', obsCode, stnCode )
if array.time() < mjdStart + (9414-10)*second:
  subarray.execute(mjdStart + 9409*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9414*second) + ' since array.time is ' + str(array.time())

# Scan 259 = No0263
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9431*second, mjdStart+9439*second, 'No0263', obsCode, stnCode )
if array.time() < mjdStart + (9439-10)*second:
  subarray.execute(mjdStart + 9434*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9439*second) + ' since array.time is ' + str(array.time())

# Scan 260 = No0264
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9456*second, mjdStart+9464*second, 'No0264', obsCode, stnCode )
if array.time() < mjdStart + (9464-10)*second:
  subarray.execute(mjdStart + 9459*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9464*second) + ' since array.time is ' + str(array.time())

# Scan 261 = No0265
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9481*second, mjdStart+9489*second, 'No0265', obsCode, stnCode )
if array.time() < mjdStart + (9489-10)*second:
  subarray.execute(mjdStart + 9484*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9489*second) + ' since array.time is ' + str(array.time())

# Scan 262 = No0266
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9507*second, mjdStart+9514*second, 'No0266', obsCode, stnCode )
if array.time() < mjdStart + (9514-10)*second:
  subarray.execute(mjdStart + 9509*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9514*second) + ' since array.time is ' + str(array.time())

# Scan 263 = No0267
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9531*second, mjdStart+9538*second, 'No0267', obsCode, stnCode )
if array.time() < mjdStart + (9538-10)*second:
  subarray.execute(mjdStart + 9533*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9538*second) + ' since array.time is ' + str(array.time())

# Scan 264 = No0268
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9556*second, mjdStart+9563*second, 'No0268', obsCode, stnCode )
if array.time() < mjdStart + (9563-10)*second:
  subarray.execute(mjdStart + 9558*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9563*second) + ' since array.time is ' + str(array.time())

# Scan 265 = No0269
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9581*second, mjdStart+9588*second, 'No0269', obsCode, stnCode )
if array.time() < mjdStart + (9588-10)*second:
  subarray.execute(mjdStart + 9583*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9588*second) + ' since array.time is ' + str(array.time())

# Scan 266 = No0270
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9606*second, mjdStart+9613*second, 'No0270', obsCode, stnCode )
if array.time() < mjdStart + (9613-10)*second:
  subarray.execute(mjdStart + 9608*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9613*second) + ' since array.time is ' + str(array.time())

# Scan 267 = No0271
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9631*second, mjdStart+9638*second, 'No0271', obsCode, stnCode )
if array.time() < mjdStart + (9638-10)*second:
  subarray.execute(mjdStart + 9633*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9638*second) + ' since array.time is ' + str(array.time())

# Scan 268 = No0272
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9657*second, mjdStart+9663*second, 'No0272', obsCode, stnCode )
if array.time() < mjdStart + (9663-10)*second:
  subarray.execute(mjdStart + 9658*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9663*second) + ' since array.time is ' + str(array.time())

# Scan 269 = No0273
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9681*second, mjdStart+9688*second, 'No0273', obsCode, stnCode )
if array.time() < mjdStart + (9688-10)*second:
  subarray.execute(mjdStart + 9683*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9688*second) + ' since array.time is ' + str(array.time())

# Scan 270 = No0274
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9707*second, mjdStart+9713*second, 'No0274', obsCode, stnCode )
if array.time() < mjdStart + (9713-10)*second:
  subarray.execute(mjdStart + 9708*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9713*second) + ' since array.time is ' + str(array.time())

# Scan 271 = No0275
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9731*second, mjdStart+9738*second, 'No0275', obsCode, stnCode )
if array.time() < mjdStart + (9738-10)*second:
  subarray.execute(mjdStart + 9733*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9738*second) + ' since array.time is ' + str(array.time())

# Scan 272 = No0276
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9757*second, mjdStart+9763*second, 'No0276', obsCode, stnCode )
if array.time() < mjdStart + (9763-10)*second:
  subarray.execute(mjdStart + 9758*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9763*second) + ' since array.time is ' + str(array.time())

# Scan 273 = No0277
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9782*second, mjdStart+9788*second, 'No0277', obsCode, stnCode )
if array.time() < mjdStart + (9788-10)*second:
  subarray.execute(mjdStart + 9783*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9788*second) + ' since array.time is ' + str(array.time())

# Scan 274 = No0278
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9807*second, mjdStart+9813*second, 'No0278', obsCode, stnCode )
if array.time() < mjdStart + (9813-10)*second:
  subarray.execute(mjdStart + 9808*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9813*second) + ' since array.time is ' + str(array.time())

# Scan 275 = No0279
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9832*second, mjdStart+9838*second, 'No0279', obsCode, stnCode )
if array.time() < mjdStart + (9838-10)*second:
  subarray.execute(mjdStart + 9833*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9838*second) + ' since array.time is ' + str(array.time())

# Scan 276 = No0280
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9858*second, mjdStart+9863*second, 'No0280', obsCode, stnCode )
if array.time() < mjdStart + (9863-10)*second:
  subarray.execute(mjdStart + 9858*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9863*second) + ' since array.time is ' + str(array.time())

# Scan 277 = No0281
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9882*second, mjdStart+9888*second, 'No0281', obsCode, stnCode )
if array.time() < mjdStart + (9888-10)*second:
  subarray.execute(mjdStart + 9883*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9888*second) + ' since array.time is ' + str(array.time())

# Scan 278 = No0282
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9908*second, mjdStart+9912*second, 'No0282', obsCode, stnCode )
if array.time() < mjdStart + (9912-10)*second:
  subarray.execute(mjdStart + 9907*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9912*second) + ' since array.time is ' + str(array.time())

# Scan 279 = No0283
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9931*second, mjdStart+9937*second, 'No0283', obsCode, stnCode )
if array.time() < mjdStart + (9937-10)*second:
  subarray.execute(mjdStart + 9932*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9937*second) + ' since array.time is ' + str(array.time())

# Scan 280 = No0284
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9957*second, mjdStart+9962*second, 'No0284', obsCode, stnCode )
if array.time() < mjdStart + (9962-10)*second:
  subarray.execute(mjdStart + 9957*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9962*second) + ' since array.time is ' + str(array.time())

# Scan 281 = No0285
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9981*second, mjdStart+9987*second, 'No0285', obsCode, stnCode )
if array.time() < mjdStart + (9987-10)*second:
  subarray.execute(mjdStart + 9982*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9987*second) + ' since array.time is ' + str(array.time())

# Scan 282 = No0286
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10007*second, mjdStart+10012*second, 'No0286', obsCode, stnCode )
if array.time() < mjdStart + (10012-10)*second:
  subarray.execute(mjdStart + 10007*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10012*second) + ' since array.time is ' + str(array.time())

# Scan 283 = No0287
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10031*second, mjdStart+10037*second, 'No0287', obsCode, stnCode )
if array.time() < mjdStart + (10037-10)*second:
  subarray.execute(mjdStart + 10032*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10037*second) + ' since array.time is ' + str(array.time())

# Scan 284 = No0288
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10057*second, mjdStart+10062*second, 'No0288', obsCode, stnCode )
if array.time() < mjdStart + (10062-10)*second:
  subarray.execute(mjdStart + 10057*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10062*second) + ' since array.time is ' + str(array.time())

# Scan 285 = No0289
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10081*second, mjdStart+10087*second, 'No0289', obsCode, stnCode )
if array.time() < mjdStart + (10087-10)*second:
  subarray.execute(mjdStart + 10082*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10087*second) + ' since array.time is ' + str(array.time())

# Scan 286 = No0290
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10108*second, mjdStart+10112*second, 'No0290', obsCode, stnCode )
if array.time() < mjdStart + (10112-10)*second:
  subarray.execute(mjdStart + 10107*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10112*second) + ' since array.time is ' + str(array.time())

# Scan 287 = No0291
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10131*second, mjdStart+10137*second, 'No0291', obsCode, stnCode )
if array.time() < mjdStart + (10137-10)*second:
  subarray.execute(mjdStart + 10132*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10137*second) + ' since array.time is ' + str(array.time())

# Scan 288 = No0292
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10158*second, mjdStart+10162*second, 'No0292', obsCode, stnCode )
if array.time() < mjdStart + (10162-10)*second:
  subarray.execute(mjdStart + 10157*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10162*second) + ' since array.time is ' + str(array.time())

# Scan 289 = No0293
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10181*second, mjdStart+10187*second, 'No0293', obsCode, stnCode )
if array.time() < mjdStart + (10187-10)*second:
  subarray.execute(mjdStart + 10182*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10187*second) + ' since array.time is ' + str(array.time())

# Scan 290 = No0294
# changing to mode rdbe.1cm
subarray.setChannels(dbe0, channelSet2)
subarray.setVLBALoIfSetup(dbe0, loif2)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10193*second, mjdStart+10217*second, 'No0294', obsCode, stnCode )
if array.time() < mjdStart + (10217-10)*second:
  subarray.execute(mjdStart + 10212*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10217*second) + ' since array.time is ' + str(array.time())

# Scan 291 = No0295
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10237*second, mjdStart+10247*second, 'No0295', obsCode, stnCode )
if array.time() < mjdStart + (10247-10)*second:
  subarray.execute(mjdStart + 10242*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10247*second) + ' since array.time is ' + str(array.time())

# Scan 292 = No0296
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10266*second, mjdStart+10276*second, 'No0296', obsCode, stnCode )
if array.time() < mjdStart + (10276-10)*second:
  subarray.execute(mjdStart + 10271*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10276*second) + ' since array.time is ' + str(array.time())

# Scan 293 = No0297
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10296*second, mjdStart+10306*second, 'No0297', obsCode, stnCode )
if array.time() < mjdStart + (10306-10)*second:
  subarray.execute(mjdStart + 10301*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10306*second) + ' since array.time is ' + str(array.time())

# Scan 294 = No0298
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10325*second, mjdStart+10336*second, 'No0298', obsCode, stnCode )
if array.time() < mjdStart + (10336-10)*second:
  subarray.execute(mjdStart + 10331*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10336*second) + ' since array.time is ' + str(array.time())

# Scan 295 = No0299
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10356*second, mjdStart+10366*second, 'No0299', obsCode, stnCode )
if array.time() < mjdStart + (10366-10)*second:
  subarray.execute(mjdStart + 10361*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10366*second) + ' since array.time is ' + str(array.time())

# Scan 296 = No0300
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10384*second, mjdStart+10396*second, 'No0300', obsCode, stnCode )
if array.time() < mjdStart + (10396-10)*second:
  subarray.execute(mjdStart + 10391*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10396*second) + ' since array.time is ' + str(array.time())

# Scan 297 = No0301
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10415*second, mjdStart+10426*second, 'No0301', obsCode, stnCode )
if array.time() < mjdStart + (10426-10)*second:
  subarray.execute(mjdStart + 10421*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10426*second) + ' since array.time is ' + str(array.time())

# Scan 298 = No0302
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10444*second, mjdStart+10456*second, 'No0302', obsCode, stnCode )
if array.time() < mjdStart + (10456-10)*second:
  subarray.execute(mjdStart + 10451*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10456*second) + ' since array.time is ' + str(array.time())

# Scan 299 = No0303
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10474*second, mjdStart+10486*second, 'No0303', obsCode, stnCode )
if array.time() < mjdStart + (10486-10)*second:
  subarray.execute(mjdStart + 10481*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10486*second) + ' since array.time is ' + str(array.time())

# Scan 300 = No0304
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10503*second, mjdStart+10516*second, 'No0304', obsCode, stnCode )
if array.time() < mjdStart + (10516-10)*second:
  subarray.execute(mjdStart + 10511*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10516*second) + ' since array.time is ' + str(array.time())

# Scan 301 = No0305
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10534*second, mjdStart+10546*second, 'No0305', obsCode, stnCode )
if array.time() < mjdStart + (10546-10)*second:
  subarray.execute(mjdStart + 10541*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10546*second) + ' since array.time is ' + str(array.time())

# Scan 302 = No0306
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10562*second, mjdStart+10576*second, 'No0306', obsCode, stnCode )
if array.time() < mjdStart + (10576-10)*second:
  subarray.execute(mjdStart + 10571*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10576*second) + ' since array.time is ' + str(array.time())

# Scan 303 = No0307
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10593*second, mjdStart+10606*second, 'No0307', obsCode, stnCode )
if array.time() < mjdStart + (10606-10)*second:
  subarray.execute(mjdStart + 10601*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10606*second) + ' since array.time is ' + str(array.time())

# Scan 304 = No0308
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10621*second, mjdStart+10635*second, 'No0308', obsCode, stnCode )
if array.time() < mjdStart + (10635-10)*second:
  subarray.execute(mjdStart + 10630*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10635*second) + ' since array.time is ' + str(array.time())

# Scan 305 = No0309
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10651*second, mjdStart+10665*second, 'No0309', obsCode, stnCode )
if array.time() < mjdStart + (10665-10)*second:
  subarray.execute(mjdStart + 10660*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10665*second) + ' since array.time is ' + str(array.time())

# Scan 306 = No0310
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10679*second, mjdStart+10695*second, 'No0310', obsCode, stnCode )
if array.time() < mjdStart + (10695-10)*second:
  subarray.execute(mjdStart + 10690*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10695*second) + ' since array.time is ' + str(array.time())

# Scan 307 = No0311
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10709*second, mjdStart+10725*second, 'No0311', obsCode, stnCode )
if array.time() < mjdStart + (10725-10)*second:
  subarray.execute(mjdStart + 10720*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10725*second) + ' since array.time is ' + str(array.time())

# Scan 308 = No0312
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10738*second, mjdStart+10755*second, 'No0312', obsCode, stnCode )
if array.time() < mjdStart + (10755-10)*second:
  subarray.execute(mjdStart + 10750*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10755*second) + ' since array.time is ' + str(array.time())

# Scan 309 = No0313
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10768*second, mjdStart+10785*second, 'No0313', obsCode, stnCode )
if array.time() < mjdStart + (10785-10)*second:
  subarray.execute(mjdStart + 10780*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10785*second) + ' since array.time is ' + str(array.time())

# Scan 310 = No0314
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10797*second, mjdStart+10815*second, 'No0314', obsCode, stnCode )
if array.time() < mjdStart + (10815-10)*second:
  subarray.execute(mjdStart + 10810*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10815*second) + ' since array.time is ' + str(array.time())

# Scan 311 = No0315
# changing to mode rdbe.2cm
subarray.setChannels(dbe0, channelSet1)
subarray.setVLBALoIfSetup(dbe0, loif1)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10821*second, mjdStart+10875*second, 'No0315', obsCode, stnCode )
if array.time() < mjdStart + (10875-10)*second:
  subarray.execute(mjdStart + 10870*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10875*second) + ' since array.time is ' + str(array.time())

# Scan 312 = No0316
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10886*second, mjdStart+10935*second, 'No0316', obsCode, stnCode )
if array.time() < mjdStart + (10935-10)*second:
  subarray.execute(mjdStart + 10930*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10935*second) + ' since array.time is ' + str(array.time())

# Scan 313 = No0317
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10945*second, mjdStart+10994*second, 'No0317', obsCode, stnCode )
if array.time() < mjdStart + (10994-10)*second:
  subarray.execute(mjdStart + 10989*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10994*second) + ' since array.time is ' + str(array.time())

# Scan 314 = No0318
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11004*second, mjdStart+11054*second, 'No0318', obsCode, stnCode )
if array.time() < mjdStart + (11054-10)*second:
  subarray.execute(mjdStart + 11049*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11054*second) + ' since array.time is ' + str(array.time())

# Scan 315 = No0319
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11064*second, mjdStart+11114*second, 'No0319', obsCode, stnCode )
if array.time() < mjdStart + (11114-10)*second:
  subarray.execute(mjdStart + 11109*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11114*second) + ' since array.time is ' + str(array.time())

# Scan 316 = No0320
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11124*second, mjdStart+11174*second, 'No0320', obsCode, stnCode )
if array.time() < mjdStart + (11174-10)*second:
  subarray.execute(mjdStart + 11169*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11174*second) + ' since array.time is ' + str(array.time())

# Scan 317 = No0321
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11184*second, mjdStart+11234*second, 'No0321', obsCode, stnCode )
if array.time() < mjdStart + (11234-10)*second:
  subarray.execute(mjdStart + 11229*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11234*second) + ' since array.time is ' + str(array.time())

# Scan 318 = No0322
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11244*second, mjdStart+11294*second, 'No0322', obsCode, stnCode )
if array.time() < mjdStart + (11294-10)*second:
  subarray.execute(mjdStart + 11289*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11294*second) + ' since array.time is ' + str(array.time())

# Scan 319 = No0323
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11304*second, mjdStart+11353*second, 'No0323', obsCode, stnCode )
if array.time() < mjdStart + (11353-10)*second:
  subarray.execute(mjdStart + 11348*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11353*second) + ' since array.time is ' + str(array.time())

# Scan 320 = No0324
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11363*second, mjdStart+11413*second, 'No0324', obsCode, stnCode )
if array.time() < mjdStart + (11413-10)*second:
  subarray.execute(mjdStart + 11408*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11413*second) + ' since array.time is ' + str(array.time())

# Scan 321 = No0325
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11423*second, mjdStart+11473*second, 'No0325', obsCode, stnCode )
if array.time() < mjdStart + (11473-10)*second:
  subarray.execute(mjdStart + 11468*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11473*second) + ' since array.time is ' + str(array.time())

# Scan 322 = No0326
# changing to mode trdbe.sx
subarray.setChannels(dbe0, channelSet0)
subarray.setVLBALoIfSetup(dbe0, loif0)
subarray.set4x4Switch('1A', 1)
subarray.set4x4Switch('1B', 2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11479*second, mjdStart+11533*second, 'No0326', obsCode, stnCode )
if array.time() < mjdStart + (11533-10)*second:
  subarray.execute(mjdStart + 11528*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11533*second) + ' since array.time is ' + str(array.time())

# Scan 323 = No0327
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11544*second, mjdStart+11593*second, 'No0327', obsCode, stnCode )
if array.time() < mjdStart + (11593-10)*second:
  subarray.execute(mjdStart + 11588*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11593*second) + ' since array.time is ' + str(array.time())

# Scan 324 = No0328
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11604*second, mjdStart+11653*second, 'No0328', obsCode, stnCode )
if array.time() < mjdStart + (11653-10)*second:
  subarray.execute(mjdStart + 11648*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11653*second) + ' since array.time is ' + str(array.time())

# Scan 325 = No0329
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11664*second, mjdStart+11713*second, 'No0329', obsCode, stnCode )
if array.time() < mjdStart + (11713-10)*second:
  subarray.execute(mjdStart + 11708*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11713*second) + ' since array.time is ' + str(array.time())

# Scan 326 = No0330
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11724*second, mjdStart+11772*second, 'No0330', obsCode, stnCode )
if array.time() < mjdStart + (11772-10)*second:
  subarray.execute(mjdStart + 11767*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11772*second) + ' since array.time is ' + str(array.time())

# Scan 327 = No0331
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11783*second, mjdStart+11832*second, 'No0331', obsCode, stnCode )
if array.time() < mjdStart + (11832-10)*second:
  subarray.execute(mjdStart + 11827*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11832*second) + ' since array.time is ' + str(array.time())

# Scan 328 = No0332
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11844*second, mjdStart+11892*second, 'No0332', obsCode, stnCode )
if array.time() < mjdStart + (11892-10)*second:
  subarray.execute(mjdStart + 11887*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11892*second) + ' since array.time is ' + str(array.time())

# Scan 329 = No0333
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11903*second, mjdStart+11952*second, 'No0333', obsCode, stnCode )
if array.time() < mjdStart + (11952-10)*second:
  subarray.execute(mjdStart + 11947*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11952*second) + ' since array.time is ' + str(array.time())

# Scan 330 = No0334
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11964*second, mjdStart+12012*second, 'No0334', obsCode, stnCode )
if array.time() < mjdStart + (12012-10)*second:
  subarray.execute(mjdStart + 12007*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12012*second) + ' since array.time is ' + str(array.time())

# Scan 331 = No0335
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12023*second, mjdStart+12072*second, 'No0335', obsCode, stnCode )
if array.time() < mjdStart + (12072-10)*second:
  subarray.execute(mjdStart + 12067*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12072*second) + ' since array.time is ' + str(array.time())

# Scan 332 = No0336
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12084*second, mjdStart+12131*second, 'No0336', obsCode, stnCode )
if array.time() < mjdStart + (12131-10)*second:
  subarray.execute(mjdStart + 12126*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12131*second) + ' since array.time is ' + str(array.time())

# Scan 333 = No0337
subarray.setSource(source0)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12178*second, mjdStart+12251*second, 'No0337', obsCode, stnCode )
if array.time() < mjdStart + (12251-10)*second:
  subarray.execute(mjdStart + 12246*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12251*second) + ' since array.time is ' + str(array.time())

# Scan 334 = No0338
# changing to mode rdbe.1cm
subarray.setChannels(dbe0, channelSet2)
subarray.setVLBALoIfSetup(dbe0, loif2)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12257*second, mjdStart+12371*second, 'No0338', obsCode, stnCode )
if array.time() < mjdStart + (12371-10)*second:
  subarray.execute(mjdStart + 12366*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12371*second) + ' since array.time is ' + str(array.time())

# Scan 335 = No0339
# changing to mode rdbe.2cm
subarray.setChannels(dbe0, channelSet1)
subarray.setVLBALoIfSetup(dbe0, loif1)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12377*second, mjdStart+12490*second, 'No0339', obsCode, stnCode )
if array.time() < mjdStart + (12490-10)*second:
  subarray.execute(mjdStart + 12485*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12490*second) + ' since array.time is ' + str(array.time())

# Scan 336 = No0340
# changing to mode rdbe.7mm
subarray.setChannels(dbe0, channelSet3)
subarray.setVLBALoIfSetup(dbe0, loif3)
subarray.set4x4Switch('1A', 3)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12496*second, mjdStart+12610*second, 'No0340', obsCode, stnCode )
if array.time() < mjdStart + (12610-10)*second:
  subarray.execute(mjdStart + 12605*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12610*second) + ' since array.time is ' + str(array.time())

# Scan 337 = No0342
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12646*second, mjdStart+12660*second, 'No0342', obsCode, stnCode )
if array.time() < mjdStart + (12660-10)*second:
  subarray.execute(mjdStart + 12655*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12660*second) + ' since array.time is ' + str(array.time())

# Scan 338 = No0343
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12671*second, mjdStart+12685*second, 'No0343', obsCode, stnCode )
if array.time() < mjdStart + (12685-10)*second:
  subarray.execute(mjdStart + 12680*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12685*second) + ' since array.time is ' + str(array.time())

# Scan 339 = No0344
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12696*second, mjdStart+12710*second, 'No0344', obsCode, stnCode )
if array.time() < mjdStart + (12710-10)*second:
  subarray.execute(mjdStart + 12705*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12710*second) + ' since array.time is ' + str(array.time())

# Scan 340 = No0345
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12721*second, mjdStart+12735*second, 'No0345', obsCode, stnCode )
if array.time() < mjdStart + (12735-10)*second:
  subarray.execute(mjdStart + 12730*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12735*second) + ' since array.time is ' + str(array.time())

# Scan 341 = No0346
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12746*second, mjdStart+12760*second, 'No0346', obsCode, stnCode )
if array.time() < mjdStart + (12760-10)*second:
  subarray.execute(mjdStart + 12755*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12760*second) + ' since array.time is ' + str(array.time())

# Scan 342 = No0347
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12771*second, mjdStart+12785*second, 'No0347', obsCode, stnCode )
if array.time() < mjdStart + (12785-10)*second:
  subarray.execute(mjdStart + 12780*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12785*second) + ' since array.time is ' + str(array.time())

# Scan 343 = No0348
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12796*second, mjdStart+12810*second, 'No0348', obsCode, stnCode )
if array.time() < mjdStart + (12810-10)*second:
  subarray.execute(mjdStart + 12805*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12810*second) + ' since array.time is ' + str(array.time())

# Scan 344 = No0349
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12821*second, mjdStart+12834*second, 'No0349', obsCode, stnCode )
if array.time() < mjdStart + (12834-10)*second:
  subarray.execute(mjdStart + 12829*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12834*second) + ' since array.time is ' + str(array.time())

# Scan 345 = No0350
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12845*second, mjdStart+12859*second, 'No0350', obsCode, stnCode )
if array.time() < mjdStart + (12859-10)*second:
  subarray.execute(mjdStart + 12854*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12859*second) + ' since array.time is ' + str(array.time())

# Scan 346 = No0351
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12870*second, mjdStart+12884*second, 'No0351', obsCode, stnCode )
if array.time() < mjdStart + (12884-10)*second:
  subarray.execute(mjdStart + 12879*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12884*second) + ' since array.time is ' + str(array.time())

# Scan 347 = No0352
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12895*second, mjdStart+12909*second, 'No0352', obsCode, stnCode )
if array.time() < mjdStart + (12909-10)*second:
  subarray.execute(mjdStart + 12904*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12909*second) + ' since array.time is ' + str(array.time())

# Scan 348 = No0353
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12920*second, mjdStart+12934*second, 'No0353', obsCode, stnCode )
if array.time() < mjdStart + (12934-10)*second:
  subarray.execute(mjdStart + 12929*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12934*second) + ' since array.time is ' + str(array.time())

# Scan 349 = No0354
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12945*second, mjdStart+12959*second, 'No0354', obsCode, stnCode )
if array.time() < mjdStart + (12959-10)*second:
  subarray.execute(mjdStart + 12954*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12959*second) + ' since array.time is ' + str(array.time())

# Scan 350 = No0355
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12970*second, mjdStart+12984*second, 'No0355', obsCode, stnCode )
if array.time() < mjdStart + (12984-10)*second:
  subarray.execute(mjdStart + 12979*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12984*second) + ' since array.time is ' + str(array.time())

# Scan 351 = No0356
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12995*second, mjdStart+13009*second, 'No0356', obsCode, stnCode )
if array.time() < mjdStart + (13009-10)*second:
  subarray.execute(mjdStart + 13004*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13009*second) + ' since array.time is ' + str(array.time())

# Scan 352 = No0357
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13020*second, mjdStart+13034*second, 'No0357', obsCode, stnCode )
if array.time() < mjdStart + (13034-10)*second:
  subarray.execute(mjdStart + 13029*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13034*second) + ' since array.time is ' + str(array.time())

# Scan 353 = No0358
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13045*second, mjdStart+13059*second, 'No0358', obsCode, stnCode )
if array.time() < mjdStart + (13059-10)*second:
  subarray.execute(mjdStart + 13054*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13059*second) + ' since array.time is ' + str(array.time())

# Scan 354 = No0359
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13070*second, mjdStart+13084*second, 'No0359', obsCode, stnCode )
if array.time() < mjdStart + (13084-10)*second:
  subarray.execute(mjdStart + 13079*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13084*second) + ' since array.time is ' + str(array.time())

# Scan 355 = No0360
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13095*second, mjdStart+13109*second, 'No0360', obsCode, stnCode )
if array.time() < mjdStart + (13109-10)*second:
  subarray.execute(mjdStart + 13104*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13109*second) + ' since array.time is ' + str(array.time())

# Scan 356 = No0361
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13120*second, mjdStart+13134*second, 'No0361', obsCode, stnCode )
if array.time() < mjdStart + (13134-10)*second:
  subarray.execute(mjdStart + 13129*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13134*second) + ' since array.time is ' + str(array.time())

# Scan 357 = No0362
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13145*second, mjdStart+13159*second, 'No0362', obsCode, stnCode )
if array.time() < mjdStart + (13159-10)*second:
  subarray.execute(mjdStart + 13154*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13159*second) + ' since array.time is ' + str(array.time())

# Scan 358 = No0363
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13170*second, mjdStart+13183*second, 'No0363', obsCode, stnCode )
if array.time() < mjdStart + (13183-10)*second:
  subarray.execute(mjdStart + 13178*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13183*second) + ' since array.time is ' + str(array.time())

# Scan 359 = No0364
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13194*second, mjdStart+13208*second, 'No0364', obsCode, stnCode )
if array.time() < mjdStart + (13208-10)*second:
  subarray.execute(mjdStart + 13203*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13208*second) + ' since array.time is ' + str(array.time())

# Scan 360 = No0365
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13219*second, mjdStart+13233*second, 'No0365', obsCode, stnCode )
if array.time() < mjdStart + (13233-10)*second:
  subarray.execute(mjdStart + 13228*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13233*second) + ' since array.time is ' + str(array.time())

# Scan 361 = No0366
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13244*second, mjdStart+13258*second, 'No0366', obsCode, stnCode )
if array.time() < mjdStart + (13258-10)*second:
  subarray.execute(mjdStart + 13253*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13258*second) + ' since array.time is ' + str(array.time())

# Scan 362 = No0367
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13269*second, mjdStart+13283*second, 'No0367', obsCode, stnCode )
if array.time() < mjdStart + (13283-10)*second:
  subarray.execute(mjdStart + 13278*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13283*second) + ' since array.time is ' + str(array.time())

# Scan 363 = No0368
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13294*second, mjdStart+13308*second, 'No0368', obsCode, stnCode )
if array.time() < mjdStart + (13308-10)*second:
  subarray.execute(mjdStart + 13303*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13308*second) + ' since array.time is ' + str(array.time())

# Scan 364 = No0369
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13319*second, mjdStart+13333*second, 'No0369', obsCode, stnCode )
if array.time() < mjdStart + (13333-10)*second:
  subarray.execute(mjdStart + 13328*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13333*second) + ' since array.time is ' + str(array.time())

# Scan 365 = No0370
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13344*second, mjdStart+13358*second, 'No0370', obsCode, stnCode )
if array.time() < mjdStart + (13358-10)*second:
  subarray.execute(mjdStart + 13353*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13358*second) + ' since array.time is ' + str(array.time())

# Scan 366 = No0371
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13369*second, mjdStart+13383*second, 'No0371', obsCode, stnCode )
if array.time() < mjdStart + (13383-10)*second:
  subarray.execute(mjdStart + 13378*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13383*second) + ' since array.time is ' + str(array.time())

# Scan 367 = No0372
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13394*second, mjdStart+13408*second, 'No0372', obsCode, stnCode )
if array.time() < mjdStart + (13408-10)*second:
  subarray.execute(mjdStart + 13403*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13408*second) + ' since array.time is ' + str(array.time())

# Scan 368 = No0373
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13419*second, mjdStart+13433*second, 'No0373', obsCode, stnCode )
if array.time() < mjdStart + (13433-10)*second:
  subarray.execute(mjdStart + 13428*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13433*second) + ' since array.time is ' + str(array.time())

# Scan 369 = No0374
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13444*second, mjdStart+13458*second, 'No0374', obsCode, stnCode )
if array.time() < mjdStart + (13458-10)*second:
  subarray.execute(mjdStart + 13453*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13458*second) + ' since array.time is ' + str(array.time())

# Scan 370 = No0375
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13469*second, mjdStart+13483*second, 'No0375', obsCode, stnCode )
if array.time() < mjdStart + (13483-10)*second:
  subarray.execute(mjdStart + 13478*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13483*second) + ' since array.time is ' + str(array.time())

# Scan 371 = No0376
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13494*second, mjdStart+13508*second, 'No0376', obsCode, stnCode )
if array.time() < mjdStart + (13508-10)*second:
  subarray.execute(mjdStart + 13503*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13508*second) + ' since array.time is ' + str(array.time())

# Scan 372 = No0377
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13519*second, mjdStart+13533*second, 'No0377', obsCode, stnCode )
if array.time() < mjdStart + (13533-10)*second:
  subarray.execute(mjdStart + 13528*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13533*second) + ' since array.time is ' + str(array.time())

# Scan 373 = No0378
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13544*second, mjdStart+13557*second, 'No0378', obsCode, stnCode )
if array.time() < mjdStart + (13557-10)*second:
  subarray.execute(mjdStart + 13552*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13557*second) + ' since array.time is ' + str(array.time())

# Scan 374 = No0379
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13568*second, mjdStart+13582*second, 'No0379', obsCode, stnCode )
if array.time() < mjdStart + (13582-10)*second:
  subarray.execute(mjdStart + 13577*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13582*second) + ' since array.time is ' + str(array.time())

# Scan 375 = No0380
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13593*second, mjdStart+13607*second, 'No0380', obsCode, stnCode )
if array.time() < mjdStart + (13607-10)*second:
  subarray.execute(mjdStart + 13602*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13607*second) + ' since array.time is ' + str(array.time())

# Scan 376 = No0381
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13618*second, mjdStart+13632*second, 'No0381', obsCode, stnCode )
if array.time() < mjdStart + (13632-10)*second:
  subarray.execute(mjdStart + 13627*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13632*second) + ' since array.time is ' + str(array.time())

# Scan 377 = No0382
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13643*second, mjdStart+13657*second, 'No0382', obsCode, stnCode )
if array.time() < mjdStart + (13657-10)*second:
  subarray.execute(mjdStart + 13652*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13657*second) + ' since array.time is ' + str(array.time())

# Scan 378 = No0383
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13668*second, mjdStart+13682*second, 'No0383', obsCode, stnCode )
if array.time() < mjdStart + (13682-10)*second:
  subarray.execute(mjdStart + 13677*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13682*second) + ' since array.time is ' + str(array.time())

# Scan 379 = No0384
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13693*second, mjdStart+13707*second, 'No0384', obsCode, stnCode )
if array.time() < mjdStart + (13707-10)*second:
  subarray.execute(mjdStart + 13702*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13707*second) + ' since array.time is ' + str(array.time())

# Scan 380 = No0385
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13718*second, mjdStart+13732*second, 'No0385', obsCode, stnCode )
if array.time() < mjdStart + (13732-10)*second:
  subarray.execute(mjdStart + 13727*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13732*second) + ' since array.time is ' + str(array.time())

# Scan 381 = No0386
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13743*second, mjdStart+13757*second, 'No0386', obsCode, stnCode )
if array.time() < mjdStart + (13757-10)*second:
  subarray.execute(mjdStart + 13752*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13757*second) + ' since array.time is ' + str(array.time())

# Scan 382 = No0387
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13768*second, mjdStart+13782*second, 'No0387', obsCode, stnCode )
if array.time() < mjdStart + (13782-10)*second:
  subarray.execute(mjdStart + 13777*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13782*second) + ' since array.time is ' + str(array.time())

# Scan 383 = No0388
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13793*second, mjdStart+13807*second, 'No0388', obsCode, stnCode )
if array.time() < mjdStart + (13807-10)*second:
  subarray.execute(mjdStart + 13802*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13807*second) + ' since array.time is ' + str(array.time())

# Scan 384 = No0389
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13818*second, mjdStart+13832*second, 'No0389', obsCode, stnCode )
if array.time() < mjdStart + (13832-10)*second:
  subarray.execute(mjdStart + 13827*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13832*second) + ' since array.time is ' + str(array.time())

# Scan 385 = No0390
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13843*second, mjdStart+13857*second, 'No0390', obsCode, stnCode )
if array.time() < mjdStart + (13857-10)*second:
  subarray.execute(mjdStart + 13852*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13857*second) + ' since array.time is ' + str(array.time())

# Scan 386 = No0391
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13868*second, mjdStart+13882*second, 'No0391', obsCode, stnCode )
if array.time() < mjdStart + (13882-10)*second:
  subarray.execute(mjdStart + 13877*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13882*second) + ' since array.time is ' + str(array.time())

# Scan 387 = No0392
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13893*second, mjdStart+13907*second, 'No0392', obsCode, stnCode )
if array.time() < mjdStart + (13907-10)*second:
  subarray.execute(mjdStart + 13902*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13907*second) + ' since array.time is ' + str(array.time())

# Scan 388 = No0393
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13918*second, mjdStart+13931*second, 'No0393', obsCode, stnCode )
if array.time() < mjdStart + (13931-10)*second:
  subarray.execute(mjdStart + 13926*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13931*second) + ' since array.time is ' + str(array.time())

# Scan 389 = No0394
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13942*second, mjdStart+13956*second, 'No0394', obsCode, stnCode )
if array.time() < mjdStart + (13956-10)*second:
  subarray.execute(mjdStart + 13951*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13956*second) + ' since array.time is ' + str(array.time())

# Scan 390 = No0395
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13967*second, mjdStart+13981*second, 'No0395', obsCode, stnCode )
if array.time() < mjdStart + (13981-10)*second:
  subarray.execute(mjdStart + 13976*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13981*second) + ' since array.time is ' + str(array.time())

# Scan 391 = No0396
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13992*second, mjdStart+14006*second, 'No0396', obsCode, stnCode )
if array.time() < mjdStart + (14006-10)*second:
  subarray.execute(mjdStart + 14001*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14006*second) + ' since array.time is ' + str(array.time())

# Scan 392 = No0397
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14017*second, mjdStart+14031*second, 'No0397', obsCode, stnCode )
if array.time() < mjdStart + (14031-10)*second:
  subarray.execute(mjdStart + 14026*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14031*second) + ' since array.time is ' + str(array.time())

# Scan 393 = No0398
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14042*second, mjdStart+14056*second, 'No0398', obsCode, stnCode )
if array.time() < mjdStart + (14056-10)*second:
  subarray.execute(mjdStart + 14051*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14056*second) + ' since array.time is ' + str(array.time())

# Scan 394 = No0399
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14067*second, mjdStart+14081*second, 'No0399', obsCode, stnCode )
if array.time() < mjdStart + (14081-10)*second:
  subarray.execute(mjdStart + 14076*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14081*second) + ' since array.time is ' + str(array.time())

# Scan 395 = No0400
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14092*second, mjdStart+14106*second, 'No0400', obsCode, stnCode )
if array.time() < mjdStart + (14106-10)*second:
  subarray.execute(mjdStart + 14101*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14106*second) + ' since array.time is ' + str(array.time())

# Scan 396 = No0401
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14117*second, mjdStart+14131*second, 'No0401', obsCode, stnCode )
if array.time() < mjdStart + (14131-10)*second:
  subarray.execute(mjdStart + 14126*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14131*second) + ' since array.time is ' + str(array.time())

# Scan 397 = No0402
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14141*second, mjdStart+14156*second, 'No0402', obsCode, stnCode )
if array.time() < mjdStart + (14156-10)*second:
  subarray.execute(mjdStart + 14151*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14156*second) + ' since array.time is ' + str(array.time())

# Scan 398 = No0403
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14166*second, mjdStart+14181*second, 'No0403', obsCode, stnCode )
if array.time() < mjdStart + (14181-10)*second:
  subarray.execute(mjdStart + 14176*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14181*second) + ' since array.time is ' + str(array.time())

# Scan 399 = No0404
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14191*second, mjdStart+14206*second, 'No0404', obsCode, stnCode )
if array.time() < mjdStart + (14206-10)*second:
  subarray.execute(mjdStart + 14201*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14206*second) + ' since array.time is ' + str(array.time())

# Scan 400 = No0405
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14216*second, mjdStart+14231*second, 'No0405', obsCode, stnCode )
if array.time() < mjdStart + (14231-10)*second:
  subarray.execute(mjdStart + 14226*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14231*second) + ' since array.time is ' + str(array.time())

# Scan 401 = No0406
# changing to mode rdbe.1cm
subarray.setChannels(dbe0, channelSet2)
subarray.setVLBALoIfSetup(dbe0, loif2)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14237*second, mjdStart+14261*second, 'No0406', obsCode, stnCode )
if array.time() < mjdStart + (14261-10)*second:
  subarray.execute(mjdStart + 14256*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14261*second) + ' since array.time is ' + str(array.time())

# Scan 402 = No0407
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14271*second, mjdStart+14290*second, 'No0407', obsCode, stnCode )
if array.time() < mjdStart + (14290-10)*second:
  subarray.execute(mjdStart + 14285*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14290*second) + ' since array.time is ' + str(array.time())

# Scan 403 = No0408
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14300*second, mjdStart+14320*second, 'No0408', obsCode, stnCode )
if array.time() < mjdStart + (14320-10)*second:
  subarray.execute(mjdStart + 14315*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14320*second) + ' since array.time is ' + str(array.time())

# Scan 404 = No0409
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14330*second, mjdStart+14350*second, 'No0409', obsCode, stnCode )
if array.time() < mjdStart + (14350-10)*second:
  subarray.execute(mjdStart + 14345*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14350*second) + ' since array.time is ' + str(array.time())

# Scan 405 = No0410
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14360*second, mjdStart+14380*second, 'No0410', obsCode, stnCode )
if array.time() < mjdStart + (14380-10)*second:
  subarray.execute(mjdStart + 14375*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14380*second) + ' since array.time is ' + str(array.time())

# Scan 406 = No0411
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14390*second, mjdStart+14410*second, 'No0411', obsCode, stnCode )
if array.time() < mjdStart + (14410-10)*second:
  subarray.execute(mjdStart + 14405*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14410*second) + ' since array.time is ' + str(array.time())

# Scan 407 = No0412
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14420*second, mjdStart+14440*second, 'No0412', obsCode, stnCode )
if array.time() < mjdStart + (14440-10)*second:
  subarray.execute(mjdStart + 14435*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14440*second) + ' since array.time is ' + str(array.time())

# Scan 408 = No0413
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14450*second, mjdStart+14470*second, 'No0413', obsCode, stnCode )
if array.time() < mjdStart + (14470-10)*second:
  subarray.execute(mjdStart + 14465*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14470*second) + ' since array.time is ' + str(array.time())

# Scan 409 = No0414
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14480*second, mjdStart+14500*second, 'No0414', obsCode, stnCode )
if array.time() < mjdStart + (14500-10)*second:
  subarray.execute(mjdStart + 14495*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14500*second) + ' since array.time is ' + str(array.time())

# Scan 410 = No0415
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14510*second, mjdStart+14530*second, 'No0415', obsCode, stnCode )
if array.time() < mjdStart + (14530-10)*second:
  subarray.execute(mjdStart + 14525*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14530*second) + ' since array.time is ' + str(array.time())

# Scan 411 = No0416
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14540*second, mjdStart+14560*second, 'No0416', obsCode, stnCode )
if array.time() < mjdStart + (14560-10)*second:
  subarray.execute(mjdStart + 14555*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14560*second) + ' since array.time is ' + str(array.time())

# Scan 412 = No0417
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14570*second, mjdStart+14590*second, 'No0417', obsCode, stnCode )
if array.time() < mjdStart + (14590-10)*second:
  subarray.execute(mjdStart + 14585*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14590*second) + ' since array.time is ' + str(array.time())

# Scan 413 = No0418
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14600*second, mjdStart+14620*second, 'No0418', obsCode, stnCode )
if array.time() < mjdStart + (14620-10)*second:
  subarray.execute(mjdStart + 14615*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14620*second) + ' since array.time is ' + str(array.time())

# Scan 414 = No0419
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14630*second, mjdStart+14649*second, 'No0419', obsCode, stnCode )
if array.time() < mjdStart + (14649-10)*second:
  subarray.execute(mjdStart + 14644*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14649*second) + ' since array.time is ' + str(array.time())

# Scan 415 = No0420
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14659*second, mjdStart+14679*second, 'No0420', obsCode, stnCode )
if array.time() < mjdStart + (14679-10)*second:
  subarray.execute(mjdStart + 14674*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14679*second) + ' since array.time is ' + str(array.time())

# Scan 416 = No0421
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14689*second, mjdStart+14709*second, 'No0421', obsCode, stnCode )
if array.time() < mjdStart + (14709-10)*second:
  subarray.execute(mjdStart + 14704*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14709*second) + ' since array.time is ' + str(array.time())

# Scan 417 = No0422
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14719*second, mjdStart+14739*second, 'No0422', obsCode, stnCode )
if array.time() < mjdStart + (14739-10)*second:
  subarray.execute(mjdStart + 14734*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14739*second) + ' since array.time is ' + str(array.time())

# Scan 418 = No0423
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14749*second, mjdStart+14769*second, 'No0423', obsCode, stnCode )
if array.time() < mjdStart + (14769-10)*second:
  subarray.execute(mjdStart + 14764*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14769*second) + ' since array.time is ' + str(array.time())

# Scan 419 = No0424
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14779*second, mjdStart+14799*second, 'No0424', obsCode, stnCode )
if array.time() < mjdStart + (14799-10)*second:
  subarray.execute(mjdStart + 14794*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14799*second) + ' since array.time is ' + str(array.time())

# Scan 420 = No0425
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14809*second, mjdStart+14829*second, 'No0425', obsCode, stnCode )
if array.time() < mjdStart + (14829-10)*second:
  subarray.execute(mjdStart + 14824*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14829*second) + ' since array.time is ' + str(array.time())

# Scan 421 = No0426
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14839*second, mjdStart+14859*second, 'No0426', obsCode, stnCode )
if array.time() < mjdStart + (14859-10)*second:
  subarray.execute(mjdStart + 14854*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14859*second) + ' since array.time is ' + str(array.time())

# Scan 422 = No0427
# changing to mode rdbe.2cm
subarray.setChannels(dbe0, channelSet1)
subarray.setVLBALoIfSetup(dbe0, loif1)
subarray.set4x4Switch('1A', 4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14865*second, mjdStart+14919*second, 'No0427', obsCode, stnCode )
if array.time() < mjdStart + (14919-10)*second:
  subarray.execute(mjdStart + 14914*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14919*second) + ' since array.time is ' + str(array.time())

# Scan 423 = No0428
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14929*second, mjdStart+14979*second, 'No0428', obsCode, stnCode )
if array.time() < mjdStart + (14979-10)*second:
  subarray.execute(mjdStart + 14974*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14979*second) + ' since array.time is ' + str(array.time())

# Scan 424 = No0429
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14989*second, mjdStart+15038*second, 'No0429', obsCode, stnCode )
if array.time() < mjdStart + (15038-10)*second:
  subarray.execute(mjdStart + 15033*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15038*second) + ' since array.time is ' + str(array.time())

# Scan 425 = No0430
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15048*second, mjdStart+15098*second, 'No0430', obsCode, stnCode )
if array.time() < mjdStart + (15098-10)*second:
  subarray.execute(mjdStart + 15093*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15098*second) + ' since array.time is ' + str(array.time())

# Scan 426 = No0431
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15108*second, mjdStart+15158*second, 'No0431', obsCode, stnCode )
if array.time() < mjdStart + (15158-10)*second:
  subarray.execute(mjdStart + 15153*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15158*second) + ' since array.time is ' + str(array.time())

# Scan 427 = No0432
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15168*second, mjdStart+15218*second, 'No0432', obsCode, stnCode )
if array.time() < mjdStart + (15218-10)*second:
  subarray.execute(mjdStart + 15213*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15218*second) + ' since array.time is ' + str(array.time())

# Scan 428 = No0433
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15228*second, mjdStart+15278*second, 'No0433', obsCode, stnCode )
if array.time() < mjdStart + (15278-10)*second:
  subarray.execute(mjdStart + 15273*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15278*second) + ' since array.time is ' + str(array.time())

# Scan 429 = No0434
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15288*second, mjdStart+15338*second, 'No0434', obsCode, stnCode )
if array.time() < mjdStart + (15338-10)*second:
  subarray.execute(mjdStart + 15333*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15338*second) + ' since array.time is ' + str(array.time())

# Scan 430 = No0435
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15348*second, mjdStart+15397*second, 'No0435', obsCode, stnCode )
if array.time() < mjdStart + (15397-10)*second:
  subarray.execute(mjdStart + 15392*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15397*second) + ' since array.time is ' + str(array.time())

# Scan 431 = No0436
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15407*second, mjdStart+15457*second, 'No0436', obsCode, stnCode )
if array.time() < mjdStart + (15457-10)*second:
  subarray.execute(mjdStart + 15452*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15457*second) + ' since array.time is ' + str(array.time())

# Scan 432 = No0437
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15467*second, mjdStart+15517*second, 'No0437', obsCode, stnCode )
if array.time() < mjdStart + (15517-10)*second:
  subarray.execute(mjdStart + 15512*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15517*second) + ' since array.time is ' + str(array.time())

# Scan 433 = No0438
# changing to mode trdbe.sx
subarray.setChannels(dbe0, channelSet0)
subarray.setVLBALoIfSetup(dbe0, loif0)
subarray.set4x4Switch('1A', 1)
subarray.set4x4Switch('1B', 2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15523*second, mjdStart+15577*second, 'No0438', obsCode, stnCode )
if array.time() < mjdStart + (15577-10)*second:
  subarray.execute(mjdStart + 15572*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15577*second) + ' since array.time is ' + str(array.time())

# Scan 434 = No0439
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15587*second, mjdStart+15637*second, 'No0439', obsCode, stnCode )
if array.time() < mjdStart + (15637-10)*second:
  subarray.execute(mjdStart + 15632*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15637*second) + ' since array.time is ' + str(array.time())

# Scan 435 = No0440
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15647*second, mjdStart+15697*second, 'No0440', obsCode, stnCode )
if array.time() < mjdStart + (15697-10)*second:
  subarray.execute(mjdStart + 15692*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15697*second) + ' since array.time is ' + str(array.time())

# Scan 436 = No0441
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15707*second, mjdStart+15756*second, 'No0441', obsCode, stnCode )
if array.time() < mjdStart + (15756-10)*second:
  subarray.execute(mjdStart + 15751*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15756*second) + ' since array.time is ' + str(array.time())

# Scan 437 = No0442
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15766*second, mjdStart+15816*second, 'No0442', obsCode, stnCode )
if array.time() < mjdStart + (15816-10)*second:
  subarray.execute(mjdStart + 15811*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15816*second) + ' since array.time is ' + str(array.time())

# Scan 438 = No0443
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15826*second, mjdStart+15876*second, 'No0443', obsCode, stnCode )
if array.time() < mjdStart + (15876-10)*second:
  subarray.execute(mjdStart + 15871*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15876*second) + ' since array.time is ' + str(array.time())

# Scan 439 = No0444
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15886*second, mjdStart+15936*second, 'No0444', obsCode, stnCode )
if array.time() < mjdStart + (15936-10)*second:
  subarray.execute(mjdStart + 15931*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15936*second) + ' since array.time is ' + str(array.time())

# Scan 440 = No0445
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15946*second, mjdStart+15996*second, 'No0445', obsCode, stnCode )
if array.time() < mjdStart + (15996-10)*second:
  subarray.execute(mjdStart + 15991*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15996*second) + ' since array.time is ' + str(array.time())

# Scan 441 = No0446
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 16006*second, mjdStart+16056*second, 'No0446', obsCode, stnCode )
if array.time() < mjdStart + (16056-10)*second:
  subarray.execute(mjdStart + 16051*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+16056*second) + ' since array.time is ' + str(array.time())

# Scan 442 = No0447
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 16066*second, mjdStart+16115*second, 'No0447', obsCode, stnCode )
if array.time() < mjdStart + (16115-10)*second:
  subarray.execute(mjdStart + 16110*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+16115*second) + ' since array.time is ' + str(array.time())

# Scan 443 = No0448
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 16125*second, mjdStart+16175*second, 'No0448', obsCode, stnCode )
if array.time() < mjdStart + (16175-10)*second:
  subarray.execute(mjdStart + 16170*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+16175*second) + ' since array.time is ' + str(array.time())

array.wait(mjdStart + 16176*second)
