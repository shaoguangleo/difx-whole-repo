bin_PROGRAMS = \
	analysis \
	benchmark

LDADD = \
	$(top_builddir)/src/libbeamformer.la \
	-larmadillo -lblas -llapack

analysis_CXXFLAGS = -I../src/ -Wall -O3

analysis_SOURCES = analysis.cpp

benchmark_CXXFLAGS = -I../src/ -Wall -O3
# benchmark_CXXFLAGS = -I../src/ -Wall -O3 -DUSE_SINGLE_PRECISION=1

benchmark_SOURCES = benchmark.cpp
