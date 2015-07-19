#ifndef __VEX_SETUP_H__
#define __VEX_SETUP_H__

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "vex_if.h"
#include "vex_channel.h"
#include "vex_stream.h"

class VexSetup	// Container for all antenna-specific settings
{
public:
	VexSetup() : streams(1) {};
	int phaseCalIntervalMHz() const;
	const VexIF *getIF(const std::string &ifName) const;
	double firstTuningForIF(const std::string &ifName) const;	// returns Hz
	double dataRateMbps() const;
	void setPhaseCalInterval(int phaseCalIntervalMHz);
	void selectTones(enum ToneSelection selection, double guardBandMHz);
	int nRecordChan() const;
	int getBits() const;
	int getMinBits() const;
	int getMaxBits() const;
	double getLowestSampleRate() const;
	double getHighestSampleRate() const;
	double getAverageSampleRate() const;

	std::map<std::string,VexIF> ifs;		// Indexed by name in the vex file, such as IF_A
	std::vector<VexChannel> channels;
	std::vector<VexStream> streams;			// or "datastreams".  
};

std::ostream& operator << (std::ostream &os, const VexSetup &x);

#endif
