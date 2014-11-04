/***************************************************************************
 *   Copyright (C) 2008, 2009 by Walter Brisken                            *
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
 * $Id: fitsUV.h 1637 2009-11-18 14:55:32Z JohnMorgan $
 * $HeadURL: https://svn.atnf.csiro.au/difx/master_tags/DiFX-1.5.2/applications/difx2fits/src/fitsUV.h $
 * $LastChangedRevision: 1637 $
 * $Author: JohnMorgan $
 * $LastChangedDate: 2009-11-18 15:55:32 +0100 (Wed, 18 Nov 2009) $
 *
 *==========================================================================*/

#ifndef __FITS_UV_H__
#define __FITS_UV_H__

#include <complex.h>
#include <math.h>
#include <glob.h>
#include <sys/types.h>
#include "difxio/parsedifx.h"
#include "difx2fits.h"

struct __attribute__((packed)) UVrow
{
	float U, V, W;
	double jd, iat;
	int32_t baseline, filter;
	int32_t sourceId1, freqId1;	/* 1-based FITS indices */
	float intTime;
	/* FIXME -- no pulsar gate id! */
	float data[0];	/* this takes no room in the "sizeof" operation */
};

/* Information useful for tracking properies of visibility records */
typedef struct
{
	glob_t globbuf;
	int curFile, nFile;
	const DifxInput *D, *OldModel;
	DifxParameters *dp;
	FILE *in;
	double U, V, W;
	double mjd, iat;
	float tInt;
	int baseline;
	int *antennaIdRemap;		/* to convert baseline number */
	int jobId;
	int configId;
	int sourceId;
	int scanId;
	int freqId;			/* DiFX configId or FITS freqId */
	int bandId;			/* FITS IF index, 0-based */
	int polId;			/* FITS polarization index, 0-based */
	int pulsarBin;
	int nPol, nFreq;
	int polStart;			/* start of polarization FITS axis */
	float *spectrum;		/* input visibility spectrum */
	float recweight;
	int nData;
	int nComplex;
	struct UVrow *record;
	float *weight;
	float *data;
	int changed;
	int first;
	double scale;
	int flagTransition;
	double shiftDelay;
} DifxVis;

DifxVis *newDifxVis(const DifxInput *D, const DifxInput *OldModel, int jobId);
void deleteDifxVis(DifxVis *dv);
int DifxVisNextFile(DifxVis *dv);
int DifxVisNewUVData(DifxVis *dv, int verbose, int pulsarBin);
int DifxVisCollectRandomParams(const DifxVis *dv);
int addPhase(DifxVis *dv, float phase);

#endif
