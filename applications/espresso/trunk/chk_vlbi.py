#!/usr/bin/env python
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

# Simple script to check the headers of VLBI data and print out a list of
# valid and corrupt files.
# Cormac Reynolds: June 2012 - python parallel version replaces old perl script

from math import *
import os
import sys
import re
import mx.DateTime
import espressolib
import subprocess


def lbaFileLength(filesize, headervals):

    byterate = (
            headervals["BANDWIDTH"] * headervals["NUMBITS"] *
            headervals["NCHAN"] * 2 * 1e6/8.)
    filelength = (filesize - headervals["HEADERSIZE"])/byterate
    return filelength


def lbafile_timerange(filename, header):
    headerkeys = {
            "TIME": str, "HEADERSIZE": int, "NUMBITS": int, "NCHAN": int,
            "BANDWIDTH": float}

    parsehead = dict()
    for headline in header:
        for headerkey in headerkeys.keys():
            headline = headline.strip()
            if re.match(headerkey, headline):
                headline = headline.replace(headerkey, "")
                parsehead[headerkey] = headerkeys[headerkey](headline.strip())

    vexstarttime = ""
    vexendtime = ""

    if "TIME" in parsehead.keys():
        headerdate = parsehead["TIME"]
        headerformat = "%Y%m%d:%H%M%S"
        vexformat = "%Yy%jd%Hh%Mm%Ss"

        startdate = mx.DateTime.strptime(headerdate, headerformat)
        vexstarttime = startdate.strftime(vexformat)

        # calculate the length of the file in seconds
        filesize = os.path.getsize(filename)
        filelength = lbaFileLength(filesize, parsehead)

        enddate = startdate + mx.DateTime.RelativeDateTime(seconds=filelength)
        vexendtime = enddate.strftime(vexformat)

    return vexstarttime, vexendtime


def vsib_header(filename):
    FILE = open(filename)
    header = FILE.read(4096).split("\n")

    return header


def parse_m5findformats(m5_output):
    """Select the highest data rate format consistent with data"""
    m5format = None
    m5_output.reverse()
    for line in m5_output:
        if re.match("^OK:", line):
            m5format = line.split()[3]
            break

    return m5format


def m5_to_vextime(m5time):
    """Convert from m5time (MJD = MJD/hh:mm:ss.ss) to vex time"""

    if m5time is None:
        return m5time
    m5time = m5time.split("=")[1]
    m5time = m5time.strip()
    # convert m5time to constitute parts, noting this match truncates the
    # seconds (which vextime requires anyway)
    day, hours, mins, secs = re.match(
            "(\d+)/(\d+):(\d+):(\d+)", m5time).groups()
    day = int(day)
    hours = int(hours)
    mins = int(mins)
    secs = int(secs)

    # account for fact that sometimes day wraps in m5time aren't recognised so
    # get times like: 56990/24:00:01.00 instead of the expected
    # 56991/00:00:01.00
    while hours >= 24:
        hours -= 24
        day += 1

    fracday = hours/24. + mins/(24.*60.) + secs/(24.*60.*60.)
    vextime = espressolib.convertdate(day + fracday, outformat="vex")

    return vextime


def check_file(infile):
    """ check each file, then return time range and format. Check for
    Corrupt/missing files. """

    corrupt = False
    starttime = None
    endtime = None
    m5time = espressolib.which("m5time")
    m5bsum = espressolib.which("m5bsum")
    vsum = espressolib.which("vsum")
    m5findformats = espressolib.which("m5findformats")

    if not os.path.exists(infile):
        sys.stderr.write(infile + " missing\n")
        corrupt = True

    elif os.path.getsize(infile) == 0:
        # 0 file size will cause difx to hang (at least for LBA format)
        sys.stderr.write(infile + " empty\n")
        corrupt = True

    elif re.search(r".lba$", infile):
        # LBA format
        header = vsib_header(infile)
        if not (header and re.match(r"^TIME\s\d{8}:\d{6}",  header[0])):
            if (header):
                sys.stderr.write(
                        "header for " + infile + " is corrupt: " + header[0] +
                        "\n\n")
            else:
                sys.stderr.write(
                        "header for " + infile + " is corrupt or missing\n\n")
            corrupt = True

        starttime, endtime = lbafile_timerange(infile, header)

    elif (m5bsum and m5time and vsum and m5findformats):
        # assume it is a mark5 or vdif file of some description. Details of
        # the format are not very important for extracting the start time
        # (YMMV). If we don't have m5bsum and m5time in our path we simply will
        # not do this.

        summary_program = m5bsum

        if '.vdif' in infile.lower():
            # assume we must have a VDIF file.

            summary_program = vsum
            #m5format = "VDIF_1000-64-1-2"
            ## get the file start time with m5time
            #command = " ".join([m5time, infile, m5format])
            #starttime_m5, error = subprocess.Popen(
            #        command, shell=True, stdout=subprocess.PIPE,
            #        stderr=subprocess.PIPE).communicate()
            #try:
            #    starttime = m5_to_vextime(starttime_m5)
            #except:
            #    starttime = None
            #    endtime = None
            #    sys.stderr.write(
            #            "cannot decode start time for " + infile + "\n\n")
            #endtime = None

        try:
            # get the start/stop time with m5bsum/vsum
            command = " ".join([summary_program, "-s", infile])
            error = str()
            m5bsum_out, error = subprocess.Popen( 
                    command, shell=True, stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE).communicate()
            starttime_m5, endtime_m5 = m5bsum_out.split()[1:3]
            starttime = espressolib.convertdate(float(starttime_m5), "vex")
            endtime = espressolib.convertdate(float(endtime_m5), "vex")
            #print starttime, endtime
        except:
            try:
                sys.stderr.write(infile)
                command = " ".join([m5findformats, infile])
                stdout, error = subprocess.Popen( command, shell=True,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE).communicate()
                sys.stderr.write(error)
                m5_output = stdout.split("\n")
                m5format = parse_m5findformats(m5_output)
                command = " ".join([m5time, infile, m5format])
                starttime_m5, error = subprocess.Popen( command, shell=True,
                        stderr=subprocess.PIPE).communicate()
                sys.stderr.write(error)

                lastsample = 1000000
                filesize = os.path.getsize(infile)
                command = " ".join([m5time, infile, m5format,
                    str(filesize-lastsample)])
                endtime_m5, error = subprocess.Popen(
                        command, shell=True, stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE).communicate()
                sys.stderr.write(error)

                starttime = m5_to_vextime(starttime_m5)
                endtime = m5_to_vextime(endtime_m5)

            except:
                # couldn't decode time. 
                sys.stderr.write(
                        "cannot decode time for " + infile + "\n\n")
                sys.stderr.write(error)

                starttime = None
                endtime = None


        if not starttime:
            # we know nothing about this format, abandon this file
            corrupt = True

    return infile, starttime, endtime, corrupt


def fix_filelist(outfilelist):
    """turn a check_files list into a list of strings suitable for printing.
    """

    # Prepend a comment character for corrupt files. Where the end time is not
    # known, set it to the start time of the next file.

    # do in reverse for convenience
    outfilelist.reverse()
    previous_starttime = None
    for idx, outfile in enumerate(outfilelist):
        #sys.stderr.write(str(idx) + str(outfile))
        filename, starttime, endtime, corrupt = outfile
        if corrupt:
            filename = "#" + filename
            starttime = None
            endtime = None
        if starttime is None:
            starttime = ""
            endtime = ""
        elif endtime is None:
            if previous_starttime:
                endtime = previous_starttime
            else:
                # put last file  to end way in the future, if actual end time
                # not known
                endtime = "2100y001d00h00m00s"
        outfilelist[idx] = " ".join([filename, " ", starttime, endtime])
        if starttime:
            if previous_starttime:
                if starttime > previous_starttime:
                    sys.stderr.write(
                            "files not in time order for " + filelistname)
            previous_starttime = starttime

    # the last good file should always get in the .input so we can be sure the
    # D/STREAM is not empty. So comment out its timerange.
    for i in range(len(outfilelist)):
        if "#" not in outfilelist[i]: 
            outfilelist[i] = re.sub(
                    "(\s+)(\w)", r"\1#\2", outfilelist[i], count=1)
            break

    outfilelist.reverse()

    return outfilelist


if __name__ == "__main__":
    # read the list of files
    filelist = []
    outfilelist = []
    for filelistname in sys.argv[1:]:
        filelist += open(filelistname).readlines()
        filelist = [line.rstrip() for line in filelist]

    # check each file, then print it with its time range. 

    for infile in filelist:
        outfile = check_file(infile)
        outfilelist.append(outfile)

    # fix up the list for printing
    outfilelist = fix_filelist(outfilelist)

    for outfile in outfilelist:
        print outfile
