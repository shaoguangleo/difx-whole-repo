if WITH_SS
mark5_files = \
	nativemk5.cpp \
	mark5dir.cpp
else
mark5_files = \
	nativemk5_stubs.cpp
endif

bin_PROGRAMS = mpifxcorr neuteredmpifxcorr

mpifxcorr_SOURCES = \
	mpifxcorr.cpp \
	fxmanager.cpp \
	core.cpp \
	datastream.cpp \
	visibility.cpp \
	configuration.cpp \
	mode.cpp \
        model.cpp \
	mk5.cpp \
	mk5mode.cpp \
	rawmode.cpp \
	polyco.cpp \
	helpers.cpp \
	alert.cpp \
	pcal.cpp \
	switchedpower.cpp \
	datamuxer.cpp \
	filters.cpp \
	filterchain.cpp \
	filterhelpers.cpp \
	filter_dec.cpp \
	filter_int.cpp \
	filter_iir.cpp \
	filter_iirsos.cpp \
	filter_dsvf.cpp \
	filter_mavg.cpp \
	$(mark5_files)

library_includedir = $(includedir)/mpifxcorr
library_include_HEADERS = \
	fxmanager.h \
	pcal.h \
	core.h \
	datastream.h \
	architecture.h \
	visibility.h \
	configuration.h \
	mk5.h \
	mk5mode.h \
        model.h \
        mode.h \
	polyco.h \
	mark5dir.h \
	nativemk5.h \
	watchdog.h \
	mpifxcorr.h \
	switchedpower.h \
	datamuxer.h \
	helpers.h \
	alert.h 

# historically these have been in both $(includedir)/{.,mpifxcorr}
include_HEADERS = $(library_include_HEADERS)

lib_LIBRARIES = libmpifxcorr.a libfxcorr.a
libmpifxcorr_a_SOURCES = \
	pcal.cpp \
	configuration.cpp \
	mode.cpp \
	core.cpp \
	datastream.cpp \
	polyco.cpp \
	mk5.cpp \
	mk5mode.cpp \
	rawmode.cpp \
	fxmanager.cpp \
        model.cpp \
	visibility.cpp \
	helpers.cpp \
	alert.cpp \
	switchedpower.cpp \
	datamuxer.cpp \
	filters.cpp \
	filterchain.cpp \
	filterhelpers.cpp \
	filter_dec.cpp \
	filter_int.cpp \
	filter_iir.cpp \
	filter_iirsos.cpp \
	filter_dsvf.cpp \
	filter_mavg.cpp \
	$(mark5_files)

libfxcorr_a_SOURCES = \
	configuration.cpp \
	pcal.cpp \
	mode.cpp \
	mk5mode.cpp \
	rawmode.cpp \
	polyco.cpp \
	visibility.cpp \
        model.cpp \
	datamuxer.cpp \
	helpers.cpp \
	filters.cpp \
	filterchain.cpp \
	filterhelpers.cpp \
	filter_dec.cpp \
	filter_int.cpp \
	filter_iir.cpp \
	filter_iirsos.cpp \
	filter_dsvf.cpp \
	filter_mavg.cpp \
	alert.cpp

neuteredmpifxcorr_SOURCES = \
	mpifxcorr.cpp \
	fxmanager.cpp \
	core.cpp \
	datastream.cpp \
	mk5.cpp \
	helpers.cpp \
	switchedpower.cpp \
	datamuxer.cpp \
	$(mark5_files)

neuteredmpifxcorr_CXXFLAGS = -DNEUTERED_DIFX $(AM_CXXFLAGS)

neuteredmpifxcorr_LDADD = $(top_builddir)/src/libfxcorr.a
