from edu.nrao.evla.observe import Mark5C
from edu.nrao.evla.observe import ESSR
from edu.nrao.evla.observe import MatrixSwitch
from edu.nrao.evla.observe import RDBE
from edu.nrao.evla.observe import VLBALoIfSetup
from edu.nrao.evla.observe import Parameters
from edu.nrao.evla.observe import bbc

second = 1.0/86400.0

deltat2 = 1

obsCode = 'BL188B'
stnCode = 'MK'
mjdStart = 56470 + 44363*second

# File written by vex2script version 0.24 vintage 20131209

dbe0 = RDBE(0, 'pfb', 'PFBG_1_4.bin')
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
loif0.setIf('A', '6cm', 'R', 4100, 'U', 'NA', 0, '6cm', 784)
loif0.setIf('C', '6cm', 'L', 4100, 'U', 'NA', 0, '6cm', 784)
loif0.setPhaseCal(5)
loif0.setDBEParams(0, -1, -1, 10, 0)
loif0.setDBEParams(1, -1, -1, 10, 0)
loif0.setDBERemember(0, 1)
loif0.setDBERemember(1, 1)
channelSet0 = [ \
  bbc(0, 784, 32, 'L', 2, 0), \  # IF A
  bbc(1, 784, 32, 'L', 2, 0), \  # IF C
  bbc(0, 752, 32, 'L', 2, 0), \  # IF A
  bbc(1, 752, 32, 'L', 2, 0), \  # IF C
  bbc(0, 720, 32, 'L', 2, 0), \  # IF A
  bbc(1, 720, 32, 'L', 2, 0), \  # IF C
  bbc(0, 688, 32, 'L', 2, 0), \  # IF A
  bbc(1, 688, 32, 'L', 2, 0), \  # IF C
  bbc(0, 656, 32, 'L', 2, 0), \  # IF A
  bbc(1, 656, 32, 'L', 2, 0), \  # IF C
  bbc(0, 624, 32, 'L', 2, 0), \  # IF A
  bbc(1, 624, 32, 'L', 2, 0), \  # IF C
  bbc(0, 592, 32, 'L', 2, 0), \  # IF A
  bbc(1, 592, 32, 'L', 2, 0), \  # IF C
  bbc(0, 560, 32, 'L', 2, 0), \  # IF A
  bbc(1, 560, 32, 'L', 2, 0) \  # IF C
  ]

source0 = Source(0.543579832909951, 0.158191130487435)
source0.setName('J0204+0903')

source1 = Source(0.708979711424031, 0.192280635129234)
source1.setName('J0242+1101')

source2 = Source(0.762318515363372, 0.689864227441385)
source2.setName('J0254+3931')

source3 = Source(0.790942801650617, -0.548691715660016)
source3.setName('J0301-3126')

source4 = Source(0.848880299750432, -0.595642891074781)
source4.setName('J0314-3407')

source5 = Source(0.944667283261964, 0.563883603345626)
source5.setName('J0336+3218')

source6 = Source(0.975126536439036, 0.634778331564468)
source6.setName('J0343+3622')

source7 = Source(0.992623953500464, 0.350486568966089)
source7.setName('J0347+2004')

source8 = Source(1.00671437019772, -0.568101895723423)
source8.setName('J0350-3232')

source9 = Source(1.06332265164643, -0.431719109431222)
source9.setName('J0403-2444')

source10 = Source(1.07426793761681, -0.311240335217893)
source10.setName('J0406-1749')

source11 = Source(1.07587238752769, 0.115555294301863)
source11.setName('J0406+0637')

source12 = Source(1.09702903644613, -0.488211306243707)
source12.setName('J0411-2756')

source13 = Source(1.24365099178722, 0.126797751278368)
source13.setName('J0445+0715')

source14 = Source(1.40667696057376, -0.129510596577481)
source14.setName('J0522-0725')

source15 = Source(1.55121995875874, 0.694879400600691)
source15.setName('DA193')

source16 = Source(0.541922551082674, 0.163044246916812)
source16.setName('J0204+0920')

source17 = Source(0.71733032256967, 0.198940482123948)
source17.setName('J0244+1123')

source18 = Source(0.788128149235776, 0.68109234219872)
source18.setName('J0300+3901')

source19 = Source(0.804854221234055, -0.543564808946065)
source19.setName('J0304-3108')

source20 = Source(0.813703040541666, -0.591532468480515)
source20.setName('J0306-3353')

source21 = Source(0.927078174310175, 0.560178355688958)
source21.setName('J0332+3205')

source22 = Source(0.970326705954072, 0.661337978504126)
source22.setName('J0342+3753')

source23 = Source(1.0126800291358, 0.345290169931753)
source23.setName('J0352+1947')

source24 = Source(1.02452645143371, -0.578419567403441)
source24.setName('J0354-3308')

source25 = Source(1.06491337032486, -0.429351723211127)
source25.setName('J0404-2436')

source26 = Source(1.06762808453223, 0.125757081258573)
source26.setName('J0404+0712')

source27 = Source(1.0892076262921, -0.325201866573611)
source27.setName('J0409-1837')

source28 = Source(1.12530830742856, -0.493612613089361)
source28.setName('J0417-2816')

source29 = Source(1.23718827580453, 0.117315796331303)
source29.setName('J0443+0643')

source30 = Source(1.39232477525013, -0.143504995052527)
source30.setName('J0519-0813')

# Setup Scan 
# changing to mode trdbe.6cm
subarray.setVLBALoIfSetup(dbe0, loif0)
subarray.set4x4Switch('1A', 1)
subarray.set4x4Switch('1B', 3)
subarray.setChannels(dbe0, channelSet0)
subarray.setSource(source0)
# Setup scan - run right away, but do not start recording
subarray.execute( array.time() + 2*second )

# Scan 0 = No0001
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 0*second, mjdStart+239*second, 'No0001', obsCode, stnCode )
if array.time() < mjdStart + (239-10)*second:
  subarray.execute(mjdStart + 234*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+239*second) + ' since array.time is ' + str(array.time())

# Scan 1 = No0002
subarray.setSource(source16)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 247*second, mjdStart+539*second, 'No0002', obsCode, stnCode )
if array.time() < mjdStart + (539-10)*second:
  subarray.execute(mjdStart + 534*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+539*second) + ' since array.time is ' + str(array.time())

# Scan 2 = No0003
subarray.setSource(source0)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 547*second, mjdStart+598*second, 'No0003', obsCode, stnCode )
if array.time() < mjdStart + (598-10)*second:
  subarray.execute(mjdStart + 593*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+598*second) + ' since array.time is ' + str(array.time())

# Scan 3 = No0004
# Antenna MK not in scan No0004

# Scan 4 = No0005
# Antenna MK not in scan No0005

# Scan 5 = No0006
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 987*second, mjdStart+1047*second, 'No0006', obsCode, stnCode )
if array.time() < mjdStart + (1047-10)*second:
  subarray.execute(mjdStart + 1042*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1047*second) + ' since array.time is ' + str(array.time())

# Scan 6 = No0007
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1074*second, mjdStart+1167*second, 'No0007', obsCode, stnCode )
if array.time() < mjdStart + (1167-10)*second:
  subarray.execute(mjdStart + 1162*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1167*second) + ' since array.time is ' + str(array.time())

# Scan 7 = No0008
subarray.setSource(source18)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1177*second, mjdStart+1466*second, 'No0008', obsCode, stnCode )
if array.time() < mjdStart + (1466-10)*second:
  subarray.execute(mjdStart + 1461*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1466*second) + ' since array.time is ' + str(array.time())

# Scan 8 = No0009
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1477*second, mjdStart+1526*second, 'No0009', obsCode, stnCode )
if array.time() < mjdStart + (1526-10)*second:
  subarray.execute(mjdStart + 1521*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1526*second) + ' since array.time is ' + str(array.time())

# Scan 9 = No0010
# Antenna MK not in scan No0010

# Scan 10 = No0011
subarray.setSource(source22)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 1616*second, mjdStart+1915*second, 'No0011', obsCode, stnCode )
if array.time() < mjdStart + (1915-10)*second:
  subarray.execute(mjdStart + 1910*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+1915*second) + ' since array.time is ' + str(array.time())

# Scan 11 = No0012
# Antenna MK not in scan No0012

# Scan 12 = No0013
# Antenna MK not in scan No0013

# Scan 13 = No0014
subarray.setSource(source21)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2064*second, mjdStart+2364*second, 'No0014', obsCode, stnCode )
if array.time() < mjdStart + (2364-10)*second:
  subarray.execute(mjdStart + 2359*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2364*second) + ' since array.time is ' + str(array.time())

# Scan 14 = No0015
subarray.setSource(source5)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 2373*second, mjdStart+2423*second, 'No0015', obsCode, stnCode )
if array.time() < mjdStart + (2423-10)*second:
  subarray.execute(mjdStart + 2418*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+2423*second) + ' since array.time is ' + str(array.time())

# Scan 15 = No0016
# Antenna MK not in scan No0016

# Scan 16 = No0017
# Antenna MK not in scan No0017

# Scan 17 = No0018
# Antenna MK not in scan No0018

# Scan 18 = No0019
# Antenna MK not in scan No0019

# Scan 19 = No0020
# Antenna MK not in scan No0020

# Scan 20 = No0021
# Antenna MK not in scan No0021

# Scan 21 = No0022
# Antenna MK not in scan No0022

# Scan 22 = No0023
# Antenna MK not in scan No0023

# Scan 23 = No0024
# Antenna MK not in scan No0024

# Scan 24 = No0025
# Antenna MK not in scan No0025

# Scan 25 = No0026
# Antenna MK not in scan No0026

# Scan 26 = No0027
# Antenna MK not in scan No0027

# Scan 27 = No0028
# Antenna MK not in scan No0028

# Scan 28 = No0029
# Antenna MK not in scan No0029

# Scan 29 = No0030
# Antenna MK not in scan No0030

# Scan 30 = No0031
# Antenna MK not in scan No0031

# Scan 31 = No0032
# Antenna MK not in scan No0032

# Scan 32 = No0033
# Antenna MK not in scan No0033

# Scan 33 = No0034
# Antenna MK not in scan No0034

# Scan 34 = No0035
# Antenna MK not in scan No0035

# Scan 35 = No0036
# Antenna MK not in scan No0036

# Scan 36 = No0037
# Antenna MK not in scan No0037

# Scan 37 = No0038
# Antenna MK not in scan No0038

# Scan 38 = No0039
# Antenna MK not in scan No0039

# Scan 39 = No0040
# Antenna MK not in scan No0040

# Scan 40 = No0041
# Antenna MK not in scan No0041

# Scan 41 = No0042
# Antenna MK not in scan No0042

# Scan 42 = No0043
# Antenna MK not in scan No0043

# Scan 43 = No0044
# Antenna MK not in scan No0044

# Scan 44 = No0045
# Antenna MK not in scan No0045

# Scan 45 = No0046
subarray.setSource(source0)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7001*second, mjdStart+7240*second, 'No0046', obsCode, stnCode )
if array.time() < mjdStart + (7240-10)*second:
  subarray.execute(mjdStart + 7235*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7240*second) + ' since array.time is ' + str(array.time())

# Scan 46 = No0047
subarray.setSource(source16)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7248*second, mjdStart+7539*second, 'No0047', obsCode, stnCode )
if array.time() < mjdStart + (7539-10)*second:
  subarray.execute(mjdStart + 7534*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7539*second) + ' since array.time is ' + str(array.time())

# Scan 47 = No0048
subarray.setSource(source0)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7547*second, mjdStart+7599*second, 'No0048', obsCode, stnCode )
if array.time() < mjdStart + (7599-10)*second:
  subarray.execute(mjdStart + 7594*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7599*second) + ' since array.time is ' + str(array.time())

# Scan 48 = No0049
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7624*second, mjdStart+7689*second, 'No0049', obsCode, stnCode )
if array.time() < mjdStart + (7689-10)*second:
  subarray.execute(mjdStart + 7684*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7689*second) + ' since array.time is ' + str(array.time())

# Scan 49 = No0050
subarray.setSource(source17)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7697*second, mjdStart+7988*second, 'No0050', obsCode, stnCode )
if array.time() < mjdStart + (7988-10)*second:
  subarray.execute(mjdStart + 7983*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+7988*second) + ' since array.time is ' + str(array.time())

# Scan 50 = No0051
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 7996*second, mjdStart+8048*second, 'No0051', obsCode, stnCode )
if array.time() < mjdStart + (8048-10)*second:
  subarray.execute(mjdStart + 8043*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8048*second) + ' since array.time is ' + str(array.time())

# Scan 51 = No0052
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8079*second, mjdStart+8168*second, 'No0052', obsCode, stnCode )
if array.time() < mjdStart + (8168-10)*second:
  subarray.execute(mjdStart + 8163*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8168*second) + ' since array.time is ' + str(array.time())

# Scan 52 = No0053
subarray.setSource(source18)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8178*second, mjdStart+8467*second, 'No0053', obsCode, stnCode )
if array.time() < mjdStart + (8467-10)*second:
  subarray.execute(mjdStart + 8462*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8467*second) + ' since array.time is ' + str(array.time())

# Scan 53 = No0054
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8477*second, mjdStart+8527*second, 'No0054', obsCode, stnCode )
if array.time() < mjdStart + (8527-10)*second:
  subarray.execute(mjdStart + 8522*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8527*second) + ' since array.time is ' + str(array.time())

# Scan 54 = No0055
subarray.setSource(source6)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8555*second, mjdStart+8617*second, 'No0055', obsCode, stnCode )
if array.time() < mjdStart + (8617-10)*second:
  subarray.execute(mjdStart + 8612*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8617*second) + ' since array.time is ' + str(array.time())

# Scan 55 = No0056
subarray.setSource(source22)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8626*second, mjdStart+8916*second, 'No0056', obsCode, stnCode )
if array.time() < mjdStart + (8916-10)*second:
  subarray.execute(mjdStart + 8911*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8916*second) + ' since array.time is ' + str(array.time())

# Scan 56 = No0057
subarray.setSource(source6)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8925*second, mjdStart+8976*second, 'No0057', obsCode, stnCode )
if array.time() < mjdStart + (8976-10)*second:
  subarray.execute(mjdStart + 8971*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+8976*second) + ' since array.time is ' + str(array.time())

# Scan 57 = No0058
subarray.setSource(source5)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 8987*second, mjdStart+9065*second, 'No0058', obsCode, stnCode )
if array.time() < mjdStart + (9065-10)*second:
  subarray.execute(mjdStart + 9060*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9065*second) + ' since array.time is ' + str(array.time())

# Scan 58 = No0059
subarray.setSource(source21)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9075*second, mjdStart+9364*second, 'No0059', obsCode, stnCode )
if array.time() < mjdStart + (9364-10)*second:
  subarray.execute(mjdStart + 9359*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9364*second) + ' since array.time is ' + str(array.time())

# Scan 59 = No0060
subarray.setSource(source5)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9374*second, mjdStart+9424*second, 'No0060', obsCode, stnCode )
if array.time() < mjdStart + (9424-10)*second:
  subarray.execute(mjdStart + 9419*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9424*second) + ' since array.time is ' + str(array.time())

# Scan 60 = No0061
subarray.setSource(source7)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9442*second, mjdStart+9514*second, 'No0061', obsCode, stnCode )
if array.time() < mjdStart + (9514-10)*second:
  subarray.execute(mjdStart + 9509*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9514*second) + ' since array.time is ' + str(array.time())

# Scan 61 = No0062
subarray.setSource(source23)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9524*second, mjdStart+9813*second, 'No0062', obsCode, stnCode )
if array.time() < mjdStart + (9813-10)*second:
  subarray.execute(mjdStart + 9808*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9813*second) + ' since array.time is ' + str(array.time())

# Scan 62 = No0063
subarray.setSource(source7)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9823*second, mjdStart+9873*second, 'No0063', obsCode, stnCode )
if array.time() < mjdStart + (9873-10)*second:
  subarray.execute(mjdStart + 9868*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+9873*second) + ' since array.time is ' + str(array.time())

# Scan 63 = No0064
subarray.setSource(source3)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 9919*second, mjdStart+10023*second, 'No0064', obsCode, stnCode )
if array.time() < mjdStart + (10023-10)*second:
  subarray.execute(mjdStart + 10018*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10023*second) + ' since array.time is ' + str(array.time())

# Scan 64 = No0065
subarray.setSource(source19)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10032*second, mjdStart+10322*second, 'No0065', obsCode, stnCode )
if array.time() < mjdStart + (10322-10)*second:
  subarray.execute(mjdStart + 10317*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10322*second) + ' since array.time is ' + str(array.time())

# Scan 65 = No0066
subarray.setSource(source3)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10331*second, mjdStart+10382*second, 'No0066', obsCode, stnCode )
if array.time() < mjdStart + (10382-10)*second:
  subarray.execute(mjdStart + 10377*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10382*second) + ' since array.time is ' + str(array.time())

# Scan 66 = No0067
subarray.setSource(source4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10398*second, mjdStart+10471*second, 'No0067', obsCode, stnCode )
if array.time() < mjdStart + (10471-10)*second:
  subarray.execute(mjdStart + 10466*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10471*second) + ' since array.time is ' + str(array.time())

# Scan 67 = No0068
subarray.setSource(source20)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10482*second, mjdStart+10771*second, 'No0068', obsCode, stnCode )
if array.time() < mjdStart + (10771-10)*second:
  subarray.execute(mjdStart + 10766*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10771*second) + ' since array.time is ' + str(array.time())

# Scan 68 = No0069
subarray.setSource(source4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10782*second, mjdStart+10830*second, 'No0069', obsCode, stnCode )
if array.time() < mjdStart + (10830-10)*second:
  subarray.execute(mjdStart + 10825*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10830*second) + ' since array.time is ' + str(array.time())

# Scan 69 = No0070
subarray.setSource(source8)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10850*second, mjdStart+10920*second, 'No0070', obsCode, stnCode )
if array.time() < mjdStart + (10920-10)*second:
  subarray.execute(mjdStart + 10915*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+10920*second) + ' since array.time is ' + str(array.time())

# Scan 70 = No0071
subarray.setSource(source24)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 10930*second, mjdStart+11219*second, 'No0071', obsCode, stnCode )
if array.time() < mjdStart + (11219-10)*second:
  subarray.execute(mjdStart + 11214*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11219*second) + ' since array.time is ' + str(array.time())

# Scan 71 = No0072
subarray.setSource(source8)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11229*second, mjdStart+11279*second, 'No0072', obsCode, stnCode )
if array.time() < mjdStart + (11279-10)*second:
  subarray.execute(mjdStart + 11274*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11279*second) + ' since array.time is ' + str(array.time())

# Scan 72 = No0073
subarray.setSource(source9)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11293*second, mjdStart+11369*second, 'No0073', obsCode, stnCode )
if array.time() < mjdStart + (11369-10)*second:
  subarray.execute(mjdStart + 11364*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11369*second) + ' since array.time is ' + str(array.time())

# Scan 73 = No0074
subarray.setSource(source25)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11376*second, mjdStart+11668*second, 'No0074', obsCode, stnCode )
if array.time() < mjdStart + (11668-10)*second:
  subarray.execute(mjdStart + 11663*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11668*second) + ' since array.time is ' + str(array.time())

# Scan 74 = No0075
subarray.setSource(source9)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11675*second, mjdStart+11728*second, 'No0075', obsCode, stnCode )
if array.time() < mjdStart + (11728-10)*second:
  subarray.execute(mjdStart + 11723*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11728*second) + ' since array.time is ' + str(array.time())

# Scan 75 = No0076
subarray.setSource(source12)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11742*second, mjdStart+11818*second, 'No0076', obsCode, stnCode )
if array.time() < mjdStart + (11818-10)*second:
  subarray.execute(mjdStart + 11813*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+11818*second) + ' since array.time is ' + str(array.time())

# Scan 76 = No0077
subarray.setSource(source28)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 11829*second, mjdStart+12117*second, 'No0077', obsCode, stnCode )
if array.time() < mjdStart + (12117-10)*second:
  subarray.execute(mjdStart + 12112*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12117*second) + ' since array.time is ' + str(array.time())

# Scan 77 = No0078
subarray.setSource(source12)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12128*second, mjdStart+12177*second, 'No0078', obsCode, stnCode )
if array.time() < mjdStart + (12177-10)*second:
  subarray.execute(mjdStart + 12172*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12177*second) + ' since array.time is ' + str(array.time())

# Scan 78 = No0079
subarray.setSource(source10)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12197*second, mjdStart+12267*second, 'No0079', obsCode, stnCode )
if array.time() < mjdStart + (12267-10)*second:
  subarray.execute(mjdStart + 12262*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12267*second) + ' since array.time is ' + str(array.time())

# Scan 79 = No0080
subarray.setSource(source27)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12277*second, mjdStart+12566*second, 'No0080', obsCode, stnCode )
if array.time() < mjdStart + (12566-10)*second:
  subarray.execute(mjdStart + 12561*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12566*second) + ' since array.time is ' + str(array.time())

# Scan 80 = No0081
subarray.setSource(source10)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12576*second, mjdStart+12626*second, 'No0081', obsCode, stnCode )
if array.time() < mjdStart + (12626-10)*second:
  subarray.execute(mjdStart + 12621*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12626*second) + ' since array.time is ' + str(array.time())

# Scan 81 = No0082
subarray.setSource(source11)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12655*second, mjdStart+12745*second, 'No0082', obsCode, stnCode )
if array.time() < mjdStart + (12745-10)*second:
  subarray.execute(mjdStart + 12740*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+12745*second) + ' since array.time is ' + str(array.time())

# Scan 82 = No0083
subarray.setSource(source26)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 12754*second, mjdStart+13044*second, 'No0083', obsCode, stnCode )
if array.time() < mjdStart + (13044-10)*second:
  subarray.execute(mjdStart + 13039*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13044*second) + ' since array.time is ' + str(array.time())

# Scan 83 = No0084
subarray.setSource(source11)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13053*second, mjdStart+13104*second, 'No0084', obsCode, stnCode )
if array.time() < mjdStart + (13104-10)*second:
  subarray.execute(mjdStart + 13099*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13104*second) + ' since array.time is ' + str(array.time())

# Scan 84 = No0085
subarray.setSource(source13)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13130*second, mjdStart+13194*second, 'No0085', obsCode, stnCode )
if array.time() < mjdStart + (13194-10)*second:
  subarray.execute(mjdStart + 13189*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13194*second) + ' since array.time is ' + str(array.time())

# Scan 85 = No0086
subarray.setSource(source29)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13202*second, mjdStart+13493*second, 'No0086', obsCode, stnCode )
if array.time() < mjdStart + (13493-10)*second:
  subarray.execute(mjdStart + 13488*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13493*second) + ' since array.time is ' + str(array.time())

# Scan 86 = No0087
subarray.setSource(source13)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13501*second, mjdStart+13553*second, 'No0087', obsCode, stnCode )
if array.time() < mjdStart + (13553-10)*second:
  subarray.execute(mjdStart + 13548*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13553*second) + ' since array.time is ' + str(array.time())

# Scan 87 = No0088
subarray.setSource(source14)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13590*second, mjdStart+13643*second, 'No0088', obsCode, stnCode )
if array.time() < mjdStart + (13643-10)*second:
  subarray.execute(mjdStart + 13638*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13643*second) + ' since array.time is ' + str(array.time())

# Scan 88 = No0089
subarray.setSource(source30)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13652*second, mjdStart+13942*second, 'No0089', obsCode, stnCode )
if array.time() < mjdStart + (13942-10)*second:
  subarray.execute(mjdStart + 13937*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+13942*second) + ' since array.time is ' + str(array.time())

# Scan 89 = No0090
subarray.setSource(source14)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 13951*second, mjdStart+14002*second, 'No0090', obsCode, stnCode )
if array.time() < mjdStart + (14002-10)*second:
  subarray.execute(mjdStart + 13997*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14002*second) + ' since array.time is ' + str(array.time())

# Scan 90 = No0091
subarray.setSource(source15)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14044*second, mjdStart+14241*second, 'No0091', obsCode, stnCode )
if array.time() < mjdStart + (14241-10)*second:
  subarray.execute(mjdStart + 14236*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14241*second) + ' since array.time is ' + str(array.time())

# Scan 91 = No0092
subarray.setSource(source0)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14346*second, mjdStart+14480*second, 'No0092', obsCode, stnCode )
if array.time() < mjdStart + (14480-10)*second:
  subarray.execute(mjdStart + 14475*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14480*second) + ' since array.time is ' + str(array.time())

# Scan 92 = No0093
subarray.setSource(source16)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14488*second, mjdStart+14780*second, 'No0093', obsCode, stnCode )
if array.time() < mjdStart + (14780-10)*second:
  subarray.execute(mjdStart + 14775*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14780*second) + ' since array.time is ' + str(array.time())

# Scan 93 = No0094
subarray.setSource(source0)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14788*second, mjdStart+14840*second, 'No0094', obsCode, stnCode )
if array.time() < mjdStart + (14840-10)*second:
  subarray.execute(mjdStart + 14835*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14840*second) + ' since array.time is ' + str(array.time())

# Scan 94 = No0095
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14864*second, mjdStart+14929*second, 'No0095', obsCode, stnCode )
if array.time() < mjdStart + (14929-10)*second:
  subarray.execute(mjdStart + 14924*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+14929*second) + ' since array.time is ' + str(array.time())

# Scan 95 = No0096
subarray.setSource(source17)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 14937*second, mjdStart+15228*second, 'No0096', obsCode, stnCode )
if array.time() < mjdStart + (15228-10)*second:
  subarray.execute(mjdStart + 15223*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15228*second) + ' since array.time is ' + str(array.time())

# Scan 96 = No0097
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15236*second, mjdStart+15288*second, 'No0097', obsCode, stnCode )
if array.time() < mjdStart + (15288-10)*second:
  subarray.execute(mjdStart + 15283*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15288*second) + ' since array.time is ' + str(array.time())

# Scan 97 = No0098
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15332*second, mjdStart+15408*second, 'No0098', obsCode, stnCode )
if array.time() < mjdStart + (15408-10)*second:
  subarray.execute(mjdStart + 15403*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15408*second) + ' since array.time is ' + str(array.time())

# Scan 98 = No0099
subarray.setSource(source18)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15418*second, mjdStart+15707*second, 'No0099', obsCode, stnCode )
if array.time() < mjdStart + (15707-10)*second:
  subarray.execute(mjdStart + 15702*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15707*second) + ' since array.time is ' + str(array.time())

# Scan 99 = No0100
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15717*second, mjdStart+15767*second, 'No0100', obsCode, stnCode )
if array.time() < mjdStart + (15767-10)*second:
  subarray.execute(mjdStart + 15762*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15767*second) + ' since array.time is ' + str(array.time())

# Scan 100 = No0101
subarray.setSource(source6)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15792*second, mjdStart+15857*second, 'No0101', obsCode, stnCode )
if array.time() < mjdStart + (15857-10)*second:
  subarray.execute(mjdStart + 15852*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+15857*second) + ' since array.time is ' + str(array.time())

# Scan 101 = No0102
subarray.setSource(source22)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 15866*second, mjdStart+16156*second, 'No0102', obsCode, stnCode )
if array.time() < mjdStart + (16156-10)*second:
  subarray.execute(mjdStart + 16151*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+16156*second) + ' since array.time is ' + str(array.time())

# Scan 102 = No0103
subarray.setSource(source6)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 16166*second, mjdStart+16216*second, 'No0103', obsCode, stnCode )
if array.time() < mjdStart + (16216-10)*second:
  subarray.execute(mjdStart + 16211*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+16216*second) + ' since array.time is ' + str(array.time())

# Scan 103 = No0104
subarray.setSource(source5)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 16228*second, mjdStart+16305*second, 'No0104', obsCode, stnCode )
if array.time() < mjdStart + (16305-10)*second:
  subarray.execute(mjdStart + 16300*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+16305*second) + ' since array.time is ' + str(array.time())

# Scan 104 = No0105
subarray.setSource(source21)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 16315*second, mjdStart+16605*second, 'No0105', obsCode, stnCode )
if array.time() < mjdStart + (16605-10)*second:
  subarray.execute(mjdStart + 16600*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+16605*second) + ' since array.time is ' + str(array.time())

# Scan 105 = No0106
subarray.setSource(source5)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 16615*second, mjdStart+16665*second, 'No0106', obsCode, stnCode )
if array.time() < mjdStart + (16665-10)*second:
  subarray.execute(mjdStart + 16660*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+16665*second) + ' since array.time is ' + str(array.time())

# Scan 106 = No0107
subarray.setSource(source7)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 16687*second, mjdStart+16754*second, 'No0107', obsCode, stnCode )
if array.time() < mjdStart + (16754-10)*second:
  subarray.execute(mjdStart + 16749*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+16754*second) + ' since array.time is ' + str(array.time())

# Scan 107 = No0108
subarray.setSource(source23)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 16764*second, mjdStart+17053*second, 'No0108', obsCode, stnCode )
if array.time() < mjdStart + (17053-10)*second:
  subarray.execute(mjdStart + 17048*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+17053*second) + ' since array.time is ' + str(array.time())

# Scan 108 = No0109
subarray.setSource(source7)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 17063*second, mjdStart+17113*second, 'No0109', obsCode, stnCode )
if array.time() < mjdStart + (17113-10)*second:
  subarray.execute(mjdStart + 17108*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+17113*second) + ' since array.time is ' + str(array.time())

# Scan 109 = No0110
subarray.setSource(source3)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 17169*second, mjdStart+17263*second, 'No0110', obsCode, stnCode )
if array.time() < mjdStart + (17263-10)*second:
  subarray.execute(mjdStart + 17258*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+17263*second) + ' since array.time is ' + str(array.time())

# Scan 110 = No0111
subarray.setSource(source19)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 17271*second, mjdStart+17562*second, 'No0111', obsCode, stnCode )
if array.time() < mjdStart + (17562-10)*second:
  subarray.execute(mjdStart + 17557*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+17562*second) + ' since array.time is ' + str(array.time())

# Scan 111 = No0112
subarray.setSource(source3)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 17570*second, mjdStart+17622*second, 'No0112', obsCode, stnCode )
if array.time() < mjdStart + (17622-10)*second:
  subarray.execute(mjdStart + 17617*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+17622*second) + ' since array.time is ' + str(array.time())

# Scan 112 = No0113
subarray.setSource(source4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 17638*second, mjdStart+17712*second, 'No0113', obsCode, stnCode )
if array.time() < mjdStart + (17712-10)*second:
  subarray.execute(mjdStart + 17707*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+17712*second) + ' since array.time is ' + str(array.time())

# Scan 113 = No0114
subarray.setSource(source20)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 17722*second, mjdStart+18011*second, 'No0114', obsCode, stnCode )
if array.time() < mjdStart + (18011-10)*second:
  subarray.execute(mjdStart + 18006*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+18011*second) + ' since array.time is ' + str(array.time())

# Scan 114 = No0115
subarray.setSource(source4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 18021*second, mjdStart+18071*second, 'No0115', obsCode, stnCode )
if array.time() < mjdStart + (18071-10)*second:
  subarray.execute(mjdStart + 18066*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+18071*second) + ' since array.time is ' + str(array.time())

# Scan 115 = No0116
subarray.setSource(source8)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 18085*second, mjdStart+18160*second, 'No0116', obsCode, stnCode )
if array.time() < mjdStart + (18160-10)*second:
  subarray.execute(mjdStart + 18155*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+18160*second) + ' since array.time is ' + str(array.time())

# Scan 116 = No0117
subarray.setSource(source24)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 18170*second, mjdStart+18460*second, 'No0117', obsCode, stnCode )
if array.time() < mjdStart + (18460-10)*second:
  subarray.execute(mjdStart + 18455*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+18460*second) + ' since array.time is ' + str(array.time())

# Scan 117 = No0118
subarray.setSource(source8)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 18470*second, mjdStart+18519*second, 'No0118', obsCode, stnCode )
if array.time() < mjdStart + (18519-10)*second:
  subarray.execute(mjdStart + 18514*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+18519*second) + ' since array.time is ' + str(array.time())

# Scan 118 = No0119
subarray.setSource(source9)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 18536*second, mjdStart+18609*second, 'No0119', obsCode, stnCode )
if array.time() < mjdStart + (18609-10)*second:
  subarray.execute(mjdStart + 18604*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+18609*second) + ' since array.time is ' + str(array.time())

# Scan 119 = No0120
subarray.setSource(source25)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 18616*second, mjdStart+18908*second, 'No0120', obsCode, stnCode )
if array.time() < mjdStart + (18908-10)*second:
  subarray.execute(mjdStart + 18903*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+18908*second) + ' since array.time is ' + str(array.time())

# Scan 120 = No0121
subarray.setSource(source9)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 18915*second, mjdStart+18968*second, 'No0121', obsCode, stnCode )
if array.time() < mjdStart + (18968-10)*second:
  subarray.execute(mjdStart + 18963*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+18968*second) + ' since array.time is ' + str(array.time())

# Scan 121 = No0122
subarray.setSource(source12)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 18983*second, mjdStart+19058*second, 'No0122', obsCode, stnCode )
if array.time() < mjdStart + (19058-10)*second:
  subarray.execute(mjdStart + 19053*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+19058*second) + ' since array.time is ' + str(array.time())

# Scan 122 = No0123
subarray.setSource(source28)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 19068*second, mjdStart+19357*second, 'No0123', obsCode, stnCode )
if array.time() < mjdStart + (19357-10)*second:
  subarray.execute(mjdStart + 19352*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+19357*second) + ' since array.time is ' + str(array.time())

# Scan 123 = No0124
subarray.setSource(source12)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 19367*second, mjdStart+19417*second, 'No0124', obsCode, stnCode )
if array.time() < mjdStart + (19417-10)*second:
  subarray.execute(mjdStart + 19412*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+19417*second) + ' since array.time is ' + str(array.time())

# Scan 124 = No0125
subarray.setSource(source10)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 19443*second, mjdStart+19507*second, 'No0125', obsCode, stnCode )
if array.time() < mjdStart + (19507-10)*second:
  subarray.execute(mjdStart + 19502*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+19507*second) + ' since array.time is ' + str(array.time())

# Scan 125 = No0126
subarray.setSource(source27)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 19517*second, mjdStart+19806*second, 'No0126', obsCode, stnCode )
if array.time() < mjdStart + (19806-10)*second:
  subarray.execute(mjdStart + 19801*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+19806*second) + ' since array.time is ' + str(array.time())

# Scan 126 = No0127
subarray.setSource(source10)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 19816*second, mjdStart+19866*second, 'No0127', obsCode, stnCode )
if array.time() < mjdStart + (19866-10)*second:
  subarray.execute(mjdStart + 19861*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+19866*second) + ' since array.time is ' + str(array.time())

# Scan 127 = No0128
subarray.setSource(source11)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 19905*second, mjdStart+19985*second, 'No0128', obsCode, stnCode )
if array.time() < mjdStart + (19985-10)*second:
  subarray.execute(mjdStart + 19980*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+19985*second) + ' since array.time is ' + str(array.time())

# Scan 128 = No0129
subarray.setSource(source26)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 19994*second, mjdStart+20285*second, 'No0129', obsCode, stnCode )
if array.time() < mjdStart + (20285-10)*second:
  subarray.execute(mjdStart + 20280*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+20285*second) + ' since array.time is ' + str(array.time())

# Scan 129 = No0130
subarray.setSource(source11)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 20294*second, mjdStart+20344*second, 'No0130', obsCode, stnCode )
if array.time() < mjdStart + (20344-10)*second:
  subarray.execute(mjdStart + 20339*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+20344*second) + ' since array.time is ' + str(array.time())

# Scan 130 = No0131
subarray.setSource(source13)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 20370*second, mjdStart+20434*second, 'No0131', obsCode, stnCode )
if array.time() < mjdStart + (20434-10)*second:
  subarray.execute(mjdStart + 20429*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+20434*second) + ' since array.time is ' + str(array.time())

# Scan 131 = No0132
subarray.setSource(source29)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 20442*second, mjdStart+20733*second, 'No0132', obsCode, stnCode )
if array.time() < mjdStart + (20733-10)*second:
  subarray.execute(mjdStart + 20728*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+20733*second) + ' since array.time is ' + str(array.time())

# Scan 132 = No0133
subarray.setSource(source13)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 20741*second, mjdStart+20793*second, 'No0133', obsCode, stnCode )
if array.time() < mjdStart + (20793-10)*second:
  subarray.execute(mjdStart + 20788*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+20793*second) + ' since array.time is ' + str(array.time())

# Scan 133 = No0134
subarray.setSource(source14)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 20832*second, mjdStart+20883*second, 'No0134', obsCode, stnCode )
if array.time() < mjdStart + (20883-10)*second:
  subarray.execute(mjdStart + 20878*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+20883*second) + ' since array.time is ' + str(array.time())

# Scan 134 = No0135
subarray.setSource(source30)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 20892*second, mjdStart+21182*second, 'No0135', obsCode, stnCode )
if array.time() < mjdStart + (21182-10)*second:
  subarray.execute(mjdStart + 21177*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+21182*second) + ' since array.time is ' + str(array.time())

# Scan 135 = No0136
subarray.setSource(source14)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 21191*second, mjdStart+21242*second, 'No0136', obsCode, stnCode )
if array.time() < mjdStart + (21242-10)*second:
  subarray.execute(mjdStart + 21237*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+21242*second) + ' since array.time is ' + str(array.time())

# Scan 136 = No0137
subarray.setSource(source0)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 21340*second, mjdStart+21481*second, 'No0137', obsCode, stnCode )
if array.time() < mjdStart + (21481-10)*second:
  subarray.execute(mjdStart + 21476*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+21481*second) + ' since array.time is ' + str(array.time())

# Scan 137 = No0138
subarray.setSource(source16)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 21489*second, mjdStart+21780*second, 'No0138', obsCode, stnCode )
if array.time() < mjdStart + (21780-10)*second:
  subarray.execute(mjdStart + 21775*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+21780*second) + ' since array.time is ' + str(array.time())

# Scan 138 = No0139
subarray.setSource(source0)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 21788*second, mjdStart+21840*second, 'No0139', obsCode, stnCode )
if array.time() < mjdStart + (21840-10)*second:
  subarray.execute(mjdStart + 21835*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+21840*second) + ' since array.time is ' + str(array.time())

# Scan 139 = No0140
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 21883*second, mjdStart+21930*second, 'No0140', obsCode, stnCode )
if array.time() < mjdStart + (21930-10)*second:
  subarray.execute(mjdStart + 21925*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+21930*second) + ' since array.time is ' + str(array.time())

# Scan 140 = No0141
subarray.setSource(source17)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 21940*second, mjdStart+22229*second, 'No0141', obsCode, stnCode )
if array.time() < mjdStart + (22229-10)*second:
  subarray.execute(mjdStart + 22224*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+22229*second) + ' since array.time is ' + str(array.time())

# Scan 141 = No0142
subarray.setSource(source1)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 22240*second, mjdStart+22289*second, 'No0142', obsCode, stnCode )
if array.time() < mjdStart + (22289-10)*second:
  subarray.execute(mjdStart + 22284*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+22289*second) + ' since array.time is ' + str(array.time())

# Scan 142 = No0143
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 22402*second, mjdStart+22409*second, 'No0143', obsCode, stnCode )
if array.time() < mjdStart + (22409-10)*second:
  subarray.execute(mjdStart + 22404*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+22409*second) + ' since array.time is ' + str(array.time())

# Scan 143 = No0144
subarray.setSource(source18)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 22419*second, mjdStart+22708*second, 'No0144', obsCode, stnCode )
if array.time() < mjdStart + (22708-10)*second:
  subarray.execute(mjdStart + 22703*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+22708*second) + ' since array.time is ' + str(array.time())

# Scan 144 = No0145
subarray.setSource(source2)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 22718*second, mjdStart+22768*second, 'No0145', obsCode, stnCode )
if array.time() < mjdStart + (22768-10)*second:
  subarray.execute(mjdStart + 22763*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+22768*second) + ' since array.time is ' + str(array.time())

# Scan 145 = No0146
subarray.setSource(source6)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 22795*second, mjdStart+22858*second, 'No0146', obsCode, stnCode )
if array.time() < mjdStart + (22858-10)*second:
  subarray.execute(mjdStart + 22853*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+22858*second) + ' since array.time is ' + str(array.time())

# Scan 146 = No0147
subarray.setSource(source22)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 22868*second, mjdStart+23157*second, 'No0147', obsCode, stnCode )
if array.time() < mjdStart + (23157-10)*second:
  subarray.execute(mjdStart + 23152*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+23157*second) + ' since array.time is ' + str(array.time())

# Scan 147 = No0148
subarray.setSource(source6)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 23167*second, mjdStart+23217*second, 'No0148', obsCode, stnCode )
if array.time() < mjdStart + (23217-10)*second:
  subarray.execute(mjdStart + 23212*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+23217*second) + ' since array.time is ' + str(array.time())

# Scan 148 = No0149
subarray.setSource(source5)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 23233*second, mjdStart+23306*second, 'No0149', obsCode, stnCode )
if array.time() < mjdStart + (23306-10)*second:
  subarray.execute(mjdStart + 23301*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+23306*second) + ' since array.time is ' + str(array.time())

# Scan 149 = No0150
subarray.setSource(source21)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 23315*second, mjdStart+23605*second, 'No0150', obsCode, stnCode )
if array.time() < mjdStart + (23605-10)*second:
  subarray.execute(mjdStart + 23600*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+23605*second) + ' since array.time is ' + str(array.time())

# Scan 150 = No0151
subarray.setSource(source5)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 23614*second, mjdStart+23665*second, 'No0151', obsCode, stnCode )
if array.time() < mjdStart + (23665-10)*second:
  subarray.execute(mjdStart + 23660*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+23665*second) + ' since array.time is ' + str(array.time())

# Scan 151 = No0152
subarray.setSource(source7)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 23709*second, mjdStart+23755*second, 'No0152', obsCode, stnCode )
if array.time() < mjdStart + (23755-10)*second:
  subarray.execute(mjdStart + 23750*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+23755*second) + ' since array.time is ' + str(array.time())

# Scan 152 = No0153
subarray.setSource(source23)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 23765*second, mjdStart+24054*second, 'No0153', obsCode, stnCode )
if array.time() < mjdStart + (24054-10)*second:
  subarray.execute(mjdStart + 24049*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+24054*second) + ' since array.time is ' + str(array.time())

# Scan 153 = No0154
subarray.setSource(source7)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 24064*second, mjdStart+24114*second, 'No0154', obsCode, stnCode )
if array.time() < mjdStart + (24114-10)*second:
  subarray.execute(mjdStart + 24109*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+24114*second) + ' since array.time is ' + str(array.time())

# Scan 154 = No0155
subarray.setSource(source3)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 24208*second, mjdStart+24264*second, 'No0155', obsCode, stnCode )
if array.time() < mjdStart + (24264-10)*second:
  subarray.execute(mjdStart + 24259*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+24264*second) + ' since array.time is ' + str(array.time())

# Scan 155 = No0156
subarray.setSource(source19)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 24272*second, mjdStart+24563*second, 'No0156', obsCode, stnCode )
if array.time() < mjdStart + (24563-10)*second:
  subarray.execute(mjdStart + 24558*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+24563*second) + ' since array.time is ' + str(array.time())

# Scan 156 = No0157
subarray.setSource(source3)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 24571*second, mjdStart+24623*second, 'No0157', obsCode, stnCode )
if array.time() < mjdStart + (24623-10)*second:
  subarray.execute(mjdStart + 24618*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+24623*second) + ' since array.time is ' + str(array.time())

# Scan 157 = No0158
subarray.setSource(source4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 24636*second, mjdStart+24712*second, 'No0158', obsCode, stnCode )
if array.time() < mjdStart + (24712-10)*second:
  subarray.execute(mjdStart + 24707*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+24712*second) + ' since array.time is ' + str(array.time())

# Scan 158 = No0159
subarray.setSource(source20)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 24721*second, mjdStart+25012*second, 'No0159', obsCode, stnCode )
if array.time() < mjdStart + (25012-10)*second:
  subarray.execute(mjdStart + 25007*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+25012*second) + ' since array.time is ' + str(array.time())

# Scan 159 = No0160
subarray.setSource(source4)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 25021*second, mjdStart+25071*second, 'No0160', obsCode, stnCode )
if array.time() < mjdStart + (25071-10)*second:
  subarray.execute(mjdStart + 25066*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+25071*second) + ' since array.time is ' + str(array.time())

# Scan 160 = No0161
subarray.setSource(source8)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 25085*second, mjdStart+25161*second, 'No0161', obsCode, stnCode )
if array.time() < mjdStart + (25161-10)*second:
  subarray.execute(mjdStart + 25156*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+25161*second) + ' since array.time is ' + str(array.time())

# Scan 161 = No0162
subarray.setSource(source24)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 25170*second, mjdStart+25460*second, 'No0162', obsCode, stnCode )
if array.time() < mjdStart + (25460-10)*second:
  subarray.execute(mjdStart + 25455*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+25460*second) + ' since array.time is ' + str(array.time())

# Scan 162 = No0163
subarray.setSource(source8)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 25469*second, mjdStart+25520*second, 'No0163', obsCode, stnCode )
if array.time() < mjdStart + (25520-10)*second:
  subarray.execute(mjdStart + 25515*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+25520*second) + ' since array.time is ' + str(array.time())

# Scan 163 = No0164
subarray.setSource(source9)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 25543*second, mjdStart+25610*second, 'No0164', obsCode, stnCode )
if array.time() < mjdStart + (25610-10)*second:
  subarray.execute(mjdStart + 25605*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+25610*second) + ' since array.time is ' + str(array.time())

# Scan 164 = No0165
subarray.setSource(source25)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 25617*second, mjdStart+25909*second, 'No0165', obsCode, stnCode )
if array.time() < mjdStart + (25909-10)*second:
  subarray.execute(mjdStart + 25904*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+25909*second) + ' since array.time is ' + str(array.time())

# Scan 165 = No0166
subarray.setSource(source9)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 25916*second, mjdStart+25969*second, 'No0166', obsCode, stnCode )
if array.time() < mjdStart + (25969-10)*second:
  subarray.execute(mjdStart + 25964*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+25969*second) + ' since array.time is ' + str(array.time())

# Scan 166 = No0167
subarray.setSource(source12)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 25984*second, mjdStart+26059*second, 'No0167', obsCode, stnCode )
if array.time() < mjdStart + (26059-10)*second:
  subarray.execute(mjdStart + 26054*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+26059*second) + ' since array.time is ' + str(array.time())

# Scan 167 = No0168
subarray.setSource(source28)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 26068*second, mjdStart+26358*second, 'No0168', obsCode, stnCode )
if array.time() < mjdStart + (26358-10)*second:
  subarray.execute(mjdStart + 26353*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+26358*second) + ' since array.time is ' + str(array.time())

# Scan 168 = No0169
subarray.setSource(source12)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 26367*second, mjdStart+26418*second, 'No0169', obsCode, stnCode )
if array.time() < mjdStart + (26418-10)*second:
  subarray.execute(mjdStart + 26413*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+26418*second) + ' since array.time is ' + str(array.time())

# Scan 169 = No0170
subarray.setSource(source10)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 26448*second, mjdStart+26508*second, 'No0170', obsCode, stnCode )
if array.time() < mjdStart + (26508-10)*second:
  subarray.execute(mjdStart + 26503*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+26508*second) + ' since array.time is ' + str(array.time())

# Scan 170 = No0171
subarray.setSource(source27)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 26518*second, mjdStart+26807*second, 'No0171', obsCode, stnCode )
if array.time() < mjdStart + (26807-10)*second:
  subarray.execute(mjdStart + 26802*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+26807*second) + ' since array.time is ' + str(array.time())

# Scan 171 = No0172
subarray.setSource(source10)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 26817*second, mjdStart+26867*second, 'No0172', obsCode, stnCode )
if array.time() < mjdStart + (26867-10)*second:
  subarray.execute(mjdStart + 26862*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+26867*second) + ' since array.time is ' + str(array.time())

# Scan 172 = No0173
subarray.setSource(source11)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 26926*second, mjdStart+26986*second, 'No0173', obsCode, stnCode )
if array.time() < mjdStart + (26986-10)*second:
  subarray.execute(mjdStart + 26981*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+26986*second) + ' since array.time is ' + str(array.time())

# Scan 173 = No0174
subarray.setSource(source26)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 26995*second, mjdStart+27285*second, 'No0174', obsCode, stnCode )
if array.time() < mjdStart + (27285-10)*second:
  subarray.execute(mjdStart + 27280*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+27285*second) + ' since array.time is ' + str(array.time())

# Scan 174 = No0175
subarray.setSource(source11)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 27294*second, mjdStart+27345*second, 'No0175', obsCode, stnCode )
if array.time() < mjdStart + (27345-10)*second:
  subarray.execute(mjdStart + 27340*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+27345*second) + ' since array.time is ' + str(array.time())

# Scan 175 = No0176
subarray.setSource(source13)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 27376*second, mjdStart+27435*second, 'No0176', obsCode, stnCode )
if array.time() < mjdStart + (27435-10)*second:
  subarray.execute(mjdStart + 27430*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+27435*second) + ' since array.time is ' + str(array.time())

# Scan 176 = No0177
subarray.setSource(source29)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 27444*second, mjdStart+27734*second, 'No0177', obsCode, stnCode )
if array.time() < mjdStart + (27734-10)*second:
  subarray.execute(mjdStart + 27729*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+27734*second) + ' since array.time is ' + str(array.time())

# Scan 177 = No0178
subarray.setSource(source13)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 27743*second, mjdStart+27794*second, 'No0178', obsCode, stnCode )
if array.time() < mjdStart + (27794-10)*second:
  subarray.execute(mjdStart + 27789*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+27794*second) + ' since array.time is ' + str(array.time())

# Scan 178 = No0179
subarray.setSource(source14)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 27839*second, mjdStart+27884*second, 'No0179', obsCode, stnCode )
if array.time() < mjdStart + (27884-10)*second:
  subarray.execute(mjdStart + 27879*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+27884*second) + ' since array.time is ' + str(array.time())

# Scan 179 = No0180
subarray.setSource(source30)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 27893*second, mjdStart+28183*second, 'No0180', obsCode, stnCode )
if array.time() < mjdStart + (28183-10)*second:
  subarray.execute(mjdStart + 28178*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+28183*second) + ' since array.time is ' + str(array.time())

# Scan 180 = No0181
subarray.setSource(source14)
recorder0.setPacket(0, 0, 36, 5008)
subarray.setRecord(mjdStart + 28192*second, mjdStart+28243*second, 'No0181', obsCode, stnCode )
if array.time() < mjdStart + (28243-10)*second:
  subarray.execute(mjdStart + 28238*second)
else:
  print 'Skipping scan which ended at time ' + str(mjdStart+28243*second) + ' since array.time is ' + str(array.time())

array.wait(mjdStart + 28244*second)
