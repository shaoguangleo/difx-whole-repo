/************************************************************************/
/*                                                                      */
/* Takes a point selected by the user with a cursor on the psplot       */
/* screen, and attempts to pop up a fringe plot for that point.  The    */
/* usual hardcopy options are available.                                */
/*                                                                      */
/*      Inputs:                                                         */
/*                                                                      */
/*      Output:                                                         */
/*                                                                      */
/* Created 22 February 1993 by CJL                                      */
/* Added Mk4 fringe plot popup, 2 February 2001 by CJL                  */
/* removed mk3 code                          2010.6.8  rjc              */
/************************************************************************/
#include <stdio.h>
#include "mk4_data.h"
#include "mk4_dfio.h"
#include "aedata.h"
#include "psplot.h"
#include "aedit.h"

int ps_fplot (struct ps_array *psarray, fringesum *fdatum)
    {
    static struct mk4_fringe fringe4;
    int index, ret;
    char *fname, *fringename(), fullname[256];
    char *argv[2];
    static int argc = 1;
    extern char datadir[];
                                /* This gets rel. filename of selected point */
    if ((fname = fringename (fdatum)) == NULL)
        {
        msg ("Could not generate data file name", 2);
        return (1);
        }
    sprintf (fullname, "%s/%s", datadir, fname);
                                        /* Establish file type */
                                        /* Mk4 data */
    if (read_mk4fringe (fullname, &fringe4) != 0)
        {
        msg ("Failure reading fringe file %s", 2, fullname);
        return (1);
        }
    display_221 (fringe4.t221, -1);

    return(ret);
    }
