if WITH_STREAMSTOR
streamstor_sources = \
	modules.cpp \
	mark5a.cpp \
	smart.cpp \
	smart.h \
	watchdog.cpp \
	watchdog.h \
	ssversions.cpp
else
streamstor_sources = 
endif

bin_PROGRAMS = \
	sukill \
	mk5daemon

mk5daemon_SOURCES = \
	difx.cpp \
	loadmon.cpp \
	swapmon.cpp \
	logger.cpp \
	logger.h \
	monitor.cpp \
	vsis.cpp \
	vsis_commands.h \
	vsis_commands.cpp \
	getdir.cpp \
	getfilelist.cpp \
	datacopy.cpp \
	fuse.cpp \
	condition.cpp \
	options.cpp \
	options.h \
	proc.cpp \
	proc.h \
	swap.cpp \
	swap.h \
	mk5daemon.cpp \
	mk5daemon.h \
	$(streamstor_sources)

sukill_SOURCES = \
	proc.cpp \
	proc.h \
	sukill.cpp
	
