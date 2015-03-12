/*#include "STDDEFS.H"*/
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <stdio.h>    /* defines stderr */
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <math.h>
#include <sys/syslog.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <inttypes.h>
#include "CALCServer.h"
#include "CALC_9_1_RA_Server.h"     /* RPCGEN creates this from CALC_9_1_RA_Server.x */
#include "DiFX_Delay_Server.h"





/* GLOBALS */
int verbosity = 0;


int  ifirst, ilogdate = 1;
FILE  *flog = 0;
static unsigned int count = (unsigned int)(-1);
#define SPAWN_TIME_SIZE 64
char spawn_time[SPAWN_TIME_SIZE];
const char Version[] = "DiFX_Delay_Server Version 0.0.0";
static struct timeval TIMEOUT = {25, 0};
#define SECONDS_PER_DAY INT64_C(86400)
#define SECONDS_PER_DAY_DBL (86400.0)
#define MJD_AT_UNIX_TIME_ORIGIN 40587



/******************************************************************************/
static void check_date_for_logfile(void)
{
    struct tm *timeptr;
    time_t  timep, *tp;
    int itoday;

#define FILENAME_SIZE 256
#define HOSTNAME_SIZE 24
    char    filename[FILENAME_SIZE], hostname[HOSTNAME_SIZE];


    /* check if we have crossed a day boundry in local time. If so, open a
     * new logfile */
    if (ilogdate != 0)
    {
        tp = &timep;
        time (tp);
        timeptr = localtime (tp);
        itoday = timeptr->tm_mday + 100*(timeptr->tm_mon+1) + 10000*(timeptr->tm_year+1900);
        if (itoday != ilogdate)
        {
            gethostname(hostname, HOSTNAME_SIZE-1);
            snprintf (filename, FILENAME_SIZE, "DiFX_Delay_Server:%s%02d%02d%02d:%02d%02d%02d.log", hostname, 
                      timeptr->tm_year-100,
                      timeptr->tm_mon+1, timeptr->tm_mday, timeptr->tm_hour,
                      timeptr->tm_min, timeptr->tm_sec);
            ilogdate = itoday;
            if (flog != 0)
            {
                fclose(flog);
            }
            if ((flog = fopen (filename, "w")) == NULL)
            {
                fprintf (stderr, "ERROR: cannot open log file %s\n", filename);
                exit(EXIT_FAILURE);
            }
            printf ("opened new DiFX_Delay_Server log file : %s\n", filename);
            fprintf (flog, "%s Started : %s", Version, spawn_time);
            fprintf (flog, "Logfile Opened      : %s", ctime(tp));
            ifirst = 1;
        }
    }
    return;
}



/******************************************************************************/
getCALC_res *
getcalc_1(argp, clnt)
	struct getCALC_arg *argp;
	CLIENT *clnt;
{
	static getCALC_res clnt_res;
    enum clnt_stat clnt_stat;

	memset((char *)&clnt_res, 0, sizeof (clnt_res));
	clnt_stat = clnt_call(clnt, GETCALC,
                          (xdrproc_t) xdr_getCALC_arg, (caddr_t) argp,
                          (xdrproc_t) xdr_getCALC_res, (caddr_t) &clnt_res,
                          TIMEOUT);

    if (clnt_stat != RPC_SUCCESS)
    {
        fprintf (flog, "clnt_call failed in getcalc_1\n");
        return NULL;
    }

	return (&clnt_res);
}


/******************************************************************************/
static int difx_delay_server_prog_1_CALCPROG(getDIFX_DELAY_SERVER_1_arg *arg_0, DIFX_DELAY_SERVER_1_res *res_0)
{
    static int initialized = 0;
    static char localhost[] = "localhost";
    static CLIENT *cl = NULL;
    /* Copy incoming arguments to the format needed for CALC_9_1_RA */
    getCALC_arg localargument;
    getCALC_arg *arg_1 = &localargument;
    getCALC_res *res_1 = NULL;
    unsigned int station;
    unsigned int source;
    unsigned int k;
    unsigned int K_MAX = (NUM_DIFX_DELAY_SERVER_1_KFLAGS < 64) ? NUM_DIFX_DELAY_SERVER_1_KFLAGS:64;
    char station_a_code[3];
    char station_b_code[3];
    static const int NUM_1_EOP = 5;

    if(!initialized)
    {
        char *host = getenv("CALC_SERVER");
        if(host == NULL)
        {
            fprintf(flog, "No CALC_SERVER environment variable --- using localhost\n");
            host = localhost;
        }
        if (!(cl = clnt_create (host, CALCPROG, CALCVERS, "tcp")))
        {
            clnt_pcreateerror (host);
            fprintf(flog, "ERROR: rpc clnt_create fails for host : '%s'\n", host);
            return -1;
        }
    }

    /* sanity check */

    if((arg_0 == NULL) || (res_0 == NULL))
    {
        fprintf(flog, "NULL argument\n");
        return -2;
    }
    if(arg_0->Num_Stations < 2)
    {
        fprintf(flog, "Not enough stations in RPC argument\n");
        return -3;
    }
    if((int)arg_0->Num_EOPs < NUM_1_EOP)
    {
        fprintf(flog, "Not enough EOPs in RPC argument\n");
        fprintf(flog, "CALCServer requires %d EOPs, but only %u provided\n", NUM_1_EOP, arg_0->Num_EOPs);
        return -4;
    }
    if(arg_0->Num_Stations != res_0->Num_Stations)
    {
        fprintf(flog, "arg_0 and res_0 Num_Stations do not agree\n");
        return -5;
    }
    if(arg_0->Num_Sources != res_0->Num_Sources)
    {
        fprintf(flog, "arg_0 and res_0 Num_Sources do not agree\n");
        return -6;
    }
    if((res_0->result.result_val == NULL) || (res_0->result.result_len != arg_0->Num_Stations*arg_0->Num_Sources))
    {
        fprintf(flog, "res_0 result area not set up properly\n");
        return -7;
    }

    

    /* clear memory */
    memset(&arg_1, 0, sizeof(getCALC_arg));

    /* copy the general stuff */
    arg_1->request_id = arg_0->request_id;
    arg_1->date = arg_0->date;
    arg_1->ref_frame = arg_0->ref_frame;
    arg_1->time = arg_0->time;
    for(k=0; k < K_MAX; ++k)
    {
        arg_1->kflags[k] = arg_0->kflags[k];
    }
    {
        int min_loc = 0;
        int max_loc = arg_0->Num_EOPs;
        int best_loc = 0;
        double best_diff = 1E300;
        int e;
        double this_date = arg_0->date + arg_0->time;
        for(e=0; e < (int)arg_0->Num_EOPs; ++e)
        {
            double diff = fabs(arg_0->EOP.EOP_val[e].EOP_time - this_date);
            if(diff < best_diff)
            {
                best_diff = diff;
                best_loc = e;
            }
        }
        min_loc = best_loc - 2;
        if(min_loc < 0)
        {
            min_loc = 0;
        }
        max_loc = min_loc + NUM_1_EOP;
        if(max_loc > (int)arg_0->Num_EOPs)
        {
            max_loc = (int)arg_0->Num_EOPs;
        }
        min_loc = max_loc - NUM_1_EOP;
        for(e=min_loc; e < max_loc; ++e)
        {
            arg_1->EOP_time[e-min_loc] = arg_0->EOP.EOP_val[e].EOP_time;
            arg_1->tai_utc[e-min_loc]  = arg_0->EOP.EOP_val[e].tai_utc;
            arg_1->ut1_utc[e-min_loc]  = arg_0->EOP.EOP_val[e].ut1_utc;
            arg_1->xpole[e-min_loc]    = arg_0->EOP.EOP_val[e].xpole;
            arg_1->ypole[e-min_loc]    = arg_0->EOP.EOP_val[e].ypole;
        }
    }

    /* copy station 0 over to CALC_9_1_RA station a */
    /* also copy over constant stuff for station b */
    arg_1->a_x = arg_0->station.station_val[0].station_pos.x;
    arg_1->a_y = arg_0->station.station_val[0].station_pos.y;
    arg_1->a_z = arg_0->station.station_val[0].station_pos.z;
    arg_1->axis_off_a = arg_0->station.station_val[0].axis_off;
    arg_1->pressure_a = arg_0->station.station_val[0].pressure;
    station_a_code[0] = (char)(arg_0->station.station_val[0].site_ID&0xFF);
    station_a_code[1] = (char)(arg_0->station.station_val[0].site_ID>>8);
    station_a_code[2] = 0;
    arg_1->station_a = station_a_code;
    arg_1->axis_type_a = arg_0->station.station_val[0].axis_type;

    /* Loop over stations and sources */
    for(station = 1; station < arg_0->Num_Stations; ++station)
    {
        /* fill in station b information */
        arg_1->b_x = arg_0->station.station_val[station].station_pos.x;
        arg_1->b_y = arg_0->station.station_val[station].station_pos.y;
        arg_1->b_z = arg_0->station.station_val[station].station_pos.z;
        arg_1->axis_off_b = arg_0->station.station_val[station].axis_off;
        arg_1->pressure_b = arg_0->station.station_val[station].pressure;
        station_b_code[0] = (char)(arg_0->station.station_val[station].site_ID&0xFF);
        station_b_code[1] = (char)(arg_0->station.station_val[station].site_ID>>8);
        station_b_code[2] = 0;
        arg_1->station_b = station_b_code;
        arg_1->axis_type_b = arg_0->station.station_val[station].axis_type;

        /* Loop over sources */
        for(source = 0; source < arg_0->Num_Sources; ++source)
        {
            arg_1->ra = arg_0->source.source_val[source].ra;
            arg_1->dec = arg_0->source.source_val[source].dec;
            arg_1->dra = arg_0->source.source_val[source].dra;
            arg_1->ddec = arg_0->source.source_val[source].ddec;
            arg_1->depoch = arg_0->source.source_val[source].depoch;
            arg_1->parallax = arg_0->source.source_val[source].parallax;
            arg_1->source = arg_0->source.source_val[source].source_name;

            /* call server */
            res_1 = getcalc_1(&arg_1, cl);
            if(res_1 == NULL)
            {
                /* call failed */
                fprintf(flog, "Call to getcalc_1 failed\n");
                return -8;
            }

            /* copy server result into our large result table */
            res_0->server_error = res_1->error;
            res_0->model_error  = res_1->error;
            if((res_1->error))
            {
                fprintf(flog, "Call to getcalc_1 returned error code %d\n", res_1->error);
                clnt_freeres(cl, (xdrproc_t) xdr_getCALC_res, (caddr_t) res_1);
                return -9;
            }
            res_0->request_id = res_1->getCALC_res_u.record.request_id;
            res_0->delay_server = CALCPROG;
            res_0->date = res_1->getCALC_res_u.record.date;
            res_0->time = res_1->getCALC_res_u.record.time;
            {
                /* convert MJD time to Unix time */
                /* keep operations separate in order to preserve precision */
                double d;
                int_fast64_t unix_sec;
                int_fast64_t i;
                uint_fast64_t u;
                unix_sec = ((int_fast64_t)res_0->date-MJD_AT_UNIX_TIME_ORIGIN)*SECONDS_PER_DAY_DBL;
                d = res_0->time*SECONDS_PER_DAY_DBL;
                i = d;
                unix_sec += i;
                d -= i;
                if(d < 0.0)
                {
                    d += 1.0;
                    --unix_sec;
                }
                u = unix_sec;
                res_0->unix_utc_seconds_0 = u & 0xFFFFFFFF;
                res_0->unix_utc_seconds_0 = (u >> 32);
                res_0->utc_second_fraction = d;
            }
            /* station data */
            res_0->result.result_val[station*arg_0->Num_Sources+source].delay = res_1->getCALC_res_u.record.delay[0];
            res_0->result.result_val[station*arg_0->Num_Sources+source].dry_atmos = res_1->getCALC_res_u.record.dry_atmos[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].wet_atmos = res_1->getCALC_res_u.record.wet_atmos[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].az = res_1->getCALC_res_u.record.az[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].el = res_1->getCALC_res_u.record.el[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].primary_axis_angle = DIFX_DELAY_SERVER_1_MISSING_GENERAL_DATA;
            res_0->result.result_val[station*arg_0->Num_Sources+source].secondary_axis_angle = DIFX_DELAY_SERVER_1_MISSING_GENERAL_DATA;
            res_0->result.result_val[station*arg_0->Num_Sources+source].mount_source_angle = DIFX_DELAY_SERVER_1_MISSING_GENERAL_DATA;
            /* station 0 data */
            res_0->result.result_val[source].dry_atmos = res_1->getCALC_res_u.record.dry_atmos[0];
            res_0->result.result_val[source].wet_atmos = res_1->getCALC_res_u.record.wet_atmos[0];
            res_0->result.result_val[source].az = res_1->getCALC_res_u.record.az[0];
            res_0->result.result_val[source].el = res_1->getCALC_res_u.record.el[0];
            res_0->result.result_val[source].primary_axis_angle = DIFX_DELAY_SERVER_1_MISSING_GENERAL_DATA;
            res_0->result.result_val[source].secondary_axis_angle = DIFX_DELAY_SERVER_1_MISSING_GENERAL_DATA;
            res_0->result.result_val[source].mount_source_angle = DIFX_DELAY_SERVER_1_MISSING_GENERAL_DATA;
            /* UVW data */
            res_0->result.result_val[station*arg_0->Num_Sources+source].UVW.x = res_1->getCALC_res_u.record.UV[0];
            res_0->result.result_val[station*arg_0->Num_Sources+source].UVW.y = res_1->getCALC_res_u.record.UV[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].UVW.z = res_1->getCALC_res_u.record.UV[2];

            /* free the results */
            
            if(clnt_freeres(cl, (xdrproc_t) xdr_getCALC_res, (caddr_t) res_1) != 1)
            {
                fprintf(flog, "Failed to free results buffer\n");
                return -10;
            }
        } /* for sources */
    } /* for stations */
    return 0;
}


/******************************************************************************/
getCALC_9_1_RA_res *
getcalc_9_1_ra_1(argp, clnt)
	struct getCALC_9_1_RA_arg *argp;
	CLIENT *clnt;
{
	static getCALC_9_1_RA_res clnt_res;
    enum clnt_stat clnt_stat;

	memset((char *)&clnt_res, 0, sizeof (clnt_res));
	clnt_stat = clnt_call(clnt, GETCALC_9_1_RA,
                          (xdrproc_t) xdr_getCALC_9_1_RA_arg, (caddr_t) argp,
                          (xdrproc_t) xdr_getCALC_9_1_RA_res, (caddr_t) &clnt_res,
                          TIMEOUT);

    if (clnt_stat != RPC_SUCCESS)
    {
        fprintf (flog, "clnt_call failed in getcalc_9_1_ra_1\n");
        return NULL;
    }

	return (&clnt_res);
}


/******************************************************************************/
static int difx_delay_server_prog_1_CALC_9_1_RAPROG(getDIFX_DELAY_SERVER_1_arg *arg_0, DIFX_DELAY_SERVER_1_res *res_0)
{
    static int initialized = 0;
    static char localhost[] = "localhost";
    static CLIENT *cl = NULL;
    /* Copy incoming arguments to the format needed for CALC_9_1_RA */
    getCALC_9_1_RA_arg localargument;
    getCALC_9_1_RA_arg *arg_1 = &localargument;
    getCALC_9_1_RA_res *res_1 = NULL;
    unsigned int station;
    unsigned int source;
    unsigned int k;
    unsigned int K_MAX = (NUM_DIFX_DELAY_SERVER_1_KFLAGS < 64) ? NUM_DIFX_DELAY_SERVER_1_KFLAGS:64;
    char station_a_code[3];
    char station_b_code[3];
    static const int NUM_1_EOP = 5;
    double fractional_MJD;

    if(!initialized)
    {
        char *host = getenv("CALC_9_1_RA_SERVER");
        if(host == NULL)
        {
            fprintf(flog, "No CALC_9_1_RA_SERVER environment variable --- using localhost\n");
            host = localhost;
        }
        if (!(cl = clnt_create (host, CALC_9_1_RAPROG, CALC_9_1_RAVERS, "tcp")))
        {
            clnt_pcreateerror (host);
            fprintf(flog, "ERROR: rpc clnt_create fails for host : '%s'\n", host);
            return -1;
        }
    }

    /* sanity check */

    if((arg_0 == NULL) || (res_0 == NULL))
    {
        fprintf(flog, "NULL argument\n");
        return -2;
    }
    if(arg_0->Num_Stations < 2)
    {
        fprintf(flog, "Not enough stations in RPC argument\n");
        return -3;
    }
    if((int)arg_0->Num_EOPs < NUM_1_EOP)
    {
        fprintf(flog, "Not enough EOPs in RPC argument\n");
        fprintf(flog, "CALC_9_1_RA requires %d EOPs, but only %u provided\n", NUM_1_EOP, arg_0->Num_EOPs);
        return -4;
    }
    if(arg_0->Num_Stations != res_0->Num_Stations)
    {
        fprintf(flog, "arg_0 and res_0 Num_Stations do not agree\n");
        return -5;
    }
    if(arg_0->Num_Sources != res_0->Num_Sources)
    {
        fprintf(flog, "arg_0 and res_0 Num_Sources do not agree\n");
        return -6;
    }
    if((res_0->result.result_val == NULL) || (res_0->result.result_len != arg_0->Num_Stations*arg_0->Num_Sources))
    {
        fprintf(flog, "res_0 result area not set up properly\n");
        return 7;
    }
    fractional_MJD = arg_0->date + arg_0->time;

    

    /* clear memory */
    memset(&arg_1, 0, sizeof(getCALC_9_1_RA_arg));

    /* copy the general stuff */
    arg_1->request_id = arg_0->request_id;
    arg_1->struct_code = arg_0->server_struct_setup_code;
    arg_1->date = arg_0->date;
    arg_1->ref_frame = arg_0->ref_frame;
    arg_1->time = arg_0->time;
    for(k=0; k < K_MAX; ++k)
    {
        arg_1->kflags[k] = arg_0->kflags[k];
    }
    {
        int min_loc = 0;
        int max_loc = arg_0->Num_EOPs;
        int best_loc = 0;
        double best_diff = 1E300;
        int e;
        double this_date = arg_0->date + arg_0->time;
        for(e=0; e < (int)arg_0->Num_EOPs; ++e)
        {
            double diff = fabs(arg_0->EOP.EOP_val[e].EOP_time - this_date);
            if(diff < best_diff)
            {
                best_diff = diff;
                best_loc = e;
            }
        }
        min_loc = best_loc - 2;
        if(min_loc < 0)
        {
            min_loc = 0;
        }
        max_loc = min_loc + NUM_1_EOP;
        if(max_loc > (int)arg_0->Num_EOPs)
        {
            max_loc = (int)arg_0->Num_EOPs;
        }
        min_loc = max_loc - NUM_1_EOP;
        for(e=min_loc; e < max_loc; ++e)
        {
            arg_1->EOP_time[e-min_loc] = arg_0->EOP.EOP_val[e].EOP_time;
            arg_1->tai_utc[e-min_loc]  = arg_0->EOP.EOP_val[e].tai_utc;
            arg_1->ut1_utc[e-min_loc]  = arg_0->EOP.EOP_val[e].ut1_utc;
            arg_1->xpole[e-min_loc]    = arg_0->EOP.EOP_val[e].xpole;
            arg_1->ypole[e-min_loc]    = arg_0->EOP.EOP_val[e].ypole;
        }
    }

    /* copy station 0 over to CALC_9_1_RA station a */
    /* also copy over constant stuff for station b */
    arg_1->a_x = arg_0->station.station_val[0].station_pos.x;
    arg_1->a_y = arg_0->station.station_val[0].station_pos.y;
    arg_1->a_z = arg_0->station.station_val[0].station_pos.z;
    arg_1->a_dx = arg_0->station.station_val[0].station_vel.x;
    arg_1->a_dy = arg_0->station.station_val[0].station_vel.y;
    arg_1->a_dz = arg_0->station.station_val[0].station_vel.z;
    arg_1->a_ddx = arg_0->station.station_val[0].station_acc.x;
    arg_1->a_ddy = arg_0->station.station_val[0].station_acc.y;
    arg_1->a_ddz = arg_0->station.station_val[0].station_acc.z;
    arg_1->axis_off_a = arg_0->station.station_val[0].axis_off;
    arg_1->pointing_pos_a_z[0] = arg_0->station.station_val[0].station_pointing_dir.x;
    arg_1->pointing_pos_a_z[1] = arg_0->station.station_val[0].station_pointing_dir.y;
    arg_1->pointing_pos_a_z[2] = arg_0->station.station_val[0].station_pointing_dir.z;
    arg_1->pointing_pos_a_x[0] = arg_0->station.station_val[0].station_reference_dir.x;
    arg_1->pointing_pos_a_x[1] = arg_0->station.station_val[0].station_reference_dir.y;
    arg_1->pointing_pos_a_x[2] = arg_0->station.station_val[0].station_reference_dir.z;
    /* No information available on Y axis or velocities */
    arg_1->pointing_epoch_a = fractional_MJD;
    arg_1->pointing_epoch_b = fractional_MJD;
    arg_1->pointing_parallax = arg_0->source.source_val[0].parallax;
    arg_1->pressure_a = arg_0->station.station_val[0].pressure;
    station_a_code[0] = (char)(arg_0->station.station_val[0].site_ID&0xFF);
    station_a_code[1] = (char)(arg_0->station.station_val[0].site_ID>>8);
    station_a_code[2] = 0;
    arg_1->station_a = station_a_code;
    arg_1->axis_type_a = arg_0->station.station_val[0].axis_type;

    /* Loop over stations and sources */
    for(station = 1; station < arg_0->Num_Stations; ++station)
    {
        /* fill in station b information */
        arg_1->b_x = arg_0->station.station_val[station].station_pos.x;
        arg_1->b_y = arg_0->station.station_val[station].station_pos.y;
        arg_1->b_z = arg_0->station.station_val[station].station_pos.z;
        arg_1->b_dx = arg_0->station.station_val[station].station_vel.x;
        arg_1->b_dy = arg_0->station.station_val[station].station_vel.y;
        arg_1->b_dz = arg_0->station.station_val[station].station_vel.z;
        arg_1->b_ddx = arg_0->station.station_val[station].station_acc.x;
        arg_1->b_ddy = arg_0->station.station_val[station].station_acc.y;
        arg_1->b_ddz = arg_0->station.station_val[station].station_acc.z;
        arg_1->axis_off_b = arg_0->station.station_val[station].axis_off;
        arg_1->pointing_pos_b_z[0] = arg_0->station.station_val[station].station_pointing_dir.x;
        arg_1->pointing_pos_b_z[1] = arg_0->station.station_val[station].station_pointing_dir.y;
        arg_1->pointing_pos_b_z[2] = arg_0->station.station_val[station].station_pointing_dir.z;
        arg_1->pointing_pos_b_x[0] = arg_0->station.station_val[station].station_reference_dir.x;
        arg_1->pointing_pos_b_x[1] = arg_0->station.station_val[station].station_reference_dir.y;
        arg_1->pointing_pos_b_x[2] = arg_0->station.station_val[station].station_reference_dir.z;
        arg_1->pressure_b = arg_0->station.station_val[station].pressure;
        station_b_code[0] = (char)(arg_0->station.station_val[station].site_ID&0xFF);
        station_b_code[1] = (char)(arg_0->station.station_val[station].site_ID>>8);
        station_b_code[2] = 0;
        arg_1->station_b = station_b_code;
        arg_1->axis_type_b = arg_0->station.station_val[station].axis_type;

        /* Loop over sources */
        for(source = 0; source < arg_0->Num_Sources; ++source)
        {
            arg_1->ra = arg_0->source.source_val[source].ra;
            arg_1->dec = arg_0->source.source_val[source].dec;
            arg_1->dra = arg_0->source.source_val[source].dra;
            arg_1->ddec = arg_0->source.source_val[source].ddec;
            arg_1->depoch = arg_0->source.source_val[source].depoch;
            arg_1->parallax = arg_0->source.source_val[source].parallax;
            arg_1->source_pos[0] = arg_0->source.source_val[source].source_pos.x;
            arg_1->source_pos[1] = arg_0->source.source_val[source].source_pos.y;
            arg_1->source_pos[2] = arg_0->source.source_val[source].source_pos.z;
            arg_1->source_vel[0] = arg_0->source.source_val[source].source_vel.x;
            arg_1->source_vel[1] = arg_0->source.source_val[source].source_vel.y;
            arg_1->source_vel[2] = arg_0->source.source_val[source].source_vel.z;
            arg_1->source_epoch = arg_0->source.source_val[source].depoch;
            arg_1->source_parallax = arg_0->source.source_val[source].parallax;
            arg_1->source = arg_0->source.source_val[source].source_name;

            /* call server */
            res_1 = getcalc_9_1_ra_1(&arg_1, cl);
            if(res_1 == NULL)
            {
                /* call failed */
                fprintf(flog, "Call to getcalc_9_1_ra_1 failed\n");
                return -8;
            }

            /* copy server result into our large result table */
            res_0->server_error = res_1->error;
            res_0->model_error  = res_1->error;
            if((res_1->error))
            {
                fprintf(flog, "Call to getcalc_9_1_ra_1 returned error code %d\n", res_1->error);
                clnt_freeres(cl, (xdrproc_t) xdr_getCALC_9_1_RA_res, (caddr_t) res_1);
                return -9;
            }
            res_0->request_id = res_1->getCALC_9_1_RA_res_u.record.request_id;
            res_0->delay_server = CALC_9_1_RAPROG;
            res_0->date = res_1->getCALC_9_1_RA_res_u.record.date;
            res_0->time = res_1->getCALC_9_1_RA_res_u.record.time;
            {
                /* convert MJD time to Unix time */
                /* keep operations separate in order to preserve precision */
                double d;
                int_fast64_t unix_sec;
                int_fast64_t i;
                uint_fast64_t u;
                unix_sec = ((int_fast64_t)res_0->date-MJD_AT_UNIX_TIME_ORIGIN)*SECONDS_PER_DAY;
                d = res_0->time*SECONDS_PER_DAY_DBL;
                i = d;
                unix_sec += i;
                d -= i;
                if(d < 0.0)
                {
                    d += 1.0;
                    --unix_sec;
                }
                u = unix_sec;
                res_0->unix_utc_seconds_0 = u & 0xFFFFFFFF;
                res_0->unix_utc_seconds_0 = (u >> 32);
                res_0->utc_second_fraction = d;
            }
            /* station data */
            res_0->result.result_val[station*arg_0->Num_Sources+source].delay = res_1->getCALC_9_1_RA_res_u.record.delay[0];
            res_0->result.result_val[station*arg_0->Num_Sources+source].dry_atmos = res_1->getCALC_9_1_RA_res_u.record.dry_atmos[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].wet_atmos = res_1->getCALC_9_1_RA_res_u.record.wet_atmos[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].az = res_1->getCALC_9_1_RA_res_u.record.az[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].el = res_1->getCALC_9_1_RA_res_u.record.el[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].primary_axis_angle = DIFX_DELAY_SERVER_1_MISSING_GENERAL_DATA;
            res_0->result.result_val[station*arg_0->Num_Sources+source].secondary_axis_angle = DIFX_DELAY_SERVER_1_MISSING_GENERAL_DATA;
            res_0->result.result_val[station*arg_0->Num_Sources+source].mount_source_angle = res_1->getCALC_9_1_RA_res_u.record.msa[1];
            /* station 0 data */
            res_0->result.result_val[source].dry_atmos = res_1->getCALC_9_1_RA_res_u.record.dry_atmos[0];
            res_0->result.result_val[source].wet_atmos = res_1->getCALC_9_1_RA_res_u.record.wet_atmos[0];
            res_0->result.result_val[source].az = res_1->getCALC_9_1_RA_res_u.record.az[0];
            res_0->result.result_val[source].el = res_1->getCALC_9_1_RA_res_u.record.el[0];
            res_0->result.result_val[source].primary_axis_angle = DIFX_DELAY_SERVER_1_MISSING_GENERAL_DATA;
            res_0->result.result_val[source].secondary_axis_angle = DIFX_DELAY_SERVER_1_MISSING_GENERAL_DATA;
            res_0->result.result_val[source].mount_source_angle = res_1->getCALC_9_1_RA_res_u.record.msa[0];
            /* UVW data */
            res_0->result.result_val[station*arg_0->Num_Sources+source].UVW.x = res_1->getCALC_9_1_RA_res_u.record.UV[0];
            res_0->result.result_val[station*arg_0->Num_Sources+source].UVW.y = res_1->getCALC_9_1_RA_res_u.record.UV[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].UVW.z = res_1->getCALC_9_1_RA_res_u.record.UV[2];
            res_0->result.result_val[station*arg_0->Num_Sources+source].baselineP2000.x = res_1->getCALC_9_1_RA_res_u.record.baselineP2000[0];
            res_0->result.result_val[station*arg_0->Num_Sources+source].baselineP2000.y = res_1->getCALC_9_1_RA_res_u.record.baselineP2000[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].baselineP2000.z = res_1->getCALC_9_1_RA_res_u.record.baselineP2000[2];
            res_0->result.result_val[station*arg_0->Num_Sources+source].baselineV2000.x = res_1->getCALC_9_1_RA_res_u.record.baselineV2000[0];
            res_0->result.result_val[station*arg_0->Num_Sources+source].baselineV2000.y = res_1->getCALC_9_1_RA_res_u.record.baselineV2000[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].baselineV2000.z = res_1->getCALC_9_1_RA_res_u.record.baselineV2000[2];
            res_0->result.result_val[station*arg_0->Num_Sources+source].baselineA2000.x = res_1->getCALC_9_1_RA_res_u.record.baselineA2000[0];
            res_0->result.result_val[station*arg_0->Num_Sources+source].baselineA2000.y = res_1->getCALC_9_1_RA_res_u.record.baselineA2000[1];
            res_0->result.result_val[station*arg_0->Num_Sources+source].baselineA2000.z = res_1->getCALC_9_1_RA_res_u.record.baselineA2000[2];

            /* free the results */
            
            if(clnt_freeres(cl, (xdrproc_t) xdr_getCALC_9_1_RA_res, (caddr_t) res_1) != 1)
            {
                fprintf(flog, "Failed to free results buffer\n");
                return -10;
            }
        } /* for sources */
    } /* for stations */
    return 0;
}




/******************************************************************************/
static void
difx_delay_server_prog_1(struct svc_req *pRequest, SVCXPRT *pTransport)
/*
 * This function is called each time a client requests service.  It is passed 
 * an argument via the RPC specifing the requested delay input data ---  the
 * station and source coordinates and date.  It returns via the RPC the
 * interferometer delay.
 * 
 * This function was built by modifying the DiFX CALC5Server.c code, which in
 * turn was based on Ron Heald's Tcal RPC server function.
 */
{
    static getDIFX_DELAY_SERVER_1_arg argument;/* RPC caller passes this */
    static getDIFX_DELAY_SERVER_1_res result;  /* we return this to RPC caller */
    int my_error_code = 0;
    int handler_error_code = 0;

    static unsigned int NUM_STATIONS = 0;
    static unsigned int NUM_SOURCES  = 0;
    static size_t MEMORY_SIZE = 0;
    static void* memory = NULL;

    count++;
    check_date_for_logfile();

    fprintf(flog, "Processing RPC request %u\n", count);
    /* handle procedures */
    switch(pRequest->rq_proc) {
    case NULLPROC:
        fprintf(flog, "NULLPROC request --- returning\n");
        if (!svc_sendreply (pTransport, (xdrproc_t)xdr_void, NULL))
        {
            fprintf(flog, "svc_sendreply failure\n");
            svcerr_systemerr (pTransport);
        }
        return;
    case GETDIFX_DELAY_SERVER:
        break;
    default:
        /* handle unimplemented procedure number */
        fprintf(flog, "Unimplemented procedure %d --- returning\n", (int)pRequest->rq_proc);
        svcerr_noproc (pTransport);
        return;
	}

    /* get RPC input argument */
    memset(&argument, 0, sizeof(argument));    /* see "The Art of DA" p193 */
    if (!svc_getargs (pTransport, (xdrproc_t) xdr_getDIFX_DELAY_SERVER_1_arg, (caddr_t) &argument))
	{
        fprintf(flog, "Decode failure --- returning\n");
        svcerr_decode (pTransport);
        return;
	}

    /* sanity check */
    if((argument.Num_Stations != argument.station.station_len)
      || (argument.Num_Sources != argument.source.source_len)
      || (argument.Num_EOPs != argument.EOP.EOP_len))
    {
        fprintf(flog, "Caller implementation error: variable length arrays do not match alternate data values (%u,%u) (%u,%u) (%u,%u)\n", argument.Num_Stations, argument.station.station_len, argument.Num_Sources, argument.source.source_len, argument.Num_EOPs, argument.EOP.EOP_len);
        my_error_code = 1;
    }
    if(argument.Num_Stations < 1)
    {
        fprintf(flog, "No stations in RPC argument\n");
        my_error_code = 2;
    }
    if(argument.Num_Sources < 1)
    {
        fprintf(flog, "No sources in RPC argument\n");
        my_error_code = 3;
    }
    if(argument.Num_EOPs < 1)
    {
        fprintf(flog, "No EOPs in RPC argument\n");
        my_error_code = 4;
    }


    

    if(argument.verbosity >= 2) {
        unsigned int s, e;
        fprintf(flog, "Processing RPC message %u\n", count);
        fprintf(flog, "request arg: request_id=0x%lX delay_server=0x%lX server_struct_setup_code=0x%lX\n", argument.request_id, argument.delay_server, argument.server_struct_setup_code);
        fprintf(flog, "request arg: date=%ld time=%16.12f ref_frame=%ld verbosity=%d\n", argument.date, argument.time, argument.ref_frame, argument.verbosity);
        if(argument.verbosity >= 3) {
            unsigned int k;
            for(k=0; k < NUM_DIFX_DELAY_SERVER_1_KFLAGS; k++) {
                fprintf(flog, "request arg: kflag[%02u]=%hd\n", k, argument.kflags[k]);
            }
        }
        fprintf(flog, "request arg: unix_utc_seconds_0 = 0x%lX unix_utc_seconds_1 = 0x%lX\n", argument.unix_utc_seconds_0, argument.unix_utc_seconds_1);
        fprintf(flog, "request arg: utc_second_fraction = %E\n", argument.utc_second_fraction);
        fprintf(flog, "request arg: sky_frequency = %E\n", argument.sky_frequency);
        fprintf(flog, "Station information\n");
        fprintf(flog, "request arg: Use_Server_Station_Table=%d Num_Stations=%d\n", argument.Use_Server_Station_Table, argument.Num_Stations);
        for(s=0; s < argument.Num_Stations; s++) {
            char ID0, ID1;
            DIFX_DELAY_SERVER_vec v;
            fprintf(flog, "request arg: station=%02u station_name='%s'\n", s, argument.station.station_val[s].station_name);
            fprintf(flog, "request arg: station=%02u antenna_name='%s'\n", s, argument.station.station_val[s].antenna_name);
            fprintf(flog, "request arg: station=%02u site_name=   '%s'\n", s, argument.station.station_val[s].site_name);
            ID0 = (char)(argument.station.station_val[s].site_ID&0xFF);
            ID1 = (char)(argument.station.station_val[s].site_ID>>8);
            fprintf(flog, "request arg: station=%02u site_ID=     '%c%c' 0x%04hX\n", s, ID0, ID1, argument.station.station_val[s].site_ID);
            fprintf(flog, "request arg: station=%02u site_type=   '%s'\n", s, argument.station.station_val[s].site_type);
            fprintf(flog, "request arg: station=%02u axis_type=   '%s'\n", s, argument.station.station_val[s].axis_type);
            v = argument.station.station_val[s].station_pos;
            fprintf(flog, "request arg: station=%02u station_pos= [%14.4f, %14.4f, %14.4f]\n", s, v.x, v.y, v.z);
            v = argument.station.station_val[s].station_vel;
            fprintf(flog, "request arg: station=%02u station_vel= [%14.3E, %14.3E, %14.3E]\n", s, v.x, v.y, v.z);
            v = argument.station.station_val[s].station_acc;
            fprintf(flog, "request arg: station=%02u station_acc= [%14.3E, %14.3E, %14.3E]\n", s, v.x, v.y, v.z);
            v = argument.station.station_val[s].station_pointing_dir;
            fprintf(flog, "request arg: station=%02u station_pointing_dir= [%14.9f, %14.9f, %14.9f]\n", s, v.x, v.y, v.z);
            v = argument.station.station_val[s].station_reference_dir;
            fprintf(flog, "request arg: station=%02u station_reference_dir= [%14.9f, %14.9f, %14.9f]\n", s, v.x, v.y, v.z);
            fprintf(flog, "request arg: station=%02u pointing_coord_frame='%s'\n", s, argument.station.station_val[s].pointing_coord_frame);
            fprintf(flog, "request arg: station=%02u pointing_corrections_applied=%d\n", s, argument.station.station_val[s].pointing_corrections_applied);
            fprintf(flog, "request arg: station=%02u station_position_delay_offset=%E\n", s, argument.station.station_val[s].station_position_delay_offset);
            fprintf(flog, "request arg: station=%02u axis_off=%7.4f primary_axis_wrap=%2d secondary_axis_wrap=%2d\n", s, argument.station.station_val[s].axis_off, argument.station.station_val[s].primary_axis_wrap, argument.station.station_val[s].secondary_axis_wrap);
            fprintf(flog, "request arg: station=%02u receiver_name='%s'\n", s, argument.station.station_val[s].receiver_name);
            fprintf(flog, "request arg: station=%02u pressure=%12.3E antenna_pressure=%12.3E temperature=%6.1f\n", s, argument.station.station_val[s].pressure, argument.station.station_val[s].antenna_pressure, argument.station.station_val[s].temperature);
            fprintf(flog, "request arg: station=%02u wind_speed=%6.1f wind_direction=%7.2f antenna_phys_temperature=%6.1f\n", s, argument.station.station_val[s].wind_speed, argument.station.station_val[s].wind_direction, argument.station.station_val[s].antenna_phys_temperature);
        }
        fprintf(flog, "Source information\n");
        fprintf(flog, "request arg: Use_Server_Source_Table=%d Num_Sources=%d\n", argument.Use_Server_Source_Table, argument.Num_Sources);
        for(s=0; s < argument.Num_Sources; s++) {
            DIFX_DELAY_SERVER_vec v;
            fprintf(flog, "request arg: source=%02u source_name='%s'\n", s, argument.source.source_val[s].source_name);
            fprintf(flog, "request arg: source=%02u IAU_name=   '%s'\n", s, argument.source.source_val[s].IAU_name);
            fprintf(flog, "request arg: source=%02u source_type='%s'\n", s, argument.source.source_val[s].source_type);
            fprintf(flog, "request arg: source=%02u ra=           %20.16f\n", s, argument.source.source_val[s].ra);
            fprintf(flog, "request arg: source=%02u dec=          %20.16f\n", s, argument.source.source_val[s].dec);
            fprintf(flog, "request arg: source=%02u dra=          %20.10f\n", s, argument.source.source_val[s].dra);
            fprintf(flog, "request arg: source=%02u ddec=         %20.10f\n", s, argument.source.source_val[s].ddec);
            fprintf(flog, "request arg: source=%02u depoch=       %20.16f\n", s, argument.source.source_val[s].depoch);
            fprintf(flog, "request arg: source=%02u parallax=     %20.3f\n", s, argument.source.source_val[s].parallax);
            fprintf(flog, "request arg: source=%02u coord_frame= '%s'\n", s, argument.source.source_val[s].coord_frame);
            v = argument.source.source_val[s].source_pos;
            fprintf(flog, "request arg: source=%02u source_pos= [%24.15E, %24.15E, %24.15E]\n", s, v.x, v.y, v.z);
            v = argument.source.source_val[s].source_vel;
            fprintf(flog, "request arg: source=%02u source_vel= [%24.15E, %24.15E, %24.15E]\n", s, v.x, v.y, v.z);
            v = argument.source.source_val[s].source_acc;
            fprintf(flog, "request arg: source=%02u source_acc= [%24.15E, %24.15E, %24.15E]\n", s, v.x, v.y, v.z);
            v = argument.source.source_val[s].source_pointing_dir;
            fprintf(flog, "request arg: source=%02u source_pointing_dir= [%24.15E, %24.15E, %24.15E]\n", s, v.x, v.y, v.z);
            v = argument.source.source_val[s].source_pointing_reference_dir;
            fprintf(flog, "request arg: source=%02u source_pointing_reference_dir= [%24.15E, %24.15E, %24.15E]\n", s, v.x, v.y, v.z);
        }
        fprintf(flog, "EOP information\n");
        fprintf(flog, "request arg: Use_Server_EOP_Table=%d Num_EOPs=%d\n", argument.Use_Server_EOP_Table, argument.Num_EOPs);
        for(e=0; e < argument.Num_EOPs; e++) {
            fprintf(flog, "request arg: EOP=%02u EOP_time=  %20.11f\n", e, argument.EOP.EOP_val[e].EOP_time);
            fprintf(flog, "request arg: EOP=%02u tai_utc=   %20.12f\n", e, argument.EOP.EOP_val[e].tai_utc);
            fprintf(flog, "request arg: EOP=%02u ut1_utc=   %20.12f\n", e, argument.EOP.EOP_val[e].ut1_utc);
            fprintf(flog, "request arg: EOP=%02u xpole=     %10.6f\n", e, argument.EOP.EOP_val[e].xpole);
            fprintf(flog, "request arg: EOP=%02u ypole=     %10.6f\n", e, argument.EOP.EOP_val[e].ypole);
        }
    }

    /* make sure the results area is clean */
    memset(&result.getDIFX_DELAY_SERVER_1_res_u.response, 0, sizeof(result.getDIFX_DELAY_SERVER_1_res_u.response));
    if((argument.Num_Stations != NUM_STATIONS)
      || (argument.Num_Sources != NUM_SOURCES))
    {
        free(memory);
        NUM_STATIONS = argument.Num_Stations;
        NUM_SOURCES = argument.Num_Sources;
        MEMORY_SIZE = sizeof(DIFX_DELAY_SERVER_1_RESULTS) * NUM_STATIONS * NUM_SOURCES;
        memory = malloc(MEMORY_SIZE);
        if(memory == NULL)
        {
            fprintf(flog, "Unable to malloc %"PRIXLEAST64" bytes\n", (uint_least64_t)MEMORY_SIZE);
            exit(EXIT_FAILURE);
        }
    }
    memset(memory, 0, MEMORY_SIZE);
    result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Stations = NUM_STATIONS;
    result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources = NUM_SOURCES;
    result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_len = NUM_STATIONS*NUM_SOURCES;
    result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val = memory;

    /* Figure out which delay server to call */
    handler_error_code = 0;
    if(!my_error_code)
    {
        switch(argument.delay_server) {
        case CALCPROG:
            handler_error_code = difx_delay_server_prog_1_CALCPROG(&argument, &result.getDIFX_DELAY_SERVER_1_res_u.response);
            break;
        case CALC_9_1_RAPROG:
            handler_error_code = difx_delay_server_prog_1_CALC_9_1_RAPROG(&argument, &result.getDIFX_DELAY_SERVER_1_res_u.response);
            break;
        case DIFX_DELAY_SERVER_PROG:
            fprintf(flog, "Recursive calling of this delay server is not allowed\n");
            my_error_code = 5;
            break;
        default:
            fprintf(flog, "Unknown delay_server code 0x%lX\n", argument.delay_server);
            my_error_code = 6;
        }
        if((handler_error_code))
        {
            fprintf(flog, "Error code %d returned by handler\n", handler_error_code);
            my_error_code = 7;
        }
    }
    result.getDIFX_DELAY_SERVER_1_res_u.response.delay_server_error = my_error_code;
    if((!handler_error_code) && (argument.verbosity >= 2))
    {
        unsigned int st, so;
        fprintf(flog, "Results\n");
        fprintf(flog, "request res: delay_server_error=%d server_error=%d model_error=%d\n", result.getDIFX_DELAY_SERVER_1_res_u.response.delay_server_error, result.getDIFX_DELAY_SERVER_1_res_u.response.server_error, result.getDIFX_DELAY_SERVER_1_res_u.response.model_error);
        fprintf(flog, "request res: request_id=%ld delay_server=0x%lX server_struct_setup_code=0x%lX\n", result.getDIFX_DELAY_SERVER_1_res_u.response.request_id, result.getDIFX_DELAY_SERVER_1_res_u.response.delay_server, result.getDIFX_DELAY_SERVER_1_res_u.response.server_struct_setup_code);
        fprintf(flog, "request res: date=%ld time=%.15f\n", result.getDIFX_DELAY_SERVER_1_res_u.response.date, result.getDIFX_DELAY_SERVER_1_res_u.response.time);
        fprintf(flog, "request res: unix_utc_seconds_0=0x%lX unix_utc_seconds_1=0x%lX utc_second_fraction=%.15f\n", result.getDIFX_DELAY_SERVER_1_res_u.response.unix_utc_seconds_0, result.getDIFX_DELAY_SERVER_1_res_u.response.unix_utc_seconds_1, result.getDIFX_DELAY_SERVER_1_res_u.response.utc_second_fraction);
        for(st=0; st < result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Stations; ++st)
        {
            for(so=0; so < result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources; ++so)
            {
                fprintf(flog, "request res: station=%02u source=%02u delay=%24.15E\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].delay);
                fprintf(flog, "request res: station=%02u source=%02u dry_atmos=%24.15E\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].dry_atmos);
                fprintf(flog, "request res: station=%02u source=%02u wet_atmos=%24.15E\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].wet_atmos);
                fprintf(flog, "request res: station=%02u source=%02u az=%10.6f\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].az);
                fprintf(flog, "request res: station=%02u source=%02u el=%10.6f\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].el);
                fprintf(flog, "request res: station=%02u source=%02u primary_axis_angle=%10.6f\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].primary_axis_angle);
                fprintf(flog, "request res: station=%02u source=%02u secondary_axis_angle=%10.6f\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].secondary_axis_angle);
                fprintf(flog, "request res: station=%02u source=%02u mount_source_angle=%10.6f\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].mount_source_angle);
                fprintf(flog, "request res: station=%02u source=%02u station_antenna_theta=%10.6f\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].station_antenna_theta);
                fprintf(flog, "request res: station=%02u source=%02u station_antenna_phi=%10.6f\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].station_antenna_phi);
                fprintf(flog, "request res: station=%02u source=%02u source_antenna_theta=%10.6f\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].source_antenna_theta);
                fprintf(flog, "request res: station=%02u source=%02u source_antenna_phi=%10.6f\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].source_antenna_phi);
                fprintf(flog, "request res: station=%02u source=%02u UVW = [%24.15E, %24.15E, %24.15E]\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].UVW.x, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].UVW.y, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].UVW.z);
                fprintf(flog, "request res: station=%02u source=%02u baselineP2000 = [%24.15E, %24.15E, %24.15E]\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].baselineP2000.x, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].baselineP2000.y, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].baselineP2000.z);
                fprintf(flog, "request res: station=%02u source=%02u baselineV2000 = [%24.15E, %24.15E, %24.15E]\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].baselineV2000.x, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].baselineV2000.y, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].baselineV2000.z);
                fprintf(flog, "request res: station=%02u source=%02u baselineA2000 = [%24.15E, %24.15E, %24.15E]\n", st, so, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].baselineA2000.x, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].baselineA2000.y, result.getDIFX_DELAY_SERVER_1_res_u.response.result.result_val[st*result.getDIFX_DELAY_SERVER_1_res_u.response.Num_Sources+so].baselineA2000.z);
            }
        }
    }

    if(((handler_error_code)) || ((my_error_code)))
    {
        result.this_error = 1;
    }
    else {
        result.this_error = 0;
    }
            

    /* return result to client */

    if (!svc_sendreply (pTransport, (xdrproc_t) xdr_getDIFX_DELAY_SERVER_1_res, (char *)&result))
    {
        fprintf(flog, "Cound not send getDIFX_DELAY_SERVER_1_res results back\n");
        svcerr_systemerr (pTransport);
    }

    /* free any memory allocated by xdr routines when argument was decoded */
    if (!svc_freeargs (pTransport, (xdrproc_t) xdr_getDIFX_DELAY_SERVER_1_arg, (char *)&argument))
	{
        fprintf(flog, "Cound not free getDIFX_DELAY_SERVER_1_arg memory\n");
        syslog (LOG_ERR, "unable to free arguments %m\n");
        exit(EXIT_FAILURE);
	}
    return;
}

/*++****************************************************************************
 */
int main (int argc, char *argv[])
/*
 * This program is a remote procedure call(RPC) server for the GSFC
 * CALC_9_1_RA Program. The current client is the VLBA Correlator on-line
 * computer. The CALC_9_1_RA Server is called from within the modlTask, and
 * replaces running the CALC_9_1_RA program itself in the on-line computer.
 -*/
{
    SVCXPRT *pTransport;    /* transport handle */
    time_t  timep, *tp;
    const char *filename;

    /* Save the Server startup time and version number for the log file */
    tp = &timep;
    time (tp);
    strncpy(spawn_time, ctime(tp), SPAWN_TIME_SIZE);
    spawn_time[SPAWN_TIME_SIZE-1] = 0;

    /* open system log file to send messages */
    openlog ("DiFX_Delay_Server", LOG_PID, LOG_USER);

    /* if logfile name is specified in argv[1], use it instead of datetime
     * logfile name that is opened later in the program */
    if (argc > 1)
    {
        filename = argv[1];
        if (flog != 0)
        {
            fclose(flog);
        }
        if((argv[1][0] == '-') && (argv[1][1] == 0))
        {
            flog = stdout;
        }
        else
        {
            if ((flog = fopen (filename, "w")) == NULL)
            {
                fprintf(stderr, "ERROR: cannot open log file %s\n", filename);
                exit(EXIT_FAILURE);
            }
        }
        printf("opened new DiFX_Delay_Server log file %s\n", filename);
        ilogdate = 0;
    }

    /* un-register service with portmapper in case one already exists */
    (void) pmap_unset (DIFX_DELAY_SERVER_PROG, DIFX_DELAY_SERVER_VERS_1);

    /* create TCP transport handle */
    if ((pTransport = svctcp_create (RPC_ANYSOCK, 0, 0)) == NULL)
	{
		fprintf (stderr, "cannot create tcp service\n");
        syslog (LOG_ERR, "cannot create tcp service %m\n");
        exit(EXIT_FAILURE);
	}

    /* register service with portmapper */
    if (svc_register (pTransport, DIFX_DELAY_SERVER_PROG, DIFX_DELAY_SERVER_VERS_1, difx_delay_server_prog_1, IPPROTO_TCP) == 0)
	{
		fprintf (stderr, "unable to register (DIFX_DELAY_SERVER_PROG=0x%X, DIFX_DELAY_SERVER_VERS_1=%d, tcp)\n", DIFX_DELAY_SERVER_PROG, DIFX_DELAY_SERVER_VERS_1);
        syslog (LOG_ERR, "unable to register service %m\n");
        exit(EXIT_FAILURE);
	}
    /*
    if (svc_register (pTransport, DIFX_DELAY_SERVER_PROG, DIFX_DELAY_SERVER_VERS_2, difx_delay_server_prog_2, IPPROTO_TCP) == 0)
	{
		fprintf (stderr, "unable to register (DIFX_DELAY_SERVER_PROG=0x%X, DIFX_DELAY_SERVER_VERS_2=%d, tcp)\n", DIFX_DELAY_SERVER_PROG, DIFX_DELAY_SERVER_VERS_2);
        syslog (LOG_ERR, "unable to register service %m\n");
        exit(EXIT_FAILURE);
	}
    */

    ifirst = 1;

    /* dispatch RPC client calls */
    svc_run ();    /* should never return */

    /* svc_run() returned ??? */
    svc_destroy (pTransport);
    syslog (LOG_ERR, "svc_run returned %m\n");

    return EXIT_SUCCESS;
}

