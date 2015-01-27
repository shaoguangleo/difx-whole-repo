#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <stdint.h>
#include <time.h>

#include "vdifio.h"

#define DOFLIP 0
/* Benchmark application to test, single threaded, speeds of extracting select channels from raw Xcube VLBI data.

The raw data consists of data from 4 IFs with 16 channels per IF (a total of 64 channels). The individual channels are 2bit complex (2 bit real, 2 bit imaginary so 4 bits/sample) - so there are 2 samples per byte. 

The raw data comes in 4 MByte chunks. These chucks are made up from 8192 byte packets, which in turn are made up of 8 bytes frames. The first frame of each packet is a header follower by 1023 data frames. The data frames are the 16x4bit time samples from a single IF. Each packet only consists of samples from a single IF. The data from the 4 IFs then come in sequence is alternate packets in the data stream.

The layout of the header frame is (as bytes)
    MSB                       LSB
    |      |      |      |      |
    |   Unused    | Type | IFid |
    |      Sequence Number      |

For simplicity, this program ignores the header and assumes it is correct 

The data needs to be written out in VDIF format - VDIF multipleces all the channels. If the simplest case of copying all 
channels this means taking a single 64bit word (frame) from 4 consesutive packets and writing them out into a VDIF packet, with is usually around 8kB in size

*/

#define MAX_PACKETSIZE 8200
#define BLOCKSIZE            4   // MB
#define NBLOCK              20
#define SAMPLEPERSEC       32e6  // 32 MHz channels
#define PACKETSIZE        1024   // Frames/packet - each frame is 8 bytes/64 bits
#define DEFAULT_LOOP        50
#define MAXSTR              256

#if DOFLIP
#define FLIP8(x)  ((((x)>>1)&0x5555555555555555) | (((x)<<1)&0xAAAAAAAAAAAAAAAA))
#define FLIPPAIR(x,y) ((x>>(y+1))&0x55) | ((x>>(y-1))&0xAA) 	  // This will fail if first channel selected
#else
#define FLIP8(x) x
#define FLIPPAIR(x,y) (x>>y)&0xFF;
#endif

double tim(void);
void printtime(char *type, double time, int nchan, uint64_t totalbytes);
void extractAll(int nloop, int nblock, int bufsize, uint64_t **data);
void extractGeneric(uint64_t chans, int nloop, int nblock, int bufsize, uint64_t **data);
void extract2chanPair(int nloop, int nblock, int bufsize, uint64_t **data);
void extract4chanPair(int nloop, int nblock, int bufsize, uint64_t **data);
void extract4chan(int nloop, int nblock, int bufsize, uint64_t **data);
void extract4chanAlt(int nloop, int nblock, int bufsize, uint64_t **data);
void extract8chan(int nloop, int nblock, int bufsize, uint64_t **data);

// Some global variables to avoid passing lots of parameters
int main (int argc, char * const argv[]) {
  uint64_t **data;
  int i, j, opt, status, tmp, packetsperblock;
  long int *lint;
  int nblock = NBLOCK;

  struct option options[] = {
    {"nblock", 1, 0, 'N'},
    {"nloop", 1, 0, 'n'},
    {"help", 0, 0, 'h'},
    {0, 0, 0, 0}
  };
  
  int bufsize = BLOCKSIZE * 1024*1024;
  int nloop = DEFAULT_LOOP;

  packetsperblock = BLOCKSIZE*1024*1024/(PACKETSIZE*8);
  bufsize = packetsperblock*PACKETSIZE*8;

  /* Read command line options */
  while (1) {
    opt = getopt_long_only(argc, argv, "m:n:N:", 
			   options, NULL);
    if (opt==EOF) break;

    switch (opt) {
      
    case 'N':
      status = sscanf(optarg, "%d", &tmp);
      if (status!=1)
	fprintf(stderr, "Bad nblock option %s\n", optarg);
      else 
	nblock = tmp;
      break;

    case 'n':
      status = sscanf(optarg, "%d", &tmp);
      if (status!=1)
	fprintf(stderr, "Bad nloop option %s\n", optarg);
      else 
	nloop = tmp;
     break;

    case 'h':
      printf("No help here\n");
      return(1);
      break;
    
    case '?':
    default:
      break;
    }
  }

  // Allocate amd initlalize memory
  data = malloc(sizeof(uint64_t*)*nblock);
  if (data==NULL) {
    perror("Trying to allocate memory");
    exit(EXIT_FAILURE);
  }
  srandom((unsigned int)time(NULL));
  for (i=0;i<nblock;i++) {
    data[i] = malloc(bufsize);
    if (data[i]==NULL) {
      perror("Trying to allocate memory");
      exit(EXIT_FAILURE);
    }
    lint = (long int*)data[i];
    for (j=0; j<bufsize/sizeof(long int); j++) {
      *lint = random();
      lint++;
    }
  }

  extractAll(nloop, nblock, bufsize, data);

  // All channels
  extractGeneric(0xFFFFFFFFFFFFFFFF, nloop/10, nblock, bufsize, data);

  // Every second
  extractGeneric(0x5555555555555555, nloop/10, nblock, bufsize, data);
  
  // 2 per IF
  extract2chanPair(nloop, nblock, bufsize, data);

  // 4 per IF
  extract4chanPair(nloop, nblock, bufsize, data);

  // 4 per IF, not paired
  extract4chan(nloop, nblock, bufsize, data);

  // 4 per IF, not paired
  extract4chanAlt(nloop, nblock, bufsize, data);

  // 8 per IF, not paired
  extract8chan(nloop, nblock, bufsize, data);

  for (i=0;i<nblock;i++) {
    free(data[i]);
  }  
  free(data);

  return(0);
}

double tim(void) {
  struct timeval tv;
  double t;

  gettimeofday(&tv, NULL);
  t = (double)tv.tv_sec + (double)tv.tv_usec/1000000.0;

  return t;
}

void extractAll(int nloop, int nblock, int bufsize, uint64_t **data) {
  /* This funtion essentially just takes the 4 data streams and interleaves them. As the 16 channels per 
     IF fill a single 64 bit word, this is most easily acheived ust shuffeling 64bit integers around */
  int i, j, k, l, vdifwords;
  uint64_t *if1, *if2, *if3, *if4, *vdifdata;
  double t0, t1;
  vdif_header header;
  int vdif_packetsize;
  int framespersec;

  int nchan = 64;

  uint64_t rate = nchan*SAMPLEPERSEC*4/8; // Bytes/sec excluding header (4 bit/sample as complex)
  vdif_packetsize = MAX_PACKETSIZE;
  vdif_packetsize = (vdif_packetsize/8)*8; // Round down to multiple of 8 bytes
  while (vdif_packetsize>0) {
    if ((rate % vdif_packetsize) == 0) {
      printf("Choosing VDIf framesize of = %d\n", vdif_packetsize);
      break;
    }
    vdif_packetsize-=8;
  }
  if (vdif_packetsize<=0) {
    printf("Could not find appropriate VDIF header size for data rate %.2f Mbytes/sec\n", rate/1e6);
    exit(EXIT_FAILURE);
  }

  vdifdata = malloc(vdif_packetsize);
  if (vdifdata==NULL) {
    perror("Allocating memory");
    exit(EXIT_FAILURE);
  }
  
  int samplesperframe = (vdif_packetsize*8)/(nchan*4);  // 4 bits/sample
  framespersec = SAMPLEPERSEC/samplesperframe;
  printf("Frame/sec = %d\n", framespersec);

  createVDIFHeader(&header, vdif_packetsize+VDIF_HEADER_BYTES, 0,  2, nchan, 1, "Tt");
  time_t t = time(NULL);
  setVDIFEpochTime(&header, t);
  setVDIFFrameTime(&header, t);
  
  t0 = tim();
  vdifwords = 0;
  for (i=0; i<nloop; i++) {
    for (j=0; j<nblock; j++) {
      if1 = (&data[j][0]) - 4096;
      if2 = if1+1024;
      if3 = if2+1024;
      if4 = if3+1024;

      for (k=0; k<bufsize/(8192*4); k++) {
	if1 += 4096;
	if2 += 4096;
	if3 += 4096;
	if4 += 4096;

	for (l=1; l<1024; l++) {
	  vdifdata[vdifwords] = FLIP8(if1[l]);
	  vdifwords++;
	  vdifdata[vdifwords] = FLIP8(if2[l]);
	  vdifwords++;
	  vdifdata[vdifwords] = FLIP8(if3[l]);
	  vdifwords++;
	  vdifdata[vdifwords] = FLIP8(if4[l]);
	  vdifwords++;

	  if (vdifwords>= vdif_packetsize/8) {
	    // Would write VDIF packet here
	    vdifwords = 0;
	    nextVDIFHeader(&header, framespersec);
	  }
	}
      }
    }
  }
  t1 = tim();

  printtime("Extracting all", t1-t0, nchan, (uint64_t)nloop*nblock*bufsize);

  free(vdifdata);
}

void printtime(char *type, double time, int nchan, uint64_t totalbytes) {

  printf("%s took %.1f sec (%.1f Gbits/sec input, %.1f Gbits/sec output\n\n", 
	 type, time, (totalbytes/time)*8/1e9, totalbytes/time*8/1e9*nchan/64);
}

void extractGeneric(uint64_t chans, int nloop, int nblock, int bufsize, uint64_t **data) {
  /* This funtion allows for arbitary selection of channels from the 4 IFs. The total number of channels must be be even */

  int c, i, j, k, l, m, n, vdifNybles, shift[64], mask[64];
  uint64_t *IF[4];
  uint8_t *vdifdata;
  double t0, t1;
  vdif_header header;
  int vdif_packetsize;
  int framespersec;

  printf("Generic extraction of 0x%16llX\n", (unsigned long long)chans);
  int nchan = 0;
  for (i=0; i<64; i++) {
    if (chans&((uint64_t)1<<i)) {
      if (nchan%2) { 
	// Will end up as high nyble
	mask[i] = 0xF0;
	shift[i] = (i-1)*4;  // i==0 can never get here
      } else {
	mask[i] = 0x0F;
	shift[i] = i*4;
      }
      nchan++;
    }
  }
  printf("Generic selecting %d channels\n", nchan);
  if (nchan%2) {
    printf("Can only extract an even number of channels. Tried to select %d\n", nchan);
    exit(EXIT_FAILURE);
  }

  uint64_t rate = nchan*SAMPLEPERSEC*4/8; // Bytes/sec excluding header (4 bit/sample as complex)
  vdif_packetsize = MAX_PACKETSIZE;
  vdif_packetsize = (vdif_packetsize/8)*8; // Round down to multiple of 8 bytes
  while (vdif_packetsize>0) {
    if ((rate % vdif_packetsize) == 0) {
      printf("Choosing VDIf framesize of = %d\n", vdif_packetsize);
      break;
    }
    vdif_packetsize-=8;
  }
  if (vdif_packetsize<=0) {
    printf("Could not find appropriate VDIF header size for data rate %.2f Mbytes/sec\n", rate/1e6);
    exit(EXIT_FAILURE);
  }

  vdifdata = malloc(vdif_packetsize);
  if (vdifdata==NULL) {
    perror("Allocating memory");
    exit(EXIT_FAILURE);
  }
  
  int samplesperframe = (vdif_packetsize*8)/(nchan*4);  // 4 bits/sample
  framespersec = SAMPLEPERSEC/samplesperframe;
  printf("Frame/sec = %d\n", framespersec);

  createVDIFHeader(&header, vdif_packetsize+VDIF_HEADER_BYTES, 0,  2, nchan, 1, "Tt");
  time_t t = time(NULL);
  setVDIFEpochTime(&header, t);
  setVDIFFrameTime(&header, t);
  
  t0 = tim();
  vdifNybles = 0;
  bzero(vdifdata, vdif_packetsize);
  for (i=0; i<nloop; i++) {
    for (j=0; j<nblock; j++) {
      IF[0] = (&data[j][0]) - 4096;
      IF[1] = IF[0]+1024;
      IF[2] = IF[1]+1024;
      IF[3] = IF[2]+1024;

      for (k=0; k<bufsize/(8192*4); k++) {
	IF[0] += 4096;
	IF[1] += 4096;
	IF[2] += 4096;
	IF[3] += 4096;

	for (l=1; l<1024; l++) {
	  c = 0; // Channel number 0..63
	  for (m=0; m<4; m++) {
	    for (n=0; n<16; n++) {
	      if (chans&(1<<c)) { // Is this channel selected
		vdifdata[vdifNybles/2] |= (IF[m][l]>>shift[c])&mask[c];
		vdifNybles++;
	      }
	      c++;
	    }
	  }

	  if (vdifNybles>= vdif_packetsize*2) {
	    // Would write VDIF packet here
	    vdifNybles = 0;
	    bzero(vdifdata, vdif_packetsize);
	    nextVDIFHeader(&header, framespersec);
	  }
	}
      }
    }
  }
  t1 = tim();

  printtime("Extracting Generic", t1-t0, nchan, (uint64_t)nloop*nblock*bufsize);

  free(vdifdata);
}

void extract2chanPair(int nloop, int nblock, int bufsize, uint64_t **data) {
  /* This funtion just extracts 2 channels per IF, but does all IFs. Essentially it is 0x00C000C000C000C0
     Because it is working on pairs of channels, it is based around bytes, not words. */

  int i, j, k, l, vdifbytes;
  uint64_t *if1, *if2, *if3, *if4;
  uint8_t *vdifdata;
  double t0, t1;
  vdif_header header;
  int vdif_packetsize;
  int framespersec;

  int nchan = 2*4;
  int shift[4] = {24, 32, 24, 32};

  uint64_t rate = nchan*SAMPLEPERSEC*4/8; // Bytes/sec excluding header (4 bit/sample as complex)
  vdif_packetsize = MAX_PACKETSIZE;
  vdif_packetsize = (vdif_packetsize/8)*8; // Round down to multiple of 8 bytes
  while (vdif_packetsize>0) {
    if ((rate % vdif_packetsize) == 0) {
      printf("Choosing VDIf framesize of = %d\n", vdif_packetsize);
      break;
    }
    vdif_packetsize-=8;
  }
  if (vdif_packetsize<=0) {
    printf("Could not find appropriate VDIF header size for data rate %.2f Mbytes/sec\n", rate/1e6);
    exit(EXIT_FAILURE);
  }

  vdifdata = malloc(vdif_packetsize);
  if (vdifdata==NULL) {
    perror("Allocating memory");
    exit(EXIT_FAILURE);
  }
  
  int samplesperframe = (vdif_packetsize*8)/(nchan*4);  // 4 bits/sample
  framespersec = SAMPLEPERSEC/samplesperframe;
  printf("Frame/sec = %d\n", framespersec);

  createVDIFHeader(&header, vdif_packetsize+VDIF_HEADER_BYTES, 0,  2, nchan, 1, "Tt");
  time_t t = time(NULL);
  setVDIFEpochTime(&header, t);
  setVDIFFrameTime(&header, t);
  
  t0 = tim();
  vdifbytes = 0;
  for (i=0; i<nloop; i++) {
    for (j=0; j<nblock; j++) {
      if1 = (&data[j][0]) - 4096;
      if2 = if1+1024;
      if3 = if2+1024;
      if4 = if3+1024;

      for (k=0; k<bufsize/(8192*4); k++) {
	if1 += 4096;
	if2 += 4096;
	if3 += 4096;
	if4 += 4096;

	for (l=1; l<1024; l++) {
	  vdifdata[vdifbytes] = FLIPPAIR(if1[l], shift[0]);
	  vdifbytes++;
	  vdifdata[vdifbytes] = FLIPPAIR(if2[l], shift[1]);
	  vdifbytes++;
	  vdifdata[vdifbytes] = FLIPPAIR(if3[l], shift[2]);
	  vdifbytes++;
	  vdifdata[vdifbytes] = FLIPPAIR(if4[l], shift[3]);
	  vdifbytes++;

	  if (vdifbytes>= vdif_packetsize) {
	    // Would write VDIF packet here
	    vdifbytes = 0;
	    nextVDIFHeader(&header, framespersec);
	  }
	}
      }
    }
  }
  t1 = tim();

  printtime("Extracting 2 channels (pairs) per IF", t1-t0, nchan, (uint64_t)nloop*nblock*bufsize);

  free(vdifdata);
}

void extract4chanPair(int nloop, int nblock, int bufsize, uint64_t **data) {
  /* This funtion just extracts 4 channels per IF, in 2 pairs.
     Because it is working on pairs of channels, it is based around bytes, not words. */

  int i, j, k, l, vdifbytes;
  uint64_t *if1, *if2, *if3, *if4;
  uint8_t *vdifdata;
  double t0, t1;
  vdif_header header;
  int vdif_packetsize;
  int framespersec;

  int nchan = 4*4;
  int shift1[4] = {16, 24, 16, 24};
  int shift2[4] = {32, 48, 32, 48};

  uint64_t rate = nchan*SAMPLEPERSEC*4/8; // Bytes/sec excluding header (4 bit/sample as complex)
  vdif_packetsize = MAX_PACKETSIZE;
  vdif_packetsize = (vdif_packetsize/8)*8; // Round down to multiple of 8 bytes
  while (vdif_packetsize>0) {
    if ((rate % vdif_packetsize) == 0) {
      printf("Choosing VDIf framesize of = %d\n", vdif_packetsize);
      break;
    }
    vdif_packetsize-=8;
  }
  if (vdif_packetsize<=0) {
    printf("Could not find appropriate VDIF header size for data rate %.2f Mbytes/sec\n", rate/1e6);
    exit(EXIT_FAILURE);
  }

  vdifdata = malloc(vdif_packetsize);
  if (vdifdata==NULL) {
    perror("Allocating memory");
    exit(EXIT_FAILURE);
  }
  
  int samplesperframe = (vdif_packetsize*8)/(nchan*4);  // 4 bits/sample
  framespersec = SAMPLEPERSEC/samplesperframe;
  printf("Frame/sec = %d\n", framespersec);

  createVDIFHeader(&header, vdif_packetsize+VDIF_HEADER_BYTES, 0,  2, nchan, 1, "Tt");
  time_t t = time(NULL);
  setVDIFEpochTime(&header, t);
  setVDIFFrameTime(&header, t);
  
  t0 = tim();
  vdifbytes = 0;
  for (i=0; i<nloop; i++) {
    for (j=0; j<nblock; j++) {
      if1 = (&data[j][0]) - 4096;
      if2 = if1+1024;
      if3 = if2+1024;
      if4 = if3+1024;

      for (k=0; k<bufsize/(8192*4); k++) {
	if1 += 4096;
	if2 += 4096;
	if3 += 4096;
	if4 += 4096;

	for (l=1; l<1024; l++) {
	  vdifdata[vdifbytes] = (if1[l]>>shift1[0])&0xFF;
	  vdifbytes++;
	  vdifdata[vdifbytes] = (if1[l]>>shift2[0])&0xFF;
	  vdifbytes++;
	  vdifdata[vdifbytes] = (if2[l]>>shift1[1])&0xFF;
	  vdifbytes++;
	  vdifdata[vdifbytes] = (if2[l]>>shift2[1])&0xFF;
	  vdifbytes++;
	  vdifdata[vdifbytes] = (if3[l]>>shift1[2])&0xFF;
	  vdifbytes++;
	  vdifdata[vdifbytes] = (if3[l]>>shift2[2])&0xFF;
	  vdifbytes++;
	  vdifdata[vdifbytes] = (if4[l]>>shift1[3])&0xFF;
	  vdifbytes++;
	  vdifdata[vdifbytes] = (if4[l]>>shift2[3])&0xFF;
	  vdifbytes++;

	  if (vdifbytes>= vdif_packetsize) {
	    // Would write VDIF packet here
	    vdifbytes = 0;
	    nextVDIFHeader(&header, framespersec);
	  }
	}
      }
    }
  }
  t1 = tim();

  printtime("Extracting 4 channels (pair) per IF", t1-t0, nchan, (uint64_t)nloop*nblock*bufsize);

  free(vdifdata);
}

void extract4chan(int nloop, int nblock, int bufsize, uint64_t **data) {
  /* This function extracts 4 arbitary channels per IF */

  int i, j, k, l, vdifbytes;
  uint64_t *if1, *if2, *if3, *if4;
  uint8_t *vdifdata;
  double t0, t1;
  vdif_header header;
  int vdif_packetsize;
  int framespersec;

  int nchan = 4*4;
  int shift1[4] = {4, 16, 32, 48};
  int shift2[4] = {8, 12, 24, 32};
  int shift3[4] = {4, 16, 32, 48};
  int shift4[4] = {8, 12, 24, 32};

  uint64_t rate = nchan*SAMPLEPERSEC*4/8; // Bytes/sec excluding header (4 bit/sample as complex)
  vdif_packetsize = MAX_PACKETSIZE;
  vdif_packetsize = (vdif_packetsize/8)*8; // Round down to multiple of 8 bytes
  while (vdif_packetsize>0) {
    if ((rate % vdif_packetsize) == 0) {
      printf("Choosing VDIf framesize of = %d\n", vdif_packetsize);
      break;
    }
    vdif_packetsize-=8;
  }
  if (vdif_packetsize<=0) {
    printf("Could not find appropriate VDIF header size for data rate %.2f Mbytes/sec\n", rate/1e6);
    exit(EXIT_FAILURE);
  }

  vdifdata = malloc(vdif_packetsize);
  if (vdifdata==NULL) {
    perror("Allocating memory");
    exit(EXIT_FAILURE);
  }
  
  int samplesperframe = (vdif_packetsize*8)/(nchan*4);  // 4 bits/sample
  framespersec = SAMPLEPERSEC/samplesperframe;
  printf("Frame/sec = %d\n", framespersec);

  createVDIFHeader(&header, vdif_packetsize+VDIF_HEADER_BYTES, 0,  2, nchan, 1, "Tt");
  time_t t = time(NULL);
  setVDIFEpochTime(&header, t);
  setVDIFFrameTime(&header, t);
  
  t0 = tim();
  vdifbytes = 0;
  for (i=0; i<nloop; i++) {
    for (j=0; j<nblock; j++) {
      if1 = (&data[j][0]) - 4096;
      if2 = if1+1024;
      if3 = if2+1024;
      if4 = if3+1024;

      for (k=0; k<bufsize/(8192*4); k++) {
	if1 += 4096;
	if2 += 4096;
	if3 += 4096;
	if4 += 4096;

	for (l=1; l<1024; l++) {
	  vdifdata[vdifbytes] = (if1[l]>>shift1[0])&0x0F;
	  vdifdata[vdifbytes] |= (if1[l]>>shift1[1])&0xF0;
	  vdifbytes++;
	  vdifdata[vdifbytes] = (if1[l]>>shift1[2])&0x0F;
	  vdifdata[vdifbytes] |= (if1[l]>>shift1[3])&0xF0;
	  vdifbytes++;

	  vdifdata[vdifbytes] = (if2[l]>>shift2[0])&0x0F;
	  vdifdata[vdifbytes] |= (if2[l]>>shift2[1])&0xF0;
	  vdifbytes++;
	  vdifdata[vdifbytes] = (if2[l]>>shift2[2])&0x0F;
	  vdifdata[vdifbytes] |= (if2[l]>>shift2[3])&0xF0;
	  vdifbytes++;

	  vdifdata[vdifbytes] = (if3[l]>>shift3[0])&0x0F;
	  vdifdata[vdifbytes] |= (if3[l]>>shift3[1])&0xF0;
	  vdifbytes++;
	  vdifdata[vdifbytes] = (if3[l]>>shift3[2])&0x0F;
	  vdifdata[vdifbytes] |= (if3[l]>>shift3[3])&0xF0;
	  vdifbytes++;

	  vdifdata[vdifbytes] = (if4[l]>>shift4[0])&0x0F;
	  vdifdata[vdifbytes] |= (if4[l]>>shift4[1])&0xF0;
	  vdifbytes++;
	  vdifdata[vdifbytes] = (if4[l]>>shift4[2])&0x0F;
	  vdifdata[vdifbytes] |= (if4[l]>>shift4[3])&0xF0;
	  vdifbytes++;

	  if (vdifbytes>= vdif_packetsize) {
	    // Would write VDIF packet here
	    vdifbytes = 0;
	    nextVDIFHeader(&header, framespersec);
	  }
	}
      }
    }
  }
  t1 = tim();

  printtime("Extracting 4 arbitary channels per IF", t1-t0, nchan, (uint64_t)nloop*nblock*bufsize);

  free(vdifdata);
}

void extract4chanAlt(int nloop, int nblock, int bufsize, uint64_t **data) {
  /* This function extracts 4 arbitary channels per IF */

  int i, j, k, l, vdifwords;
  uint64_t *if1, *if2, *if3, *if4;
  uint16_t *vdifdata;
  double t0, t1;
  vdif_header header;
  int vdif_packetsize;
  int framespersec;

  int nchan = 4*4;
  int shift1[4] = {4, 16, 16, 32};
  int shift2[4] = {8, 12,  8, 16};
  int shift3[4] = {4, 16, 16, 32};
  int shift4[4] = {8, 12,  8, 16};

  uint64_t rate = nchan*SAMPLEPERSEC*4/8; // Bytes/sec excluding header (4 bit/sample as complex)
  vdif_packetsize = MAX_PACKETSIZE;
  vdif_packetsize = (vdif_packetsize/8)*8; // Round down to multiple of 8 bytes
  while (vdif_packetsize>0) {
    if ((rate % vdif_packetsize) == 0) {
      printf("Choosing VDIf framesize of = %d\n", vdif_packetsize);
      break;
    }
    vdif_packetsize-=8;
  }
  if (vdif_packetsize<=0) {
    printf("Could not find appropriate VDIF header size for data rate %.2f Mbytes/sec\n", rate/1e6);
    exit(EXIT_FAILURE);
  }

  vdifdata = malloc(vdif_packetsize);
  if (vdifdata==NULL) {
    perror("Allocating memory");
    exit(EXIT_FAILURE);
  }
  
  int samplesperframe = (vdif_packetsize*8)/(nchan*4);  // 4 bits/sample
  framespersec = SAMPLEPERSEC/samplesperframe;
  printf("Frame/sec = %d\n", framespersec);

  createVDIFHeader(&header, vdif_packetsize+VDIF_HEADER_BYTES, 0,  2, nchan, 1, "Tt");
  time_t t = time(NULL);
  setVDIFEpochTime(&header, t);
  setVDIFFrameTime(&header, t);
  
  t0 = tim();
  vdifwords = 0;
  for (i=0; i<nloop; i++) {
    for (j=0; j<nblock; j++) {
      if1 = (&data[j][0]) - 4096;
      if2 = if1+1024;
      if3 = if2+1024;
      if4 = if3+1024;

      for (k=0; k<bufsize/(8192*4); k++) {
	if1 += 4096;
	if2 += 4096;
	if3 += 4096;
	if4 += 4096;

	for (l=1; l<1024; l++) {
	  vdifdata[vdifwords] = ((if1[l]>>shift1[0])&0x000F) | ((if1[l]>>shift1[1])&0x00F0) |
	                        ((if1[l]>>shift1[2])&0x0F00) | ((if1[l]>>shift1[3])&0xF000);
	  vdifwords++;
	  vdifdata[vdifwords] = ((if2[l]>>shift2[0])&0x000F) | ((if2[l]>>shift2[1])&0x00F0) |
	                        ((if2[l]>>shift2[2])&0x0F00) | ((if2[l]>>shift2[3])&0xF000);
	  vdifwords++;
	  vdifdata[vdifwords] = ((if3[l]>>shift3[0])&0x000F) | ((if3[l]>>shift3[1])&0x00F0) |
	                        ((if3[l]>>shift3[2])&0x0F00) | ((if3[l]>>shift3[3])&0xF000);
	  vdifwords++;
	  vdifdata[vdifwords] = ((if4[l]>>shift4[0])&0x000F) | ((if4[l]>>shift4[1])&0x00F0) |
	                        ((if4[l]>>shift4[2])&0x0F00) | ((if4[l]>>shift4[3])&0xF000);
	  vdifwords++;

	  if (vdifwords>= vdif_packetsize/2) {
	    // Would write VDIF packet here
	    vdifwords = 0;
	    nextVDIFHeader(&header, framespersec);
	  }
	}
      }
    }
  }
  t1 = tim();

  printtime("Extracting 4 arbitary channels per IF", t1-t0, nchan, (uint64_t)nloop*nblock*bufsize);

  free(vdifdata);
}



void extract8chan(int nloop, int nblock, int bufsize, uint64_t **data) {
  /* This function extracts 4 arbitary channels per IF */

  int i, j, k, l, m, vdifbytes;
  uint64_t *if1, *if2, *if3, *if4;
  uint8_t *vdifdata;
  double t0, t1;
  vdif_header header;
  int vdif_packetsize;
  int framespersec;

  int nchan = 4*8;
  int shift1[8] = {1,2,5,7,9,10,13,15}; // Actually channel numbers, 1 based
  int shift2[8] = {2,5,6,7,8,11,13,15};
  int shift3[8] = {1,2,5,7,9,10,13,15};
  int shift4[8] = {2,5,6,7,8,11,13,15};

  // Convert to bit shifts
  for (j=0; j<8; j++) {
    shift1[j]--; // Convert to zero based
    shift2[j]--; // Convert to zero based
    shift3[j]--; // Convert to zero based
    shift4[j]--; // Convert to zero based
    if (j%2) { // Will end up as high nyble
      shift1[j]--; // Shift is one less. This assume channel 1 is not the second selected channel
      shift2[j]--; // Shift is one less. This assume channel 1 is not the second selected channel
      shift3[j]--; // Shift is one less. This assume channel 1 is not the second selected channel
      shift4[j]--; // Shift is one less. This assume channel 1 is not the second selected channel
    }
    shift1[j] *= 4;
    shift2[j] *= 4;
    shift3[j] *= 4;
    shift4[j] *= 4;
  }

  uint64_t rate = nchan*SAMPLEPERSEC*4/8; // Bytes/sec excluding header (4 bit/sample as complex)
  vdif_packetsize = MAX_PACKETSIZE;
  vdif_packetsize = (vdif_packetsize/8)*8; // Round down to multiple of 8 bytes
  while (vdif_packetsize>0) {
    if ((rate % vdif_packetsize) == 0) {
      printf("Choosing VDIf framesize of = %d\n", vdif_packetsize);
      break;
    }
    vdif_packetsize-=8;
  }
  if (vdif_packetsize<=0) {
    printf("Could not find appropriate VDIF header size for data rate %.2f Mbytes/sec\n", rate/1e6);
    exit(EXIT_FAILURE);
  }

  vdifdata = malloc(vdif_packetsize);
  if (vdifdata==NULL) {
    perror("Allocating memory");
    exit(EXIT_FAILURE);
  }
  
  int samplesperframe = (vdif_packetsize*8)/(nchan*4);  // 4 bits/sample
  framespersec = SAMPLEPERSEC/samplesperframe;
  printf("Frame/sec = %d\n", framespersec);

  createVDIFHeader(&header, vdif_packetsize+VDIF_HEADER_BYTES, 0,  2, nchan, 1, "Tt");
  time_t t = time(NULL);
  setVDIFEpochTime(&header, t);
  setVDIFFrameTime(&header, t);
  
  t0 = tim();
  vdifbytes = 0;
  for (i=0; i<nloop; i++) {
    for (j=0; j<nblock; j++) {
      if1 = (&data[j][0]) - 4096;
      if2 = if1+1024;
      if3 = if2+1024;
      if4 = if3+1024;

      for (k=0; k<bufsize/(8192*4); k++) {
	if1 += 4096;
	if2 += 4096;
	if3 += 4096;
	if4 += 4096;

	for (l=1; l<1024; l++) {
	  for (m=0; m<4; m++) {
	    vdifdata[vdifbytes] = (if1[l]>>shift1[m*2])&0xFF;
	    vdifdata[vdifbytes] |= (if1[l]>>shift1[m*2+1])&0xFF00;
	    vdifbytes++;
	  }
	  for (m=0; m<4; m++) {
	    vdifdata[vdifbytes] = (if2[l]>>shift1[m*2])&0xFF;
	    vdifdata[vdifbytes] |= (if2[l]>>shift1[m*2+1])&0xFF00;
	    vdifbytes++;
	  }
	  for (m=0; m<4; m++) {
	    vdifdata[vdifbytes] = (if3[l]>>shift1[m*2])&0xFF;
	    vdifdata[vdifbytes] |= (if3[l]>>shift1[m*2+1])&0xFF00;
	    vdifbytes++;
	  }
	  for (m=0; m<4; m++) {
	    vdifdata[vdifbytes] = (if4[l]>>shift1[m*2])&0xFF;
	    vdifdata[vdifbytes] |= (if4[l]>>shift1[m*2+1])&0xFF00;
	    vdifbytes++;
	  }

	  if (vdifbytes>= vdif_packetsize) {
	    // Would write VDIF packet here
	    vdifbytes = 0;
	    nextVDIFHeader(&header, framespersec);
	  }
	}
      }
    }
  }
  t1 = tim();

  printtime("Extracting 8 arbitary channels per IF", t1-t0, nchan, (uint64_t)nloop*nblock*bufsize);

  free(vdifdata);
}
