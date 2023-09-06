/************************************************************************/
/*                                                                      */
/* This routine takes a type-221 record, which contains a postscript    */
/* fringeplot, and displays it in a (persistent) ghostscript window.    */
/* It modifies the plot to insert instructions at the bottom of the     */
/* plot for interactive control, then switches the user's terminal into */
/* unbuffered mode and captures the next keystroke.  It takes action    */
/* depending on the result.  There are two modes, which offer different */
/* actions.  The ghostscript executable is assumed to be in /usr/bin,   */
/* and the program is assumed to be running from a normal terminal      */
/* session which can be controlled using stty(1) commands.  The         */
/* hardcopy capability, which is system-dependent, is implemented in    */
/* the shell script $BIN/pplot_print.                                   */
/*                                                                      */
/*      Inputs:     t221        pointer to type 221 record, filled in   */
/*                  mode        0=basic, 1='p' and 'n' for prev, next   */
/*                              -1=display plot and return immediately  */
/*                              -2=shut down display window             */
/*                                                                      */
/*      Output:     display or hardcopy                                 */
/*                  return value    key pressed, or space for mission   */
/*                                  accomplished.  Null for error.      */
/*                                                                      */
/* Created November 10 1999 by CJL                                      */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "mk4_data.h"
#include "mk4_dfio.h"
#include "mk4_util.h"

#ifndef GS_EXEC
# define GS_EXEC "/usr/bin/gs"
# warning "GS_EXEC default of /usr/bin/gs used"
#else /* GS_EXEC */
# warning "GS_EXEC provided by preprocessor"
#endif /* GS_EXEC */
#ifdef P_tmpdir
# define P_tmpdir "/tmp"
#endif /* P_tmpdir */

#define TRUE 1
#define FALSE 0
                                        /* Set up macro for inserting justified */
                                        /* text using native postscript fonts */
                                        /* (taken from make_postplot() in fourfit) */
#define psleft(xcoord, ycoord, ps_string)\
             {xval = xcoord * 7800; yval = ycoord * 10500;\
                sprintf (psbuf, "%d %d M (%s) SL\n", xval, yval, ps_string);}

char
display_221 (struct type_221 *t221,
             int mode)
    {
    char *pplot, *end_of_plot, c, *ptr;
    int i, nchar, size, xval, yval, loop;
    static int gsopen = FALSE;
    static FILE *gs;
    static char psbuf[2560], cmd[1280], response[1024], fname[1024];
    static char ps_file[1024] = "sub-dfio_";
    extern char progname[];
    FILE *fp;

    if (mode == -2)
        {
        if (gsopen) pclose (gs);
        else msg ("Error, attempt to close non-existent gs pipe", 2);
        gsopen = FALSE;
        return ('\0');
        }

    pplot = t221->pplot;
                                        /* Locate end of original plot */
    end_of_plot = strstr (pplot, "PGPLOT restore showpage");
    if (end_of_plot == NULL)
        {
        msg ("Error, invalid postscript plot in type 221 record", 2);
        return ('\0');
        }
    nchar = end_of_plot - pplot;
                                        /* Open gs and display the plot */
    if (! gsopen)
        {
        gs = popen (GS_EXEC " -q -", "w");
//      gs = popen ("/usr/bin/gs -q -", "w");
//      gs = popen ("/usr/bin/gs -", "w");
        gsopen = TRUE;
        if (gs == NULL)
            {
            msg ("Cannot open ghostscript display", 2);
            return ('\0');
            }
        }
                                        /* Last command must have been a */
                                        /* copypage (see below), so we still */
                                        /* need an erase */
    else
        {
        sprintf (psbuf, " erasepage\n");
        fwrite (psbuf, 1, strlen (psbuf), gs);
        }
                                        /* Dump the postscript instructions */
    fwrite (pplot, 1, nchar, gs);
                                        /* Now add some instructions at the */
                                        /* bottom of the plot */
    sprintf (psbuf, "/Helvetica findfont 110 scalefont setfont\n");
    fwrite (psbuf, 1, strlen (psbuf), gs);

    sprintf (psbuf, "0.7 0.0 0.7 setrgbcolor\n");
    fwrite (psbuf, 1, strlen (psbuf), gs);

    if (mode == -1)
        ptr = "                                                             ";
    else if (mode == 0)
        ptr = "Press a key: 'h'=hardcopy, 's'=save, 'q'=quit, other=continue";
    else if (mode == 1)
        ptr = "Press a key: 'h'=hardcopy, 'p'=previous, 'n'=next, 's'=save, 'q'=quit, other=continue";
    else
        {
        msg ("Invalid mode %d in display_221()", 2, mode);
        return ('\0');
        }

    for (i=0; i<200; i++)           // put in extra padding to circumvent flushing problem
        {                           // rjc 2009.3.11
        psleft (0.1, 0.01, ptr);
        fwrite (psbuf, 1, strlen (psbuf), gs);
        }
                                        /* Substitute copypage for showpage to */
                                        /* keep ghostscript happy */
    sprintf (psbuf, "PGPLOT restore copypage");
    fwrite (psbuf, 1, strlen (psbuf), gs);
                                        /* Append the rest of the file */
    end_of_plot += strlen (psbuf);
                                        /* following line causes blank page
                                           to be written; comment out rjc
                                           2004.5.24 */
    /*  fwrite (end_of_plot, 1, strlen (end_of_plot), gs); */

                                        /* Flush to force the display */ 
    fflush (gs);
                                        /* Wait for a keystroke unless mode=-1 */
    loop = TRUE;
    if (mode == -1) 
        {
        c = 'q';
        loop = FALSE;
        }
    while (loop)
        {
        system ("stty -echo -icanon min 1");
        c = getchar();
                                        /* send rest of plot after char
                                           received; otherwise it gets
                                           prematurely erased 
                                                     rjc 2004.11.23 */
        fwrite (end_of_plot, 1, strlen (end_of_plot), gs);
        system ("stty echo icanon");

        switch (c)
            {
                                        /* Hardcopy to color printer */
            case 'h':
            case 'H':
                {
//              ps_file = tmpnam (NULL);
//              if ((fp = fopen (ps_file, "w")) == NULL)
                strcpy(ps_file, P_tmpdir "/sub-dfio_XXXXXX");
                if ((fp = fdopen(size=mkstemp(ps_file), "w")) == NULL)
                    {
                    msg ("fdopen of PS file (%s,%d) for printing failed",   
                        2, ps_file, size);
                    return ('\0');
                    }
                size = strlen (pplot);
                fwrite (pplot, 1, size, fp);
                fclose (fp);
                sprintf (cmd, "pplot_print %s", ps_file);
                system (cmd);
                msg ("Printing hardcopy of postscript fringe plot", 2);
                                        /* Tidy up */
                unlink (ps_file);
                }
                break;
                                        /* Save to a file */
            case 's':
            case 'S':
                printf ("%s: Type name of file to save plot in:  ", progname); 
                fflush(stdout);
                fgets (response, 127, stdin);
                ptr = response;
                                        /* Find first whitespace-delimited */
                                        /* string in the response */
                for (i=0; i<strlen(response); i++)
                    if (! isspace(response[i])) break;
                sscanf (response+i, "%s", fname);
                                        /* Open and save */
                if ((fp = fopen (fname, "w")) == NULL)
                    {
                    msg ("Could not open file '%s' to save fringe plot", 2, fname);
                    return ('\0');
                    }
                size = strlen (pplot);
                nchar = fwrite (pplot, 1, size, fp);
                fclose (fp);
                msg ("Wrote %d bytes into file '%s'", 2, nchar, fname);
                break;
                                        /* Terminate loop and pass typed */
                                        /* character back to caller */
            default:
                loop = FALSE;
                break;
            }
        }

    return (c);
    }
