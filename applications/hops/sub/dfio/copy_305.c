/************************************************************************/
/*									*/
/* This is the inverse of addr_305().  It takes an application		*/
/* structure and a target address, and places an overlay structure	*/
/* of the appropriate version into the target address.  Sometimes this	*/
/* will require a field-by-field copying operation, sometimes it will	*/
/* be a ptr assignment operation, depending on version control status.	*/
/* To handle byte-flipping, the copy operation is now done in all cases	*/
/* which allows use of the bytflp.h macros				*/
/*									*/
/*	Inputs:		t305		application structure pointer	*/
/*									*/
/*	Output:		ptr		overlay structure address	*/
/*					with data filled in		*/
/*			return value	number of bytes filled in	*/
/*									*/
/* Created 25 September 1995 by CJL					*/
/* Redesigned 23 September 1997 by CJL					*/
/*									*/
/************************************************************************/
#include <stdio.h>
#include <string.h>
#include "bytflp.h"
#include "type_305.h"
#include "mk4_dfio.h"
#include "mk4_util.h"

int
copy_305 (struct type_305 *t305,
          char **ptr)
    {
    int version;
    struct type_305_v0 *t305_v0;
					/* What version is requested for */
					/* the disk format? */
    sscanf (t305->version_no, "%2d", &version);
					/* Disk format same as app struct */
					/* Simple pointer assignment */
    if (version == T305_VERSION) *ptr = (char *)t305;
    else if (version == 0)
	{
	*ptr = (char *)malloc (sizeof (struct type_305_v0));
	if (*ptr == NULL)
	    {
	    msg ("Memory allocation failure in copy_305()", 2);
	    return (-1);
	    }
	}
    else
	{
	msg ("Unrecognized version number %d in copy_305()", 2, version);
	return (-1);
	}
					/* Handle each version number */
					/* individually. */
    if (version == 0)
	{
	t305_v0 = (struct type_305_v0 *) *ptr;
	strncpy (t305_v0->record_id, "305", 3);
	strncpy (t305_v0->version_no, "00", 2);

/* COPY RECORD_SPECIFIC DATA HERE */

	return (sizeof (struct type_305_v0));
	}
    else
	{
	msg ("Unrecognized version number %d in copy_305()", 2, version);
	return (-1);
	}
    }
