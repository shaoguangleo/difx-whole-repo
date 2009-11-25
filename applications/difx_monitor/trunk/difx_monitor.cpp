/***************************************************************************
 *   Copyright (C) 2006 by Adam Deller                                     *
 *                                                                         *
 *   This program is free for non-commercial use: see the license file     *
 *   at http://astronomy.swin.edu.au:~adeller/software/difx/ for more      *
 *   details.                                                              *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <cpgplot.h>
#include <string.h>
#include <sstream>
#include "architecture.h"
#include "configuration.h"
#include "monserver.h"
#include <iomanip>

using namespace std;

#define MAXPOL 4

//prototypes
void plot_results();
void change_config();
void maxmin(f32 **vals, int nchan, int npol, float *max, float *min);

Configuration * config;
int socketnumber, currentconfigindex, maxresultlength, nav, atseconds;
int resultlength, numchannels, nprod=0;
double intseconds = 1;
IppsFFTSpec_R_32f* fftspec=NULL;
cf32 ** products=NULL;
f32  *phase[MAXPOL], *amplitude[MAXPOL];
f32 *lags[MAXPOL];
f32 *xval=NULL;

int main(int argc, const char * argv[])
{
  int i, status = 1;
  int prod, nvis, startsec;
  cf32 *vis;
  struct monclient monserver;

  if(argc < 3 || argc > 4)
  {
    cerr << "Error - invoke with difx_monitor <inputfile> <host> [int time (s)]" << endl;
    return EXIT_FAILURE;
  }

  //work out the config stuff
  config = new Configuration(argv[1], 0);
  config->loaduvwinfo(true);
  startsec = config->getStartSeconds();

  if(argc == 4)
    intseconds = atof(argv[3]);


  status  = monserver_connect(&monserver, (char*)argv[2],  Configuration::MONITOR_TCP_WINDOWBYTES);
  if (status) exit(1);

  status = monserver_requestall(monserver);
  if (status) exit(1);

  //get into the loop of receiving, processing and plotting!
  currentconfigindex = -1;
  atseconds = 0;

  umask(002);

  for (i=0; i<MAXPOL; i++) {
    phase[i] = NULL;
    amplitude[i] = NULL;
    lags[i] = NULL;
  }

  nvis=0;
  while (1) {

    status = monserver_readvis(&monserver);
    if (status) break;
    nvis++;

    cout << "Got visibility # " << monserver.timestamp << endl;

    if (monserver.timestamp==-1) continue;

    atseconds = monserver.timestamp-startsec;
    if(config->getConfigIndex(atseconds) != currentconfigindex) 
      change_config();

    while (!monserver_nextvis(&monserver, &prod, &vis)) {
      if (prod>=nprod) {
	cerr << "Got product larger than expected - aborting" << endl;
	exit(1);
      }

      status = vectorAdd_cf32_I(vis, products[prod], numchannels);

      if(status != vecNoErr) {
	cerr << "Error trying to add to accumulation buffer - aborting!" << endl;
	exit(1);
      }
    }

    //plot
    if(nvis==nav) {
      plot_results();
      nvis=0;
      for (i=0; i<nprod; i++) {
	status = vectorZero_cf32(products[i], numchannels);
	if(status != vecNoErr) {
	  cerr << "Error trying to zero visibility buffer - aborting!" << endl;
	  exit(1);
	}
      }
    }
  }

  //close the socket
  monserver_close(&monserver);
}

void plot_results()
{
  int i, j, k, npol;
  char pgplotname[256];
  char polpair[MAXPOL][3];
  char timestr[10];
  string sourcename;
  ostringstream ss;
  f32 temp;
  cf32 div;
  int status;
  int colours[MAXPOL] = {2,3,4,5};

  for (i=0; i<MAXPOL; i++)  polpair[i][2] = 0;

  div.re = 1.0/nav;
  div.im = 0;
  for(int i=0;i<nprod;i++) {

    status = vectorMulC_cf32_I(div, products[i], numchannels);
    if(status != vecNoErr) {
          cerr << "Error trying to add to accumulation buffer - aborting!" << endl;
	  exit(1);
    }
  }

  int binloop = 1;
  if(config->pulsarBinOn(currentconfigindex) && !config->scrunchOutputOn(currentconfigindex))
    binloop = config->getNumPulsarBins(currentconfigindex);

  int at = 0;
  //int sourceindex = config->getSourceIndex
  for(i=0;i<config->getNumBaselines();i++)
  {

    int ds1index = config->getBDataStream1Index(currentconfigindex, i);
    int ds2index = config->getBDataStream2Index(currentconfigindex, i);

    for(j=0;j<config->getBNumFreqs(currentconfigindex,i);j++)
    {
      int freqindex = config->getBFreqIndex(currentconfigindex, i, j);
      for(int b=0;b<binloop;b++) {
	npol = config->getBNumPolProducts(currentconfigindex,i,j);
	if (npol>MAXPOL) {
	  cerr << "Too many polarisation products - aborting" << endl;
	  exit(1);
	}
        for(k=0;k<npol;k++) {
          config->getBPolPair(currentconfigindex,i,j,k,polpair[k]);

	  // Calculate amplitude, phase and lags
	  status = vectorPhase_cf32(products[at], phase[k], numchannels);
	  if(status != vecNoErr) {
	    cerr << "Error trying to calculate phase - aborting!" << endl;
	    exit(1);
	  }
	  vectorMulC_f32_I(180/M_PI, phase[k], numchannels);

	  status = vectorMagnitude_cf32(products[at], amplitude[k], numchannels);
	  if(status != vecNoErr) {
	    cerr << "Error trying to calculate amplitude - aborting!" << endl;
	    exit(1);
	  }

	  ippsFFTInv_CCSToR_32f((Ipp32f*)products[at], lags[k], fftspec, 0);
	  //rearrange the lags into order
	  for(int l=0;l<numchannels;l++) {
            temp = lags[k][l];
            lags[k][l] = lags[k][l+numchannels];
	    lags[k][l+numchannels] = temp;
          }
          at++;
	}

	//plot something - data is from resultbuffer[at] to resultbuffer[at+numchannels+1]
	sprintf(pgplotname, "lba-%d-f%d-b%d.png/png", i, j, b);

	status = cpgbeg(0,pgplotname,1,3);
	if (status != 1) {
	  cout << "Error opening pgplot device: " << pgplotname << endl;
	} else {
	  float max, min;

	  cpgscr(0,1,1,1);
	  cpgscr(1,0,0,0);

	  // Plot lags
	  maxmin(lags, numchannels*2, npol, &max, &min);

	  cpgsch(1.5);
	  cpgsci(1);
	  cpgenv(0,numchannels*2,min,max,0,0);
	  cpglab("Channel", "Correlation coefficient", "");
	  cpgsci(2);

	  for (k=0; k<npol; k++) {
	    cpgsci(colours[k]);
	    cpgline(numchannels*2, xval, lags[k]);
	  }

	  // Annotate
	  config->getUVW()->getSourceName(config->getStartMJD(), 
		  atseconds+config->getStartSeconds(),sourcename);

	  cpgsci(4);
	  cpgsch(2.5);

	  ss << config->getTelescopeName(ds1index) 
	     << "-" 
	     <<  config->getTelescopeName(ds2index)
	     << "  " << sourcename; 

	  cpgmtxt("T", -1.5,0.02, 0, ss.str().c_str());	    
	  ss.str("");

	  ss << config->getFreqTableFreq(freqindex) << " MHz";
	  cpgmtxt("T", -2.6,0.98,1,ss.str().c_str());	    
	  ss.str("");

	  int seconds = atseconds+config->getStartSeconds();
	  int hours = seconds/3600;
	  seconds -= hours*3600;
	  int minutes = seconds/60;
	  seconds %= 60;
	  sprintf(timestr, "%02d:%02d:%02d", hours, minutes, seconds);
	  cpgmtxt("T", -1.5,0.98,1,timestr);

	  for (k=0; k<npol; k++) {
	    int p = npol-k-1;
	    cpgsci(colours[p]);
	    cpgmtxt("B", -1,0.97-0.03*k,1,polpair[p]);
	  }

	  cpgsch(1.5);
	  cpgsci(1);

	  // Plot Amplitude
	  maxmin(amplitude, numchannels, npol, &max, &min);

	  cpgsci(1);
	  cpgenv(0,numchannels,min,max,0,0);
	  cpglab("Channel", "Amplitude (Jy)", "");
	  cpgsci(2);

	  for (k=0; k<npol; k++) {
	    cpgsci(colours[k]);
	    cpgline(numchannels, xval, amplitude[k]);
	  }


	  // Plot Phase
	  cpgsci(1);
	  cpgenv(0,numchannels,-180,180,0,0);
	  cpglab("Channel", "Phase (deg)", "");
	  cpgsci(2);
	  cpgsch(2);

	  for (k=0; k<npol; k++) {
	    cpgsci(colours[k]);
	    cpgpt(numchannels, xval, phase[k], 17);
	  }
	  cpgsch(1);

	  cpgend();

        }
      }
    }
  }

  int autocorrwidth = (config->getMaxProducts()>1)?2:1;
  for(int i=0;i<config->getNumDataStreams();i++) {
    for(int j=0;j<autocorrwidth;j++) {
      for(int k=0;k<config->getDNumOutputBands(currentconfigindex, i); k++) {

	if (j==0) {
	  sprintf(pgplotname, "lba-auto%d-b%d.png/png",
		  i, k);
	  status = cpgbeg(0,pgplotname,1,1);
	} else {
	  sprintf(pgplotname, "lba-autocross%d-b%d.png/png",
		  i, k);
	  status = cpgbeg(0,pgplotname,1,2);
	}
	if (status != 1) {
	  cout << "Error opening pgplot device: " << pgplotname << endl;
	} else {
	  float max, min;

	  cpgscr(0,1,1,1);
	  cpgscr(1,0,0,0);

	  // Plot Amplitude

	  status = vectorMagnitude_cf32(products[at], amplitude[0], numchannels);
	  if(status != vecNoErr) {
	    cerr << "Error trying to calculate amplitude - aborting!" << endl;
	    exit(1);
	  }

	  max = amplitude[0][0];
	  min = max;
	  for (int n=1; n<numchannels; n++) {
	    if (amplitude[0][n] > max) max = amplitude[0][n];
	    if (amplitude[0][n] < min) min = amplitude[0][n];
	  }
	  if (min==max) {
	    min-=1;
	    max+=1;
	  }

	  cpgsci(1);
	  cpgenv(0,numchannels+1,min,max,0,0);
	  cpglab("Channel", "Amplitude (Jy)", "");
	  cpgsci(2);
	  cpgline(numchannels, xval, amplitude[0]);

	  // Annotate
	  cpgsci(4);

	  if (j==0) {
	    cpgsch(1);
	  } else {
	    cpgsch(2);
	  }

	  ss << config->getDStationName(currentconfigindex, i) 
	     << "  " << sourcename; 
	  cpgmtxt("T", -1.5,0.02, 0, ss.str().c_str());	    
	  ss.str("");

	  int freqindex = config->getDFreqIndex(currentconfigindex, i, k);
	  ss << config->getFreqTableFreq(freqindex) << " MHz";
	  cpgmtxt("T", -2.6,0.98,1,ss.str().c_str());	    
	  ss.str("");

	  ss << timestr << "     " 
	     << config->getDBandPol(currentconfigindex, i, k);
	  if (j==0)
	    ss << config->getDBandPol(currentconfigindex, i, k);
	  else
	    ss << ((config->getDBandPol(currentconfigindex, i, k) == 'R')?'L':'R');
	  cpgmtxt("T", -1.5,0.98,1,ss.str().c_str());
	  ss.str("");

	  if (j!=0) {
	    status = vectorPhase_cf32(products[at], phase[0], numchannels);
	    if(status != vecNoErr) {
	      cerr << "Error trying to calculate phase - aborting!" << endl;
	      exit(1);
	    }
	    vectorMulC_f32_I(180/M_PI, phase[0], numchannels);

	    // Plot Phase
	    cpgsci(1);
	    cpgsch(1);
	    cpgenv(0,numchannels,-180,180,0,0);
	    cpglab("Channel", "Phase (deg)", "");
	    cpgsci(2);
	    cpgline(numchannels, xval, phase[0]);
	  }

	  cpgend();
	}
	at++;
      }
    }
  }
}

void change_config()
{
  int status, i;

  currentconfigindex = config->getConfigIndex(atseconds);
  numchannels = config->getNumChannels(currentconfigindex);

  cout << "New config " << currentconfigindex << " at " << atseconds << endl;

  if(xval) vectorFree(xval);
  if(fftspec) ippsFFTFree_R_32f(fftspec);

  for (i=0; i<MAXPOL; i++) {
    if(lags[i]) vectorFree(lags[i]);
    if(phase[i]) vectorFree(phase[i]);
    if(amplitude[i]) vectorFree(amplitude[i]);
  }

  if (products) {
    for (i=0; i<nprod; i++) {
      if (products[i]) vectorFree(products[i]);
    }
    delete [] products;
  }

  xval = vectorAlloc_f32(numchannels*2);
  for(int i=0;i<numchannels*2;i++)
    xval[i] = i;

  for (i=0; i<MAXPOL; i++) {
    lags[i] = vectorAlloc_f32(numchannels*2);
    phase[i] = vectorAlloc_f32(numchannels);
    amplitude[i] = vectorAlloc_f32(numchannels);
  }

  int order = 0;
  while(((numchannels*2) >> order) > 1)
    order++;
  ippsFFTInitAlloc_R_32f(&fftspec, order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast);

  nav = ceil(intseconds/config->getIntTime(currentconfigindex));
  cout << "#Integrations to average = " << nav << endl;

  resultlength = config->getResultLength(currentconfigindex);

  nprod = resultlength/(numchannels+1);
  cout << "Got " << nprod << " products" << endl;

  products = new cf32*[nprod];
  for (i=0; i<nprod; i++) {
    products[i] = vectorAlloc_cf32(numchannels);

    status = vectorZero_cf32(products[i], numchannels);
    if(status != vecNoErr) {
      cerr << "Error trying to zero visibility buffer - aborting!" << endl;
      exit(1);
    }
  }
}



void maxmin(f32 **vals, int nchan, int npol, float *max, float *min) {
  float delta;

  ippsMax_32f(vals[0], nchan, max);
  ippsMin_32f(vals[0], nchan, min);

  for (int i=1; i<npol; i++) {
    float thismax, thismin;

    ippsMax_32f(vals[i], nchan, &thismax);
    ippsMin_32f(vals[i], nchan, &thismin);
    if (thismax>*max) *max = thismax;
    if (thismin<*min) *min = thismin;
  }

  delta = (*max-*min)*0.05;
  if (delta==0) delta = 1;
  *min -= delta;
  *max += delta;

  return;
}
