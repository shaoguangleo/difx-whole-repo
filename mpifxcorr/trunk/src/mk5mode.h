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
//===========================================================================
// SVN properties (DO NOT CHANGE)
//
// $Id$
// $HeadURL: $
// $LastChangedRevision$
// $Author$
// $LastChangedDate$
//
//============================================================================
#include <mark5access.h>
#include "mode.h"

/** 
 @class Mk5Mode 
 @brief A mode for Mk4 formatted or VLBA formatted Mk5 data

 A mode for MkIV, VLBA, or Mark5B formatted Mk5 data.  All types of band setup should be supported.
 @author Adam Deller
 */
class Mk5Mode : public Mode
{
  public:
 /**
   * Constructor: calls Mode constructor then creates a mark5_format struct, using Walter Brisken's mark5access library
   * @param conf The configuration object, containing all information about the duration and setup of this correlation
   * @param confindex The index of the configuration this Mode is for
   * @param dsindex The index of the datastream this Mode is for
   * @param recordedbandchan The number of channels for each recorded subband
   * @param chanstoavg The number of channels to average for each subband
   * @param bpersend The number of FFT blocks to be processed in a message
   * @param gsamples The number of additional guard samples at the end of a message
   * @param nrecordedfreqs The number of recorded frequencies for this Mode
   * @param recordedbw The bandwidth of each of these IFs
   * @param recordedfreqclkoffs The time offsets in microseconds to be applied post-F for each of the frequencies
   * @param recordedfreqlooffs The LO offsets in Hz for each recorded frequency
   * @param nrecordedbands The total number of subbands recorded
   * @param nzoombands The number of subbands to be taken from within the recorded bands - can be zero
   * @param nbits The number of bits per sample
   * @param sampling The sampling type (REAL, COMPLEX)
   * @param fbank Whether to use a polyphase filterbank to channelise (instead of FFT)
   * @param linear2circular Whether to do a linear to circular conversion after the FFT
   * @param fringerotorder The interpolation order across an FFT (Oth, 1st or 2nd order; 0th = post-F)
   * @param arraystridelen The number of samples to stride when doing complex multiplies to implement sin/cos operations efficiently
   * @param cacorrs Whether cross-polarisation autocorrelations are to be calculated
   * @param framebytes The number of bytes in a frame
   * @param framesamples The number of samples in a frame per channel
   * @param format The data format type e.g. MARK5B, VDIF, VLBA etc
  */
  Mk5Mode(Configuration * conf, int confindex, int dsindex, int recordedbandchan, int chanstoavg, int bpersend, int gsamples, int nrecordedfreqs, double recordedbw, double * recordedfreqclkoffs, double * recordedfreqclkoffsdelta,double * recordedfreqphaseoffs, double * recordedfreqlooffs, int nrecordedbands, int nzoombands, int nbits, Configuration::datasampling sampling, bool fbank, bool linear2circular, int fringerotorder, int arraystridelen, bool cacorrs, int framebytes, int framesamples, Configuration::dataformat format);
    virtual ~Mk5Mode();

  protected:
 /** 
   * Uses mark5access library to unpack multiplexed, quantised data into the separate float arrays
   * @return The fraction of samples returned
   * @param sampleoffset The offset in number of time samples into the data array
  */
    virtual float unpack(int sampleoffset);

    int framesamples, framebytes, samplestounpack, fanout;
    struct mark5_stream *mark5stream;
};
// vim: shiftwidth=2:softtabstop=2:expandtab
