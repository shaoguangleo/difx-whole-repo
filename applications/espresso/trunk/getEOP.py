#!/usr/bin/env python2
# =======================================================================
# Copyright (C) 2016 Cormac Reynolds
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the
# Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# =======================================================================

# Cormac Reynolds, December 2011: program to fetch 5 days of EOPs around the
# given date and return in format suitable for appending to .v2d or .vex file.
# Takes dates in MJD, VEX, ISO8601 or VLBA format.
# October 2017: change to use requests module. Optional vex output.
# EOP data from: "https://vlbi.gsfc.nasa.gov/apriori/usno_finals.erp"
# Leap second data come from: "https://vlbi.gsfc.nasa.gov/apriori/ut1ls.dat"
#

import optparse
import re
#import urllib
import requests
import espressolib
import sys
import time
import os


def get_leapsec(leapsec_page, targetJD):
    # parse the leap seconds page
    tai_utc = None
    for line in leapsec_page:
        linedate = float(line[17:27])
        if linedate > targetJD:
            break
        else:
            tai_utc = int(float(line[38:49]))
    return tai_utc


usage = """%prog <date>
<date> can either be in MJD, VEX, ISO8601 or VLBA format
Returns 5 days of EOPs around <date> in .v2d format"""
parser = optparse.OptionParser(usage=usage, version="%prog " + "1.0")
parser.add_option(
        "--vex", "-v",
        dest="vex", action="store_true", default=False,
        help="Produce vex format output")
parser.add_option(
        "--local", "-l",
        dest="local", action="store_true", default=False,
        help="Take EOPs from local $DIFX_EOPS and $DIFX_UT1LS files instead of"
        " web")
parser.add_option(
        "--noverify",
        dest="verify", action="store_false", default=True,
        help="Disable HTTPS certificate checking (at own risk!)")

(options, args) = parser.parse_args()

if len(args) != 1:
    parser.print_help()
    parser.error("Give a date in MJD or VEX format")

if options.vex:
    comment = "*"
else:
    comment = "#"

mjd_jd = 2400000.5
# get target MJD
targetMJD = args[0]
# convert to MJD if necessary
targetMJD = espressolib.convertdate(targetMJD, "mjd")
targetJD = round(targetMJD) + mjd_jd


# dates before June 1979 not valid (earliest EOPs)
if (targetJD < 2444055.5):
    raise Exception("Date too early. No valid EOPs before July 1979")

# fetch EOP data
if not options.local:
    #eop_url = "https://gemini.gsfc.nasa.gov/solve_save/usno_finals.erp"
    #leapsec_url = (
            #"https://gemini.gsfc.nasa.gov/500/oper/solve_apriori_files/ut1ls.dat")
    eop_url = "https://vlbi.gsfc.nasa.gov/apriori/usno_finals.erp"
    leapsec_url = "https://vlbi.gsfc.nasa.gov/apriori/ut1ls.dat"

    print >>sys.stderr, "Fetching EOP data from ", eop_url
    eop_page = requests.get(eop_url, verify=options.verify).content.split("\n")

    print >>sys.stderr, "Fetching leap second data from", leapsec_url
    #leapsec_page = urllib.FancyURLopener().open(leapsec_url).readlines()
    leapsec_page = requests.get(
            leapsec_url, verify=options.verify).content.split("\n")
    print comment, "EOPs downloaded at", time.strftime(
            "%Y-%m-%d %H:%M:%S (%z)")

else:
    # or read from local files
    eop_filename = os.environ.get("DIFX_EOPS")
    print >>sys.stderr, "Reading EOP data from ", eop_filename
    eop_page = open(eop_filename).readlines()
    eop_page_stats = os.stat(eop_filename)
    eop_update_time = time.gmtime(os.stat(eop_filename)[9])
    leapsec_filename = os.environ.get("DIFX_UT1LS")
    print >>sys.stderr, "Reading leap second data from", leapsec_filename
    leapsec_page = open(leapsec_filename).readlines()
    print comment, "EOPs from", eop_filename, "last updated at", time.strftime(
            "%Y-%m-%d %H:%M:%S (%z)", eop_update_time)

v2deop = "EOP {0:d} {{ xPole={1:f} yPole={2:f} tai_utc={3:d} ut1_utc={4:f} }}"

vexeop = """def EOP{5:d};
  TAI-UTC = {3:d};
  A1-TAI = 0 sec;
  eop_ref_epoch = {0:s};
  num_eop_points = 1;
  eop_interval = 24 hr;
  ut1-utc = {4:0.6f};
  x_wobble = {1:0.6f} asec;
  y_wobble = {2:0.6f} asec;
enddef;"""

if options.vex:
    print "$EOP;"
    eopformat = vexeop
else:
    eopformat = v2deop

# parse the eop page
nlines = 0
neop = -1
for nlines, line in enumerate(eop_page):
    # skip the first line, which isn't commented
    if (nlines == 0):
        continue
    if not line:
        continue
    # strip comments
    if (line[0] == "#"):
        continue
    # split the line on whitespace and convert to floats
    eop_fields = line.split()
    eop_fields = [float(field) for field in eop_fields]
    # print an EOP line if we're within 3 days of the target day
    if (abs(eop_fields[0] - targetJD) < 3):
        neop += 1
        tai_utc = get_leapsec(leapsec_page, eop_fields[0])
        if tai_utc is None:
            raise Exception("Leap seconds not found! Check your UT1LS file")
        xpole = eop_fields[1]/10.
        ypole = eop_fields[2]/10.
        ut1_utc = tai_utc+eop_fields[3]/1.e6
        eopdate = int(eop_fields[0] - mjd_jd)
        if options.vex:
            eopdate = espressolib.convertdate(eopdate, outformat="vex")
        print eopformat.format(eopdate, xpole, ypole, tai_utc, ut1_utc, neop)

print >>sys.stderr, "Processed {0:d} lines".format(nlines)
