#!/bin/bash
#
# $Id: run_testsuite.sh Thu Dec  8 14:32:03 EST 2016 jpb
#
# python test suite for fourfit, alist, aedit
#

verb=false
[ -n "$testverb" ] && verb=true
[ -n "$testverb" ] && [ "$testverb" -gt 1 ] && set -x

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
DATADIR_ORIG=`cd $srcdir/testdata; pwd`

#see if chk_env has been run
DO_SETUP=${HOPS_PYTEST_SETUP_NEEDED-'false'}

#some necessary directory envronmental vars
CURRENT_TEST_DIR=`pwd`
PATH_TO_MK4IOLIB=$CURRENT_TEST_DIR/../../sub/mk4py/
PATH_TO_MK4IOSOURCE=$srcdir/../../sub/mk4py/

#make sure we have permission to mess around in the test/build directory
#since distcheck plays silly games with where we are allowed to r/w
chmod -R u+rw $CURRENT_TEST_DIR

#to avoid making a mess in the source directory its easiest to
#just copy all the data files we need into the build directory
if [ ! -d "$CURRENT_TEST_DIR/test_data_copy" ]; then
  mkdir "$CURRENT_TEST_DIR/test_data_copy"
fi

cp -r "$DATADIR_ORIG/." "$CURRENT_TEST_DIR/test_data_copy"
export DATADIR="$CURRENT_TEST_DIR/test_data_copy"
chmod -R u+rw $DATADIR

#copy in the python libraries which have been orphaned by automake distcheck
#set up some environmental variable so we can find the python libs
#and the libmk4io.so shared library
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PATH_TO_MK4IOLIB
export PYTHONPATH=$PYTHONPATH:$PATH_TO_MK4IOLIB:$CURRENT_TEST_DIR:$srcdir

#run the test suite (environmental var DATADIR should be set before running this)
#the -B option is makes sure that python doesn't produce any __pycache__ (.pyc)
#files that we have to clean up later
python -B ./hopstestsuite.py
PASS_FAIL=$?

#once again make sure we have permission to mess around in the test directory
chmod -R u+rw $CURRENT_TEST_DIR
chmod -R u+rw $DATADIR

#clean up the mess we made, since distcheck will complain about it
if [ "$DO_SETUP" == 'true' ]
then
  for pyfile in hopstest mk4 afio
  do
    if [ -f "$CURRENT_TEST_DIR/$pyfile.py" ]
    then
      rm -f $CURRENT_TEST_DIR/$pyfile.py
    fi
  done
fi

if [ -d "$DATADIR" ]; then
  rm -r "$DATADIR"
fi

if [ $PASS_FAIL -eq 0 ]
then
  echo "Test suite passed."
  exit 0
else
  echo "Test suite failed."
  exit 1
fi

#
# eof
#
