#!/usr/bin/env python
'''
Recovers VLBI recordings from raw disk images or directly from block
devices (/dev/sd*/) of disks of a broken StreamStor Mark5A/B/C module.

If the Mark5 module had SATA drives these can be relocated into an empty
Mark6 module after which scans can be extracted directly via e.g.

  $ sudo chmod a+rx /dev/sd{c,d,e,r,s,t,u}
  $ mark5_scan_recovery.py /dev/sd{c,d,e,r,s,t,u}

If the Mark5(A) module had PATA drives one could use eight USB-PATA dongles
or PCIe 1xIDE cards and access them directly, as above, or alternatively
use  'dd if=/dev/<blkdevice> of=disk0.img bs=16M'  or similar to store
drive content as disk images first and then extract scans via e.g.

  $ mark5_scan_recovery.py --mark5a /data/raw/MPI+1200/disk{0,1,4,5,6,7}.raw

This script supports recordings on modules that were only partially
populated at recording time, and/or where some disk(s) may have failed later.
The recovered scans are copied to files under a new directory ./recovered/
located under the current working directory.
'''

__version__ = "1.0.2"
__author__ = "Bob Eldering, Jan Wagner"

import argparse
import ctypes
import os, sys, string
from contextlib import contextmanager
from socket import create_connection

debug = False
Nmaxdisks = 8             # expected nr of disks
usedir_offset = 11272704  # location of user dir before from EOF
block_size = 2**16-8      # StreamStor hardware block size, without 8-byte block header
outdir = './recovered/'

fill_word = '\x44\x33\x22\x11'
fill_block = fill_word * (block_size / 4)

class Header(ctypes.LittleEndianStructure):
    _fields_ = [("dummy1", ctypes.c_uint32),
                ("sequence_number", ctypes.c_uint32, 28),
                ("dummy2", ctypes.c_uint32, 4)]

class UserDirMark5BC(ctypes.LittleEndianStructure):
    _fields_ = [("dir_version", ctypes.c_uint32),
                ("status", ctypes.c_uint32),
                ("VSN", ctypes.c_char * 32),
                ("cont_from_VSN", ctypes.c_char * 32),
                ("cont_to_VSN", ctypes.c_char * 32),
                ("spare", ctypes.c_char * 24)]

class UserDirMark5BCEntry(ctypes.LittleEndianStructure):
    _fields_ = [("datatype", ctypes.c_uint8, 8),
                ("scannumber", ctypes.c_uint32, 24),
                ("framelen", ctypes.c_uint16, 16),
                ("station", ctypes.c_char * 2),
                ("scanname", ctypes.c_char * 32),
                ("exp", ctypes.c_char * 8),
                ("start", ctypes.c_uint64),
                ("stop", ctypes.c_uint64),
                ("bcd", ctypes.c_uint64),
                ("frame0", ctypes.c_uint32),
                ("readoffset", ctypes.c_uint32),
                ("writeoffset", ctypes.c_uint32),
                ("bitstreammask", ctypes.c_uint32),
                ("filler", ctypes.c_char * 40)]

MK5A_DIR_MAXSCANS = 1024
MK5A_EXTNAME_MAXLEN = 64
class UserDirMark5A(ctypes.LittleEndianStructure):
    _fields_ = [("numscans", ctypes.c_uint32),
                ("nextscan", ctypes.c_uint32),
                #("allnames", ctypes.c_char * (MK5A_EXTNAME_MAXLEN*MK5A_DIR_MAXSCANS)),
                ("allnames", ctypes.c_ubyte * (MK5A_EXTNAME_MAXLEN*MK5A_DIR_MAXSCANS)),
                ("startbyte", ctypes.c_uint64 * MK5A_DIR_MAXSCANS),
                ("length", ctypes.c_uint64 * MK5A_DIR_MAXSCANS),
                ("writeoffset", ctypes.c_uint64),
                ("readoffset", ctypes.c_uint64),
                ("playrate", ctypes.c_double),
                ("vsn", ctypes.c_char * 32)]


def read_sequence_number(file_):
    header = Header()
    file_.readinto(header)
    return header.sequence_number


def add_sequence_number(dict_, file_):
    if file_ == None: return
    sequence_number = read_sequence_number(file_)
    if sequence_number in dict_:
        raise RuntimeError("Double sequence number detected {n}, "
                           "in both {f1} and {f2}".format(
                               n=sequence_number,
                               f1=dict_[sequence_number].name,
                               f2=file_.name))
    dict_[sequence_number] = file_


def analyze_disks(inputs_):
    '''
    Checks the given list of file descriptors (open disks/partitions) and
    inspects the beginnings of the files for Mark5 sequence numbers.
    Determines the number of disks originally in the module, and returns file
    descriptors placed into correct order, with 'None' at indices of missing disks.
    '''
    for file_ in inputs_:
        file_.seek(0)
        seq = read_sequence_number(file_)
        file_.seek(block_size + 8)
        seq_next = read_sequence_number(file_)
        num_disks_used = seq_next - seq

    reordered = [None] * num_disks_used

    for file_ in inputs_:
        file_.seek(0)
        reordered[read_sequence_number(file_) % num_disks_used] = file_
        file_.seek(0)

    print ('The original module seems to have recorded onto %d disks' % (num_disks_used))
    for d in range(num_disks_used):
        print ('Module disk #%d --> %s' % (d,str(reordered[d])))

    return reordered


##############################################################################################

def get_scans_mark5a(inputs_):
    '''
    Reads a really old Mark5A-format user directory from Mark5 raw disks.
    Inspects the first valid file descriptor only.
    '''
    scans = []
    udirinfo = UserDirMark5A()
    for input in inputs_:
        if input == None: continue
        input.seek(-usedir_offset, 2)
        input.readinto(udirinfo)
        if udirinfo.numscans > MK5A_DIR_MAXSCANS:
            continue
        print("Module has VSN %s and %d scans" % (udirinfo.vsn,udirinfo.numscans))
        print(udirinfo.allnames, len(udirinfo.allnames))

        for n in range(udirinfo.numscans):
            fileext = '.m5a'
            scanname = ''.join([chr(i) for i in udirinfo.allnames[(n*MK5A_EXTNAME_MAXLEN):((n+1)*MK5A_EXTNAME_MAXLEN)]]).rstrip('\x00')
            #print(n,fileext,scanname)

            try:
                scanfile = open(outdir+scanname+fileext, 'r+b')
            except Exception as e:
                scanfile = open(outdir+scanname+fileext, 'wb')

            if udirinfo.length[n] <= 16:
                print(n,fileext,scanname,udirinfo.startbyte[n],udirinfo.startbyte[n] + udirinfo.length[n],'skipping this scan')
                continue
            print(n,fileext,scanname,udirinfo.startbyte[n],udirinfo.startbyte[n] + udirinfo.length[n])

            scans.append(
                {'name': scanname,
                'start': udirinfo.startbyte[n],
                'stop':  udirinfo.startbyte[n] + udirinfo.length[n],
                'size':  udirinfo.length[n],
                'fdout': scanfile })
        input.seek(0)
        break # look just at the first user directory in disk set

    print ("Module has {n} scans in the Mark5A-format user directory".format(n=len(scans)))

    return scans


def get_scans_mark5bc(inputs_):
    '''
    Reads the user directory from Mark5 raw disks.
    Inspects the first valid file descriptor only.
    '''
    scans = []
    udir = UserDirMark5BC()
    scan = UserDirMark5BCEntry()
    for input in inputs_:
        if input == None: continue
        input.seek(-usedir_offset, 2)
        input.readinto(udir)
        print('Module has VSN %s' % (udir.VSN))
        for n in range(8192):
            input.readinto(scan)
            if any(c not in string.printable for c in scan.scanname):
                break

            fileext = '.m5a'
            if scan.datatype == 9:
                fileext = '.vdif'
            scanname = '_'.join([scan.exp, scan.station, scan.scanname])

            if scan.stop <= scan.start:
                print(n,fileext,scanname,scan.start,scan.stop,'skipping this scan')
                continue
            print(n,fileext,scanname,scan.start,scan.stop)

            try:
                scanfile = open(outdir+scanname+fileext, 'r+b')
            except Exception as e:
                scanfile = open(outdir+scanname+fileext, 'wb')

            # some debug-speedup:
            # scan.start = int(scan.start / (8*16))
            # scan.stop = int(scan.stop / (8*16))

            scans.append(
                {'name': scanname,
                'start': scan.start,
                'stop':  scan.stop,
                'size':  scan.stop-scan.start+1,
                'fdout': scanfile })
        input.seek(0)
        break # look just at the first user directory in disk set

    print ("Module has {n} scans in the Mark5B/5C-type user directory".format(n=len(scans)))

    return scans


##############################################################################################

def output_write(linearoffset_, data, currscan_, scanlist_):
    '''
    Writes one block of fill-pattern.
    Output is redirected to the corresponding scan(s).
    '''
    if len(data) == 0:
        return currscan_
    while (currscan_ < len(scanlist_)) and (linearoffset_ < scanlist_[currscan_]['start']):
        currscan_ += 1
    if currscan_ >= len(scanlist_):
        return currscan_

    s = scanlist_[currscan_]
    remain = s['stop'] - linearoffset_
    if remain >= len(data):
        s['fdout'].write(data)
        return currscan_
    else:
        excess = data[remain:]
        print ('output_write() at module offset %d, completed scan %d, leftover %d byte to next scan' % (linearoffset_,currscan_,len(excess)))
        newscan = output_write(linearoffset_ + remain, excess, currscan_ + 1, scanlist_)
        return newscan

def copy(source, linearoffset_, currscan_, scanlist_):
    '''
    Reads one block of data from source, returns True if successful
    and when further reading from the source is possible.
    Stores the block of data into the respective output scan(s).
    '''
    success = True
    try:
        data = source.read(block_size)
        assert len(data) == block_size
    except Exception as e:
        data = fill_block
        success = False
    scan = output_write(linearoffset_, data, currscan_, scanlist_)
    return scan,success

def fill(linearoffset_, currscan_, scanlist_):
    '''
    Writes one block of fill-pattern.
    Output is redirected to the corresponding scan(s).
    '''
    scan = output_write(linearoffset_, fill_block, currscan_, scanlist_)
    return scan

##############################################################################################

def module_offset_to_file(mod_offset_, inputs_):
    '''
    Translate a linear offset (module, record pointers) into
    a particular disk in the module and a read offset on that disk.
    FIXME: does not seem to work properly, maybe blocks are
           randomly scattered rather than round-robin?
    '''
    Norigdisks = len(inputs_)
    blk = int(mod_offset_ / block_size)
    input_idx = blk % Norigdisks
    blk_in_file = int(blk / Norigdisks)
    blkstart = blk_in_file * (block_size + 8)
    datastart = blkstart + 8 + (mod_offset_ % block_size)
    avail = block_size - (mod_offset_ % block_size)
    print ('Read : ', mod_offset_, blk, input_idx, blk_in_file, blkstart, avail)
    return inputs[input_idx], blk, blkstart, datastart, avail


##############################################################################################
##############################################################################################

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("inputfile",
                        help="Input file to read data blocks from",
                        nargs="+")
    parser.add_argument('--mark5a', default=False, action='store_true', help='Raw data are from a Mark5A module, rather than 5B/5C')
    parser.add_argument('--version', action='version', version=__version__)
    opts = parser.parse_args()

    try:
        os.mkdir(outdir)
    except Exception as e:
        print (str(e))

    rawsources = [open(filename, "rb") for filename in opts.inputfile]
    if opts.mark5a:
        scans = get_scans_mark5a(rawsources)
    else:
        scans = get_scans_mark5bc(rawsources)
    inputs = analyze_disks(rawsources)
    Nscans = len(scans)

    nfilled = 0
    linearoffset = 0
    currscan = 0

    inputs = [open(filename, "rb") for filename in opts.inputfile]  # todo: could use rawsources, inputs of above
    sequence_numbers = {}
    for file_ in inputs:
        add_sequence_number(sequence_numbers, file_)

    previous_sequence_number = -1
    while sequence_numbers:
        number = min(sequence_numbers.keys())
        file_ = sequence_numbers.pop(number)
        if number != previous_sequence_number + 1:
            if debug:
                print "Sequence number error: expected {e}, got {g}, "\
                    "difference {d}, source file '{f}'".format(
                    e=previous_sequence_number+1,
                    g=number,
                    d=previous_sequence_number+1-number,
                    f=file_.name)
            nfilled = nfilled + (number - previous_sequence_number - 1)
            if (number % 1000) == 0:
                print ('At block %d : loss so far %.1f%%' % (number, 100.0*nfilled/number))
        if number < previous_sequence_number:
            print "Removing file from read list"
            continue

        # add fill pattern for missing sequence numbers
        for dummy in xrange(number - previous_sequence_number - 1):
            currscan = fill(linearoffset, currscan, scans)
            linearoffset += block_size

        # copy the data
        currscan,success = copy(file_, linearoffset, currscan, scans)
        linearoffset += block_size
        if success:
            # only try to read the sequence number if the copy succeeded
            try:
                add_sequence_number(sequence_numbers, file_)
            except Exception as e:
                print "Failed to read next sequence number from '{f}'".\
                    format(f=file_.name)
        else:
            print "Dropping source file '{f}'".format(f=file_.name)
        previous_sequence_number = number

        # reporting
        if (number % 1000) == 0 and currscan >= 0 and currscan < len(scans):
            scanlen = scans[currscan]['stop'] - scans[currscan]['start']
            print (scans[currscan]['name'], linearoffset, scans[currscan]['stop'], '%.1f%%' % (100.0*(linearoffset-scans[currscan]['start'])/scanlen))
