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
#include "event.h"
#include "vex_exper.h"
#include "vex_basebanddata.h"
#include "vex_clock.h"
#include "vex_stream.h"
#include "vex_antenna.h"
#include "vex_scan.h"
#include "vex_source.h"
#include "vex_subband.h"
#include "vex_channel.h"
#include "vex_if.h"
#include "vex_setup.h"
#include "vex_mode.h"
#include "vex_eop.h"


class VexData
{
private:
	VexExper exper;
	std::vector<VexSource> sources;
	std::vector<VexScan> scans;
	std::vector<VexMode> modes;
	std::vector<VexAntenna> antennas;
	std::vector<VexEOP> eops;

	std::string directory;

public:
	char vexStartTime[50];		// FIXME: figure out why these are needed and are not in VexExper
	char vexStopTime[50];

	int sanityCheck();

	VexSource *newSource();
	VexScan *newScan();
	VexMode *newMode();
	VexAntenna *newAntenna();
	VexEOP *newEOP();
	void swapPolarization(const std::string &antName);
	void setPhaseCalInterval(const std::string &antName, int phaseCalIntervalMHz);
	void selectTones(const std::string &antName, enum ToneSelection selection, double guardBandMHz);
	void setClock(const std::string &antName, const VexClock &clock);
	void setTcalFrequency(const std::string &antName, int tcalFrequency);
	void addExperEvents(std::list<Event> &events) const;
	void addClockEvents(std::list<Event> &events) const;
	void addScanEvents(std::list<Event> &events) const;
	void addVSNEvents(std::list<Event> &events) const;
	void addBreakEvents(std::list<Event> &events, const std::vector<double> &breaks) const;
	void addLeapSecondEvents(std::list<Event> &events) const;
	void generateEvents(std::list<Event> &events) const;
	void setFiles(unsigned int antId, unsigned int streamId, const std::vector<VexBasebandData> &files);
	void setModule(unsigned int antId, unsigned int streamId, const std::string &vsn);
	void setNetworkParameters(unsigned int antId, unsigned int streamId, const std::string &networkPort, int windowSize);
	void setFake(unsigned int antId);
	void setCanonicalVDIF(const std::string &modeName, const std::string &antName);
	void cloneStreams(const std::string &modeName, const std::string &antName, int copies);
	bool setFormat(const std::string &modeName, const std::string &antName, int dsId, const std::string &formatName);
	void setStreamBands(const std::string &modeName, const std::string &antName, int dsId, int nBand, int startBand);

	double obsStart() const { return exper.mjdStart; }
	double obsStop() const { return exper.mjdStop; }


	const std::string &getDirectory() const { return directory; }
	void setDirectory(const std::string &dir) { directory = dir; }

	unsigned int nSource() const { return sources.size(); }
	int getSourceIdByDefName(const std::string &defName) const;
	const VexSource *getSource(unsigned int num) const;
	const VexSource *getSourceByDefName(const std::string &defName) const;
	const VexSource *getSourceBySourceName(const std::string &name) const;
	void setSourceCalCode(const std::string &name, char calCode);

	unsigned int nScan() const { return scans.size(); }
	const VexScan *getScan(unsigned int num) const;
	const VexScan *getScanByDefName(const std::string &defName) const;
	const VexScan *getScanByAntennaTime(const std::string &antName, double mjd) const;
	void reduceScans(int minSubarraySize, const Interval &timerange);
	void setScanSize(unsigned int num, double size);
	void getScanList(std::list<std::string> &scans) const;
	unsigned int nAntennasWithRecordedData(const VexScan &scan) const;
	bool removeScan(const std::string &name);

	unsigned int nAntenna() const { return antennas.size(); }
	int getAntennaIdByName(const std::string &antName) const;
	int getAntennaIdByDefName(const std::string &antName) const;
	const VexAntenna *getAntenna(unsigned int num) const;
	const VexAntenna *getAntenna(const std::string &name) const;
	double getAntennaStartMJD(const std::string &name) const;
	double getAntennaStopMJD(const std::string &name) const;
	void setAntennaPosition(const std::string &antName, double X, double Y, double Z);
	void setAntennaAxisOffset(const std::string &antName, double axisOffset);
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

	void addVSN(const std::string &antName, unsigned int datastreamId, const std::string &vsn, const Interval &timeRange);
	// If datastreamId < 0, remove data from all associated datastreams
	void removeBasebandData(const std::string &antName, int datastreamId);

	const VexExper *getExper() const { return &exper; }
	void setExper(const std::string &name, const Interval &experTimeRange);
};

std::ostream& operator << (std::ostream &os, const VexData &x);

#endif
