#
# $Id$
#

SUBDIRS = .

AM_CPPFLAGS = -I. -I$(srcdir)/include -I$(srcdir)/../../msg/include  -I$(srcdir)/../../mk4util/include -I$(srcdir)/../../dfio/include -DHAVE_CONFIG_H

pkginclude_HEADERS = ./include/ffmath.h

pkglib_LTLIBRARIES = libffmathb.la

libffmathb_la_SOURCES = ./src/ap_mean.c ./src/interp555.c ./src/linterp.c ./src/max555.c ./src/parabola.c ./src/minvert.c

libffmathb_la_LIBADD = ../../msg/libmsgb.la ../../mk4util/libmk4utilb.la ../../dfio/libdfiob.la

libffmathb_la_LDFLAGS = -avoid-version

check_SCRIPTS = ./import_ffmath.sh
EXTRA_DIST = ./import_ffmath.sh

#
# eof
#
