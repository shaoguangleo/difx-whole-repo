/***************************************************************************
 *   Copyright (C) 2006-2015 by Adam Deller and Walter Brisken             *
 *                                                                         *
 *   This program is free for non-commercial use: see the license file     *
 *   at http://astronomy.swin.edu.au:~adeller/software/difx/ for more      *
 *   details.                                                              *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
//===========================================================================
// SVN properties (DO NOT CHANGE)
//
// $Id: vdiffile.cpp 6684 2015-06-02 11:40:23Z HelgeRottmann $
// $HeadURL: https://svn.atnf.csiro.au/difx/mpifxcorr/trunk/src/mk5.cpp $
// $LastChangedRevision: 6684 $
// $Author: HelgeRottmann $
// $LastChangedDate: 2015-06-02 06:40:23 -0500 (Tue, 02 Jun 2015) $
//
//============================================================================
#include <cmath>
#include <cstring>
#include <mpi.h>
#include <iomanip>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "config.h"
#include "alert.h"
#include "vdifmark6.h"
#include "mode.h"
#include "vdifmark6_datastream.h"



/* TODO: 
   - make use of activesec and activescan
 */


/// VDIFMark6DataStream -------------------------------------------------------

VDIFMark6DataStream::VDIFMark6DataStream(const Configuration * conf, int snum, int id, int ncores, int * cids, int bufferfactor, int numsegments)
 : VDIFDataStream(conf, snum, id, ncores, cids, bufferfactor, numsegments)
{
	cwarn << startl << "Starting Mark6 datastream.  This is experimental at this time!" << endl;
	mark6gather = 0;
	mark6eof = false;
}

VDIFMark6DataStream::~VDIFMark6DataStream()
{
	cwarn << startl << "Ending Mark6 datastream.  Maybe it worked?" << endl;
	closeMark6();
}

void VDIFMark6DataStream::closeMark6()
{
	if(mark6gather != 0)
	{
		closeMark6Gatherer(mark6gather);
	}
	mark6gather = 0;
	mark6eof = false;
}

void VDIFMark6DataStream::openfile(int configindex, int fileindex)
{
  closeMark6();

  cverbose << startl << "Mark6 datastream " << mpiid << " is about to try and open file index " << fileindex << " of configindex " << configindex << endl;
  if(fileindex >= confignumfiles[configindex]) //run out of files - time to stop reading
  {
    dataremaining = false;
    keepreading = false;
    cinfo << startl << "Mark6 datastream " << mpiid << " is exiting because fileindex is " << fileindex << ", while confignumfiles is " << confignumfiles[configindex] << endl;
    return;
  }
  
  dataremaining = true;

  mark6gather = openMark6GathererFromTemplate(datafilenames[configindex][fileindex].c_str());

  cverbose << startl << "mark6gather is " << mark6gather << endl;
  if(mark6gather == 0)
  {
    cerror << startl << "Cannot open mark6 data file " << datafilenames[configindex][fileindex] << endl;
    dataremaining = false;
    return;
  }

  if(isMark6GatherComplete(mark6gather) == 0)
  {
    cwarn << startl << "Warning: Mark6 file " << datafilenames[configindex][fileindex] << " seems to have an incomplete set of files.  Your weights may suffer if this is true." << endl;
  }

  cinfo << startl << "Mark6 datastream " << mpiid << " has opened file index " << fileindex << ", which was " << datafilenames[configindex][fileindex] << endl;

  isnewfile = true;
  //read the header and set the appropriate times etc based on this information
  initialiseFile(configindex, fileindex);
}

void VDIFMark6DataStream::initialiseFile(int configindex, int fileindex)
{
	int nrecordedbands, fanout;
	Configuration::datasampling sampling;
	Configuration::dataformat format;
	double bw;
	int rv;

	long long dataoffset = 0;
	struct vdif_file_summary fileSummary;
	int jumpseconds, currentdsseconds;

	format = config->getDataFormat(configindex, streamnum);
	sampling = config->getDSampling(configindex, streamnum);
	nbits = config->getDNumBits(configindex, streamnum);
	nrecordedbands = config->getDNumRecordedBands(configindex, streamnum);
	inputframebytes = config->getFrameBytes(configindex, streamnum);
	framespersecond = config->getFramesPerSecond(configindex, streamnum)/config->getDNumMuxThreads(configindex, streamnum);
	bw = config->getDRecordedBandwidth(configindex, streamnum, 0);

	nGap = framespersecond/4;	// 1/4 second gap of data yields a mux break
	if(nGap > 1024)
	{
		nGap = 1024;
	}
	startOutputFrameNumber = -1;

	nthreads = config->getDNumMuxThreads(configindex, streamnum);
	threads = config->getDMuxThreadMap(configindex, streamnum);

	rv = configurevdifmux(&vm, inputframebytes, framespersecond, nbits, nthreads, threads, nSort, nGap, VDIF_MUX_FLAG_RESPECTGRANULARITY);
	if(rv < 0)
	{
		cfatal << startl << "configurevmux failed with return code " << rv << endl;
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	int nChanPerThread = nrecordedbands/nthreads;
	if(nChanPerThread != 1)
	{
		cinfo << startl << "Note: " << nChanPerThread << " channels reside on each thread.  Support for this is new.  Congratulations for being bold and trying this out!  Warranty void in the 193 UN recognized nations." << endl;
	}
	if(nChanPerThread * nthreads != nrecordedbands)
	{
		cerror << startl << "Error: " << nrecordedbands << " bands were recorded but they are divided unequally across " << nthreads << " threads.  This is not allowed.  Things will probably get very bad soon..." << endl;
	}
	setvdifmuxinputchannels(&vm, nChanPerThread);

	fanout = config->genMk5FormatName(format, nrecordedbands, bw, nbits, sampling, vm.outputFrameSize, config->getDDecimationFactor(configindex, streamnum), config->getDNumMuxThreads(configindex, streamnum), formatname);
	if(fanout != 1)
	{
		cfatal << startl << "Fanout is " << fanout << ", which is impossible; no choice but to abort!" << endl;
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	cinfo << startl << "VDIFMark6DataStream::initialiseFile format=" << formatname << endl;

	// Here we need to open the file, read the start time, jump if necessary, and if past end of file, dataremaining = false.  Then set readseconds...

	// First we get a description of the contents of the purported VDIF file and exit if it looks like not VDIF at all
	rv = summarizevdifmark6(&fileSummary, datafilenames[configindex][fileindex].c_str(), inputframebytes);
	if(rv < 0)
	{
		cwarn << startl << "VDIFMark6DataStream::initialiseFile: summary of file " << datafilenames[configindex][fileindex] << " resulted in error code " << rv << ".  This does not look like valid VDIF data." << endl;
		dataremaining = false;

		return;
	}
	vdiffilesummarysetsamplerate(&fileSummary, static_cast<int64_t>(bw*2*1000000));

	// If verbose...
	printvdiffilesummary(&fileSummary);

	// Here set readseconds to time since beginning of job
	readseconds = 86400*(vdiffilesummarygetstartmjd(&fileSummary)-corrstartday) + vdiffilesummarygetstartsecond(&fileSummary)-corrstartseconds + intclockseconds;
	readnanoseconds = vdiffilesummarygetstartns(&fileSummary);
	currentdsseconds = activesec + model->getScanStartSec(activescan, config->getStartMJD(), config->getStartSeconds());

	if(currentdsseconds > readseconds+1)
	{
		jumpseconds = currentdsseconds - readseconds;
		if(activens < readnanoseconds)
		{
			jumpseconds--;
		}

		// set byte offset to the requested time

		int n, d;	// numerator and demoninator of frame/payload size ratio
		n = fileSummary.frameSize;
		d = fileSummary.frameSize - 32;

		dataoffset = static_cast<long long>(jumpseconds*vdiffilesummarygetbytespersecond(&fileSummary)/d*n + 0.5);

		readseconds += jumpseconds;
	}

	// Now set readseconds to time since beginning of scan
	readseconds = readseconds - model->getScanStartSec(readscan, corrstartday, corrstartseconds);
	
	// Advance into file if requested
#if 0

FIXME: get seeking working

	if(fileSummary.firstFrameOffset + dataoffset > 0)
	{
		off_t seek_pos;
		cverbose << startl << "Mark6 about to seek to byte " << fileSummary.firstFrameOffset << " plus jump " << dataoffset << " to get to the first wanted frame" << endl;

		seek_pos = mark6_sg_lseek(mark6gather, fileSummary.firstFrameOffset + dataoffset, SEEK_SET);

		if(seek_pos != fileSummary.firstFrameOffset + dataoffset)
		{
			cinfo << startl << "File " << datafilenames[configindex][fileindex] << " ended before the currently desired time" << endl;
			dataremaining = false;
			closeMark6();
		}
	}
#endif
}


// This function does the actual file IO, readbuffer management, and VDIF multiplexing.  The result after each
// call is, hopefully, readbytes of multiplexed data being put into buffer segment with potentially some 
// read data left over in the read buffer ready for next time
int VDIFMark6DataStream::dataRead(int buffersegment)
{
	unsigned char *destination;
	int bytes, bytestoread;
	int muxReturn;
	unsigned int bytesvisible;

	destination = reinterpret_cast<unsigned char *>(&databuffer[buffersegment*(bufferbytes/numdatasegments)]);

	// Bytes to read
	bytes = readbuffersize - readbufferleftover;

	// if the file is exhausted, just multiplex any leftover data and return
	if(mark6eof)
	{
		// If there is some data left over, just demux that and send it out
		if(readbufferleftover > minleftoverdata)
		{
			vdifmux(destination, readbytes, readbuffer, readbufferleftover, &vm, startOutputFrameNumber, &vstats);
			readbufferleftover = 0;
			bufferinfo[buffersegment].validbytes = vstats.destUsed;

			startOutputFrameNumber = -1;
		}
		else
		{
			// Really, this should not happen based, but just in case...
			bufferinfo[buffersegment].validbytes = 0;
		}
		dataremaining = false;

		return 0;
	}

	// execute the file read
	input.read(reinterpret_cast<char *>(readbuffer) + readbufferleftover, bytes);
	bytestoread = bytes;
	bytes = mark6Gather(mark6gather, reinterpret_cast<char *>(readbuffer) + readbufferleftover, bytestoread);
	if(bytes < bytestoread)
	{
		// file ran out
		mark6eof = true;
	}

	bytesvisible = readbufferleftover + bytes;

	// multiplex and corner turn the data
	muxReturn = vdifmux(destination, readbytes, readbuffer, bytesvisible, &vm, startOutputFrameNumber, &vstats);

	if(muxReturn < 0)
	{
		cwarn << startl << "vdifmux returned " << muxReturn << endl;
	}

	consumedbytes += bytes;
	bufferinfo[buffersegment].validbytes = vstats.destUsed;
	bufferinfo[buffersegment].readto = true;
	if(bufferinfo[buffersegment].validbytes > 0)
	{
		// In the case of VDIF, we can get the time from the data, so use that just in case there was a jump
		bufferinfo[buffersegment].scanns = (((vstats.startFrameNumber) % framespersecond) * 1000000000LL) / framespersecond;
		// FIXME: warning! here we are assuming no leap seconds since the epoch of the VDIF stream. FIXME
		// FIXME: below assumes each scan is < 86400 seconds long
		bufferinfo[buffersegment].scanseconds = ((vstats.startFrameNumber / framespersecond) % 86400) + intclockseconds - corrstartseconds - model->getScanStartSec(readscan, corrstartday, corrstartseconds);
		if(bufferinfo[buffersegment].scanseconds > 86400/2)
		{
			bufferinfo[buffersegment].scanseconds -= 86400;
		}
		else if(bufferinfo[buffersegment].scanseconds < -86400/2)
		{
			bufferinfo[buffersegment].scanseconds += 86400;
		}

		readnanoseconds = bufferinfo[buffersegment].scanns;
		readseconds = bufferinfo[buffersegment].scanseconds;

		// look at difference in data frames consumed and produced and proceed accordingly
		int deltaDataFrames = vstats.srcUsed/(nthreads*inputframebytes) - vstats.destUsed/(nthreads*(inputframebytes-VDIF_HEADER_BYTES) + VDIF_HEADER_BYTES);
		if(deltaDataFrames == 0)
		{
			// We should be able to preset startOutputFrameNumber.  Warning: early use of this was frought with peril but things seem OK now.
			startOutputFrameNumber = vstats.startFrameNumber + vstats.nOutputFrame;
		}
		else
		{
			if(deltaDataFrames < -10)
			{
				static int nGapWarn = 0;

				++nGapWarn;
				if( (nGapWarn & (nGapWarn - 1)) == 0)
				{
					cwarn << startl << "Data gap of " << (vstats.destUsed-vstats.srcUsed) << " bytes out of " << vstats.destUsed << " bytes found. startOutputFrameNumber=" << startOutputFrameNumber << " bytesvisible=" << bytesvisible << " N=" << nGapWarn << endl;
				}
			}
			else if(deltaDataFrames > 10)
			{
				static int nExcessWarn = 0;

				++nExcessWarn;
				if( (nExcessWarn & (nExcessWarn - 1)) == 0)
				{
					cwarn << startl << "Data excess of " << (vstats.srcUsed-vstats.destUsed) << " bytes out of " << vstats.destUsed << " bytes found. startOutputFrameNumber=" << startOutputFrameNumber << " bytesvisible=" << bytesvisible << " N=" << nExcessWarn << endl;
				}
			}
			startOutputFrameNumber = -1;
		}
	}
	else
	{
		startOutputFrameNumber = -1;
	}

	readbufferleftover += (bytes - vstats.srcUsed);

	if(readbufferleftover > 0)
	{
		memmove(readbuffer, readbuffer+vstats.srcUsed, readbufferleftover);
	}
	else if(readbufferleftover < 0)
	{
		cwarn << startl << "readbufferleftover = " << readbufferleftover << "; it should never be negative." << endl;

		readbufferleftover = 0;
	}
	if(readbufferleftover <= minleftoverdata && mark6eof)
	{
		readbufferleftover = 0;

		// here we've in one call both read all the remaining data from a file and multiplexed it all without leftovers
		dataremaining = false;
	}

	return bytes;
}

void VDIFMark6DataStream::mark6ToMemory(int buffersegment)
{
	u32 *buf;

	buf = reinterpret_cast<u32 *>(&databuffer[buffersegment*(bufferbytes/numdatasegments)]);

	//do the buffer housekeeping
	waitForBuffer(buffersegment);

	// This function call abstracts away all the details.  The result is multiplexed data populating the 
	// desired buffer segment.
	dataRead(buffersegment);

	// Update estimated read timing variables
	readnanoseconds += (bufferinfo[buffersegment].nsinc % 1000000000);
	readseconds += (bufferinfo[buffersegment].nsinc / 1000000000);
	readseconds += readnanoseconds/1000000000;
	readnanoseconds %= 1000000000;

	if(vstats.destUsed == 0)
	{
		++invalidtime;
	}
	else
	{
		invalidtime = 0;
	}

	// did we just cross into next scan?
	if(readseconds >= model->getScanDuration(readscan))
	{
		cinfo << startl << "mark6ToMemory: end of schedule scan " << readscan << " of " << model->getNumScans() << " detected" << endl;

		// find next valid schedule scan
		do
		{
			++readscan;
		} while(readscan < model->getNumScans() && config->getScanConfigIndex(readscan));

		cinfo << startl << "readscan incremented to " << readscan << endl;

		if(readscan < model->getNumScans())
		{
			//if we need to, change the config
			if(config->getScanConfigIndex(readscan) != bufferinfo[(lastvalidsegment + 1)%numdatasegments].configindex)
			{
				updateConfig((lastvalidsegment + 1)%numdatasegments);
			}
		}
		else
		{
			// here we just crossed over the end of the job
			cverbose << startl << "readscan==getNumScans -> keepreading=false" << endl;
			
			keepreading = false;

			bufferinfo[(lastvalidsegment+1)%numdatasegments].scan = model->getNumScans()-1;
			bufferinfo[(lastvalidsegment+1)%numdatasegments].scanseconds = model->getScanDuration(model->getNumScans()-1);
			bufferinfo[(lastvalidsegment+1)%numdatasegments].scanns = 0;
		}
		cinfo << startl << "mark6ToMemory: starting schedule scan " << readscan << endl;
	}

	if(switchedpower && bufferinfo[buffersegment].validbytes > 0)
	{
		static int nt = 0;

		++nt;

                // feed switched power detector
		if(nt % switchedpowerincrement == 0)
		{
			struct mark5_stream *m5stream = new_mark5_stream_absorb(
				new_mark5_stream_memory(buf, bufferinfo[buffersegment].validbytes),
				new_mark5_format_generic_from_string(formatname) );
			if(m5stream)
			{
				mark5_stream_fix_mjd(m5stream, config->getStartMJD());
				switchedpower->feed(m5stream);
				delete_mark5_stream(m5stream);
			}
                }
	}
}

void VDIFMark6DataStream::loopfileread()
{
	int perr;
	int numread = 0;

cverbose << startl << "Starting VDIFMark6DataStream::loopfileread()" << endl;

	//lock the outstanding send lock
	perr = pthread_mutex_lock(&outstandingsendlock);
	if(perr != 0)
	{
		csevere << startl << "Error in initial readthread lock of outstandingsendlock!" << endl;
	}

	//lock the first section to start reading
	dataremaining = false;
	keepreading = true;
	while(!dataremaining && keepreading)
	{
cverbose << startl << "Mark6: opening file " << filesread[bufferinfo[0].configindex] << endl;
		openfile(bufferinfo[0].configindex, filesread[bufferinfo[0].configindex]++);
		if(!dataremaining && mark6gather != 0)
		{
			closeMark6();
		}
	}

cverbose << startl << "Mark6: Opened first usable file" << endl;

	if(keepreading)
	{
		mark6ToMemory(numread++);
		mark6ToMemory(numread++);
		lastvalidsegment = numread;
		perr = pthread_mutex_lock(&(bufferlock[numread]));
		if(perr != 0)
		{
			csevere << startl << "Error in initial readthread lock of first buffer section!" << endl;
		}
	}
	else
	{
		csevere << startl << "Couldn't find any valid data; will be shutting down gracefully!" << endl;
	}
	readthreadstarted = true;
	perr = pthread_cond_signal(&initcond);
	if(perr != 0)
	{
		csevere << startl << "Datastream readthread error trying to signal main thread to wake up!" << endl;
	}
	if(keepreading)
	{
		mark6ToMemory(numread++);
	}

cverbose << startl << "Mark6: Opened first usable file" << endl;

	while(keepreading && (bufferinfo[lastvalidsegment].configindex < 0 || filesread[bufferinfo[lastvalidsegment].configindex] <= confignumfiles[bufferinfo[lastvalidsegment].configindex]))
	{
		while(dataremaining && keepreading)
		{
			lastvalidsegment = (lastvalidsegment + 1)%numdatasegments;

			//lock the next section
			perr = pthread_mutex_lock(&(bufferlock[lastvalidsegment]));
			if(perr != 0)
			{
				csevere << startl << "Error in readthread lock of buffer section!" << lastvalidsegment << endl;
			}

			if(!isnewfile) //can unlock previous section immediately
			{
				//unlock the previous section
				perr = pthread_mutex_unlock(&(bufferlock[(lastvalidsegment-1+numdatasegments)% numdatasegments]));    
				if(perr != 0)
				{
					csevere << startl << "Error (" << perr << ") in readthread unlock of buffer section!" << (lastvalidsegment-1+numdatasegments)%numdatasegments << endl;
				}
			}

			//do the read
			mark6ToMemory(lastvalidsegment);
			numread++;

			if(isnewfile) //had to wait before unlocking file
			{
				//unlock the previous section
				perr = pthread_mutex_unlock(&(bufferlock[(lastvalidsegment-1+numdatasegments)% numdatasegments]));
				if(perr != 0)
				{
					csevere << startl << "Error (" << perr << ") in readthread unlock of buffer section!" << (lastvalidsegment-1+numdatasegments)%numdatasegments << endl;
				}
			}
			isnewfile = false;
		}
		if(keepreading)
		{
cverbose << startl << "keepreading is true" << endl;
			closeMark6();

			//if we need to, change the config
			int nextconfigindex = config->getScanConfigIndex(readscan);
			while(nextconfigindex < 0 && readscan < model->getNumScans())
			{
				readseconds = 0; 
				nextconfigindex = config->getScanConfigIndex(++readscan);
			}
			if(readscan == model->getNumScans())
			{
				bufferinfo[(lastvalidsegment+1)%numdatasegments].scan = model->getNumScans()-1;
				bufferinfo[(lastvalidsegment+1)%numdatasegments].scanseconds = model->getScanDuration(model->getNumScans()-1);
				bufferinfo[(lastvalidsegment+1)%numdatasegments].scanns = 0;
				keepreading = false;
			}
			else
			{
				if(config->getScanConfigIndex(readscan) != bufferinfo[(lastvalidsegment + 1)%numdatasegments].configindex)
				{
					updateConfig((lastvalidsegment + 1)%numdatasegments);
				}
				//if the datastreams for two or more configs are common, they'll all have the same 
				//files.  Therefore work with the lowest one
				int lowestconfigindex = bufferinfo[(lastvalidsegment+1)%numdatasegments].configindex;
				for(int i=config->getNumConfigs()-1;i>=0;i--)
				{
					if(config->getDDataFileNames(i, streamnum) == config->getDDataFileNames(lowestconfigindex, streamnum))
					lowestconfigindex = i;
				}
				openfile(lowestconfigindex, filesread[lowestconfigindex]++);
				bool skipsomefiles = (config->getScanConfigIndex(readscan) < 0)?true:false;
				while(skipsomefiles)
				{
					int nextscan = peekfile(lowestconfigindex, filesread[lowestconfigindex]);
					if(nextscan == readscan || (nextscan == readscan+1 && config->getScanConfigIndex(nextscan) < 0))
					{
						openfile(lowestconfigindex, filesread[lowestconfigindex]++);
					}
					else
					{
						skipsomefiles = false;
					}
				}
			}
		}
	}
	if(mark6gather != 0)
	{
		closeMark6();
	}
	if(numread > 0)
	{
		perr = pthread_mutex_unlock(&(bufferlock[lastvalidsegment]));
		if(perr != 0)
		{
			csevere << startl << "Error (" << perr << ") in readthread unlock of buffer section!" << lastvalidsegment << endl;
		}
	}

	//unlock the outstanding send lock
	perr = pthread_mutex_unlock(&outstandingsendlock);
	if(perr != 0)
	{
		csevere << startl << "Error (" << perr << ") in readthread unlock of outstandingsendlock!" << endl;
	}

	if(lastvalidsegment >= 0)
	{
		cverbose << startl << "Datastream readthread is exiting! Filecount was " << filesread[bufferinfo[lastvalidsegment].configindex] << ", confignumfiles was " << confignumfiles[bufferinfo[lastvalidsegment].configindex] << ", dataremaining was " << dataremaining << ", keepreading was " << keepreading << endl;
	}
	else
	{
		cverbose << startl << "Datastream readthread is exiting, after not finding any data at all!" << endl;
	}
}
