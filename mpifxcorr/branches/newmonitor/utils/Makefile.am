bin_PROGRAMS = checkmpifxcorr dedisperse_difx mpispeed

AM_CXXFLAGS = -I$(srcdir)/../src/ -I../src/

checkmpifxcorr_SOURCES = \
	checkmpifxcorr.cpp

dedisperse_difx_SOURCES = \
	dedisperse_difx.cpp

mpispeed_SOURCES = \
	mpispeed.cpp

checkmpifxcorr_LDADD = ../src/libmpifxcorr.a

dedisperse_difx_LDADD = ../src/libmpifxcorr.a
