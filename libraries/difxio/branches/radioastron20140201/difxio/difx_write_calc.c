/***************************************************************************
 *	 Copyright (C) 2008-2012, 2014, 2015 by Walter Brisken							   *
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

#include <stdlib.h>
#include <string.h>
#include "difxio/difx_write.h"

int writeDifxCalc(const DifxInput *D)
{
	FILE *out;

	if(!D)
	{
		return -1;
	}

	if(D->nJob != 1)
	{
		fprintf(stderr, "writeDifxCalc: nJob = %d (not 1)\n", D->nJob);

		return -1;
	}

	if(!D->job)
	{
		fprintf(stderr, "writeDifxCalc: job=0\n");

		return -1;
	}

	if(D->job->calcFile[0] == 0)
	{
		fprintf(stderr, "developer error: writeDifxCalc: D->job->calcFile is null\n");

		return -1;
	}

	out = fopen(D->job->calcFile, "w");
	if(!out)
	{
		fprintf(stderr, "Cannot open %s for write\n", D->job->calcFile);

		return -1;
	}

	writeDifxLineInt(out, "JOB ID", D->job->jobId);
	if(D->fracSecondStartTime > 0)
	{
		writeDifxLineDouble(out, "JOB START TIME", "%13.7f", D->job->jobStart);
		writeDifxLineDouble(out, "JOB STOP TIME", "%13.7f", D->job->jobStop);
	}
	else
	{
		writeDifxLineDouble(out, "JOB START TIME", "%13.7f", roundSeconds(D->job->jobStart));
		writeDifxLineDouble(out, "JOB STOP TIME", "%13.7f", roundSeconds(D->job->jobStop));
	}
	if(D->job->dutyCycle > 0.0)
	{
		writeDifxLineDouble(out, "DUTY CYCLE", "%5.3f", D->job->dutyCycle);
	}
	writeDifxLine(out, "OBSCODE", D->job->obsCode);
	writeDifxLine(out, "DIFX VERSION", D->job->difxVersion);
	if(strlen(D->job->difxLabel) > 0)
	{
		writeDifxLine(out, "DIFX LABEL", D->job->difxLabel);
	}
	writeDifxLineInt(out, "SUBJOB ID", D->job->subjobId);
	writeDifxLineInt(out, "SUBARRAY ID", D->job->subarrayId);
	if(D->job->vexFile[0] != 0)
	{
		writeDifxLine(out, "VEX FILE", D->job->vexFile);
	}
	if(D->fracSecondStartTime > 0)
	{
		writeDifxLineDouble(out, "START MJD", "%13.7f", truncSeconds(D->mjdStart));
		writeDifxDateLines(out, truncSeconds(D->mjdStart));
	}
	else
	{
		//round to nearest second - consistent with what is done in write_input
		writeDifxLineDouble(out, "START MJD", "%13.7f", roundSeconds(D->mjdStart));
		writeDifxDateLines(out, roundSeconds(D->mjdStart));
	}
	writeDifxLineInt(out,    "SPECTRAL AVG", D->specAvg);
	writeDifxLine(out,       "TAPER FUNCTION", D->job->taperFunction);
	writeDifxLineInt(out,    "DELAY POLY ORDER", D->job->polyOrder);
	writeDifxLineInt(out,    "DELAY POLY INTERVAL", D->job->polyInterval);
	writeDifxLineDouble(out, "DELAY MODEL PREC", "%10.2e",  D->job->delayModelPrecision);
	if(D->job->delayServerHost != 0)
	{
		writeDifxLine(out,   "DELAY SERVER HOST", D->job->delayServerHost);		
	}
	writeDifxLine(out,       "DELAY SERVER TYPE", delayServerTypeNames[D->job->delayServerType]);
	writeDifxLineULong(out,  "DELAY VERSION", D->job->delayVersion);
	writeDifxLineULong(out,  "DELAY PROGRAM", D->job->delayProgram);
	writeDifxLineULong(out,  "DELAY HANDLER", D->job->delayHandler);
	writeDifxLine(out,       "JOB PERFORM UVW", performDirectionDerivativeTypeNames[D->job->perform_uvw_deriv]);
	writeDifxLine(out,       "JOB PERFORM LMN", performDirectionDerivativeTypeNames[D->job->perform_lmn_deriv]);
	writeDifxLine(out,       "JOB PERFORM XYZ", performDirectionDerivativeTypeNames[D->job->perform_xyz_deriv]);
	writeDifxLineDouble(out, "JOB DELTA LMN", "%24.16e",  D->job->delta_lmn);
	writeDifxLineDouble(out, "JOB DELTA XYZ", "%24.16e",  D->job->delta_xyz);
	writeDifxLine(out,       "JOB ABER CORR",  aberCorrStrings[D->job->aberCorr]);
	writeDifxLineBoolean(out,"JOB CALC_RETARDED_POSITION", D->job->calculate_own_retarded_position);
	if((D->job->calcParamTable))
	{
		writeDifxLineDouble(out,"JOB CALCPARAM ACCELGRV",  "%24.16e", D->job->calcParamTable->accelgrv);
		writeDifxLineDouble(out,"JOB CALCPARAM E-FLAT",    "%24.16e", D->job->calcParamTable->e_flat);
		writeDifxLineDouble(out,"JOB CALCPARAM EARTHRAD",  "%24.16e", D->job->calcParamTable->earthrad);
		writeDifxLineDouble(out,"JOB CALCPARAM MMSEMS",    "%24.16e", D->job->calcParamTable->mmsems);
		writeDifxLineDouble(out,"JOB CALCPARAM EPHEPOC",   "%24.16e", D->job->calcParamTable->ephepoc);
		writeDifxLineDouble(out,"JOB CALCPARAM GAUSS",     "%24.16e", D->job->calcParamTable->gauss);
		writeDifxLineDouble(out,"JOB CALCPARAM U-GRV-CN",  "%24.16e", D->job->calcParamTable->u_grv_cn);
		writeDifxLineDouble(out,"JOB CALCPARAM GMSUN",     "%24.16e", D->job->calcParamTable->gmsun);
		writeDifxLineDouble(out,"JOB CALCPARAM GMMERCURY", "%24.16e", D->job->calcParamTable->gmmercury);
		writeDifxLineDouble(out,"JOB CALCPARAM GMVENUS",   "%24.16e", D->job->calcParamTable->gmvenus);
		writeDifxLineDouble(out,"JOB CALCPARAM GMEARTH",   "%24.16e", D->job->calcParamTable->gmearth);
		writeDifxLineDouble(out,"JOB CALCPARAM GMMOON",    "%24.16e", D->job->calcParamTable->gmmoon);
		writeDifxLineDouble(out,"JOB CALCPARAM GMMARS",    "%24.16e", D->job->calcParamTable->gmmars);
		writeDifxLineDouble(out,"JOB CALCPARAM GMJUPITER", "%24.16e", D->job->calcParamTable->gmjupiter);
		writeDifxLineDouble(out,"JOB CALCPARAM GMSATURN",  "%24.16e", D->job->calcParamTable->gmsaturn);
		writeDifxLineDouble(out,"JOB CALCPARAM GMURANUS",  "%24.16e", D->job->calcParamTable->gmuranus);
		writeDifxLineDouble(out,"JOB CALCPARAM GMNEPTUNE", "%24.16e", D->job->calcParamTable->gmneptune);
		writeDifxLineDouble(out,"JOB CALCPARAM ETIDELAG",  "%24.16e", D->job->calcParamTable->etidelag);
		writeDifxLineDouble(out,"JOB CALCPARAM LOVE_H",    "%24.16e", D->job->calcParamTable->love_h);
		writeDifxLineDouble(out,"JOB CALCPARAM LOVE_L",    "%24.16e", D->job->calcParamTable->love_l);
		writeDifxLineDouble(out,"JOB CALCPARAM PRE_DATA",  "%24.16e", D->job->calcParamTable->pre_data);
		writeDifxLineDouble(out,"JOB CALCPARAM REL_DATA",  "%24.16e", D->job->calcParamTable->rel_data);
		writeDifxLineDouble(out,"JOB CALCPARAM TIDALUT1",  "%24.16e", D->job->calcParamTable->tidalut1);
		writeDifxLineDouble(out,"JOB CALCPARAM AU",        "%24.16e", D->job->calcParamTable->au);
		writeDifxLineDouble(out,"JOB CALCPARAM TSECAU",    "%24.16e", D->job->calcParamTable->tsecau);
		writeDifxLineDouble(out,"JOB CALCPARAM VLIGHT",    "%24.16e", D->job->calcParamTable->vlight);
	}
	writeDifxAntennaArray(out, D->nAntenna, D->antenna, 1, 1, 1, 0, 1, 1);
	writeDifxSourceArray(out, D->nSource, D->source, 1, 1, 1);
	writeDifxScanArray(out, D->nScan, D->scan, D->config);
	writeDifxEOPArray(out, D->nEOP, D->eop);
	writeDifxSpacecraftArray(out, D->nSpacecraft, D->spacecraft);
	writeDifxLine(out,       "IM FILENAME", D->job->imFile);
	writeDifxLine(out,       "FLAG FILENAME", D->job->flagFile);

	fclose(out);

	return 0;
}
