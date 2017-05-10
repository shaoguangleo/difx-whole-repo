/************************************************************************/
/*                                                                      */
/*  Fills in a type_210 record                                          */
/*                                                                      */
/*      Inputs:         via externs                                     */
/*                                                                      */
/*      Output:         t210        Filled in type_210 record           */
/*                                                                      */
/* Created 8 March 2000                                                 */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include "mk4_data.h"
#include "vex.h"
#include "pass_struct.h"
#include "param_struct.h"

#define pi 3.141592654

int
fill_210 (
struct type_pass *pass,
struct type_status *status,
struct type_210 *t210)
    {
    int i;

    clear_210 (t210);
                                        /* Precalculated in make_plotdata() */
    for (i=0; i<pass->nfreq; i++)
        {
        t210->amp_phas[i].ampl = (float)cabs (status->fringe[i]) / 10000.0;
        t210->amp_phas[i].phase = (float)carg (status->fringe[i]) * 180.0 / pi;
        }

    return (0);
    }
