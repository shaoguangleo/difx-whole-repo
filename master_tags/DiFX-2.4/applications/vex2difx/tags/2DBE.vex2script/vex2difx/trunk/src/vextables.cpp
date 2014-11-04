/***************************************************************************
 *   Copyright (C) 2009-2012 by Walter Brisken & Adam Deller               *
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

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "vextables.h"

const double RAD2ASEC=180.0*3600.0/M_PI;

using namespace std;

// Note: the ordering here is crucial!
const char VexEvent::eventName[][20] =
{
	"None",
	"Ant Off-source",
	"Scan Stop",
	"Job Stop",
	"Observe Stop",
	"Record Stop",
	"Clock Break",
	"Leap Second",
	"Manual Break",
	"Record Start",
	"Observe Start",
	"Job Start",
	"Scan Start",
	"Ant On-source"
};


bool operator<(const VexEvent &a, const VexEvent &b)
{
	if(a.mjd < b.mjd - 0.000001)
	{
		return true;
	}
	else if(a.mjd > b.mjd + 0.000001)
	{
		return false;
	}
	if(a.eventType < b.eventType)
	{
		return true;
	}
	else if(a.eventType > b.eventType)
	{
		return false;
	}

	return a.name < b.name;
}

// returns a negative number if no overlap
double VexInterval::overlap(const VexInterval &v) const
{
	return min(mjdStop, v.mjdStop) - max(mjdStart, v.mjdStart);
}

void VexInterval::logicalAnd(double start, double stop)
{
	if(mjdStart < start)
	{
		mjdStart = start;
	}
	if(mjdStop > stop)
	{
		mjdStop = stop;
	}
}

void VexInterval::logicalAnd(const VexInterval &v)
{
	if(mjdStart < v.mjdStart)
	{
		mjdStart = v.mjdStart;
	}
	if(mjdStop > v.mjdStop)
	{
		mjdStop = v.mjdStop;
	}
}

void VexInterval::logicalOr(double start, double stop)
{
	if(mjdStart > start)
	{
		mjdStart = start;
	}
	if(mjdStop < stop)
	{
		mjdStop = stop;
	}
}

void VexInterval::logicalOr(const VexInterval &v)
{
	if(mjdStart > v.mjdStart)
	{
		mjdStart = v.mjdStart;
	}
	if(mjdStop < v.mjdStop)
	{
		mjdStop = v.mjdStop;
	}
}

void VexChannel::selectTones(int toneIntervalMHz, enum ToneSelection selection, double guardBandMHz)
{
	double epsilonHz = 1.0;
	int tonesInBand;
	int firstToneMHz;

	if(toneIntervalMHz <= 0)
	{
		return;
	}

	if(guardBandMHz < 0.0)
	{
		guardBandMHz = bbcBandwidth*1.0e-6/8.0;	// default to 1/8 of the band
	}

	if(bbcSideBand == 'U')
	{
		firstToneMHz = static_cast<int>((bbcFreq + epsilonHz)*1.0e-6 + toneIntervalMHz);
		tonesInBand = static_cast<int>((bbcFreq + bbcBandwidth)*1.0e-6 - firstToneMHz)/toneIntervalMHz + 1;
	}
	else
	{
		firstToneMHz = static_cast<int>((bbcFreq - epsilonHz)*1.0e-6);
		tonesInBand = static_cast<int>(firstToneMHz - (bbcFreq - bbcBandwidth)*1.0e-6)/toneIntervalMHz + 1;
	}

	if(selection == ToneSelectionVex)
	{
		vector<int>::iterator it;

		// Here what we do is turn negative tone indices (i.e., counting from end of band) to positive ones
		for(it = tones.begin(); it != tones.end(); ++it)
		{
			if(*it < 0)
			{
				*it = tonesInBand + *it;	// For 8 tones: -1 -> 7, -2 -> 6, ...
			}
		}
		sort(tones.begin(), tones.end());
	}
	else
	{
		tones.clear();
	}

	switch(selection)
	{
	case ToneSelectionVex:
		// Nothing to do
		break;
	case ToneSelectionNone:
		// Nothing to do
		break;
	case ToneSelectionEnds:
		tones.push_back(0);
		tones.push_back(tonesInBand - 1);
		break;
	case ToneSelectionAll:
		for(int i = 0; i < tonesInBand; ++i)
		{
			tones.push_back(i);
		}
		break;
	case ToneSelectionSmart:
		for(int i = 0; i < tonesInBand; ++i)
		{
			if(bbcSideBand == 'U')
			{
				double f = firstToneMHz + i*toneIntervalMHz;
				if(f > (bbcFreq*1.0e-6+guardBandMHz) && f < ((bbcFreq+bbcBandwidth)*1.0e-6-guardBandMHz))
				{
					if(tones.size() < 2)
					{
						tones.push_back(i);
					}
					else
					{
						tones[1] = i;
					}
				}
			}
			else
			{
				double f = firstToneMHz - i*toneIntervalMHz;
				if(f < (bbcFreq*1.0e-6-guardBandMHz) && f > ((bbcFreq-bbcBandwidth)*1.0e-6+guardBandMHz))
				{
					if(tones.size() < 2)
					{
						tones.push_back(i);
					}
					else
					{
						tones[1] = i;
					}
				}
			}
		}
		if(tones.size() < 2 && guardBandMHz > bbcBandwidth*1.0e-6/200)
		{
			// If not enough tones are found, recurse a bit...
			selectTones(toneIntervalMHz, selection, guardBandMHz/2.0);	
		}
		break;
	case ToneSelectionMost:
		for(int i = 0; i < tonesInBand; ++i)
		{
			if(bbcSideBand == 'U')
			{
				double f = firstToneMHz + i*toneIntervalMHz;
				if(f > (bbcFreq*1.0e-6+guardBandMHz) && f < ((bbcFreq+bbcBandwidth)*1.0e-6-guardBandMHz))
				{
					tones.push_back(i);
				}
			}
			else
			{
				double f = firstToneMHz - i*toneIntervalMHz;
				if(f < (bbcFreq*1.0e-6-guardBandMHz) && f > ((bbcFreq-bbcBandwidth)*1.0e-6+guardBandMHz))
				{
					tones.push_back(i);
				}
			}
		}
		if(tones.size() < 2 && guardBandMHz > bbcBandwidth*1.0e-6/200)
		{
			// If not enough tones are found, recurse a bit...
			selectTones(toneIntervalMHz, selection, guardBandMHz/2.0);	
		}
		break;
	default:
		cerr << "Error: selectTones: unexpected value of selection: " << selection << endl;
		
		exit(EXIT_FAILURE);
	}
}

int VexMode::addSubband(double freq, double bandwidth, char sideband, char pol)
{
	VexSubband S(freq, bandwidth, sideband, pol);

	for(vector<VexSubband>::const_iterator it = subbands.begin(); it != subbands.end(); ++it)
	{
		if(S == *it)
		{
			return it - subbands.begin();
		}
	}

	subbands.push_back(S);

	return subbands.size() - 1;
}

#if 0
int VexMode::getOversampleFactor() const
{
	return static_cast<int>(sampRate/(2.0*subbands[0].bandwidth) + 0.001);
}
#endif

int VexMode::getPols(char *pols) const
{
	int n=0;
	bool L=false, R=false, X=false, Y=false;
	vector<VexSubband>::const_iterator it;

	for(it = subbands.begin(); it != subbands.end(); ++it)
	{
		if(it->pol == 'R')
		{
			R = true;
		}
		else if(it->pol == 'L')
		{
			L = true;
		}
		else if(it->pol == 'X')
		{
			X = true;
		}
		else if(it->pol == 'Y')
		{
			Y = true;
		}
		else
		{
			cerr << "Error: VexMode::getPols: subband with illegal polarization (" << it->pol << ") encountered." << endl;
			
			exit(EXIT_FAILURE);
		}
	}

	if(R) 
	{
		*pols = 'R';
		++pols;
		++n;
	}
	if(L)
	{
		*pols = 'L';
		++pols;
		++n;
	}
	if(n)
	{
		return n;
	}
	if(X) 
	{
		*pols = 'X';
		++pols;
		++n;
	}
	if(Y)
	{
		*pols = 'Y';
		++pols;
		++n;
	}

	return n;
}

int VexMode::getBits() const
{
	static int firstTime = 1;
	unsigned int nBit = 0;
	map<string,VexSetup>::const_iterator it;

	nBit = setups.begin()->second.nBit;

	for(it = setups.begin(); it != setups.end(); ++it)
	{
		if(it->second.nBit != nBit)
		{
			if(nBit != 0 && it->second.nBit != 0 && firstTime)
			{
				cerr << "Warning: getBits: differing number of bits: " << nBit << "," << it->second.nBit << endl;
				cerr << "  Will proceed, but note that some metadata may be incorrect." << endl;

				firstTime = 0;
			}

			if(it->second.nBit > nBit)
			{
				nBit = it->second.nBit;
			}
		}

	}

	return nBit;
}

const VexSetup* VexMode::getSetup(const string &antName) const
{
	map<string,VexSetup>::const_iterator it;

	it = setups.find(antName);
	if(it == setups.end())
	{
		cerr << "Error: VexMode::getSetup: antName=" << antName << " not found." << endl;
		
		exit(EXIT_FAILURE);
	}

	return &it->second;
}

double VexMode::getLowestSampleRate() const
{
	if(setups.empty())
	{
		return 0.0;
	}
	else
	{
		double sr = 1.0e30;	// A very large number
		
		for(map<string,VexSetup>::const_iterator it = setups.begin(); it != setups.end(); ++it)
		{
			if(it->second.sampRate < sr && it->second.sampRate > 0.0)
			{
				sr = it->second.sampRate;
			}
		}

		if(sr > 1.0e29)
		{
			sr = 0.0;
		}

		return sr;
	}
}

double VexMode::getHighestSampleRate() const
{
	if(setups.empty())
	{
		return 0.0;
	}
	else
	{
		double sr = 0.0;
		
		for(map<string,VexSetup>::const_iterator it = setups.begin(); it != setups.end(); ++it)
		{
			if(it->second.sampRate > sr)
			{
				sr = it->second.sampRate;
			}
		}

		return sr;
	}
}

double VexMode::getAverageSampleRate() const
{
	if(setups.empty())
	{
		return 0.0;
	}
	else
	{
		double sr = 0.0;
		
		for(map<string,VexSetup>::const_iterator it = setups.begin(); it != setups.end(); ++it)
		{
			sr += it->second.sampRate;
		}

		sr /= setups.size();

		return sr;
	}
}

double VexIF::getLowerEdgeFreq() const
{
	double bandCenter = ifSSLO;

	// Calculate the center of the 500-1000 MHz IF range;
	if(ifSideBand == 'L')
	{
		bandCenter -= 750.0e6;
	}
	else
	{
		bandCenter += 750.0e6;
	}

	return bandCenter - 500.0e6;
}

char VexChannel::bandCode() const
{
	if(bbcFreq < 1.0e9)
	{
		return 'P';
	}
	else if(bbcFreq < 2.0e9)
	{
		return 'L';
	}
	else if(bbcFreq < 3.0e9)
	{
		return 'S';
	}
	else if(bbcFreq < 6.0e9)
	{
		return 'C';
	}
	else if(bbcFreq < 9.5e9)
	{
		return 'X';
	}
	else if(bbcFreq < 17.0e9)
	{
		return 'U';
	}
	else if(bbcFreq < 25.0e9)
	{
		return 'K';
	}
	else if(bbcFreq < 40.5e9)
	{
		return 'A';
	}
	else if(bbcFreq < 60.0e9)
	{
		return 'Q';
	}
	else if(bbcFreq < 100.0e9)
	{
		return 'W';
	}

	return '?';
}

string VexIF::VLBABandName() const
{
	double bandCenter = ifSSLO;

	// Calculate the center of the 500-1000 MHz IF range;
	if(ifSideBand == 'L')
	{
		bandCenter -= 750.0e6;
	}
	else
	{
		bandCenter += 750.0e6;
	}

	if(bandCenter < 1.0e9)
	{
		return "90cm";
	}
	else if(bandCenter < 2.0e9)
	{
		return "20cm";
	}
	else if(bandCenter < 3.0e9)
	{
		return "13cm";
	}
	else if(bandCenter < 6.0e9)
	{
		return "6cm";
	}
	else if(bandCenter < 9.5e9)
	{
		return "4cm";
	}
	else if(bandCenter < 17.0e9)
	{
		return "2cm";
	}
	else if(bandCenter < 25.0e9)
	{
		return "1cm";
	}
	else if(bandCenter < 40.5e9)
	{
		return "9mm";
	}
	else if(bandCenter < 60.0e9)
	{
		return "7mm";
	}
	else if(bandCenter < 100.0e9)
	{
		return "3mm";
	}

	return "None";
}

bool operator == (const VexSubband &s1, const VexSubband &s2)
{
	if(s1.pol       != s2.pol      ||
	   s1.freq      != s2.freq     ||
	   s1.sideBand  != s2.sideBand ||
	   s1.bandwidth != s2.bandwidth)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool VexSource::hasSourceName(const string &name) const
{
	// if result of find is .end(), then it is not in the list
	return find(sourceNames.begin(), sourceNames.end(), name) != sourceNames.end();
}

int VexData::sanityCheck()
{
        int nWarn = 0;

        if(eops.size() < 5)
        {
                cerr << "Warning: Fewer than 5 EOPs specified" << endl;
                ++nWarn;
        }

        for(vector<VexAntenna>::const_iterator it = antennas.begin(); it != antennas.end(); ++it)
        {
                if(it->clocks.empty())
                {
                        cerr << "Warning: no clock values for antenna " << it->name << " ." << endl;
                        ++nWarn;
                }
        }

        for(vector<VexAntenna>::const_iterator it = antennas.begin(); it != antennas.end(); ++it)
        {
		if(it->dataSource == DataSourceFile && it->basebandFiles.empty())
		{
			cerr << "Warning: file based correlation desired but no files provided for antenna " << it->name << " ." << endl;
			++nWarn;
		}
		if(it->dataSource == DataSourceModule && it->basebandFiles.empty())
		{
			cerr << "Warning: module based correlation desired but no media specified for antenna " << it->name << " ." << endl;
			++nWarn;
		}
		if(it->dataSource == DataSourceNone)
		{
			cerr << "Warning: data source is NONE for antenna " << it->name << " ." << endl;
			++nWarn;
		}
        }

        return nWarn;
}

VexSource *VexData::newSource()
{
	sources.push_back(VexSource());

	return &sources.back();
}

// get the clock epoch as a MJD value (with fractional component), negative 
// means not found.  Also fills in the first two coeffs, returned in seconds
double VexAntenna::getVexClocks(double mjd, double *coeffs) const
{
	double epoch = -1.0;

	for(vector<VexClock>::const_iterator it = clocks.begin(); it != clocks.end(); ++it)
	{
		if(it->mjdStart <= mjd)
		{
			epoch = it->offset_epoch;
			coeffs[0] = it->offset;
			coeffs[1] = it->rate;
		}
	}

	return epoch;
}

const VexSource *VexData::getSource(unsigned int num) const
{
	if(num >= nSource())
	{
		return 0;
	}

	return &sources[num];
}

int VexData::getSourceIdByDefName(const string &defName) const
{
	for(vector<VexSource>::const_iterator it = sources.begin(); it != sources.end(); ++it)
	{
		if(it->defName == defName)
		{
			return it - sources.begin();
		}
	}

	return -1;
}

const VexSource *VexData::getSourceByDefName(const string &defName) const
{
	for(vector<VexSource>::const_iterator it = sources.begin(); it != sources.end(); ++it)
	{
		if(it->defName == defName)
		{
			return &(*it);
		}
	}

	return 0;
}

const VexSource *VexData::getSourceBySourceName(const string &name) const
{
	for(vector<VexSource>::const_iterator it = sources.begin(); it != sources.end(); ++it)
	{
		if(find(it->sourceNames.begin(), it->sourceNames.end(), name) != it->sourceNames.end())
		{
			return &(*it);
		}
	}

	return 0;
}

VexScan *VexData::newScan()
{
	scans.push_back(VexScan());

	return &scans.back();
}

void VexJob::assignVSNs(const VexData &V)
{
	list<string> antennas;

	for(vector<string>::const_iterator s = scans.begin(); s != scans.end(); ++s)
	{
		const VexScan* S = V.getScanByDefName(*s);
		for(map<string,VexInterval>::const_iterator a = S->stations.begin(); a != S->stations.end(); ++a)
		{
			if(find(antennas.begin(), antennas.end(), a->first) == antennas.end())
			{
				antennas.push_back(a->first);
			}
		}
	}
	
	for(list<string>::const_iterator a = antennas.begin(); a != antennas.end(); ++a)
	{
		if(V.getAntenna(*a)->dataSource != DataSourceModule)
		{
			vsns[*a] = "None";
		}
		else
		{
			const string &vsn = V.getVSN(*a, *this);
			if(vsn != "None")
			{
				vsns[*a] = vsn;
			}
		}
	}
}

string VexJob::getVSN(const string &antName) const
{
	for(map<string,string>::const_iterator a = vsns.begin(); a != vsns.end(); ++a)
	{
		if(a->first == antName)
		{
			return a->second;
		}
	}

	return string("None");
}

/* Modified from http://www-graphics.stanford.edu/~seander/bithacks.html */
static int intlog2(unsigned int v)
{
	const unsigned int b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
	const unsigned int S[] = {1, 2, 4, 8, 16};
	unsigned int r = 0; // result of log2(v) will go here

	for(int i = 4; i >= 0; --i) 
	{
		if(v & b[i])
		{
			v >>= S[i];
			r |= S[i];
		} 
	}

	return r;
}

double VexJob::calcOps(const VexData *V, int fftSize, bool doPolar) const
{
	double ops = 0.0;
	int nAnt, nPol, nSubband;
	double sampRate, seconds;
	const VexMode *M;
	char pols[8];
	double opsPerSample;

	for(vector<string>::const_iterator si = scans.begin(); si != scans.end(); ++si)
	{
		const VexScan *S = V->getScanByDefName(*si);
		M = V->getModeByDefName(S->modeDefName);
		if(!M)
		{
			return 0.0;
		}
		
		sampRate = M->getAverageSampleRate();
		nPol = M->getPols(pols);
		if(nPol > 1 && doPolar)
		{
			nPol = 2;
		}
		else
		{
			nPol = 1;
		}

		seconds = S->duration_seconds();
		nAnt = S->stations.size();
		nSubband = M->subbands.size();

		// Estimate number of operations based on VLBA Sensitivity Upgrade Memo 16
		// Note: currently this does not consider oversampling
		// Note: this assumes all polarizations are matched
		opsPerSample = 16.0 + 5.0*intlog2(fftSize) + 2.5*nAnt*nPol;
		ops += opsPerSample*seconds*sampRate*nSubband*nAnt;
	}

	return ops;
}

double VexJob::calcSize(const VexData *V) const
{
	double size = 0.0;

	for(vector<string>::const_iterator it = scans.begin(); it != scans.end(); ++it)
	{
		size += V->getScanByDefName(*it)->size;
	}

	return size;
}

bool VexJobGroup::hasScan(const string &scanName) const
{
	return find(scans.begin(), scans.end(), scanName) != scans.end();
}

void VexJobGroup::genEvents(const list<VexEvent> &eventList)
{
	for(list<VexEvent>::const_iterator it = eventList.begin(); it != eventList.end(); ++it)
	{
		if(it->eventType == VexEvent::SCAN_START ||
		   it->eventType == VexEvent::SCAN_STOP ||
		   it->eventType == VexEvent::ANT_SCAN_START ||
		   it->eventType == VexEvent::ANT_SCAN_STOP)
		{
			if(hasScan(it->scan))
			{
				events.push_back(*it);
			}
		}
		else
		{
			events.push_back(*it);
		}
	}

	// Now remove any module changes that don't occur within scans

	map<string,bool> inScan;
	map<string,bool> inScanNow;

	// initialize inScan

	for(list<VexEvent>::const_iterator it = events.begin(); it != events.end(); ++it)
	{
		if(it->eventType == VexEvent::RECORD_START)
		{
			inScan[it->name] = false;
			inScanNow[it->name] = false;
		}
	}

	list<VexEvent>::iterator rstart, rstop;
	for(rstart = events.begin(); rstart != events.end();)
	{
		if(rstart->eventType == VexEvent::ANT_SCAN_START)
		{
			inScan[rstart->name] = true;
			inScanNow[rstart->name] = true;
		}
		else if(rstart->eventType == VexEvent::ANT_SCAN_STOP)
		{
			inScanNow[rstart->name] = false;
		}
		if(rstart->eventType == VexEvent::RECORD_START && !inScanNow[rstart->name])
		{
			inScan[rstart->name] = inScanNow[rstart->name];
			for(rstop = rstart, ++rstop; rstop != events.end(); ++rstop)
			{
				if(rstart->name != rstop->name)
				{
					continue;
				}

				if(rstop->eventType == VexEvent::ANT_SCAN_START)
				{
					inScan[rstart->name] = true;
				}

				if(rstop->eventType == VexEvent::RECORD_STOP)
				{
					if(!inScan[rstop->name])
					{
						inScan[rstop->name] = inScanNow[rstop->name];
						events.erase(rstop);
						rstart = events.erase(rstart);
					}
					else
					{
						++rstart;
					}

					break;
				}
			}
		}
		else
		{
			++rstart;
		}
	}
}

bool VexJob::hasScan(const string &scanName) const
{
	// if find returns .end(), then it was not found
	return find(scans.begin(), scans.end(), scanName) != scans.end();
}

int VexJob::generateFlagFile(const VexData &V, const char *fileName, unsigned int invalidMask) const
{
	vector<VexJobFlag> flags;
	map<string,int> antIds;
	unsigned int nAnt = 0;
	ofstream of;
	const list<VexEvent> &eventList = *V.getEvents();

	for(map<string,string>::const_iterator a = vsns.begin(); a != vsns.end(); ++a)
	{
		antIds[a->first] = nAnt;
		++nAnt;
	}

	// Assume all flags from the start.  
	vector<unsigned int> flagMask(nAnt, 
		VexJobFlag::JOB_FLAG_RECORD | 
		VexJobFlag::JOB_FLAG_POINT | 
		VexJobFlag::JOB_FLAG_TIME | 
		VexJobFlag::JOB_FLAG_SCAN);
	vector<double> flagStart(nAnt, mjdStart);

	// Except if not a Mark5 Module case, don't assume RECORD flag is on
	for(map<string,string>::const_iterator a = vsns.begin(); a != vsns.end(); ++a)
	{
		const VexAntenna *ant = V.getAntenna(a->first);

		if(!ant)
		{
			cerr << "Developer error: generateFlagFile: antenna " << a->first << " not found in antenna table." << endl;

			exit(EXIT_FAILURE);
		}

		if(!ant->basebandFiles.empty())
		{
			// Aha! not module based so unflag JOB_FLAG_RECORD
			flagMask[antIds[a->first]] &= ~VexJobFlag::JOB_FLAG_RECORD;
		}
	}

	// Then go through each event, adjusting current flag state.  
	for(list<VexEvent>::const_iterator e = eventList.begin(); e != eventList.end(); ++e)
	{
		if(e->eventType == VexEvent::RECORD_START)
		{
			if(antIds.count(e->name) > 0)
			{
				flagMask[antIds[e->name]] &= ~VexJobFlag::JOB_FLAG_RECORD;
			}
		}
		else if(e->eventType == VexEvent::RECORD_STOP)
		{
			if(antIds.count(e->name) > 0)
			{
				flagMask[antIds[e->name]] |= VexJobFlag::JOB_FLAG_RECORD;
			}
		}
		else if(e->eventType == VexEvent::SCAN_START)
		{
			if(hasScan(e->scan))
			{
				const VexScan *scan = V.getScanByDefName(e->scan);

				if(!scan)
				{
					cerr << "Developer error: generateFlagFile: SCAN_START, scan=0" << endl;

					exit(EXIT_FAILURE);
				}
				for(map<string,VexInterval>::const_iterator sa = scan->stations.begin(); sa != scan->stations.end(); ++sa)
				{
					if(antIds.count(sa->first) == 0)
					{
						continue;
					}
					flagMask[antIds[sa->first]] &= ~VexJobFlag::JOB_FLAG_SCAN;
				}
			}
		}
		else if(e->eventType == VexEvent::SCAN_STOP)
		{
			if(hasScan(e->scan))
			{
				const VexScan *scan = V.getScanByDefName(e->scan);

				if(!scan)
				{
					cerr << "Developer error! generateFlagFile: SCAN_STOP, scan=0" << endl;

					exit(EXIT_FAILURE);
				}
				for(map<string,VexInterval>::const_iterator sa = scan->stations.begin(); sa != scan->stations.end(); ++sa)
				{
					if(antIds.count(sa->first) == 0)
					{
						continue;
					}
					flagMask[antIds[sa->first]] |= VexJobFlag::JOB_FLAG_SCAN;
				}
			}
		}
		else if(e->eventType == VexEvent::ANT_SCAN_START)
		{
			if(hasScan(e->scan) && antIds.count(e->name) > 0)
			{
				flagMask[antIds[e->name]] &= ~VexJobFlag::JOB_FLAG_POINT;
			}
		}
		else if(e->eventType == VexEvent::ANT_SCAN_STOP)
		{
			if(hasScan(e->scan) && antIds.count(e->name) > 0)
			{
				flagMask[antIds[e->name]] |= VexJobFlag::JOB_FLAG_POINT;
			}
		}
		else if(e->eventType == VexEvent::JOB_START)
		{
			if(fabs(e->mjd - mjdStart) < 0.5/86400.0)
			{
				for(map<string,string>::const_iterator a = vsns.begin(); a != vsns.end(); ++a)
				{
					flagMask[antIds[a->first]] &= ~VexJobFlag::JOB_FLAG_TIME;
				}
			}
		}
		else if(e->eventType == VexEvent::JOB_STOP)
		{
			if(fabs(e->mjd - mjdStart) < 0.5/86400.0)
			{
				for(map<string,string>::const_iterator a = vsns.begin(); a != vsns.end(); ++a)
				{
					flagMask[antIds[a->first]] |= VexJobFlag::JOB_FLAG_TIME;
				}
			}
		}

		for(unsigned int antId = 0; antId < nAnt; ++antId)
		{
			if( (flagMask[antId] & invalidMask) == 0)
			{
				if(flagStart[antId] > 0)
				{
					if(e->mjd - flagStart[antId] > 0.5/86400.0)
					{
						VexJobFlag f(flagStart[antId], e->mjd, antId);
						// only add flag if it overlaps in time with this job
						if(overlap(f))
						{
							flags.push_back(f);
						}
					}
					flagStart[antId] = -1;
				}
			}
			else
			{
				if(flagStart[antId] <= 0)
				{
					flagStart[antId] = e->mjd;
				}
			}
		}
	}

	// At end of loop see if any flag->unflag (or vice-versa) occurs.
	for(unsigned int antId = 0; antId < nAnt; ++antId)
	{
		if( (flagMask[antId] & invalidMask) != 0)
		{
			if(mjdStop - flagStart[antId] > 0.5/86400.0)
			{
				VexJobFlag f(flagStart[antId], mjdStop, antId);
				// only add flag if it overlaps in time with this job
				if(overlap(f))
				{
					flags.push_back(f);
				}
			}
		}
	}

	// write data to file
	of.open(fileName);
	of << flags.size() << endl;
	for(vector<VexJobFlag>::const_iterator it = flags.begin(); it != flags.end(); ++it)
	{
		of << "  " << *it << endl;
	}
	of.close();

	return flags.size();
}

void VexJobGroup::createJobs(vector<VexJob> &jobs, VexInterval &jobTimeRange, const VexData *V, double maxLength, double maxSize) const
{
	list<VexEvent>::const_iterator s, e;
	jobs.push_back(VexJob());
	VexJob *J = &jobs.back();
	double totalTime, scanTime = 0.0;
	double size = 0.0;
	string id("");

	// note these are backwards now; will set these to minimum range covering scans
	J->setTimeRange(jobTimeRange.mjdStop, jobTimeRange.mjdStart);

	for(e = events.begin(); e != events.end(); ++e)
	{
		if(e->eventType == VexEvent::SCAN_START)
		{
			s = e;
			id = e->name;
		}
		if(e->eventType == VexEvent::SCAN_STOP)
		{
			if(id != e->name)
			{
				cerr << "Programming error: createJobs: id != e->name  (" << id << " != " << e->name << ")" << endl;
				cerr << "Contact developer" << endl;

				exit(EXIT_FAILURE);
			}
			VexInterval scanTimeRange(s->mjd, e->mjd);
			scanTimeRange.logicalAnd(jobTimeRange);
			if(scanTimeRange.duration() > 0.0)
			{
				J->scans.push_back(e->name);
				J->logicalOr(scanTimeRange);
				scanTime += scanTimeRange.duration();

				// Work in progress: calculate correlated size of scan
				size += V->getScanByDefName(id)->size;

				/* start a new job at scan boundary if maxLength exceeded */
				if(J->duration() > maxLength || size > maxSize)
				{
					totalTime = J->duration();
					J->dutyCycle = scanTime / totalTime;
					J->dataSize = size;
					jobs.push_back(VexJob());
					J = &jobs.back();
					scanTime = 0.0;
					size = 0.0;
					J->setTimeRange(jobTimeRange.mjdStop, jobTimeRange.mjdStart);
				}
			}
		}
	}

	totalTime = J->duration();
	
	if(totalTime <= 0.0)
	{
		jobs.pop_back();
	}
	else
	{
		J->dutyCycle = scanTime / totalTime;
		J->dataSize = size;
	}
}

const VexScan *VexData::getScan(unsigned int num) const
{
	if(num >= nScan())
	{
		return 0;
	}

	return &scans[num];
}

const VexScan *VexData::getScanByDefName(const string &defName) const
{
	for(vector<VexScan>::const_iterator it = scans.begin(); it != scans.end(); ++it)
	{
		if(it->defName == defName)
		{
			return &(*it);
		}
	}

	return 0;
}

unsigned int VexScan::nAntennasWithRecordedData(const VexData *V) const
{
	unsigned int nAnt = 0;

	const VexMode *M = V->getModeByDefName(modeDefName);
	if(!M)
	{
		return 0;
	}

	for(map<string,VexInterval>::const_iterator it = stations.begin(); it != stations.end(); ++it)
	{
		map<string,VexSetup>::const_iterator S = M->setups.find(it->first);
		if(S != M->setups.end() && S->second.nRecordChan > 0 && S->second.formatName != "NONE")
		{
			++nAnt;
		}
	}

	return nAnt;
}

unsigned int VexScan::nRecordChan(const VexData *V, const string &antName) const
{
	unsigned int nRecChan = 0;
	const VexMode *M = V->getModeByDefName(modeDefName);

	if(M)
	{
		map<string,VexSetup>::const_iterator it = M->setups.find(antName);
		if(it != M->setups.end())
		{
			nRecChan = it->second.nRecordChan;
		}
		else
		{
			cerr << "Warning: Ant " << antName << " not found in mode " << M->defName << endl;
		}
	}

	return nRecChan;
}

void VexData::setScanSize(unsigned int num, double size)
{
	if(num >= nScan())
	{
		return;
	}
	
	scans[num].size = size;
}

void VexData::getScanList(list<string> &scanList) const
{
	for(vector<VexScan>::const_iterator it = scans.begin(); it != scans.end(); ++it)
	{
		scanList.push_back(it->defName);
	}
}

int VexEOP::setkv(const string &key, const string &value)
{
	stringstream ss;
	int nWarn = 0;

	ss << value;

	if(key == "tai_utc")
	{
		ss >> tai_utc;
	}
	else if(key == "ut1_utc")
	{
		ss >> ut1_utc;
	}
	else if(key == "xPole")
	{
		ss >> xPole;
		xPole /= RAD2ASEC;
	}
	else if(key == "yPole")
	{
		ss >> yPole;
		yPole /= RAD2ASEC;
	}
	else
	{
		cerr << "Warning: EOP: Unknown parameter '" << key << "'." << endl;
		++nWarn;
	}

	return nWarn;
}

VexAntenna *VexData::newAntenna()
{
	antennas.push_back(VexAntenna());

	return &antennas.back();
}

const VexAntenna *VexData::getAntenna(unsigned int num) const
{
	if(num >= nAntenna())
	{
		return 0;
	}

	return &antennas[num];
}

const VexAntenna *VexData::getAntenna(const string &name) const
{
	for(vector<VexAntenna>::const_iterator it = antennas.begin(); it != antennas.end(); ++it)
	{
		if(it->name == name)
		{
			return &(*it);
		}
	}

	return 0;
}

int VexSetup::phaseCalIntervalMHz() const
{
	int p;
	int pc = 0;

	for(map<string,VexIF>::const_iterator it = ifs.begin(); it != ifs.end(); ++it)
	{
		p = it->second.phaseCalIntervalMHz;
		if(p > 0 && (p < pc || pc == 0))
		{
			pc = p;
		}
	}

	return pc;
}

const VexIF *VexSetup::getIF(const string &ifName) const
{
	for(map<string,VexIF>::const_iterator it = ifs.begin(); it != ifs.end(); ++it)
	{
		if(it->second.name == ifName)
		{
			return &it->second;
		}
	}

	return 0;
}

double VexSetup::firstTuningForIF(const string &ifName) const	// return Hz
{
	double tune = 0.0;
	string cn;

	for(vector<VexChannel>::const_iterator ch=channels.begin(); ch != channels.end(); ++ch)
	{
		if(ch->ifName == ifName && (cn == "" || ch->bbcName < cn))
		{
			cn = ch->bbcName;
			tune = ch->bbcFreq;
		}
	}

	return tune;
}

bool operator ==(const VexChannel &c1, const VexChannel &c2)
{
	if( (c1.recordChan  != c2.recordChan)   ||
	    (c1.subbandId   != c2.subbandId)    ||
	    (c1.ifName      != c2.ifName)       ||
	    (c1.bbcFreq     != c2.bbcFreq)      ||
	    (c1.bbcSideBand != c2.bbcSideBand)  ||
	    (c1.tones       != c2.tones) )
	{
		return false;
	}

	return true;
}

VexMode *VexData::newMode()
{
	modes.push_back(VexMode());

	return &modes.back();
}

const VexMode *VexData::getMode(unsigned int num) const
{
	if(num >= nMode())
	{
		return 0;
	}

	return &modes[num];
}

const VexMode *VexData::getModeByDefName(const string &defName) const
{
	for(vector<VexMode>::const_iterator it = modes.begin(); it != modes.end(); ++it)
	{
		if(it->defName == defName)
		{
			return &(*it);
		}
	}

	return 0;
}

int VexData::getModeIdByDefName(const string &defName) const
{
	for(vector<VexMode>::const_iterator it = modes.begin(); it != modes.end(); ++it)
	{
		if(it->defName == defName)
		{
			return it - modes.begin();
		}
	}

	return -1;
}


VexEOP *VexData::newEOP()
{
	eops.push_back(VexEOP());

	return &eops.back();
}

const VexEOP *VexData::getEOP(unsigned int num) const
{
	if(num > nEOP())
	{
		return 0;
	}

	return &eops[num];
}

bool VexData::usesAntenna(const string &antennaName) const
{
	unsigned int n = nAntenna();

	for(unsigned int i = 0; i < n; ++i)
	{
		if(getAntenna(i)->name == antennaName)
		{
			return true;
		}
	}

	return false;
}

bool VexData::usesMode(const string &modeDefName) const
{
	unsigned int n = nScan();

	for(unsigned int i = 0; i < n; ++i)
	{
		if(getScan(i)->modeDefName == modeDefName)
		{
			return true;
		}
	}

	return false;
}

unsigned int VexData::nVSN(const string &antName) const
{
	const VexAntenna *A;

	A = getAntenna(antName);
	if(!A)
	{
		return 0;
	}
	else
	{
		return A->basebandFiles.size();
	}
}

void VexData::addVSN(const string &antName, const string &vsn, const VexInterval &timeRange)
{
	for(vector<VexAntenna>::iterator it = antennas.begin(); it != antennas.end(); ++it)
	{
		if(it->name == antName)
		{
			it->basebandFiles.push_back(VexBasebandFile(vsn, timeRange));
			it->dataSource = DataSourceModule;
		}
	}
}

string VexData::getVSN(const string &antName, const VexInterval &timeRange) const
{
	const VexAntenna *A;
	double best = 0.0;
	string bestVSN("None");

	A = getAntenna(antName);
	if(!A)
	{
		return bestVSN;
	}

	if(A->dataSource != DataSourceModule)
	{
		return bestVSN;
	}

	for(vector<VexBasebandFile>::const_iterator v = A->basebandFiles.begin(); v != A->basebandFiles.end(); ++v)
	{
		double timeOverlap = timeRange.overlap(*v);
		if(timeOverlap > best)
		{
			best = timeOverlap;
			bestVSN = v->filename;
		}
	}

	return bestVSN;
}

void VexData::setExper(const string &name, const VexInterval &experTimeRange)
{
	double a=1.0e7, b=0.0;

	for(list<VexEvent>::const_iterator it = events.begin(); it != events.end(); ++it)
	{
		if(it->mjd < a && it->eventType != VexEvent::CLOCK_BREAK)
		{
			a = it->mjd;
		}
		if(it->mjd > b && it->eventType != VexEvent::CLOCK_BREAK)
		{
			b = it->mjd;
		}
	}

	exper.name = name;
	exper.setTimeRange(experTimeRange);
	if(exper.mjdStart < 10000)
	{
		exper.mjdStart = a;
	}
	if(exper.mjdStop < 10000)
	{
		exper.mjdStop = b;
	}

	addEvent(exper.mjdStart, VexEvent::OBSERVE_START, name); 
	addEvent(exper.mjdStop, VexEvent::OBSERVE_STOP, name); 
}

const list<VexEvent> *VexData::getEvents() const
{
	return &events;
}

void VexData::addEvent(double mjd, VexEvent::EventType eventType, const string &name)
{
	events.push_back(VexEvent(mjd, eventType, name));
}

void VexData::addEvent(double mjd, VexEvent::EventType eventType, const string &name, const string &scan)
{
	events.push_back(VexEvent(mjd, eventType, name, scan));
}

void VexData::sortEvents()
{
	events.sort();
}

void VexData::findLeapSeconds()
{
        int n = eops.size();

        if(n < 2)
        {
                return;
        }

        for(int i = 1; i < n; ++i)
        {
                if(eops[i-1].tai_utc != eops[i].tai_utc)
                {
                        addEvent(eops[i].mjd, VexEvent::LEAP_SECOND, "Leap second");
                        cout << "Leap second detected at day " << eops[i].mjd << endl;
                }
        }
}

void VexData::addBreaks(const vector<double> &breaks)
{
	for(vector<double>::const_iterator t = breaks.begin(); t != breaks.end(); ++t)
        {
                if(exper.contains(*t))
                {
                        addEvent(*t, VexEvent::MANUAL_BREAK, "");
                }
        }
}

ostream& operator << (ostream &os, const VexInterval &x)
{
	int p = os.precision();

	os.precision(12);
	os << "mjd(" << x.mjdStart << "," << x.mjdStop << ")";
	os.precision(p);

	return os;
}

ostream& operator << (ostream &os, const VexSource &x)
{
	os << "Source " << x.defName << endl;
	for(vector<string>::const_iterator it = x.sourceNames.begin(); it != x.sourceNames.end(); ++it)
	{
		os << "  name=" << *it << endl;
	}
	os << "  ra=" << x.ra <<
		"\n  dec=" << x.dec <<
		"\n  calCode=" << x.calCode <<
		"\n  qual=" << x.qualifier << endl;

	return os;
}

ostream& operator << (ostream &os, const VexScan &x)
{
	os << "Scan " << x.defName << 
		"\n  timeRange=" << (const VexInterval&)x <<
		"\n  mode=" << x.modeDefName <<
		"\n  source=" << x.sourceDefName << 
		"\n  size=" << x.size << " bytes \n";

	for(map<string,VexInterval>::const_iterator iter = x.stations.begin(); iter != x.stations.end(); ++iter)
	{
		os << "  " << iter->first << " range=" << iter->second << endl;
	}

	for(map<string,bool>::const_iterator iter = x.recordEnable.begin(); iter != x.recordEnable.end(); ++iter)
	{
		os << "  " << iter->first << " enable=" << iter->second << endl;
	}

	os << "  setup=" << x.corrSetupName << endl;

	return os;
}

ostream& operator << (ostream &os, const VexClock &x)
{
	os << "Clock(" << x.mjdStart << ": " << x.offset << ", " << x.rate << ", " << x.offset_epoch << ")";

	return os;
}

ostream& operator << (ostream &os, const VexAntenna &x)
{
	os << "Antenna " << x.name <<
		"\n  x=" << x.x << "  dx/dt=" << x.dx <<
		"\n  y=" << x.y << "  dy/dt=" << x.dy <<
		"\n  z=" << x.z << "  dz/dt=" << x.dz <<
		"\n  posEpoch=" << x.posEpoch <<
		"\n  axisType=" << x.axisType <<
		"\n  axisOffset=" << x.axisOffset << endl;

	os << "  dataSource=" << dataSourceNames[x.dataSource] << endl;

	for(vector<VexBasebandFile>::const_iterator it = x.basebandFiles.begin(); it != x.basebandFiles.end(); ++it)
	{
		os << "  " << *it << endl;
	}

	for(vector<VexClock>::const_iterator it = x.clocks.begin(); it != x.clocks.end(); ++it)
	{
		os << "  " << *it << endl;
	}

	return os;
}

ostream& operator << (ostream &os, const VexSubband &x)
{
	os << "[" << x.freq << " Hz, " << x.bandwidth << " Hz, sb=" << x.sideBand << ", pol=" << x.pol << "]";
	
	return os;
}

ostream& operator << (ostream &os, const VexChannel &x)
{
	os << "[name=" << x.bbcName << " IF=" << x.ifName << " s=" << x.subbandId << " -> r=" << x.recordChan << " tones=";
	for(vector<int>::const_iterator v = x.tones.begin(); v != x.tones.end(); ++v)
	{
		if(v != x.tones.begin())
		{
			os << ",";
		}
		os << *v;
	}
	os << "]";

	return os;
}

ostream& operator << (ostream &os, const VexIF &x)
{
	os << "[name=" << x.name << ", SSLO=" << x.ifSSLO << ", sb=" << x.ifSideBand << ", pol=" << x.pol << ", phaseCalInterval=" << x.phaseCalIntervalMHz << " MHz]";

	return os;
}

ostream& operator << (ostream &os, const VexSetup &x)
{
	os << "    Format = [format=" << x.formatName << ", nBit=" << x.nBit << ", nRecordChan=" << x.nRecordChan;
	for(vector<VexChannel>::const_iterator it = x.channels.begin(); it != x.channels.end(); ++it)
	{
		os << ", " << *it;
	}
	os << "]" << endl;
	for(map<string,VexIF>::const_iterator it = x.ifs.begin(); it != x.ifs.end(); ++it)
	{
		os << "    IF: " << it->first << " " << it->second << endl;
	}

	return os;
}

ostream& operator << (ostream &os, const VexMode &x)
{
	unsigned int nSubband = x.subbands.size();

	os << "Mode " << x.defName << endl;
	for(unsigned int i = 0; i < nSubband; ++i)
	{
		os << "  Subband[" << i << "]=" << x.subbands[i] << endl;
	}
	for(map<string,VexSetup>::const_iterator it = x.setups.begin(); it != x.setups.end(); ++it)
	{
		os << "  Setup[" << it->first << "]" << endl;
		os << it->second;
	}
	
	return os;
}

ostream& operator << (ostream &os, const VexEOP &x)
{
	os << "EOP(" << x.mjd << ", " << x.tai_utc << ", " << x.ut1_utc << ", " << (x.xPole*RAD2ASEC) << ", " << (x.yPole*RAD2ASEC) << ")";

	return os;
}


ostream& operator << (ostream &os, const VexBasebandFile &x)
{
	os << "Baseband(" << x.filename << ", " << (const VexInterval&)x << ")";

	return os;
}

ostream& operator << (ostream &os, const VexJob &x)
{
	int p = os.precision();
	
	os.precision(12);
	os << "Job " << x.jobSeries << "_" << x.jobId << endl;
	os << "  " << (const VexInterval&)x << endl;
	os << "  duty cycle = " << x.dutyCycle << endl;
	os << "  scans =";
	for(vector<string>::const_iterator s = x.scans.begin(); s != x.scans.end(); ++s)
	{
		os << " " << *s;
	}
	os << endl;
	for(map<string,string>::const_iterator v = x.vsns.begin(); v != x.vsns.end(); ++v)
	{
		os << "  " << "VSN[" << v->first << "] = " << v->second << endl;
	}
	os << "  size = " << x.dataSize << " bytes" << endl;

	os.precision(p);

	return os;
}

ostream& operator << (ostream &os, const VexJobGroup &x)
{
	int p = os.precision();
	
	os.precision(12);
	os << "Group: scans " << x.scans.front() << " - " << x.scans.back() << " = " << (const VexInterval &)x << endl;
	os.precision(p);
	
	return os;
}

ostream& operator << (ostream &os, const VexEvent &x)
{
	int d, s;

	d = static_cast<int>(x.mjd);
	s = static_cast<int>((x.mjd - d)*86400.0 + 0.5);

	os << "mjd=" << d << " sec=" << s << " : " << VexEvent::eventName[x.eventType] << " " << x.name;

	return os;
}

ostream& operator << (ostream &os, const VexJobFlag &x)
{
	int p = os.precision();

	os.precision(12);

	os << x.mjdStart << " " << x.mjdStop << " " << x.antId;

	os.precision(p);

	return os;
}

ostream& operator << (ostream &os, const VexData &x)
{
	int n = x.nSource();

	os << "Vex:" << endl;
	os << n << " sources:" << endl;
	for(int i = 0; i < n; ++i)
	{
		os << *x.getSource(i);
	}

	n = x.nScan();
	os << n << " scans:" << endl;
	for(int i = 0; i < n; ++i)
	{
		os << *x.getScan(i);
	}

	n = x.nAntenna();
	os << n << " antennas:" << endl;
	for(int i = 0; i < n; ++i)
	{
		os << *x.getAntenna(i);
	}

	n = x.nMode();
	os << n << " modes:" << endl;
	for(int i = 0; i < n; ++i)
	{
		os << *x.getMode(i);
	}

	n = x.nEOP();
	os << n << " eops:" << endl;
	for(int i = 0; i < n; ++i)
	{
		os << "   " << *x.getEOP(i) << endl;
	}

	const list<VexEvent> *events = x.getEvents();
	os << "Events:" << endl;
	for(list<VexEvent>::const_iterator iter = events->begin(); iter != events->end(); ++iter)
	{
		os << "   " << *iter << endl;
	}

	return os;
}
