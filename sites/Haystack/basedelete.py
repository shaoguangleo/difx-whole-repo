#!/usr/bin/python
#

#
# basedelete - deletes one baseline from a group of one or more
#              difx xxxx.input files
# 
# basedelete <baseline> <input file list>
# 
# original code                                  2014.8.15   rjc

import glob
import optparse
import os
import string
import sys



usage_text = '\n  basedelete [options] <baseline> <input_files>' \
             '\n     baseline is 2x two char stn codes, separated by a hyphen' \
             '\n     input_files can be a wild-carded list of 1 or more input files' \
             '\n  options:' \
             '\n     -v or --verbose generates extra debug information' \
             '\n  deletes a single baseline from 1 or more .input files' \
             '\n  example:   basedelete.py EF-GS "h_12*.input"' \
             '\n  (quotes are needed if there is wild-card matching)'
parser = optparse.OptionParser(usage=usage_text)
parser.add_option(
    '-v', '--verbose', action='store_true', dest='verbose', help='verbose mode (false)',
    default=False)

(opts, args) = parser.parse_args()

if len(args) < 2:
    print "use -h option for help"
    sys.exit (-1)

bline, infiles = args

                    # form list of all input files matching wildcard spec
all_files = glob.glob (infiles)

if opts.verbose:
    print 'basedelete is running in verbose mode'
    print 'deleting baseline', bline, 'from the files', all_files

                    # separate the two stations
ant = bline.split ('-')

                    # loop over all files that need modification
for phyle in all_files:
    if opts.verbose:
        print 'processing', phyle

                    # initialize variables
    ants = []       # antenna list
    dstrs = []      # datastreams
    outlines = []   # copy of lines for output file
    dsi = -1        # datastream index
    nstart = -1     # starting line# of deleted block
    nstop = -1      # last     line# of deleted block
    n = 0

                    # open and read file
    f = open (phyle, 'r+')
    inlines = f.readlines ()
                    # first pass through file, gathering information
    nl = 0
    for line in inlines:
                    # determine telescope #'s corresponding to 2 char names
        fields = line.split ()
        if len(fields) > 0:
            if fields[0] == 'TELESCOPE' and fields[1] == 'NAME':
                if fields[3] == ant[0] or fields[3] == ant[1]:
                    ants.append (fields[2][0])

                    # into datastream table, check that both antennas were found
            elif fields[0] == 'DATASTREAM' and fields[1] == 'ENTRIES:':
                if len (ants) != 2:
                    break

                    # determine datastream indices
            elif fields[0] == 'TELESCOPE' and fields[1] == 'INDEX:':
                dsi += 1
                if fields[2] == ants[0] or fields[2] == ants[1]:
                    dstrs.append (dsi)

            elif fields[0] == 'D/STREAM' and fields[1] == 'A' and fields[2] == 'INDEX':
                if nstart < 0:
                    # no matching A-line yet, examine this one
                    if fields[4] == ants[0] or fields[4] == ants[1]:
                        nstart = nl
                elif nstop < 0:
                    nstop = nl - 1

            elif fields[0] == 'D/STREAM' and fields[1] == 'B' and fields[2] == 'INDEX':
                if fields[4] != ants[0] and fields[4] != ants[1]:
                    if nstop < 0:
                    # false alarm, reset nstart whether set or not
                        nstart = -1
                    # catch case of last baseline in table
            elif fields[1] == 'DATA' and fields[2] == 'TABLE':
                if nstop < 0:
                    nstop = nl -2
        nl += 1
                    # check that the baseline was found
    if nstart < 0:
        print 'did not find baseline', bline, 'in',phyle,'- skipping it'
        f.close ()
        break

    elif opts.verbose:
        print 'deleting lines', nstart, 'through', nstop

                
                    # second pass through file image, deleting baseline
    nl = 0
    for line in inlines:
        fields = line.split ()
        if len(fields) > 0:
                    # modify lines
                    # subtract one from the number of baselines
            if fields[0] == 'BASELINE' and fields[1] == 'ENTRIES:':
                line = 'BASELINE ENTRIES:   ' + str(int(fields[2]) - 1) + '\n'
                outlines.append (line)
                    # copy up til deleted block
            elif nl < nstart:
                outlines.append (line)
                    # copy lines after deleted block
            elif nl > nstop:
                    # change indexing down by one after deleted block
                if fields[0] == 'D/STREAM' and fields[2] == 'INDEX':
                    idx = str (int(fields[3].rstrip (':'))-1) + ': '
                    line = 'D/STREAM ' + fields[1] + ' INDEX ' + idx + fields[4] + '\n'
                outlines.append (line)
        else:       # make sure empty lines get appended, too
            outlines.append (line)
        nl += 1
            
    f.seek (0)      # rewind current file to start
                    # write out modified file contents
    f.writelines (outlines)
    f.truncate ()   # get rid of remainder of input file
    f.close ()      # close this file, go on to next
