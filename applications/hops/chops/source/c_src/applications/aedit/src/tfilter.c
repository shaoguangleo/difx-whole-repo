/************************************************************************/
/*                                                                      */
/* This routine determines the status of a data point with respect to   */
/* the input filter settings.  It has two modes.  If the mode is QUICK, */
/* the routine returns a non-zero value as soon as a filter is not      */
/* passed, without checking additional filters.  If the mode is SLOW,   */
/* all filters are checked, and the return value has the appropriate    */
/* bit set for each filter not passed.  Efficiency is achieved by first */
/* calling active_filter(), which allows filters in their pass-all      */
/* state to be completely bypassed.                                     */
/*                                                                      */
/*      Inputs:         datum           one element of data array       */
/*                      mode            QUICK or SLOW                   */
/*                      inp structure   User input settings             */
/*                      fcheck,nfilt    Active filter information       */
/*                                      (external from active_filter()) */
/*                                                                      */
/*      Output:         return value    0 ==> passed all filters        */
/*                                                                      */
/* Created 30 April 1990 by CJL                                         */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <string.h>
#include "aedata.h"
#include "aedit.h"
#include "flags.h"

#define QUICK 0
#define SLOW 1

int tfilter (trianglearray *tdatum, int mode)
    {
    extern struct inputs inp;
    extern int fcheck[12], nfilt;
    int i, ret, nf, frac, p0;
    float pval;
    trianglesum *datum;

    ret = 0;
    datum = &(tdatum->data);
    for (i=0;i<nfilt;i++) 
        {
        switch (fcheck[i]) {
            case F_TIMETAG:
                if (inp.begin == 0 && inp.end == 0) break;
                if (datum->time_tag < inp.begin || datum->time_tag > inp.end)
                                ret |= BADTIME;
                break;
            case F_STATION:
                if (strchr (inp.stations, datum->triangle[0]) == NULL ||
                        strchr (inp.stations, datum->triangle[1]) == NULL ||
                        strchr (inp.stations, datum->triangle[2]) == NULL)
                                ret |= BADSTATION;
                break;
            case F_TRIANGLE:
                if (! smatch (inp.triangles, datum->triangle)) ret |= BADTRNGL;
                break;
            case F_FREQUENCY:
                if (strchr (inp.frequencies, datum->freq_code) == NULL)
                                ret |= BADFREQA;
                break;
            case F_EXPERIMENT:
                if (inp.experiment != datum->expt_no) ret |= BADEXPT;
                break;
            case F_QCODE:
                if (strchr (inp.qcodes, datum->scan_quality) == NULL)
                                ret |= BADQF;
                break;
            case F_SOURCE:
                if (smatch (inp.sources, datum->source) != 2) ret |= BADSOURCE;
                break;
            case F_TYPE:
                if (strchr (inp.type, '3') == NULL) ret |= BADTYPE;
                break;
            case F_BSNR:
                if (inp.bsnr[0] > datum->bis_snr) ret |= BADBSNR;
                if (inp.bsnr[1] < datum->bis_snr) ret |= BADBSNR;
                break;
            case F_LENGTH:
                if (inp.length > datum->length[0]) ret |= BADLENGTH;
                if (inp.length > datum->length[1]) ret |= BADLENGTH;
                if (inp.length > datum->length[2]) ret |= BADLENGTH;
                break;
            case F_PROCDATE:
            case F_BASELINE:
            case F_QUAD:
            case F_SNR:
            case F_FRACTION:
            case F_NFREQ:
            case F_PARAMETER:
            case F_POLARIZATION:
                break;

            default:
                msg ("error in tfilter.c", 2);
                return (ERROR);
            }                           /* End switch */
        if (mode == QUICK && ret != 0) return (ret);
        }
    return (ret);
}
