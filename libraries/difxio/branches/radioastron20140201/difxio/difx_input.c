/***************************************************************************
 *	 Copyright (C) 2007-2015 by Walter Brisken & Adam Deller			   *
 *																		   *
 *	 This program is free software; you can redistribute it and/or modify  *
 *	 it under the terms of the GNU General Public License as published by  *
 *	 the Free Software Foundation; either version 3 of the License, or	   *
 *	 (at your option) any later version.								   *
 *																		   *
 *	 This program is distributed in the hope that it will be useful,	   *
 *	 but WITHOUT ANY WARRANTY; without even the implied warranty of		   *
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		   *
 *	 GNU General Public License for more details.						   *
 *																		   *
 *	 You should have received a copy of the GNU General Public License	   *
 *	 along with this program; if not, write to the						   *
 *	 Free Software Foundation, Inc.,									   *
 *	 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.			   *
 ***************************************************************************/
//===========================================================================
// SVN properties (DO NOT CHANGE)
//
// $Id$
// $HeadURL$
// $LastChangedRevision$
// $Author$
// $LastChangedDate$
//
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

static const void** jma_malloc_malloc_area = NULL;
static size_t* jma_malloc_malloc_size_area = NULL;
static const void** jma_malloc_free_area = NULL;
static size_t jma_malloc_malloc_size = 0;
static size_t jma_malloc_malloc_num = 0;
static size_t jma_malloc_free_size = 0;
static size_t jma_malloc_free_num = 0;
static const size_t jma_malloc_edge_size = 256;
static void jma_malloc_add_malloc(void* p, const size_t s)
{
	if(p == NULL)
	{
		return;
	}
	if(jma_malloc_malloc_num == jma_malloc_malloc_size)
	{
		if(jma_malloc_malloc_size == 0)
		{
			jma_malloc_malloc_size = 1024*1024;
			jma_malloc_malloc_area = (const void**)malloc(sizeof(void*)*jma_malloc_malloc_size);
			jma_malloc_malloc_size_area = (size_t*)malloc(sizeof(size_t)*jma_malloc_malloc_size);
		}
		else {
			jma_malloc_malloc_size *= 2;
			jma_malloc_malloc_area = (const void**)realloc(jma_malloc_malloc_area, sizeof(void*)*jma_malloc_malloc_size);
			jma_malloc_malloc_size_area = (size_t*)realloc(jma_malloc_malloc_size_area, sizeof(size_t)*jma_malloc_malloc_size);
		}
	}
	jma_malloc_malloc_area[jma_malloc_malloc_num] = p;
	jma_malloc_malloc_size_area[jma_malloc_malloc_num] = s;
	++jma_malloc_malloc_num;
	if(s > 2*jma_malloc_edge_size)
	{
		unsigned char* u = p;
		size_t i;
		for(i=0; i < jma_malloc_edge_size; ++i)
		{
			u[i] = u[s-jma_malloc_edge_size+i] = 0x5A;
		}
	}
	if(jma_malloc_free_num)
	{
		size_t i;
		for(i=jma_malloc_free_num; i > 0;)
		{
			--i;
			if(jma_malloc_free_area[i] == p)
			{
				jma_malloc_free_area[i] = NULL;
			}
		}
	}
	return;
}
static void jma_malloc_add_free(const void* p, const char* file, int line)
{
	size_t i;
	int flag = 1;
	if(p == NULL)
	{
		return;
	}
	if(jma_malloc_free_num)
	{
		for(i=jma_malloc_free_num; i > 0;)
		{
			--i;
			if(jma_malloc_free_area[i] == p)
			{
				fprintf(stderr, "Trying to free already freed memory in %s:%d\n", file, line);
				raise(SIGSEGV);
			}
		}
	}
	for(i=jma_malloc_malloc_num; i > 0;)
	{
		--i;
		if(jma_malloc_malloc_area[i] == p)
		{
			jma_malloc_malloc_area[i] = NULL;
			flag = 0;
			/* check for writing outside of allocated area */
			if(jma_malloc_malloc_size_area[i] > 2*jma_malloc_edge_size)
			{
				const unsigned char* u = p;
				size_t s = jma_malloc_malloc_size_area[i];
				size_t j;
				for(j=0; j < jma_malloc_edge_size; ++j)
				{
					if((u[j] != 0x5A) || (u[s-jma_malloc_edge_size+j] != 0x5A))
					{
						fprintf(stderr, "Memory overrun in %s:%d\n", file, line);
						raise(SIGSEGV);
					}
				}
			}
			break;
		}
	}
	if((flag))
	{
		fprintf(stderr, "Trying to free memory that was not allocated here in %s:%d\n", file, line);
		raise(SIGSEGV);
	}
	if(jma_malloc_free_num == jma_malloc_free_size)
	{
		if(jma_malloc_free_size == 0)
		{
			jma_malloc_free_size = 1024*1024;
			jma_malloc_free_area = (const void**)malloc(sizeof(void*)*jma_malloc_free_size);
		}
		else {
			jma_malloc_free_size *= 2;
			jma_malloc_free_area = (const void**)realloc(jma_malloc_free_area, sizeof(void*)*jma_malloc_free_size);
		}
	}
	jma_malloc_free_area[jma_malloc_free_num++] = p;
	return;
}
void* jma_malloc_malloc(size_t size)
{
	unsigned char* u;
	if(size == 0)
	{
		return NULL;
	}
	size += 2*jma_malloc_edge_size;
	u=malloc(size);
	jma_malloc_add_malloc(u, size);
	return u + jma_malloc_edge_size;
}
void jma_malloc_free(void *ptr, const char* file, int line)
{
	unsigned char* u = ptr;
	if(ptr == NULL)
	{
		return;
	}
	u -= jma_malloc_edge_size;
	jma_malloc_add_free(u, file, line);
	free(u);
	return;
}
void *jma_malloc_calloc(size_t nmemb, size_t size)
{
	unsigned char* u;
	size = nmemb*size;
	if(size == 0)
	{
		return NULL;
	}
	size +=  2*jma_malloc_edge_size;
	u= calloc(1, size);
	jma_malloc_add_malloc(u, size);
	return u + jma_malloc_edge_size;
}
void *jma_malloc_realloc(void *ptr, size_t size, const char* file, int line)
{
	unsigned char* u = ptr;
	size_t newsize = size + 2*jma_malloc_edge_size;
	if(ptr == NULL)
	{
		return jma_malloc_malloc(size);
	}
	u -= jma_malloc_edge_size;
	if((size == 0) && (u!=NULL))
	{
		jma_malloc_add_free(u, file, line);
		free(u);
		return NULL;
	}
	u = realloc(u, newsize);
	jma_malloc_add_malloc(u, newsize);
	return u + jma_malloc_edge_size;
}
char *jma_malloc_strdup(const char *s)
{
	size_t len = strlen(s);
	char* p;
	len += 1;
	p = jma_malloc_malloc(len);
	memcpy(p, s, len);
	return p;
}
char *jma_malloc_strndup(const char *s, size_t n)
{
	size_t len = strlen(s);
	char* p;
	if(len > n)
	{
		len = n;
	}
	len += 1;
	p = jma_malloc_malloc(len);
	memcpy(p, s, len);
	p[len-1] = 0;
	return p;
}
	    








#include <string.h>
#include <float.h>
#include <stdint.h>
#include "difxio/difx_input.h"
#include "difxio/parsedifx.h"

const int MaxFlags = 32000;

const char toneSelectionNames[][MAX_TONE_SELECTION_STRING_LENGTH] =
	{
		"vex",
		"none",
		"ends",
		"all",
		"smart",
		"most",
		"unknown"
	};

enum ToneSelection stringToToneSelection(const char *str)
{
	enum ToneSelection ts;

	for(ts = 0; ts < NumToneSelections; ++ts)
	{
		if(strcasecmp(str, toneSelectionNames[ts]) == 0)
		{
			break;
		}
	}

	return ts;
}

/* allocate empty structure, do minimal initialization */
DifxInput *newDifxInput()
{
	DifxInput *D;

	D = (DifxInput *)calloc(1, sizeof(DifxInput));
	D->specAvg = 1;
	D->visBufferLength = 32;

	return D;
}

void deleteDifxInput(DifxInput *D)
{
	if(D)
	{
		deleteDifxConfigArray(D->config, D->nConfig);
		deleteDifxDatastreamArray(D->datastream, D->nDatastream);
		deleteDifxBaselineArray(D->baseline, D->nBaseline);
		deleteDifxFreqArray(D->freq, D->nFreq);
		deleteDifxAntennaArray(D->antenna, D->nAntenna);
		deleteDifxScanArray(D->scan, D->nScan);
		deleteDifxSourceArray(D->source, D->nSource);
		deleteDifxEOPArray(D->eop);
		deleteDifxJobArray(D->job, D->nJob);
		deleteDifxRuleArray(D->rule, D->nRule);
		DifxInputAllocThreads(D, 0);
		free(D);
	}
}

void DifxConfigMapAntennas(DifxConfig *dc, const DifxDatastream *ds)
{
	int a, maxa = 0, d;

	for(d = 0; d < dc->nDatastream; ++d)
	{
		a = ds[dc->datastreamId[d]].antennaId;
		if(a > maxa)
		{
			maxa = a;
		}
	}

	dc->nAntenna = maxa+1;
	free(dc->ant2dsId);
	dc->ant2dsId = (int *)malloc((maxa+2)*sizeof(int));
	for(a = 0; a < maxa; ++a)
	{
		dc->ant2dsId[a] = -1;
	}

	for(d = 0; d < dc->nDatastream; ++d)
	{
		a = ds[dc->datastreamId[d]].antennaId;
		dc->ant2dsId[a] = dc->datastreamId[d];
	}
}

void fprintDifxInput(FILE *fp, const DifxInput *D)
{
	int i;

	fprintf(fp, "DifxInput : %p\n", D);
	if(!D)
	{
		return;
	}

	fprintf(fp, "  mjdStart = %14.8f\n", D->mjdStart);
	fprintf(fp, "  mjdStop  = %14.8f\n", D->mjdStop);
	fprintf(fp, "  vis buffer length = %d\n", D->visBufferLength);
	fprintf(fp, "  input Channels = %d\n", D->nInChan);
	fprintf(fp, "  start Channel = %d\n", D->startChan);
	fprintf(fp, "  spectral Avg = %d\n", D->specAvg);
	fprintf(fp, "  output Channels = %d\n", D->nOutChan);

	fprintf(fp, "  nJob = %d\n", D->nJob);
	for(i = 0; i < D->nJob; ++i)
	{
		fprintDifxJob(fp, D->job + i);
	}

	fprintf(fp, "  nConfig = %d\n", D->nConfig);
	for(i = 0; i < D->nConfig; ++i)
	{
		fprintDifxConfig(fp, D->config + i);
	}

	fprintf(fp, "  nFreq = %d\n", D->nFreq);
	for(i = 0; i < D->nFreq; ++i)
	{
		fprintDifxFreq(fp, D->freq + i);
	}

	fprintf(fp, "  nAntenna = %d\n", D->nAntenna);
	for(i = 0; i < D->nAntenna; ++i)
	{
		fprintDifxAntenna(fp, D->antenna + i);
	}

	fprintf(fp, "  nSource = %d\n", D->nSource);
	for(i = 0; i < D->nSource; ++i)
	{
		fprintDifxSource(fp, D->source + i);
	}

	fprintf(fp, "  nScan = %d\n", D->nScan);
	for(i = 0; i < D->nScan; ++i)
	{
		fprintDifxScan(fp, D->scan + i);
	}

	fprintf(fp, "  nEOP = %d\n", D->nEOP);
	if(D->eop) 
	{
		for(i = 0; i < D->nEOP; ++i)
		{
			fprintDifxEOP(fp, D->eop + i);
		}
	}

	fprintf(fp, "  nDatastreamEntries = %d\n", D->nDatastream);
	for(i = 0; i < D->nDatastream; ++i)
	{
		fprintDifxDatastream(fp, D->datastream + i);
	}

	fprintf(fp, "  nBaselineEntries = %d\n", D->nBaseline);
	for(i = 0; i < D->nBaseline; ++i)
		if(D->nBaseline > 1)
		{
			fprintDifxBaseline(fp, D->baseline + i);
		}

	fprintf(fp, "  nSpacecraft = %d\n", D->nSpacecraft);
	for(i = 0; i < D->nSpacecraft; ++i)
	{
		fprintDifxSpacecraft(fp, D->spacecraft + i);
	}

	fprintf(fp, "  nPulsar = %d\n", D->nPulsar);
	for(i = 0; i < D->nPulsar; ++i)
	{
		fprintDifxPulsar(fp, D->pulsar + i);
	}

	fprintf(fp, "\n");
}

void printDifxInput(const DifxInput *D)
{
	fprintDifxInput(stdout, D);
}

void fprintDifxInputSummary(FILE *fp, const DifxInput *D)
{
	int i;

	fprintf(fp, "Summary\n");
	if(!D)
	{
		return;
	}

	fprintf(fp, "  mjdStart = %14.8f\n", D->mjdStart);
	fprintf(fp, "  mjdStop	= %14.8f\n", D->mjdStop);
	//fprintf(fp, "  Input Channels = %d\n", D->nInChan);
	fprintf(fp, "  Start Channel = %d\n", D->startChan);
	fprintf(fp, "  Spectral Avg = %d\n", D->specAvg);
	//fprintf(fp, "  Output Channels = %d\n", D->nOutChan);

	fprintf(fp, "  nJob = %d\n", D->nJob);
	for(i = 0; i < D->nJob; ++i)
	{
		fprintDifxJob(fp, D->job + i);
	}

	fprintf(fp, "  nConfig = %d\n", D->nConfig);
	for(i = 0; i < D->nConfig; ++i)
	{
		fprintDifxConfigSummary(fp, D->config + i);
	}

	fprintf(fp, "  nAntenna = %d\n", D->nAntenna);
	for(i = 0; i < D->nAntenna; ++i)
	{
		fprintDifxAntennaSummary(fp, D->antenna + i);
	}

	fprintf(fp, "  nSource = %d\n", D->nSource);
	for(i = 0; i < D->nSource; ++i)
	{
		fprintDifxSourceSummary(fp, D->source + i);
	}

	fprintf(fp, "  nScan = %d\n", D->nScan);
	for(i = 0; i < D->nScan; ++i)
	{
		fprintDifxScanSummary(fp, D->scan + i);
	}

	fprintf(fp, "  nEOP = %d\n", D->nEOP);
	if(D->eop) for(i = 0; i < D->nEOP; ++i)
			   {
				   fprintDifxEOPSummary(fp, D->eop + i);
			   }

	if(D->nSpacecraft > 0)
	{
		fprintf(fp, "  nSpacecraft = %d\n", D->nSpacecraft);
	}
	for(i = 0; i < D->nSpacecraft; ++i)
	{
		fprintDifxSpacecraft(fp, D->spacecraft + i);
	}

	if(D->nPulsar > 0)
	{
		fprintf(fp, "  nPulsar = %d\n", D->nPulsar);
	}
	for(i = 0; i < D->nPulsar; ++i)
	{
		fprintDifxPulsar(fp, D->pulsar + i);
	}
}

void printDifxInputSummary(const DifxInput *D)
{
	fprintDifxInputSummary(stdout, D);
}

static int *deriveAntennaMap(const DifxInput *D, DifxParameters *p, int *nTelescope)
{
	int nTel, t, r;
	int *antennaMap;
	int nFound = 0;

	r = DifxParametersfind(p, 0, "NUM TELESCOPES");
	if(r >= 0)
	{
		nTel = atoi(DifxParametersvalue(p, r));
	}
	else
	{
		fprintf(stderr, "deriveAntennaMap: NUM TELESCOPES not defined\n");

		return 0;
	}
	
	if(nTel < D->nAntenna)
	{
		fprintf(stderr, "deriveAntennaMap: NUM TELESCOPES too small: \n%d < %d\n", nTel, D->nAntenna);
	}

	antennaMap = (int *)malloc(nTel* sizeof(int));

	for(t = 0; t < nTel; ++t)
	{
		int antennaId;

		r = DifxParametersfind1(p, r+1, "TELESCOPE %d NAME", t);
		if(r < 0)
		{
			fprintf(stderr, "deriveAntennaMap: TELESCOPE %d NAME not found\n", t);
			free(antennaMap);
			
			return 0;
		}
		antennaId = DifxInputGetAntennaId(D, DifxParametersvalue(p, r));
		if(antennaId < 0)
		{
			antennaMap[t] = -1;
		}
		else
		{
			antennaMap[t] = antennaId;
			++nFound;
		}
	}

	if(nFound < D->nAntenna)
	{
		fprintf(stderr, "deriveAntennaMap: too few antenna name matches\n");
		free(antennaMap);

		return 0;
	}

	if(nTelescope)
	{
		*nTelescope = nTel;
	}

	return antennaMap;
}

int polMaskValue(char polName)
{
	switch(polName)
	{
	case 'R':
	case 'r':
		return DIFXIO_POL_R;
	case 'L':
	case 'l':
		return DIFXIO_POL_L;
	case 'X':
	case 'x':
		return DIFXIO_POL_X;
	case 'Y':
	case 'y':
		return DIFXIO_POL_Y;
	default:
		return DIFXIO_POL_ERROR;
	}
}

/* This function populates four array fields (and their
 * scalar-valued companions) in a DifxConfig object:
 *	 freqId2IF[]
 *	 freqIdUsed[]
 *	 IF[]
 *	 pol[]
 */
static int generateAipsIFs(DifxInput *D, int configId)
{
	DifxConfig *dc;
	int bl, f;
	int fqId;

	if(configId < 0)
	{
		fprintf(stderr, "Warning: generateAipsIFs: configId = %d\n", configId);

		return 0;
	}

	dc = D->config + configId;


	/* Prepare some arrays */
	free(dc->freqIdUsed);
	dc->freqIdUsed = (int *)calloc(D->nFreq+1, sizeof(int));
	dc->freqIdUsed[D->nFreq] = -1;

	deleteDifxIFArray(dc->IF);
	dc->IF = 0;
	D->nIF = 0;

	free(dc->freqId2IF);
	dc->freqId2IF = (int *)malloc((D->nFreq+2)*sizeof(int));
	for(f = 0; f < D->nFreq; ++f)
	{
		dc->freqId2IF[f] = -1;
	}
	dc->freqId2IF[D->nFreq] = -2;	/* special list terminator */

	dc->polMask = 0;
	dc->nPol = 0;
	dc->pol[0] = dc->pol[1] = ' ';


	/* Look for used D->freq[] entries and polarization entries */
	for(bl = 0; bl < dc->nBaseline; ++bl)
	{
		int blId;
		DifxBaseline *db;

		blId = dc->baselineId[bl];
		if(blId < 0)
		{
			continue;
		}
		db = D->baseline + blId;

		/* f here refers to the baseline frequency list */
		for(f = 0; f < db->nFreq; ++f)
		{
			int p;

			if(db->nPolProd[f] < 0)
			{
				continue;
			}
			for(p = 0; p < db->nPolProd[f]; ++p)
			{
				DifxDatastream *ds;
				int dsId;
				int band;
				int localFqId;
				int polValue;
				char polName;

				dsId = db->dsA;
				ds = D->datastream + dsId;
				band = db->bandA[f][p];
				if(band < 0 || band >= ds->nRecBand + ds->nZoomBand)
				{
					fprintf(stderr, "Error: generateAipsIFs: bandA=%d out of range: baselineId=%d nRecBandA=%d nZoomBandA=%d\n", band, bl, ds->nRecBand, ds->nZoomBand);
					
					exit(EXIT_FAILURE);
				}
				if(band < ds->nRecBand)	/* this is a rec band */
				{
					localFqId = ds->recBandFreqId[band];
					polName = ds->recBandPolName[band];
					fqId = ds->recFreqId[localFqId];
				}
				else /* this is a zoom band */
				{
					int zb;

					zb = band - ds->nRecBand;
					localFqId = ds->zoomBandFreqId[zb];
					polName = ds->zoomBandPolName[zb];
					fqId = ds->zoomFreqId[localFqId];
				}
				++dc->freqIdUsed[fqId];
				polValue = polMaskValue(polName);
				if(polValue == DIFXIO_POL_ERROR)
				{
					fprintf(stderr, "bad pol name: <%c> for baselineId=%d f=%d fqId=%d localFqIf=%d polId=%d bandA=%d\n", polName, bl, f, fqId, localFqId, p, band);
				}
				dc->polMask |= polValue;

				dsId = db->dsB;
				ds = D->datastream + dsId;
				band = db->bandB[f][p];
				if(band < 0 || band >= ds->nRecBand + ds->nZoomBand)
				{
					fprintf(stderr, "Error: generateAipsIFs: bandB=%d out of range: baselineId=%d nRecBandB=%d nZoomBandB=%d\n", band, bl, ds->nRecBand, ds->nZoomBand);
					
					exit(EXIT_FAILURE);
				}
				if(band < ds->nRecBand)	/* this is a rec band */
				{
					localFqId = ds->recBandFreqId[band];
					polName = ds->recBandPolName[band];
					fqId = ds->recFreqId[localFqId];
				}
				else /* this is a zoom band */
				{
					int zb;

					zb = band - ds->nRecBand;
					localFqId = ds->zoomBandFreqId[zb];
					polName = ds->zoomBandPolName[zb];
					fqId = ds->zoomFreqId[localFqId];
				}
				++dc->freqIdUsed[fqId];
				polValue = polMaskValue(polName);
				if(polValue == DIFXIO_POL_ERROR)
				{
					fprintf(stderr, "bad pol name: <%c> for baselineId=%d f=%d fqId=%d localFqIf=%d polId=%d bandB=%d\n", polName, bl, f, fqId, localFqId, p, band);
				}
				dc->polMask |= polValue;
			}
		}
	}

	if(dc->polMask & DIFXIO_POL_ERROR || 
	   dc->polMask == 0 || 
	   ((dc->polMask & DIFXIO_POL_RL) && (dc->polMask & DIFXIO_POL_XY)) )
	{
		fprintf(stderr, "Error: generateAipsIFs: polMask = 0x%03x is unsupported!\n", dc->polMask);

		return -1;
	}

	/* populate polarization matrix for this configuration */
	if(dc->polMask & DIFXIO_POL_R)
	{
		dc->pol[dc->nPol] = 'R';
		++dc->nPol;
	}
	if(dc->polMask & DIFXIO_POL_L)
	{
		dc->pol[dc->nPol] = 'L';
		++dc->nPol;
	}
	if(dc->polMask & DIFXIO_POL_X)
	{
		dc->pol[dc->nPol] = 'X';
		++dc->nPol;
	}
	if(dc->polMask & DIFXIO_POL_Y)
	{
		dc->pol[dc->nPol] = 'Y';
		++dc->nPol;
	}

	/* Actually construct the IF array */

	/* First count IFs and make map to freqId */
	/* This could be an overestimate if multiple frequencies map to one IF, as could happen if two otherwise identical FreqIds have different pulse cal extractions */
	for(fqId = 0; fqId < D->nFreq; ++fqId)
	{
		if(dc->freqIdUsed[fqId] > 0)
		{
			++dc->nIF;
		}
	}

	/* Then actually build it */
	dc->IF = newDifxIFArray(dc->nIF);
	dc->nIF = 0;	/* zero and recount */
	for(fqId = 0; fqId < D->nFreq; ++fqId)
	{
		int i;

		if(dc->freqIdUsed[fqId] <= 0)
		{
			continue;
		}
		for(i = 0; i < dc->nIF; ++i)
		{
			if(D->freq[fqId].bw == dc->IF[i].bw && (
													(D->freq[fqId].sideband == 'U' && D->freq[fqId].freq == dc->IF[i].freq) ||
													(D->freq[fqId].sideband == 'L' && D->freq[fqId].freq == dc->IF[i].freq + dc->IF[i].bw) ))
			{
				break;
			}
		}
		if(i < dc->nIF)
		{
			dc->freqId2IF[fqId] = i;
		}
		else
		{
			dc->freqId2IF[fqId] = i;
			/* Be nice to downstream code and make _everything_ USB */
			if(D->freq[fqId].sideband == 'L')
			{
				dc->IF[i].freq = D->freq[fqId].freq - D->freq[fqId].bw;
			}
			else
			{
				dc->IF[i].freq = D->freq[fqId].freq;
			}
			dc->IF[i].sideband	= 'U';
			dc->IF[i].bw		= D->freq[fqId].bw;
			dc->IF[i].nPol		= dc->nPol;
			dc->IF[i].pol[0]	= dc->pol[0];
			dc->IF[i].pol[1]	= dc->pol[1];
			strncpy(dc->IF[i].rxName, D->freq[fqId].rxName, DIFXIO_RX_NAME_LENGTH);
			dc->IF[i].rxName[DIFXIO_RX_NAME_LENGTH-1] = 0;

			++dc->nIF;
		}
	}

	/* Set reference frequency to the bottom edge of the first frequency */
	D->refFreq = dc->IF[0].freq;

	return 0;
}

static DifxInput *parseDifxInputCommonTable(DifxInput *D, const DifxParameters *ip)
{
	const char commonKeys[][MAX_DIFX_KEY_LEN] =
		{
			"CALC FILENAME",
			"CORE CONF FILENAME",
			"EXECUTE TIME (SEC)",
			"START MJD",
			"START SECONDS",
			"ACTIVE DATASTREAMS",
			"ACTIVE BASELINES",
			"VIS BUFFER LENGTH",
			"OUTPUT FILENAME"
		};
	const int N_COMMON_ROWS = sizeof(commonKeys)/sizeof(commonKeys[0]);
	int N;
	int rows[N_COMMON_ROWS];
	int v;
	
	if(!D || !ip)
	{
		return 0;
	}

	N = DifxParametersbatchfind(ip, 0, commonKeys, N_COMMON_ROWS, rows);
	if(N < N_COMMON_ROWS)
	{
		fprintf(stderr, "populateInput: N < N_COMMON_ROWS %d < %d\n", N, N_COMMON_ROWS);

		return 0;
	}

	/* Initialize some of the structures */
	
	D->job->duration = atoi(DifxParametersvalue(ip, rows[2]));
	D->job->mjdStart = atoi(DifxParametersvalue(ip, rows[3])) +
					   atof(DifxParametersvalue(ip, rows[4]))/SEC_DAY_DBL;
	if(atof(DifxParametersvalue(ip, rows[4])) != atoi(DifxParametersvalue(ip, rows[4])))
	{
		D->fracSecondStartTime = 1;
	}
	D->job->activeDatastreams = atoi(DifxParametersvalue(ip, rows[5]));
	D->job->activeBaselines	  = atoi(DifxParametersvalue(ip, rows[6]));
	D->visBufferLength		  = atoi(DifxParametersvalue(ip, rows[7]));
	v = snprintf(D->job->calcFile, DIFXIO_FILENAME_LENGTH, "%s", DifxParametersvalue(ip, rows[0]));
	if(v >= DIFXIO_FILENAME_LENGTH)
	{
		fprintf(stderr, "populateInput: CALC FILENAME too long (%d > %d)\n", v, DIFXIO_FILENAME_LENGTH-1);

		return 0;
	}
	v = snprintf(D->job->threadsFile, DIFXIO_FILENAME_LENGTH, "%s", DifxParametersvalue(ip, rows[1]));
	if(v >= DIFXIO_FILENAME_LENGTH)
	{
		fprintf(stderr, "populateInput: CORE CONF FILENAME too long (%d > %d)\n", v, DIFXIO_FILENAME_LENGTH-1);

		return 0;
	}
	v = snprintf(D->job->outputFile, DIFXIO_FILENAME_LENGTH, "%s", DifxParametersvalue(ip, rows[8]));
	if(v >= DIFXIO_FILENAME_LENGTH)
	{
		fprintf(stderr, "populateInput: OUTPUT FILENAME too long (%d > %d)\n", v, DIFXIO_FILENAME_LENGTH-1);

		return 0;
	}

	return D;
}	

/* return -1 on a failure */
static int loadPhasedArrayConfigFile(DifxInput *D, const char *fileName)
{
	DifxParameters *pp;
	DifxPhasedArray *dpa;
	int r;

	if (!D)
	{
		return -1;
	}

	pp = newDifxParametersfromfile(fileName);
	if(!pp) 
	{
		fprintf(stderr, "Problem opening or reading %s\n", fileName);

		return -1;
	}

	D->phasedarray = growDifxPhasedarrayArray(D->phasedarray, D->nPhasedArray);
	dpa = D->phasedarray + D->nPhasedArray;

	/* Fill in the info about quantisation, format etc for this phased array output */
	r = DifxParametersfind(pp, 0, "OUTPUT TYPE");
	dpa->outputType = stringToPhasedArrayOutputType(DifxParametersvalue(pp, r));
	r = DifxParametersfind(pp, r, "OUTPUT FORMAT");
	dpa->outputFormat = stringToPhasedArrayOutputFormat(DifxParametersvalue(pp, r));
	r = DifxParametersfind(pp, r, "ACC TIME (NS)");
	dpa->accTime = atoi(DifxParametersvalue(pp, r));
	r = DifxParametersfind(pp, r, "COMPLEX OUTPUT");
	dpa->complexOutput = 0;
	if(strcasecmp(DifxParametersvalue(pp, r), "TRUE"))
	{
		dpa->complexOutput = 1;
	}
	r = DifxParametersfind(pp, r, "OUTPUT BITS");
	dpa->quantBits = atoi(DifxParametersvalue(pp, r));

	++D->nPhasedArray;

	return D->nPhasedArray-1;
}

/* return -1 on a failure */
int loadPulsarConfigFile(DifxInput *D, const char *fileName)
{
	DifxParameters *pp;
	DifxPulsar *dp;
	int i, r;
	int nPolycoFiles;

	if (!D)
	{
		return -1;
	}

	pp = newDifxParametersfromfile(fileName);
	if(!pp)
	{
		fprintf(stderr, "Problem opening or reading %s\n", fileName);

		return -1;
	}

	D->pulsar = growDifxPulsarArray(D->pulsar, D->nPulsar);
	dp = D->pulsar + D->nPulsar;

	r = DifxParametersfind(pp, 0, "NUM POLYCO FILES");
	if(r < 0)
	{
		deleteDifxParameters(pp);
		fprintf(stderr, "NUM POLYCO FILES not found\n");

		return -1;
	}
	nPolycoFiles = atoi(DifxParametersvalue(pp, r));
	dp->nPolyco = 0;
	dp->polyco = 0;

	snprintf(dp->fileName, DIFXIO_FILENAME_LENGTH, "%s", fileName);

	for(i = 0; i < nPolycoFiles; ++i)
	{
		r = DifxParametersfind1(pp, r, "POLYCO FILE %d", i);
		if(r < 0)
		{
			deleteDifxParameters(pp);
			fprintf(stderr, "POLYCO FILE %d not found\n", i);

			return -1;
		}
		r = loadPulsarPolycoFile(&dp->polyco, &dp->nPolyco, DifxParametersvalue(pp, r));
		if(r < 0)
		{
			deleteDifxParameters(pp);

			return -1;
		}
	}

	r = DifxParametersfind(pp, 0, "NUM PULSAR BINS");
	if(r < 0)
	{
		deleteDifxParameters(pp);
		fprintf(stderr, "NUM PULSAR BINS not found\n");

		return -1;
	}
	dp->nBin = atoi(DifxParametersvalue(pp, r));
	dp->binEnd = (double *)calloc(dp->nBin, sizeof(double));
	dp->binWeight = (double *)calloc(dp->nBin, sizeof(double));

	r = DifxParametersfind(pp, 0, "SCRUNCH OUTPUT");
	if(r < 0)
	{
		deleteDifxParameters(pp);
		fprintf(stderr, "SCRUNCH OUTPUT not found\n");

		return -1;
	}
	if(strcasecmp(DifxParametersvalue(pp, r), "TRUE") == 0)
	{
		dp->scrunch = 1;
	}

	for(i = 0; i < dp->nBin; ++i)
	{
		r = DifxParametersfind1(pp, r, "BIN PHASE END %d", i);
		if(r < 0)
		{
			deleteDifxParameters(pp);
			fprintf(stderr, "BIN PHASE END %d not found\n", i);

			return -1;
		}
		dp->binEnd[i] = atof(DifxParametersvalue(pp, r));

		r = DifxParametersfind1(pp, r, "BIN WEIGHT %d", i);
		if(r < 0)
		{
			deleteDifxParameters(pp);
			fprintf(stderr, "BIN WEIGHT %d not found\n", i);

			return -1;
		}
		dp->binWeight[i] = atof(DifxParametersvalue(pp, r));
	}

	++D->nPulsar;
	deleteDifxParameters(pp);

	return D->nPulsar-1;
}

static DifxInput *parseDifxInputConfigurationTable(DifxInput *D, const DifxParameters *ip)
{
	const char configKeys[][MAX_DIFX_KEY_LEN] =
		{
			"CONFIG NAME",
			"INT TIME (SEC)",
			"SUBINT NANOSECONDS",
			"GUARD NANOSECONDS",
			"FRINGE ROTN ORDER",
			"ARRAY STRIDE LENGTH",
			"XMAC STRIDE LENGTH",
			"NUM BUFFERED FFTS",
			"WRITE AUTOCORRS",
			"PULSAR BINNING",
			"PHASED ARRAY"
		};
	const int N_CONFIG_ROWS = sizeof(configKeys)/sizeof(configKeys[0]);
	int c, r;
	int rows[N_CONFIG_ROWS];

	if(!D || !ip)
	{
		return 0;
	}

	r = DifxParametersfind(ip, 0, "NUM CONFIGURATIONS");
	if(r < 0)
	{
		fprintf(stderr, "NUM CONFIGURATIONS not found\n");

		return 0;
	}
	deleteDifxConfigArray(D->config, D->nConfig);
	D->nConfig	= atoi(DifxParametersvalue(ip, r));
	D->config	= newDifxConfigArray(D->nConfig);
	rows[N_CONFIG_ROWS-1] = 0;	/* initialize start */
	for(c = 0; c < D->nConfig; ++c)
	{
		DifxConfig *dc;
		int N;
		int a, b;

		dc = D->config + c;
		N = DifxParametersbatchfind(ip, rows[N_CONFIG_ROWS-1], configKeys, N_CONFIG_ROWS, rows);
		if(N < N_CONFIG_ROWS)
		{
			fprintf(stderr, "parseDifxInputConfigurations: N < N_CONFIG_ROWS %d "
					"< %d\n", N, N_CONFIG_ROWS);

			return 0;
		}
		snprintf(dc->name, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(ip, rows[0]));
		dc->tInt		   = atof(DifxParametersvalue(ip, rows[1]));
		dc->subintNS	   = atoi(DifxParametersvalue(ip, rows[2]));
		dc->guardNS		   = atoi(DifxParametersvalue(ip, rows[3]));
		dc->fringeRotOrder = atoi(DifxParametersvalue(ip, rows[4]));
		dc->strideLength   = atoi(DifxParametersvalue(ip, rows[5]));
		dc->xmacLength	   = atoi(DifxParametersvalue(ip, rows[6]));
		dc->numBufferedFFTs= atoi(DifxParametersvalue(ip, rows[7]));
		dc->doAutoCorr	   = abs(strcmp("FALSE", DifxParametersvalue(ip, rows[8])));
		dc->nDatastream	 = D->job->activeDatastreams;
		dc->nBaseline	 = D->job->activeBaselines;

		/* MSA stuff --- This is not required to be present */
		r =  DifxParametersfind(ip, 0, "WRITE MSA CALIB");
		if(r >= 0)
		{
			dc->doMSAcalibration = abs(strcmp("FALSE", DifxParametersvalue(ip, r)));
		}
		/* model component table interval --- This is not required to be present */
		r =  DifxParametersfind(ip, 0, "MC TABLE INTERVAL");
		if(r >= 0)
		{
			dc->MC_table_output_interval = atof(DifxParametersvalue(ip, r));
		}
		/* pulsar stuff */
		if(strcmp(DifxParametersvalue(ip, rows[9]), "TRUE") == 0)
		{
			r = DifxParametersfind(ip, rows[9], "PULSAR CONFIG FILE");
			if(r <= 0)
			{
				fprintf(stderr, "input file row %d : PULSAR CONFIG FILE expected\n", rows[11] + 2);

				return 0;
			}
			dc->pulsarId = loadPulsarConfigFile(D, DifxParametersvalue(ip, r));
			if(dc->pulsarId < 0)
			{
				return 0;
			}
		}
		/* phased array stuff */
		if(strcmp(DifxParametersvalue(ip, rows[10]), "TRUE") == 0)
		{
			r = DifxParametersfind(ip, rows[10], "PHASED ARRAY CONFIG FILE");
			if(r <= 0)
			{
				fprintf(stderr, "input file row %d : PHASED ARRAY CONFIG FILE expected\n", rows[12] + 2);

				return 0;
			}
			dc->phasedArrayId = loadPhasedArrayConfigFile(D, DifxParametersvalue(ip, r));
			if(dc->phasedArrayId < 0)
			{
				return 0;
			}
		}
		N = strlen(dc->name);
		dc->doPolar = -1;	/* to be calculated later */

		

		/* initialize datastream index array */
		dc->datastreamId = (int *)malloc(sizeof(int)*(dc->nDatastream + 1));
		
		/* here "a" is "datastream # within conf", not "antenna" */
		for(a = 0; a <= D->nDatastream; a++)
		{
			dc->datastreamId[a] = -1;
		}

		/* populate datastream index array */
		/* here "a" is "datastream # within conf", not "antenna" */
		for(a = 0; a < dc->nDatastream; a++)
		{

			r = DifxParametersfind1(ip, r+1, "DATASTREAM %d INDEX", a);
			if(r < 0)
			{
				fprintf(stderr, "DATASTREAM %d INDEX not found\n", a);

				return 0;
			}
			dc->datastreamId[a] = atoi(DifxParametersvalue(ip, r));
		}

		/* initialize baseline index array; -1 terminated */
		dc->baselineId = (int *)malloc(sizeof(int)* (dc->nBaseline+1));
		for(b = 0; b <= dc->nBaseline; ++b)
		{
			dc->baselineId[b] = -1;
		}

		/* populate baseline index array */
		for(b = 0; b < dc->nBaseline; ++b)
		{
			r = DifxParametersfind1(ip, r+1, 
									"BASELINE %d INDEX", b);
			if(r < 0)
			{
				fprintf(stderr, "BASELINE %d INDEX not found\n", b);

				return 0;
			}
			dc->baselineId[b] = atoi(DifxParametersvalue(ip, r));
		}
	}

	return D;
}

static DifxInput *parseDifxInputRuleTable(DifxInput *D, const DifxParameters *ip)
{
	int r, rule;

	if (!D || !ip)
	{
		return 0;
	}

	r = DifxParametersfind(ip, 0, "NUM RULES");
	if(r<0)
	{
		fprintf(stderr, "NUM RULES not found\n");

		return 0;
	}
	deleteDifxRuleArray(D->rule, D->nRule);
	D->nRule = atoi(DifxParametersvalue(ip, r));
	D->rule	 = newDifxRuleArray(D->nRule);
	for(rule = 0; rule < D->nRule; ++rule)
	{
		r = DifxParametersfind1(ip, r+1, "RULE %d SOURCE", rule);
		if(r>=0)
		{
			//snprintf(D->rule[rule].sourceName, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(ip, r));
			DifxStringArrayaddlist(&D->rule[rule].sourceName, DifxParametersvalue(ip, r));
		}
		r = DifxParametersfind1(ip, r+1, "RULE %d SCAN ID", rule);
		if(r>=0)
		{
			//snprintf(D->rule[rule].scanId, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(ip, r));
			DifxStringArrayaddlist(&D->rule[rule].scanId, DifxParametersvalue(ip, r));
		}
		r = DifxParametersfind1(ip, r+1, "RULE %d CALCODE", rule);
		if(r>=0)
		{
			snprintf(D->rule[rule].calCode, DIFXIO_CALCODE_LENGTH, "%s", DifxParametersvalue(ip, r));
		}
		r = DifxParametersfind1(ip, r+1, "RULE %d QUAL", rule);
		if(r>=0)
		{
			D->rule[rule].qual = atoi(DifxParametersvalue(ip, r));
		}
		r = DifxParametersfind1(ip, r+1, "RULE %d MJD START", rule);
		if(r>=0)
		{
			D->rule[rule].mjdStart = atof(DifxParametersvalue(ip, r));
		}
		r = DifxParametersfind1(ip, r+1, "RULE %d MJD STOP", rule);
		if(r>=0)
		{
			D->rule[rule].mjdStop = atof(DifxParametersvalue(ip, r));
		}
		r = DifxParametersfind1(ip, r+1, "RULE %d CONFIG NAME", rule);
		if(r<0)
		{
			fprintf(stderr, "RULE %d CONFIG NAME not found\n", rule);

			return 0;
		}
		snprintf(D->rule[rule].configName, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(ip, r));
	}

	return D;
}

static DifxInput *parseDifxInputFreqTable(DifxInput *D, const DifxParameters *ip)
{
	const char freqKeys[][MAX_DIFX_KEY_LEN] =
		{
			"FREQ (MHZ) %d",
			"BW (MHZ) %d",
			"SIDEBAND %d",
			"NUM CHANNELS %d",
			"CHANS TO AVG %d",
			"OVERSAMPLE FAC. %d",
			"DECIMATION FAC. %d"
		};
	const int N_FREQ_ROWS = sizeof(freqKeys)/sizeof(freqKeys[0]);
	int b, r;
	int rows[N_FREQ_ROWS];
	
	if(!D || !ip)
	{
		return 0;
	}

	r = DifxParametersfind(ip, 0, "FREQ ENTRIES");
	if(r < 0)
	{
		fprintf(stderr, "FREQ ENTRIES not found\n");

		return 0;
	}
	deleteDifxFreqArray(D->freq, D->nFreq);
	D->nFreq	= atoi(DifxParametersvalue(ip, r));
	D->freq		= newDifxFreqArray(D->nFreq);
	rows[N_FREQ_ROWS-1] = 0;	/* initialize start */
	for(b = 0; b < D->nFreq; ++b)
	{
		int N;

		N = DifxParametersbatchfind1(ip, rows[N_FREQ_ROWS-1], freqKeys, b, N_FREQ_ROWS, rows);
		if(N < N_FREQ_ROWS)
		{
			fprintf(stderr, "populateInput: N < N_FREQ_ROWS %d < %d\n", N, N_FREQ_ROWS);

			return 0;
		}
		D->freq[b].freq		= atof(DifxParametersvalue(ip, rows[0]));
		D->freq[b].bw		= atof(DifxParametersvalue(ip, rows[1]));
		D->freq[b].sideband = DifxParametersvalue(ip, rows[2])[0];
		D->freq[b].nChan	= atoi(DifxParametersvalue(ip, rows[3]));
		D->freq[b].specAvg	= atoi(DifxParametersvalue(ip, rows[4]));
		D->freq[b].overSamp = atoi(DifxParametersvalue(ip, rows[5]));
		D->freq[b].decimation = atoi(DifxParametersvalue(ip, rows[6]));

		r = DifxParametersfind1(ip, rows[2]+1, "RX NAME %d", b);
		if(r > 0 && r < rows[2]+5)
		{
			int v;

			v = snprintf(D->freq[b].rxName, DIFXIO_RX_NAME_LENGTH, "%s", DifxParametersvalue(ip, r));
			if(v > DIFXIO_RX_NAME_LENGTH-1)
			{
				fprintf(stderr, "populateInput: DIFXIO_RX_NAME_LENGTH=%d is too short for rx Name '%s'\n", DIFXIO_RX_NAME_LENGTH, DifxParametersvalue(ip, r));

				return 0;
			}
		}
		
		r = DifxParametersfind1(ip, rows[6]+1, "PHASE CALS %d OUT", b);
		if(r > 0 && r < rows[6]+5)
		{
			int t;

			DifxFreqAllocTones(&(D->freq[b]), atoi(DifxParametersvalue(ip, r)));
			for(t = 0; t < D->freq[b].nTone; ++t)
			{
				r = DifxParametersfind2(ip, r+1, "PHASE CAL %d/%d INDEX", b, t);
				if(r < 0)
				{
					fprintf(stderr, "PHASE CAL %d/%d INDEX not found in .input file\n", b, t);

					return 0;
				}
				D->freq[b].tone[t] = atoi(DifxParametersvalue(ip, r));
			}
		}

		D->nInChan = D->freq[b].nChan;
		D->nOutChan = D->freq[b].nChan/D->freq[b].specAvg;
	}
	
	return D;
}

static DifxInput *parseDifxInputTelescopeTable(DifxInput *D, const DifxParameters *ip)
{
	const char antKeys[][MAX_DIFX_KEY_LEN] =
		{
			"TELESCOPE NAME %d",
			"CLOCK REF MJD %d",
			"CLOCK POLY ORDER %d"
		};
	const int N_ANT_ROWS = sizeof(antKeys)/sizeof(antKeys[0]);
	int a, r;
	int rows[N_ANT_ROWS];

	if(!D || !ip)
	{
		return 0;
	}

	r = DifxParametersfind(ip, 0, "TELESCOPE ENTRIES");
	if(r < 0)
	{
		fprintf(stderr, "Cannot find TELESCOPE ENTRIES table\n");

		return 0;
	}
	deleteDifxAntennaArray(D->antenna, D->nAntenna);
	D->nAntenna = atoi(DifxParametersvalue(ip, r));
	D->antenna	= newDifxAntennaArray(D->nAntenna);

	rows[N_ANT_ROWS-1] = 0;		/* initialize start */
	for(a = 0; a < D->nAntenna; ++a)
	{
		int N;
		int i;

		N = DifxParametersbatchfind1(ip, rows[N_ANT_ROWS-1], antKeys, a, N_ANT_ROWS, rows);
		if(N < N_ANT_ROWS)
		{
			fprintf(stderr, "populateInput: N < N_ANT_ROWS %d < %d\n", N, N_ANT_ROWS);

			return 0;
		}
		snprintf(D->antenna[a].name, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(ip, rows[0]));
		D->antenna[a].clockrefmjd = atof(DifxParametersvalue(ip, rows[1]));
		D->antenna[a].clockorder  = atoi(DifxParametersvalue(ip, rows[2]));
		r = rows[2];
		for(i = 0; i < D->antenna[a].clockorder+1; ++i)
		{
			r = DifxParametersfind2(ip, r, "CLOCK COEFF %d/%d", a, i);
			D->antenna[a].clockcoeff[i] = atof(DifxParametersvalue(ip, r));
		}
	}

	return D;
}

static DifxInput *parseDifxInputDatastreamTable(DifxInput *D, const DifxParameters *ip)
{
	int e, r, r2, nr, nz;
	int nRecBand, nZoomBand;

	if(!D || !ip)
	{
		return 0;
	}

	r = DifxParametersfind(ip, 0, "DATASTREAM ENTRIES");
	if(r < 0)
	{
		fprintf(stderr, "Cannot find DATASTREAM ENTRIES table\n");

		return 0;
	}
	deleteDifxDatastreamArray(D->datastream, D->nDatastream);
	D->nDatastream = atoi(DifxParametersvalue(ip, r));
	D->datastream = newDifxDatastreamArray(D->nDatastream);

	r2 = DifxParametersfind(ip, r+1, "DATA BUFFER FACTOR");
	if(r2 > 0)
	{
		D->dataBufferFactor = atoi(DifxParametersvalue(ip, r2));
	}

	r2 = DifxParametersfind(ip, r+1, "NUM DATA SEGMENTS");
	if(r2 > 0)
	{
		D->nDataSegments = atoi(DifxParametersvalue(ip, r2));
	}

	for(e = 0; e < D->nDatastream; ++e)
	{
		int i, r1;

		r = DifxParametersfind(ip, r+1, "TELESCOPE INDEX");
		if(r < 0)
		{
			fprintf(stderr, "TELESCOPE INDEX not found\n");
			
			return 0;
		}
		D->datastream[e].antennaId = atoi(DifxParametersvalue(ip, r));
		
		r = DifxParametersfind(ip, r+1, "DATA FORMAT");
		if(r < 0)
		{
			fprintf(stderr, "DATA FORMAT not found\n");
			
			return 0;
		}
		snprintf(D->datastream[e].dataFormat, DIFXIO_FORMAT_LENGTH, "%s", DifxParametersvalue(ip, r));
	
		r = DifxParametersfind(ip, r+1, "QUANTISATION BITS");
		if(r < 0)
		{
			fprintf(stderr, "Cannot determine quantization bits\n");
			
			return 0;
		}
		D->datastream[e].quantBits = atoi(DifxParametersvalue(ip, r));

		r = DifxParametersfind(ip, r+1, "DATA FRAME SIZE");
		if(r < 0)
		{
			fprintf(stderr, "Cannot determine data frame size\n");
			
			return 0;
		}
		D->datastream[e].dataFrameSize = atoi(DifxParametersvalue(ip, r));

		r = DifxParametersfind(ip, r+1, "DATA SAMPLING");
		if(r < 0)
		{
			fprintf(stderr, "DATA SAMPLING not found\n");
			
			return 0;
		}
		D->datastream[e].dataSampling = stringToSamplingType( DifxParametersvalue(ip, r) );
		if(D->datastream[e].dataSampling >= NumSamplingTypes)
		{
			fprintf(stderr, "Error: DATA SAMPLING was %s and is not supported\n", 
					DifxParametersvalue(ip, r) );
			
			return 0;
		}
		
		r = DifxParametersfind(ip, r+1, "DATA SOURCE");
		if(r < 0)
		{
			fprintf(stderr, "Cannot determine data source\n");
			
			return 0;
		}
		D->datastream[e].dataSource = stringToDataSource( DifxParametersvalue(ip, r) );
		if(D->datastream[e].dataSource >= NumDataSources)
		{
			fprintf(stderr, "Error: DATA SOURCE was %s and is not supported\n",
					DifxParametersvalue(ip, r) );
			
			return 0;
		}

		/* note use of r1 here: this parameter is optional and not unique. */
		r1 = DifxParametersfind_limited(ip, r+1, 5, "TCAL FREQUENCY");
		if(r1 > 0)
		{
			D->datastream[e].tcalFrequency = atoi(DifxParametersvalue(ip, r1));
		}

		r = DifxParametersfind(ip, r+1, "PHASE CAL INT (MHZ)");
		if(r < 0)
		{
			fprintf(stderr, "Cannot determine phase cal interval\n");

			return 0;
		}
		D->datastream[e].phaseCalIntervalMHz = atoi(DifxParametersvalue(ip, r));

		r = DifxParametersfind(ip, r+1, "NUM RECORDED FREQS");
		if(r < 0)
		{
			fprintf(stderr, "NUM RECORDED FREQS not found\n");
			
			return 0;
		}

		DifxDatastreamAllocFreqs(D->datastream + e, atoi(DifxParametersvalue(ip, r)));

		nRecBand = 0;
		for(i = 0; i < D->datastream[e].nRecFreq; ++i)
		{
			char *clockOffStr, *str, *tok;

			r = DifxParametersfind1(ip, r+1, "REC FREQ INDEX %d", i);
			D->datastream[e].recFreqId[i] = atoi(DifxParametersvalue(ip, r));
			r = DifxParametersfind1(ip, r+1, "CLK OFFSET %d (us)", i);

			/* look for value of form	ClockOffset[:ClockOffsetDelta[:PhaseOffset]] */

			clockOffStr = strdup(DifxParametersvalue(ip, r));
			if(!clockOffStr)
			{
				perror("Could not allocate temporary memory");
				
				return 0;
			}
			str = clockOffStr;
			tok = strsep(&str, ":");
			if(tok == 0)
			{
				fprintf(stderr, "Failed to parse CLK OFFSET %d: %s\n", i, DifxParametersvalue(ip, r));

				return	0;
			}
			D->datastream[e].clockOffset[i] = atof(tok);
			tok = strsep(&str, ":");
			if(tok != 0)
			{
				D->datastream[e].clockOffsetDelta[i] = atof(tok);
				tok = strsep(&str, ":");
				if(tok != 0)
				{
					D->datastream[e].phaseOffset[i] = atof(tok);
				}
			}
			free(clockOffStr);
			r = DifxParametersfind1(ip, r+1, "FREQ OFFSET %d (Hz)", i);
			D->datastream[e].freqOffset[i] = atof(DifxParametersvalue(ip, r));
			r = DifxParametersfind1(ip, r+1, "NUM REC POLS %d", i);
			D->datastream[e].nRecPol[i] = atoi(DifxParametersvalue(ip, r));
			nRecBand += D->datastream[e].nRecPol[i];
		}

		/* count rec chans to make sure we have enough */
		nr = 0;
		for(i = 0; ; ++i)
		{
			int v;

			v = DifxParametersfind1(ip, r, "REC BAND %d POL", i);
			if(v <= 0 || v > r+2*i+2)
			{
				break;
			}
			++nr;
		}

		if(nr > nRecBand)
		{
			nRecBand = nr;
		}

		DifxDatastreamAllocBands(D->datastream + e, nRecBand);

		for(i = 0; i < nRecBand; ++i)
		{
			int a;

			r = DifxParametersfind1(ip, r+1, "REC BAND %d POL", i);
			if(r < 0)
			{
				fprintf(stderr, "Warning: parseDifxInputDatastreamTable: REC BAND %d POL not found\n", i);
				continue;
			}
			D->datastream[e].recBandPolName[i] = DifxParametersvalue(ip, r)[0];
			r = DifxParametersfind1(ip, r+1, "REC BAND %d INDEX", i);
			if(r < 0)
			{
				fprintf(stderr, "Error: parseDifxInputDatastreamTable: REC BAND %d INDEX not found\n", i);

				return 0;
			}
			a = atoi(DifxParametersvalue(ip, r));
			D->datastream[e].recBandFreqId[i] = a;
		}
		//Now do the zoom freqs/bands
		r = DifxParametersfind(ip, r+1, "NUM ZOOM FREQS");
		if(r < 0)
		{
			fprintf(stderr, "NUM ZOOM FREQS not found\n");
			
			return 0;
		}
		DifxDatastreamAllocZoomFreqs(D->datastream + e, atoi(DifxParametersvalue(ip, r)));
		nZoomBand = 0;
		for(i = 0; i < D->datastream[e].nZoomFreq; ++i)
		{
			r = DifxParametersfind1(ip, r+1, "ZOOM FREQ INDEX %d", i);
			D->datastream[e].zoomFreqId[i] = atoi(DifxParametersvalue(ip, r));
			r = DifxParametersfind1(ip, r+1, "NUM ZOOM POLS %d", i);
			D->datastream[e].nZoomPol[i] = atoi(DifxParametersvalue(ip, r));
			nZoomBand += D->datastream[e].nZoomPol[i];
		}
		/* count zoom chans to make sure we have enough */
		nz = 0;
		for(i = 0; ; ++i)
		{
			int v;

			v = DifxParametersfind1(ip, r, "ZOOM BAND %d POL", i);
			if(v <= 0 || v > r+2*i+2)
			{
				break;
			}
			++nz;
		}

		if(nz > nZoomBand)
		{
			nZoomBand = nz;
		}

		DifxDatastreamAllocZoomBands(D->datastream + e, nZoomBand);
		
		for(i = 0; i < nZoomBand; ++i)
		{
			int a;

			r = DifxParametersfind1(ip, r+1, "ZOOM BAND %d POL", i);
			if(r < 0)
			{
				fprintf(stderr, "Warning: parseDifxInputDatastreamTable: ZOOM BAND %d POL not found\n", i);
				continue;
			}
			D->datastream[e].zoomBandPolName[i] = DifxParametersvalue(ip, r)[0];
			r = DifxParametersfind1(ip, r+1, "ZOOM BAND %d INDEX", i);
			if(r < 0)
			{
				fprintf(stderr, "Error: parseDifxInputDatastreamTable: ZOOM BAND %d INDEX not found\n", i);
				
				return 0;
			}
			a = atoi(DifxParametersvalue(ip, r));
			D->datastream[e].zoomBandFreqId[i] = a;
		}
		//Figure out how many phase cal tones and their location for each band
		DifxDatastreamCalculatePhasecalTones(&(D->datastream[e]), &(D->freq[D->datastream[e].recFreqId[0]]));
	}

	return D;
}

static DifxInput *parseDifxInputBaselineTable(DifxInput *D, const DifxParameters *ip)
{
	int b, r;
	
	if(!D || !ip)
	{
		return 0;
	}

	r = DifxParametersfind(ip, 0, "BASELINE ENTRIES");
	if(r  < 0)
	{
		fprintf(stderr, "Error: no BASELINE ENTRIES table found\n");

		return 0;
	}
	deleteDifxBaselineArray(D->baseline, D->nBaseline);
	D->nBaseline = atoi(DifxParametersvalue(ip, r));
	D->baseline = newDifxBaselineArray(D->nBaseline);

	for(b = 0; b < D->nBaseline; ++b)
	{
		int f;

		r = DifxParametersfind1(ip, r+1, "D/STREAM A INDEX %d", b);
		if(r < 0)
		{
			fprintf(stderr, "D/STREAM A INDEX %d not found\n", b);

			return 0;
		}
		D->baseline[b].dsA = atoi(DifxParametersvalue(ip, r));
		r = DifxParametersfind1(ip, r+1, "D/STREAM B INDEX %d", b);
		if(r < 0)
		{
			fprintf(stderr, "D/STREAM B INDEX %d not found\n", b);
			
			return 0;
		}
		D->baseline[b].dsB = atoi(DifxParametersvalue(ip, r));
		r = DifxParametersfind1(ip, r+1, "NUM FREQS %d", b);
		if(r < 0)
		{
			fprintf(stderr, "NUM FREQS %d not found\n", b);
			
			return 0;
		}
		DifxBaselineAllocFreqs(D->baseline + b, atoi(DifxParametersvalue(ip, r)));
		
		for(f = 0; f < D->baseline[b].nFreq; ++f)
		{
			int p;

			r = DifxParametersfind2(ip, r+1, "POL PRODUCTS %d/%d", b, f);
			if(r < 0)
			{
				fprintf(stderr, "POL PRODUCTS %d/%d not found\n", b, f);
			
				return 0;
			}
			DifxBaselineAllocPolProds(D->baseline + b, f, atoi(DifxParametersvalue(ip, r)));
			for(p = 0; p < D->baseline[b].nPolProd[f]; p++)
			{
				r = DifxParametersfind1(ip, r+1, "D/STREAM A BAND %d", p);
				if(r < 0)
				{
					fprintf(stderr, "D/STREAM A BAND %d not found\n", p);

					return 0;
				}
				D->baseline[b].bandA[f][p] = atoi(DifxParametersvalue(ip, r));
				r = DifxParametersfind1(ip, r+1, "D/STREAM B BAND %d", p);
				if(r < 0)
				{
					fprintf(stderr, "D/STREAM B BAND %d not found\n", p);

					return 0;
				}
				D->baseline[b].bandB[f][p] = atoi(DifxParametersvalue(ip, r));
			}
		}
	}

	return D;
}

static DifxInput *parseDifxInputDataTable(DifxInput *D, 
										  const DifxParameters *ip)
{
	int j, r;

	if(!D || !ip)
	{
		return 0;
	}

	r = 1;
	for(j = 0; j < D->nAntenna; ++j)
	{
		DifxDatastream *ds;
		int N;
		
		ds = D->datastream + j;
		r = DifxParametersfind1(ip, r, "D/STREAM %d FILES", j);
		if(r < 0)
		{
			fprintf(stderr, "D/STREAM %d FILES not found\n", j);

			return 0;
		}
		N = atoi(DifxParametersvalue(ip, r));
		if(N < 0)
		{
			fprintf(stderr, "D/STREAM %d FILES has illegal value [%s]\n", j, DifxParametersvalue(ip, r));

			return 0;
		}
		if(N > 0)
		{
			int i;

			DifxDatastreamAllocFiles(ds, N);
			for(i = 0; i < N; ++i)
			{
				r = DifxParametersfind2(ip, r, "FILE %d/%d", j, i);
				if(r < 0)
				{
					fprintf(stderr, "FILE %d/%d not found\n", j, i);

					return 0;
				}
				ds->file[i] = strdup( DifxParametersvalue(ip, r) );
			}
		}
	}

	return D;
}

static DifxInput *parseDifxInputNetworkTable(DifxInput *D,
											 const DifxParameters *ip)
{
	int i, r;

	if(!D || !ip)
	{
		return 0;
	}

	r = 1;
	for(i = 0; i < D->nDatastream; ++i)
	{
		DifxDatastream *ds;
				
		ds = D->datastream + i;

		r = DifxParametersfind1(ip, r, "PORT NUM %d", i);
		if(r > 0)
		{
			snprintf(ds->networkPort, DIFXIO_ETH_DEV_SIZE, "%s", DifxParametersvalue(ip, r));
		}

		r = DifxParametersfind1(ip, r, "TCP WINDOW (KB) %d", i);
		if(r > 0)
		{
			ds->windowSize = atoi(DifxParametersvalue(ip, r));
		}
	}

	return D;
}

static DifxInput *deriveDifxInputValues(DifxInput *D)
{
	int c;
	
	if(!D)
	{
		return 0;
	}

	if(D->nConfig < 1)
	{
		fprintf(stderr, "deriveDifxInputValues : nConfig < 1\n");

		return 0;
	}

	for(c = 0; c < D->nConfig; ++c)
	{
		int dsId;
		int qb;
		
		/* determine number of bits, or zero if different among
		 * antennas */
		D->config[c].quantBits = -1;
		qb = 0;

		for(dsId = 0; dsId < D->config[c].nDatastream; ++dsId)
		{
			DifxDatastream *ds;
			int e;

			e = D->config[c].datastreamId[dsId];
			if(e < 0)
			{
				continue;
			}
			ds = D->datastream + e;
			if(qb == 0)
			{
				qb = ds->quantBits;
			}
			else if(qb != ds->quantBits)
			{
				qb = -1;
				break;
			}
		}
		if(qb < 0)
		{
			qb = 0;
		}
		D->config[c].quantBits = qb;
	}

	for(c = 0; c < D->nConfig; ++c)
	{
		int v;

		v = generateAipsIFs(D, c);
		if(v < 0)
		{
			fprintf(stderr, "Fatal error processing configId %d\n", c);

			return 0;
		}
	}

	return D;
}

static DifxInput *populateInput(DifxInput *D, const DifxParameters *ip)
{
	if(!D || !ip)
	{
		fprintf(stderr, "populateInput: D = 0 or ip = 0\n");

		return 0;
	}

	/* COMMON SETTINGS */
	D = parseDifxInputCommonTable(D, ip);
	
	/* CONFIGURATIONS */
	D = parseDifxInputConfigurationTable(D, ip);

	/* RULES */
	D = parseDifxInputRuleTable(D, ip);
	
	/* FREQ TABLE */
	D = parseDifxInputFreqTable(D, ip);

	/* TELESCOPE TABLE */
	D = parseDifxInputTelescopeTable(D, ip);
	
	/* DATASTREAM TABLE */
	D = parseDifxInputDatastreamTable(D, ip);

	/* BASELINE TABLE */
	D = parseDifxInputBaselineTable(D, ip);

	/* DATA TABLE */
	D = parseDifxInputDataTable(D, ip);

	/* NETWORK TABLE */
	D = parseDifxInputNetworkTable(D, ip);

	/* THREADS per CORE */
	DifxInputLoadThreads(D);

	return D;
}

static DifxInput *populateCalc(DifxInput *D, DifxParameters *cp)
{
	const char initKeys[][MAX_DIFX_KEY_LEN] = 
		{
			"JOB ID",
			"OBSCODE",
			"NUM TELESCOPES",
			"NUM SOURCES",
			"NUM SCANS",
			"NUM EOPS"
		};
	const int N_INIT_ROWS = sizeof(initKeys)/sizeof(initKeys[0]);
	
	const char antKeys[][MAX_DIFX_KEY_LEN] =
		{
			"TELESCOPE %d NAME",
			"TELESCOPE %d MOUNT",
			"TELESCOPE %d OFFSET (m)",
			"TELESCOPE %d X (m)",
			"TELESCOPE %d Y (m)",
			"TELESCOPE %d Z (m)"
		};
	const int N_ANT_ROWS = sizeof(antKeys)/sizeof(antKeys[0]);

	const char srcKeys[][MAX_DIFX_KEY_LEN] =
		{
			"SOURCE %d NAME",
			"SOURCE %d RA",
			"SOURCE %d DEC",
			"SOURCE %d CALCODE",
			"SOURCE %d QUAL",
		};
	const int N_SRC_ROWS = sizeof(srcKeys)/sizeof(srcKeys[0]);

	const char eopKeys[][MAX_DIFX_KEY_LEN] =
		{
			"EOP %d TIME (mjd)",
			"EOP %d TAI_UTC (sec)",
			"EOP %d UT1_UTC (sec)",
			"EOP %d XPOLE (arcsec)",
			"EOP %d YPOLE (arcsec)"
		};
	const int N_EOP_ROWS = sizeof(eopKeys)/sizeof(eopKeys[0]);
	
	const char spacecraftKeys[][MAX_DIFX_KEY_LEN] =
		{
			"SPACECRAFT %d NAME"
		};
	const int N_SPACECRAFT_ROWS = 
		sizeof(spacecraftKeys)/sizeof(spacecraftKeys[0]);
	
	int rows[20];
	int i, row, N, r, v;
	int nTel;
	int nScan = 0;
	int nFound = 0;
	const int nSmallSkip = 20;
	const int nMedSkip = 100;

	if (!D || !cp)
	{
		return 0;
	}

	N = DifxParametersbatchfind(cp, 0, initKeys, N_INIT_ROWS, rows);
	if(N < N_INIT_ROWS)
	{
		fprintf(stderr, "Error reading initial rows\n");

		return 0;
	}

	deleteDifxSourceArray(D->source, D->nSource);
	deleteDifxScanArray(D->scan, D->nScan);
	deleteDifxEOPArray(D->eop);

	D->job->jobId	 = atoi(DifxParametersvalue(cp, rows[0]));
	snprintf(D->job->obsCode, DIFXIO_OBSCODE_LENGTH, "%s", DifxParametersvalue(cp, rows[1]));
	nTel			 = atoi(DifxParametersvalue(cp, rows[2]));
	D->nSource		 = atoi(DifxParametersvalue(cp, rows[3]));
	D->nScan		 = atoi(DifxParametersvalue(cp, rows[4]));
	D->nEOP			 = atoi(DifxParametersvalue(cp, rows[5]));

	if(D->nAntenna == 0)
	{
		fprintf(stderr, "Error: populateCalc: D->nAntenna == 0\n");

		return 0;
	}
	if(nTel < D->nAntenna)
	{
		fprintf(stderr, "Error: populateCalc: NUM TELESCOPES too small: %d < %d\n", nTel, D->nAntenna);

		return 0;
	}

	D->source = newDifxSourceArray(D->nSource);
	D->scan = newDifxScanArray(D->nScan);
	D->eop = newDifxEOPArray(D->nEOP);

	row = DifxParametersfind_limited(cp, 0, nMedSkip, "DIFX VERSION");
	if(row > 0)
	{
		snprintf(D->job->difxVersion, DIFXIO_VERSION_LENGTH, "%s", DifxParametersvalue(cp, row));
	}

	row = DifxParametersfind_limited(cp, 0, nMedSkip, "DIFX LABEL");
	if(row > 0)
	{
		snprintf(D->job->difxLabel, DIFXIO_VERSION_LENGTH, "%s", DifxParametersvalue(cp, row));
	}

	row = DifxParametersfind_limited(cp, 0, nMedSkip, "SESSION");
	if(row > 0)
	{
		snprintf(D->job->obsSession, DIFXIO_SESSION_LENGTH, "%s", DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind_limited(cp, 0, nMedSkip, "TAPER FUNCTION");
	if(row > 0)
	{
		D->job->taperFunction = stringToTaperFunction(DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind_limited(cp, 0, nMedSkip, "DELAY POLY ORDER");
	if(row > 0)
	{
		D->job->polyOrder = atoi(DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind_limited(cp, 0, nMedSkip, "DELAY POLY INTERVAL");
	if(row > 0)
	{
		D->job->polyInterval = atoi(DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind_limited(cp, 0, nMedSkip, "DELAY MODEL PREC");
	if(row > 0)
	{
		D->job->delayModelPrecision = atof(DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind_limited(cp, 0, nMedSkip, "DELAY SERVER HOST");
	if(row > 0)
	{
		snprintf(D->job->delayServerHost, DIFXIO_HOSTNAME_LENGTH, "%s", DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind_limited(cp, 0, nMedSkip, "DELAY SERVER TYPE");
	if(row > 0)
	{
		D->job->delayServerType = stringToDelayServerType(DifxParametersvalue(cp, row));
	}
	else
	{
		fprintf(stderr, "No delay server type found --- default in job is %d (%s)\n", (int)D->job->delayServerType, delayServerTypeNames[D->job->delayServerType]);
		row=0;
	}
	row = DifxParametersfind_limited(cp, row, nMedSkip, "DELAY VERSION");
	if(row > 0)
	{
		D->job->delayVersion = strtoul(DifxParametersvalue(cp, row), NULL, 0);
	}
	else
	{
		D->job->delayVersion = DIFXIO_DEFAULT_DELAY_SERVER_VERSION;
		row=0;
	}
	row = DifxParametersfind_limited(cp, row, nMedSkip, "DELAY PROGRAM");
	if(row > 0)
	{
		D->job->delayProgram = strtoul(DifxParametersvalue(cp, row), NULL, 0);
	}
	else
	{
		row=0;
	}
	row = DifxParametersfind_limited(cp, row, nMedSkip, "DELAY HANDLER");
	if(row > 0)
	{
		D->job->delayHandler = strtoul(DifxParametersvalue(cp, row), NULL, 0);
	}
	row = DifxParametersfind_limited(cp, 0, nMedSkip, "JOB PERFORM UVW");
	if(row > 0)
	{
		D->job->perform_uvw_deriv = stringToPerformDirectionDerivativeType(DifxParametersvalue(cp, row));
	}
	else
	{
		row=0;
	}
	row = DifxParametersfind_limited(cp, row, nMedSkip, "JOB PERFORM LMN");
	if(row > 0)
	{
		D->job->perform_lmn_deriv = stringToPerformDirectionDerivativeType(DifxParametersvalue(cp, row));
	}
	else
	{
		row=0;
	}
	row = DifxParametersfind_limited(cp, row, nMedSkip, "JOB PERFORM XYZ");
	if(row > 0)
	{
		D->job->perform_xyz_deriv = stringToPerformDirectionDerivativeType(DifxParametersvalue(cp, row));
	}
	else
	{
		row=0;
	}
	row = DifxParametersfind_limited(cp, row, nMedSkip, "JOB DELTA LMN");
	if(row > 0)
	{
		D->job->delta_lmn = atof(DifxParametersvalue(cp, row));
	}
	else
	{
		row=0;
	}
	row = DifxParametersfind_limited(cp, row, nMedSkip, "JOB DELTA XYZ");
	if(row > 0)
	{
		D->job->delta_xyz = atof(DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind_limited(cp, 0, nMedSkip, "JOB ABER CORR");
	if(row > 0)
	{
		D->job->aberCorr = stringToAberCorr(DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind_limited(cp, 0, nMedSkip, "JOB CALC_OWN_RETARDED_POSITION");
	if(row > 0)
	{
		char t = DifxParametersvalue(cp, row)[0];
		if((t == '1') || (t == 'T') || (t == 't'))
		{
			D->job->calculate_own_retarded_position = 1;
		}
	}
	row = DifxParametersfind(cp, 0, "VEX FILE");
	if(row > 0)
	{
		snprintf(D->job->vexFile, DIFXIO_FILENAME_LENGTH, "%s", DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind(cp, 0, "DUTY CYCLE");
	if(row > 0)
	{
		D->job->dutyCycle = atof(DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind(cp, 0, "JOB START TIME");
	if(row > 0)
	{
		D->job->jobStart = atof(DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind(cp, 0, "JOB STOP TIME");
	if(row > 0)
	{
		D->job->jobStop = atof(DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind(cp, 0, "SUBJOB ID");
	if(row > 0)
	{
		D->job->subjobId = atoi(DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind(cp, 0, "SUBARRAY ID");
	if(row > 0)
	{
		D->job->subarrayId = atoi(DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind(cp, 0, "SPECTRAL AVG");
	if(row > 0)
	{
		D->specAvg = atoi(DifxParametersvalue(cp, row));
	}
	else
	{
		D->specAvg = 1;
	}

	rows[N_ANT_ROWS-1] = 0;		/* initialize start */
	for(i = 0; i < nTel; ++i)
	{
		int a;

		N = DifxParametersbatchfind1(cp, rows[N_ANT_ROWS-1], antKeys, i, N_ANT_ROWS, rows);
		if(N < N_ANT_ROWS)
		{
			if(i == 0)
			{
				fprintf(stderr, "Warning: no antenna axis offsets available\n");
				break;
			}
			else
			{
				fprintf(stderr, "Error reading telescope table %d\n", i);

				return 0;
			}
		}
		a = DifxInputGetAntennaId(D, DifxParametersvalue(cp, rows[0]));
		if(a < 0)
		{
			fprintf(stderr, "populateCalc: skipping telescope table entry %d\n", i);
			continue;
		}
		++nFound;
		row = DifxParametersfind1_limited(cp, rows[0], nSmallSkip, "TELESCOPE %d CALCNAME", i);
		if(row >= 0)
		{
			snprintf(D->antenna[a].calcname, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(cp, row));
		}
		else
		{
			snprintf(D->antenna[a].calcname, DIFXIO_NAME_LENGTH, "%s", D->antenna[a].name);
		}
		D->antenna[a].mount = stringToMountType(DifxParametersvalue(cp, rows[1]));
		if(D->antenna[a].mount >= AntennaMountOther)
		{
			fprintf(stderr, "Warning: populateCalc: unknown antenna mount type encountered for telescope table entry %d: %s.  Changing to AZEL\n", i, DifxParametersvalue(cp, rows[1]));
			D->antenna[a].mount = AntennaMountAltAz;
		}
		row = DifxParametersfind1_limited(cp, rows[0], nSmallSkip, "TELESCOPE %d SITETYPE", i);
		if(row >= 0)
		{
			D->antenna[a].sitetype = stringToSiteType(DifxParametersvalue(cp, row));
		}
		else
		{
			D->antenna[a].sitetype = AntennaSiteOther;
		}
		if(D->antenna[a].sitetype >= AntennaSiteOther)
		{
			fprintf(stderr, "Warning: populateCalc: unknown antenna site type encountered for telescope table entry %d: %s.	 Changing to fixed\n", i, DifxParametersvalue(cp, row));
			D->antenna[a].sitetype = AntennaSiteFixed;
		}
		row = DifxParametersfind1_limited(cp, rows[0], nSmallSkip, "TELESCOPE %d SITEFRAME", i);
		if(row >= 0)
		{
			D->antenna[a].site_coord_frame = stringToSourceCoordinateFrameType(DifxParametersvalue(cp, row));
		}
		else
		{
			D->antenna[a].site_coord_frame = SourceCoordinateFrameUnknown;
		}
		if(D->antenna[a].site_coord_frame == SourceCoordinateFrameUnknown)
		{
			fprintf(stderr, "Warning: populateCalc: unknown antenna site frame encountered for telescope table entry %d: %s.	 Changing to SourceCoordinateFrameITRF2008\n", i, DifxParametersvalue(cp, row));
			D->antenna[a].site_coord_frame = SourceCoordinateFrameITRF2008;
		}
		D->antenna[a].offset[0]= atof(DifxParametersvalue(cp, rows[2]));

#warning "FIXME: In the future add other two axes of axis offset"
		D->antenna[a].offset[1]= 0.0;
		D->antenna[a].offset[2]= 0.0;

		D->antenna[a].X		   = atof(DifxParametersvalue(cp, rows[3]));
		D->antenna[a].Y		   = atof(DifxParametersvalue(cp, rows[4]));
		D->antenna[a].Z		   = atof(DifxParametersvalue(cp, rows[5]));
		row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "TELESCOPE %d SHELF", i);
		if(row > 0)
		{
			snprintf(D->antenna[a].shelf, DIFXIO_SHELF_LENGTH, "%s", DifxParametersvalue(cp, row));
		}
		row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "TELESCOPE %d S/CRAFT ID", i);
		if(row > 0)
		{
			D->antenna[a].spacecraftId = atoi(DifxParametersvalue(cp, row));
		}
		else {
			D->antenna[a].spacecraftId = -1;
		}
		row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "TELESCOPE %d S/CRAFT NAME", i);
		if(row > 0)
		{
			snprintf(D->antenna[a].sc_name, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(cp, row));
		}
	}
	
	if(nFound < D->nAntenna)
	{
		fprintf(stderr, "populateCalc: too few antenna matches\n");

		return 0;
	}

	rows[N_SRC_ROWS-1] = 0;			/* initialize start */
	for(i = 0; i < D->nSource; ++i)
	{
		N = DifxParametersbatchfind1(cp, rows[N_SRC_ROWS-1], srcKeys, i, N_SRC_ROWS, rows);
		if(N < N_SRC_ROWS)
		{
			fprintf(stderr, "Error reading source table %d\n", i);

			return 0;
		}
		snprintf(D->source[i].name, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(cp, rows[0]));
		D->source[i].ra = atof(DifxParametersvalue(cp, rows[1]));
		D->source[i].dec = atof(DifxParametersvalue(cp, rows[2]));
		snprintf(D->source[i].calCode, DIFXIO_CALCODE_LENGTH, "%s", DifxParametersvalue(cp, rows[3]));
		D->source[i].qual = atoi(DifxParametersvalue(cp, rows[4]));
		//The fitsSourceId is left unset for now - the only way to set this is by calling updateDifxInput
		row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SOURCE %d FRAME", i);
		if(row > 0)
		{
			D->source[i].coord_frame = stringToSourceCoordinateFrameType(DifxParametersvalue(cp, row));
		}
		row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SOURCE %d PERFORM UVW", i);
		if(row > 0)
		{
			D->source[i].perform_uvw_deriv = stringToPerformDirectionDerivativeType(DifxParametersvalue(cp, row));
			row = DifxParametersfind1_limited(cp, row, nSmallSkip, "SOURCE %d PERFORM LMN", i);
			D->source[i].perform_lmn_deriv = stringToPerformDirectionDerivativeType(DifxParametersvalue(cp, row));
			row = DifxParametersfind1_limited(cp, row, nSmallSkip, "SOURCE %d PERFORM XYZ", i);
			D->source[i].perform_xyz_deriv = stringToPerformDirectionDerivativeType(DifxParametersvalue(cp, row));
			row = DifxParametersfind1_limited(cp, row, nSmallSkip, "SOURCE %d DELTA LMN", i);
			D->source[i].delta_lmn = atof(DifxParametersvalue(cp, row));
			row = DifxParametersfind1_limited(cp, row, nSmallSkip, "SOURCE %d DELTA XYZ", i);
			D->source[i].delta_xyz = atof(DifxParametersvalue(cp, row));
		}
		
		row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SOURCE %d PM RA (ARCSEC/YR)", i);
		if(row > 0)
		{
			D->source[i].pmRA = atof(DifxParametersvalue(cp, row));
			row = DifxParametersfind1(cp, row, "SOURCE %d PM DEC (ARCSEC/YR)", i);
			D->source[i].pmDec = atof(DifxParametersvalue(cp, row));
			row = DifxParametersfind1(cp, row, "SOURCE %d PARALLAX (ARCSEC)", i);
			D->source[i].parallax = atof(DifxParametersvalue(cp, row));
			row = DifxParametersfind1(cp, row, "SOURCE %d PM EPOCH (MJD)", i);
			D->source[i].pmEpoch = atof(DifxParametersvalue(cp, row));
		}
		row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SOURCE %d S/CRAFT ID", i);
		if(row > 0)
		{
			D->source[i].spacecraftId = atoi(DifxParametersvalue(cp, row));
		}
		else {
			D->source[i].spacecraftId = -1;
		}
		row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SOURCE %d S/CRAFT NAME", i);
		if(row > 0)
		{
			snprintf(D->source[i].sc_name, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(cp, row));
		}
		row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SOURCE %d S/CRAFT EPOCH", i);
		if(row < 0)
		{
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SOURCE %d SC_EPOCH", i);
		}
		if(row > 0)
		{
			D->source[i].sc_epoch = atof(DifxParametersvalue(cp, row));
		}
		else {
			D->source[i].sc_epoch = 0.0;
		}
		
	}

	rows[N_EOP_ROWS-1] = 0;		/* initialize start */
	if(D->eop)
	{
		for(i = 0; i < D->nEOP; ++i)
		{
			N = DifxParametersbatchfind1(cp, rows[N_EOP_ROWS-1], eopKeys, i, N_EOP_ROWS, rows);
			if(N < N_EOP_ROWS)
			{
				fprintf(stderr, "Error reading EOP table %d\n", i);

				return 0;
			}
			D->eop[i].mjd	  = atoi(DifxParametersvalue(cp, rows[0]));
			D->eop[i].tai_utc = atoi(DifxParametersvalue(cp, rows[1]));
			D->eop[i].ut1_utc = atof(DifxParametersvalue(cp, rows[2]));
			D->eop[i].xPole	  = atof(DifxParametersvalue(cp, rows[3]));
			D->eop[i].yPole	  = atof(DifxParametersvalue(cp, rows[4]));
		}
	}

	for(i = 0; i < D->nScan; ++i)
	{
		int startSeconds;
		int durSeconds;
		int j;

		row = DifxParametersfind1(cp, 0, "SCAN %d IDENTIFIER", i);
		if(row < 0)
		{
			fprintf(stderr, "SCAN %d START (S) not found\n", i);
				
			return 0;
		}
		snprintf(D->scan[i].identifier, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(cp, row));
		row = DifxParametersfind1(cp, 0, "SCAN %d START (S)", i);
		if(row < 0)
		{
			fprintf(stderr, "SCAN %d START (S) not found\n", i);
				
			return 0;
		}
		startSeconds = atoi(DifxParametersvalue(cp, row));
		row = DifxParametersfind1(cp, row, "SCAN %d DUR (S)", i);
		if(row < 0)
		{
			fprintf(stderr, "SCAN %d DUR (S) not found\n", i);

			return 0;
		}
		durSeconds = atoi(DifxParametersvalue(cp, row));
		D->scan[i].nAntenna = nTel;
		D->scan[i].startSeconds = startSeconds;
		D->scan[i].durSeconds	= durSeconds;
		D->scan[i].mjdStart = D->job->mjdStart + startSeconds/SEC_DAY_DBL;
		D->scan[i].mjdEnd	= D->job->mjdStart + (startSeconds+durSeconds)/SEC_DAY_DBL;
		row = DifxParametersfind1(cp, row, "SCAN %d OBS MODE NAME", i);
		if(row < 0)
		{
			fprintf(stderr, "SCAN %d OBS MODE NAME not found\n", i);

			return 0;
		}
		snprintf(D->scan[i].obsModeName, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(cp, row));
		row = DifxParametersfind1(cp, row, "SCAN %d UVSHIFT INTERVAL (NS)", i);
		if(row < 0)
		{
			fprintf(stderr, "SCAN %d UVSHIFT INTERVAL (NS) not found\n", i);

			return 0;
		}	
		D->scan[i].maxNSBetweenUVShifts = atoi(DifxParametersvalue(cp, row));
		row = DifxParametersfind1(cp, row, "SCAN %d AC AVG INTERVAL (NS)", i);
		if(row < 0)
		{
			fprintf(stderr, "SCAN %d AC AVG INTERVAL (NS) not found\n", i);
			
			return 0;
		}
		D->scan[i].maxNSBetweenACAvg = atoi(DifxParametersvalue(cp, row));
		row = DifxParametersfind1(cp, row, "SCAN %d POINTING SRC", i);
		if(row < 0)
		{
			fprintf(stderr, "SCAN %d POINTING SRC not found\n", i);

			return 0;
		}
		D->scan[i].pointingCentreSrc = atoi(DifxParametersvalue(cp, row));
		row = DifxParametersfind1(cp, row, "SCAN %d NUM PHS CTRS", i);
		if(row < 0)
		{
			fprintf(stderr, "SCAN %d NUM PHS CTRS not found\n", i);

			return 0;
		}
		D->scan[i].nPhaseCentres = atoi(DifxParametersvalue(cp, row));
		if(D->scan[i].nPhaseCentres > MAX_PHS_CENTRES)
		{
			fprintf(stderr, "SCAN %d NUM PHS CTRS (%d) exceeds max (%d)\n", i, D->scan[i].nPhaseCentres, MAX_PHS_CENTRES);

			return 0;
		}
		for(j = 0; j < D->scan[i].nPhaseCentres; j++)
		{
			row = DifxParametersfind2(cp, row, "SCAN %d PHS CTR %d", i, j);
			if(row < 0)
			{
				fprintf(stderr, "SCAN %d PHS CTR %d not found\n", i, j);
						
				return 0;
			}
			D->scan[i].phsCentreSrcs[j] = atoi(DifxParametersvalue(cp, row));
			D->scan[i].orgjobPhsCentreSrcs[j] = D->scan[i].phsCentreSrcs[j];
		}
		D->scan[i].configId = -1;
		for(r = 0; r < D->nRule; r++)
		{
			int applies = 0;
			int src;

			for(src = 0; src < D->scan[i].nPhaseCentres; ++src)
			{
				//printf("Checking if rule %d applies to scan %d, phase centre %d\n", r, i, src);
				//printf("The phase centre src index is %d\n", D->scan[i].phsCentreSrcs[src]);
				//printf("And its name is %s\n", D->source[D->scan[i].phsCentreSrcs[src]].name);
				//printf("Rule sourceName is %s\n", D->rule[r].sourceName);
				if(ruleAppliesToScanSource(&(D->rule[r]), &(D->scan[i]), &(D->source[D->scan[i].phsCentreSrcs[src]])) != 0)
				{
					applies = 1;
				}
			}
			if(ruleAppliesToScanSource(&(D->rule[r]), &(D->scan[i]), &(D->source[D->scan[i].pointingCentreSrc])) != 0)
			{
				applies = 1;
			}
			if(applies > 0)
			{
				int c;

				//printf("Yes, it does apply!\n");
				for(c = 0; c < D->nConfig; ++c)
				{
					if(strcmp(D->rule[r].configName, D->config[c].name) == 0)
					{
						if(D->scan[i].configId < 0 || D->scan[i].configId == c)
						{
							D->scan[i].configId = c;
						}
						else
						{
							fprintf(stderr, "Warning: Rules produce conflicting configs for scan %d!\n", i);
						}
						applies = 0;
					}
				}
				if(applies > 0)
				{
					fprintf(stderr, "Couldn't find the config (%s) that is supposed to match rule %d\n", 
							D->rule[r].configName, r);
				}
			}
		}
		++nScan;
	}
	D->nScan = nScan;	/* Change to actual number of scans found */

	row = DifxParametersfind(cp, 0, "NUM SPACECRAFT");
	if(row >= 0)
	{
		deleteDifxSpacecraftArray(D->spacecraft, D->nSpacecraft);
		D->nSpacecraft = atoi(DifxParametersvalue(cp, row));
		D->spacecraft  = newDifxSpacecraftArray(D->nSpacecraft);
	}

	rows[N_SPACECRAFT_ROWS-1] = 0;
	if(D->spacecraft) 
	{
		int s;

		for(s = 0; s < D->nSpacecraft; ++s)
		{
			char testc;
			const char* str;
			N = DifxParametersbatchfind1(cp, rows[N_SPACECRAFT_ROWS-1], spacecraftKeys, s, N_SPACECRAFT_ROWS, rows);
			if(N < N_SPACECRAFT_ROWS)
			{
				fprintf(stderr, "Spacecraft %d table screwed up\n", s);
				
				return 0;
			}
			snprintf(D->spacecraft[s].name, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(cp, rows[0]));
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d ISANT", s);
			D->spacecraft[s].is_antenna = 0;
			if(row > 0)
			{
				testc = DifxParametersvalue(cp, row)[0];
				if((testc == '1') || (testc == 'T') || (testc == 't'))
				{
					D->spacecraft[s].is_antenna = 1;
				}
			}
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d SC_TIMETYPE", s);
			D->spacecraft[s].spacecraft_time_type = stringToSpacecraftTimeType(DifxParametersvalue(cp, row));
			if(D->spacecraft[s].spacecraft_time_type >= SpacecraftTimeOther)
			{
				fprintf(stderr, "Warning: populateCalc: unknown spacecraft time type type encountered\n"
				        "for spacecraft table entry %d: %s.	 Changing to 'Local'\n",
				        i, DifxParametersvalue(cp, row));
				D->spacecraft[s].spacecraft_time_type = SpacecraftTimeLocal;
			}
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d POSITION_FRAME", s);
			if(row > 0)
			{
				D->spacecraft[s].position_coord_frame = stringToSourceCoordinateFrameType(DifxParametersvalue(cp, row));
			}
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d POINTING_FRAME", s);
			if(row > 0)
			{
				D->spacecraft[s].pointing_coord_frame = stringToSourceCoordinateFrameType(DifxParametersvalue(cp, row));
			}
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d SC_REC_DELAY (s)", s);
			if(row > 0)
			{
				D->spacecraft[s].SC_recording_delay = atof(DifxParametersvalue(cp, row));
			}
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d SC_COMM_REC_DELAY (s)", s);
			if(row > 0)
			{
				D->spacecraft[s].SC_Comm_Rec_to_Elec = atof(DifxParametersvalue(cp, row));
			}
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d SC_REC_COMM_DELAY (s)", s);
			if(row > 0)
			{
				D->spacecraft[s].SC_Elec_to_Comm = atof(DifxParametersvalue(cp, row));
			}
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_EXISTS", s);
			D->spacecraft[s].GS_exists = 1;
			if(row > 0)
			{
				testc = DifxParametersvalue(cp, row)[0];
				if((testc == '1') || (testc == 'T') || (testc == 't'))
				{
					D->spacecraft[s].GS_exists = 1;
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_NAME", s);
					snprintf(D->spacecraft[s].GS_Name, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(cp, row));
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_CALCNAME", s);
					snprintf(D->spacecraft[s].GS_calcName, DIFXIO_NAME_LENGTH, "%s", DifxParametersvalue(cp, row));
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_MJD_SYNC", s);
					D->spacecraft[s].GS_mjd_sync = atoi(DifxParametersvalue(cp, row));
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_DAYFRAC_SYNC", s);
					D->spacecraft[s].GS_dayfraction_sync = atof(DifxParametersvalue(cp, row));
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_CLOCK_FUDGE (s)", s);
					D->spacecraft[s].GS_clock_break_fudge_sec = atof(DifxParametersvalue(cp, row));
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_MOUNT", s);
					D->spacecraft[s].GS_mount = stringToMountType(DifxParametersvalue(cp, row));
					if(D->spacecraft[s].GS_mount >= AntennaMountOther)
					{
						fprintf(stderr, "Warning: populateCalc: unknown spacecraft ground station mount type encountered\n"
						        "for telescope table entry %d: %s.	Changing to AZEL\n",
						        i, DifxParametersvalue(cp, row));
						D->spacecraft[s].GS_mount = AntennaMountAltAz;
					}
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_OFFSET (m)", s);
					D->spacecraft[s].GS_offset[0] = atof(DifxParametersvalue(cp, row));
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_OFFSET1 (m)", s);
					D->spacecraft[s].GS_offset[1] = atof(DifxParametersvalue(cp, row));
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_OFFSET2 (m)", s);
					D->spacecraft[s].GS_offset[2] = atof(DifxParametersvalue(cp, row));
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_X (m)", s);
					D->spacecraft[s].GS_X = atof(DifxParametersvalue(cp, row));
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_Y (m)", s);
					D->spacecraft[s].GS_Y = atof(DifxParametersvalue(cp, row));
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_Z (m)", s);
					D->spacecraft[s].GS_Z = atof(DifxParametersvalue(cp, row));
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_CLOCK REFMJD", s);
					D->spacecraft[s].GS_clockrefmjd = atof(DifxParametersvalue(cp, row));
					row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d GS_CLOCK ORDER", s);
					D->spacecraft[s].GS_clockorder = atoi(DifxParametersvalue(cp, row));
					for(i=0;i<D->spacecraft[s].GS_clockorder+1;i++)
					{
						row = DifxParametersfind2_limited(cp, row+1, nSmallSkip, "SPACECRAFT %d GS_CLOCK COEFF %d", s, i);
						if(row < 0)
						{
							fprintf(stderr, "Spacecraft %d GS_clockorder table, row %d screwed up\n", s, i);
							
							return 0;
						}
						D->spacecraft[s].GS_clockcoeff[i] = atof(DifxParametersvalue(cp, row));
					}
				}
			}
			/* position offset information */
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d SC_POSOFFSET REFMJD", s);
			if(row > 0) {
				D->spacecraft[s].SC_pos_offset_refmjd = atoi(DifxParametersvalue(cp, row));
			}
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d SC_POSOFFSET REFFRAC", s);
			if(row > 0) {
				D->spacecraft[s].SC_pos_offset_reffracDay = atof(DifxParametersvalue(cp, row));
			}
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d SC_POSOFFSET ORDER", s);
			if(row > 0) {
				D->spacecraft[s].SC_pos_offsetorder = atoi(DifxParametersvalue(cp, row));
				for(i = 0; i <= D->spacecraft[s].SC_pos_offsetorder; ++i)
				{
					int n;
					row = DifxParametersfind2_limited(cp, row+1, nSmallSkip, "SPACECRAFT %d SC_POSOFFSET %d", s, i);
					if(row < 0)
					{
						fprintf(stderr, "Spacecraft %d table, SC_pos_offset %d screwed up\n", s, i);

						return 0;
					}
					str = DifxParametersvalue(cp, row);
					n = sscanf(str, "%lf%lf%lf",
							  &(D->spacecraft[s].SC_pos_offset[i].X),
							  &(D->spacecraft[s].SC_pos_offset[i].Y),
							  &(D->spacecraft[s].SC_pos_offset[i].Z)
							   );
					if(n != 3)
					{
						fprintf(stderr, "Spacecraft %d table, SC_pos_offset %d format screwed up\n", s, i);
				
						return 0;
					}
				}
			}
			/* calculate retarded position */
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d CALC_OWN_RETARDATION", s);
			if(row > 0) {
				D->spacecraft[s].calculate_own_retarded_position = atoi(DifxParametersvalue(cp, row));
			}
				
				
			/* start reading ephemeris information */
			row = DifxParametersfind1_limited(cp, rows[0], nMedSkip, "SPACECRAFT %d ROWS", s);
			if(row < 0)
			{
				fprintf(stderr, "SPACECRAFT %d ROWS not found --- no position information for the spacecraft\n", s);
				return 0;
			}
			D->spacecraft[s].nPoint = atoi(DifxParametersvalue(cp, row));
			D->spacecraft[s].pos = (sixVector *)calloc(D->spacecraft[s].nPoint, sizeof(sixVector));
			D->spacecraft[s].TFrameOffset = (spacecraftTimeFrameOffset *)calloc(D->spacecraft[s].nPoint, sizeof(spacecraftTimeFrameOffset));
			D->spacecraft[s].SCAxisVectors = (spacecraftAxisVectors *)calloc(D->spacecraft[s].nPoint, sizeof(spacecraftAxisVectors));
			for(i = 0; i < D->spacecraft[s].nPoint; ++i)
			{
				/* double time; */
				const char *str;
				int n;
				
				row = DifxParametersfind2_limited(cp, row+1, nSmallSkip, "SPACECRAFT %d ROW %d", s, i);
				if(row < 0)
				{
					fprintf(stderr, "Spacecraft %d table, row %d screwed up\n", s, i);

					return 0;
				}
				str = DifxParametersvalue(cp, row);
				n = sscanf(str, "%d%lf%Lf%Lf%Lf%Lf%Lf%Lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
						  &(D->spacecraft[s].pos[i].mjd),
						  &(D->spacecraft[s].pos[i].fracDay),
						  &(D->spacecraft[s].pos[i].X),
						  &(D->spacecraft[s].pos[i].Y),
						  &(D->spacecraft[s].pos[i].Z),
						  &(D->spacecraft[s].pos[i].dX),
						  &(D->spacecraft[s].pos[i].dY),
						  &(D->spacecraft[s].pos[i].dZ),
						  &(D->spacecraft[s].TFrameOffset[i].Delta_t),
						  &(D->spacecraft[s].TFrameOffset[i].dtdtau),
						  &(D->spacecraft[s].SCAxisVectors[i].X[0]),
						  &(D->spacecraft[s].SCAxisVectors[i].X[1]),
						  &(D->spacecraft[s].SCAxisVectors[i].X[2]),
						  &(D->spacecraft[s].SCAxisVectors[i].Y[0]),
						  &(D->spacecraft[s].SCAxisVectors[i].Y[1]),
						  &(D->spacecraft[s].SCAxisVectors[i].Y[2]),
						  &(D->spacecraft[s].SCAxisVectors[i].Z[0]),
						  &(D->spacecraft[s].SCAxisVectors[i].Z[1]),
						  &(D->spacecraft[s].SCAxisVectors[i].Z[2])
						   );
				if(n != 19)
				{
					/* try the old format */
					n = sscanf(str, "%lf%Lf%Lf%Lf%Lf%Lf%Lf",
							  &(D->spacecraft[s].pos[i].fracDay),
							  &(D->spacecraft[s].pos[i].X),
							  &(D->spacecraft[s].pos[i].Y),
							  &(D->spacecraft[s].pos[i].Z),
							  &(D->spacecraft[s].pos[i].dX),
							  &(D->spacecraft[s].pos[i].dY),
							  &(D->spacecraft[s].pos[i].dZ)
							   );
					if(n != 7) {
						fprintf(stderr, "Spacecraft %d table, row %d format screwed up\n", s, i);
				
						return 0;
					}
					D->spacecraft[s].pos[i].mjd = (int)(D->spacecraft[s].pos[i].fracDay);
					D->spacecraft[s].pos[i].fracDay -= D->spacecraft[s].pos[i].mjd;
				}
				/* 2012 Mar 09	James M Anderson
				   The conversion to exact seconds below is commented
				   out, as there is no guarantee that the input
				   spacecraft data are provided at exact second
				   boundaries.	Let the data timestamps stand as they
				   are. */
				/* Force to be exactly on second boundary */
				/* D->spacecraft[s].pos[i].fracDay = ((int)(time*SEC_DAY_DBL + 0.5))/SEC_DAY_DBL; */
			}
		}
	}

	row = DifxParametersfind(cp, 0, "IM FILENAME");
	if(row < 0)
	{
		fprintf(stderr, "File %s has no IM FILENAME specified!\n", D->job->calcFile);

		return 0;
	}
	v = snprintf(D->job->imFile, DIFXIO_FILENAME_LENGTH, "%s", DifxParametersvalue(cp, row));
	if(v >= DIFXIO_FILENAME_LENGTH)
	{
		fprintf(stderr, "File %s IM FILENAME is too long (%d > %d)\n", D->job->calcFile, v, DIFXIO_FILENAME_LENGTH-1);
		
		return 0;
	}

	row = DifxParametersfind(cp, 0, "FLAG FILENAME");
	if(row >= 0)
	{
		v = snprintf(D->job->flagFile, DIFXIO_FILENAME_LENGTH, "%s", DifxParametersvalue(cp, row));
		if(v >= DIFXIO_FILENAME_LENGTH)
		{
			fprintf(stderr, "File %s FLAG FILENAME is too long (%d > %d)\n", D->job->calcFile, v, DIFXIO_FILENAME_LENGTH-1);

			return 0;
		}
	}
	else
	{
		D->job->flagFile[0] = 0;
	}

	return D;
}

static DifxInput *parseCalcServerInfo(DifxInput *D, DifxParameters *cp)
{
	int row;

	if(!D || !cp)
	{
		return 0;
	}


	row = DifxParametersfind(cp, 0, "DELAY SERVER HOST");
	if(row >= 0)
	{
		snprintf(D->job->delayServerHost, DIFXIO_HOSTNAME_LENGTH, "%s", DifxParametersvalue(cp, row));
	}
	row = DifxParametersfind(cp, 0, "DELAY SERVER TYPE");
	if(row >= 0)
	{
		D->job->delayServerType = stringToDelayServerType(DifxParametersvalue(cp, row));
	}
	else
	{
		row=0;
	}
	row = DifxParametersfind(cp, row, "DELAY VERSION");
	if(row >= 0)
	{
		D->job->delayVersion = strtoul(DifxParametersvalue(cp, row), NULL, 0);
	}
	else
	{
		D->job->delayVersion = DIFXIO_DEFAULT_DELAY_SERVER_VERSION;
		row=0;
	}
	row = DifxParametersfind(cp, row, "DELAY PROGRAM");
	if(row >= 0)
	{
		D->job->delayProgram = strtoul(DifxParametersvalue(cp, row), NULL, 0);
	}
	else
	{
		row=0;
	}
	row = DifxParametersfind(cp, row, "DELAY HANDLER");
	if(row >= 0)
	{
		D->job->delayHandler = strtoul(DifxParametersvalue(cp, row), NULL, 0);
	}
	
	return D;
}

static int parsePoly1_limited(DifxParameters *p, int r, int dr, char *key, int i1, int i2, double *array, int n, int zeroMissing)
{
	const char *v;
	int i, l, m;
    /* Note: This is a particular NaN variant the FITS-IDI format/convention 
     * wants, namely 0xFFFFFFFFFFFFFFFF */
    static const union
    {
	    uint64_t u64;
	    double d;
	    float f;
    } fitsnan = {UINT64_C(0xFFFFFFFFFFFFFFFF)};

	if(r < 0)
	{
		return -1;
	}

	r = DifxParametersfind2_limited(p, r, dr, key, i1, i2);
	if(r < 0)
	{
		if(zeroMissing)
		{
			for(i = 0; i < n; ++i)
			{
				array[i] = 0.0;
			}
		}
		else
		{
			for(i = 0; i < n; ++i)
			{
				array[i] = fitsnan.d;
			}
		}
		return -1;
	}

	v = DifxParametersvalue(p, r);
	m = 0;
	for(i = 0; i < n; ++i)
	{
		double d;

		if(sscanf(v+m, "%lf%n", &d, &l) < 1)
		{
			return -1;
		}
		m += l;
		array[i] = d;
	}

	return r;
}

/*
static int parsePoly1(DifxParameters *p, int r, char *key, int i1, int i2, double *array, int n, int zeroMissing)
{
	return parsePoly1_limited(p, r, p->num_rows, key, i1, i2, array, n, zeroMissing);
}
*/

static DifxInput *populateIM(DifxInput *D, DifxParameters *mp)
{
	int r, s, nScan, nTel;
	int order, interval;
	int *antennaMap;
	const int nSmallSkip = 20;
	const int nMedSkip = 100;
    /* Note: This is a particular NaN variant the FITS-IDI format/convention 
     * wants, namely 0xFFFFFFFFFFFFFFFF */
    static const union
    {
	    uint64_t u64;
	    double d;
	    float f;
    } fitsnan = {UINT64_C(0xFFFFFFFFFFFFFFFF)};


	if(!D)
	{
		return 0;
	}

	if(!mp)
	{
		return D;
	}

	antennaMap = deriveAntennaMap(D, mp, &nTel);
	if(antennaMap == 0)
	{
		fprintf(stderr, "Error: populateIM: deriveAntennaMap failed\n");
		
		return 0;
	}

	D = parseCalcServerInfo(D, mp);

	r = DifxParametersfind(mp, 0, "POLYNOMIAL ORDER");
	if(r < 0)
	{
		fprintf(stderr, "Error: populateIM: POLYNOMIAL ORDER not found\n");
		
		return 0;
	}
	order = atoi(DifxParametersvalue(mp, r));
	D->job->polyOrder = order;

	r = DifxParametersfind(mp, 0, "INTERVAL (SECS)");
	if(r < 0)
	{
		fprintf(stderr, "Error: populateIM: INTERVAL (SECS) not found\n");
		free(antennaMap);
		
		return 0;
	}
	interval = atoi(DifxParametersvalue(mp, r));
	D->job->polyInterval = interval;
	
	r = DifxParametersfind_limited(mp, 0, nMedSkip, "ABERRATION CORR");
	if(r < 0)
	{
		fprintf(stderr, "Warning: populateIM: Assuming aberration not corrected\n");
	}
	else
	{
		D->job->aberCorr = stringToAberCorr(DifxParametersvalue(mp, r));
	}
	r = DifxParametersfind_limited(mp, 0, nMedSkip, "CALC_OWN_RETARDED_POSITION");
	if(r >= 0)
	{
		char t = DifxParametersvalue(mp, r)[0];
		if((t == '1') || (t == 'T') || (t == 't'))
		{
			D->job->calculate_own_retarded_position = 1;
		}
	}
	r = DifxParametersfind_limited(mp, 0, nMedSkip, "CALCPARAM ACCELGRV");
	if(r > 0)
	{
		int row_start = r;
		free(D->job->calcParamTable);
		D->job->calcParamTable = (DifxCalcParamTable*)malloc(sizeof(DifxCalcParamTable));
		D->job->calcParamTable->accelgrv = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM E-FLAT");
		D->job->calcParamTable->e_flat = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM EARTHRAD");
		D->job->calcParamTable->earthrad = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM MMSEMS");
		D->job->calcParamTable->mmsems = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM EPHEPOC");
		D->job->calcParamTable->ephepoc = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM GAUSS");
		D->job->calcParamTable->gauss = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM U-GRV-CN");
		D->job->calcParamTable->u_grv_cn = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM GMSUN");
		D->job->calcParamTable->gmsun = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM GMMERCURY");
		D->job->calcParamTable->gmmercury = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM GMVENUS");
		D->job->calcParamTable->gmvenus = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM GMEARTH");
		D->job->calcParamTable->gmearth = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM GMMOON");
		D->job->calcParamTable->gmmoon = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM GMMARS");
		D->job->calcParamTable->gmmars = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM GMJUPITER");
		D->job->calcParamTable->gmjupiter = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM GMSATURN");
		D->job->calcParamTable->gmsaturn = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM GMURANUS");
		D->job->calcParamTable->gmuranus = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM GMNEPTUNE");
		D->job->calcParamTable->gmneptune = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM ETIDELAG");
		D->job->calcParamTable->etidelag = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM LOVE_H");
		D->job->calcParamTable->love_h = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM LOVE_L");
		D->job->calcParamTable->love_l = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM PRE_DATA");
		D->job->calcParamTable->pre_data = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM REL_DATA");
		D->job->calcParamTable->rel_data = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM TIDALUT1");
		D->job->calcParamTable->tidalut1 = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM AU");
		D->job->calcParamTable->au = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM TSECAU");
		D->job->calcParamTable->tsecau = atof(DifxParametersvalue(mp, r));
		r = DifxParametersfind_limited(mp, row_start, nMedSkip, "CALCPARAM VLIGHT");
		D->job->calcParamTable->vlight = atof(DifxParametersvalue(mp, r));
	}

	/* Now get the models! */
	r = DifxParametersfind(mp, 0, "NUM SCANS");
	if(r < 0)
	{
		fprintf(stderr, "Error: populateIM: NUM SCANS not found\n");
		free(antennaMap);
		
		return 0;
	}

	nScan = atoi(DifxParametersvalue(mp, r));
	if(D->nScan != nScan)
	{
		fprintf(stderr, "Error: populateIM: NUM SCANS disagrees\n");
		free(antennaMap);

		return 0;
	}

	for(s = 0; s < nScan; ++s)
	{
		DifxScan *scan;
		int p;
		int r2;

		scan = D->scan + s;

		r = DifxParametersfind1(mp, r, "SCAN %d NUM POLY", s);
		if(r < 0)
		{
			fprintf(stderr, "Error: populateIM: Malformed scan\n");
			free(antennaMap);
			
			return 0;
		}

		deleteDifxPolyModelArray(scan->im, scan->nAntenna, scan->nPhaseCentres + 1);
		scan->im = NULL;
		deleteDifxPolyModelLMNExtensionArray(scan->imLMN, scan->nAntenna, scan->nPhaseCentres + 1);
		scan->imLMN = NULL;
		deleteDifxPolyModelXYZExtensionArray(scan->imXYZ, scan->nAntenna, scan->nPhaseCentres + 1);
		scan->imXYZ = NULL;
		scan->nPoly = atoi(DifxParametersvalue(mp, r));
		scan->im = newDifxPolyModelArray(scan->nAntenna, scan->nPhaseCentres + 1, scan->nPoly);

		r2 = DifxParametersfind1_limited(mp, r, nSmallSkip, "SCAN %d LMN EXT EXISTS", s);
		if(r2 > 0)
		{
			char t;
			t = DifxParametersvalue(mp, r2)[0];
			if((t == '1') || (t == 'T') || (t == 't'))
			{
				/* allocate the LMN extension to delay model */
				scan->imLMN = newDifxPolyModelLMNExtensionArray(scan->nAntenna, scan->nPhaseCentres + 1, scan->nPoly);
				if(!scan->imLMN)
				{
					fprintf(stderr, "Error: populateIM: Could not allocate LMN Extension\n");
					free(antennaMap);
					
					return 0;
				}
			}
		}
		r2 = DifxParametersfind1_limited(mp, r, nSmallSkip, "SCAN %d XYZ EXT EXISTS", s);
		if(r2 > 0)
		{
			char t;
			t = DifxParametersvalue(mp, r2)[0];
			if((t == '1') || (t == 'T') || (t == 't'))
			{
				/* allocate the XYZ extension to delay model */
				scan->imXYZ = newDifxPolyModelXYZExtensionArray(scan->nAntenna, scan->nPhaseCentres + 1, scan->nPoly);
				if(!scan->imXYZ)
				{
					fprintf(stderr, "Error: populateIM: Could not allocate XYZ Extension\n");
					free(antennaMap);
					
					return 0;
				}
			}
		}

		for(p = 0; p < scan->nPoly; ++p)
		{
			int mjd, sec;
			int src;

			r = DifxParametersfind2_limited(mp, r, nSmallSkip, "SCAN %d POLY %d MJD", s, p);
			if(r < 0)
			{
				fprintf(stderr, "Error: populateIM: Could not find SCAN %d POLY %d MJD", s, p);
				free(antennaMap);
				
				return 0;
			}
			mjd = atoi(DifxParametersvalue(mp, r));
			r = DifxParametersfind2(mp, r, "SCAN %d POLY %d SEC", s, p);
			if(r < 0)
			{
				fprintf(stderr, "Error: populateIM: Could not find SCAN %d POLY %d SEC", s, p);
				free(antennaMap);
				
				return 0;
			}
			sec = atoi(DifxParametersvalue(mp, r));

			for(src = 0; src <= scan->nPhaseCentres; ++src)
			{
				int t;

				for(t = 0; t < nTel; ++t)
				{
					int a;

					a = antennaMap[t];
					if(a < 0)
					{
						continue;
					}
					scan->im[a][src][p].mjd = mjd;
					scan->im[a][src][p].sec = sec;
					scan->im[a][src][p].order = order;
					scan->im[a][src][p].validDuration = interval;
					r2 = DifxParametersfind2_limited(mp, r, nSmallSkip, "SRC %d ANT %d DELTA UVW", src, a);
					if(r2 > 0)
					{
						scan->im[a][src][p].delta = atof(DifxParametersvalue(mp, r2));
					}
					r2 = parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d DELAY (us)", src, t, scan->im[a][src][p].delay, order+1, 1);
					if(r2 < 0)
					{
						fprintf(stderr, "Error: populateIM: Could not find SRC %d ANT %d DELAY (us)\n", src, t);
						
						return 0;
					}
					r = r2;
					/* don't require the following 10 parameters, so don't adjust r when reading them */
					parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d DRY (us)", src, t, scan->im[a][src][p].dry, order+1, 1);
					parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d WET (us)", src, t, scan->im[a][src][p].wet, order+1, 1);
					parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d IONO (us at 1 GHz)", src, t, scan->im[a][src][p].iono, order+1, 0);
					parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d AZ", src, t, scan->im[a][src][p].az, order+1, 0);
					parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d EL CORR", src, t, scan->im[a][src][p].elcorr, order+1, 0);
					parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d EL GEOM", src, t, scan->im[a][src][p].elgeom, order+1, 0);
					parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d PAR ANGLE", src, t, scan->im[a][src][p].parangle, order+1, 0);
					parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d SC_GS_DELAY (us)", src, t, scan->im[a][src][p].sc_gs_delay, order+1, 0);
					parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d GS_SC_DELAY (us)", src, t, scan->im[a][src][p].gs_sc_delay, order+1, 0);
					parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d GS_CLOCK_DELAY (us)", src, t, scan->im[a][src][p].gs_clock_delay, order+1, 0);
					parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d MSA (rad)", src, t, scan->im[a][src][p].msa, order+1, 0);
					/* the next three again are required */
					r = parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d U (m)", src, t, scan->im[a][src][p].u, order+1, 0);
					if(r < 0)
					{
						fprintf(stderr, "Error: populateIM: Could not find SRC %d ANT %d U (m)\n", src, t);
						
						return 0;
					}
					r = parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d V (m)", src, t, scan->im[a][src][p].v, order+1, 0);
					if(r < 0)
					{
						fprintf(stderr, "Error: populateIM: Could not find SRC %d ANT %d V (m)\n", src, t);
						
						return 0;
					}
					r = parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d W (m)", src, t, scan->im[a][src][p].w, order+1, 0);
					if(r < 0)
					{
						fprintf(stderr, "Error: populateIM: Could not find SRC %d ANT %d W (m)\n", src, t);
						
						return 0;
					}
					if((scan->imLMN))
					{
						if((scan->imLMN[a][src]))
						{
							r2 = DifxParametersfind2_limited(mp, r, nSmallSkip, "SRC %d ANT %d DELTA LMN", src, a);
							if(r2 > 0)
							{
								scan->imLMN[a][src][p].delta = atof(DifxParametersvalue(mp, r));
							}
							r2 = parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d dDELAYdL", src, t, scan->imLMN[a][src][p].dDelay_dl, order+1, 0);
							if(r2 > 0)
							{
								r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d dDELAYdM", src, t, scan->imLMN[a][src][p].dDelay_dm, order+1, 0);
								if(r2 < 0)
								{
									fprintf(stderr, "Error: populateIM: missing dDELAYdM around line %d for SRC %d ANT %d\n", r, src, t);
									return 0;
								}
								r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d dDELAYdN", src, t, scan->imLMN[a][src][p].dDelay_dn, order+1, 0);
								if(r2 < 0)
								{
									fprintf(stderr, "Error: populateIM: missing dDELAYdN around line %d for SRC %d ANT %d\n", r, src, t);
									return 0;
								}
								r = r2;
								r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d d2DELAYdLdL", src, t, scan->imLMN[a][src][p].d2Delay_dldl, order+1, 0);
								if(r2 > 0)
								{
									r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d d2DELAYdLdM", src, t, scan->imLMN[a][src][p].d2Delay_dldm, order+1, 0);
									if(r2 < 0)
									{
										fprintf(stderr, "Error: populateIM: missing dDELAYdLDM around line %d for SRC %d ANT %d\n", r, src, t);
										return 0;
									}
									r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d d2DELAYdLdN", src, t, scan->imLMN[a][src][p].d2Delay_dldn, order+1, 0);
									if(r2 < 0)
									{
										fprintf(stderr, "Error: populateIM: missing dDELAYdLDN around line %d for SRC %d ANT %d\n", r, src, t);
										return 0;
									}
									r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d d2DELAYdMdM", src, t, scan->imLMN[a][src][p].d2Delay_dmdm, order+1, 0);
									if(r2 < 0)
									{
										fprintf(stderr, "Error: populateIM: missing dDELAYdMDM around line %d for SRC %d ANT %d\n", r, src, t);
										return 0;
									}
									r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d d2DELAYdMdN", src, t, scan->imLMN[a][src][p].d2Delay_dmdn, order+1, 0);
									if(r2 < 0)
									{
										fprintf(stderr, "Error: populateIM: missing dDELAYdMDN around line %d for SRC %d ANT %d\n", r, src, t);
										return 0;
									}
									r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d d2DELAYdNdN", src, t, scan->imLMN[a][src][p].d2Delay_dndn, order+1, 0);
									if(r2 < 0)
									{
										fprintf(stderr, "Error: populateIM: missing dDELAYdNDN around line %d for SRC %d ANT %d\n", r, src, t);
										return 0;
									}
									r = r2;
								}
								else
								{
									int o;
									for(o=0; o < order+1; ++o)
									{
										scan->imLMN[a][src][p].d2Delay_dldl[o] = fitsnan.d;
										scan->imLMN[a][src][p].d2Delay_dldm[o] = fitsnan.d;
										scan->imLMN[a][src][p].d2Delay_dldn[o] = fitsnan.d;
										scan->imLMN[a][src][p].d2Delay_dmdm[o] = fitsnan.d;
										scan->imLMN[a][src][p].d2Delay_dmdn[o] = fitsnan.d;
										scan->imLMN[a][src][p].d2Delay_dndn[o] = fitsnan.d;
									}
								}
							}
							else
							{
								free(scan->imLMN[a][src]);
								scan->imLMN[a][src] = NULL;
							}
						}
					}
					if((scan->imXYZ))
					{
						if((scan->imXYZ[a][src]))
						{
							r2 = DifxParametersfind2_limited(mp, r, nSmallSkip, "SRC %d ANT %d DELTA XYZ", src, a);
							if(r2 > 0)
							{
								scan->imXYZ[a][src][p].delta = atof(DifxParametersvalue(mp, r));
							}
							r2 = parsePoly1_limited(mp, r, nSmallSkip, "SRC %d ANT %d dDELAYdX", src, t, scan->imXYZ[a][src][p].dDelay_dX, order+1, 0);
							if(r2 > 0)
							{
								r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d dDELAYdY", src, t, scan->imXYZ[a][src][p].dDelay_dY, order+1, 0);
								if(r2 < 0)
								{
									fprintf(stderr, "Error: populateIM: missing dDELAYdY around line %d for SRC %d ANT %d\n", r, src, t);
									return 0;
								}
								r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d dDELAYdZ", src, t, scan->imXYZ[a][src][p].dDelay_dZ, order+1, 0);
								if(r2 < 0)
								{
									fprintf(stderr, "Error: populateIM: missing dDELAYdZ around line %d for SRC %d ANT %d\n", r, src, t);
									return 0;
								}
								r = r2;
								r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d d2DELAYdLXdX", src, t, scan->imXYZ[a][src][p].d2Delay_dXdX, order+1, 0);
								if(r2 > 0)
								{
									r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d d2DELAYdXdY", src, t, scan->imXYZ[a][src][p].d2Delay_dXdY, order+1, 0);
									if(r2 < 0)
									{
										fprintf(stderr, "Error: populateIM: missing dDELAYdXDY around line %d for SRC %d ANT %d\n", r, src, t);
										return 0;
									}
									r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d d2DELAYdXdZ", src, t, scan->imXYZ[a][src][p].d2Delay_dXdZ, order+1, 0);
									if(r2 < 0)
									{
										fprintf(stderr, "Error: populateIM: missing dDELAYdXDZ around line %d for SRC %d ANT %d\n", r, src, t);
										return 0;
									}
									r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d d2DELAYdYdY", src, t, scan->imXYZ[a][src][p].d2Delay_dYdY, order+1, 0);
									if(r2 < 0)
									{
										fprintf(stderr, "Error: populateIM: missing dDELAYdYDY around line %d for SRC %d ANT %d\n", r, src, t);
										return 0;
									}
									r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d d2DELAYdYdZ", src, t, scan->imXYZ[a][src][p].d2Delay_dYdZ, order+1, 0);
									if(r2 < 0)
									{
										fprintf(stderr, "Error: populateIM: missing dDELAYdYDZ around line %d for SRC %d ANT %d\n", r, src, t);
										return 0;
									}
									r2 = parsePoly1_limited(mp, r2, nSmallSkip, "SRC %d ANT %d d2DELAYdZdZ", src, t, scan->imXYZ[a][src][p].d2Delay_dZdZ, order+1, 0);
									if(r2 < 0)
									{
										fprintf(stderr, "Error: populateIM: missing dDELAYdZDZ around line %d for SRC %d ANT %d\n", r, src, t);
										return 0;
									}
									r = r2;
								}
								else
								{
									int o;
									for(o=0; o < order+1; ++o)
									{
										scan->imXYZ[a][src][p].d2Delay_dXdX[o] = fitsnan.d;
										scan->imXYZ[a][src][p].d2Delay_dXdY[o] = fitsnan.d;
										scan->imXYZ[a][src][p].d2Delay_dXdZ[o] = fitsnan.d;
										scan->imXYZ[a][src][p].d2Delay_dYdY[o] = fitsnan.d;
										scan->imXYZ[a][src][p].d2Delay_dYdZ[o] = fitsnan.d;
										scan->imXYZ[a][src][p].d2Delay_dZdZ[o] = fitsnan.d;
									}
								}
							}
							else
							{
								free(scan->imXYZ[a][src]);
								scan->imXYZ[a][src] = NULL;
							}
						}
					}
				}
			}
		}
	}

	free(antennaMap);

	return D;
}

static int populateFlags(DifxInput *D)
{
	const int MaxLineLength=1000;
	FILE *in;
	int n=0, p;
	int nUndecoded = 0;
	char line[MaxLineLength+1];
	char *ptr;
	DifxJob *J;
	int nFlag = 0;

	if(!D)
	{
		return 0;
	}

	J = D->job;

	if(J->flagFile[0] == 0)
	{
		return 0;
	}

	in = fopen(J->flagFile, "r");
	if(!in)
	{
		return 0;
	}

	ptr = fgets(line, MaxLineLength, in);
	if(ptr == 0)
	{
		fprintf(stderr, "Warning: premature end of file %s\n", J->flagFile);
		fclose(in);

		return 0;
	}
	p = sscanf(line, "%d", &n);
	if(p == 1 && n > 0 && n < MaxFlags)
	{
		int i;

		deleteDifxAntennaFlagArray(J->flag);
		J->nFlag = n;
		J->flag = newDifxAntennaFlagArray(J->nFlag);
		for(i = 0; i < n; ++i)
		{
			double mjd1, mjd2;
			int antennaId;

			ptr = fgets(line, MaxLineLength, in);
			if(ptr == 0)
			{
				fprintf(stderr, "Warning: premature end of file %s\n", J->flagFile);
				J->nFlag = i;
				break;
			}
			line[MaxLineLength] = 0;

			/* Allow read of plain numbers */
			p = sscanf(line, "%lf%lf%d", &mjd1, &mjd2, &antennaId);
			if(p != 3)
			{
				/* or formatted in one particular way */
				p = sscanf(line, "  mjd(%lf,%lf)%d", &mjd1, &mjd2, &antennaId);
			}
			if(p == 3)
			{
				if(antennaId < 0 || antennaId >= D->nAntenna)
				{
					fprintf(stderr, "populateFlags : file=%s line=%d: antennaId=%d\n", J->flagFile, i+2, antennaId);
					nUndecoded++;
				}
				else
				{
					J->flag[nFlag].mjd1	 = mjd1;
					J->flag[nFlag].mjd2	 = mjd2;
					J->flag[nFlag].antennaId = antennaId;
					nFlag++;
				}
			}
			else
			{
				nUndecoded++;
			}
		}
	}
	else if(n > 0)
	{
		fprintf(stderr, "populateFlags : unreasonable number of flags : %d\n", n);
	}

	if(nUndecoded > 0)
	{
		fprintf(stderr, "Warning: %d flags from file %s were not properly parsed\n", nUndecoded, J->flagFile);
		J->nFlag = nFlag;
	}

	fclose(in);

	return n;
}

int isAntennaFlagged(const DifxJob *J, double mjd, int antennaId)
{
	int flagId;

	for(flagId = 0; flagId < J->nFlag; ++flagId)
	{
		if(J->flag[flagId].antennaId == antennaId)
		{
			if(mjd > J->flag[flagId].mjd1 && mjd < J->flag[flagId].mjd2)
			{
				return 1;
			}
		}
	}

	return 0;
}

DifxInput *allocateSourceTable(DifxInput *D, int length)
{
	if(D)
	{
		deleteDifxSourceArray(D->source, D->nSource);
		D->source = newDifxSourceArray(length);
		D->nSource = 0;
	}
	return D;
}

/* FIXME: variable names here are confusing at best */
static DifxInput *deriveFitsSourceIds(DifxInput *D)
{
	int i, n=0;
	int *fs;
	int *fc;

	if(!D)
	{
		return 0;
	}

	if(D->nSource < 1 || D->source == 0)
	{
		fprintf(stderr, "No sources to work with!\n");

		return 0;
	}

	fs = (int *)calloc(D->nSource*D->nConfig, sizeof(int));
	fc = (int *)calloc(D->nSource*D->nConfig, sizeof(int));

	for(i = 0; i < D->nSource; ++i)
	{
		int configId;

		D->source[i].numFitsSourceIds = D->nConfig;
		free(D->source[i].fitsSourceIds);
		D->source[i].fitsSourceIds = (int*)malloc(D->nConfig * sizeof(int));
		for(configId = 0; configId < D->nConfig; ++configId)
		{
			int a, j, l;
			int match = -1;
			
			for(a = 0; a < n; ++a)
			{
				j = fs[a];
				l = fc[a];
				if(isSameDifxSourceBasic(D->source+i,D->source+j) &&
				   D->config[configId].fitsFreqId == D->config[l].fitsFreqId)
				{
					match = a;
					break;
				}
			}
			if(match < 0)
			{
				D->source[i].fitsSourceIds[configId] = n;
				fs[n] = i;
				fc[n] = configId;
				++n;
			}
			else
			{
				D->source[i].fitsSourceIds[configId] = match;
			}
		}
		if(D->nSpacecraft > 0)
		{
			int sc;

			for(sc = 0; sc < D->nSpacecraft; ++sc)
			{
				if(strcmp(D->spacecraft[sc].name, D->source[i].sc_name) == 0)
				{
					D->source[i].spacecraftId = sc;
					break;
				}
			}
			if(sc == D->nSpacecraft)
			{
				for(sc = 0; sc < D->nSpacecraft; ++sc)
				{
					if(strcmp(D->spacecraft[sc].name, D->source[i].name) == 0)
					{
						D->source[i].spacecraftId = sc;
						break;
					}
				}
			}
		}
	}

	free(fs);
	free(fc);
	
	return D;
}
	
static void setOrbitingAntennas(DifxInput *D)
{
	int a;
	if(!D)
	{
		return;
	}
	
	if(D->nSpacecraft > 0 && D->nAntenna > 0)
	{
		for(a = 0; a < D->nAntenna; ++a)
		{
			int sc;

			for(sc = 0; sc < D->nSpacecraft; ++sc)
			{
				if(strcmp(D->spacecraft[sc].name, D->antenna[a].sc_name) == 0)
				{
					D->antenna[a].spacecraftId = sc;
					break;
				}
			}
			if(sc == D->nSpacecraft)
			{
				for(sc = 0; sc < D->nSpacecraft; ++sc)
				{
					if(strcmp(D->spacecraft[sc].name, D->antenna[a].name) == 0)
					{
						D->antenna[a].spacecraftId = sc;
					break;
					}
				}
			}
		}
	}
	for(a = 0; a < D->nAntenna; ++a)
	{
		if((D->antenna[a].sitetype == AntennaSiteEarth_Orbiting)
		  && (D->antenna[a].spacecraftId == -1))
		{
			fprintf(stderr, "Error: antenna %s has sitetype=='%s', but does not have a spacecraft ID\n", D->antenna[a].name, antennaSiteTypeNames[D->antenna[a].sitetype]);
			exit(EXIT_FAILURE);
		}
		else if((D->antenna[a].sitetype != AntennaSiteEarth_Orbiting)
			   && (D->antenna[a].spacecraftId >= 0))
		{
			fprintf(stderr, "Error: antenna %s is given a spacecraft ID, but has sitetype=\"%s\", not sitetype=='%s'\n", D->antenna[a].name, antennaSiteTypeNames[D->antenna[a].sitetype], antennaSiteTypeNames[AntennaSiteEarth_Orbiting]);
			exit(EXIT_FAILURE);
		}
	}

	return;
}

int DifxInputCalculateDoPolar(DifxInput *D, int configId)
{
	int b, f, blId;
	DifxBaseline *bl;
	DifxConfig *dc;

	dc = D->config + configId;

	for(b = 0; b < dc->nBaseline; b++)
	{
		blId = dc->baselineId[b];
		if(blId < 0)
		{
			break;
		}

		bl = D->baseline + blId;
		for(f = 0; f < bl->nFreq; f++)
		{
			int pp;

			for(pp = 0; pp < bl->nPolProd[f]; ++pp)
			{
				char polA, polB;

				polA = getDifxDatastreamBandPol(D->datastream+bl->dsA, bl->bandA[f][pp]);
				polB = getDifxDatastreamBandPol(D->datastream+bl->dsB, bl->bandB[f][pp]);

				if(polA != polB)
				{
					D->config[configId].doPolar = 1;

					return 1;
				}
			}
		}
	}

	D->config[configId].doPolar = 0;

	return 0;
}

static void setGlobalValues(DifxInput *D)
{
	int jobId;
	int configId;
	int hasR = 0;
	int hasL = 0;
	int hasX = 0;
	int hasY = 0;

	if(!D)
	{
		return;
	}
	
	D->nIF = -1;
	D->nPol = 0;
	D->doPolar = 0;
	D->nPolar = -1;
	D->chanBW = -1.0;
	D->quantBits = -1;
	strcpy(D->polPair, "  ");

	D->mjdStart = D->mjdStop = D->job->mjdStart;

	for(jobId = 0; jobId < D->nJob; ++jobId)
	{
		double mjdStop;
		
		if(D->job[jobId].mjdStart < D->mjdStart)
		{
			D->mjdStart = D->job[jobId].mjdStart;
		}
		mjdStop = D->job[jobId].mjdStart + D->job[jobId].duration/SEC_DAY_DBL;
		if(mjdStop > D->mjdStop)
		{
			D->mjdStop = mjdStop;
		}
	}

	for(configId = 0; configId < D->nConfig; ++configId)
	{
		DifxConfig *dc;
		int doPolar;
		int i;
		int nIF, qb;
		
		dc = D->config + configId;

		nIF = dc->nIF;
		qb	= dc->quantBits;
		if(D->nIF < nIF)
		{
			D->nIF = nIF;
		}
		if(D->quantBits < 0)
		{
			D->quantBits = qb;
		}
		else if(D->quantBits != qb)
		{
			D->quantBits = 0;
		}
		doPolar = DifxInputCalculateDoPolar(D, configId);
		if(D->doPolar < doPolar)
		{
			D->doPolar = doPolar;
		}
		for(i = 0; i < nIF; ++i)
		{
			double bw;
			int p, nPol;
			char pol[2];

			nPol   = dc->IF[i].nPol;
			bw	   = dc->IF[i].bw;
			pol[0] = dc->IF[i].pol[0];
			pol[1] = dc->IF[i].pol[1];
			if(doPolar)
			{
				nPol *= 2;
			}
			if(D->nPolar < nPol)
			{
				D->nPolar = nPol;
			}
			if(D->chanBW < 0.0)
			{
				D->chanBW = bw;
			}
			else if(D->chanBW != bw)
			{
				D->chanBW = 0.0;

				return; 
			}
			if(nPol > 0)
			{
				int n;

				n = nPol > 1 ? 2 : 1;
				for(p = 0; p < n; ++p)
				{
					switch(pol[p])
					{
					case 'R':
						hasR = 1;
						break;
					case 'L':
						hasL = 1;
						break;
					case 'X':
						hasX = 1;
						break;
					case 'Y':
						hasY = 1;
						break;
					}
				}
			}
		}
	}
	if(D->nPolar == 4)
	{
		D->nPol = 2;
		D->doPolar = 1;
	}
	else
	{
		D->nPol = D->nPolar;
		D->doPolar = 0;
	}
	if(hasR)
	{
		D->polPair[0] = 'R';
		if(hasL)
		{
			D->polPair[1] = 'L';
		}
	}
	else if(hasL)
	{
		D->polPair[0] = 'L';
	}
	else if(hasX)
	{
		D->polPair[0] = 'X';
		if(hasY)
		{
			D->polPair[1] = 'Y';
		}
	}
	else if(hasY)
	{
		D->polPair[0] = 'Y';
	}
}

static int sameFQ(const DifxConfig *C1, const DifxConfig *C2)
{
	int i;
	
	if(C1->nIF != C2->nIF)
	{
		return 0;
	}

	for(i = 0; i < C1->nIF; ++i)
	{
		int p;

		if(C1->IF[i].freq != C2->IF[i].freq)
		{
			return 0;
		}
		if(C1->IF[i].bw != C2->IF[i].bw)
		{
			return 0;
		}
		if(C1->IF[i].sideband != C2->IF[i].sideband)
		{
			return 0;
		}
		if(C1->IF[i].nPol != C2->IF[i].nPol)
		{
			return 0;
		}
		for(p = 0; p < C1->IF[i].nPol; ++p)
		{
			if(C1->IF[i].pol[p] != C2->IF[i].pol[p])
			{
				return 0;
			}
		}
	}

	return 1;
}

static int calcFreqIds(DifxInput *D)
{
	int configId;
	int nFQ = 0;

	if(!D)
	{
		return 0;
	}

	if(D->nConfig < 1)
	{
		return 0;
	}
	
	D->config[0].fitsFreqId = nFQ;
	++nFQ;
	
	if(D->nConfig < 2)
	{
		return 1;
	}

	for(configId = 1; configId < D->nConfig; ++configId)
	{
		int configId2;

		D->config[configId].fitsFreqId = -1;
		for(configId2 = 0; configId2 < configId; ++configId2)
		{
			if(sameFQ(&(D->config[configId]), &(D->config[configId2])))
			{
				D->config[configId].fitsFreqId = D->config[configId2].fitsFreqId;
				configId2 = configId; /* terminate inner loop */
			}
		}
		if(D->config[configId].fitsFreqId == -1)
		{
			D->config[configId].fitsFreqId = nFQ;
			++nFQ;
		}
	}

	return nFQ;
}

DifxInput *updateDifxInput(DifxInput *D)
{
	D = deriveDifxInputValues(D);
	calcFreqIds(D);
	D = deriveFitsSourceIds(D);
	setGlobalValues(D);
	setOrbitingAntennas(D);
	
	return D;
}

DifxInput *loadDifxInput(const char *filePrefix)
{
	DifxParameters *ip, *cp, *mp;
	DifxInput *D, *DSave;
	char inputFile[DIFXIO_FILENAME_LENGTH];
	const char *calcFile;
	int c, r, v, l;

	l = strlen(filePrefix);
	if(strcmp(filePrefix + l - 6, ".input") == 0)
	{
		r = snprintf(inputFile, DIFXIO_FILENAME_LENGTH, "%s", filePrefix);
	}
	else
	{
		r = snprintf(inputFile, DIFXIO_FILENAME_LENGTH, "%s.input", filePrefix);
	}
	if(r >= DIFXIO_FILENAME_LENGTH)
	{
		return 0;
	}

	ip = newDifxParametersfromfile(inputFile);
	if(!ip)
	{
		return 0;
	}

	/* get .calc filename and open it. */
	r = DifxParametersfind(ip, 0, "CALC FILENAME");
	if(r < 0)
	{
		return 0;
	}
	calcFile = DifxParametersvalue(ip, r);

	cp = newDifxParametersfromfile(calcFile);
	if(!cp)
	{
		deleteDifxParameters(ip);
		
		return 0;
	}

	D = DSave = newDifxInput();

	/* When creating a DifxInput via this function, there will always
	 * be a single DifxJob
	 */
	D->job = newDifxJobArray(1);
	D->nJob = 1;


	v = snprintf(D->job->inputFile, DIFXIO_FILENAME_LENGTH, "%s", inputFile);
	if(v >= DIFXIO_FILENAME_LENGTH)
	{
		fprintf(stderr, "Developer error: loadDifxInput: inputFile wants %d bytes which is more than %d\n", v, DIFXIO_FILENAME_LENGTH);
		D = 0;
	}

	D = populateInput(D, ip);
	D = populateCalc(D, cp);
	if (D)
	{
		mp = newDifxParametersfromfile(D->job->imFile);
	}
	else
	{
		mp = NULL;
	}

	if(mp)
	{
		D = populateIM(D, mp);
	}

	if(!D)
	{
		deleteDifxInput(DSave);
	}
	
	deleteDifxParameters(ip);
	deleteDifxParameters(cp);
	deleteDifxParameters(mp);

	/* read in flags, if any */
	populateFlags(D);

	for(c = 0; D && (c < D->nConfig); c++)
	{
		DifxConfigMapAntennas(D->config + c, D->datastream);
	}
	
	return D;
}

DifxInput *loadDifxCalc(const char *filePrefix)
{
	DifxParameters *ip, *cp;
	DifxInput *D, *DSave;
	char inputFile[DIFXIO_FILENAME_LENGTH];
	const char *calcFile;
	int configId, r;
	int l;

	l = strlen(filePrefix);
	if(strcmp(filePrefix + l - 6, ".input") == 0)
	{
		r = snprintf(inputFile, DIFXIO_FILENAME_LENGTH, "%s", filePrefix);
	}
	else
	{
		r = snprintf(inputFile, DIFXIO_FILENAME_LENGTH, "%s.input", filePrefix);
	}
	if(r >= DIFXIO_FILENAME_LENGTH)
	{
		fprintf(stderr, "Developer error: loadDifxCalc: inputFile wanted %d bytes, not %d\n", r, DIFXIO_FILENAME_LENGTH);

		return 0;
	}

	ip = newDifxParametersfromfile(inputFile);
	if(!ip)
	{
		fprintf(stderr, "Cannot load .input file: %s\n", inputFile);

		return 0;
	}

	r = DifxParametersfind(ip, 0, "CALC FILENAME");
	if(r < 0)
	{
		fprintf(stderr, "File %s does not contain CALC FILENAME parameter\n", inputFile);

		return 0;
	}

	calcFile = DifxParametersvalue(ip, r);

	cp = newDifxParametersfromfile(calcFile);
	if(!cp)
	{
		fprintf(stderr, "Error: loadDifxCalc: newDifxParametersfromfile returned 0\n");

		deleteDifxParameters(ip);
		
		return 0;
	}

	D = DSave = newDifxInput();

	/* When creating a DifxInput via this function, there will always
	 * be a single DifxJob
	 */
	D->job = newDifxJobArray(1);
	D->nJob = 1;

	D = populateInput(D, ip);
	if(!D)
	{
		deleteDifxInput(DSave);
		
		fprintf(stderr, "Error: Something failed during reading of %s\n", inputFile);

		return 0;
	}

	D = populateCalc(D, cp);
	if(!D)
	{
		deleteDifxInput(DSave);
		
		fprintf(stderr, "Error: Something failed during reading of %s\n", calcFile);

		return 0;
	}

	deleteDifxParameters(ip);
	deleteDifxParameters(cp);

	for(configId = 0; configId < D->nConfig; ++configId)
	{
		DifxConfigMapAntennas(D->config + configId, D->datastream);
	}
	
	return D;
}

/* return < 0 if no suitable source found */
int DifxInputGetScanIdByJobId(const DifxInput *D, double mjd, int jobId)
{
	int scanId;

	if(!D)
	{
		return -1;
	}

	if(mjd <= D->job[jobId].mjdStart) 
	{
		return -2;
	}

	for(scanId = 0; scanId < D->nScan; ++scanId)
	{
		if(mjd < D->scan[scanId].mjdEnd && D->scan[scanId].jobId == jobId)
		{
			return scanId;
		}
	}

	return -3;
}

/* return < 0 if no suitable source found */
int DifxInputGetScanIdByJobIdVis(const DifxInput *D, double mjd, int jobId)
{
	int scanId;

	if(!D)
	{
		return -1;
	}

	if(mjd <= D->job[jobId].mjdStart) 
	{
		return -2;
	}

	for(scanId = 0; scanId < D->nScan; ++scanId)
	{
		double half_tInt = D->config[D->scan[scanId].configId].tInt * 0.5;
		if(mjd < D->scan[scanId].mjdEnd + half_tInt && D->scan[scanId].jobId == jobId)
		{
			return scanId;
		}
	}

	return -3;
}

/* return < 0 if no suitable scan found */
int DifxInputGetScanIdByAntennaId(const DifxInput *D, double mjd, int antennaId)
{
	int scanId;

	if(!D)
	{
		return -1;
	}

	for(scanId = 0; scanId < D->nScan; ++scanId)
	{
		const DifxConfig *config;
		int d, configId;
		int antId = -1;

		if(mjd > D->scan[scanId].mjdEnd	 ||
		   mjd < D->scan[scanId].mjdStart)
		{
			continue;
		}
		configId = D->scan[scanId].configId;
		if(configId < 0 || configId >= D->nConfig)
		{
			continue;
		}
		config = D->config + configId;

		/* here "d" is "datastream # within conf.", not "antenanId" */
		for(d = 0; d < config->nDatastream; ++d)
		{
			int dsId;

			dsId = config->datastreamId[d];
			if(dsId < 0 || dsId >= D->nDatastream)
			{
				continue;
			}
			
			antId = D->datastream[dsId].antennaId;
			
			if(antennaId == antId)
			{
				break;
			}
		}
		if(d == config->nDatastream || antId < 0)
		{
			/* end of loop reached without finding a match */
			continue;
		}

		if(isAntennaFlagged(D->job + D->scan[scanId].jobId, mjd, antennaId))
		{
			continue;
		}
		
		if(D->scan[scanId].im[antId] != 0)
		{
			return scanId;
		}
	}

	return -2;
}

int DifxInputGetPointingSourceIdByJobId(const DifxInput *D, double mjd, int jobId)
{
	int scanId;

	scanId = DifxInputGetScanIdByJobId(D, mjd, jobId);
	if(scanId < 0 || scanId >= D->nScan)
	{
		return -1;
	}
	else
	{
		return D->scan[scanId].pointingCentreSrc;
	}
}

int DifxInputGetPointingSourceIdByAntennaId(const DifxInput *D, double mjd, int antennaId)
{
	int scanId;

	scanId = DifxInputGetScanIdByAntennaId(D, mjd, antennaId);
	if(scanId < 0 || scanId >= D->nScan)
	{
		return -1;
	}
	else
	{
		return D->scan[scanId].pointingCentreSrc;
	}
}

/* return 0-based index of antName, or -1 if not in array */
int DifxInputGetAntennaId(const DifxInput *D, const char *antennaName)
{
	if(D)
	{
		int a;
	
		for(a = 0; a < D->nAntenna; ++a)
		{
			if(strcmp(D->antenna[a].name, antennaName) == 0)
			{
				return a;
			}
		}
	}

	return -1;
}

static int AntennaCompare(const void *a1, const void *a2)
{
	DifxAntenna *A1, *A2;

	A1 = (DifxAntenna *)a1;
	A2 = (DifxAntenna *)a2;

	return strcmp(A1->name, A2->name);
}

/* go through antenna table and reorder.  Then clean up the collateral damage */ 
int DifxInputSortAntennas(DifxInput *D, int verbose)
{
	int i, n;
	int antennaId;
	int dsId;
	int jobId;
	int scanId;
	int *old2new;
	int nChanged = 0;

	if(!D)
	{
		return -1;
	}
	if(D->nAntenna < 2)
	{
		return -1;
	}
	
	old2new = newRemap(D->nAntenna);
	
	/* sort antenna table and derive reorder table */
	for(antennaId = 0; antennaId < D->nAntenna; ++antennaId)
	{
		D->antenna[antennaId].origId = antennaId;
	}

	if(verbose > 0)
	{
		printf("Pre-sort :");
		for(antennaId = 0; antennaId < D->nAntenna; ++antennaId)
		{
			printf(" %s", D->antenna[antennaId].name);
		}
		printf("\n");
	}
	qsort(D->antenna, D->nAntenna, sizeof(DifxAntenna), AntennaCompare);
	if(verbose > 0)
	{
		printf("Post-sort:");
		for(antennaId = 0; antennaId < D->nAntenna; ++antennaId)
		{
			printf(" %s", D->antenna[antennaId].name);
		}
		printf("\n");
	}

	/* look for no-sort condition and leave successfully if no sort done */
	for(antennaId = 0; antennaId < D->nAntenna; antennaId++)
	{
		old2new[D->antenna[antennaId].origId] = antennaId;
		if(D->antenna[antennaId].origId != antennaId)
		{
			++nChanged;
		}
	}
	if(nChanged == 0)
	{
		deleteRemap(old2new);

		return 0;
	}

	/* OK -- antennas have been reordered.	Fix the tables. */

	/* 1. Datastream table */
	for(dsId = 0; dsId < D->nDatastream; ++dsId)
	{
		D->datastream[dsId].antennaId = old2new[D->datastream[dsId].antennaId];
	}

	/* 2. Flags & antenna id remaps */
	for(jobId = 0; jobId < D->nJob; ++jobId)
	{
		DifxJob *job;
		int flagId;
		
		job = D->job + jobId;

		for(flagId = 0; flagId < job->nFlag; ++flagId)
		{
			job->flag[flagId].antennaId = old2new[job->flag[flagId].antennaId];
		}

		if(job->antennaIdRemap)
		{
			n = sizeofRemap(job->antennaIdRemap);
			for(i = 0; i < n; ++i)
			{
				job->antennaIdRemap[i] = old2new[job->antennaIdRemap[i]];
			}
		}
		else
		{
			job->antennaIdRemap = dupRemap(old2new);
		}
	}

	/* 3. The model tables for each scan */
	for(scanId = 0; scanId < D->nScan; scanId++)
	{
		/* correct the polynomial model table */
		if(D->scan[scanId].im)
		{
			DifxPolyModel ***p2;
			DifxPolyModelLMNExtension*** p2LMN = NULL;
			DifxPolyModelXYZExtension*** p2XYZ = NULL;
			
			p2 = (DifxPolyModel ***)calloc(D->nAntenna*(D->scan[scanId].nPhaseCentres+1), sizeof(DifxPolyModel **));
			if(D->scan[scanId].imLMN)
			{
				p2LMN = (DifxPolyModelLMNExtension ***)calloc(D->nAntenna*(D->scan[scanId].nPhaseCentres+1), sizeof(DifxPolyModelLMNExtension **));
			}
			if(D->scan[scanId].imXYZ)
			{
				p2XYZ = (DifxPolyModelXYZExtension ***)calloc(D->nAntenna*(D->scan[scanId].nPhaseCentres+1), sizeof(DifxPolyModelXYZExtension **));
			}
			for(antennaId = 0; antennaId < D->scan[scanId].nAntenna; antennaId++)
			{
				if(D->scan[scanId].im[antennaId])
				{
					int antennaId2;

					antennaId2 = old2new[antennaId];
					if(antennaId2 < 0 || antennaId2 >= D->nAntenna)
					{
						fprintf(stderr, "Developer error: DifxInputSortAntennas: old2new[%d] = %d; nAnt = %d\n", antennaId, antennaId2, D->scan[scanId].nAntenna);

						continue;
					}
								
					p2[antennaId2] = D->scan[scanId].im[antennaId];
					if(p2LMN)
					{
						if(D->scan[scanId].imLMN[antennaId])
						{
							p2LMN[antennaId2] = D->scan[scanId].imLMN[antennaId];
						}
					}
					if(p2XYZ)
					{
						if(D->scan[scanId].imXYZ[antennaId])
						{
							p2XYZ[antennaId2] = D->scan[scanId].imXYZ[antennaId];
						}
					}
				}
			}

			free(D->scan[scanId].im);
			D->scan[scanId].im = p2;
			free(D->scan[scanId].imLMN);
			D->scan[scanId].imLMN = p2LMN;
			free(D->scan[scanId].imXYZ);
			D->scan[scanId].imXYZ = p2XYZ;
		}
	
		D->scan[scanId].nAntenna = D->nAntenna;
	}

	deleteRemap(old2new);

	/* success */
	return 0;
}

/* note -- this will not work if different integration times are requested within one job */
int DifxInputSimFXCORR(DifxInput *D)
{
	double quantum;
	double tInt, sec, mjdStart;
	double sec_old, deltasec;
	int d, n, mjd;
	int speedUp = 4, su, fanout;
	int nBitstream, sampRate;
	int configId;

	fprintf(stderr, "\n\n*** WARNING TO DEVELOPERS: DifxInputSimFXCORR is depricated ***\n\n");

	if(!D)
	{
		fprintf(stderr, "Error: DifxInputSimFXCORR: D = 0\n");

		return -1;
	}

	if(D->nDatastream < 1 || D->nConfig < 1 || D->nFreq < 1)
	{
		fprintf(stderr, "Error: DifxInputSimFXCORR: incomplete DifxInput structure\n");

		return -2;
	}

	for(d = 0; d < D->nDatastream; ++d)
	{
		const DifxDatastream *dd;

		dd = D->datastream + d;
		if(dd->quantBits < 1)
		{
			fprintf(stderr, "Error: datastream %d quantBits=%d\n", d, dd->quantBits);
			continue;
		}
		if(dd->nRecBand < 1)
		{
			fprintf(stderr, "Error: datastream %d nRecBand=%d\n", d, dd->nRecBand);
			continue;
		}

		nBitstream = dd->dataFrameSize/20000;
		nBitstream *= 8;
		fanout = nBitstream/(dd->quantBits*dd->nRecBand);
		sampRate = (int)(D->freq[dd->recFreqId[0]].bw*2.0+0.00001);
		if(sampRate < 1)
		{
			continue;
		}
		su = 8*fanout/sampRate;
		if(su < speedUp)
		{
			speedUp = su;
		}
	}
	if(speedUp > 4)
	{
		fprintf(stderr, "Warning: reducing speedUp to 4 from %d\n", speedUp);
		speedUp = 4;
	}

	/* the quantum of integration time */
	quantum = 0.131072*speedUp;

	for(configId = 0; configId < D->nConfig; ++configId)
	{
		DifxConfig *dc;
		
		dc = D->config + configId;
		n = (int)(dc->tInt/quantum + 0.5);
		dc->tInt = n * quantum;
	}

	/* here use the first config's tInt to derive the start time of the job */
	tInt = D->config[0].tInt;

	mjd = D->mjdStart;
	sec = sec_old = (D->mjdStart - mjd)*SEC_DAY_DBL;

	n = sec / tInt;
	sec = n*tInt;

	deltasec = sec - sec_old;

	mjdStart = mjd + sec/SEC_DAY_DBL;
	if(mjdStart < D->mjdStart)
	{
		++n;
		mjdStart += tInt/SEC_DAY_DBL;
		deltasec += tInt;
	}

	/* Work around problem that occurs if frac sec >= 0.5 */
	while(SEC_DAY_DBL*mjdStart - (long long)(SEC_DAY_DBL*mjdStart) > 0.49999)
	{
		++n;
		mjdStart += tInt/SEC_DAY_DBL;
		deltasec += tInt;
	}

	/* recompute to avoid bad rounding issues */
	mjdStart = mjd + n*tInt/SEC_DAY_DBL;

	D->mjdStart = mjdStart;
	D->job[0].jobStart = D->mjdStart;
	D->fracSecondStartTime = 1;

	printf("FXCORR Simulator: delayed job start time by %8.6f seconds\n", deltasec);

	/* Now that clocks have their own reference times, this doesn't matter */

	return 0;
}

int DifxInputGetMaxTones(const DifxInput *D)
{
	int d;
	int maxTones = 0;

	for(d = 0; d < D->nDatastream; d++)
	{
		int f;

		if(D->datastream[d].phaseCalIntervalMHz == 0)
		{
			continue;
		}
		for(f = 0; f < D->datastream[d].nRecFreq; f++)
		{
			int fd;

			fd = D->datastream[d].recFreqId[f];
			if (fd < 0)
			{
				break;
			}
			if(D->freq[fd].nTone > maxTones)
			{
				maxTones = D->freq[fd].nTone;
			}
		}
	}

	return maxTones;
}

int DifxInputGetDatastreamId(const DifxInput *D, int jobId, int antId)
{
	int s;

	if(!D)
	{
		return -2;
	}

	if(jobId < 0 || jobId >= D->nJob)
	{
		return -3;
	}

	if(antId < 0 || antId >= D->nAntenna)
	{
		return -4;
	}

	/* This is a bit convoluted.  Loop over scans attempting to connect a configId to the jobId, ... */
	for(s = 0; s < D->nScan; ++s)
	{
		int configId;
		int d;

		if(D->scan[s].jobId != jobId)
		{
			continue;
		}
		configId = D->scan[s].configId;
		if(configId < 0 || configId >= D->nConfig)
		{
			continue;
		}
		for(d = 0; d < D->config[configId].nDatastream; d++)
		{
			int dsId;

			dsId = D->config[configId].datastreamId[d];
			if(dsId < 0 || dsId >= D->nDatastream)
			{
				continue;
			}
			if(D->datastream[dsId].antennaId == antId)
			{
				return dsId;
			}
		}
	}

	return -1;
}

int DifxInputGetFreqIdByBaselineFreq(const DifxInput *D, int baselineId, int baselineFreq)
{
	int band;
	DifxDatastream *ds;
	int freqId;

	if(!D)
	{
		return -1;
	}
	if(baselineId > D->nBaseline || baselineId < 0)
	{
		return -2;
	}
	if(baselineFreq > D->baseline[baselineId].nFreq || baselineFreq < 0)
	{
		return -3;
	}
	if(D->baseline[baselineId].dsA < 0 || D->baseline[baselineId].dsA > D->nDatastream)
	{
		return -4;
	}

	ds = D->datastream + D->baseline[baselineId].dsA;

	band = D->baseline[baselineId].bandA[baselineFreq][0];
	if(band < 0 || band > ds->nRecBand + ds->nZoomBand)
	{
		return -5;
	}

	if(band < ds->nRecBand)	/* a record band */
	{
		int localFreqId;

		localFreqId = ds->recBandFreqId[band];
		freqId = ds->recFreqId[localFreqId];
	}
	else /* a zoom band */
	{
		int zb, localFreqId;
		
		zb = band - ds->nRecBand;
		localFreqId = ds->zoomBandFreqId[zb];
		freqId = ds->zoomFreqId[localFreqId];
	}

	return freqId;
}

int DifxInputGetDatastreamIdsByAntennaId(int *dsIds, const DifxInput *D, int antennaId, int maxCount)
{
	int n = 0;
	int d;

	if(!D)
	{
		return -1;
	}
	if(antennaId >= D->nAntenna)
	{
		return -2;
	}
	if(D->nDatastream <= 0)
	{
		return 0;
	}

	for(d = 0; d < D->nDatastream; ++d)
	{
		if(D->datastream[d].antennaId == antennaId)
		{
			if(dsIds && n < maxCount)
			{
				dsIds[n] = d;
			}
			++n;
		}
	}

	return n;
}

int DifxInputGetOriginalDatastreamIdsByAntennaIdJobId(int *dsIds, const DifxInput *D, int antennaId, int jobId, int maxCount)
{
	int n;
	int m = 0;
	int i;

	if(D->nJob <= jobId)
	{
		return -2;
	}

	n = DifxInputGetDatastreamIdsByAntennaId(dsIds, D, antennaId, maxCount);

	if(n <= 0 || D->job[jobId].datastreamIdRemap == 0)
	{
		return n;
	}

	/* There may be fewer datastreans here */
	for(i = 0; i < n; ++i)
	{
		int j;

		for(j = 0; D->job[jobId].datastreamIdRemap[j] >= 0; ++j)
		{
			if(D->job[jobId].datastreamIdRemap[j] == dsIds[i])
			{
				dsIds[m] = j;
				++m;
				break;
			}
		}
	}

	return m;
}

int DifxInputGetMaxPhaseCentres(const DifxInput *D)
{
	int maxPhaseCentres = 0;

	if(D)
	{
		int s;

		for(s = 0; s < D->nScan; ++s)
		{
			int n;
			
			n = D->scan[s].nPhaseCentres;
			if(n > maxPhaseCentres)
			{
				maxPhaseCentres = n;
			}
		}
	}
	
	return maxPhaseCentres;
}

/* Warning: this function returns the pointing center for the first scan containing 
 * the given sourceId.	Nothing stops multiple distinct pointing centers from
 * containing the same source!
 */
int DifxInputGetPointingCentreSource(const DifxInput *D, int sourceId)
{
	if(D)
	{
		int s;

		for(s = 0; s < D->nScan; ++s)
		{
			const DifxScan *ds;
			int p, n;

			n = D->scan[s].nPhaseCentres;

			ds = D->scan + s;

			for(p = 0; p < n; ++p)
			{
				if(ds->phsCentreSrcs[p] == sourceId)
				{
					return ds->pointingCentreSrc;
				}
			}
		}
	}

	return -1;
}

const DifxSource *DifxInputGetSource(const DifxInput *D, const char *sourceName)
{
	if(D)
	{
		int s;

		for(s = 0; s < D->nSource; ++s)
		{
			if(strcmp(D->source[s].name, sourceName) == 0)
			{
				return D->source + s;
			}
		}

		return 0;
	}
	else
	{
		return 0;
	}
}


char* difx_strlcpy(char *dest, const char *src, size_t n)
{
	char* r = strncpy(dest, src, n);
	if(n>0)
	{
		dest[n-1] = 0;
	}
	return r;
}
