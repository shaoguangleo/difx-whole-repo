dist_bin_SCRIPTS = \
	addMark5b.pl \
	addVDIF.pl \
	clockshift.py \
	day2mjd.py \
	difxcopy.py \
	mvdifxhere.py \
	dir2filelist.pl \
	filelist2human.pl \
	filterDifx2Fits.py \
	filterDifx2Mark4.py \
	fslog2difx.pl \
	geteop.pl \
	mjd2day.py \
	pcList.pl \
	sec2time.pl \
	startcorr.pl \
	summarizeDifxlogs.py \
	summarizePolconvertLogs.py \
	time.pl \
	time2sec.pl \
	update_eop \
	vdifheader.pl \
	vex2setup.pl \
	mk5bheader.pl \
	m6modinit.py \
	updateDelay.pl \
	updateRate.pl \
	mjd2hms.pl

bin_PROGRAMS = m5fb_makeheader

m5fb_makeheader_SOURCES = \
	m5fb_makeheader.c

install-exec-hook:
	mv $(DESTDIR)$(bindir)/filterDifx2Fits.py $(DESTDIR)$(bindir)/filterDifx2Fits
	mv $(DESTDIR)$(bindir)/filterDifx2Mark4.py $(DESTDIR)$(bindir)/filterDifx2Mark4
	mv $(DESTDIR)$(bindir)/mjd2day.py $(DESTDIR)$(bindir)/mjd2day
	mv $(DESTDIR)$(bindir)/day2mjd.py $(DESTDIR)$(bindir)/day2mjd
	mv $(DESTDIR)$(bindir)/difxcopy.py $(DESTDIR)$(bindir)/difxcopy
	mv $(DESTDIR)$(bindir)/mvdifxhere.py $(DESTDIR)$(bindir)/mvdifxhere
