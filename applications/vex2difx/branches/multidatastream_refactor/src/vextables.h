/***************************************************************************
 *   Copyright (C) 2009-2015 by Walter Brisken & Adam Deller               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/*===========================================================================
 * SVN properties (DO NOT CHANGE)
 *
 * $Id$
 * $HeadURL$
 * $LastChangedRevision$
 * $Author$
 * $LastChangedDate$
 *
 *==========================================================================*/

#ifndef __VEXTABLES_H__
#define __VEXTABLES_H__

#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <map>
#include <difxio.h>
#include "interval.h"
#include "vex_basebanddata.h"
#include "vex_clock.h"
#include "vex_datastream.h"
#include "vex_antenna.h"
#include "vex_scan.h"

extern const double RAD2ASEC;

class VexData;

class VexEvent
{
public:
	enum EventType	// NOTE! keep VexEvent::eventName up to date
	{
		NO_EVENT,
		ANTENNA_STOP,
		ANT_SCAN_STOP,
		SCAN_STOP,
		JOB_STOP,
		OBSERVE_STOP,
		RECORD_STOP,
		CLOCK_BREAK,
		LEAP_SECOND,
		MANUAL_BREAK,
		RECORD_START,
		OBSERVE_START,
		JOB_START,
		SCAN_START,
		ANT_SCAN_START,
		ANTENNA_START
	};

	static const char eventName[][20];

	double mjd;
	enum EventType eventType;
	std::string name;
	std::string scan;

	VexEvent() : mjd(0.0), eventType(NO_EVENT), name("") {}
	VexEvent(double m, enum EventType e, const std::string &a) : mjd(m), eventType(e), name(a), scan("") {}
	VexEvent(double m, enum EventType e, const std::string &a, const std::string &b) : mjd(m), eventType(e), name(a), scan(b) {}
};

class VexSource
{
public:
	VexSource() : ra(0.0), dec(0.0) {}
	bool hasSourceName(const std::string &name) const;

	std::string defName;			// in the "def ... ;" line in Vex
	
	std::vector<std::string> sourceNames;	// from source_name statements
	double ra;		// (rad)
	double dec;		// (rad)

	static const unsigned int MAX_SRCNAME_LENGTH = 12;
};

class VexSubband		// Mode-specific frequency details for all antennas
{
public:
	VexSubband(double f=0.0, double b=0.0, char s=' ', char p=' ', std::string name="", int os=1) : freq(f), bandwidth(b), sideBand(s), pol(p) {}

	double freq;		// (Hz)
	double bandwidth;	// (Hz)
	char sideBand;		// net side band of channel (U or L)
	char pol;		// R or L
};

class VexChannel		// Antenna-specific baseband channel details
{
public:
	VexChannel() : recordChan(-1), subbandId(-1), bbcFreq(0.0), bbcBandwidth(0.0), bbcSideBand(' ') {}
	void selectTones(int toneIntervalMHz, enum ToneSelection selection, double guardBandMHz);
	char bandCode() const;
	friend bool operator ==(const VexChannel &c1, const VexChannel &c2);
	friend bool operator <(const VexChannel &c1, const VexChannel &c2);

	int recordChan;		// channel number on recorded media or threadnum on stream	(< 0 indicates non-recording)
	int streamId;		// stream number
	int subbandId;		// 0-based index; -1 means unset
	std::string ifName;	// name of the IF this channel came from
	double bbcFreq;		// sky frequency tuning of the BBC (Hz)
	double bbcBandwidth;	// bandwidth (Hz)
	char bbcSideBand;	// sideband of the BBC
	std::string name;
	std::string bbcName;	// name given in VEX of this channel in the BBC table
	std::vector<int> tones;	// pulse cal tones to extract, directly from PHASE_CAL_DETECT
	int threadId;		// thread Id for this channel (assigned based on channel names)
};

class VexIF
{
public:
	VexIF() : ifSSLO(0.0), ifSideBand(' '), pol(' '), phaseCalIntervalMHz(0) {}
	std::string bandName() const;
	std::string VLBABandName() const;
	double getLowerEdgeFreq() const;

	std::string name;
	double ifSSLO;		// [Hz] SSLO of the IF
	char ifSideBand;	// U or L
	char pol;		// R or L
	int phaseCalIntervalMHz;// MHz, typically 1 or 5 (or 0 if none)

	// special values needed for VLBA, extracted from comment line
	std::string comment;
};

class VexSetup	// Container for all antenna-specific settings
{
public:
	VexSetup() : sampRate(0.0), nBit(0), nRecordChan(0), dataSampling(SamplingReal) {}
	int phaseCalIntervalMHz() const;
	const VexIF *getIF(const std::string &ifName) const;
	double firstTuningForIF(const std::string &ifName) const;	// returns Hz
	double dataRateMbps() const { return sampRate*nBit*nRecordChan/1000000.0; }
	void setPhaseCalInterval(int phaseCalIntervalMHz);
	void selectTones(enum ToneSelection selection, double guardBandMHz);

	std::map<std::string,VexIF> ifs;		// Indexed by name in the vex file, such as IF_A
	std::vector<VexChannel> channels;

	double sampRate;		// [Hz]
	unsigned int nBit;
	unsigned int nRecordChan;	// number of recorded channels
	std::string formatName;		// e.g. VLBA, MKIV, Mk5B, VDIF, LBA, K5, ...
	enum SamplingType dataSampling;	// Real or Complex
};

class VexMode
{
public:
	VexMode() {}

	int addSubband(double freq, double bandwidth, char sideband, char pol);
	int getPols(char *pols) const;
	int getBits() const;
	int getMinBits() const;
	int getMinSubbands() const;
	const VexSetup* getSetup(const std::string &antName) const;
	double getLowestSampleRate() const;
	double getHighestSampleRate() const;
	double getAverageSampleRate() const;
	void swapPolarization(const std::string &antName);
	void setPhaseCalInterval(const std::string &antName, int phaseCalIntervalMHz);
	void selectTones(const std::string &antName, enum ToneSelection selection, double guardBandMHz);

	std::string defName;

	std::vector<VexSubband> subbands;
	std::map<std::string,VexSetup> setups;	// indexed by antenna name
};

class VexEOP
{
public:
	VexEOP() : mjd(0), tai_utc(0), ut1_utc(0.0), xPole(0.0), yPole(0.0) {}

	double mjd;		// days
	double tai_utc;		// seconds
	double ut1_utc;		// seconds
	double xPole, yPole;	// radian

	int setkv(const std::string &key, const std::string &value);
};

class VexExper : public Interval
{
public:
	VexExper() : Interval(0.0, 1000000.0) {}

	std::string name;
};

class VexJob : public Interval
{
public:
	VexJob() : Interval(0.0, 1000000.0), jobSeries("Bogus"), jobId(-1), dutyCycle(1.0), dataSize(0.0) {}

//	void assignVSNs(const VexData &V);
//	std::string getVSN(const std::string &antName) const;
	void assignAntennas(const VexData &V);
	bool hasScan(const std::string &scanName) const;
	int generateFlagFile(const VexData &V, const char *fileName, unsigned int invalidMask=0xFFFFFFFF) const;

	// return the approximate number of Operations required to compute this scan
	double calcOps(const VexData *V, int fftSize, bool doPolar) const;
	double calcSize(const VexData *V) const;

	std::string jobSeries;
	int jobId;
	std::vector<std::string> scans;
//std::map<std::string,std::string> vsns;	// vsn, indexed by antenna name
	std::vector<std::string> jobAntennas;	// vector of antennas used in this job
	double dutyCycle;		// fraction of job spent in scans
	double dataSize;		// [bytes] estimate of data output size
};

class VexJobFlag : public Interval
{
public:
	static const unsigned int JOB_FLAG_RECORD = 1 << 0;
	static const unsigned int JOB_FLAG_POINT  = 1 << 1;
	static const unsigned int JOB_FLAG_TIME   = 1 << 2;
	static const unsigned int JOB_FLAG_SCAN   = 1 << 3;
	VexJobFlag() : antId(-1) {}
	VexJobFlag(double start, double stop, int ant) : Interval(start, stop), antId(ant) {}

	int antId;
};

class VexJobGroup : public Interval
{
public:
	std::vector<std::string> scans;
	std::list<VexEvent> events;

	bool hasScan(const std::string &scanName) const;
	void genEvents(const std::list<VexEvent> &eventList);
	void createJobs(std::vector<VexJob> &jobs, Interval &jobTimeRange, const VexData *V, double maxLength, double maxSize) const;
};

class VexData
{
private:
	VexExper exper;
	std::vector<VexSource> sources;
	std::vector<VexScan> scans;
	std::vector<VexMode> modes;
	std::vector<VexAntenna> antennas;
	std::vector<VexEOP> eops;

	std::list<VexEvent> events;
	std::string directory;

public:
	int sanityCheck();

	VexSource *newSource();
	VexScan *newScan();
	VexMode *newMode();
	VexAntenna *newAntenna();
	VexEOP *newEOP();
	void findLeapSeconds();
	void addBreaks(const std::vector<double> &breaks);
	void swapPolarization(const std::string &antName);
	void setPhaseCalInterval(const std::string &antName, int phaseCalIntervalMHz);
	void selectTones(const std::string &antName, enum ToneSelection selection, double guardBandMHz);
	void setClock(const std::string &antName, const VexClock &clock);
	void setTcalFrequency(const std::string &antName, int tcalFrequency);
	void setAntennaPosition(const std::string &antName, double X, double Y, double Z);
	void setAntennaAxisOffset(const std::string &antName, double axisOffset);

	double obsStart() const
	{
		return events.front().mjd;
	}

	double obsStop() const
	{
		return events.back().mjd;
	}
	char vexStartTime[50];
	char vexStopTime[50];

	const std::string &getDirectory() const { return directory; }
	void setDirectory(const std::string &dir) { directory = dir; }

	unsigned int nSource() const { return sources.size(); }
	int getSourceIdByDefName(const std::string &defName) const;
	const VexSource *getSource(unsigned int num) const;
	const VexSource *getSourceByDefName(const std::string &defName) const;
	const VexSource *getSourceBySourceName(const std::string &name) const;

	unsigned int nScan() const { return scans.size(); }
	const VexScan *getScan(unsigned int num) const;
	const VexScan *getScanByDefName(const std::string &defName) const;
	const VexScan *getScanByAntennaTime(const std::string &antName, double mjd) const;
	void reduceScans(int minSubarraySize, const Interval &timerange);
	void addScanEvents();
	void setScanSize(unsigned int num, double size);
	void getScanList(std::list<std::string> &scans) const;
	unsigned int nAntennasWithRecordedData(const VexScan &scan) const;

	unsigned int nAntenna() const { return antennas.size(); }
	int getAntennaIdByName(const std::string &antName) const;
	int getAntennaIdByDefName(const std::string &antName) const;
	const VexAntenna *getAntenna(unsigned int num) const;
	const VexAntenna *getAntenna(const std::string &name) const;
	double getAntennaStartMJD(const std::string &name) const;
	double getAntennaStopMJD(const std::string &name) const;
	int getNumAntennaRecChans(const std::string &name) const;
	bool removeAntenna(const std::string &name);

	unsigned int nMode() const { return modes.size(); }
	int getModeIdByDefName(const std::string &defName) const;
	const VexMode *getMode(unsigned int num) const;
	const VexMode *getModeByDefName(const std::string &defName) const;
	unsigned int nRecordChan(const VexMode &mode, const std::string &antName) const;

	unsigned int nEOP() const { return eops.size(); }
	void addEOP(const VexEOP &e);
	const VexEOP *getEOP(unsigned int num) const;
	const std::vector<VexEOP> &getEOPs() const { return eops; }

	bool usesAntenna(const std::string &antennaName) const;
	bool usesMode(const std::string &modeDefName) const;

//	unsigned int nVSN(const std::string &antName) const;
	void addVSN(const std::string &antName, unsigned int datastreamId, const std::string &vsn, const Interval &timeRange);
	void addVSNEvents();
//	std::string getVSN(const std::string &antName, const Interval &timeRange) const;

	unsigned int nEvent() const { return events.size(); }
	const std::list<VexEvent> *getEvents() const;
	void addEvent(double mjd, VexEvent::EventType eventType, const std::string &name);
	void addEvent(double mjd, VexEvent::EventType eventType, const std::string &name, const std::string &scanName);
	void sortEvents();

	const VexExper *getExper() const { return &exper; }
	void setExper(const std::string &name, const Interval &experTimeRange);
};

bool operator < (const VexEvent &a, const VexEvent &b);
std::ostream& operator << (std::ostream &os, const VexSource &x);
std::ostream& operator << (std::ostream &os, const VexSubband &x);
std::ostream& operator << (std::ostream &os, const VexChannel &x);
std::ostream& operator << (std::ostream &os, const VexIF &x);
std::ostream& operator << (std::ostream &os, const VexSetup &x);
std::ostream& operator << (std::ostream &os, const VexMode &x);
std::ostream& operator << (std::ostream &os, const VexEOP &x);
std::ostream& operator << (std::ostream &os, const VexJob &x);
std::ostream& operator << (std::ostream &os, const VexJobGroup &x);
std::ostream& operator << (std::ostream &os, const VexEvent &x);
std::ostream& operator << (std::ostream &os, const VexJobFlag &x);
std::ostream& operator << (std::ostream &os, const VexData &x);
bool operator == (const VexSubband &s1, const VexSubband &s2);

#endif
