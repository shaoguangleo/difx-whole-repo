/***************************************************************************
 *   Copyright (C) 2009-2016 by Walter Brisken & Adam Deller               *
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

#include <vector>
#include <set>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <sys/time.h>
#include <sys/stat.h>
#include <difxio/difx_input.h>
#include <difxmessage.h>
#include <vexdatamodel.h>

#include "corrparams.h"
#include "freq.h"
#include "job.h"
#include "makejobs.h"
#include "timeutils.h"
#include "sanitycheck.h"
#include "applycorrparams.h"
#include "shelves.h"
#include "../config.h"

using namespace std;


const string version(VERSION);
const string program("vex2difx");
const string verdate("20160226");
const string author("Walter Brisken/Adam Deller");

const int defaultMaxNSBetweenACAvg = 2000000;	// 2ms, good default for use with transient detection

static int calculateWorstcaseGuardNS(double sampleRate, int subintNS, int nBit, int nSubband)
{
	double sampleTimeNS = 1.0e9/sampleRate;
	double nsAccumulate = sampleTimeNS;
	const double MaxEarthGeomSlipRate = 1600.0;	// ns/sec
	
	while(fabs(nsAccumulate - static_cast<int>(nsAccumulate)) > 1.0e-12)
	{
		nsAccumulate += sampleTimeNS;
	}

	if(nBit*nSubband < 8)
	{
		nsAccumulate = nsAccumulate*8.0/(nBit*nSubband);
	}

	return static_cast<int>(nsAccumulate + MaxEarthGeomSlipRate*subintNS*1.0e-9 + 1.0);
}

static DifxJob *makeDifxJob(string directory, const Job& J, int nAntenna, const string& obsCode, int *n, int nDigit, char ext, const CorrParams *P)
{
	DifxJob *job;
	const char *difxVersion;
	const char *difxLabel;
	char jobName[DIFXIO_FILENAME_LENGTH];
	char fileBase[DIFXIO_FILENAME_LENGTH];
	int v;

	*n = 1;
	job = newDifxJobArray(*n);
	difxVersion = getenv("DIFX_VERSION");
	if(difxVersion)
	{
		snprintf(job->difxVersion, DIFXIO_VERSION_LENGTH, "%s", difxVersion);
	}
	else
	{
		snprintf(job->difxVersion, DIFXIO_VERSION_LENGTH, "%s", "Unknown");
	}
	difxLabel = getenv("DIFX_LABEL");
	if(difxLabel)
	{
		snprintf(job->difxLabel, DIFXIO_VERSION_LENGTH, "%s", difxLabel);
	}
	else
	{
		job->difxLabel[0] = 0;
	}
	snprintf(job->vexFile, DIFXIO_FILENAME_LENGTH, "%s", P->vexFile.c_str());
	job->jobStart = J.mjdStart;
	job->jobStop  = J.mjdStop;
	job->mjdStart = J.mjdStart;
	job->duration = trunc((J.mjdStop - J.mjdStart) * 86400.0 + 0.001);
	job->jobId    = J.jobId;
	job->subarrayId = 0;
	snprintf(job->obsCode, DIFXIO_OBSCODE_LENGTH, "%s", obsCode.c_str());
	job->obsCode[7] = 0;
	job->taperFunction = TaperFunctionUniform;
	job->polyOrder = 5;
	job->polyInterval = 120;
	job->aberCorr = AberCorrExact;
	job->activeDatastreams = nAntenna;
	job->activeBaselines = nAntenna*(nAntenna-1)/2;
	job->dutyCycle = J.dutyCycle;
	snprintf(job->delayModel, DIFXIO_FILENAME_LENGTH, "%s", P->delayModel.c_str());

	// The following line defines the format of the job filenames

	if(J.jobId > 0)
	{
		const int FormatLength = 20;
		char format[FormatLength];

		v = snprintf(format, FormatLength, "%%s_%%0%dd%%c", nDigit);
		if(v >= FormatLength)
		{
			cerr << "Developer error: makeDifxJob: format being truncated.  Needed " << v << " bytes." << endl;

			exit(EXIT_FAILURE);
		}
		v = snprintf(jobName, DIFXIO_FILENAME_LENGTH, format, J.jobSeries.c_str(), J.jobId, ext);
	}
	else
	{
		if(ext != 0)
		{
			cerr << "Warning: makeDifxJob: ext!=0 and making job names without extensions!" << endl;

			exit(EXIT_FAILURE);
		}
		v = snprintf(jobName, DIFXIO_FILENAME_LENGTH, "%s", J.jobSeries.c_str());
	}
	v = snprintf(fileBase, DIFXIO_FILENAME_LENGTH, "%s/%s", directory.c_str(), jobName);
	if(v >= DIFXIO_FILENAME_LENGTH)
	{
		cerr << "Developer error: makeDifxJob: fileBase needed " << v << " bytes." << endl;

		exit(EXIT_FAILURE);
	}

	snprintf(job->inputFile,   DIFXIO_FILENAME_LENGTH, "%s.input", fileBase);
	snprintf(job->calcFile,    DIFXIO_FILENAME_LENGTH, "%s.calc",  fileBase);
	snprintf(job->flagFile,    DIFXIO_FILENAME_LENGTH, "%s.flag",  fileBase);
	snprintf(job->imFile,      DIFXIO_FILENAME_LENGTH, "%s.im",    fileBase);
	if(P->outPath.empty())
	{
		snprintf(job->outputFile, DIFXIO_FILENAME_LENGTH, "%s.difx", fileBase);
	}
	else
	{
		snprintf(job->outputFile, DIFXIO_FILENAME_LENGTH, "%s/%s.difx", P->outPath.c_str(), jobName);
	}
	if(P->threadsFile.empty())
	{
		v = snprintf(job->threadsFile, DIFXIO_FILENAME_LENGTH, "%s.threads", fileBase);
	}
	else
	{
		v = snprintf(job->threadsFile, DIFXIO_FILENAME_LENGTH, "%s", P->threadsFile.c_str());
	}
	if(v >= DIFXIO_FILENAME_LENGTH)
	{
		cerr << "Developer error: makeDifxJob: threadsFile wanted " << v << " bytes, not " << DIFXIO_FILENAME_LENGTH << " .  Truncating."  << endl;
	}

	return job;
}

static DifxAntenna *makeDifxAntennas(const Job &J, const VexData *V, const CorrParams *P, int *n)
{
	DifxAntenna *A;
	double mjd;
	vector<string>::const_iterator a;
	int i;

	mjd = 0.5*(V->obsStart() + V->obsStop());

	*n = J.jobAntennas.size();

	A = newDifxAntennaArray(*n);

	// Note: the vsns vector here is used even for non-module corrlation.  It will map an antenna to a non-allowed VSN name which won't be used in case of non-module correlation.
	for(i = 0, a = J.jobAntennas.begin(); a != J.jobAntennas.end(); ++i, ++a)
	{
		double clockrefmjd;
		
		const VexAntenna *ant = V->getAntenna(*a);
		
		snprintf(A[i].name, DIFXIO_NAME_LENGTH, "%s", a->c_str());
		A[i].X = ant->x + ant->dx*(mjd-ant->posEpoch)*86400.0;
		A[i].Y = ant->y + ant->dy*(mjd-ant->posEpoch)*86400.0;
		A[i].Z = ant->z + ant->dz*(mjd-ant->posEpoch)*86400.0;
		A[i].mount = stringToMountType(ant->axisType.c_str());
		clockrefmjd = ant->getVexClocks(J.mjdStart, A[i].clockcoeff);
		if(clockrefmjd < 0.0 && !P->fakeDatasource)
		{
			cerr << "Warning: Job " << J.jobSeries << " " << J.jobId << ": no clock offsets being applied to antenna " << *a << endl;
			cerr << "          Unless this is intentional, your results will suffer!" << endl;
		}
		A[i].clockrefmjd = clockrefmjd;
		A[i].clockorder = 1;
		A[i].clockcoeff[0] *= 1.0e6;	// convert to us from sec
		A[i].clockcoeff[1] *= 1.0e6;	// convert to us/sec from sec/sec
		A[i].offset[0] = ant->axisOffset;
		A[i].offset[1] = 0.0;
		A[i].offset[2] = 0.0;

		/* override with antenna setup values? */
		const AntennaSetup *antSetup = P->getAntennaSetup(*a);
		if(antSetup)
		{
			if(!antSetup->difxName.empty())
			{
				snprintf(A[i].name, DIFXIO_NAME_LENGTH, "%s", antSetup->difxName.c_str());
			}

			// FIXME: below here should probably be done in the applyCorrParams function
			A[i].clockcoeff[0] += antSetup->deltaClock*1.0e6;	// convert to us from sec
			A[i].clockcoeff[1] += antSetup->deltaClockRate*1.0e6;	// convert to us/sec from sec/sec
			A[i].clockorder  = antSetup->clockorder;
			switch(A[i].clockorder)
			{
			case 5:
				A[i].clockcoeff[5] = antSetup->clock5*1.0e6; // convert to us/sec^5 from sec/sec^5
			case 4:
				A[i].clockcoeff[4] = antSetup->clock4*1.0e6; // convert to us/sec^4 from sec/sec^4
			case 3:
				A[i].clockcoeff[3] = antSetup->clock3*1.0e6; // convert to us/sec^3 from sec/sec^3
			case 2:
				A[i].clockcoeff[2] = antSetup->clock2*1.0e6; // convert to us/sec^2 from sec/sec^2
			case 1:
				break;
			default:
				cerr << "Crazy clock order " << A[i].clockorder << "!" << endl;
			}
		}
	}

	return A;
}

// NOTE: FIXME: before this gets called, all datasource=NONE datastreams should be stripped.

static DifxDatastream *makeDifxDatastreams(const Job& J, const VexData *V, const CorrParams *P, int nSet, DifxAntenna *difxAntennas, const Shelves &shelves)
{
	DifxDatastream *datastreams;
	int nDatastream;
	int di;
	const VexMode *M;

	// Determine worst case (but typical) number of datastreams for this job
	M = V->getModeByDefName(J.modeName);
	if(!M)
	{
		std::cerr << "Developer error: makeDifxDatastreams: getModeByDefName() returns null for modeName=" << J.modeName << std::endl;

		exit(EXIT_FAILURE);
	}
	nDatastream = M->nStream();

	// for each setup these are duplicated
	nDatastream *= nSet;
	
	datastreams = newDifxDatastreamArray(nDatastream);

	di = 0;	// datastream array index
	for(int s = 0; s < nSet; ++s)
	{
		int antennaId = 0;
		for(std::vector<std::string>::const_iterator a = J.jobAntennas.begin(); a != J.jobAntennas.end(); ++a)
		{
			int nd;
			const VexAntenna *ant = V->getAntenna(*a);
			std::string shelf;

			std::map<std::string,VexSetup>::const_iterator sit = M->setups.find(*a);
			if(sit == M->setups.end())
			{
				std::cerr << "Developer error: makeDifxDatastreams: setup for antenna " << *a << " not found in mode " << M->defName << std::endl;

				exit(EXIT_FAILURE);
			}
			const VexSetup &setup = sit->second;

			nd = setup.nStream();

			shelf.clear();
			for(int d = 0; d < nd; ++d)
			{
				DifxDatastream *dd = datastreams + di;
				const VexStream &stream = setup.streams[d];

				dd->antennaId = antennaId;
				dd->dataSource = stream.dataSource;
				dd->tSys = 0.0;
				dd->dataSampling = stream.dataSampling;
				switch(dd->dataSource)
				{
				case DataSourceNetwork:
					dd->windowSize = ant->ports[d].windowSize;
					snprintf(dd->networkPort, DIFXIO_ETH_DEV_SIZE, "%s", ant->ports[d].networkPort.c_str());
					break;
				case DataSourceFile:
				case DataSourceMark6:
					{
						int nFile = ant->files.size();
						int count = 0;

						for(int j = 0; j < nFile; ++j)
						{
							if(ant->files[j].streamId == d && J.overlap(ant->files[j]) > 0.0)
							{
								++count;
							}
						}

						DifxDatastreamAllocFiles(dd, count);

						count = 0;

						for(int j = 0; j < nFile; ++j)
						{
							if(ant->files[j].streamId == d && J.overlap(ant->files[j]) > 0.0)
							{
								dd->file[count] = strdup(ant->files[j].filename.c_str());
								++count;
							}
						}
					}
					break;
				case DataSourceModule:
					{
						int nVSN = ant->vsns.size();
						int count = 0;
						
						DifxDatastreamAllocFiles(dd, 1);
						for(int j = 0; j < nVSN; ++j)
						{
							if(ant->vsns[j].streamId == d && J.overlap(ant->vsns[j]) > 0.0)
							{
								dd->file[0] = strdup(ant->vsns[j].filename.c_str());
								++count;
							}
						}
						if(count > 1)
						{
							std::cerr << "Developer error: got into job creation and antenna " << *a << " datastream " << d << " had " << count << " > 1 module valid in time range of job: " << J << std::endl;

							exit(EXIT_FAILURE);
						}
						if(count == 0)
						{
							std::cerr << "Developer error: got into job creation and antenna " << *a << " datastream " << d << " had no module valid in time range of job: " << J << std::endl;

							exit(EXIT_FAILURE);
						}
						if(!shelf.empty())
						{
							shelf += ",";
						}
						shelf += shelves.getShelf(dd->file[0]);
					}
					break;
				case DataSourceFake:
					break;
				default:
					std::cerr << "Developer error: got into job creation with antenna " << *a << " datastream num " << d << " having unsupported data source " << dataSourceNames[dd->dataSource] << std::endl;

					exit(EXIT_FAILURE);
				}

				++di;
			}

			if(shelf.empty())
			{
				snprintf(difxAntennas[antennaId].shelf, DIFXIO_SHELF_LENGTH, "NONE");
			}
			else
			{
				snprintf(difxAntennas[antennaId].shelf, DIFXIO_SHELF_LENGTH, "%s", shelf.c_str());
			}

			++antennaId;
		}
	}

	return datastreams;
}

static int getBand(vector<pair<int,int> >& bandMap, int fqId)
{
	for(vector<pair<int,int> >::iterator it = bandMap.begin(); it != bandMap.end(); ++it)
	{
		if(it->first == fqId)
		{
			++it->second;

			return it - bandMap.begin();
		}
	}

	// not in list yet, so add
	bandMap.push_back(pair<int,int>(fqId, 1));

	return bandMap.size() - 1;
}

static int getToneSetId(vector<vector<unsigned int> > &toneSets, const vector<unsigned int> &tones)
{
	for(vector<vector<unsigned int> >::const_iterator it = toneSets.begin(); it != toneSets.end(); ++it)
	{
		if(*it == tones)
		{
			return it - toneSets.begin();
		}
	}

	// not in list yet, so add
	toneSets.push_back(tones);

	return toneSets.size() - 1;
}

static int setFormat(DifxInput *D, int dsId, vector<freq>& freqs, vector<vector<unsigned int> >& toneSets, const VexMode *mode, const string &antName, int startBand, const VexSetup &setup, const VexStream &stream, const CorrSetup *corrSetup, enum V2D_Mode v2dMode)
{
	vector<pair<int,int> > bandMap;

	if(mode == 0)
	{
		cerr << "Developer error: setFormat: mode is NULL" << endl;

		exit(EXIT_FAILURE);
	}

	// just check to make sure antId is legal
	int antId = D->datastream[dsId].antennaId;
	if(antId < 0 || antId >= D->nAntenna)
	{
		cerr << "Developer error: setFormat: antId=" << antId << " while nAntenna=" << D->nAntenna << endl;
		
		exit(EXIT_FAILURE);
	}
	int nRecordChan = stream.nRecordChan;

	stream.snprintDifxFormatName(D->datastream[dsId].dataFormat, DIFXIO_FORMAT_LENGTH);
	D->datastream[dsId].dataFrameSize = stream.dataFrameSize();
	D->datastream[dsId].quantBits = stream.nBit;
	DifxDatastreamAllocBands(D->datastream + dsId, nRecordChan);

	for(int i = 0; i < nRecordChan; ++i)
	{
		const VexChannel *ch = &setup.channels[i + startBand];
		if(ch->subbandId < 0 || ch->subbandId >= static_cast<int>(mode->subbands.size()))
		{
			cerr << "Error: setFormat: index to subband=" << ch->subbandId << " is out of range.  antName=" << antName << " mode=" << mode->defName << endl;

			exit(EXIT_FAILURE);
		}

		int r = ch->recordChan;
		if(r >= 0)
		{
			unsigned int toneSetId, fqId;
			const VexSubband& subband = mode->subbands[ch->subbandId];

			r -= startBand;
			if(r < 0 || r >= D->datastream[dsId].nRecBand)
			{
				cerr << "Error: setFormat: index to record channel=" << r << " is out of range.  antName=" << antName << " mode=" << mode->defName << endl;
				cerr << "nRecBand = " << D->datastream[dsId].nRecBand << endl;
				cerr << "startBand = " << startBand << endl;
				cerr << "subband = " << mode->subbands[ch->subbandId] << endl;
				cerr << "nRecordChan = " << nRecordChan << "  i = " << i << endl;

				exit(EXIT_FAILURE);
			}
			
			if(v2dMode == V2D_MODE_PROFILE || setup.phaseCalIntervalMHz() == 0)
			{
				// In profile mode don't extract any tones
				toneSetId = 0;
			}
			else
			{
				toneSetId = getToneSetId(toneSets, ch->tones);
			}
			
			fqId = getFreqId(freqs, subband.freq, subband.bandwidth, subband.sideBand, corrSetup->FFTSpecRes, corrSetup->outputSpecRes, 1, 0, toneSetId);	// 0 means not zoom band
			
			D->datastream[dsId].recBandFreqId[r] = getBand(bandMap, fqId);
			D->datastream[dsId].recBandPolName[r] = subband.pol;
		}
	}
	DifxDatastreamAllocFreqs(D->datastream + dsId, bandMap.size());
	for(size_t j = 0; j < bandMap.size(); ++j)
	{
		D->datastream[dsId].recFreqId[j] = bandMap[j].first;
		D->datastream[dsId].nRecPol[j]   = bandMap[j].second;
	}

	return nRecordChan;
}

static void populateRuleTable(DifxInput *D, const CorrParams *P)
{
	D->nRule = P->rules.size();
	D->rule = newDifxRuleArray(D->nRule);
	for(int i = 0; i < D->nRule; ++i)
	{
		if(!P->rules[i].scanName.empty())
		{
			for(list<string>::const_iterator s = (P->rules[i].scanName).begin(); s != (P->rules[i].scanName).end(); ++s)
			{
				DifxStringArrayadd(&D->rule[i].scanId, s->c_str(), 0);
			}
		}
		if(!P->rules[i].sourceName.empty())
		{
			for(list<string>::const_iterator s = (P->rules[i].sourceName).begin(); s != (P->rules[i].sourceName).end(); ++s)
			{
				DifxStringArrayadd(&D->rule[i].sourceName, P->getNewSourceName(*s).c_str(), 0);
			}
		}
		if(!P->rules[i].modeName.empty())
		{
			// Note: this is not a problem as the mode selection is being done by vex2difx
			//cerr << "Cannot rule on modeName at this time; ignoring." << endl;
		}
		if(!P->rules[i].calCode.empty())
		{
			if(P->rules[i].calCode.size() > 1)
			{
				cerr << "Cannot handle rules for more than one calCode simultaneously." << endl;

				exit(EXIT_FAILURE);
			}
			D->rule[i].calCode[0] = P->rules[i].calCode.front();
			D->rule[i].calCode[1] = 0;
		}
		if(!P->rules[i].qualifier.empty())
		{
			if(P->rules[i].qualifier.size() > 1)
			{
				cerr << "Cannot handle rules for more than one qualifier simultaneously." << endl;
				
				exit(EXIT_FAILURE);
			}
			D->rule[i].qual = P->rules[i].qualifier.front();
		}
		snprintf(D->rule[i].configName, DIFXIO_NAME_LENGTH, "%s", P->rules[i].corrSetupName.c_str());
	}
}

static void populateFreqTable(DifxInput *D, const vector<freq>& freqs, const vector<vector<unsigned int> > &toneSets)
{
	D->nFreq = freqs.size();
	D->freq = newDifxFreqArray(D->nFreq);

	for(int f = 0; f < D->nFreq; ++f)
	{
		static int firstChanBWWarning=1;
		DifxFreq *df = D->freq + f;
		double chanBW;

		df->freq = freqs[f].fq/1.0e6;
		df->bw   = freqs[f].bw/1.0e6;
		df->sideband = freqs[f].sideBand;
		df->nChan = static_cast<int>(freqs[f].bw/freqs[f].inputSpecRes + 0.5);	// df->nChan is the number of pre-averaged channels
		df->specAvg = freqs[f].specAvg();
		df->decimation = freqs[f].decimation;
		df->overSamp = 1;	// FIXME: eventually provide this again.

		chanBW = freqs[f].outputSpecRes*1e-6;
		if(chanBW > 0.51 && firstChanBWWarning)
		{
			firstChanBWWarning = 0;
			cout << "Warning: channel bandwidth is " << chanBW << " MHz, which is larger than the minimum recommended 0.5 MHz.  Consider decreasing the output spectral resolution." << endl;
		}

		if(freqs[f].toneSetId >= toneSets.size())
		{
			cerr << "Developer error: populateFreqTable: toneSetId=" << freqs[f].toneSetId << " nToneSet=" << toneSets.size() << endl;
		}
		const vector<unsigned int> &tones = toneSets[freqs[f].toneSetId];

		if(!tones.empty())
		{
			DifxFreqAllocTones(df, tones.size());
		
			for(unsigned int t = 0; t < tones.size(); ++t)
			{
				df->tone[t] = tones[t];
			}
		}
	}
}

static double populateBaselineTable(DifxInput *D, const CorrParams *P, const CorrSetup *corrSetup, vector<set <int> > blockedfreqids)
{	
	int n1, n2;
	int nPol;
	int a1c[2], a2c[2];
	char a1p[2], a2p[2];
	int nFreq;
	DifxBaseline *bl;
	DifxConfig *config;
	int freqId, altFreqId, blId, configId;
	double lowedgefreq, altlowedgefreq;
	double globalBandwidth = 0;

	// Calculate maximum number of possible baselines based on list of configs
	D->nBaseline = 0;

	for(configId = 0; configId < D->nConfig; ++configId)
	{
		D->nBaseline += D->config[configId].nBaseline;
	}

	D->baseline = newDifxBaselineArray(D->nBaseline);

	bl = D->baseline;
	blId = 0;	// baseline table index

	for(configId = 0; configId < D->nConfig; ++configId)
	{
		config = D->config + configId;

		config->nBaseline = 0;

		// Note: these should loop over antennaIds
		if(P->v2dMode == V2D_MODE_PROFILE)
		{
			// Disable writing of standard autocorrelations
			config->doAutoCorr = 0;

			// Instead, make autocorrlations from scratch
			for(int a1 = 0; a1 < D->nAntenna; ++a1)
			{
				for(int configds1 = 0; configds1 < config->nDatastream; ++configds1)
				{
					int ds1;

					ds1 = config->datastreamId[configds1];

					if(ds1 >= D->nDatastream)
					{
						std::cerr << "Developer error: populateBaselineTable pos 1: ds1=" << ds1 << " , nDatastream=" << D->nDatastream << std::endl;
						std::cerr << "configds1=" << configds1 << " config->nDatastream=" << config->nDatastream << " a1=" << a1 << std::endl;
						std::cerr << "All values of config->datastreamId[] array are:";
						for(int y = 0; y < config->nDatastream; ++y)
						{
							std::cerr << " " << config->datastreamId[y];
						}
						std::cerr << std::endl;

						exit(EXIT_FAILURE);
					}

					if(a1 != D->datastream[ds1].antennaId)
					{
						continue;
					}

					bl->dsA = ds1;
					bl->dsB = ds1;

					DifxBaselineAllocFreqs(bl, D->datastream[ds1].nRecFreq);

					nFreq = 0; // this counts the actual number of freqs

					for(int f = 0; f < D->datastream[ds1].nRecFreq; ++f)
					{
						freqId = D->datastream[ds1].recFreqId[f];

						if(!corrSetup->correlateFreqId(freqId))
						{
							continue;
						}
						if(!blockedfreqids[a1].empty() && blockedfreqids[a1].find(freqId) != blockedfreqids[a1].end())
						{
							continue;
						}

						DifxBaselineAllocPolProds(bl, nFreq, 4);

						n1 = DifxDatastreamGetRecBands(D->datastream+ds1, freqId, a1p, a1c);

						nPol = 0;
						for(int u = 0; u < n1; ++u)
						{
							int v;

							for(v = 0; v < n1; ++v)
							{
								if(corrSetup->doPolar || (a1p[u] == a1p[v] && (corrSetup->onlyPol == ' ' || corrSetup->onlyPol == a1p[u])))
								{
									bl->bandA[nFreq][nPol] = a1c[u];
									bl->bandB[nFreq][nPol] = a1c[v];
									++nPol;
								}
							}
						}
						bl->nPolProd[nFreq] = nPol;

						if(nPol == 0)
						{
							// This deallocates
							DifxBaselineAllocPolProds(bl, nFreq, 0);

							continue;
						}

						++nFreq;
					}
					for(int f = 0; f < D->datastream[ds1].nZoomFreq; ++f)
					{
						freqId = D->datastream[ds1].zoomFreqId[f];

						DifxBaselineAllocPolProds(bl, nFreq, 4);

						n1 = DifxDatastreamGetZoomBands(D->datastream+ds1, freqId, a1p, a1c);

						if(n1 < 0 || n1 > 2)
						{
							std::cerr << "Developer error: populateBaselineTable: n1=" << n1 << " for ds1=" << ds1 << " a1=" << a1 <<" freqId=" << freqId << std::endl;

							exit(EXIT_FAILURE);
						}

						nPol = 0;
						for(int u = 0; u < n1; ++u)
						{
							for(int v = 0; v < n1; ++v)
							{
								if(corrSetup->doPolar || (a1p[u] == a1p[v] && (corrSetup->onlyPol == ' ' || corrSetup->onlyPol == a1p[u])))
								{
									bl->bandA[nFreq][nPol] = D->datastream[ds1].nRecBand + a1c[u];
									bl->bandB[nFreq][nPol] = D->datastream[ds1].nRecBand + a1c[v];
									++nPol;
								}
							}
						}
						bl->nPolProd[nFreq] = nPol;

						if(nPol == 0)
						{
							// This deallocates
							DifxBaselineAllocPolProds(bl, nFreq, 0);

							continue;
						}

						++nFreq;
					}

					bl->nFreq = nFreq;

					if(bl->nFreq > 0)
					{
						config->baselineId[config->nBaseline] = blId;
						++config->nBaseline;
						++bl;
						++blId;
					}
				} // config datastream loop
			} // antenna loop
		} // if profile mode
		else // Not profile mode
		{
			// Beware those who try to follow the logic below!
			// Its actually not to tricky.  There are 8 cases of matching between 
			// sub-bands to consider.
			// First all antenna 1 recorded bands to be correlated are considered.  Pairing
			// with sub-bands from antenna 2 is done with the following priority:
			// 1. a recorded band with same sideband
			// 2. a recorded band with opposite sideband
			// 3. a zoom band with same sideband
			// 4. a zoom band with opposite sideband
			//
			// Further down, the antenna 1 zoom bands are considered with priority
			// 5. a recorded band with same sideband
			// 6. a recorded band with opposite sideband
			// 7. a zoom band with same sideband
			// 8. a zoom band with opposite sideband

			// Needless to say, this logic can probably be simplified some, but it seems to work!

			for(int a1 = 0; a1 < D->nAntenna-1; ++a1)
			{
				for(int a2 = a1 + 1; a2 < D->nAntenna; ++a2)
				{
					for(int configds1 = 0; configds1 < config->nDatastream; ++configds1)
					{
						int ds1;

						ds1 = config->datastreamId[configds1];

						if(ds1 >= D->nDatastream)
						{
							std::cerr << "Developer error: populateBaselineTable pos 2: ds1=" << ds1 << " , nDatastream=" << D->nDatastream << std::endl;
							std::cerr << "configds1=" << configds1 << " config->nDatastream=" << config->nDatastream << " a1=" << a1 << std::endl;

							std::cerr << "All values of config->datastreamId[] array are:";
							for(int y = 0; y < config->nDatastream; ++y)
							{
								std::cerr << " " << config->datastreamId[y];
							}
							std::cerr << std::endl;

							exit(EXIT_FAILURE);
						}

						if(a1 != D->datastream[ds1].antennaId)
						{
							continue;
						}

						for(int configds2 = 0; configds2 < config->nDatastream; ++configds2)
						{
							int ds2;

							ds2 = config->datastreamId[configds2];
							if(a2 != D->datastream[ds2].antennaId)
							{
								continue;
							}
							
							// Excape if this baseline is not requested
							if(!P->useBaseline(D->antenna[a1].name, D->antenna[a2].name))
							{
								continue;
							}

							bl->dsA = ds1;
							bl->dsB = ds2;

							// Allocate enough space for worst case possibility
							DifxBaselineAllocFreqs(bl, D->datastream[ds1].nRecFreq + D->datastream[ds1].nZoomFreq);

							nFreq = 0; // this counts the actual number of freqs

							// Note: eventually we need to loop over all datastreams associated with this antenna!
							for(int f = 0; f < D->datastream[ds1].nRecFreq; ++f)
							{
								bool zoom2 = false;	// did antenna 2 zoom band make match? 

								freqId = D->datastream[ds1].recFreqId[f];

								if(!corrSetup->correlateFreqId(freqId))
								{
									continue;
								}
								if(!blockedfreqids[a1].empty() && blockedfreqids[a1].find(freqId) != blockedfreqids[a1].end())
								{
									continue;
								}
								if(!blockedfreqids[a2].empty() && blockedfreqids[a2].find(freqId) != blockedfreqids[a2].end())
								{
									continue;
								}

								DifxBaselineAllocPolProds(bl, nFreq, 4);

								n1 = DifxDatastreamGetRecBands(D->datastream+ds1, freqId, a1p, a1c);
								n2 = DifxDatastreamGetRecBands(D->datastream+ds2, freqId, a2p, a2c);

								lowedgefreq = D->freq[freqId].freq;
								if(D->freq[freqId].sideband == 'L')
								{
									lowedgefreq -= D->freq[freqId].bw;
								}

								if(n2 == 0)
								{
									//look for another freqId which matches band but is opposite sideband
									for(int f2 = 0; f2 < D->datastream[ds2].nRecFreq; ++f2)
									{
										altFreqId = D->datastream[ds2].recFreqId[f2];
										altlowedgefreq = D->freq[altFreqId].freq;
										if(D->freq[altFreqId].sideband == 'L')
										{
											altlowedgefreq -= D->freq[altFreqId].bw;
										}
										if(altlowedgefreq     == lowedgefreq &&
										   D->freq[freqId].bw == D->freq[altFreqId].bw)
										{
											n2 = DifxDatastreamGetRecBands(D->datastream+ds2, altFreqId, a2p, a2c);
										}
									}
								}
								if(n2 == 0)
								{
									//still no dice? Try the zoom bands of datastream 2 with the same sideband
									for(int f2 = 0; f2 < D->datastream[ds2].nZoomFreq; ++f2)
									{
										altFreqId = D->datastream[ds2].zoomFreqId[f2];
										if(D->freq[freqId].freq == D->freq[altFreqId].freq &&
										   D->freq[freqId].bw   == D->freq[altFreqId].bw &&
										   D->freq[freqId].sideband == D->freq[altFreqId].sideband)
										{
											n2 = DifxDatastreamGetZoomBands(D->datastream+ds2, altFreqId, a2p, a2c);
											zoom2 = true;
										}
									}
								}
								if(n2 == 0)
								{
									//still no dice? Try the opposite sidebands of zoom bands of datastream 2
									for(int f2 = 0; f2 < D->datastream[ds2].nZoomFreq; ++f2)
									{
										altFreqId = D->datastream[ds2].zoomFreqId[f2];
										altlowedgefreq = D->freq[altFreqId].freq;
										if(D->freq[altFreqId].sideband == 'L')
										{
											altlowedgefreq -= D->freq[altFreqId].bw;
										}
										if(altlowedgefreq == lowedgefreq &&
										   D->freq[freqId].bw == D->freq[altFreqId].bw)
										{
											n2 = DifxDatastreamGetZoomBands(D->datastream+ds2, altFreqId, a2p, a2c);
											zoom2 = true;
										}
									}
								}

								nPol = 0;
								for(int u = 0; u < n1; ++u)
								{
									for(int v = 0; v < n2; ++v)
									{
										if(corrSetup->doPolar || (a1p[u] == a2p[v] && (corrSetup->onlyPol == ' ' || corrSetup->onlyPol == a1p[u])))
										{
											bl->bandA[nFreq][nPol] = a1c[u];
											bl->bandB[nFreq][nPol] = a2c[v];
											if(zoom2)
											{
												bl->bandB[nFreq][nPol] += D->datastream[ds2].nRecBand;
											}
											++nPol;
										}
									}
								}
								bl->nPolProd[nFreq] = nPol;

								if(nPol == 0)
								{
									// This deallocates
									DifxBaselineAllocPolProds(bl, nFreq, 0);

									continue;
								}

								if(globalBandwidth == 0)
								{
									globalBandwidth = D->freq[freqId].bw;
								}
								else if(globalBandwidth > 0)
								{
									if(globalBandwidth != D->freq[freqId].bw)
									{
										globalBandwidth = -1;
									}
								}

								++nFreq;
							}

							for(int f = 0; f < D->datastream[a1].nZoomFreq; ++f)
							{
								bool zoom2 = false;	// did antenna 2 zoom band make match? 

								n2 = 0;

								freqId = D->datastream[ds1].zoomFreqId[f];

								// Unlike for recbands, don't query corrSetup->correlateFreqId as all defined zoom bands should be correlated

								DifxBaselineAllocPolProds(bl, nFreq, 4);

								n1 = DifxDatastreamGetZoomBands(D->datastream+ds1, freqId, a1p, a1c);

								lowedgefreq = D->freq[freqId].freq;
								if(D->freq[freqId].sideband == 'L')
								{
									lowedgefreq -= D->freq[freqId].bw;
								}

								for(int f2 = 0; f2 < D->datastream[ds2].nRecFreq; ++f2)
								{
									altFreqId = D->datastream[ds2].recFreqId[f2];
									if(D->freq[freqId].freq == D->freq[altFreqId].freq &&
									   D->freq[freqId].bw   == D->freq[altFreqId].bw &&
									   D->freq[altFreqId].sideband == 'U')
									{
										n2 = DifxDatastreamGetRecBands(D->datastream+ds2, altFreqId, a2p, a2c);
									}
								}

								if(n2 == 0)
								{
									//look for another freqId which matches band but is opposite sideband
									for(int f2 = 0; f2 < D->datastream[ds2].nRecFreq; ++f2)
									{
										altFreqId = D->datastream[ds2].recFreqId[f2];
										altlowedgefreq = D->freq[altFreqId].freq;
										if(D->freq[altFreqId].sideband == 'L')
										{
											altlowedgefreq -= D->freq[altFreqId].bw;
										}
										if(altlowedgefreq     == lowedgefreq &&
										   D->freq[freqId].bw == D->freq[altFreqId].bw)
										{
											n2 = DifxDatastreamGetRecBands(D->datastream+ds2, altFreqId, a2p, a2c);
										}
									}
								}
								if(n2 == 0)
								{
									n2 = DifxDatastreamGetZoomBands(D->datastream+ds2, freqId, a2p, a2c);
									if(n2 > 0)
									{
										zoom2 = true;
									}
								}
								if(n2 == 0)
								{
									//still no dice? Try the opposite sidebands of zoom bands of datastream 2
									for(int f2 = 0; f2 < D->datastream[ds2].nZoomFreq; ++f2)
									{
										altFreqId = D->datastream[ds2].zoomFreqId[f2];
										altlowedgefreq = D->freq[altFreqId].freq;
										if(D->freq[altFreqId].sideband == 'L')
										{
											altlowedgefreq -= D->freq[altFreqId].bw;
										}
										if(altlowedgefreq == lowedgefreq &&
										   D->freq[freqId].bw == D->freq[altFreqId].bw)
										{
											n2 = DifxDatastreamGetZoomBands(D->datastream+ds2, altFreqId, a2p, a2c);
											zoom2 = true;
										}
									}
								}

								nPol = 0;
								for(int u = 0; u < n1; ++u)
								{
									for(int v = 0; v < n2; ++v)
									{
										if(corrSetup->doPolar || (a1p[u] == a2p[v] && (corrSetup->onlyPol == ' ' || corrSetup->onlyPol == a1p[u])))
										{
											bl->bandA[nFreq][nPol] = D->datastream[ds1].nRecBand + a1c[u];
											bl->bandB[nFreq][nPol] = a2c[v];
											if(zoom2)
											{
												bl->bandB[nFreq][nPol] += D->datastream[ds2].nRecBand;
											}
											++nPol;
										}
									}
								}
								bl->nPolProd[nFreq] = nPol;

								if(nPol == 0)
								{
									// This deallocates
									DifxBaselineAllocPolProds(bl, nFreq, 0);

									continue;
								}

								if(globalBandwidth == 0)
								{
									globalBandwidth = D->freq[freqId].bw;
								}
								else if(globalBandwidth > 0)
								{
									if(globalBandwidth != D->freq[freqId].bw)
									{
										globalBandwidth = -1;
									}
								}

								++nFreq;
							}
			
							bl->nFreq = nFreq;
			
							if(bl->nFreq > 0)
							{
								config->baselineId[config->nBaseline] = blId;
								++config->nBaseline;
								++bl;
								++blId;
							}
						} // config datastream 2 loop
					} // config datastream 1 loop
				} // ant 2 loop
			} // ant 1 loop
		}
		config->baselineId[config->nBaseline] = -1;
	}

	// set actual number of baselines
	D->nBaseline = blId;

	return globalBandwidth;
}

static void populateEOPTable(DifxInput *D, const vector<VexEOP>& E)
{
	int nEOP;

	nEOP = E.size();
	D->nEOP = nEOP;
	D->eop = newDifxEOPArray(D->nEOP);

	for(int e = 0; e < nEOP; ++e)
	{
		D->eop[e].mjd = static_cast<int>(E[e].mjd);
		D->eop[e].tai_utc = static_cast<int>(E[e].tai_utc);
		D->eop[e].ut1_utc = E[e].ut1_utc;
		D->eop[e].xPole = E[e].xPole*180.0*3600.0/M_PI;
		D->eop[e].yPole = E[e].yPole*180.0*3600.0/M_PI;
	}
}

static int getConfigIndex(vector<pair<string,string> >& configs, DifxInput *D, const VexData *V, const CorrParams *P, const VexScan *S)
{
	int nConfig, nFFTsPerIntegration, nSubintsPerIntegration;
	int max5div, max2div;
	int fftDurNS;
	DifxConfig *config;
	const CorrSetup *corrSetup;
	const VexMode *mode;
	string configName;
	double floatReadTimeNS, floatFFTDurNS, floatSubintDurNS;
	double msgSize, dataRate, readSize;
	int64_t tintNS;
	int nDatastream;

	const std::string &corrSetupName = P->findSetup(S->defName, S->sourceDefName, S->modeDefName);
	corrSetup = P->getCorrSetup(corrSetupName);
	if(corrSetup == 0)
	{
		cerr << "Error: correlator setup[" << corrSetupName << "] == 0" << endl;
		
		exit(EXIT_FAILURE);
	}

	mode = V->getModeByDefName(S->modeDefName);
	if(mode == 0)
	{
		cerr << "Error: mode[" << S->modeDefName << "] == 0" << endl;
		
		exit(EXIT_FAILURE);
	}

	nConfig = configs.size();
	for(int i = 0; i < nConfig; ++i)
	{
		if(configs[i].first  == S->modeDefName &&
		   configs[i].second == corrSetupName)
		{
			return i;
		}
	}

	// get worst case datastream count
	nDatastream = mode->nStream();
	configName = S->modeDefName + string("_") + corrSetupName;

	configs.push_back(pair<string,string>(S->modeDefName, corrSetupName));
	config = D->config + nConfig;
	snprintf(config->name, DIFXIO_NAME_LENGTH, "%s", configName.c_str());
	for(int i = 0; i < D->nRule; ++i)
	{
		if(corrSetupName == D->rule[i].configName)
		{
			snprintf(D->rule[i].configName, DIFXIO_NAME_LENGTH, "%s", configName.c_str());
		}
	}
	config->tInt = corrSetup->tInt;
	tintNS = static_cast<int64_t>(1e9*corrSetup->tInt + 0.5);
	floatFFTDurNS = 1000000000.0/corrSetup->FFTSpecRes;
	fftDurNS = static_cast<int>(floatFFTDurNS);
	dataRate = mode->getHighestSampleRate()*mode->getBits()*mode->subbands.size();
	nFFTsPerIntegration = static_cast<int>(1e9*corrSetup->tInt/floatFFTDurNS + 0.5);

	//first test how big a single FFT is - if it is too big, fail with a warning and a suggestion
	if(floatFFTDurNS > (1LL<<31) - 1)
	{
		cerr << "A single FFT is too long (" << floatFFTDurNS << " ns)!" << endl;
		cerr << "The maximum duration of an FFT is 2^31 - 1 nanoseconds" << endl;
		cerr << "Please reduce nFFTChan accordingly" << endl;

		exit(EXIT_FAILURE);
	}

	if(corrSetup->subintNS > 0) //This is relatively easy - just see if the provided values are reasonable
	{
		config->subintNS = corrSetup->subintNS;
		if(config->subintNS % fftDurNS != 0)
		{
			cerr << "Error: The provided subintNS (" << config->subintNS << ") is not an integer multiple of the FFT duration (" << fftDurNS << ")" << endl;
			cerr << "You should adjust your subint time, or leave subint unset and vex2difx will set it for you" << endl;

			exit(EXIT_FAILURE);
		}
		if(tintNS % config->subintNS != 0)
		{
			if(P->tweakIntTime)
			{
				//leave it - we will adjust the int time later
			}
			else
			{
				cerr << "Error: The provided tInt (" << config->tInt << ") is not an integer multiple of the provided subint (" << corrSetup->subintNS/1.0e9 << ")" << endl;
				cerr << "You should adjust your subint and/or int time, or leave subint unset and vex2difx will set it for you" << endl;

				exit(EXIT_FAILURE);
			}
		}
	}
	else //first try to set a reasonable subintNS
	{
		int64_t nscounter;

		nFFTsPerIntegration = static_cast<int>(1e9*corrSetup->tInt/floatFFTDurNS + 0.5);

		// check that integration time is an integer number of FFTs
		if(fabs(1e9*corrSetup->tInt/floatFFTDurNS - nFFTsPerIntegration) > 1e-9)
		{
			//it is not - no chance of a reasonable subintNS unless we tweak intTime
			if(P->tweakIntTime)
			{
				cout << "Requested integration time (" << corrSetup->tInt << ") is not an integer multiple of the FFT time (" << floatFFTDurNS << " ns)" << endl;
				cout << "Will attempt to nudge the int time to a more appropriate value" << endl;
			}
			else
			{
				cerr << "Integration time is not an integer number of FFTs!" << endl;
				cerr << "Please change tInt to a multiple of " << floatFFTDurNS << " nanoseconds, or set tweakIntTime = true" << endl;
				cerr << "Try to use a number of FFTs which is factorable entirely by 2 and/or 5" << endl;
			
				exit(EXIT_FAILURE);
			}
		}

		config->subintNS = fftDurNS;
		msgSize = (config->subintNS*1.0e-9)*dataRate/8.0;
		readSize = msgSize*D->dataBufferFactor/D->nDataSegments;
		if(readSize > P->maxReadSize)
		{
			cerr << "Warning: a single FFT gives a read size of " << readSize << " bytes" << endl;
			cerr << "The maximum read size has been set (or defaulted) to " << P->maxReadSize << endl;
			cerr << "There are known problems with Mark5 module playback at large read sizes" << endl;
			cerr << "If you want to try with the large read size, set maxReadSize in the global area of the .v2d file" << endl;

			exit(EXIT_FAILURE);
		}

		max5div = 0;
		max2div = 0;
		nscounter = tintNS/5;
	 	while(nscounter > 0 && fabs(nscounter/floatFFTDurNS - static_cast<int>(nscounter/floatFFTDurNS + 0.5)) < 1e-9)
		{
			nscounter /= 5;
			++max5div;
		}

		nscounter = tintNS/2;
		while(nscounter > 0 && fabs(nscounter/floatFFTDurNS - static_cast<int>(nscounter/floatFFTDurNS + 0.5)) < 1e-9)
		{
			nscounter /= 2;
			++max2div;
		}

		for(int i = max2div; i >= 0; --i)
		{
			for(int j = max5div; j >= 0; --j)
			{
				int divisor = 1;
				for(int k = 0; k < i; ++k)
				{
					divisor *= 2;
				}
				for(int l = 0; l < j; ++l)
				{
					divisor *= 5;
				}

				int64_t testsubintNS = tintNS / divisor;
				msgSize = (testsubintNS*1.0e-9)*dataRate/8.0;
				readSize = msgSize*D->dataBufferFactor/D->nDataSegments;
				if(readSize > P->minReadSize && readSize < P->maxReadSize && 
					testsubintNS <= 1020000000 && testsubintNS > config->subintNS && 
					fabs(testsubintNS/floatFFTDurNS - static_cast<int>(testsubintNS/floatFFTDurNS + 0.5)) < 1e-9)
				{
					config->subintNS = testsubintNS;
				}
			}
		}
		//refuse to run if the generated read size is too small
		msgSize = (config->subintNS*1.0e-9)*dataRate/8.0;
		readSize = msgSize*D->dataBufferFactor/D->nDataSegments;
		if(readSize < P->minReadSize)
		{
			if(P->tweakIntTime)
			{
				while(((config->subintNS*1.0e-9)*dataRate/8.0)*(D->dataBufferFactor/D->nDataSegments) < P->minReadSize && config->subintNS <= 510000000)
				{
					config->subintNS *= 2;
				}
			}
			else
			{
				cerr << "Automatic subint duration selection generated " << config->subintNS << " nanoseconds" << endl;
				cerr << "This leads to a read size of " << readSize << " B" << endl;
				cerr << "The minimum read size was set or defaulted to " << P->minReadSize << " B" << endl;
				cerr << "Either decrease minReadSize (which may lead to slow correlation) or explicitly set subintNS" << endl;
				cerr << "You may find it advantageous to tweak the tInt to a more power-of-2 friendly value" << endl;

				exit(EXIT_FAILURE);
			}
		}
	}

	// change nDataSegments if needed to get send sizes under 2^31 nanoseconds
	floatReadTimeNS = static_cast<double>(config->subintNS)*D->dataBufferFactor/D->nDataSegments;
	if(floatReadTimeNS > 2140000000.0)
	{
		int f = static_cast<int>(2140000000.0/config->subintNS);
		if(f < 1)
		{
			cerr << "Error: There is no way to change dataBufferFactor to keep send sizes below 2^31 seconds" << endl;

			exit(EXIT_FAILURE);
		}
		cout << "Note: changing nDataSegments from " << D->nDataSegments << " to " << (D->dataBufferFactor/f) << " in order to keep data send sizes below 2.14 seconds" << endl;
		D->nDataSegments = D->dataBufferFactor/f;
		msgSize = (config->subintNS*1.0e-9)*dataRate/8.0;
		readSize = msgSize*D->dataBufferFactor/D->nDataSegments;
		if(readSize < P->minReadSize)
		{
			cout << "This has lead to a read size " << readSize << " smaller than the provided guideline " << P->minReadSize << ": correlation may run more slowly" << endl;
			cout << "But probably this is a low data rate experiment, so it won't matter" << endl;
		}
	}

	//now check that the int time is an integer number of subints, and tweak if necessary
	floatSubintDurNS = (double)config->subintNS;
	nSubintsPerIntegration = static_cast<int>(1e9*corrSetup->tInt/floatSubintDurNS + 0.5);
	if(fabs(1e9*corrSetup->tInt/floatSubintDurNS - nSubintsPerIntegration) > 1e-9)
	{
		if(P->tweakIntTime)
		{
			int best5div = 0;
			int best2div = 0;
			double bestfractionaldiff;

			cout << "The provided tInt (" << config->tInt << ") is not an integer multiple of the subint (" << floatSubintDurNS/1.0e9 << ")" << endl;
			max5div = static_cast<int>(config->tInt/(5*floatSubintDurNS/1.0e9)) + 1;
			max2div = static_cast<int>(config->tInt/(2*floatSubintDurNS/1.0e9)) + 1;
			bestfractionaldiff = fabs(1.0 - (floatSubintDurNS/1.0e9)/config->tInt);
			for(int i = 0; i <= max5div; ++i)
			{
				for(int j = 0; j <= max5div; ++j)
				{
					double test_tint = (floatSubintDurNS/1.0e9)*pow(5.0,i)*pow(2.0,j);
					if(fabs(1.0 - (test_tint)/config->tInt) < bestfractionaldiff)
					{
						bestfractionaldiff = fabs(1.0 - (test_tint)/config->tInt);
						best5div = i;
						best2div = j;
					}
				}
			}
			cout << "tInt has been updated from " << config->tInt << " to " << (floatSubintDurNS/1.0e9)*pow(5.0,best5div)*pow(2.0,best2div) << endl;
			cout << "(You could also try modifying the subintNS if you want to try harder to get your desired integration time)" << endl;
			config->tInt = (floatSubintDurNS/1.0e9)*pow(5.0,best5div)*pow(2.0,best2div);
		}
		else
		{
			cerr << "Error: The provided tInt (" << config->tInt << ") is not an integer multiple of the subint (" << floatSubintDurNS/1.0e9 << ")" << endl; 
			cerr << "Either change your integration time to a more friendly value, or tweak number of channels or subint time, or set tweakIntTime = true" << endl;

			exit(EXIT_FAILURE);
		}
	}

	config->guardNS = corrSetup->guardNS;
	config->fringeRotOrder = corrSetup->fringeRotOrder;
	config->strideLength = corrSetup->strideLength;
	config->xmacLength = corrSetup->xmacLength;
	config->numBufferedFFTs = corrSetup->numBufferedFFTs;

#warning "FIXME: is setting pulsarId = -1 correct"
	config->pulsarId = -1;
	config->doPolar = corrSetup->doPolar;
	config->doAutoCorr = 1;
	config->nAntenna = D->nAntenna;
	config->nDatastream = nDatastream;
	if(P->v2dMode == V2D_MODE_PROFILE)
	{
		config->nBaseline = config->nDatastream;
	}
	else
	{
		config->nBaseline = nDatastream*(nDatastream-1)/2;	// this is a worst case (but typical) scenario; may shrink later.
									// FIXME: it seems the shrinking causes seg faults.  
	}

	//if guardNS was set to negative value, change it to the right amount to allow for
	//adjustment to get to an integer NS + geometric rate slippage (assumes Earth-based antenna)
	//Note: if not set explicitly, zero will be passed to mpifxcorr where it will do the calculation
	if(config->guardNS < 0)
	{
		config->guardNS = calculateWorstcaseGuardNS(mode->getLowestSampleRate(), config->subintNS, mode->getMinBits(), mode->getMinSubbands());
	}
	
	DifxConfigAllocDatastreamIds(config, config->nDatastream, D->nConfig*config->nDatastream);
	DifxConfigAllocBaselineIds(config, config->nBaseline, nConfig*config->nBaseline);

	config->nPol = mode->getPols(config->pol);
	config->quantBits = mode->getBits();

	return nConfig;
}

static bool matchingFreq(const ZoomFreq &zoomfreq, const DifxDatastream *dd, int dfreqIndex, const vector<freq> &freqs)
{
	const double epsilon = 0.000001;
	double channeloffset;
	double parent_bottom,
	parent_top;
	const freq &f = freqs[dd->recFreqId[dfreqIndex]];

	if(f.sideBand == 'L')           // is parent LSB?
	{
		channeloffset = (f.fq - zoomfreq.frequency - zoomfreq.bandwidth)/f.inputSpecRes;
		parent_bottom = f.fq - f.bw;
		parent_top = f.fq;
	}
	else                            // parent is USB
	{
		channeloffset = (zoomfreq.frequency - f.fq)/f.inputSpecRes;
		parent_bottom = f.fq;
		parent_top = f.fq + f.bw;
	}

	if(zoomfreq.frequency < parent_bottom - epsilon)
	{
		return false;
	}

	if(zoomfreq.frequency + zoomfreq.bandwidth > parent_top + epsilon)
	{
		return false;
	}

	if(zoomfreq.spectralaverage > 0 && zoomfreq.spectralaverage != f.specAvg()) //0 means default to parent
	{
		return false;
	}

	if(fabs(channeloffset - static_cast<int>(channeloffset+0.5)) > epsilon)
	{
		return false;
	}

	return true;
}

static int writeJob(const Job& J, const VexData *V, const CorrParams *P, const std::list<Event> &events, const Shelves &shelves, int verbose, ofstream *of, int nDigit, char ext, int strict)
{
	DifxInput *D;
	DifxScan *scan;
	const CorrSetup *corrSetup;
	const SourceSetup *sourceSetup;
	const PhaseCentre *phaseCentre;
	const PhaseCentre * pointingCentre;
	const AntennaSetup *antennaSetup;
	const VexScan *S;
	set<string> configSet;
	set<string> spacecraftSet;
	vector<pair<string,string> > configs;
	vector<freq> freqs;
	vector<vector<unsigned int> > toneSets;
	int nPulsar=0;
	int nTotalPhaseCentres, nbin, maxPulsarBins, maxScanPhaseCentres, fftDurNS;
	double srcra, srcdec, radiff, decdiff;
	const double MAX_POS_DIFF = 5e-9; //radians, approximately equal to 1 mas
	int pointingSrcIndex, foundSrcIndex, atSource;
	int nZoomBands, fqId, polcount, zoomChans = 0, minChans;
	int decimation, worstcaseguardns;
	DifxDatastream *dd;
	double globalBandwidth;
	vector<set <int> > blockedfreqids;	// vector index is over antennaId

	// Initialize toneSets with the trivial case, which is used for all zoom bands
	vector<unsigned int> noTones;
	toneSets.push_back(noTones);

	// Assume same correlator setup for all scans
	if(J.scans.empty())
	{
		cerr << "Developer error: writeJob(): J.scans is empty" << endl;

		exit(EXIT_FAILURE);
	}
	
	S = V->getScanByDefName(J.scans.front());
	if(!S)
	{
		cerr << "Developer error: writeJob() top: scan[" << J.scans.front() << "] = 0" << endl;

		exit(EXIT_FAILURE);
	}
	const std::string &corrSetupName = P->findSetup(S->defName, S->sourceDefName, S->modeDefName);
	corrSetup = P->getCorrSetup(corrSetupName);
	if(!corrSetup)
	{
		cerr << "Error: writeJob(): correlator setup " << corrSetupName << ": Not found!" << endl;

		exit(EXIT_FAILURE);
	}

	// make set of unique config names
	for(vector<string>::const_iterator si = J.scans.begin(); si != J.scans.end(); ++si)
	{
		string configName;

		S = V->getScanByDefName(*si);
		if(!S)
		{
			cerr << "Developer error: writeJob() loop: scan[" << *si << "] = 0" << endl;

			exit(EXIT_FAILURE);
		}
		configName = S->modeDefName + string("_") + corrSetupName;
		configSet.insert(configName);
	}

	D = newDifxInput();

	D->mjdStart = J.mjdStart;
	D->mjdStop  = J.mjdStop;
	D->visBufferLength = P->visBufferLength;
	D->dataBufferFactor = P->dataBufferFactor;
	D->outputFormat = P->outputFormat;
	D->nDataSegments = P->nDataSegments;

	D->antenna = makeDifxAntennas(J, V, P, &(D->nAntenna));
	D->job = makeDifxJob(V->getDirectory(), J, D->nAntenna, V->getExper()->name, &(D->nJob), nDigit, ext, P);
	
	D->nScan = J.scans.size();
	D->scan = newDifxScanArray(D->nScan);
	D->nConfig = configSet.size();
	D->config = newDifxConfigArray(D->nConfig);

	blockedfreqids.resize(D->nAntenna);

	// Allocate space for the source table - first work out how many sources we'll need
	maxPulsarBins = 0;
	maxScanPhaseCentres = 0;
	nTotalPhaseCentres = 0;
	for(vector<SourceSetup>::const_iterator ss=P->sourceSetups.begin(); ss != P->sourceSetups.end(); ++ss)
	{
		nTotalPhaseCentres += ss->phaseCentres.size()+1;
		pointingCentre = &(ss->pointingCentre);
		if((pointingCentre->difxName.compare(PhaseCentre::DEFAULT_NAME) != 0) ||
		   (pointingCentre->ra > PhaseCentre::DEFAULT_RA) ||
		   (pointingCentre->dec > PhaseCentre::DEFAULT_DEC))
		{
			++nTotalPhaseCentres;
		}
	}
	allocateSourceTable(D, nTotalPhaseCentres);

	// Make rule table
	populateRuleTable(D, P);

	// now run through all scans, populating things as we go
	scan = D->scan;
	for(vector<string>::const_iterator si = J.scans.begin(); si != J.scans.end(); ++si, ++scan)
	{
		S = V->getScanByDefName(*si);
		if(!S)
		{
			cerr << "Developer error: source[" << *si << "] not found!  This cannot be!" << endl;
			
			exit(EXIT_FAILURE);
		}

		const VexSource *src = V->getSourceByDefName(S->sourceDefName);

		// Determine interval where scan and job overlap
		Interval scanInterval(*S);
		scanInterval.logicalAnd(J);

		corrSetup = P->getCorrSetup(corrSetupName);
		sourceSetup = P->getSourceSetup(src->sourceNames);
		if(!sourceSetup)
		{
			cerr << "Error: no source setup for " << S->sourceDefName << ".  Aborting!" << endl;

			exit(EXIT_FAILURE);
		}
		pointingCentre = &(sourceSetup->pointingCentre);
		scan->nPhaseCentres = sourceSetup->phaseCentres.size();
		if(sourceSetup->doPointingCentre)
		{
			++scan->nPhaseCentres;
		}
		if(scan->nPhaseCentres > maxScanPhaseCentres)
		{
			maxScanPhaseCentres = scan->nPhaseCentres;
		}
		atSource = 0;
		pointingSrcIndex = -1;
		srcra = src->ra;
		srcdec = src->dec;
		if(pointingCentre->ra > PhaseCentre::DEFAULT_RA)
		{
			srcra = pointingCentre->ra;
		}
		if(pointingCentre->dec > PhaseCentre::DEFAULT_DEC)
		{
			srcdec = pointingCentre->dec;
		}
		for(int i = 0; i < D->nSource; ++i)
		{
			radiff  = fabs(D->source[i].ra - srcra);
			decdiff = fabs(D->source[i].dec - srcdec);
			if(radiff < MAX_POS_DIFF && decdiff < MAX_POS_DIFF &&
			   D->source[i].calCode[0] == pointingCentre->calCode &&
			   D->source[i].qual == pointingCentre->qualifier)
			 {
			 	if(pointingCentre->difxName.compare(PhaseCentre::DEFAULT_NAME) != 0)
				{
					if(strcmp(D->source[i].name, pointingCentre->difxName.c_str()) == 0)
					{
						pointingSrcIndex = i;
						break;
					}
				}
				else
				{
					if(strcmp(D->source[i].name, src->defName.c_str()) == 0)
					{
						pointingSrcIndex = i;
						break;
					}
				}
			}
#warning "FIXME: There might be something fishy in the source name comparison above."
			// What happens if the source is renamed?  A better infrastructure for this is needed.
			// Also, code now compares against the def name in case that is the name basis
		}
		if(pointingSrcIndex == -1)
		{
			pointingSrcIndex = D->nSource;
			// below we take the first source name index by default
			snprintf(D->source[pointingSrcIndex].name, DIFXIO_NAME_LENGTH, "%s", src->sourceNames[0].c_str());
			D->source[pointingSrcIndex].ra = src->ra;
			D->source[pointingSrcIndex].dec = src->dec;
			D->source[pointingSrcIndex].calCode[0] = pointingCentre->calCode;
			D->source[pointingSrcIndex].qual = pointingCentre->qualifier;
			//overwrite with stuff from the source setup if it exists
			if(pointingCentre->difxName.compare(PhaseCentre::DEFAULT_NAME) != 0)
			{
				snprintf(D->source[pointingSrcIndex].name, DIFXIO_NAME_LENGTH, "%s", pointingCentre->difxName.c_str());
			}
			if(pointingCentre->ra > PhaseCentre::DEFAULT_RA)
			{
				D->source[pointingSrcIndex].ra = pointingCentre->ra;
			}
			if(pointingCentre->dec > PhaseCentre::DEFAULT_DEC)
			{
				D->source[pointingSrcIndex].dec = pointingCentre->dec;
			}
			++D->nSource;
		}
		scan->pointingCentreSrc = pointingSrcIndex;
		if(sourceSetup->doPointingCentre)
		{
			scan->phsCentreSrcs[atSource] = pointingSrcIndex;
			++atSource;
		}
		for(vector<PhaseCentre>::const_iterator p=sourceSetup->phaseCentres.begin(); p != sourceSetup->phaseCentres.end(); ++p)
		{
			foundSrcIndex = -1;
			for(int i = 0; i < D->nSource; ++i)
			{
				if(D->source[i].ra == p->ra && D->source[i].dec == p->dec &&
					D->source[i].calCode[0] == p->calCode &&
					D->source[i].qual == p->qualifier     &&
					strcmp(D->source[i].name, p->difxName.c_str()) == 0)
				{
					foundSrcIndex = i;
					break;
				}
			}
			if(foundSrcIndex == -1)
			{
				foundSrcIndex = D->nSource;
				snprintf(D->source[foundSrcIndex].name, DIFXIO_NAME_LENGTH, "%s", p->difxName.c_str());
				D->source[foundSrcIndex].ra = p->ra;
				D->source[foundSrcIndex].dec = p->dec;
				D->source[foundSrcIndex].calCode[0] = p->calCode;
				D->source[foundSrcIndex].qual = p->qualifier;
				++D->nSource;
			}
			scan->phsCentreSrcs[atSource] = foundSrcIndex; 
			++atSource;
		}

		scan->mjdStart = scanInterval.mjdStart;
		scan->mjdEnd = scanInterval.mjdStop;
		scan->startSeconds = static_cast<int>((scanInterval.mjdStart - J.mjdStart)*86400.0 + 0.01);
		scan->durSeconds = static_cast<int>(scanInterval.duration_seconds() + 0.01);
		if(scan->durSeconds == 0)
		{
			scan->durSeconds = 1;
		}
		scan->configId = getConfigIndex(configs, D, V, P, S);
		scan->maxNSBetweenUVShifts = corrSetup->maxNSBetweenUVShifts;
		fftDurNS = static_cast<int>(1000000000.0/corrSetup->FFTSpecRes);  
		if(corrSetup->maxNSBetweenACAvg > 0)
		{
			scan->maxNSBetweenACAvg = corrSetup->maxNSBetweenACAvg;
		}
		else
		{
			scan->maxNSBetweenACAvg = defaultMaxNSBetweenACAvg;
		}
		if(corrSetup->numBufferedFFTs*fftDurNS > scan->maxNSBetweenACAvg)
		{
			if(corrSetup->maxNSBetweenACAvg != 0)	// Only print warning if explicitly overriding user value
			{
				cout << "Adjusting maxNSBetweenACAvg since the number of buffered FFTs (";
				cout << corrSetup->numBufferedFFTs << ") gives a duration of ";
				cout << corrSetup->numBufferedFFTs*fftDurNS << ", longer that that specified (";
				cout << corrSetup->maxNSBetweenACAvg << ")" << endl;
			}
			scan->maxNSBetweenACAvg = corrSetup->numBufferedFFTs*fftDurNS;
		}
		if(corrSetup->numBufferedFFTs*fftDurNS > corrSetup->maxNSBetweenUVShifts)
		{
			cout << "The number of buffered FFTs (" << corrSetup->numBufferedFFTs;
			cout << ") gives a duration of " << corrSetup->numBufferedFFTs*fftDurNS;
			cout << ", longer that that specified for the UV shift interval (";
			cout << corrSetup->maxNSBetweenUVShifts;
			cout << "). Reduce FFT buffering or increase allowed interval!" << endl;

			exit(EXIT_FAILURE);
		}

		snprintf(scan->identifier, DIFXIO_NAME_LENGTH, "%s", S->defName.c_str());
		snprintf(scan->obsModeName, DIFXIO_NAME_LENGTH, "%s", S->modeDefName.c_str());

		if(sourceSetup->pointingCentre.isSpacecraft())
		{
			spacecraftSet.insert(sourceSetup->pointingCentre.difxName);
		}
		for(vector<PhaseCentre>::const_iterator p=sourceSetup->phaseCentres.begin(); p != sourceSetup->phaseCentres.end(); ++p)
		{
			if(p->isSpacecraft())
			{
				spacecraftSet.insert(p->difxName);
			}
		}
	}

	for(int configId = 0; configId < D->nConfig; ++configId)
	{
		corrSetup = P->getCorrSetup(configs[configId].second);
		if(!corrSetup->binConfigFile.empty())
		{
			++nPulsar;
		}
	}
	if(nPulsar > 0)
	{
		D->pulsar = newDifxPulsarArray(nPulsar);
	}

	// configure datastreams
	
	// Shelves are a bit awkward...  They are currently tied to an antenna, but really they belong to a datastream.
	D->datastream = makeDifxDatastreams(J, V, P, D->nConfig, D->antenna, shelves);
	D->nDatastream = 0;
	for(int configId = 0; configId < D->nConfig; ++configId)
	{
		const VexMode *mode;
		DifxConfig *config;
		int nConfigDatastream;

		config = D->config + configId;
		nConfigDatastream = 0;

		mode = V->getModeByDefName(configs[configId].first);
		if(mode == 0)
		{
			cerr << "Developer error: writeJob: mode[" << configs[configId].first << "] is null" << endl;

			exit(EXIT_FAILURE);
		}

		// Currently only this is supported
		decimation = 1;

		corrSetup = P->getCorrSetup(configs[configId].second);
		if(corrSetup == 0)
		{
			cerr << "Developer error: writeJob: correlator setup[" << configs[configId].second << "] is null" << endl;

			exit(EXIT_FAILURE);
		}

		if(!corrSetup->binConfigFile.empty())
		{
			int ok;

			ok = checkCRLF(corrSetup->binConfigFile.c_str());
			if(ok < 0)
			{
				cerr << "The pulsar bin config file " << corrSetup->binConfigFile << " has problems.  Exiting." << endl;

				exit(EXIT_FAILURE);
			}

			config->pulsarId = D->nPulsar;
			loadPulsarConfigFile(D, corrSetup->binConfigFile.c_str());
			nbin = D->pulsar[D->nPulsar-1].nBin;
			if(D->pulsar[D->nPulsar-1].scrunch > 0)
			{
				nbin = 1;
			}
			if(nbin > maxPulsarBins)
			{
				maxPulsarBins = nbin;
			}

			for(int p = 0; p < D->pulsar[D->nPulsar-1].nPolyco; ++p)
			{
				ok = checkCRLF(D->pulsar[D->nPulsar-1].polyco[p].fileName);
				if(ok < 0)
				{
					cerr << "The pulsar polyco file " << D->pulsar[D->nPulsar-1].polyco[p].fileName << " has problems.  Exiting." << endl;

					exit(EXIT_FAILURE);
				}
			}
		}

		if(!corrSetup->phasedArrayConfigFile.empty())
		{
			config->phasedArrayId = D->nPhasedArray;
			snprintf(D->phasedarray[D->nPhasedArray].fileName, DIFXIO_FILENAME_LENGTH, "%s", corrSetup->phasedArrayConfigFile.c_str());
			++D->nPhasedArray;
		}

		// first iterate over all antennas, making sure all recorded bands are allocated
		for(std::map<std::string,VexSetup>::const_iterator it = mode->setups.begin(); it != mode->setups.end(); ++it)
		{
			const std::string &antName = it->first;
			const VexSetup &setup = it->second;
			int startBand;
			startBand = 0;
			for(unsigned int ds = 0; ds < setup.nStream(); ++ds)
			{
				const VexStream &stream = setup.streams[ds];
				// the zero below is just to provide a legal slot to do some prodding.  the loop below will properly populate all datastreams.
				setFormat(D, 0, freqs, toneSets, mode, antName, startBand, setup, stream, corrSetup, P->v2dMode);
				startBand += stream.nRecordChan;
			}
		}

		minChans = corrSetup->minInputChans();
		for(std::map<std::string,VexSetup>::const_iterator it = mode->setups.begin(); it != mode->setups.end(); ++it)
		{
			const std::string &antName = it->first;
			const VexSetup &setup = it->second;
			int startBand;
			startBand = 0;

			if(find(J.jobAntennas.begin(), J.jobAntennas.end(), antName) == J.jobAntennas.end())
			{
				continue;
			}

			const VexAntenna *antenna = V->getAntenna(antName);
			antennaSetup = P->getAntennaSetup(antName);

			for(unsigned int ds = 0; ds < setup.nStream(); ++ds)
			{
				const VexStream &stream = setup.streams[ds];
				int v = setFormat(D, D->nDatastream, freqs, toneSets, mode, antName, startBand, setup, stream, corrSetup, P->v2dMode);
				if(v)
				{
					dd = D->datastream + D->nDatastream;
					dd->phaseCalIntervalMHz = setup.phaseCalIntervalMHz();
					dd->tcalFrequency = antenna->tcalFrequency;

					// FIXME: eventually zoom bands will migrate to the VexMode/VexSetup infrastructure.  until then, use antenanSetup directly
					if(antennaSetup)
					{
						nZoomBands = 0;
						
						int nZoomFreqs = antennaSetup->zoomFreqs.size();

						if(nZoomFreqs > 0)
						{
							int *parentFreqIndices = new int[nZoomFreqs];
							int nZoom;	// actual number of zoom freqs used
							int nZoomSkip = 0;

							DifxDatastreamAllocZoomFreqs(dd, nZoomFreqs);
							
							nZoom = 0;
							for(int i = 0; i < nZoomFreqs; ++i)
							{
								const ZoomFreq &zf = antennaSetup->zoomFreqs[i];

								parentFreqIndices[nZoom] = -1;
								for(int j = 0; j < dd->nRecFreq; ++j)
								{
									if(matchingFreq(zf, dd, j, freqs))
									{
										parentFreqIndices[nZoom] = j;
									}
								}
								if(parentFreqIndices[nZoom] < 0)
								{
									nZoomSkip++;
									cerr << "Warning: Cannot find a parent freq for zoom band " << i << " (" << zf.frequency << ") of datastream " << ds << " for antenna " << antName << endl;

									continue;
								}

								zoomChans = static_cast<int>(zf.bandwidth/corrSetup->FFTSpecRes);
								if(zoomChans < minChans)
								{
									minChans = zoomChans;
								}
								fqId = getFreqId(freqs, zf.frequency, zf.bandwidth, 'U', corrSetup->FFTSpecRes, corrSetup->outputSpecRes, decimation, 1, 0);	// final zero points to the noTone pulse cal setup.
								dd->zoomFreqId[nZoom] = fqId;
								dd->nZoomPol[nZoom] = dd->nRecPol[parentFreqIndices[nZoom]];
								nZoomBands += dd->nRecPol[parentFreqIndices[nZoom]];
								if(!zf.correlateparent)
								{
									blockedfreqids[dd->antennaId].insert(dd->recFreqId[parentFreqIndices[nZoom]]);
								}
								++nZoom;
							}
							if(nZoomSkip > 0)
							{
								cerr << "Warning: dropped " << nZoomSkip << " zoom bands, " << nZoom << " remain" << endl;
							}
                                                        dd->nZoomFreq = nZoom;
							DifxDatastreamAllocZoomBands(dd, nZoomBands);

							nZoomBands = 0;
							for(int i = 0; i < nZoom; ++i)
							{
								int k = 0;

								polcount = 0;
								for(int j = 0; j < dd->nZoomPol[i]; ++j)
								{
									dd->zoomBandFreqId[nZoomBands] = i;
									for(; k < dd->nRecBand; ++k)
									{
										if(dd->recBandFreqId[k] == parentFreqIndices[i])
										{
											dd->zoomBandPolName[nZoomBands] = dd->recBandPolName[k];

											++polcount;
											++k;

											break;
										}
									}
									++nZoomBands;
								}
								if(polcount != dd->nZoomPol[i])
								{
									cout << "Developer error: didn't find all zoom pols (was looking for " << dd->nZoomPol[i] << ", only found " << polcount << ")!!" << endl;
									cout << "  Ant=" << antName << " zoomfreq " << i << "/" << nZoom << " nZoomPol[" << i << "]=" << dd->nZoomPol[i] << std::endl;

									exit(EXIT_FAILURE);
								}
							}
							delete [] parentFreqIndices;
						} // if zoom freqs

						int nFreqClockOffsets = antennaSetup->freqClockOffs.size();
						int nFreqClockOffsetsDelta = antennaSetup->freqClockOffsDelta.size();
						int nFreqPhaseDelta = antennaSetup->freqPhaseDelta.size();
						if(nFreqClockOffsets > 0)
						{
							if((startBand + D->datastream[D->nDatastream].nRecFreq) > nFreqClockOffsets ||
							   (startBand + D->datastream[D->nDatastream].nRecFreq) > nFreqClockOffsetsDelta ||
							   (startBand + D->datastream[D->nDatastream].nRecFreq) > nFreqPhaseDelta)
							{
								cerr << endl;
								cerr << "Error: AntennaSetup for " << antName << " has only " << nFreqClockOffsets << " freqClockOffsets specified but " << (startBand + dd->nRecFreq) << " recorded frequencies needed, or" << endl;
								cerr << "Error: AntennaSetup for " << antName << " has only " << nFreqClockOffsetsDelta << " freqClockOffsetsDelta specified but " << (startBand + dd->nRecFreq) << " recorded frequencies needed, or" << endl;
								cerr << "Error: AntennaSetup for " << antName << " has only " << nFreqPhaseDelta << " freqPhaseDelta specified but " << (startBand + dd->nRecFreq) << " recorded frequencies needed." << endl;

								exit(EXIT_FAILURE);
							}
							if(antennaSetup->freqClockOffs.front() != 0.0)
							{
								cerr << endl;
								cerr << "Error: AntennaSetup for " << antName << " has a non-zero clock offset for the first" << " frequency offset. This is not allowed for model " << "accountability reasons." << endl;

								exit(EXIT_FAILURE);
							}
							for(int i = 0; i < D->datastream[D->nDatastream].nRecFreq; ++i)
							{
								double freqClockOffs, freqClockOffsDelta, freqPhaseDelta;

								freqClockOffs = (startBand + i < nFreqClockOffsets) ? antennaSetup->freqClockOffs.at(startBand + i) : 0.0;
								D->datastream[D->nDatastream].clockOffset[i] = freqClockOffs;

								freqClockOffsDelta = (startBand + i < nFreqClockOffsetsDelta) ? antennaSetup->freqClockOffsDelta.at(startBand + i) : 0.0;
								D->datastream[D->nDatastream].clockOffsetDelta[i] = freqClockOffsDelta;
								
								freqPhaseDelta = (startBand + i < nFreqPhaseDelta) ? antennaSetup->freqPhaseDelta.at(startBand + i) : 0.0;
								D->datastream[D->nDatastream].phaseOffset[i] = freqPhaseDelta;
							}
						}

						// TODO: consider specifying loOffsets per datastream rather than per antenna
						int nLoOffsets = antennaSetup->loOffsets.size();
						if(nLoOffsets > 0)
						{
							if((startBand + D->datastream[D->nDatastream].nRecFreq) > nLoOffsets)
							{
								cerr << endl;
								cerr << "Error: AntennaSetup for " << antName << " has only " << nLoOffsets << " loOffsets specified but " << (startBand + dd->nRecFreq) << " recorded frequencies needed." << endl;
								exit(EXIT_FAILURE);
							}
							for(int i = 0; i < D->datastream[D->nDatastream].nRecFreq; ++i)
							{
								double loOffset;
								
								loOffset = (startBand + i < nLoOffsets) ? antennaSetup->loOffsets.at(startBand + i) : 0.0;
								D->datastream[D->nDatastream].freqOffset[i] = loOffset;
							}
						}
					} // if antennaSetup
					config->datastreamId[nConfigDatastream] = D->nDatastream;
					++D->nDatastream;
					++nConfigDatastream;
				} // if valid format
				startBand += stream.nRecordChan;
			} // datastream loop
		} // antenna loop
		if(corrSetup->xmacLength > minChans)
		{
			cerr << "Error: xmacLength set explicitly to " << corrSetup->xmacLength << ", but minChans (from a zoom freq) was " << minChans << endl;
				
			exit(EXIT_FAILURE);
		}
		worstcaseguardns = calculateWorstcaseGuardNS(mode->getLowestSampleRate(), config->subintNS, mode->getMinBits(), mode->getMinSubbands());
		if(config->guardNS < worstcaseguardns && config->guardNS > 0)
		{
			cerr << "vex2difx calculates the worst-case guardNS as " << worstcaseguardns << ", but you have explicitly set " << config->guardNS << ". It is possible that mpifxcorr will refuse to run! Unless you know what you are doing, you should probably set guardNS to " << worstcaseguardns << " or above, or just leave it unset!" << endl;
			if(strict)
			{
				cerr << "\nExiting since strict mode was enabled" << endl;
				
				exit(EXIT_FAILURE);
			}
			else
			{
				cerr << "\nContinuing since --force was specified" << endl;
			}
		}
		config->nDatastream = nConfigDatastream;
	} // configId loop
		
	if(nPulsar != D->nPulsar)
	{
		cerr << "Error: nPulsar=" << nPulsar << " != D->nPulsar=" << D->nPulsar << endl;
		
		exit(EXIT_FAILURE);
	}

	// Make EOP table
	populateEOPTable(D, V->getEOPs());

	// Populate spacecraft table
	if(!spacecraftSet.empty())
	{
		DifxSpacecraft *ds;
		int v;

		D->spacecraft = newDifxSpacecraftArray(spacecraftSet.size());
		D->nSpacecraft = spacecraftSet.size();
		
		ds = D->spacecraft;

		for(set<string>::const_iterator s = spacecraftSet.begin(); s != spacecraftSet.end(); ++s, ++ds)
		{
			phaseCentre = P->getPhaseCentre(*s);
			if(!phaseCentre)
			{
				cerr << "Developer error: couldn't find " << *s << " in the spacecraft table, aborting!)" << endl;
				
				exit(EXIT_FAILURE);
			}

			const long int deltaT = phaseCentre->ephemDeltaT;	// seconds -- interval between ephemeris calculations.  24 sec is normal
			const int intMJD = static_cast<int>(D->mjdStart);
			// start time in seconds rounded down to nearest 2 minute boundary, and then one more
			const int secStart = (static_cast<int>((D->mjdStart - intMJD)*720.0) - 1)*120;
			// end time in seconds rounded up to nearest 2 minute boundary, and then one more
			const int secEnd = static_cast<int>((D->mjdStop - intMJD)*720.0 + 1.999)*120;
			const int nPoint = (secEnd - secStart)/deltaT + 1;
			const double mjd0 = intMJD + secStart/86400.0;

			/* initialize state vector structure with evaluation times */
			ds->nPoint = nPoint;
			ds->pos = (sixVector *)calloc(ds->nPoint, sizeof(sixVector));
			for(int p = 0; p < nPoint; ++p)
			{
				sixVectorSetTime(ds->pos + p, intMJD, secStart + p*deltaT);
			}

			if(!phaseCentre->ephemObject.empty())		// process a .bsp file through spice
			{
				const char *naifFile;

				if(verbose > 0)
				{
					cout << "Computing ephemeris:" << endl;
					cout << "  source name = " << phaseCentre->difxName << endl;
					cout << "  ephem object name = " << phaseCentre->ephemObject << endl;
					cout << "  start mjd = " << intMJD << "  sec = " << secStart << endl;
					cout << "  deltaT = " << phaseCentre->ephemDeltaT << " sec" << endl;
					cout << "  nPoint = " << nPoint << endl;
					cout << "  ephemFile = " << phaseCentre->ephemFile << endl;
					cout << "  naifFile = " << phaseCentre->naifFile << endl;
					cout << "  ephemStellarAber = " << phaseCentre->ephemStellarAber << endl;
					cout << "  ephemClockError = " << phaseCentre->ephemClockError << endl;
				}

				if(phaseCentre->naifFile.empty())
				{
					v = populateSpiceLeapSecondsFromEOP(D->eop, D->nEOP);
					if(v != 0)
					{
						cerr << "Error: populateSpiceLeapSecondsFromEOP returned " << v << endl;

						exit(EXIT_FAILURE);
					}
					naifFile = 0;
				}
				else
				{
					naifFile = phaseCentre->naifFile.c_str();
				}

				v = computeDifxSpacecraftEphemeris(ds, mjd0, deltaT/86400.0, nPoint, 
					phaseCentre->ephemObject.c_str(),
					naifFile,
					phaseCentre->ephemFile.c_str(), 
					phaseCentre->ephemStellarAber,
					phaseCentre->ephemClockError);
				if(v != 0)
				{
					cerr << "Error: ephemeris calculation failed.  Must stop." << endl;
					
					exit(EXIT_FAILURE);
				}
			}
			else if(phaseCentre->isFixedSource())
			{
				v = computeDifxSpacecraftEphemerisFromXYZ(ds, mjd0, deltaT/86400.0, nPoint, 
					phaseCentre->X, phaseCentre->Y, phaseCentre->Z,
					phaseCentre->naifFile.c_str(),
					phaseCentre->ephemClockError);
				if(v != 0)
				{
					cerr << "Error: ephemeris calculation failed.  Must stop." << endl;
					
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				cerr << "Developer error: unknown source type; don't know how to compute state vectors." << endl;

				exit(EXIT_FAILURE);
			}

			// give the spacecraft table the right name so it can be linked to the source
			snprintf(ds->name, DIFXIO_NAME_LENGTH, "%s", phaseCentre->difxName.c_str());
		}

		//Fill in the spacecraft IDs in the DifxInput object
		for(int s = 0; s < D->nSource; ++s)
		{
			for(int sc = 0; sc < D->nSpacecraft; ++sc)
			{
				if(strcmp(D->spacecraft[sc].name, D->source[s].name) == 0)
				{
					D->source[s].spacecraftId = sc;
					
					break;
				}
			}
		}
	}

	// Make frequency table
	populateFreqTable(D, freqs, toneSets);

	// Make baseline table
	globalBandwidth = populateBaselineTable(D, P, corrSetup, blockedfreqids);
	if(globalBandwidth < 0)	// Implies conflicting frequencies found
	{
		cerr << "Warning: differing correlation channel bandwidths found.  You can correlate this data, but won't be able to convert to FITS!" << endl;
	}
	if(globalBandwidth == 0) // Implies no baselines found
	{
		cerr << "Warning: no correlatable baselines were found." << endl;
	}

	// Merge identical table entries
	simplifyDifxFreqs(D);
	simplifyDifxDatastreams(D);
	simplifyDifxBaselines(D);
	simplifyDifxConfigs(D);

	//delete any unused rules
	simplifyDifxRules(D);

	//delete the "blocked freq" array
	blockedfreqids.clear();

	if(P->simFXCORR)
	{
		// nudge integration times and start times to match those of the VLBA HW correlator
		DifxInputSimFXCORR(D);
	}

	//All averaging will always be in correlator by default, not difx2fits
	D->specAvg  = 1;

	if(D->nBaseline > 0 || P->minSubarraySize == 1)
	{
		// write input file
		writeDifxInput(D);

		// write calc file
		writeDifxCalc(D);

		// write threads file if requested
		if(P->nCore > 0 && P->nThread > 0)
		{
			DifxInputAllocThreads(D, P->nCore);
			DifxInputSetThreads(D, P->nThread);
			DifxInputWriteThreads(D);
		}

		// write machines file if possible
		if(!P->machines.empty())
		{
			char machinesFile[DIFXIO_FILENAME_LENGTH];
			FILE *out;

			generateDifxJobFileBase(D->job, machinesFile);
			strcat(machinesFile, ".machines");

			out = fopen(machinesFile, "w");
			if(!out)
			{
				cerr << "Error: cannot open " << machinesFile << " for write." << endl;
			}
			else
			{
				list<string>::const_iterator m = P->machines.begin();

				fprintf(out, "%s\n", m->c_str());
				++m;
				
				for(int a = 0; a < D->nAntenna; ++a)
				{
					const AntennaSetup *A = P->getAntennaSetup(D->antenna[a].name);
					if(!A)
					{
						if(m == P->machines.end())
						{
							cerr << "Warning: fewer than nDatastream+1 machines specified in .v2d file" << endl;
							break;
						}
						fprintf(out, "%s\n", m->c_str());
						++m;
					}
					else
					{
						for(std::vector<DatastreamSetup>::const_iterator dsit = A->datastreamSetups.begin(); dsit != A->datastreamSetups.end(); ++dsit)
						{
							if(!dsit->machine.empty())
							{
								fprintf(out, "%s\n", dsit->machine.c_str());
							}
							else
							{
								if(m == P->machines.end())
								{
									cerr << "Warning: fewer than nDatastream+1 machines specified in .v2d file" << endl;
									break;
								}
								fprintf(out, "%s\n", m->c_str());
								++m;
							}
						}
					}
				}

				for( ;m != P->machines.end(); ++m)
				{
					fprintf(out, "%s\n", m->c_str());
				}
				fclose(out);
			}
		}

		// write flag file
		J.generateFlagFile(*V, events, D->job->flagFile, P->invalidMask);

		if(verbose > 2)
		{
			printDifxInput(D);
		}

		if(of)
		{
			char fileBase[DIFXIO_FILENAME_LENGTH];
			double tops;    // Trillion operations
			int p;

			generateDifxJobFileBase(D->job, fileBase);

			tops = J.calcOps(V, 2*corrSetup->maxInputChans(), corrSetup->doPolar) * 1.0e-12;

			*of << fileBase << " " << J.mjdStart << " " << J.mjdStop << " " << D->nAntenna << " ";
			*of << maxPulsarBins << " " << maxScanPhaseCentres << " ";
			p = of->precision();
			of->precision(4);
			*of << tops << " ";
			*of << (J.dataSize/1000000) << "  #";
			of->precision(p);

			for(vector<string>::const_iterator ai = J.jobAntennas.begin(); ai != J.jobAntennas.end(); ++ai)
			{
				*of << " " << *ai;
			}
			*of << endl;
		}
	}
	else
	{
		char fileBase[DIFXIO_FILENAME_LENGTH];

		generateDifxJobFileBase(D->job, fileBase);

		cerr << "Warning: job " << fileBase << " not written since it correlates no data" << endl;
		cerr << "This is often due to media not being specified, all frequency Ids being" << endl;
		cerr << "unselected, or too many antennas being explicitly unselected by time." << endl;
		cerr << "It is also possible that the vex file is faulty and missing, e.g., an $IF" << endl;
		cerr << "section, leading to missing polarisation information." << endl;

		cerr << "nBaseline=" << D->nBaseline << "  minSubarraySize=" << P->minSubarraySize << endl;
	}

	if(D->nBaseline > 0 || P->minSubarraySize == 1)
	{
		// clean up and return that job was created
		D->config->IF = 0;
		deleteDifxInput(D);

		return 1;
	}
	else
	{
		// clean up and return that job was not created
		deleteDifxInput(D);

		return 0;
	}
}

static void usage(int argc, char **argv)
{
	cout << endl;
	cout << program << " version " << version << "  " << author << " " << verdate << endl;
	cout << endl;
	cout << "Usage:  " << argv[0] << " [<options>] <v2d file>" << endl;
	cout << endl;
	cout << "  <options> can include:" << endl;
	cout << "     -h" << endl;
	cout << "     --help        display this information and quit." << endl;
	cout << endl;
	cout << "     -v" << endl;
	cout << "     --verbose     increase the verbosity of the output; -v -v for more." << endl;
	cout << endl;
	cout << "     -o" << endl;
	cout << "     --output      create a v2d file with all defaults populated." << endl;
	cout << endl;
	cout << "     -d" << endl;
	cout << "     --delete-old  delete all job(s) in this series before running." << endl;
	cout << endl;
	cout << "     -f" << endl;
	cout << "     --force       continue desipte warnings." << endl;
	cout << endl;
	cout << "     -s" << endl;
	cout << "     --strict      treat some warnings as errors and quit [default]." << endl;
	cout << endl;
	cout << "  <v2d file> is the vex2difx configuration file to process." << endl;
	cout << endl;
	cout << "When running " << program << " you will likely see some output to the screen." << endl;
	cout << "Some messages may be important.  Most messages are categorized" << endl;
	cout << "with one of three qualifiers:" << endl;
	cout << "  * Note    This may be normal but usually indicates " << program << " is changing" << endl;
	cout << "            something automatically and that may not be what you want." << endl;
	cout << "  * Warning This is something that does not prevent " << program << " from running" << endl;
	cout << "            but has a high likelihood of doing something different than" << endl;
	cout << "            you intend." << endl;
	cout << "  * Error   " << program << " could not complete due to this problem." << endl;
	cout << endl;
	cout << "See http://cira.ivec.org/dokuwiki/doku.php/difx/vex2difx for more information" << endl;
	cout << endl;
}

static void runCommand(const char *cmd, int verbose)
{
	if(verbose > 0)
	{
		cout << "Executing: " << cmd << endl;
	}
	int s = system(cmd);
	if(s == -1)
	{
		cerr << "Error executing: " << cmd << endl;

		exit(EXIT_FAILURE);
	}
}

// Note: this is approximate, assumes all polarizations matched and no IFs being selected out
static void calculateScanSizes(VexData *V, const CorrParams &P)
{
	int nScan;

	nScan = V->nScan();

	for(int s = 0; s < nScan; ++s)
	{
		const VexScan *scan;
		const VexMode *mode;
		const CorrSetup *setup;
		int nSubband, nBaseline;
		
		scan = V->getScan(s);
		mode = V->getModeByDefName(scan->modeDefName);
		const std::string &corrSetupName = P.findSetup(scan->defName, scan->sourceDefName, scan->modeDefName);
		setup = P.getCorrSetup(corrSetupName);
		if(!setup)
		{
			cerr << "Warning: calculateScanSizes: CorrSetup for scan " << corrSetupName << " not found" << endl;
		}
		else
		{
			nSubband = mode->subbands.size();
			nBaseline = scan->stations.size()*(scan->stations.size()+1)/2;
			V->setScanSize(s, scan->duration()*86400*nBaseline*nSubband*setup->bytesPerSecPerBLPerBand());
		}
	}
}

void writeRemovedAntennasFile(const std::string &missingDataFile, const list<pair<int,string> > &removedAntennas)
{
	set<string> uniq;
	ofstream of;
	int lastJobId = -1;
	int n = 0;

	of.open(missingDataFile.c_str());
	of << "The following job number(s) had antennas removed because they have no baseband data:";
	for(list<pair<int,string> >::const_iterator it = removedAntennas.begin(); it != removedAntennas.end(); ++it)
	{
		if(it->first != lastJobId)
		{
			uniq.clear();
			of << endl;
			of << "job " << it->first << " :";
			lastJobId = it->first;
			++n;
		}
		if(uniq.find(it->second) == uniq.end())
		{
			of << " " << it->second;
			uniq.insert(it->second);
		}
	}
	of << endl;
	of.close();

	cout << endl;
	cout << "Note: " << n << " job(s) had one or more antennas removed due to missing baseband data." << endl;
	cout << "See " << missingDataFile << " for details." << endl;
}

int main(int argc, char **argv)
{
	CorrParams *P;
	VexData *V;
	Shelves shelves;
	const VexScan *S;
	const SourceSetup *sourceSetup;
	list<Event> events;
	set<string> canonicalVDIFUsers;
	vector<Job> J;
	string shelfFile;
	string missingDataFile;	// created if file-based and no files for a particular antenna/job are found
	string v2dFile;
	string command;
	int verbose = 0;
	int ok;
	bool writeParams = false;
	bool deleteOld = false;
	bool strict = true;
	int nWarn = 0;
	int nError = 0;
	int nSkip = 0;
	int nDigit;
	int nJob = 0;
	std::list<std::pair<int,std::string> > removedAntennas;

	if(argc < 2)
	{
		usage(argc, argv);

		return EXIT_FAILURE;
	}

	// force program to work in Universal Time
	setenv("TZ", "", 1);
	tzset();


	for(int a = 1; a < argc; ++a)
	{
		if(argv[a][0] == '-')
		{
			if(strcmp(argv[a], "-h") == 0 ||
			   strcmp(argv[a], "--help") == 0)
			{
				usage(argc, argv);

				return EXIT_SUCCESS;
			}
			else if(strcmp(argv[a], "-v") == 0 ||
				strcmp(argv[a], "--verbose") == 0)
			{
				++verbose;
			}
			else if(strcmp(argv[a], "-o") == 0 ||
				strcmp(argv[a], "--output") == 0)
			{
				writeParams = 1;
			}
			else if(strcmp(argv[a], "-d") == 0 ||
				strcmp(argv[a], "--delete-old") == 0)
			{
				deleteOld = 1;
			}
			else if(strcmp(argv[a], "-f") == 0 ||
				strcmp(argv[a], "--force") == 0)
			{
				strict = 0;
			}
			else if(strcmp(argv[a], "-s") == 0 ||
				strcmp(argv[a], "--strict") == 0)
			{
				strict = 1;
			}
			else
			{
				cerr << "Error: unknown option " << argv[a] << endl;
				cerr << "Run with -h for help information." << endl;
				
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			if(!v2dFile.empty())
			{
				cerr << "Error: multiple configuration files provided, only one expected." << endl;
				cerr << "Run with -h for help information." << endl;
				
				exit(EXIT_FAILURE);
			}
			v2dFile = argv[a];
		}
	}

	if(v2dFile.size() > DIFX_MESSAGE_PARAM_LENGTH-2)
	{
		//job numbers into the tens of thousands will be truncated in difxmessage.  Better warn the user.
		cout << "Filename " << v2dFile << " is too long - its job name might be truncated by difxmessage!" << endl;
		cout << "You are strongly encouraged to choose a shorter .v2d name (root shorter than 26 characters)" << endl;
	}

	if(v2dFile.empty())
	{
		cerr << "Error: configuration (.v2d) file expected." << endl;
		cerr << "Run with -h for help information." << endl;
		
		exit(EXIT_FAILURE);
	}

	if(v2dFile.find("_") != string::npos)
	{
		cerr << "Error: you cannot have an underscore (_) in the filename!" << endl;
		cerr << "Please rename it and run again." << endl;
		
		exit(EXIT_FAILURE);
	}

	if(!isalpha(v2dFile[0]))
	{
		cerr << "Error: pass name (.v2d file name) must start with a letter!" << endl;
		cerr << "Please rename it and run again." << endl;
		
		exit(EXIT_FAILURE);
	}

	ok = checkCRLF(v2dFile.c_str());
	if(ok < 0)
	{
		cerr << "The .v2d file has problems.  Exiting." << endl;

		exit(EXIT_FAILURE);
	}

	P = new CorrParams(v2dFile);
	if(P->vexFile.empty())
	{
		cerr << "Error: vex file parameter (vex) not found in file." << endl;
		
		exit(EXIT_FAILURE);
	}

	ok = checkCRLF(P->vexFile.c_str());
	if(ok < 0)
	{
		cerr << "The vex file has problems.  Exiting." << endl;

		exit(EXIT_FAILURE);
	}

	nWarn = P->parseWarnings;

	umask(02);

	shelfFile = P->vexFile.substr(0, P->vexFile.find_last_of('.'));
	shelfFile += string(".shelf");
	nWarn += shelves.load(shelfFile);

	if(verbose > 1 && !shelves.empty())
	{
		std::cout << shelves << std::endl;
	}

	// delete "no data" file before starting
	missingDataFile = v2dFile.substr(0, v2dFile.find_last_of('.'));
	missingDataFile += string(".nodata");
	command = "rm -f " + missingDataFile;
	system(command.c_str());

	V = loadVexFile(P->vexFile, &nWarn);

	if(!V)
	{
		cerr << "Error: cannot load vex file: " << P->vexFile << endl;
		
		exit(EXIT_FAILURE);
	}

	applyCorrParams(V, *P, nWarn, nError, canonicalVDIFUsers);
	calculateScanSizes(V, *P);

	if(!canonicalVDIFUsers.empty())
	{
		cout << "Note: canonical VDIF threads were assumed for the following antennas:";

		for(set<string>::const_iterator it = canonicalVDIFUsers.begin(); it != canonicalVDIFUsers.end(); ++it)
		{
			cout << " " << *it;
		}

		cout << endl;
	}
	
	V->generateEvents(events);
	V->addBreakEvents(events, P->manualBreaks);
	// find a function for this
	for(std::vector<AntennaSetup>::const_iterator as = P->antennaSetups.begin(); as != P->antennaSetups.end(); ++as)
	{
		if(as->mjdStart > 0.0)
		{
			addEvent(events, as->mjdStart, Event::ANTENNA_START, as->vexName);
		}
		if(as->mjdStop > 0.0)
		{
			addEvent(events, as->mjdStop, Event::ANTENNA_STOP, as->vexName);
		}
	}
	events.sort();

	if(verbose > 1)
	{
		cout << *V << endl;
		cout << *P << endl;
	}

	// set min and max bandwidths for each setup
	for(unsigned int s = 0; s < V->nScan(); ++s)
	{
		const VexScan *scan = V->getScan(s);
		const std::string &corrSetupName = P->findSetup(scan->defName, scan->sourceDefName, scan->modeDefName);
		CorrSetup *corrSetup = P->getNonConstCorrSetup(corrSetupName);
		const VexMode *mode = V->getModeByDefName(scan->modeDefName);
		for(map<string,VexSetup>::const_iterator sp = mode->setups.begin(); sp != mode->setups.end(); ++sp)
		{
			for(vector<VexChannel>::const_iterator cp = sp->second.channels.begin(); cp != sp->second.channels.end(); ++cp)
			{
				corrSetup->addRecordedBandwidth(cp->bbcBandwidth);
			}
		}
	}

	// set datastream channels
	// for each antenna with an ANTENNA section
	//   * if ndatastream = 0, just zero the nBand and startBand params to keep them flexible
	//   * else
	//     * get number of recorded channels
	//     * if it changes with mode, then bail out
	//     * in cases where nBand is not set, set it to totalrecchans/ndatastream
	//     * increment startBand to make set of datastreams contiguous
	//     * if startBand + nBand for the last datastream does not sum to ndatastream then bail out
	for(unsigned int a = 0; a < V->nAntenna(); ++a)
	{
		const VexAntenna *ant = V->getAntenna(a);
		AntennaSetup *antSetup = P->getNonConstAntennaSetup(ant->name);
		if(antSetup)
		{
			int nads = antSetup->datastreamSetups.size();	// number of antenna datastreams

			if(nads == 0)
			{
				cerr << "Developer error: Number of Antenna Datastreams for antenna " << ant->name << " is zero!" << endl;

				exit(EXIT_FAILURE);
			}
			else if(nads == 1)
			{
				antSetup->datastreamSetups[0].nBand = 0;
				antSetup->datastreamSetups[0].startBand = 0;
			}
			else
			{
				int nRecChan = V->getNumAntennaRecChans(ant->name);	// returns < 0 if it varies with mode
				int chanCount;

				if(nRecChan < 0)
				{
					cerr << "Error: Cannot use multiple explicit datastreams in cases where number of record channels varies with mode." << endl;
					++nError;
				}

				chanCount = 0;
				for(int ads = 0; ads < nads; ++ads)
				{
					antSetup->datastreamSetups[ads].startBand = chanCount;
					if(antSetup->datastreamSetups[ads].nBand == 0)
					{
						if(nRecChan % nads != 0)
						{
							cerr << "Error: Number of record channels (" << nRecChan << ") does not divide evenly into number of datastreams (" << nads << ").  This error can be avoided by explicitly setting number of channels allocated to each datastream in the DATASTREAM blocks." << endl;
							++nError;
						}
						antSetup->datastreamSetups[ads].nBand = nRecChan/nads;
					}
					chanCount += antSetup->datastreamSetups[ads].nBand;
				}
				if(chanCount != nRecChan)
				{
					cerr << "Error: Number of channels represented in DATASTREAMS (" << chanCount << ") does not equal number in vex file (" << nRecChan << ")." << endl;
					++nError;
				}
			}
		}
	}
	

	if(nError > 0)
	{
		cerr << endl;
		cerr << nError << " fatal errors encountered.  Quitting." << endl;
		cerr << endl;

		exit(EXIT_FAILURE);
	}

	nWarn += P->sanityCheck();
	if(!P->fakeDatasource)
	{
		nWarn += V->sanityCheck();
	}
	else
	{
		cout << "Note: some sanity checks have been disabled because fake data is selected." << endl;
		if(V->nEOP() == 0)
		{
			cout << "Assuming EOPs with zero values" << endl;
			for(int i = -2; i <= 2; ++i)
			{
				V->newEOP()->mjd = static_cast<int>(V->getExper()->mjdStart) + i;
			}
		}
	}
	nWarn += sanityCheckConsistency(V, P);
	if(strict && nWarn > 0)
	{
		if(nWarn == 1)
		{
			cerr << "Quitting since there was a warning and strict mode was enabled." << endl;
		}
		else
		{
			cerr << "Quitting since " << nWarn << " warnings were found and strict mode was enabled." << endl;
		}
		cerr << "Strict mode can be disabled with --force if needed." << endl;
		
		exit(EXIT_FAILURE);
	}
	else if(nWarn > 0)
	{
		if(nWarn == 1)
		{
			cout << "Note: Proceeding even though there was a warning." << endl;
		}
		else
		{
			cout << "Note: Proceeding even though there were " << nWarn << " warnings." << endl;
		}
	}

	// run through all the scans once, creating source setups for any sources that don't have one
	for(unsigned int i = 0; i < V->nScan(); ++i)
	{
		SourceSetup * added;
		S = V->getScan(i);
		sourceSetup = P->getSourceSetup(S->sourceDefName);
		if(!sourceSetup)
		{
			const VexSource *src = V->getSourceByDefName(S->sourceDefName);
			
			added = new SourceSetup(S->sourceDefName);
			added->doPointingCentre = true;
			added->pointingCentre = PhaseCentre(src->ra, src->dec, src->sourceNames[0]);
			P->addSourceSetup(*added);
		}
	}

	if(verbose > 3)
	{

		cout << "Pre-job making events:" << endl;
		printEventList(events);
	}

	makeJobs(J, V, P, events, removedAntennas, verbose);

	if(verbose > 2)
	{
		printEventList(events);
	}

	if(deleteOld)
	{
		const int CommandSize = 512;
		char cmd[CommandSize];
		int v;

		v = snprintf(cmd, CommandSize, "rm -f %s.params %s_*.{input,calc,flag}", v2dFile.c_str(), P->jobSeries.c_str());
		if(v < CommandSize)
		{
			if(verbose > 1)
			{
				cerr << "About to execute: " << cmd << endl;
			}
			runCommand(cmd, verbose);
		}
		else
		{
			cerr << "Developer warning: deletion of old files failed due to string length: " << v << " >= " << CommandSize << endl;
		}
	}

	if(writeParams)
	{
		ofstream of;
		string paramsFile = v2dFile + ".params";

		of.open(paramsFile.c_str());
		of << *P << endl;
		of.close();
	}

	ofstream of;
	string jobListFile = P->jobSeries + ".joblist";
	const char *difxVersion;
	const char *difxLabel;

	difxVersion = getenv("DIFX_VERSION");
	if(!difxVersion)
	{
		cout << endl;
		cout << "Warning: env. variable DIFX_VERSION is not set.  Setting to 'Unknown'" << endl;
		cout << "This means that your version accountability is being compromised!" << endl;
		cout << endl;
		difxVersion = "Unknown";
	}
	difxLabel = getenv("DIFX_LABEL");
	of.open(jobListFile.c_str());
	of.precision(12);
	of << "exper=" << V->getExper()->name << "  v2d=" << v2dFile <<"  pass=" << P->jobSeries << "  mjd=" << current_mjd() << "  DiFX=" << difxVersion << "  vex2difx=" << version << "  vex=" << P->vexFile;
	if(difxLabel)
	{
		of << "  label=" << difxLabel;
	}
	of << endl;
	
	nDigit=0;
	for(int l = J.size()+P->startSeries-1; l > 0; l /= 10)
	{
		++nDigit;
	}
	
	for(vector<Job>::iterator j = J.begin(); j != J.end(); ++j)
	{
		if(verbose > 0)
		{
			cout << *j;
		}
		if(j->jobSeries == "-")
		{
			++nSkip;
		}
		else
		{
			nJob += writeJob(*j, V, P, events, shelves, verbose, &of, nDigit, 0, strict);
		}
	}
	of.close();

	cout << endl;
	cout << nJob << " job(s) created." << endl;

	if(nJob > 0 && P->v2dComment.length() > 0)
	{
		cout << endl;
		cout << "The user supplied the following comments in the .v2d file:" << endl;
		cout << P->v2dComment << endl;
	}

	if(!removedAntennas.empty())
	{
		writeRemovedAntennasFile(missingDataFile, removedAntennas);
	}

	delete V;
	delete P;

	cout << endl;

	if(nJob>0)
	{
		return EXIT_SUCCESS;
	}
	else
	{
		return EXIT_FAILURE;
	}
}
