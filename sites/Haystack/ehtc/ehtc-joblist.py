#!/usr/bin/env python
#
# Script to parse a joblist and a vex file and produce
# a variety of things in support of generating EHTC
# correlator tarball products.
#
'''
Script to parse a joblist and a vex file and produce lists of job numbers
'''

import argparse
import glob
import math
import re
import os
import subprocess
import sys
import xml.etree.ElementTree

def parseOptions():
    '''
    Build a parser for command-line options
    '''
    des = 'This script requires a DiFX joblist file, the vex.obs file and '
    des += 'selection criteria; and it produces a list of jobs to process.'
    inp = None
    act = None
    sel = None
    tst = None
    epi = 'For example to generate a report on 3C279 outside ALMA projects, '
    epi += ' try this: '
    epi += ' ehtc-joblist.py -i *.input -o *.vex.obs -p na -s 3C279 -R'
    use = '%(prog)s [options]\n'
    use += '  Version $Id: ehtc-joblist.py 2989 2020-08-06 15:44:03Z gbc $'
    parser = argparse.ArgumentParser(epilog=epi, description=des, usage=use)
    inputs = parser.add_argument_group('input options', inp)
    action = parser.add_argument_group('action options', act)
    select = parser.add_argument_group('selection options', sel)
    tester = parser.add_argument_group('testing options', tst)
    inputs.add_argument('-v', '--verbose', dest='verb',
        action='store_true', default=False,
        help='provide some commentary')
    inputs.add_argument('-j', '--job', dest='job',
        metavar='FILE', default='',
        help='The name of the DiFX job.')
    inputs.add_argument('-l', '--joblist', dest='joblist',
        metavar='FILE', default='',
        help='The path of the difx *.joblist joblist')
    inputs.add_argument('-o', '--vexobs', dest='vexobs',
        metavar='FILE', default='',
        help='The path of the *.vex.obs vex file')
    inputs.add_argument('-i', '--inputs', dest='inputs',
        metavar='FILE-PATTERN', default='',
        help='The path to job input/calc files up to but excluding '
            + 'the underscore preceding the job number.')
    inputs.add_argument('-c', '--codes', dest='codes',
        metavar='FILE', default='',
        help='difx2mark4 station code file augmented with a column'
            + ' of number of polarizations per station')
    inputs.add_argument('-x', '--usev2x', dest='usev2x',
        action='store_true', default=False,
        help='allow use of VEX2XML in input processing, this is a '
            + 'different path to gathering information not yet '
            + 'thoroughly tested and thus should not be used')
    action.add_argument('-A', '--antennas', dest='antennas',
        action='store_true', default=False,
        help='provide a list of antennas')
    action.add_argument('-C', '--scans', dest='scans',
        action='store_true', default=False,
        help='provide a list of scan numbers')
    action.add_argument('-N', '--numbers', dest='numbers',
        action='store_true', default=False,
        help='provide a list of job numbers')
    action.add_argument('-J', '--jobinputs', dest='jobinputs',
        action='store_true', default=False,
        help='provide a list of job input files')
    action.add_argument('-S', '--sources', dest='sources',
        action='store_true', default=False,
        help='provide a list of sources from the SOURCE section')
    action.add_argument('-P', '--projects', dest='projects',
        action='store_true', default=False,
        help='provide a list of ALMA projects from the vex file')
    action.add_argument('-R', '--report', dest='report',
        action='store_true', default=False,
        help='provide a summary list of scans ' +
             'with source, project and antennas')
    action.add_argument('-K', '--check', dest='check',
        action='store_true', default=False,
        help='provide a summary list checking 4fit products')
    action.add_argument('-T', '--timing', dest='timing',
        action='store_true', default=False,
        help='provide timing information on polconversion')
    action.add_argument('-G', '--groups', dest='groups',
        action='store_true', default=False,
        help='provide a summary list of proj/targ/class groups')
    action.add_argument('-L', '--labels', dest='labels',
        action='store_true', default=False,
        help='provide a summary list of proj/targ/class groups and label')
    action.add_argument('-B', '--blprods', dest='blprods',
        action='store_true', default=False,
        help='provide a report on baseline-pol channel products')
    action.add_argument('-F', '--ffconf', dest='ffconf',
        action='store_true', default=False,
        help='provide a report for fourfit channel usage')
    action.add_argument('-D', '--ffdetail', dest='ffdetail',
        action='store_true', default=False,
        help='provide a detailed report for fourfit channel usage')
    select.add_argument('-d', '--difx', dest='difx',
	action='store_true', default=False,
	help='Restrict to scans with data in .difx dir')
    select.add_argument('-s', '--source', dest='source',
        metavar='STRING', default='',
        help='The name of the target source as found in the SOURCE section')
    select.add_argument('-p', '--project', dest='project',
        metavar='STRING', default='',
        help='The name of the ALMA project as declared by intents ' +
            'ALMA:PROJECT_FIRST_SCAN:... and ALMA:PROJECT_FINAL_SCAN:...')
    select.add_argument('-u', '--uniq', dest='uniq',
        action='store_true', default=False,
        help='Restrict jobs to a uniq set of largest job numbers')
    tester.add_argument('-V', '--Vex', dest='vex',
        metavar='VEXTIME', default='',
        help='convert a Vex Time into MJD (and exit)')
    tester.add_argument('-M', '--MJD', dest='mjd',
        metavar='MJDATE', default='',
        help='convert an MJD Time into Vex time (and exit)')
    return parser.parse_args()

def vex2MJD(vex):
    '''
    Convert Vex into MJD.  Crudely J2000 is 51544.0 and years
    divisible by 4 (including 2000) are leap years.  Ok for life of EHTC.
    '''
    dte_re = re.compile(r'(....)y(...)d(..)h(..)m(..)s{0,1}')
    dte = dte_re.search(vex)
    if dte:
        mjd = int((int(dte.group(1)) - 2000) * 365.25 + 0.76) + 51544.0
        mjd += float(int(dte.group(2)) - 1)
        mjd += float(int(dte.group(3))) / 24.0
        mjd += float(int(dte.group(4))) / 1440.0
        mjd += float(int(dte.group(5))) / 86400.0
    return mjd

def MJD2Vex(mjd, verb=False):
    '''
    Convert MJD into Vex, by inverting the above.
    '''
    epoch = float(mjd) - 51544.0
    years = 2000 + int((epoch) / 365.25)
    epoch -= int((years - 2000) * 365.25 + 0.76)
    doy = int(epoch + 1)
    epoch -= doy - 1
    epoch *= 24
    hours = int(epoch)
    epoch -= hours
    epoch *= 60
    mins = int(epoch)
    epoch -= mins
    epoch *= 60
    secs = int(epoch)
    epoch -= secs
    if verb: print '%04d %03d %02d %02d %02d  rem %.9f d' % (
        years, doy, hours, mins, secs, (epoch/86400.0))
    return '%04dy%03dd%02dh%02dm%02ds' % (years, doy, hours, mins, secs)

def doTestVex(vex, verb=False):
    m = vex2MJD(vex)
    print vex, '->', m
    v = MJD2Vex(m, verb)
    print v
    sys.exit(0)
def doTestMJD(mjd, verb=False):
    v = MJD2Vex(mjd, verb)
    print mjd, '->', v
    m = vex2MJD(v)
    print m
    sys.exit(0)

def parseInputCalc(inp, clc, vrb):
    '''
    Read lines of .input and .calc to find the things we want.
    o.jobbage[#] = [start,stop [antennas]]
    o.cabbage[jn] = [MJDstart, MJDstop, [antennas], [vexinfo]]
    vexinfo is [scan,vexstart,mjdstart,sdur,vsrc,mode]
    '''
    jni = inp.split('_')[-1].split('.')[0]
    jnc = clc.split('_')[-1].split('.')[0]
    if len(jni) != len(jnc): return None,None,None
    jid_re = re.compile(r'^JOB ID:\s*([0-9]+)')
    sta_re = re.compile(r'^JOB START TIME:\s*([0-9.]+)')
    stp_re = re.compile(r'^JOB STOP TIME:\s*([0-9.]+)')
    stn_re = re.compile(r'^TELESCOPE\s*([0-9]+)\s*NAME:\s*([A-Z0-9]+)')
    src_re = re.compile(r'^SOURCE\s*([0-9]+)\s*NAME:\s*(.*)$')
    scn_re = re.compile(r'^SCAN\s*([0-9]+)\s*IDENTIFIER:\s*(.*)$')
    dur_re = re.compile(r'^SCAN\s*([0-9]+)\s*DUR.*:\s*([0-9]+)$')
    mde_re = re.compile(r'^SCAN\s*([0-9]+)\s*OBS.*:\s*(.*)$')
    jid = None
    mjdstart = None
    vsrc = None
    scan = None
    sdur = None
    mode = None
    antenniset = set()
    jfmt = '%' + ('0%dd' % len(jni))
    # parse calc file
    fc = open(clc)
    for liner in fc.readlines():
        line = liner.rstrip()
        jid_hit = jid_re.search(line)
        if jid_hit: jid = jfmt % (int(jid_hit.group(1)))
        sta_hit = sta_re.search(line)
        if sta_hit: mjdstart = float(sta_hit.group(1))
        stp_hit = stp_re.search(line)
        if stp_hit: mjdstop  = float(stp_hit.group(1))
        stn_hit = stn_re.search(line)
        if stn_hit: antenniset.add(stn_hit.group(2))
        src_hit = src_re.search(line)
        if src_hit: vsrc = src_hit.group(2)
        scn_hit = scn_re.search(line)
        if scn_hit: scan = scn_hit.group(2)
        dur_hit = dur_re.search(line)
        if dur_hit: sdur = dur_hit.group(2)
        mde_hit = mde_re.search(line)
        if mde_hit: mode = mde_hit.group(2)
    fc.close()
    if mjdstart: vexstart = MJD2Vex(mjdstart, vrb)
    vexinfo = [scan,vexstart,mjdstart,sdur,vsrc,mode]
    if jid != jni or jid != jnc: print '#bogus job',jni,jnc,jid
    answer = [mjdstart, mjdstop, list(antenniset), vexinfo]
    if vrb: print '# ',jid,answer
    return jid,answer,antenniset

def doInputs(o):
    '''
    Use the input pattern to glob for matching input/calc
    files and read them to provide the job information.
    o.jobbage[#] = [start,stop,[antennas],[name,start,smjd,dur,vsrc,mode]]
    o.inputs is non-empty if we were called, but we should check that
    it points to some directory
    '''
    if o.verb: print '# globbing with:', o.inputs + '_*.input'
    dirn = os.path.dirname(o.inputs)
    if dirn == '':
        print '# globbing for files in the current working directory'
    else:
        if not os.path.exists(dirn):
            raise Exception, '-i argument must be set sensibly'
    if o.job == '':
        o.job = os.path.basename(o.inputs)
        if o.verb: print '# set job to', o.job
    o.inptfiles = glob.glob(o.inputs + '_*.input')
    o.calcfiles = glob.glob(o.inputs + '_*.calc')
    if len(o.inptfiles) != len(o.calcfiles):
        print 'Mismatch in number of input/calc files, bailing'
        sys.exit(1)
    if not o.inptfiles:
        print 'No input files matching pattern %s_*.input found! Stopping' % (
            o.inputs)
        sys.exit(1)
    o.pairs = map(lambda x,y:(x,y), sorted(o.inptfiles), sorted(o.calcfiles))
    o.cabbage = {}
    for inp,clc in o.pairs:
        if o.verb: print '#Input:',inp,'\n#Calc: ',clc
        jn,dets,antset = parseInputCalc(inp,clc,o.verb)
        o.antset |= antset
        if dets and jn: o.cabbage[jn] = dets

def parseFirst(line, o):
    '''
    Parse the first line of the job file and gather some info.
    '''
    task = line.split()
    EXPER = task[0].split('=')[1]
    V2D   = task[1].split('=')[1]
    PASS  = task[2].split('=')[1]
    MJD   = task[3].split('=')[1]
    VER   = task[4].split('=')[1]
    V2DV  = task[5].split('=')[1]
    VEX   = task[6].split('=')[1]
    if o.verb:
        print '# Job EXPER %s V2D %s PASS %s MJD %s' % (EXPER, V2D, PASS, MJD)
        print '# DiFX/V2D version is %s/%s' % (VER, V2DV)
        print '# Vexfile is %s' % VEX
    ovp = os.path.abspath(o.vexobs)
    vxp = os.path.abspath(VEX)
    if ovp != vxp and o.verb:
        print '#Warning, this job file refers to a different vexfile:'
        print '# ',ovp
        print '# ',vxp

def doJobList(o):
    '''
    Parse the joblist file and gather useful information
    into a dict, o.jobbage.  After this routine completes,
    o.jobbage[#] = [start,stop [antennas]]
    '''
    if not os.path.exists(o.joblist): return
    if o.verb: print '# examining ',o.joblist
    f = open(o.joblist)
    first = True
    o.jobbage = {}
    for line in f.readlines():
        if line[0] == '#': continue
        if first:
            parseFirst(line.rstrip(), o)
            first = False
        dets = line.rstrip().split()
        if len(dets) > 9:
            JOBNUM = dets[0].split('_')[1]
            MSTART = float(dets[1])
            MSTOP  = float(dets[2])
            ANTENNAS = dets[9:]
            if o.verb:
                print '#Job %s MJD (%.7f..%.7f) antennas %s' % (
                    JOBNUM, MSTART, MSTOP, '-'.join(ANTENNAS))
            o.jobbage[JOBNUM] = [MSTART, MSTOP, ANTENNAS]
            for b in ANTENNAS: o.antennaset.add(b)
    f.close()
    if o.verb:
        print '# Unique antennas: ' + ' '.join(o.antennaset)

def doParseVex(o):
    '''
    Parse the vex obs file and gather useful information
    '''
    if os.path.exists(o.vexobs) and os.path.isfile(o.vexobs):
        o.vxoxml = os.path.basename(o.vexobs[0:-8]) + '.xml'
        args = ['VEX2XML', '-in', o.vexobs, '-out', o.vxoxml]
        if o.verb:
            print '#Converting VEX to XML with:\n# ' + ' '.join(args)
        try:
            p = subprocess.Popen(args,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        except Exception, ex:
            raise Exception, 'VEX2XML failed: ' + str(ex)
        (v2xout, v2xerr) = p.communicate()
        p.wait()
        if p.returncode:
            err = 'Return code %d from VEX2XML' % p.returncode
            raise RuntimeError, err
    else:
        raise Exception, 'no file ' + o.vexobs + ' to parse'
    o.vextree = xml.etree.ElementTree.parse(o.vxoxml)
    os.unlink(o.vxoxml)

def doFindProj(o):
    '''
    Read the lines locating the scans assigned to projects:
        ...
        scan x
        * intent = "ALMA:PROJECT_FINAL_SCAN:yyyy"
        ...
        scan x++
        * intent = "ALMA:PROJECT_FIRST_SCAN:zzzz"
        ...
    Scans not assigned go to a catch-all project na (for not
    applicable or not ALMA).
    '''
    f = open(o.vexobs)
    o.projscans = {}
    lastscan = ''
    thisproj = 'na'
    scan_re = re.compile(r'\s*scan\s*([^;]+);')
    first_re = re.compile(
        r'.*intent.*=.*"ALMA:PROJECT_FIRST_SCAN:(.*)".*$')
    final_re = re.compile(
        r'.*intent.*=.*"ALMA:PROJECT_FINAL_SCAN:(.*)".*$')
    comnt_re = re.compile(r'\s*[*]')
    for line in f.readlines():
        sre = scan_re.search(line)
        if sre:
            lastscan = sre.group(1)
            if o.verb: print 'assigning lastscan', lastscan
            continue
        first = first_re.search(line)
        final = final_re.search(line)
        comnt = comnt_re.search(line)
        if not first and not final and comnt:
            continue
        if first: thisproj = first.group(1)
        if len(thisproj) > 0 and len(lastscan) > 0:
            if thisproj in o.projscans:
                o.projscans[thisproj].append(lastscan)
                if o.verb: print 'appending lastscan', lastscan, 'to',thisproj
            else:
                o.projscans[thisproj] = [lastscan]
                if o.verb: print 'new prj w/lastscan', lastscan, 'to',thisproj
            lastscan = ''
        if final: thisproj = 'na'
    f.close()
    if o.verb:
        for p in sorted(o.projscans.keys()):
            print '# project', p, ': ', o.projscans[p]

def doFindSrcs(o):
    '''
    Generate a list of sources as found in SOURCE section
    def <dname>; source_name = <sname> ; ra = <ra> ; dec = <dec> ; ...
    '''
    if not o.vextree: return
    o.srcs = []
    for sd in o.vextree.findall('SOURCE/def'):
        dname = sd.find('defname').text
        sname = sd.find('source_name/value').text
        ra    = sd.find('ra/value').text
        dec   = sd.find('dec/value').text
        frame = sd.find('ref_coord_frame/value').text
        if o.verb:
            print dname,sname,ra,dec,frame
        o.srcs.append(sname)

def doInputSrcs(o):
    '''
    Generate a list of sources as found in o.cabbage
    '''
    if not o.cabbage: return
    o.srcset = set()
    for c in o.cabbage:
        o.srcset.add(o.cabbage[c][3][4])
    o.srcs = list(o.srcset)

def findJobMatch(o, smjd):
    '''
    Find the job with the matching mjd start smjd.
    2s/86400s  = .00002314814814814814
    '''
    for j in o.jobbage.keys():
        if math.fabs(o.jobbage[j][0] - smjd) < 0.00002:
            return j
    return None

def doScanVTree(o):
    '''
    Scan the vex file matching jobs with scans
    When jobs are so matched, the vex info is appended:
    o.jobbage[#] = [start,stop,[antennas],[name,start,smjd,dur,vsrc,mode]]
    So source is o.jobbage[#][3][4]
    '''
    if not o.vextree: return
    for sn in o.vextree.findall('SCHED/scan'):
        name = sn.find('scanname').text
        start = sn.find('start/value').text
        smjd = vex2MJD(start)
        mode = sn.find('mode/value').text
        vsrc = sn.find('source/value').text
        sits = []
        for station in sn.findall('station'):
            sc = station.find('value').text
            if sc: sits.append(sc)
        if o.jobbage: job = findJobMatch(o, smjd)
        else:         job = None
        if job:
            dur = 86400.0 * (o.jobbage[job][1] - o.jobbage[job][0])
            if o.verb:
                print job, name, start, smjd
                print job,' ', mode, vsrc, int(dur + 0.5), o.jobbage[job][0]
                print job,' ', o.jobbage[job][2]
                print job,' ', sits
            o.jobbage[job].append([name, start, smjd, dur, vsrc, mode])
            o.jobbage[job].append(sits)
        o.vexscans[name] = [name, start, smjd, vsrc, mode]

def adjustOptions(o):
    '''
    Grok arguments and make some adjustments.
    '''
    if len(o.vex) > 0:       doTestVex(o.vex, o.verb)
    if len(o.mjd) > 0:       doTestMJD(o.mjd, o.verb)
    if o.joblist == '' and o.job != '': o.joblist = o.job + '.joblist'
    if o.vexobs  == '' and o.job != '': o.vexobs  = o.job + '.vex.obs'
    o.cabbage = None
    o.antset = set()
    if len(o.inputs) > 0:    doInputs(o)
    o.jobbage = None
    o.vexscans = {}
    o.antennaset = set()
    o.projscans = None
    o.srcs = None
    if len(o.joblist) > 0:   doJobList(o)
    o.vextree = None
    if len(o.vexobs) > 0:
        tcmd = 'type VEX2XML >/dev/null'
        if not o.verb: tcmd = tcmd + ' 2>&1'
        rc = os.system(tcmd)
        if rc == 0 and o.usev2x: doParseVex(o)
        if o.vextree:
            doFindProj(o)
            doFindSrcs(o)
            doScanVTree(o)
            if o.jobbage is not None:
                o.rubbage = o.jobbage   # use info from job list
            elif o.cabbage is not None:
                o.rubbage = o.cabbage   # use info from calc files
            else:
                o.rubbage = None        # eventually to crash & burn
            o.antlers = o.antennaset
        else:
            doFindProj(o)
            doInputSrcs(o)
            o.rubbage = o.cabbage       # can only use calc files
            o.antlers = o.antset
    try:
        if   os.environ['uniq'] == 'true':  o.uniq = True
        elif os.environ['uniq'] == 'false': o.uniq = False
        else: raise Exception, 'Illegal uniq value: ' + os.environ['uniq']
    except:
        pass
    return o

def doSelectData(o):
    '''
    Remove jobs that appear to have no useful data
    '''
    if not o.difx: return
    if o.inputs == '': return
    # o.difxdirs = glob.glob(o.inputs + '_*.difx')
    newjobs = {}
    for j in o.rubbage:
	difx = '%s_%s.difx' % (o.inputs, j)
	files = glob.glob(difx + '/*')
	if len(files) > 0:
	    newjobs[j] = o.rubbage[j]
	else:
	    if o.verb: print '# No data in ' + difx + '/*'
    o.rubbage = newjobs

def doSelectSource(o):
    '''
    Pretty trivial: select on source
    So source is o.jobbage[#][3][4]
    '''
    if o.source == '': return
    if o.verb: print '# Selecting on source', o.source
    newjobs = {}
    if o.source in o.srcs:
        for j in o.rubbage:
            if o.rubbage[j][3][4] == o.source:
                newjobs[j] = o.rubbage[j]
                if o.verb: print '#S',j,str(newjobs[j])
    o.rubbage = newjobs

def doSelectProject(o):
    '''
    Pretty trivial: select on project
    So scan name is o.rubbage[#][3][0]
    '''
    if o.project == '': return
    if o.verb: print '# Selecting on project', o.project, len(o.projscans)
    newjobs = {}
    if o.project in o.projscans:
        for j in o.rubbage:
            if o.rubbage[j][3][0] in o.projscans[o.project]:
                newjobs[j] = o.rubbage[j]
                if o.verb: print '#P',j,str(newjobs[j])
    o.rubbage = newjobs

def bustedCorr(o_inputs, jobnum):
    '''
    If required correlation files are missing, return True
    '''
    for ef in ['.input','.calc','.im','.difx']:
        cfile = o_inputs + '_' + jobnum + ef
        if not os.path.exists(cfile):
            return True
    return False

def doSelectUniq(o):
    '''
    Restrict to jobs with largest job number for repeat correlations.
    As a side effect, discard jobs that are missing correlation files.
    o.jobbage[#] = [start,stop,[antennas],[name,start,smjd,dur,vsrc,mode]]
    '''
    if not o.uniq: return
    if o.rubbage == None or len(o.rubbage) == 0: return
    if o.verb: print '# Reducing joblist to uniq job set'
    scandict = {}
    joblist = sorted(o.rubbage.keys())
    joblist.reverse()
    for j in joblist:
        job = o.rubbage[j]
        ky = "%s-%s" % (job[3][0], job[3][4])
        if ky in scandict or bustedCorr(o.inputs, j):
            if o.verb: print '# Discarding duplicate or broken job',j
            del(o.rubbage[j])
            continue
        else:
            scandict[ky] = [j]

def selectOptions(o):
    '''
    Apply selections to limit things reported
    '''
    doSelectData(o)
    doSelectSource(o)
    doSelectProject(o)
    doSelectUniq(o)
    return o

def doAntennas(o):
    '''
    Generate a list of antennas from the joblist file
    '''
    if len(o.antlers) == 0: return
    print 'antennas="' + ' '.join(o.antlers) + '"'

def doJobInputs(o):
    '''
    Generate a list of job input files from the joblist file
    '''
    if len(o.rubbage) == 0: return
    jl = map(lambda x:"%s_%s.input" % (o.job, x), sorted(o.rubbage.keys()))
    #print 'jobs="' + ' '.join(sorted(o.rubbage.keys())) + '"'
    print 'jobs="' + ' '.join(jl) + '"'

def doScans(o):
    '''
    Generate a list of scan numbers
    '''
    if len(o.rubbage) == 0: return
    js = map(lambda x:o.rubbage[x][3][0], sorted(o.rubbage.keys()))
    print 'scans="' + ' '.join(js) + '"'

def doNumbers(o):
    '''
    Generate a list of job numbers from the joblist file
    '''
    if len(o.rubbage) == 0: return
    jl = map(lambda x:"%s_%s.input" % (o.job, x), sorted(o.rubbage.keys()))
    print 'numbers="' + ' '.join(sorted(o.rubbage.keys())) + '"'

def doSources(o):
    '''
    Generate a list of sources from the vex file
    '''
    if len(o.srcs) == 0: return
    print 'sources="' + ' '.join(o.srcs) + '"'

def doProjects(o):
    '''
    Generate a list of projects from the ALMA project intent comments.
    '''
    if len(o.projscans) == 0: return
    for p in o.projscans:
        print 'project_' + p + '="' + ' '.join(o.projscans[p]) + '"'

def doReport(o):
    '''
    Generate a useful report of jobs.
    o.jobbage[#] = [start,stop,[antennas],[name,start,smjd,dur,vsrc,mode]]
    '''
    if o.rubbage == None or len(o.rubbage) == 0: return
    for j in sorted(o.rubbage.keys()):
        job = o.rubbage[j]
        scan = job[3][0]
        proj = 'dunno'
        for p in o.projscans:
            if scan in o.projscans[p]: proj = p
        antlist = '-'.join(sorted(job[2]))
        print ('%5s %6s %10s %8s %s' %
            (j, job[3][0], job[3][4], proj, antlist)),
        if antlist[0:2] != 'AA':
            print '# do not polconvert!'
        else:
            print ''

def doGroups(o, doLabels):
    '''
    Generate a list of proj=yyy targ=XXX class=sci|cal
    The logic is similar to the preceding routine.
    '''
    if len(o.rubbage) == 0: return
    ans = set()
    for j in sorted(o.rubbage.keys()):
        job = o.rubbage[j]
        scan = job[3][0]
        targ = job[3][4]
        proj = 'na'
        for p in o.projscans:
            if scan in o.projscans[p]: proj = p
        # everything is calibrator except:
        # M87, SGRA as calibrators are 'eht' and proj=targ is 'sci'
        if proj.upper() == targ:                                 clss = 'sci'
        elif proj == 'na' and (targ == 'M87' or targ == 'SGRA'): clss = 'sci'
        elif proj != 'm87' and targ == 'M87':                    clss = 'eht'
        elif proj != 'sgra' and targ == 'SGRA':                  clss = 'eht'
        else:                                                    clss = 'cal'
        ans.add(':'.join([proj,targ,clss]))
    if doLabels:
        #print 'false && { # start with a short job'
        last='zippo'
        print '# The tests with exit are a reminder to make adjustments above'
        for a in sorted(list(ans)):
            proj,targ,clss = a.split(':')
            if proj != last and last != 'zippo':
                print '}'
            if proj != last:
                print '[ -z "$QA2_' + proj + '" ] && echo QA2 error && exit 1'
                print '$QA2_' + proj + ' && {'
                print '  echo processing QA2_' + proj + ' job block.'
            exprt=('  export proj=%s targ=%s class=%s' % tuple(a.split(':')))
            print  '%-54s    label=%s-%s' % (exprt,proj,targ)
            print ('  nohup $ehtc/ehtc-jsgrind.sh < /dev/null ' +
                '> $label-$subv.log 2>&1' )
            last = proj
        print '}'
    else:
        for a in sorted(list(ans)):
            print ('export proj=%s targ=%s class=%s' % tuple(a.split(':')))

def doLostScans(o):
    '''
    Provide in o.lostscans{} those which aren't found in o.rubbage.
    o.rubbage = o.cabbage
    o.rubbage[jn] = [MJDstart, MJDstop, [antennas], [vexinfo]]
    vexinfo = [scan,vexstart,mjdstart,sdur,vsrc,mode]
    o.vexscans[name] = [name, start, smjd, vsrc, mode]
    '''
    o.lostscans = o.vexscans
    for jn in o.rubbage:
        vexinfo = o.rubbage[jn][3]
        name = vexinfo[0]
        if name in o.lostscans: del o.lostscans[name]

def doReportLostScans(o):
    '''
    Provide a report of scans that don't have matching jobs.
    Each scan name has a list: [name, start, smjd, vsrc, mode]
    '''
    print '### Totally missing scans'
    for name in o.lostscans:
        print 'Missing %s at %s on %s' % (
            name, o.lostscans[name][1], o.lostscans[name][3])
    print

def prodDict(verb, codefile):
    '''
    Open the file and calculate the number of fringes for all
    possible baseline product combinations.
    '''
    cf = open(codefile)
    spol = {}
    sdic = {}
    bpol = {}
    for liner in cf.readlines():
        try:
            one,two,pol = liner.rstrip().split()
            spol[one] = int(pol)
            sdic[two.upper()] = one
        except:
            pass
    if verb: print 'station',spol
    if verb: print 'scodes',sdic
    for ref in spol:
        for rem in spol:
            if spol[ref] > 0 and spol[rem] > 0:
                if ref == rem:  # auto
                    bpol[ref+rem] = spol[ref]
                else:           # cross
                    bpol[ref+rem] = spol[ref]*spol[rem]
    if verb: print 'baseline',bpol
    cf.close()
    return sdic,bpol

def calcExpFringes(verb, scdic, blprodic, antennas, fringes):
    '''
    For each baseline work out how many pol products should be present
    and report errors.
    '''
    if verb: print antennas
    total = 0
    track = {}
    for reftwo in sorted(antennas):
        ref = scdic[reftwo]
        for remtwo in sorted(antennas):
            rem = scdic[remtwo]
            if ref+rem in track or rem+ref in track: continue
            track[ref+rem] = blprodic[ref+rem]
            total += track[ref+rem]
    if verb: print track
    error = ''
    for fringe in sorted(fringes):
        frng = os.path.basename(fringe)
        bl = frng[0]+frng[1]
        lb = frng[1]+frng[0]
        if bl in track:
            track[bl] -= 1
        elif lb in track:
            track[lb] -= 1
        else:
            error += ' ' + bl + '|' + lb
    if verb: print track
    for bl in track:
        if track[bl] > 0: error += ' ' + bl + ':' + str(track[bl])
        if track[bl] < 0: error += ' ' + bl + '!' + str(track[bl])
    if verb: print error
    return total,error

def doCheck(o):
    '''
    Find all the 4fit directories and look for all the required products
    Provide some sort of summary geared towards noticing things missing.
    '''
    doLostScans(o)
    if len(o.lostscans) > 0: doReportLostScans(o)
    o.scodes, o.blproddict = prodDict(o.verb, o.codes)
    for jn in o.cabbage:
        antennas = o.cabbage[jn][2]
        vexinfo = o.cabbage[jn][3]
        scanname = vexinfo[0]
        vexstart = vexinfo[1]
        ffdir = glob.glob('*-4fit*save/' + scanname)
        if len(ffdir) == 1:
            ffringes = glob.glob(ffdir[0] + '/' + '??.B.*.*')
            #print jn,scanname,vexstart,antennas,ffdir[0],len(ffringes)
            print jn,scanname,'have','%3d' % len(ffringes),'fringes,',
            if len(o.blproddict) > 0:
                efrng,edets = calcExpFringes(
                    o.verb, o.scodes, o.blproddict, antennas, ffringes)
                print '%3d' % efrng,'expected',
                if len(edets) > 0:
                    print '( missing:',edets,')'
                else:
                    print
            else:
                print
        else:
            print jn,scanname,'has no 4fit dir'

def doTiming(o):
    '''
    Find all the polconvert directories and present timing and status.
    '''
    tim_re = re.compile(
        r'([0-9]+)-([0-9]+)-([0-9]+)T([0-9]+):([0-9]+):([0-9]+)')
    for jn in sorted(o.cabbage):
        sdur = int(o.cabbage[jn][3][3])
        ants = o.cabbage[jn][2]
        for pcd in glob.glob('*_%s.polconvert-*' % jn):
            stf = pcd + '/status'
            if os.path.exists(stf):
                f = open(stf, 'r')
                try:    status = f.readlines().pop().rstrip()
                except: status = 'busted'
                f.close()
            else:
                status = 'missing'
            tim = pcd + '/timing'
            if os.path.exists(tim):
                f = open(tim, 'r')
                lines = f.readlines()
                f.close()
                started = tim_re.search(lines[0].rstrip())
                finnish = tim_re.search(lines[1].rstrip())
                if started and finnish:
                    yrs = int(finnish.group(1)) - int(started.group(1))
                    mos = int(finnish.group(2)) - int(started.group(2))
                    dys = int(finnish.group(3)) - int(started.group(3))
                    hrs = int(finnish.group(4)) - int(started.group(4))
                    mns = int(finnish.group(5)) - int(started.group(5))
                    scs = int(finnish.group(6)) - int(started.group(6))
                    ptime = (60*(60*(24*dys + hrs) + mns) + scs)
                    rate = float(ptime) / float(sdur)
                    timing = 'at %s for %4d / %-4d = %.2fx / %d = %.2fx' % (
                        started.group(0), ptime, sdur, rate, len(ants)-1,
                        rate / (float(len(ants)-1)))
                else:
                    timing = 'busted'
            job = pcd.split('.')[0] + ' ...'
            print job,status,timing,'-'.join(ants)

def updateBLPOL(jobinput, verb):
    '''
    Read the offered input file and parse the product table to provide
    the structure of the fourfit files that will be produced.
    This function is really just geared to the EHT setup.
    '''
    # input file re's:
    ads_re = re.compile(r'^ACTIVE\s*DATASTREAMS:\s*([0-9]+)')
    bls_re = re.compile(r'^ACTIVE\s*BASELINES:\s*([0-9]+)')
    # Cf. BASELINE ENTRIES:   24
    dsi_re = re.compile(r'^DATASTREAM\s*([0-9]+)\s*INDEX:\s*([0-9]+)')
    bli_re = re.compile(r'^BASELINE\s*([0-9]+)\s*INDEX:\s*([0-9]+)')
    fqn_re = re.compile(r'^FREQ\s*ENTRIES:\s*([0-9]+)')
    fqe_re = re.compile(r'^FREQ\s*.MHZ.\s*([0-9]+):\s*([0-9.]+)')
    bwm_re = re.compile(r'^BW\s*.MHZ.\s*([0-9]+):\s*([0-9.]+)')
    sbd_re = re.compile(r'^SIDEBAND\s*([0-9]+):\s*([UL])')
    tsi_re = re.compile(r'^TELESCOPE\s*INDEX:\s*([0-9]+)')
    rfq_re = re.compile(r'^REC\s*FREQ\s*INDEX\s*([0-9]+):\s*([0-9]+)')
    zfq_re = re.compile(r'^ZOOM\s*FREQ\s*INDEX\s*([0-9]+):\s*([0-9]+)')
    pol_re = re.compile(r'^.*POL:\s*([XYRL])')
    bla_re = re.compile(r'^D/STREAM\s*A\s*INDEX\s*([0-9]+):\s*([0-9]+)')
    blb_re = re.compile(r'^D/STREAM\s*B\s*INDEX\s*([0-9]+):\s*([0-9]+)')
    blf_re = re.compile(r'^D/STREAM\s*A\s*BAND\s*([0-9]+):\s*([0-9]+)')
    blg_re = re.compile(r'^D/STREAM\s*B\s*BAND\s*([0-9]+):\s*([0-9]+)')
    stn_re = re.compile(r'^TELESCOPE\s*NAME\s*([0-9]+):\s*([A-Z0-9]+)')
    ant_names = {}  # dictionary of antenna names
    ds_indices = [] # datastream indices [#,SC,P,[freqs]]
    bl_indices = [] # baseline indices   [#,(A,B),SC-SC:PP, [(I,I,F,F)]]
    fq_table = []   # frequency table    [#,Freq,BW,Side]
    ds_index = -1
    bl_index = -1
    blndxtmp = -1
    bl_peer = -1
    fq_peer = -1
    dspol = '?'     # really should be an array
    fc = open(jobinput)
    for liner in fc.readlines():
        line = liner.rstrip()
        # create datastream and baseline indices
        ads_hit = ads_re.search(line)
        if ads_hit: 
            ds_indices = range(int(ads_hit.group(1)))
            continue
        dsi_hit = dsi_re.search(line)
        if dsi_hit: 
            ds_indices[int(dsi_hit.group(1))] = int(dsi_hit.group(2))
            continue
        bls_hit = bls_re.search(line)
        if bls_hit: 
            bl_indices = range(int(bls_hit.group(1)))
            continue
        bli_hit = bli_re.search(line)
        if bli_hit: 
            bl_indices[int(bli_hit.group(1))] = int(bli_hit.group(2))
            continue

        # freq table
        fqn_hit = fqn_re.search(line)
        if fqn_hit: 
            fq_table = range(int(fqn_hit.group(1)))
            continue
        fqe_hit = fqe_re.search(line)
        if fqe_hit: 
            fq_table[int(fqe_hit.group(1))] = [
                int(fqe_hit.group(1)), float(fqe_hit.group(2)), '?', 0.0]
            continue
        bwm_hit = bwm_re.search(line)
        if bwm_hit:
            fq_table[int(bwm_hit.group(1))][2] = float(bwm_hit.group(2))
            continue
        sbd_hit = sbd_re.search(line)
        if sbd_hit:
            fq_table[int(sbd_hit.group(1))][3] = sbd_hit.group(2)
            continue

        # populate datastream table
        tsi_hit = tsi_re.search(line)
        if tsi_hit:
            ds_index += 1;
            ds_indices[ds_index] = [ds_indices[ds_index],
                ant_names[int(tsi_hit.group(1))], dspol, []]
            continue
        pol_hit = pol_re.search(line)
        if pol_hit:
            ds_indices[ds_index][2] = pol_hit.group(1)
            # relabled in pre-polconvert
            if ds_indices[ds_index][2] == 'X': ds_indices[ds_index][2] = 'L'
            if ds_indices[ds_index][2] == 'Y': ds_indices[ds_index][2] = 'R'
            continue
        rfq_hit = rfq_re.search(line)
        if rfq_hit:
            ds_indices[ds_index][3].append(int(rfq_hit.group(2)))
            continue
        zfq_hit = zfq_re.search(line)
        if zfq_hit:
            ds_indices[ds_index][3].append(int(zfq_hit.group(2)))
            continue

        # populate baseline table
        bla_hit = bla_re.search(line)
        if bla_hit:
            bl_index += 1
            blndxtmp = int(bla_hit.group(1))
            bl_peer = int(bla_hit.group(2))
            continue
        blb_hit = blb_re.search(line)
        if (blb_hit and
            bl_index == blndxtmp and int(blb_hit.group(1)) == blndxtmp):
            bl_this = int(blb_hit.group(2))
            bl_indices[bl_index] = [bl_index, (bl_peer, bl_this),
                "%s|%s:%s%s" % (
                    ds_indices[bl_peer][1], ds_indices[bl_this][1],
                    ds_indices[bl_peer][2], ds_indices[bl_this][2]),
                []]
            continue
        blf_hit = blf_re.search(line)
        if blf_hit:
            fq_peer = int(blf_hit.group(2))
            try: fe_peer = ds_indices[bl_peer][3][fq_peer]
            except: fe_peer = 'nada'    ### specline error
            continue
        blg_hit = blg_re.search(line)
        if blg_hit:
            fq_this = int(blg_hit.group(2))
            try: fe_this = ds_indices[bl_this][3][fq_this]
            except: fe_this = 'nowy'    ### specline error
            if fe_peer == fe_this:
                bl_indices[bl_index][3].append(fe_peer)
            else:
                bl_indices[bl_index][3].append(-1)
            continue

        # match telescope indices with names
        stn_hit = stn_re.search(line)
        if stn_hit:
            ant_names[int(stn_hit.group(1))] = stn_hit.group(2)
            continue
    fc.close()
    # at this point everything we need is in bl_indices and fq_table
    if verb:
        #print 'an',str(ant_names)
        #print 'ds',str(ds_indices)
        print 'bl',str(bl_indices)
        print 'fq',str(fq_table)
    # and we want to reduce it to some usable one-liner
    reply = {}
    for prod in bl_indices:
        reply[prod[2]] = "%f..%f(%d)" % (
            fq_table[prod[3][0]][1], fq_table[prod[3][-1]][1], len(prod[3]))
    return reply

def grokChannels(o):
    '''
    Take a harder look at the input files and provide a report
    of how fourfit channels will end up being labelled.  This
    function invokes updateBLPOL() to parse the input files and
    provides a list of channel info in
        o.chanalia [jobn,scan,source,bl|pp,chansig]
    a set of signatures (of channels as keys of a dictionary)
        o.chsig  [jobn,scan,products]
    '''
    if len(o.rubbage) == 0: return
    o.chanalia = []
    o.signature = set()
    o.chsig = {}
    jl = map(lambda x:"%s_%s.input" % (o.job, x), sorted(o.rubbage.keys()))
    for job in jl:
        report = updateBLPOL(os.path.dirname(o.inputs) + '/' + job, o.verb)
        label = re.sub('.input','',job)
        jobnm = label.split('_')[1]
        for p in report:
            vexinfo = o.rubbage[jobnm][3]
            source = "%-12.12s" % vexinfo[4][0:12]
            o.chanalia.append([jobnm,vexinfo[0],source,p,report[p]])
            ab,pol = p.split(':')
            a,b = ab.split('|')
            o.signature.add(a + ":" + report[p])
            o.signature.add(b + ":" + report[p])
    for sig in o.signature:
        o.chsig[sig] = set()
    for cha in o.chanalia:
        p = cha[3]
        ab,pol = p.split(':')
        a,b = ab.split('|')
        o.chsig[a + ":" + cha[4]].add(cha[1] + ' ' + cha[2] + ':' + b)
        o.chsig[b + ":" + cha[4]].add(cha[1] + ' ' + cha[2] + ':' + a)

def doBLProds(o):
    '''
    Provide a report on the channels used by every baseline product
    '''
    for cha in o.chanalia:
        print ' '.join(cha)

def doFFConf(o, detailed):
    '''
    Provides a report of scans with a particular station-channel config.
    '''
    for sig in sorted(list(o.chsig)):
        #count = len(list(o.chsig[sig]))
        if detailed:
            scan = 'none'
            for partner in sorted(list(o.chsig[sig])):
                temp,peer = partner.split(':')
                if temp != scan:
                    print
                    scan = temp
                    #print sig,count,scan,
                    print sig,' ',scan,
                print peer,
        else:
            #print sig,count,
            print sig,' ',
        print

# main entry point
if __name__ == '__main__':
    o = parseOptions()
    o = adjustOptions(o)
    o = selectOptions(o)

    if o.antennas:  doAntennas(o)
    if o.jobinputs: doJobInputs(o)
    if o.scans:     doScans(o)
    if o.numbers:   doNumbers(o)
    if o.sources:   doSources(o)
    if o.projects:  doProjects(o)
    if o.report:    doReport(o)
    if o.groups:    doGroups(o, False)
    if o.labels:    doGroups(o, True)
    if o.check:     doCheck(o)
    if o.timing:    doTiming(o)
    if o.blprods or o.ffconf or o.ffdetail:
        grokChannels(o)
        if o.blprods:   doBLProds(o)
        if o.ffconf:    doFFConf(o, False)
        if o.ffdetail:  doFFConf(o, True)

    sys.exit(0)

#
# eof
#
