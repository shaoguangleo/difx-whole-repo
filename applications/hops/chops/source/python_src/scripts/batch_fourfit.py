#!/usr/bin/python

#core imports
from __future__ import print_function
from builtins import range
import datetime
import argparse
import sys
import os
import logging

#hops package python libs
import vpal
import hopstestb as ht
import ffcontrol

################################################################################

def main():

    parser = argparse.ArgumentParser(
        prog='batch_fourfit.py', \
        description='''simple utility for running fourfit over multiple scans with the same control file''' \
        )

    parser.add_argument('control_file', help='the control file to be applied to all scans')
    parser.add_argument('stations', help='concatenated string of single character codes for all stations to be fringe fit')
    parser.add_argument('pol_products', help='comma separated list of polarization-products to be fringe fit')
    parser.add_argument('experiment_directory', help='relative path to directory containing experiment data')

    parser.add_argument('-n', '--num-proc', type=int, dest='num_proc', help='number of concurrent fourfit jobs to run, default=1', default=1)
    parser.add_argument('-b', '--begin-scan', dest='begin_scan_limit', help='limit the earliest scan to be used e.g 244-1719', default="000-0000")
    parser.add_argument('-e', '--end-scan', dest='end_scan_limit', help='limit the latest scan to be used, e.g. 244-2345', default="999-9999")
    parser.add_argument('-a', '--auto-corrs', action="store_true", dest='auto_corrs', help='enable auto-correlations', default='False')
    parser.add_argument('-d', '--disable-cf-caching', action="store_true", dest='disable_caching', help='disable type_222 control file record caching in fringe files (caching is on by default).', default='False')
    parser.add_argument('-p', '--progress', action='store_true', dest='use_progress_ticker', help='monitor process with progress indicator', default=False)

    args = parser.parse_args()

    #set up logging
    date_string = datetime.datetime.now().strftime('%b-%d-%I%M%p-%G')
    logging.basicConfig(filename='batch_fourfit' + date_string + '.log', level=logging.DEBUG)

    control_file = args.control_file
    stations = args.stations
    polprodstring = args.pol_products
    exp_dir = args.experiment_directory

    polprods = list( set( ( polprodstring.upper() ).split(',') ) )
    for i in list(range(0, len(polprods))):
        polprods[i] = polprods[i].strip()

    control_file_path = os.path.abspath(control_file)
    abs_exp_dir = os.path.abspath(exp_dir)

    #sanity checks:
    if len(stations) < 1:
        print("could not parse stations.")
        sys.exit(1)

    if not os.path.isfile(os.path.abspath(control_file)):
        print("could not find control file: ", control_file)
        sys.exit(1)

    #pol product:
    for pp in polprods:
        if pp not in ['XX', 'YY', 'XY', 'YX', 'I']:
            print("error: ", pp, "not understood, polarization products must be an element from: {XX, YY, XY, YX, I}")
            sys.exit(1)

    #force all fringe files generated to save the control file
    #information in the type_222 records, unless this has been disabled
    set_commands = "set gen_cf_record true"
    if args.disable_caching is True:
        set_commands = ''

    #check the format of the scan limit formats:
    if args.begin_scan_limit != "000-0000":
        if len( args.begin_scan_limit ) !=  len('DOY-HHMM'):
            print("error: begin-scan format not understood, please specify as DOY-HHMM.")
            sys.exit(1)
        doy = int(args.begin_scan_limit.split('-')[0])
        hour = int(args.begin_scan_limit.split('-')[1][:2])
        minute = int(args.begin_scan_limit.split('-')[1][2:4])
        if (doy < 0) or (doy > 366) or (hour < 0) or (hour > 23) or (minute < 0) or (minute > 59):
            print("error: could not decode end-scan format please specify as DOY-HHMM.")
            sys.exit(1)

    if args.end_scan_limit != "999-9999":
        if len( args.end_scan_limit ) !=  len('DOY-HHMM'):
            print("error: begin-scan format not understood, please specify as DOY-HHMM.")
            sys.exit(1)
        doy = int(args.end_scan_limit.split('-')[0])
        hour = int(args.end_scan_limit.split('-')[1][:2])
        minute = int(args.end_scan_limit.split('-')[1][2:4])
        if (doy < 0) or (doy > 366) or (hour < 0) or (hour > 23) or (minute < 0) or (minute > 59):
            print("error: could not decode end-scan format please specify as DOY-HHMM.")
            sys.exit(1)

    start_scan_limit = args.begin_scan_limit
    stop_scan_limit = args.end_scan_limit

    ff_list = vpal.processing.load_and_batch_fourfit(abs_exp_dir, stations[0], stations[1:], \
        control_file_path, set_commands, \
        network_reference_baselines_only=False, num_processes=args.num_proc, \
        start_scan_limit=start_scan_limit, stop_scan_limit=stop_scan_limit, \
        pol_products=polprods, use_progress_ticker=args.use_progress_ticker, log_fourfit_processes=True \
    )



if __name__ == '__main__':          # official entry point
    main()
    sys.exit(0)
