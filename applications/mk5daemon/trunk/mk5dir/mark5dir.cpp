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
 * $Id$
 * $HeadURL$
 * $LastChangedRevision$
 * $Author$
 * $LastChangedDate$
 *
 *==========================================================================*/

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <mark5access.h>
#include "mark5dir.h"
#include "../config.h"

#ifdef WORDS_BIGENDIAN
#define MARK5_FILL_WORD32 0x44332211UL
#define MARK5_FILL_WORD64 0x4433221144332211ULL
#else
#define MARK5_FILL_WORD32 0x11223344UL
#define MARK5_FILL_WORD64 0x1122334411223344ULL
#endif


using namespace std;

char Mark5DirDescription[][20] =
{
	"Short scan",
	"XLR Read error",
	"Decode error",
	"Decoded",
	"Decoded WR"
};

void countReplaced(const unsigned long *data, int len, 
	long long *wGood, long long *wBad)
{
	int i;
	int nBad=0;

	for(i = 0; i < len; i++)
	{
		if(data[i] == MARK5_FILL_WORD32)
		{
			nBad++;
		}
	}

	*wGood += (len-nBad);
	*wBad += nBad;
}

/* returns active bank, or -1 if none */
int Mark5BankGet(SSHANDLE *xlrDevice)
{
	S_BANKSTATUS bank_stat;
	XLR_RETURN_CODE xlrRC;
	int b = -1;

	xlrRC = XLRGetBankStatus(*xlrDevice, BANK_A, &bank_stat);
	if(xlrRC == XLR_SUCCESS)
	{
		if(bank_stat.Selected)
		{
			b = 0;
		}
	}
	if(b == -1)
	{
		xlrRC = XLRGetBankStatus(*xlrDevice, BANK_B, &bank_stat);
		if(xlrRC == XLR_SUCCESS)
		{
			if(bank_stat.Selected)
			{
				b = 1;
			}
		}
	}

	return b;
}

/* returns 0 or 1 for bank A or B, or < 0 if module not found or on error */
int Mark5BankSetByVSN(SSHANDLE *xlrDevice, const char *vsn)
{
	S_BANKSTATUS bank_stat;
	XLR_RETURN_CODE xlrRC;
	int b = -1;
	int bank=-1;
	int i;

	xlrRC = XLRGetBankStatus(*xlrDevice, BANK_A, &bank_stat);
	if(xlrRC == XLR_SUCCESS)
	{
		if(strncasecmp(bank_stat.Label, vsn, 8) == 0)
		{
			b = 0;
			bank = BANK_A;
		}
	}

	if(b == -1)
	{
		xlrRC = XLRGetBankStatus(*xlrDevice, BANK_B, &bank_stat);
		if(xlrRC == XLR_SUCCESS)
		{
			if(strncasecmp(bank_stat.Label, vsn, 8) == 0)
			{
				b = 1;
				bank = BANK_B;
			}
		}
	}

	if(bank < 0)
	{
		return -1;
	}

	xlrRC = XLRGetBankStatus(*xlrDevice, bank, &bank_stat);
	if(xlrRC != XLR_SUCCESS)
	{
		return -4;
	}
	if(bank_stat.Selected) // No need to bank switch
	{
		return b;
	}

	xlrRC = XLRSelectBank(*xlrDevice, bank);
	if(xlrRC != XLR_SUCCESS)
	{
		b = -2 - b;
	}
	else
	{
		sleep(5);

		for(i = 0; i < 100; i++)
		{
			xlrRC = XLRGetBankStatus(*xlrDevice, bank, &bank_stat);
			if(xlrRC != XLR_SUCCESS)
			{
				return -4;
			}
			if(bank_stat.State == STATE_READY && bank_stat.Selected)
			{
				break;
			}
			usleep(100000);
		}

		if(bank_stat.State != STATE_READY || !bank_stat.Selected)
		{
			b = -4;
		}
	}

	return b;
}

static int uniquifyScanNames(struct Mark5Module *module)
{
	char scanNames[MAXSCANS][MAXLENGTH];
	int nameCount[MAXSCANS];
	int origIndex[MAXSCANS];
	int i, j, n=0;
	char tmpStr[MAXLENGTH+5];

	if(!module)
	{
		return -1;
	}

	if(module->nscans < 2)
	{
		return 0;
	}

	strcpy(scanNames[0], module->scans[0].name);
	nameCount[0] = 1;
	origIndex[0] = 0;
	n = 1;

	for(i = 1; i < module->nscans; i++)
	{
		for(j = 0; j < n; j++)
		{
			if(strcmp(scanNames[j], module->scans[i].name) == 0)
			{
				nameCount[j]++;
				sprintf(tmpStr, "%s_%04d", scanNames[j], nameCount[j]);
				strncpy(module->scans[i].name, tmpStr, MAXLENGTH-1);
				module->scans[i].name[MAXLENGTH-1] = 0;
				break;
			}
		}
		if(j == n)
		{
			strcpy(scanNames[n], module->scans[i].name);
			nameCount[n] = 1;
			origIndex[n] = i;
			n++;
		}
	}

	/* rename those that would have had name extension _0001 */
	for(j = 0; j < n; j++)
	{
		if(nameCount[j] > 1)
		{
			i = origIndex[j];
			sprintf(tmpStr, "%s_%04d", scanNames[j], 1);
			strncpy(module->scans[i].name, tmpStr, MAXLENGTH-1);
			module->scans[i].name[MAXLENGTH-1] = 0;
		}
	}

	return 0;
}

static int getMark5Module(struct Mark5Module *module, SSHANDLE *xlrDevice, int mjdref, 
	int (*callback)(int, int, int, void *), void *data, float *replacedFrac)
{
	XLR_RETURN_CODE xlrRC;
	Mark5Directory m5dir;
	int len, i, n;
	struct mark5_format *mf;
	Mark5Scan *scan;
	char label[XLR_LABEL_LENGTH];
	int bank;
	unsigned long a, b;
	unsigned long *buffer;
	int bufferlen;
	unsigned int x, signature;
	int die = 0;
	long long wGood, wBad;
	long long wGoodSum=0, wBadSum=0;

	/* allocate a bit more than the minimum needed */
	bufferlen = 20160*8*20;

	bank = Mark5BankGet(xlrDevice);
	if(bank < 0)
	{
		return -1;
	}

	xlrRC = XLRGetLabel(*xlrDevice, label);
	if(xlrRC != XLR_SUCCESS)
	{
		return -1;
	}
	label[8] = 0;

	len = XLRGetUserDirLength(*xlrDevice);
	if(len < (signed int)sizeof(struct Mark5Directory))
	{
		return -1;
	}

	xlrRC = XLRGetUserDir(*xlrDevice, sizeof(struct Mark5Directory), 
		0, &m5dir);
	if(xlrRC != XLR_SUCCESS)
	{
		return -1;
	}

	/* the adventurous would use md5 here */
	signature = 1;
	for(i = 0; i < sizeof(struct Mark5Directory)/4; i++)
	{
		x = ((unsigned int *)(&m5dir))[i] + 1;
		signature = signature ^ x;
	}

	/* prevent a zero signature */
	if(signature == 0)
	{
		signature = 0x55555555;
	}

	if(module->signature == signature && module->nscans > 0)
	{
		module->bank = bank;
		return 0;
	}

	buffer = (unsigned long *)malloc(bufferlen);
	
	memset(module, 0, sizeof(struct Mark5Module));
	module->nscans = m5dir.nscans;
	module->bank = bank;
	strcpy(module->label, label);
	module->signature = signature;

	for(i = 0; i < module->nscans; i++)
	{
		wGood = wBad = 0;
		scan = module->scans + i;

		strncpy(scan->name, m5dir.scanName[i], MAXLENGTH);
		scan->start  = m5dir.start[i];
		scan->length = m5dir.length[i];
		if(scan->length < bufferlen*10)
		{
			if(callback)
			{
				die = callback(i, module->nscans, MARK5_DIR_SHORT_SCAN, data);
			}
			continue;
		}

		if(die)
		{
			break;
		}

		if(scan->start & 4)
		{
			scan->start -= 4;
			scan->length -= 4;
		}

		a = scan->start>>32;
		b = scan->start % (1LL<<32);

		xlrRC = XLRReadData(*xlrDevice, buffer, a, b, bufferlen);

		if(xlrRC == XLR_FAIL)
		{
			if(callback)
			{
				die = callback(i, module->nscans, MARK5_DIR_READ_ERROR, data);
			}
			scan->format = -2;
			continue;
		}

		countReplaced(buffer, bufferlen/4, &wGood, &wBad);

		if(die)
		{
			break;
		}

		mf = new_mark5_format_from_stream(new_mark5_stream_memory(buffer, bufferlen));
	
		if(!mf)
		{
			if(callback)
			{
				die = callback(i, module->nscans, MARK5_DIR_DECODE_ERROR, data);
			}
			scan->format = -1;
			continue;
		}
		
		scan->mjd = mf->mjd;
		scan->sec = mf->sec;
		n = (mjdref - scan->mjd + 500) / 1000;
		scan->mjd += n*1000;
		
		scan->format      = mf->format;
		scan->frameoffset = mf->frameoffset;
		scan->tracks      = mf->ntrack;
		scan->framespersecond = int(1000000000.0/mf->framens + 0.5);
		scan->framenuminsecond = int(mf->ns/mf->framens + 0.5);
		scan->framebytes  = mf->framebytes;
		scan->duration    = (int)((scan->length - scan->frameoffset)
			/ scan->framebytes)/(double)(scan->framespersecond);
		
		delete_mark5_format(mf);
/**/
#if 0
		mf = new_mark5_format_from_stream(new_mark5_stream_memory(buffer+(bufferlen/8), bufferlen));
	
		if(!mf)
		{
			if(callback)
			{
				die = callback(i, module->nscans, MARK5_DIR_DECODE_ERROR, data);
			}
			scan->format = -1;
			continue;
		}

		printf("sec=%d,%d  f=%d,%d  fb=%d,%d  fps=%d,%d  o=%d,%d\n", scan->sec, mf->sec, scan->framenuminsecond, int(mf->ns/mf->framens + 0.5), scan->framebytes, mf->framebytes, scan->framespersecond, int(1000000000.0/mf->framens + 0.5), scan->frameoffset, mf->frameoffset);

		scan->mjd = mf->mjd;
		scan->sec = mf->sec;
		n = (mjdref - scan->mjd + 500) / 1000;
		scan->mjd += n*1000;
		
		scan->format      = mf->format;
		scan->frameoffset = mf->frameoffset;
		scan->tracks      = mf->ntrack;
		scan->framespersecond = int(1000000000.0/mf->framens + 0.5);
		scan->framenuminsecond = int(mf->ns/mf->framens + 0.5);
		scan->framebytes  = mf->framebytes;
		scan->duration    = (int)((scan->length - scan->frameoffset)
			/ scan->framebytes)/(double)(scan->framespersecond);
		
		delete_mark5_format(mf);
#endif
/**/
		if(callback)
		{
			enum Mark5DirStatus s;

			if(wBad > 8)
			{
				s = MARK5_DIR_DECODE_WITH_REPLACEMENTS;
			}
			else
			{
				s = MARK5_DIR_DECODE_SUCCESS;
			}
			die = callback(i, module->nscans, s, data);
		}

		wGoodSum += wGood;
		wBadSum += wBad;

		if(die)
		{
			break;
		}
	}

	if(replacedFrac)
	{
		if(wGood > 0)
		{
			*replacedFrac = (double)wBad/(double)wGood;
		}
		else
		{
			*replacedFrac = 0;
		}
	}

	free(buffer);

	uniquifyScanNames(module);

	return -die;
}

void printMark5Module(const struct Mark5Module *module)
{
	int i, n;
	const Mark5Scan *scan;

	if(!module)
	{
		return;
	}
	if(module->bank < 0)
	{
		return;
	}
	
	printf("Module Name = %s   Nscans = %d   Bank = %c  Sig = %u\n", 
		module->label, module->nscans, module->bank+'A', 
		module->signature);

	n = module->nscans;
	for(i = 0; i < n; i++)
	{
		scan = module->scans + i;
	
		printf("%3d %1d %-32s %13Ld %13Ld %5d %2d %5d %5d+%d/%d %6.4f\n",
			i+1,
			scan->format,
			scan->name,
			scan->start,
			scan->start+scan->length,
			scan->frameoffset,
			scan->tracks,
			scan->mjd,
			scan->sec,
			scan->framenuminsecond,
			scan->framespersecond,
			scan->duration);
	}
}

int loadMark5Module(struct Mark5Module *module, const char *filename)
{
	FILE *in;
	struct Mark5Scan *scan;
	char line[256];
	int i, nscans, n;
	char bank;
	char label[XLR_LABEL_LENGTH];
	unsigned int signature;

	if(!module)
	{
		return -1;
	}

	module->label[0] = 0;
	module->nscans = 0;
	module->bank = -1;

	in = fopen(filename, "r");
	if(!in)
	{
		return -1;
	}

	fgets(line, 255, in);
	if(feof(in))
	{
		fclose(in);
		return -1;
	}

	n = sscanf(line, "%8s %d %c %u", label, &nscans, &bank, &signature);
	if(n < 3)
	{
		fclose(in);
		return -1;
	}
	if(n == 3)
	{
		signature = ~0;
	}

	if(nscans > MAXSCANS || nscans < 0)
	{
		fclose(in);
		return -1;
	}

	strcpy(module->label, label);
	module->nscans = nscans;
	module->bank = bank-'A';
	module->signature = signature;

	for(i = 0; i < nscans; i++)
	{
		scan = module->scans + i;

		fgets(line, 255, in);
		if(feof(in))
		{
			module->nscans = i;
			fclose(in);
			return -1;
		}
		
		sscanf(line, "%Ld%Ld%d%d%d%d%lf%d%d%d%d%63s",
			&scan->start, 
			&scan->length,
			&scan->mjd,
			&scan->sec,
			&scan->framenuminsecond,
			&scan->framespersecond,
			&scan->duration,
			&scan->framebytes,
			&scan->frameoffset,
			&scan->tracks,
			&scan->format,
			scan->name);
	}

	fclose(in);
	
	return 0;
}

int saveMark5Module(struct Mark5Module *module, const char *filename)
{
	FILE *out;
	struct Mark5Scan *scan;
	int i;
	
	if(!module)
	{
		return -1;
	}

	out = fopen(filename, "w");
	if(!out)
	{
		return -1;
	}

	fprintf(out, "%8s %d %c %u\n",
		module->label,
		module->nscans,
		module->bank+'A',
		module->signature);
	for(i = 0; i < module->nscans; i++)
	{
		scan = module->scans + i;
		
		fprintf(out, "%14Ld %14Ld %5d %d %d %d %12.6f %6d %6d %2d %1d %s\n",
			scan->start, 
			scan->length,
			scan->mjd,
			scan->sec,
			scan->framenuminsecond,
			scan->framespersecond,
			scan->duration,
			scan->framebytes,
			scan->frameoffset,
			scan->tracks,
			scan->format,
			scan->name);
	}

	fclose(out);

	return 0;
}

/* retrieves directory (either from cache or module) and makes sure
 * desired module is the active one.  On any failure return < 0 
 */
int getCachedMark5Module(struct Mark5Module *module, SSHANDLE *xlrDevice, 
	int mjdref, const char *vsn, const char *dir,
	int (*callback)(int, int, int, void *), void *data,
	float *replacedFrac)
{
	char filename[256];
	int v, curbank;

	curbank = Mark5BankSetByVSN(xlrDevice, vsn);
	if(curbank < 0)
	{
		return -1;
	}
	
	sprintf(filename, "%s/%s.dir", dir, vsn);
	
	v = loadMark5Module(module, filename);

	v = getMark5Module(module, xlrDevice, mjdref, callback, data, replacedFrac);

	if(v >= 0)
	{
		saveMark5Module(module, filename);
	}

	return v;
}

int sanityCheckModule(const struct Mark5Module *module)
{
	int i;

	for(i = 0; i < module->nscans; i++)
	{
		if(module->scans[i].format < 0)
		{
			return -1;
		}
	}

	return 0;
}

int getByteRange(const struct Mark5Scan *scan, long long *byteStart, long long *byteStop, double mjdStart, double mjdStop)
{
	double scanStart, scanStop, R;
	long long delta;

	if(scan->length <= 0)
	{
		return 0;
	}

	scanStart = scan->mjd + (scan->sec + (float)(scan->framenuminsecond)/(float)(scan->framespersecond))/86400.0;
	scanStop = scanStart + scan->duration/86400.0;

	if(scanStart >= mjdStop || scanStop <= mjdStart)
	{
		return 0;
	}

	R = scan->length*86400.0/scan->duration;

	if(mjdStart <= scanStart)
	{
		*byteStart = scan->start;
	}
	else
	{
		*byteStart = (int)(scan->start + R*(mjdStart - scanStart));
	}

	if(mjdStop >= scanStop)
	{
		*byteStop = scan->start + scan->length;
	}
	else
	{
		*byteStop = (int)(scan->start + R*(mjdStop - scanStart));
	}

	/* make sure read is aligned with data frames */
	delta = (*byteStart - scan->frameoffset) % scan->framebytes;
	*byteStart -= delta;
	if(*byteStart < scan->start)
	{
		*byteStart += scan->framebytes;
	}

	delta = (*byteStop - scan->frameoffset) % scan->framebytes;
	*byteStop  -= delta;

	return 1;
}
