/*
 * apply notches to the cross power spectrum, if specified
 *
 * for clarity of code, lsb and usb are separated into two routines
 * this is all similar to apply_passband, except there is only
 * exclusion, not inclusion.
 *
 * None of this code is called if datum->?sbfrac < 0
 * A confusing element here is npts is 2*nlags and the data
 * is really only present in 0..npts/4 or 0..npts/2 depending on
 * param->corr_type = DIFX or param->corr_type = MK4HDW
 */

#include "mk4_data.h"
#include "param_struct.h"
#include "pass_struct.h"
#include <stdio.h>
#include <stdlib.h>

extern void   msg (char *, int, ...);
extern struct type_param param;

void apply_lsb_notch(double lo, double hi,
                     struct freq_corel *fdata,
                     int *xp_flags,
                     int npts, int ap)
    {
    int ii, ibot, itop, nuke = 0, dv;
    double bw, bottom, top;

    dv = (param.corr_type == DIFX) ? 4 : 2 ;

    bw = 0.5e-6 / param.samp_period;
    top = fdata->frequency;
    bottom = top - bw;

    if (hi < bottom)
        ibot = npts + 1;
    else if (hi < top)
        ibot = (1.0 - (hi - bottom) / bw) * npts /dv + 0.5;
    else
        ibot = -1;

    if (lo < bottom)
        itop = npts + 1;
    else if (lo < top)
        itop = (1.0 - (lo - bottom) / bw) * npts /dv + 0.5;
    else
        itop = -1;

    for (ii = 0; ii < npts/dv; ii++)
        if (ii > ibot && ii < itop)
            nuke += (xp_flags[ii] = 1);   /* marking it to be nuked */

    msg ("notch indicies %3d %3d %3d/%-3d / %g..%g / top %g bot %g bw %g",
        1, ibot,itop,nuke,npts, lo,hi,  top, bottom, bw);
    }

void apply_usb_notch(double lo, double hi,
                     struct freq_corel *fdata,
                     int *xp_flags,
                     int npts, int ap)
    {
    int ii, ibot, itop, nuke = 0, dv;
    double bw, bottom, top;

    dv = (param.corr_type == DIFX) ? 4 : 2 ;

    bw = 0.5e-6 / param.samp_period;
    bottom = fdata->frequency;
    top = bottom + bw;

    if (lo < bottom)
        ibot = -1;
    else if (lo < top)
        ibot = (lo - bottom) / bw * npts /dv + 0.5;
    else
        ibot = npts + 1;

    if (hi < bottom)
        itop = -1;
    else if (hi < top)
        itop = (hi - bottom) / bw * npts /dv + 0.5;
    else
        itop = npts + 1;

    for (ii = 0; ii < npts/dv; ii++)
        if (ii > ibot && ii < itop)
            nuke += (xp_flags[ii] = 1);   /* marking it to be nuked */

    msg ("notch indicies %3d %3d %3d/%-3d / %g..%g / top %g bot %g bw %g",
        1, ibot,itop,nuke,npts, lo,hi,  top, bottom, bw);
    }

/*
 * Mark the bits to nuke in xp_flags, and then do the deed as
 * the with passband exclusion case.
 */
void apply_notches(int sb, int ap,
                   struct freq_corel *fdata,
                   complex *xp_spectrum,
                   int npts,
                   struct data_corel *datum)
    {
    int ii, nuked = 0, dv;
    double lo, hi, factor, oldfrac, newfrac, edge;
    int *xp_flags;

    if (param.nnotches <= 0) return;
    if (!(xp_flags = calloc(npts, sizeof(int))))
        {
        perror("apply_notches:calloc");
        msg("Calloc failure in apply notches", 3);
        return;
        }

    dv = (param.corr_type == DIFX) ? 4 : 2 ;

    edge = fdata->frequency + (sb ? -1.0 : 1.0) * 0.5e-6 / param.samp_period;
    msg ("Ap %d: applying %d notches to %s channel %lf..%lf",
        1,ap,param.nnotches,(sb?"lsb":"usb"),fdata->frequency, edge);
    for (ii = 0; ii < param.nnotches; ii++)
        {
        lo = param.notches[ii][0];
        hi = param.notches[ii][1];
        if (hi <= lo)
            {
            msg("Ignoring illegal notch(%d): %g <= %g",2,ii,hi,lo);
            continue;
            }
        if (sb) apply_lsb_notch(lo, hi, fdata, xp_flags, npts, ap);
        else    apply_usb_notch(lo, hi, fdata, xp_flags, npts, ap);
        }

    /* now nuke */
    for (ii = 0, nuked = 0; ii < npts/dv; ii++)
        if (xp_flags[ii])
        {
            xp_spectrum[ii] = 0.0;
            nuked ++;
        }

    /* now adjust and report */
    factor = (nuked >= npts/dv)
           ? 0.0 : (double)(npts/dv)/(double)(npts/dv - nuked);
    if (sb)
        {
        oldfrac = datum->lsbfrac;
        newfrac = (datum->lsbfrac *= factor);
        }
    else
        {
        oldfrac = datum->usbfrac;
        newfrac = (datum->usbfrac *= factor);
        }

    msg ("%s: ap %d nuke/npts %3d/%3d %.3lf->%.3lf (%.3lf)", 1,
        sb ? "lsb" : "usb", ap, nuked, npts, oldfrac, newfrac, factor);
    }

/* eof */
