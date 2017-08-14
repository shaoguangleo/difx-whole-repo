#!/usr/bin/python
#
# Script to check on zoom band uniformity across a set of jobs
#
'''
ehtc-zoomchk.py -- a program to check zoom band uniformity across jobs
'''

import argparse
import re

def parseOptions():
    '''
    This is a helper program for drivepolconvert.  The working assumption
    is that all jobs share the same zoom band setup; however, sometimes
    this is not the case.  This program provides a quick check to allow
    the drivepolconvert to be broken into several runs if necessary.
    '''
    des = parseOptions.__doc__
    epi = ''
    use = '%(prog)s [options] [input_file [...]]\n  Version'
    use += '$Id: ehtc-zoomchk.py 1951 2017-08-14 14:33:30Z gbc $'
    parser = argparse.ArgumentParser(epilog=epi, description=des, usage=use)
    # essential options
    parser.add_argument('-v', '--verbose', dest='verb',
        default=False, action='store_true',
        help='be chatty about the work')
    parser.add_argument('-a', '--ant', dest='ant',
        default=1, metavar='INT', type=int,
        help='1-based index of linear (ALMA) antenna (normally 1)')
    parser.add_argument('-A', '--ALMA', dest='alma',
        default='AA', metavar='CODE',
        help='two-letter code with linear pol (normall AA)')
    # the remaining arguments provide the list of input files
    parser.add_argument('nargs', nargs='*',
        help='List of DiFX input job files')
    return parser.parse_args()

def deduceZoomIndicies(o):
    '''
    Pull the Zoom frequency indicies from the input files and check
    that all input files produce the same first and last values.
    This code modified from drivepolconvert.py.
    '''
    zoompatt = r'^ZOOM.FREQ.INDEX.\d+:\s*(\d+)'
    amap_re = re.compile(r'^TELESCOPE NAME\s*([0-9])+:\s*([A-Z0-9][A-Z0-9])')
    freqpatt = r'^FREQ..MHZ..\d+:\s*(\d+)'
    zfirst = set()
    zfinal = set()
    mfqlst = set()
    zoomys = set()
    antmap = {}
    jskip = []
    jlist = {}
    for jobin in o.nargs:
        zfir = ''
        zfin = ''
        cfrq = []
        ji = open(jobin, 'r')
        for line in ji.readlines():
            zoom = re.search(zoompatt, line)
            freq = re.search(freqpatt, line)
            if freq: cfrq.append(freq.group(1))
            if zoom:
                if zfir == '': zfir = zoom.group(1)
                else:          zfin = zoom.group(1)
            amap = amap_re.search(line)
            if amap:
                antmap[amap.group(2)] = int(amap.group(1))
        ji.close()
        antlist = '-'.join(
            map(lambda x:x + ':' + str(antmap[x]),sorted(list(antmap))))
        if o.verb: print '# Zoom %s..%s in %s %s' % (
            zfir, zfin, jobin, antlist)
        # still worth checking frequencies
        if len(cfrq) < 1:
            raise Exception, 'Very odd, no zoom frequencies in ' + jobin
        cfrq.sort()
        mfqlst.add(cfrq[len(cfrq)/2])
        # for overall report
        zfirst.add(zfir)
        zfinal.add(zfin)
        # partition job list
        zlim = '%s..%s' % (zfir, zfin)
        zoomys.add(zlim)
        if o.alma in antmap:
            if zlim in jlist:
                jlist[zlim].append(jobin)
            else:
                jlist[zlim] = [jobin]
        else:
            jskip.append(jobin)
        antmap = {}
    if o.verb: print '##\n## Zoom mid freq is ', ' '.join(sorted(mfqlst))
    if (len(zfirst) != 1 or len(zfinal) != 1 or
        len(jskip) > 0 or len(zoomys) > 1):
        print '##'
        print '## EITHER: Ambiguities in zoom freq ranges:'
        print '##   first is %s, final is %s' % (str(zfirst), str(zfinal))
        print '## OR: ALMA is not present in all jobs.'
        print '##'
        print '## You should review, and then execute this sequence:'
        print '##'
        for j in jskip:
            print '# skip %s since %s does not appear' % (j, o.alma)
        print '#'
        for j in jlist:
            print "jobs='%s'" % ' '.join(sorted(jlist[j]))
            print "# and then:"
            print "drivepolconvert.py -v $opts -l $pcal $jobs"
            print '#'
        print '## Be sure to reset the variable jobs to the original list'
        print '## for subsequent processing.'
        print '#'
    else:
        o.zfirst = int(zfirst.pop())
        o.zfinal = int(zfinal.pop())
        if o.verb: print 'Zoom frequency indices %d..%d found in %s..%s' % (
            o.zfirst, o.zfinal, o.nargs[0], o.nargs[-1])
        print '## All jobs are compatible with the same zoom range:',
        for z in zoomys: print '##', z

#
# enter here to do the work
#
if __name__ == '__main__':
    o = parseOptions()
    deduceZoomIndicies(o)

#
# eof
#
