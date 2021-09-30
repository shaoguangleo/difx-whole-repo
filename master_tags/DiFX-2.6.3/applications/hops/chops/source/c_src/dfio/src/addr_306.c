/************************************************************************/
/*                                                                      */
/* Standard record version control.  This routine returns the address   */
/* of a structure containing the desired record information.  This can  */
/* either be the address of the raw memory image of the disk record     */
/* that was read in, or a memory-allocated structure filled in element  */
/* by element, depending on whether or not the disk format and the      */
/* structure definitions match.  Either way, byte flipping is performed */
/* as necessary by the architecture-dependent macros cp_xxxx() defined  */
/* in bytflp.h                                                          */
/*                                                                      */
/*      Inputs:         version         Version number of disk image    */
/*                      address         Memory address of disk image    */
/*                                                                      */
/*      Output:         size		number of bytes read from input	*/
/*					address				*/
/*			Return value    Address of filled app structure */
/*									*/
/* Created 25 September 1995 by CJL					*/
/* Redesigned 17 September 1997 by CJL                                  */
/*									*/
/************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bytflp.h"
#include "type_306.h"
#include "mk4_dfio.h"
#include "mk4_util.h"

#define FALSE 0
#define TRUE 1

struct type_306 *
addr_306 (short version,
          void *address,
          int *size)
    {
    int i, malloced;
    struct type_306 *t306;
    struct type_306_v0 *t306_v0;
					/* Create application structure, which */
					/* might be simply the overlay structure */
    malloced = FALSE;
    if (version == T306_VERSION) t306 = (struct type_306 *)address;
    else
	{
	t306 = (struct type_306 *)malloc (sizeof (struct type_306));
	if (t306 == NULL)
	    {
	    msg ("Memory allocation failure in addr_306()", 2);
	    return (NULL);
	    }
	clear_306 (t306);
	malloced = TRUE;
	}
					/* Handle each version number */
					/* individually. */
    if (version == 0)
	{
					/* Overlay version-specific structure */
					/* noting size so we can maintain */
					/* pointer in file image */
	*size = sizeof (struct type_306_v0);
	t306_v0 = (struct type_306_v0 *)address;
					/* Copy structure elements, */
					/* with hidden byte flipping if needed */
					/* (see bytflp.h) */
	strncpy (t306->record_id, "306", 3);
	strncpy (t306->version_no, "00", 2);
	cp_short (t306->time.year, t306_v0->time.year);
	cp_short (t306->time.day, t306_v0->time.day);
	cp_short (t306->time.hour, t306_v0->time.hour);
	cp_short (t306->time.minute, t306_v0->time.minute);
	cp_float (t306->time.second, t306_v0->time.second);
	cp_float (t306->duration, t306_v0->duration);
	for (i=0; i<16; i++)
	    {
	    strcpy (t306->stcount[i].chan_id, t306_v0->stcount[i].chan_id);
	    cp_int (t306->stcount[i].bigpos, t306_v0->stcount[i].bigpos);
	    cp_int (t306->stcount[i].pos, t306_v0->stcount[i].pos);
	    cp_int (t306->stcount[i].neg, t306_v0->stcount[i].neg);
	    cp_int (t306->stcount[i].bigneg, t306_v0->stcount[i].bigneg);
	    }

	return (t306);
	}
    else 
	{
	msg ("Unrecognized type 306 record version number %d", 2, version);
	if (malloced) free (t306);
	return (NULL);
	}
    }
