AM_CPPFLAGS = -I$(top_srcdir)/x3c/include $(VDIFIO_CFLAGS)
AM_CFLAGS = $(VDIFIO_CFLAGS)

bin_PROGRAMS = \
	lf1extract \
	vdifextract \
	lf1list \
	x3projects \
	lf1vdifcheck \
	hrft2vdif \
	hrftStats \
	channelextract

lf1extract_LDADD = \
	../x3c/lib/libpacketReader.a \
	../x3c/lib/libprojectManagement.a \
	../x3c/lib/libIndexer.a \
	../x3c/lib/libx3cCommon.a

lf1extract_SOURCES = \
	lf1extract.cpp 

vdifextract_LDADD = \
	../x3c/lib/libpacketReader.a \
	../x3c/lib/libprojectManagement.a \
	../x3c/lib/libIndexer.a \
	../x3c/lib/libx3cCommon.a

vdifextract_SOURCES = \
	vdifextract.cpp 

#x3stream_LDADD = \
#	../x3c/lib/libpacketReader.a \
#	../x3c/lib/libprojectManagement.a \
#	../x3c/lib/libIndexer.a \
#	../x3c/lib/libx3cCommon.a
#
#x3stream_SOURCES = \
#	x3stream.cpp 

lf1list_LDADD = \
	../x3c/lib/libpacketReader.a \
	../x3c/lib/libprojectManagement.a \
	../x3c/lib/libIndexer.a \
	../x3c/lib/libx3cCommon.a \
	$(VDIFIO_LIBS)

lf1list_SOURCES = \
	lf1list.cpp

x3projects_LDADD = \
	../x3c/lib/libpacketReader.a \
	../x3c/lib/libprojectManagement.a \
	../x3c/lib/libIndexer.a \
	../x3c/lib/libx3cCommon.a

x3projects_SOURCES = \
	x3projects.cpp 

lf1vdifcheck_LDADD = \
	../x3c/lib/libpacketReader.a \
	../x3c/lib/libprojectManagement.a \
	../x3c/lib/libIndexer.a \
	../x3c/lib/libx3cCommon.a \
	$(VDIFIO_LIBS)

lf1vdifcheck_SOURCES = \
	lf1vdifcheck.cpp

hrft2vdif_LDADD = \
	../x3c/lib/libpacketReader.a \
	../x3c/lib/libprojectManagement.a \
	../x3c/lib/libIndexer.a \
	../x3c/lib/libx3cCommon.a \
	$(VDIFIO_LIBS)

hrft2vdif_SOURCES = \
	hrft2vdif.cpp

channelextract_LDADD = \
	$(VDIFIO_LIBS)

channelextract_SOURCES = \
	channelextract.c

hrftStats_LDADD = \
	../x3c/lib/libpacketReader.a \
	../x3c/lib/libprojectManagement.a \
	../x3c/lib/libIndexer.a \
	../x3c/lib/libx3cCommon.a

hrftStats_SOURCES = \
	hrftStats.cpp

