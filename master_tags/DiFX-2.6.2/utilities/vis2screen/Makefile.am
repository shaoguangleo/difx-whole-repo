bin_PROGRAMS = printDiFX
dist_bin_SCRIPTS = diffDiFX.py plotDiFX.py plotDiFXPCal.py fringeFindDiFX.py plotDynamicSpectrum.py snipDiFX.py \
	plotResidualSpectrum.py polswapDiFX.py stripantennaDiFX.py printDiFX.py printDiFXInput.py replaceAntennaDiFX.py removeZerovalAutoDiFX.py

AM_CXXFLAGS = $(DIFXMESSAGE_CFLAGS) $(FXCORR_CFLAGS) $(IPP_CFLAGS) 
printDiFX_SOURCES = printDiFX.cpp
printDiFX_LDADD =  $(FXCORR_LIBS) $(DIFXMESSAGE_LIBS) $(IPP_LIBS)
