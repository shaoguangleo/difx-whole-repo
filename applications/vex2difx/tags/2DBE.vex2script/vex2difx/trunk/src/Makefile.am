bin_PROGRAMS = \
	vex2difx \
	vex2script2DBE \
	vex2opt \
	rdbetsys \
	vexpeek

LDADD = \
	$(top_builddir)/vex/libvex.la \
	-lfl

vex2difx_SOURCES = \
	vextables.h \
	vextables.cpp \
	vexload.h \
	vexload.cpp \
	corrparams.h \
	corrparams.cpp \
	util.h \
	util.cpp \
	vex2difx.cpp 

rdbetsys_SOURCES = \
	vextables.h \
	vextables.cpp \
	vexload.h \
	vexload.cpp \
	corrparams.h \
	corrparams.cpp \
	rdbetsys.cpp

vex2script2DBE_SOURCES = \
    vextables.h \
    vextables.cpp \
    vexload.h \
    vexload.cpp \
    corrparams.h \
    corrparams.cpp \
    pystream.h \
    pystream.cpp \
    vex2script.cpp

vex2opt_SOURCES = \
	vextables.h \
	vextables.cpp \
	vexload.h \
	vexload.cpp \
	corrparams.h \
	corrparams.cpp \
	optscans.h \
	optscans.cpp \
	optsources.cpp \
	optsources.h \
	vex2opt.cpp 

vexpeek_SOURCES = \
	vexpeek.cpp \
	vextables.cpp \
	vexload.cpp \
	corrparams.cpp
