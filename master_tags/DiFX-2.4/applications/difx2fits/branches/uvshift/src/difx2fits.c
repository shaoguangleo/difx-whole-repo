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
 * $Id: difx2fits.c 1447 2009-09-04 15:05:30Z WalterBrisken $
 * $HeadURL: https://svn.atnf.csiro.au/difx/master_tags/DiFX-1.5.2/applications/difx2fits/src/difx2fits.c $
 * $LastChangedRevision: 1447 $
 * $Author: WalterBrisken $
 * $LastChangedDate: 2009-09-04 17:05:30 +0200 (Fri, 04 Sep 2009) $
 *
 *==========================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glob.h>
#include <sys/stat.h>
#include "difx2fits.h"
#include "../config.h"

const char program[] = PACKAGE_NAME;
const char author[]  = PACKAGE_BUGREPORT;
const char version[] = VERSION;


static int usage(const char *pgm)
{
	fprintf(stderr, "\n%s ver. %s   %s\n\n",
		program, version, author);
	fprintf(stderr, "A program to convert DiFX format data to "
		"FITS-IDI\n\n");
	fprintf(stderr, "Usage : %s [options] <baseFilename1> "
		"[<baseFilename2> ... ] [<outfile>]\n\n", pgm);
	fprintf(stderr, "It assumed that SWIN format visibility file(s) "
		"to be converted live\n");
	fprintf(stderr, "in directory <baseFilename>.difx/\n");
	fprintf(stderr, "It is also assumed that at least 3 additional "
		"files exist:\n");
	fprintf(stderr, "  <baseFilename>.input    DiFX input file\n");
	fprintf(stderr, "  <baseFilename>.uvw      DiFX UVW file\n");
	fprintf(stderr, "  <baseFilename>.delay    DiFX delay model\n\n");
	fprintf(stderr, "Four other files are optionally read:\n");
	fprintf(stderr, "  <baseFilename>.calc     Base file for calcif \n");
	fprintf(stderr, "  <baseFilename>.im       Polynomial UVW and model\n");
	fprintf(stderr, "  <baseFilename>.rate     Extra calcif output\n");
	fprintf(stderr, "  <baseFilename>.flag     Antenna-based flagging\n\n");
	fprintf(stderr, "VLBA calibration transfer will produce 4 files:\n");
	fprintf(stderr, "  flag, tsys, pcal, weather\n");
	fprintf(stderr, "If these are present in the current directory, they "
		"will be used to\n");
	fprintf(stderr, "form the FL, TS, PH and WR tables\n\n");
	fprintf(stderr, "If env variable GAIN_CURVE_PATH is set, gain curves "
		"will be looked for\n");
	fprintf(stderr, "and turned into a GN table\n\n");
		
	fprintf(stderr, "The output file <outfile> will be written in "
		"FITS-IDI format nearly\n");
	fprintf(stderr, "identical to that made at the VLBA HW correlator.  "
		"The first two optional\n");
	fprintf(stderr, "files are required for full model accountability.\n");
	fprintf(stderr, "\noptions can include:\n");
	fprintf(stderr, "  --help\n");
	fprintf(stderr, "  -h                  Print this help message\n"); 
	fprintf(stderr, "\n");
	fprintf(stderr, "  --average <nchan>\n");
	fprintf(stderr, "  -a        <nchan>   Average <nchan> channels\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  --bin        <bin>\n");
	fprintf(stderr, "  -B           <bin>  Select on this pulsar bin number\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  --beginchan <chan>\n");
	fprintf(stderr, "  -b          <chan>  Skip <chan> correlated channels\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  --difx\n");
	fprintf(stderr, "   -d                 Run on all .difx files in directory\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  --no-model\n");
	fprintf(stderr, "  -n                  Don't write model (ML) table\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  --dont-combine\n");
	fprintf(stderr, "  -1                  Don't combine jobs\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  --outchans <nchan>\n");
	fprintf(stderr, "  -o         <nchan>  Write <nchan> channels\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  --scale <scale>\n");
	fprintf(stderr, "  -s      <scale>     Scale visibility data "
		"by <scale>\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  --deltat <deltat>\n");
	fprintf(stderr, "  -t       <deltat>   Set interval (sec) in printing job matrix\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  --keep-order\n");
	fprintf(stderr, "  -k                  Keep antenna order\n");
	fprintf(stderr, "\n");
#ifdef HAVE_FFTW
	fprintf(stderr, "  --dont-sniff\n");
	fprintf(stderr, "  -x                  Don't produce sniffer output\n");
	fprintf(stderr, "\n");
#endif
	fprintf(stderr, "  --uv-shift <baseFilename2>\n");
	fprintf(stderr, "  -u <baseFilename2>  Shift all sources to the coordinates"
			" found in the \n" 
			"                      files beginnng with <baseFilename2>\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  --verbose\n");
	fprintf(stderr, "  -v                  Be verbose.  -v -v for more!\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  --override-version  Ignore difx versions\n");
	fprintf(stderr, "\n");

	return 0;
}

struct CommandLineOptions *newCommandLineOptions()
{
	struct CommandLineOptions *opts;

	opts = (struct CommandLineOptions *)calloc(1, 
		sizeof(struct CommandLineOptions));
	
	opts->writemodel = 1;
	opts->sniffTime = 30.0;
	opts->jobMatrixDeltaT = 20.0;

	return opts;
}

void deleteCommandLineOptions(struct CommandLineOptions *opts)
{
	int i;

	if(opts)
	{
		if(opts->nBaseFile > 0)
		{
			for(i = 0; i < opts->nBaseFile; i++)
			{
				free(opts->baseFile[i]);
			}
		}
		if(opts->fitsFile)
		{
			free(opts->fitsFile);
		}
		if(opts->shiftFile)
		{
			free(opts->shiftFile);
		}
		free(opts);
	}
}

struct CommandLineOptions *parseCommandLine(int argc, char **argv)
{
	struct CommandLineOptions *opts;
	int i, l;
	glob_t globbuf;

	opts = newCommandLineOptions();

	for(i = 1; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			if(strcmp(argv[i], "--no-model") == 0 ||
			   strcmp(argv[i], "-n") == 0)
			{
				opts->writemodel = 0;
			}
			else if(strcmp(argv[i], "--quiet") == 0 ||
			        strcmp(argv[i], "-q") == 0)
			{
				opts->verbose--;
			}
			else if(strcmp(argv[i], "--difx") == 0 ||
			        strcmp(argv[i], "-d") == 0)
			{
				opts->doalldifx++;
			}
			else if(strcmp(argv[i], "--verbose") == 0 ||
			        strcmp(argv[i], "-v") == 0)
			{
				opts->verbose++;
			}
			else if(strcmp(argv[i], "--dont-sniff") == 0 ||
				strcmp(argv[i], "-x") == 0)
			{
				opts->sniffTime = -1.0;
			}
			else if(strcmp(argv[i], "--dont-combine") == 0 ||
			        strcmp(argv[i], "-1") == 0)
			{
				opts->dontCombine = 1;
			}
			else if(strcmp(argv[i], "--pretend") == 0 ||
			        strcmp(argv[i], "-p") == 0)
			{
				opts->pretend = 1;
			}
			else if(strcmp(argv[i], "--help") == 0 ||
			        strcmp(argv[i], "-h") == 0)
			{
				usage(program);
				deleteCommandLineOptions(opts);
				return 0;
			}
			else if (strcmp(argv[i], "--keep-order") == 0 ||
				 strcmp(argv[i], "-k") == 0)
			{
				opts->keepOrder = 1;
			}
			else if (strcmp(argv[i], "--override-version") == 0)
			{
				opts->overrideVersion = 1;
			}
			else if(i+1 < argc) /* one parameter arguments */
			{
				if(strcmp(argv[i], "--scale") == 0 ||
				   strcmp(argv[i], "-s") == 0)
				{
					i++;
					opts->scale = atof(argv[i]);
					printf("Scaling data by %f\n", 
						opts->scale);
				}
				else if(strcmp(argv[i], "--deltat") == 0 ||
				        strcmp(argv[i], "-t") == 0)
				{
					i++;
					opts->jobMatrixDeltaT = atof(argv[i]);
				}
				else if(strcmp(argv[i], "--average") == 0 ||
				        strcmp(argv[i], "-a") == 0)
				{
					i++;
					opts->specAvg = atoi(argv[i]);
				}
				else if(strcmp(argv[i], "--bin") == 0 ||
					strcmp(argv[i], "-B") == 0)
				{
					i++;
					opts->pulsarBin = atoi(argv[i]);
				}
				else if(strcmp(argv[i], "--outchans") == 0 ||
				        strcmp(argv[i], "-o") == 0)
				{
					i++;
					opts->nOutChan = atof(argv[i]);
				}
				else if(strcmp(argv[i], "--beginchan") == 0 ||
				        strcmp(argv[i], "-b") == 0)
				{
					i++;
					opts->startChan = atof(argv[i]);
				}
				else if(strcmp(argv[i], "--uvshift") == 0 ||
				        strcmp(argv[i], "-u") == 0)
				{
					i++;
					opts->shiftFile = strdup(argv[i]);
					printf("Shifting coordinates to those found in %s.*\n", 
						opts->shiftFile);
				}

				else
				{
					printf("Unknown param %s\n", argv[i]);
					deleteCommandLineOptions(opts);
					return 0;
				}
			}
			else
			{
				printf("Unknown param %s\n", argv[i]);
				deleteCommandLineOptions(opts);
				return 0;
			}
		}
		else
		{
			if(opts->nBaseFile >= MAX_INPUT_FILES)
			{
				printf("Error -- too many input files!\n");
				printf("Max = %d\n", MAX_INPUT_FILES);
				deleteCommandLineOptions(opts);
				return 0;
			}
			l = strlen(argv[i]);
			if(l > 5 && strcasecmp(argv[i]+l-5, ".FITS") == 0)
			{
				opts->fitsFile = strdup(argv[i]);
			}
			else
			{
				opts->baseFile[opts->nBaseFile] = 
					strdup(argv[i]);
				opts->nBaseFile++;
			}
		}
	}

	if((opts->nBaseFile >  0 && opts->doalldifx >  0) ||
	   (opts->nBaseFile == 0 && opts->doalldifx == 0))
	{
		deleteCommandLineOptions(opts);
		return 0;
	}

	if(opts->doalldifx)
	{
		glob("*.difx", 0, 0, &globbuf);
		if(globbuf.gl_pathc > MAX_INPUT_FILES)
		{
			printf("Error -- too many input files!\n");
			printf("Max = %d\n", MAX_INPUT_FILES);
			deleteCommandLineOptions(opts);
			return 0;
		}
		opts->nBaseFile = globbuf.gl_pathc;
		for(i = 0; i < opts->nBaseFile; i++)
		{
			opts->baseFile[i] = strdup(globbuf.gl_pathv[i]);
		}
		globfree(&globbuf);
	}

	if(opts->nBaseFile > 0 && opts->dontCombine && opts->fitsFile)
	{
		printf("Error -- Cannot supply output filename for multiple output files\n");
		deleteCommandLineOptions(opts);
		return 0;
	}
	
	if(opts->shiftFile && opts->pulsarBin)
	{
		printf("Error -- UV shift not yet compatible with pulsar binning\n");
		deleteCommandLineOptions(opts);
		return 0;
	}

	if(opts->shiftFile && opts->nBaseFile > 1)
	{
		printf("Error -- UV shift not yet compatible with multiple input files\n");
		deleteCommandLineOptions(opts);
		return 0;
	}

	/* if input file ends in .difx, trim it */
	for(i = 0; i < opts->nBaseFile; i++)
	{
		l = strlen(opts->baseFile[i]);
		if(l < 6)
		{
			continue;
		}
		if(strcmp(opts->baseFile[i]+l-5, ".difx") == 0)
		{
			opts->baseFile[i][l-5] = 0;
		}
	}

	return opts;
}

static int populateFitsKeywords(const DifxInput *D, struct fits_keywords *keys)
{
	strcpy(keys->obscode, D->job->obsCode);
	keys->no_stkd = D->nPolar;
	switch(D->polPair[0])
	{
		case 'R':
			keys->stk_1 = -1;
			break;
		case 'L':
			keys->stk_1 = -2;
			break;
		case 'X':
			keys->stk_1 = -5;
			break;
		case 'Y':
			keys->stk_1 = -6;
			break;
		default:
			fprintf(stderr, "Error -- unknown polarization (%c)\n", 
				D->polPair[0]);
			exit(0);
	}
	keys->no_band = D->nIF;
	keys->no_chan = D->nOutChan;
	keys->ref_freq = D->refFreq*1.0e6;
	keys->ref_date = D->mjdStart;
	keys->chan_bw = 1.0e6*D->chanBW*D->specAvg/D->nInChan;
	keys->ref_pixel = 0.5 + 1.0/(2.0*D->specAvg);
	if(D->nPolar > 1)
	{
		keys->no_pol = 2;
	}
	else
	{
		keys->no_pol = 1;
	}
	
	return 0;
}

static const DifxInput *DifxInput2FitsTables(const DifxInput *D, const DifxInput *OldModel,
	struct fitsPrivate *out, struct CommandLineOptions *opts)
{
	struct fits_keywords keys;
	long long last_bytes = 0;

	populateFitsKeywords(D, &keys);
	
	printf("\nWriting FITS tables:\n");

	printf("  Header                    ");
	fflush(stdout);
	D = DifxInput2FitsHeader(D, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  AG -- array geometry      ");
	fflush(stdout);
	D = DifxInput2FitsAG(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  SU -- source              ");
	fflush(stdout);
	D = DifxInput2FitsSU(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  AN -- antenna             ");
	fflush(stdout);
	D = DifxInput2FitsAN(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  FR -- frequency           ");
	fflush(stdout);
	D = DifxInput2FitsFR(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  ML -- model               ");
	fflush(stdout);
	D = DifxInput2FitsML(D, &keys, out, opts);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  CT -- correlator (eop)    ");
	fflush(stdout);
	D = DifxInput2FitsCT(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  MC -- model components    ");
	fflush(stdout);
	D = DifxInput2FitsMC(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  SO -- spacecraft orbit    ");
	fflush(stdout);
	D = DifxInput2FitsSO(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  GD -- pulsar gate duty    ");
	fflush(stdout);
	D = DifxInput2FitsGD(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  GM -- pulsar gate model   ");
	fflush(stdout);
	D = DifxInput2FitsGM(D, &keys, out, opts);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  UV -- visibility          ");
	fflush(stdout);
	D = DifxInput2FitsUV(D, OldModel, &keys, out, opts);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  FL -- flag                ");
	fflush(stdout);
	D = DifxInput2FitsFL(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  TS -- system temperature  ");
	fflush(stdout);
	D = DifxInput2FitsTS(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  PH -- phase cal           ");
	fflush(stdout);
	D = DifxInput2FitsPH(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  WR -- weather             ");
	fflush(stdout);
	D = DifxInput2FitsWR(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("  GN -- gain curve          ");
	fflush(stdout);
	D = DifxInput2FitsGN(D, &keys, out);
	printf("%lld bytes\n", out->bytes_written - last_bytes);
	last_bytes = out->bytes_written;

	printf("                            -----\n");
	printf("  Total                     %lld bytes\n", last_bytes);

	return D;
}

int convertFits(struct CommandLineOptions *opts, int passNum)
{
	DifxInput *D, *D1, *D2, *OldModel, *NewModel;
	struct fitsPrivate outfile;
	char outFitsName[256];
	int i;
	int nConverted = 0;
	const char *difxVersion;

	difxVersion = getenv("DIFX_VERSION");
	if(!difxVersion)
	{
		printf("Warning: env. var. DIFX_VERSION is not set\n");
	}

	D = 0;

	for(i = 0; i < opts->nBaseFile; i++)
	{
		if(opts->baseFile[i] == 0)
		{
			continue;
		}

		if(opts->verbose > 1)
		{
			printf("Loading %s\n", opts->baseFile[i]);
		}
		D2 = loadDifxInput(opts->baseFile[i]);
		if(!D2)
		{
			fprintf(stderr, "loadDifxInput failed on <%s>.\n",
				opts->baseFile[i]);
			return 0;
		}
		if(opts->specAvg)
		{
			D2->specAvg = opts->specAvg;
		}
		if(opts->nOutChan >= 1)
		{
			D2->nOutChan = opts->nOutChan;
		}
		else if(opts->nOutChan > 0.0) /* interpret in fractional sense */
		{
			D2->nOutChan = D2->config[0].nChan*opts->nOutChan/D2->specAvg;
		}
		if(opts->startChan >= 1)
		{
			D2->startChan = opts->startChan;
		}
		else if(opts->startChan > 0.0)
		{
			D2->startChan = (D2->config[0].nChan*opts->startChan) + 0.5;
		}

		if(D)
		{
			D1 = D;

			if(!areDifxInputsMergable(D1, D2) ||
			   !areDifxInputsCompatible(D1, D2))
			{
				deleteDifxInput(D2);
				continue;
			}
			else if(opts->verbose > 1)
			{
				printf("Merging %s\n", opts->baseFile[i]);
			}

			D = mergeDifxInputs(D1, D2, opts->verbose);

			deleteDifxInput(D1);
			deleteDifxInput(D2);

			if(!D)
			{
				fprintf(stderr, "Merging failed on <%s>.\n",
					opts->baseFile[i]);
				return 0;
			}
		}
		else
		{
			D = D2;
		}
		opts->baseFile[i] = 0;
		nConverted++;
		if(opts->dontCombine)
		{
			break;
		}
	}

	if(!D)
	{
		return 0;
	}

	if(opts->shiftFile)
	{
		if(opts->verbose > 1)
		{
			printf("Loading %s\n", opts->shiftFile);
		}
		NewModel = loadDifxInput(opts->shiftFile);
		if(!NewModel)
		{
			fprintf(stderr, "loadDifxInput failed on <%s>.\n",
				opts->shiftFile);
			return 0;
		}
		if (!D->scan->im ||!NewModel->scan->im)
		{
			printf("Error -- Both Models must have an .im file!");
			return 0;
		}
		/* FIXME copied straight from above. Decide if it's necessary
		 * if so put it in a function*/
		if(opts->specAvg)
		{
			NewModel->specAvg = opts->specAvg;
		}
		if(opts->nOutChan >= 1)
		{
			NewModel->nOutChan = opts->nOutChan;
		}
		else if(opts->nOutChan > 0.0) // interpret in fractional sense 
		{
			NewModel->nOutChan = NewModel->config[0].nChan*opts->nOutChan/NewModel->specAvg;
		}
		if(opts->startChan >= 1)
		{
			NewModel->startChan = opts->startChan;
		}
		else if(opts->startChan > 0.0)
		{
			NewModel->startChan = (NewModel->config[0].nChan*opts->startChan) + 0.5;
		}
		OldModel = D;
		D = NewModel;
	}
	else
	{
		OldModel = D;
	}

	if(opts->verbose > 2)
	{
		printDifxInput(D);
		if(D != OldModel)
		{
			//FIXME is there any difference in the output? 
			printf("unshifted input\n");
			printDifxInput(OldModel);
		}
	}

	D = updateDifxInput(D);
	//FIXME is this necessary?
	if(D != OldModel)
		{
		OldModel = updateDifxInput(OldModel);
		}
	if(!D)
	{
		fprintf(stderr, "updateDifxInput failed.  Aborting\n");
		return 0;
	}

	if(difxVersion && D->job->difxVersion[0])
	{
		if(strncmp(difxVersion, D->job->difxVersion, 63))
		{
			fprintf(stderr, "Attempting to run difx2fits from version %s on a job make for version %s\n", difxVersion, D->job->difxVersion);
			if(opts->overrideVersion)
			{
				fprintf(stderr, "Continuing because of --override-version but not setting a version\n");
				D->job->difxVersion[0] = 0;
			}
			else
			{
				fprintf(stderr, "Not converting.\n");
				deleteDifxInput(D);
				deleteDifxInput(OldModel);
				return 0;
			}
		}
	}
	else if(!D->job->difxVersion[0])
	{
		fprintf(stderr, "Warning -- working on unversioned job\n");
	}

	if(OldModel->job->difxVersion[0] != D->job->difxVersion[0])
	{
		if(strncmp(difxVersion, D->job->difxVersion, 63))
		{
			fprintf(stderr, "Job made for version %s with uvshift input made for %s \n", OldModel->job->difxVersion, D->job->difxVersion);
			if(opts->overrideVersion)
			{
				fprintf(stderr, "Continuing because of --override-version but not setting a version\n");
				D->job->difxVersion[0] = 0;
			}
			else
			{
				fprintf(stderr, "Not converting.\n");
				deleteDifxInput(D);
				deleteDifxInput(OldModel);
				return 0;
			}
		}
	}
	else if((D != OldModel) && (!OldModel->job->difxVersion[0]))
	{
		fprintf(stderr, "Warning -- Original input is unversioned\n");
	}

	if(opts->verbose > 1)
	{
		printDifxInput(D);
	}

	if(D->nIF <= 0 || D->nPolar <= 0)
	{
		fprintf(stderr, "Data geometry changes during obs, cannot "
			"make into FITS.\n");
		deleteDifxInput(D);
		return 0;
	}

	if(strcmp(D->job->taperFunction, "UNIFORM") != 0)
	{
		fprintf(stderr, "Taper func %s not supported.  "
			"Using UNIFORM.\n", D->job->taperFunction);
		strcpy(D->job->taperFunction, "UNIFORM");
	}

	if(opts->fitsFile)
	{
		strcpy(outFitsName, opts->fitsFile);
	}
	else
	{
		if(opts->pulsarBin == 0)
		{
			sprintf(outFitsName, "%s%s.%d.FITS",
				D->job[0].obsCode,
				D->job[0].obsSession,
				passNum);
		}
		else
		{
			sprintf(outFitsName, "%s%s.%d.bin%04d.FITS",
				D->job[0].obsCode,
				D->job[0].obsSession,
				passNum,
				opts->pulsarBin);
		}
	}

	if(!opts->pretend)
	{
		if(!opts->keepOrder)
		{
			DifxInputSortAntennas(D, opts->verbose);
		}

		if(opts->verbose > 2)
		{
			printDifxInput(D);
			if(OldModel != D)
			{
				printf("shifted input");
				printDifxInput(OldModel);
			}
		}

		if(fitsWriteOpen(&outfile, outFitsName) < 0)
		{
			deleteDifxInput(D);
			deleteDifxInput(OldModel);
			fprintf(stderr, "Cannot open output file\n");
			return 0;
		}

		if(!OldModel)
		{
			OldModel = D;
		}

		if(DifxInput2FitsTables(D, OldModel, &outfile, opts) == D)		{
			printf("\nConversion successful\n\n");
		}
		
		fitsWriteClose(&outfile);
	}

	if(OldModel != D)
	{
		deleteDifxInput(OldModel);
	}
	deleteDifxInput(D);

	return nConverted;
}

int main(int argc, char **argv)
{
	struct CommandLineOptions *opts;
	int nConverted = 0;
	int n, nFits = 0;

	if(argc < 2)
	{
		return usage(argv[0]);
	}

	if(getenv("DIFX_GROUP_ID"))
	{
		umask(2);
	}

	opts = parseCommandLine(argc, argv);
	if(opts == 0)
	{
		return 0;
	}

	for(;;)
	{
		n = convertFits(opts, nFits);
		if(n <= 0)
		{
			break;
		}
		nConverted += n;
		nFits++;
	}

	printf("%d of %d jobs converted to %d FITS files\n", nConverted,
		opts->nBaseFile, nFits);

	if(nConverted != opts->nBaseFile)
	{
		printf("\n*** Warning -- not all input files converted!\n");
	}

	printf("\n");
	
	deleteCommandLineOptions(opts);

	return 0;
}
