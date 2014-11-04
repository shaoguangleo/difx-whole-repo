/***************************************************************************
 *   Copyright (C) 2009 by Walter Brisken                                  *
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
 * $Id: jobmatrix.c 1567 2009-10-18 21:07:23Z WalterBrisken $
 * $HeadURL: https://svn.atnf.csiro.au/difx/master_tags/DiFX-1.5.2/applications/difx2fits/src/jobmatrix.c $
 * $LastChangedRevision: 1567 $
 * $Author: WalterBrisken $
 * $LastChangedDate: 2009-10-18 23:07:23 +0200 (Sun, 18 Oct 2009) $
 *
 *==========================================================================*/

#include <stdlib.h>
#include "jobmatrix.h"

static const char *stripDir(const char *fn)
{
	const char *out;
	int i;

	out = fn;

	for(i = 0; fn[i] != 0; i++)
	{
		if(fn[i] == '/')
		{
			if(fn[i+1] != 0)
			{
				out = fn+i+1;
			}
		}
	}

	return out;
}

struct _JobMatrix
{
	int **matrix;
	int nAntenna, nTime;
	const DifxInput *D;
	const char *filebase;
	double mjdStart;		/* days */
	double deltaT;			/* seconds */
};

/* deltaT parameter is in seconds */
JobMatrix *newJobMatrix(const DifxInput *D, const char *filebase, 
	double deltaT)
{
	JobMatrix *jm;
	int a, n, t;
	double frac;

	jm = (JobMatrix *)calloc(1, sizeof(JobMatrix));

	jm->deltaT = deltaT;
	frac = D->mjdStart - (int)(D->mjdStart);
	n = frac/(deltaT/86400.0);
	jm->mjdStart = (int)(D->mjdStart) + n*deltaT/86400.0;
	jm->nTime = (D->mjdStop - jm->mjdStart)/(deltaT/86400.0) + 1;
	jm->nAntenna = D->nAntenna;
	jm->matrix = (int **)calloc(jm->nTime, sizeof(int *));
	for(t = 0; t < jm->nTime; t++)
	{
		jm->matrix[t] = (int *)calloc(jm->nAntenna, sizeof(int));
		for(a = 0; a < jm->nAntenna; a++)
		{
			jm->matrix[t][a] = -1;
		}
	}
	jm->filebase = filebase;
	jm->D = D;

	return jm;
}

void writeJobMatrix(JobMatrix *jm)
{
	int *jobList;
	FILE *out;
	int nJob;
	char outname[256];
	int a, t, j;
	char name[4];
	char timeStr[40];
	char lastday[10] = "";

	if(!jm)
	{
		return;
	}

	nJob = jm->D->nJob;
	jobList = (int *)calloc(nJob, sizeof(int));
	
	sprintf(outname, "%s.jobmatrix", jm->filebase);
	out = fopen(outname, "w");

	for(a = 0; a < jm->nAntenna; a++)
	{
		strncpy(name, jm->D->antenna[a].name, 2);
		name[2] = 0;
		fprintf(out, "%s ", name);
	}
	fprintf(out, "\n\n");

	j = 0;
	for(t = 0; t < jm->nTime; t++)
	{
		for(a = 0; a < jm->nAntenna; a++)
		{
			if(jm->matrix[t][a] < 0)
			{
				fprintf(out, "   ");
			}
			else if(jm->matrix[t][a] < jm->D->nJob)
			{
				jobList[jm->matrix[t][a]] = 1;
				fprintf(out, "%c  ", 
					'A' + (jm->matrix[t][a]%26));
			}
			else
			{
				fprintf(out, "?  ");
			}
		}

		timeMjd2str(jm->mjdStart + t*jm->deltaT/86400.0, timeStr);
		timeStr[39] = 0;
		if(strncmp(timeStr, lastday, 9) == 0)
		{
			fprintf(out, "           %s", timeStr+9);
		}
		else
		{
			strncpy(lastday, timeStr, 9);
			lastday[9] = 0;
			fprintf(out, "  %s", timeStr);
		}

		if(j < nJob && jobList[j])
		{
			fprintf(out, "   %c = %s", 'A'+(j%26),
				stripDir(jm->D->job[j].fileBase));
			j++;
		}

		fprintf(out, "\n");
	}

	fclose(out);
	free(jobList);
}

void deleteJobMatrix(JobMatrix *jm)
{
	int t;

	if(jm)
	{
		if(jm->matrix)
		{
			for(t = 0; t < jm->nTime; t++)
			{
				if(jm->matrix[t])
				{
					free(jm->matrix[t]);
				}
			}
			free(jm->matrix);
		}
		free(jm);
	}
}

int feedJobMatrix(JobMatrix *jm, const struct UVrow *data, int jobId)
{
	int a, t;
	double mjd;

	if(!jm)
	{
		return -1;
	}
	a = data->baseline/256 - 1;
	if(a < 0 || a > jm->nAntenna)
	{
		return -1;
	}
	mjd = (int)(data->jd - 2400000.0) + data->iat;
	t = (mjd - jm->mjdStart)*86400.0/jm->deltaT;
	if(t < 0 || t >= jm->nTime)
	{
		return -1;
	}
	jm->matrix[t][a] = jobId;

	return 0;
}
