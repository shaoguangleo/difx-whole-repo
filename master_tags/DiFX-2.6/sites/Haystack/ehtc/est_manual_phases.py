#!/usr/bin/python
#
# Script to estimate manual phases
#
'''
Script to estimate manual phases in a fourfit control file
'''
import argparse
import fileinput
import os
import re
import subprocess
import sys

def parseOptions():
    des = '''
    This script is designed to create a fourfit control file
    from one bright fringe.  The defaults are appropriate for
    the EHT mixed-pol ALMA case where there are strong XL and XR
    fringes which allow the phase offset for the reference
    station to be measured.  The script is intended to be
    adaptable to other applications.
    '''
    epi = '''
    Typical usage requires just the name of the control file
    to build and the root file to use:
    est_manual_phases.py -r 3597/No0049/3C279.zlwmcz -c sample.conf
    '''
    use = '%(prog)s [options]\n'
    use += '  Version '
    use += '$Id: est_manual_phases.py 2988 2020-08-06 15:21:31Z gbc $'
    parser = argparse.ArgumentParser(epilog=epi, description=des, usage=use)
    required = parser.add_argument_group('required options')
    flaggers = parser.add_argument_group('flag options')
    dbugging = parser.add_argument_group('debugging options')
    optional = parser.add_argument_group('tuning options')
    required.add_argument('-c', '--control', dest='control',
        metavar='FILE', default='', required=True,
        help='Name of fourfit control file to create/update. '
            + 'Variations of the name will be used and created in the '
            + 'process; see the --tidy option')
    required.add_argument('-r', '--rootfile', dest='rootfile',
        metavar='FILE', default='', required=True,
        help='Fourfit root file for fringe-finder scan to work with')
    #
    flaggers.add_argument('-v', '--verbose', dest='verb',
        action='store_true', default=False,
        help='Provide more verbosity about activities')
    flaggers.add_argument('-n', '--nuke', dest='nuke',
        action='store_true', default=False,
        help='Nuke existing control file and start from scratch')
    flaggers.add_argument('-X', '--mixed', dest='mixed',
        action='store_true', default=False,
        help='Fall back on the LMT/ALMA mixed first strategy')
    #
    dbugging.add_argument('-d', '--dry', dest='dry',
        action='store_true', default=False,
        help='Dry run mode: show the commands, but do no work.')
    dbugging.add_argument('-p', '--prune', dest='prune',
        action='store_true', default=False,
        help='Just prune an existing control file that '
            + 'was created by a similar process')
    dbugging.add_argument('-x', '--defaults', dest='defaults',
        action='store_true', default=False,
        help='Print out the defaults and exit.')
    #
    optional.add_argument('-s', '--sites', dest='sites',
        metavar='LIST', default='A,L,Z,S,R,P,J,C,X,Y',
        help='Comma separated list of stations to process')
    optional.add_argument('-q', '--sequence', dest='sequence',
        metavar='LIST', default='8,1,1',
        help='Sequence of est_pc_manual directives')
    optional.add_argument('-m', '--max', dest='max',
        metavar='INT', default='4',
        help='Maximum number of iterations of -q sequence')
    optional.add_argument('-t', '--tolerance', dest='tolerance',
        metavar='FLOAT', default=0.000001, type=float,
        help='Continue until sbd/mbd are smaller than this')
    optional.add_argument('-a', '--additional', dest='additional',
        action='store_true', default=False,
        help='If set, just do the phase/delay on the site list.')
    optional.add_argument('arguments', nargs='*',
        help='Any remaining command-line arguments are treated '
            + 'as control file global directives.  Comments may be included: '
            + '* starts a comment and @ is translated into a newline')
    o = parser.parse_args()
    # non-arguments to pass around in the package
    o.site = {}
    o.cfd = {}
    o.ffdir = ''
    o.stamp = ''
    o.sbdmbd_re = re.compile(r'.est: sbd (.*) mbd (.*) frr (.*)')
    o.cf_default = '''
    pc_mode manual            * manual phase required
    weak_channel 0.0          * shut up about code G
    optimize_closure true     * expected to be better
    mbd_anchor sbd            * model isn't too good
    gen_cf_record true        * for debugging @
    sb_win -0.10     0.10     * SBD search window
    mb_win -0.008    0.008    * MBD search window
    dr_win -0.000001 0.000001 * Delay Rate window
    '''
    return o

def makeSiteMap(o):
    '''
    Look up the list of Mk4 site names in the root file.
    This is a very trivial parser, but works for difx2mark4
    generated root files as currently generated.
    '''
    two_re = re.compile(r'\s*site_ID\s*=\s*(..);.*')
    one_re = re.compile(r'\s*mk4_site_ID\s*=\s*(.);.*')
    two = None
    one = None
    for linen in fileinput.input(o.rootfile):
        line = linen.rstrip()
        if not one: one = one_re.search(line)
        if not two: two = two_re.search(line)
        if two and one:
            if one.group(1) in o.sites: o.site[one.group(1)] = two.group(1)
            two = None
            one = None

def saveOrigControl(o):
    '''
    Save the original control file prior to doing the work
    '''
    if o.verb: print 'Saving original %s as %s.orig' % (o.control, o.control)
    if o.nuke:
        os.rename(o.control, o.control + '.orig')
        createNewControl(o)
    else:
        os.system('cp -p %s %s' % (o.control, o.control + '.orig'))

def createFile(cfile, directives):
    '''
    Comment-process the string of directives into the control file
    '''
    f = open(cfile, 'w')
    f.write(directives.replace('@','\n'))
    f.close()

def createNewControl(o):
    '''
    Create a new control file from the arguments,
    if supplied, otherwise from the defaults.
    '''
    if len(o.arguments) > 0:
        createFile(o.control, ' '.join(o.arguments))
        how = 'from command line directives'
    else:
        createFile(o.control, o.cf_default)
        how = 'from default control file directives'
    if o.verb: print 'Created %s %s' % (o.control, how)

def doStarters(o):
    '''
    Idiot stuff to start off with.
    '''
    if not os.path.exists(o.rootfile):
        raise Exception, 'Rootfile "%s" does not exist' % o.rootfile
    if o.verb: print 'Working with rootfile ' + o.rootfile
    makeSiteMap(o)
    if o.verb:
        print 'Site map is:',
        for one in o.site: print ('%s -> %s' % (one, o.site[one])),
        print
    if os.path.exists(o.control): saveOrigControl(o)
    else:                         createNewControl(o)
    o.ffdir = os.path.dirname(o.rootfile)
    o.stamp = o.rootfile[-6:]
    if o.verb: print 'Using data files in %s with stamp %s' % (
        o.ffdir, o.stamp)

def executeFFact(o):
    '''
    Do one execution of the fourfit command, capturing the
    results and interpreting them.  Return with an assesment
    of convergence.
    '''
    if o.verb: print '    %s' % o.cmd
    p = subprocess.Popen(o.cmd.split(' '),
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    cf = open(o.control, 'r')
    nf = open(o.control + '.temp', 'w') 
    for line in cf.readlines(): nf.write(line)
    cf.close()
    nf.write('\n* executeFFact\n* %s\n' % o.cmd)
    converged = False
    ind = '      '
    for line in p.stdout:
        dly = o.sbdmbd_re.search(line.rstrip())
        if dly:
            sbd = float(dly.group(1))
            mbd = float(dly.group(2))
            frr = float(dly.group(3))
            if abs(sbd) < o.tolerance and abs(mbd) < o.tolerance:
                converged = True
            if converged: status = '(is  converged)'
            else:         status = '(not converged)'
            if o.verb: print ind, line.rstrip(), sbd, mbd, frr, status
        else:
            if o.verb: print ind, line,
        # protect control file from error messages
        if line[0:8] == 'fourfit:': line = '* ' + line
        # don't output suggested values after converging
        if converged: line = '* ' + line
        nf.write(line)
    p.wait()
    nf.close()
    os.unlink(o.control)
    os.rename(o.control + '.temp', o.control)
    return converged

def executeFFdry(o):
    '''
    Do one execution of the fourfit command, capturing the
    results and interpreting them.  Return with an assesment
    of convergence.
    '''
    print '    %s' % o.cmd
    return False

def genPhaseDelay(o, ref, rem, ref_pol, rem_pol, sgn):
    '''
    Do a 'sgn' phase/delay cycle on ref,ref_pol and rem,rem_pol.
    First we must verify that the data exists, otherwise
    there is no work to do but make a comment on that.
    '''
    if sgn > 0: what = ref
    else:       what = rem
    if o.verb: print 'Baseline %s%s pol %s%s for %s phase and delay:' % (
        ref, rem, ref_pol, rem_pol, what)
    cor = '%s/%s%s..%s' % (o.ffdir, ref, rem, o.stamp)
    if not os.path.exists(cor):
        if o.verb: print '  Missing datafile %s, skipping...' % cor
        return 0
    if o.verb: print '  Using datafile %s' % cor
    for ite in range(int(o.max)):
        for seq in o.sequence.split(','):
            o.cmd = '%s -t -c %s -b %s%s -P %s%s %s set est_pc_manual %d' % (
                'fourfit', o.control, ref, rem, ref_pol, rem_pol,
                o.rootfile, sgn * int(seq))
            if o.dry: converged = executeFFdry(o)
            else:     converged = executeFFact(o)
            if converged:
                if o.verb: print '    Converged'
                return 1
    return 0

def genPhaseOffset(o, ref, rem, ref_pol, rem_pol, sgn):
    '''
    Do a 'sgn' phase_offset cycle on ref,ref_pol and rem,rem_pol.
    First we must verify that the data exists, otherwise
    there is no work to do but make a comment on that.
    '''
    if sgn > 0: what = ref
    else:       what = rem
    if o.verb: print 'Baseline %s%s pol %s%s for %s phase offset:' % (
        ref, rem, ref_pol, rem_pol, what)
    cor = '%s/%s%s..%s' % (o.ffdir, ref, rem, o.stamp)
    if not os.path.exists(cor):
        if o.verb: print '  Missing datafile %s, skipping...' % cor
        return 0
    if o.verb: print '  Using datafile %s' % cor
    o.cmd = '%s -t -c %s -b %s%s -P %s%s %s set est_pc_manual %d' % (
        'fourfit', o.control, ref, rem, ref_pol, rem_pol, o.rootfile, sgn*64)
    if o.dry: converged = executeFFdry(o)
    else:     converged = executeFFact(o)
    if converged:
        if o.verb: print '    Converged'
        return 1
    return 0

def doTheWorkMix(o):
    '''
    Build the control file as specified in program options.
    In the standard (EHT) path, we do the following:
    0. build a site id mk4 id mapping list
    a. use AL YL to generate A R phase & delay (ref)
    b. use AL XL to generate A L phase & delay (ref)
    c. use AL XR to generate L R phase & delay (rem)
    d. use AL YR to generate L R phase offset (rem)
    e. for every additional station/pol, use
       Ax ?? to generate x R/L phase & delay (rem)
    Note that fourfit aliases X and Y into L and R.
    '''
    doStarters(o)
    sites = o.sites.split(',')
    mixed = sites.pop(0)
    ok = 0
    eok = 4 + 2 * len(sites)
    if not o.additional:
        if mixed != 'A':
            raise Exception, 'ALMA is expected to be first in the list of sites'
        fixed = sites.pop(0)
        if fixed != 'L':
            raise Exception, 'LMT is expected to be second in the list of sites'
        ok += genPhaseDelay(o, mixed, fixed, 'R', 'L', 1)  # step a.
        ok += genPhaseDelay(o, mixed, fixed, 'L', 'L', 1)  # step b.
        ok += genPhaseDelay(o, mixed, fixed, 'L', 'R', -1)  # step c.
        ok += genPhaseOffset(o, mixed, fixed, 'R', 'R', -1) # step d.
    for other in sites:
        ok += genPhaseDelay(o, mixed, other, 'R', 'R', -1)  # step e.
        ok += genPhaseDelay(o, mixed, other, 'R', 'L', -1)  # step e.
    if ok == eok:
        print 'The %d of %d steps were completed properly'%(ok,eok)
    else:
        print 'Only %d of %d steps were completed properly'%(ok,eok)

def doTheWorkAok(o):
    '''
    Build the control file as specified in program options.
    In the standard (EHT) path, we do the following:
    0. build a site id mk4 id mapping list
    a. If the first site is ALMA, assume done properly: phases/delays => 0
       This is the default, so there is no work. (pop)
    b. for every additional station/pol, use
       Ax ?? to generate x R/L phase & delay (rem)
    c. If the first site is not ALMA, assume done properly; phases/delays
       are not zero, but are consistent with ALMA and fourfit should be
       generating numbers consistent with that.
    d. for every additional station/pol, proceed to generate phases/delays
       as normally.  However, it may be necessary to
    e. swap baselines to find the proper ref/rem combination.
    '''
    doStarters(o)
    sites = o.sites.split(',')
    mixed = sites.pop(0)
    ok = 0
    eok = 2 * len(sites)
    if mixed == 'A':
        for other in sites:
            ok += genPhaseDelay(o, mixed, other, 'R', 'R', -1)  # step a.
            ok += genPhaseDelay(o, mixed, other, 'L', 'L', -1)  # step a.
        if ok == eok:
            print 'The %d of %d steps were completed properly'%(ok,eok)
        else:
            print 'Only %d of %d steps were completed properly'%(ok,eok)
    else:
        # trust the first station
        for other in sites:
            ans = genPhaseDelay(o, mixed, other, 'R', 'R', -1)   # step d.
            if ans == 0:
                ans = genPhaseDelay(o, other,mixed, 'R', 'R', 1) # step e.
            ok += ans
            ans = genPhaseDelay(o, mixed, other, 'L', 'L', -1)   # step d.
            if ans == 0:
                ans = genPhaseDelay(o, other,mixed, 'L', 'L', 1) # step e.
        if ok == eok:
            print 'The %d of %d steps were completed properly'%(ok,eok)
        else:
            print 'Only %d of %d steps were completed properly'%(ok,eok)

def pruneCF(o):
    '''
    Walk through the control file and prune it.
    '''
    os.rename(o.control, o.control + '.full')
    cf = open(o.control + '.full', 'r')
    nf = open(o.control, 'w')
    iftxt = ''
    site = None
    what = None
    if_re = re.compile(r'^if station (.)')
    do_re = re.compile(r'.*delay_offs_(.)')
    ph_re = re.compile(r'.*pc_phases_(.)')
    po_re = re.compile(r'.*pc_phase_offset_(.)')
    for lineo in cf.readlines():
        line = lineo.lstrip()
        if len(line) == 0: continue
        if line[0] == '*': continue
        if line[0] == '\n': continue
        ls = lineo.rstrip()
        hit = if_re.search(ls)
        if hit:
            if site and what:
                key = 'site:' + site + ':' + what
                o.cfd[key] = iftxt
            site = hit.group(1)
            iftxt = lineo
            continue
        hit = do_re.search(ls)
        if hit:
            what = 'do:' + hit.group(1)
            iftxt += lineo
            continue
        hit = ph_re.search(ls)
        if hit:
            what = 'ph:' + hit.group(1)
            iftxt += lineo
            continue
        hit = po_re.search(ls)
        if hit:
            what = 'po:' + hit.group(1)
            iftxt += lineo
            continue
        if site and what: iftxt += lineo
        else: nf.write(line)
    if site and what:
        key = 'site:' + site + ':' + what
        o.cfd[key] = iftxt
    cf.close()
    keys = o.cfd.keys()
    keys.sort()
    for key in keys:
        nf.write('*' + '-'*71 + '\n')
        nf.write(o.cfd[key])
    nf.write('*' + '-'*72 + '\n')
    nf.close()

def dumpOpts(o):
    '''
    Print out all the arguments.
    '''
    print ''
    print 'est_manual_phases.py \\'
    print '  [-v] [-n] [-p] [-d] [-x] -c %s -r %s \\' % (o.control, o.rootfile)
    print '  -s %s -q %s -m %s -t %s [cf directives]' % (
        o.sites, o.sequence, o.max, o.tolerance)
    print ''
    print '  The default list of fourfit cf directives is:',
    print o.cf_default
    print '  Valid bits available in each -q list element are:'
    print '    0x01 -   1 - solve for phases'
    print '    0x02 -   2 - estimate delay from median value [*]'
    print '    0x04 -   4 - estimate delay from average value [*]'
    print '    0x08 -   8 - estimate delay from total SBD value'
    print '    0x10 -  16 - use per-channel SBD values [*]'
    print '    0x20 -  32 - discard outlier per-channel SBD values on [*]'
    print '    0x40 -  64 - calculate the phase offset between polarizations'
    print '    0x80 - 128 - apply a phase bias'
    print ''
    print '  This sequence of estimates [-q] is repeated to the -m limit.'
    print '  The first station is assumed mixed pol (or that the target'
    print '    is polarized; then its R,L phase & delays are calculated.'
    print '  After that the second station R phase, and finally the'
    print '    phase offset for the second station R phase.'
    print '  Finally the first station is used to remaining phase and delays.'
    print ''

# main entry point
if __name__ == '__main__':
    o = parseOptions()
    if o.defaults:
        dumpOpts(o)
        sys.exit(0)
    if o.prune:
        pruneCF(o)
        sys.exit(0)
    try:
        if o.mixed:
            print 'Assuming ALMA is mixed (-X option used).'
            doTheWorkMix(o)
        else:
            print 'Assuming ALMA is fixed (-X option not used).'
            doTheWorkAok(o)
        pruneCF(o)
    except KeyboardInterrupt:
        print '^C, shutting down'
    except Exception, ex:
        print 'Exception: ', ex
        sys.exit(1)
    sys.exit(0)

#
# eof
#
