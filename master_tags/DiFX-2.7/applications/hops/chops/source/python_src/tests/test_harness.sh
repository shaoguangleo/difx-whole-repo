#!/bin/bash

#we only run this test if the user explicitly set this variable
if [ -z "$RUNCHOPSCHECK" ]; then
    #don't run this test unless there is an environmental variable called RUNCHOPSCHECK is present
    exit 0
else
    #run the test
    ################################################################################
    #set up a whole bunch of environmental variables so we can get access to the
    #libraries and python packages which have been built but not yet installed

    OLD_PATH=$PATH
    OLD_LD_LIBRARY_PATH=$LD_LIBRARY_PATH
    OLD_PYTHONPATH=$PYTHONPATH
    OLD_TEXT=$TEXT

    #HOPS not yet setup, so configure these variables
    export PATH=$PATH:"./../../../../postproc/fourfit"
    export LD_LIBRARY_PATH LD_LIBRARY_PATH=$PGPLOT_DIR:$LD_LIBRARY_PATH
    export DEF_CONTROL=/dev/null

    #python packages
    HOPSTESTB_DIR="./../hopstest_module/hopstestb"
    VPAL_DIR="./../vpal_module"

    #python packages with c-libraries
    FFCONTROL_DIR="./../ffcontrol_module"
    FFCONTROL_LIB_DIR="$FFCONTROL_DIR/.libs"

    AFIOB_DIR="./../mk4_module"
    MK4B_DIR="./../mk4_module"
    MK4B_LIB_DIR="$MK4B_DIR/.libs"

    VEXPY_DIR="./../vex_module"
    VEXPY_LIB_DIR="$VEXPY_DIR/.libs"

    #add them to our path
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$FFCONTROL_LIB_DIR:$MK4B_LIB_DIR:$VEXPY_LIB_DIR
    export PYTHONPATH=$PYTHONPATH:$HOPSTESTB_DIR:$VPAL_DIR:$FFCONTROL_DIR:$MK4B_DIR:$AFIOB_DIR:$VEXPY_DIR
    export CHOPS_SRC_DIR="/swc/pops/trunk/chops"
    export TEXT="/swc/pops/trunk/chops/source/c_src/vex/text"

    ################################################################################

    #make sure we have permission to mess around in the test/build directory
    #since distcheck plays silly games with where we are allowed to r/w
    chmod -R u+rw "."

    #to avoid making a mess in the source directory its easiest to
    #just copy all the data files we need into the build directory
    CURRENT_TEST_DIR="./3593"
    TESTDATA_ARCHIVE="/swc/pops/trunk/chops/source/python_src/tests/3593.tar.gz"
    if [ -d "$CURRENT_TEST_DIR" ]; then
        chmod -R u+rw $CURRENT_TEST_DIR
        rm -rf "$CURRENT_TEST_DIR"
    fi
    tar -xzvf "$TESTDATA_ARCHIVE" ./;
    chmod -R u+rw $CURRENT_TEST_DIR

    #run the test suite (environmental var DATADIR should be set before running this)
    #the -B option is makes sure that python doesn't produce any __pycache__ (.pyc)
    #files that we have to clean up later
    /usr/bin/python -B /swc/pops/trunk/chops/source/python_src/tests/test_ffres2pcp.py $CURRENT_TEST_DIR
    FFRES2PCP_PASS_FAIL=$?

    /usr/bin/python -B /swc/pops/trunk/chops/source/python_src/tests/test_fourphase.py $CURRENT_TEST_DIR
    FOURPHASE_PASS_FAIL=$?

    /usr/bin/python -B /swc/pops/trunk/chops/source/python_src/tests/test_pcc_generate.py $CURRENT_TEST_DIR
    PCC_PASS_FAIL=$?

    #reset
    export LD_LIBRARY_PATH=$OLD_LD_LIBRARY_PATH
    export PYTHONPATH=$OLD_PYTHONPATH
    export TEXT=$OLD_TEXT

    #once again make sure we have permission to mess around in the test directory
    chmod -R u+rw ./

    if [ -d "$CURRENT_TEST_DIR" ]; then
      rm -rf "$CURRENT_TEST_DIR"
    fi

    if [ "$FFRES2PCP_PASS_FAIL" -eq 0 -a  "$FOURPHASE_PASS_FAIL" -eq 0 -a "$PCC_PASS_FAIL" -eq 0 ]; then
      echo "Chops test suite passed."
      exit 0
    else
      echo "Chops test suite failed."
      exit 1
    fi
fi


#
# eof
#
