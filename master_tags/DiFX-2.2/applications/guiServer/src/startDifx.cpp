//=============================================================================
//
//   ServerSideConnection::startDifx Function (and associated functions)
//
//!  Called when an instruction to start a DiFX job is received.  This function
//!  is a member of the ServerSideConnection class.
//!
//!  Much of this code is swiped directly from the Mk5Daemon_startMpifxcorr()
//!  function of mk5daemon, with many thanks to Walter Brisken.
//
//=============================================================================
#include <ServerSideConnection.h>
#include <sys/statvfs.h>
#include <network/TCPClient.h>
#include <JobMonitorConnection.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <set>
#include <vector>
#include <fcntl.h>
#include <ExecuteSystem.h>
#include <configuration.h>

using namespace guiServer;

//-----------------------------------------------------------------------------
//!  Called in response to a user request to start a DiFX session.  This function
//!  does all of the necessary checking to assure (or at least increase the chances)
//!  that DiFX will run.  It also creates .thread and .machine files.  The actual
//!  running of DiFX is performed in a thread that this function, as its last act,
//!  spawns.
//-----------------------------------------------------------------------------
void ServerSideConnection::startDifx( DifxMessageGeneric* G ) {
	const int RestartOptionLength = 16;
	int l;
	char filebase[DIFX_MESSAGE_FILENAME_LENGTH];
	char filename[DIFX_MESSAGE_FILENAME_LENGTH];
	char workingDir[DIFX_MESSAGE_FILENAME_LENGTH];
	char machinesFilename[DIFX_MESSAGE_FILENAME_LENGTH];
	char restartOption[RestartOptionLength];
	char monitorOption[DIFX_MESSAGE_FILENAME_LENGTH];
	char inLine[MAX_COMMAND_SIZE];
	const char *jobName;
	const DifxMessageStart *S;
	bool outputExists = false;
	const char *mpiOptions;
	const char *mpiWrapper;
	JobMonitorConnection* jobMonitor;

	//  Cast the body of this message to a DifxMessageStart structure.
	S = &G->body.start;

	//  Open a TCP socket connection to the server that should be running for us on the
    //  host that requested this job start (the GUI, presumably).  This socket is used
    //  for diagnostic messages and to show progress on this specific job.  It can
    //  also be used for some rudimentary control of the job.
    network::TCPClient* guiSocket = new network::TCPClient( S->address, S->port );
    guiSocket->waitForConnect();
    //  Create a Job Monitor Connection out of this new socket if it connected properly.
    //  If it did not connect, we need to bail out now.
    if ( guiSocket->connected() ) {
        jobMonitor = new JobMonitorConnection( guiSocket );
        jobMonitor->sendPacket( JobMonitorConnection::JOB_STARTED, NULL, 0 );
    }
    else {
        diagnostic( ERROR, "client socket connection from guiServer to GUI failed - unable to start job" );
        return;
    }
    
    //=========================================================================
    //  Checks below are run BEFORE we start the thread that actually runs
    //  the job.  Why?  Because we can only count on the viability of the
    //  DifxMessageGeneric structure in this function call.  The thread can't
    //  use it.
    //=========================================================================
    jobMonitor->sendPacket( JobMonitorConnection::PARAMETER_CHECK_IN_PROGRESS, NULL, 0 );

	//  Make sure all needed parameters are included in the message.
	if ( S->headNode[0] == 0 ) {
    	diagnostic( ERROR, "DiFX start failed - no headnode specified." );
    	jobMonitor->sendPacket( JobMonitorConnection::FAILURE_NO_HEADNODE, NULL, 0 );
        jobMonitor->sendPacket( JobMonitorConnection::JOB_TERMINATED, NULL, 0 );
		return;
	}
	if ( S->nDatastream <= 0 ) {
    	diagnostic( ERROR, "DiFX start failed - no data sources specified." );
    	jobMonitor->sendPacket( JobMonitorConnection::FAILURE_NO_DATASOURCES, NULL, 0 );
        jobMonitor->sendPacket( JobMonitorConnection::JOB_TERMINATED, NULL, 0 );
		return;
	}
	if ( S->nProcess <= 0 ) {
    	diagnostic( ERROR, "DiFX start failed - no processing nodes  specified." );
    	jobMonitor->sendPacket( JobMonitorConnection::FAILURE_NO_PROCESSORS, NULL, 0 );
        jobMonitor->sendPacket( JobMonitorConnection::JOB_TERMINATED, NULL, 0 );
		return;
	}
	if ( S->inputFilename[0] == 0 ) {
    	diagnostic( ERROR, "DiFX start failed - no input file specified" );
    	jobMonitor->sendPacket( JobMonitorConnection::FAILURE_NO_INPUTFILE_SPECIFIED, NULL, 0 );
        jobMonitor->sendPacket( JobMonitorConnection::JOB_TERMINATED, NULL, 0 );
		return;
	}

	//  Check to make sure the input file exists
	if( access(S->inputFilename, R_OK) != 0 ) {
		diagnostic( ERROR, "DiFX start failed - input file %s not found.", S->inputFilename );
    	jobMonitor->sendPacket( JobMonitorConnection::FAILURE_INPUTFILE_NOT_FOUND, NULL, 0 );
        jobMonitor->sendPacket( JobMonitorConnection::JOB_TERMINATED, NULL, 0 );
		return;
	}

    //  Make sure the filename can fit in our allocated space for such things.
	if( strlen( S->inputFilename ) + 12 > DIFX_MESSAGE_FILENAME_LENGTH ) {
		diagnostic( ERROR, "Filename %s is too long.", S->inputFilename );
    	jobMonitor->sendPacket( JobMonitorConnection::FAILURE_INPUTFILE_NAME_TOO_LONG, NULL, 0 );
        jobMonitor->sendPacket( JobMonitorConnection::JOB_TERMINATED, NULL, 0 );
		return;
	}
	
    jobMonitor->sendPacket( JobMonitorConnection::PARAMETER_CHECK_SUCCESS, NULL, 0 );

    //  Find the "working directory" (where the .input file resides and data will be put), the
    //  "filebase" (the path of the input file with ".input") and the "job name" (the name of
    //  the input file without .input or directory path).
	strcpy( workingDir, S->inputFilename );
    l = strlen( workingDir );
    for( int i = l-1; i > 0; i-- ) {
        if( workingDir[i] == '/') {
    		workingDir[i] = 0;
    		break;
    	}
    }	
	strcpy( filebase, S->inputFilename );
	l = strlen( filebase );
	for( int i = l-1; i > 0; i-- ) {
		if( filebase[i] == '.' ) {
			filebase[i] = 0;
			break;
		}
	}
	jobName = filebase;
	for( int i = 0; filebase[i]; ++i ) {
		if( filebase[i] == '/' ) {
			jobName = filebase + i + 1;
		}
	}

	//  Make sure the working directory exists and has enough free space
	struct statvfs fiData;
	int v = statvfs( workingDir, &fiData );
	if( v == 0 ) {
		long long freeSpace;
		freeSpace = fiData.f_bsize * fiData.f_bavail;
		if( freeSpace < 100000000 ) {
			diagnostic( WARNING, 
			    "Working directory %s has only %lld bytes free - mpifxcorr will likely crash!", 
				workingDir, freeSpace );
		}
		else if( fiData.f_ffree < 3 )
		{
			diagnostic( WARNING, "%s has no free inodes - mpifxcorr will likely crash!", 
				workingDir );
		}
	}
	else {
		diagnostic( ERROR, 
		    "statvfs failed when accessing directory %s : it seems not to exist!", 
			workingDir );
		return;
	}
	
	//  See if the output file already exists.
	snprintf( filename, DIFX_MESSAGE_FILENAME_LENGTH, "%s.difx", filebase );
	if( access( filename, F_OK ) == 0 )
	    outputExists = true;
    //  Should we be over writing it?  If not, we need to bail out.
	if( outputExists && !S->force ) {
		diagnostic( ERROR, "Output file %s exists.  Aborting correlation.", 
			filename );
        jobMonitor->sendPacket( JobMonitorConnection::FAILURE_OUTPUT_EXISTS, NULL, 0 );
        jobMonitor->sendPacket( JobMonitorConnection::JOB_TERMINATED, NULL, 0 );
		return;
	}

	//  Create a structure to hold all information about this job for the thread that will
	//  run it.
	DifxStartInfo* startInfo = new DifxStartInfo;
	startInfo->ssc = this;
	startInfo->jobMonitor = jobMonitor;
	strncpy( startInfo->filebase, filebase, MAX_COMMAND_SIZE );
	startInfo->filebase[strlen( filebase )] = 0;
	strncpy( startInfo->inputFile, S->inputFilename, DIFX_MESSAGE_FILENAME_LENGTH );
	
	//  Sanity check of the .machines file.  We use this to get the "np" value for mpirun
	//  as well.
	int procCount = 0;
    strncpy( machinesFilename, S->inputFilename, DIFX_MESSAGE_FILENAME_LENGTH );
    l = strlen( machinesFilename );
    for( int i = l-1; i > 0; i-- ) {
	    if( machinesFilename[i] == '.' ) {
		    machinesFilename[i] = 0;
		    break;
	    }
    }
    strncat( machinesFilename, ".machines", DIFX_MESSAGE_FILENAME_LENGTH );
    FILE* inp = fopen( machinesFilename, "r" );
    if ( inp != NULL ) {
        while ( fgets( inLine, MAX_COMMAND_SIZE, inp ) ) {
            if ( strlen( inLine ) > 0 )
                ++procCount;
        }
        fclose( inp );
    }
	
	snprintf( startInfo->jobName, MAX_COMMAND_SIZE, "%s", filebase );
	for ( int i = 0; filebase[i]; ++i ) {
	    if ( filebase[i] == '/' )
	        snprintf( startInfo->jobName, MAX_COMMAND_SIZE, "%s", filebase + i + 1 );
	}
	
	//  Build the "remove" command - this removes existing data if the force option is picked.
	if( S->force && outputExists ) {
		snprintf( startInfo->removeCommand, MAX_COMMAND_SIZE, "/bin/rm -rf %s.difx/", filebase );
		startInfo->force = 1;
	}
	else
	    startInfo->force = 0;
	    
	//  Include any options specified by the user - or use the defaults.
	if( S->mpiOptions[0] )
	    mpiOptions = S->mpiOptions;
 	else
 	    mpiOptions = "--mca mpi_yield_when_idle 1 --mca rmaps seq";

    //  Option to run a different version of mpirun.
	if( S->mpiWrapper[0] )
	    mpiWrapper = S->mpiWrapper;
	else
		mpiWrapper = "mpirun";

    //  Option to run different DiFX commands.
	if( S->difxProgram[0] )
		snprintf( startInfo->difxProgram, DIFX_MESSAGE_FILENAME_LENGTH, "%s", S->difxProgram );
	else
		snprintf( startInfo->difxProgram, DIFX_MESSAGE_FILENAME_LENGTH, "mpifxcorr" );
	    
	//  Build the "restart" instruction.  This becomes part of the start command.
	if ( S->restartSeconds > 0.0 )
	    snprintf( restartOption, RestartOptionLength, "-r %f", S->restartSeconds );
	else
	    restartOption[0] = 0;

	//  Include the option to run with the difx_monitor scheme.
	int monitorServerPort = 9999;  //  this should eventually be variable
	if ( S->function == DIFX_START_FUNCTION_RUN_MONITOR ) {
        startInfo->runMonitor = 1;
/* 
	    //  See if the "monitor_server" program is running (with the proper port).  If not,
	    //  start it up.
	    char monCommand[ MAX_COMMAND_SIZE ];
	    char message[ DIFX_MESSAGE_LENGTH ];
        snprintf( monCommand, MAX_COMMAND_SIZE, "/bin/ps -ef | /bin/grep -E \'monitor_server\\s|monitor_server$\' | /bin/grep -v grep" );
        ExecuteSystem* executor = new ExecuteSystem( monCommand );
        bool found = false;
        if ( executor->pid() > -1 ) {
            while ( int ret = executor->nextOutput( message, DIFX_MESSAGE_LENGTH ) ) {
                if ( ret == 1 ) { // stdout
                    //  Any instance of monitor_server should show up as a result of this
                    //  "ps" command.  Monitor_server is started with the port number as
                    //  the last (and only) argument.
                    int portNum = -1;
                    int len = strlen( message );
                    while ( len >= 0 ) {
                        if ( message[len] == ' ' ) {
                            portNum = atoi( message + len + 1 );
                            break;
                        }
                        --len;
                    }
                    if ( portNum == monitorServerPort ) {
                        diagnostic( INFORMATION, "located monitor_server instance: %s\n", message );
                        found = true;
                    }
                }
            }
        }
        //  Do we need to start a new version of the monitor_server?
        if ( !found ) {
            snprintf( monCommand, MAX_COMMAND_SIZE, "source %s; monitor_server %d &", _difxSetupPath, monitorServerPort );
            diagnostic( WARNING, "monitor_server needs to be started - executing: %s\n", monCommand );
            system( monCommand );
        }
*/ 
        //  Build the "monitor" instruction.  This becomes part of the start command.  The
        //  monitor is run on a "host" (which can be "localhost", i.e. the headnonde) with
        //  a port number.
        snprintf( monitorOption, DIFX_MESSAGE_FILENAME_LENGTH, "-Mlocalhost:%d", monitorServerPort );
	}
	else
	    monitorOption[0] = 0;
	    
	//  Build the actual start command.
	snprintf( startInfo->startCommand, MAX_COMMAND_SIZE, 
	          "source %s; %s -np %d --bynode --hostfile %s.machines %s %s %s %s %s", 
              _difxSetupPath,
              mpiWrapper,
              procCount,
              filebase,
              mpiOptions,
              startInfo->difxProgram,
              restartOption,
              S->inputFilename, monitorOption );
	
	//  Start a thread to run difx and report back stdout/stderr feedback.
    pthread_attr_t threadAttr;
    pthread_t threadId;
    pthread_attr_init( &threadAttr );
    pthread_create( &threadId, &threadAttr, staticRunDifxThread, (void*)startInfo );
                   	
}

//-----------------------------------------------------------------------------
//!  Thread to actually run DiFX.  Running as a thread allows us to return the
//!  main thread to listen to other commands/messages.  This thread monitors
//!  all output from the running DiFX job and returns messages to the user
//!  interface.
//-----------------------------------------------------------------------------
void ServerSideConnection::runDifxThread( DifxStartInfo* startInfo ) {
	char message[DIFX_MESSAGE_LENGTH];
	
    //  Add this job to the "running" jobs list.
    addRunningJob( startInfo->inputFile );

    //  Delete data directories if "force" is in effect.
    if ( startInfo->force ) {
	    startInfo->jobMonitor->sendPacket( JobMonitorConnection::DELETING_PREVIOUS_OUTPUT, NULL, 0 );
        system( startInfo->removeCommand );
    }
    
    //  Run the data monitoring thread (see "runDifxThread" below).
    /*
    if ( startInfo->runMonitor ) {
        pthread_attr_t threadAttr;
        pthread_t threadId;
        pthread_attr_init( &threadAttr );
        pthread_create( &threadId, &threadAttr, staticRunDifxMonitor, (void*)startInfo );
    }
    */
        
    //  Run the DiFX process!
    diagnostic( WARNING, "executing: %s\n", startInfo->startCommand );
    printf( "%s\n", startInfo->startCommand );
	startInfo->jobMonitor->sendPacket( JobMonitorConnection::STARTING_DIFX, NULL, 0 );
	bool noErrors = true;  //  track whether any errors occured
	bool stillGoing = true;  //  track whether job still ran despite errors
    ExecuteSystem* executor = new ExecuteSystem( startInfo->startCommand );
    if ( executor->pid() > -1 ) {
        while ( int ret = executor->nextOutput( message, DIFX_MESSAGE_LENGTH ) ) {
            if ( ret == 1 ) { // stdout
                diagnostic( INFORMATION, "%s... %s", startInfo->difxProgram, message );
                startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_MESSAGE, message, strlen( message ) );
                stillGoing = true;
            }
            else {            // stderr
                //  Only report errors if this is still a running job!
                if ( runningJobExists( startInfo->inputFile ) ) {
                    //  See if this message contains the string "warning" (multiple case situations)
                    //  indicating that....it is a warning.
                    if ( strcasestr( message, "WARNING" ) != NULL ) {
                        diagnostic( WARNING, "%s... %s", startInfo->difxProgram, message );
                        startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_WARNING, message, strlen( message ) );
                    }
                    else {
                        diagnostic( ERROR, "%s... %s", startInfo->difxProgram, message );
	                    startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_ERROR, message, strlen( message ) );
	                    noErrors = false;
	                    stillGoing = false;
                    }
                }
            }
        }
        if ( noErrors ) {
            if ( runningJobExists( startInfo->inputFile ) ) {
                diagnostic( WARNING, "%s complete", startInfo->difxProgram );
                difxMessageSendDifxStatus2( startInfo->jobName, DIFX_STATE_MPIDONE, "" );
        		startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_COMPLETE, NULL, 0 );
                startInfo->jobMonitor->sendPacket( JobMonitorConnection::JOB_ENDED_GRACEFULLY, NULL, 0 );
            } else {
                diagnostic( WARNING, "%s terminated by user", startInfo->difxProgram );
                startInfo->jobMonitor->sendPacket( JobMonitorConnection::JOB_TERMINATED, NULL, 0 );
            }
		}
        else if ( stillGoing ) {
            if ( runningJobExists( startInfo->inputFile ) ) {
                diagnostic( WARNING, "%s complete", startInfo->difxProgram );
                difxMessageSendDifxStatus2( startInfo->jobName, DIFX_STATE_MPIDONE, "" );
        		startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_COMPLETE, NULL, 0 );
                startInfo->jobMonitor->sendPacket( JobMonitorConnection::JOB_ENDED_WITH_ERRORS, NULL, 0 );
            } else {
                diagnostic( WARNING, "%s terminated by user", startInfo->difxProgram );
                startInfo->jobMonitor->sendPacket( JobMonitorConnection::JOB_TERMINATED, NULL, 0 );
            }
		}
        else {
            if ( runningJobExists( startInfo->inputFile ) ) {
                diagnostic( ERROR, "%s FAILED", startInfo->difxProgram );
                snprintf( message, DIFX_MESSAGE_LENGTH, "%s FAILED", startInfo->difxProgram );
                startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_ERROR, message, strlen( message ) );
                startInfo->jobMonitor->sendPacket( JobMonitorConnection::JOB_FAILED, NULL, 0 );
                difxMessageSendDifxStatus2( startInfo->jobName, DIFX_STATE_ABORTING, "" );
            } else {
                diagnostic( WARNING, "%s terminated by user", startInfo->difxProgram );
                startInfo->jobMonitor->sendPacket( JobMonitorConnection::JOB_TERMINATED, NULL, 0 );
            }
        }
    }
    else {
        diagnostic( ERROR, "%s process not started for job %s; popen failed", startInfo->difxProgram, startInfo->jobName );
        snprintf( message, DIFX_MESSAGE_LENGTH, "%s process not started for job %s; popen failed", startInfo->difxProgram, startInfo->jobName );
	    startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_ERROR, message, strlen( message ) );
        startInfo->jobMonitor->sendPacket( JobMonitorConnection::JOB_FAILED, NULL, 0 );
        difxMessageSendDifxStatus2( startInfo->jobName, DIFX_STATE_ABORTING, "" );
    }
    delete executor;

    //  Shut down the monitor thread.
    startInfo->runMonitor = 0;
    
    //  Remove the job from the running job list.
    removeRunningJob( startInfo->inputFile );

}

/*
//-----------------------------------------------------------------------------
//!  Thread to monitor a running DiFX job.  A connection is made to the
//!  "monitor_server" program and data products are requested.  These products
//!  are then provided to the GUI client.
//-----------------------------------------------------------------------------	
void ServerSideConnection::runDifxMonitor( DifxStartInfo* startInfo ) {
	char message[DIFX_MESSAGE_LENGTH];
    char sendData[MAX_COMMAND_SIZE];
    char buffer[1024];
    Configuration* config;
    bool allIsWell = true;

    //  Make a new client connection to the monitor_server, which *should* be running.
    //  At the moment the port is hard-wired to 52300 which might not be a perfect
    //  long term solution.  The host is the local host (so I'm using the loopback
    //  address).
    printf( "making client connection\n" );
    network::TCPClient* monitorServerClient = new network::TCPClient( "127.0.0.1", 52300 );
    monitorServerClient->waitForConnect();
    printf( "connection established!\n" );
    if ( monitorServerClient->connected() ) {
        int status;
        monitorServerClient->reader( (char*)(&status), sizeof( int ) );
        if ( status == 0 ) {
            snprintf( message, DIFX_MESSAGE_LENGTH, "connection established with monitor_server" );
            startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_MONITOR_CONNECTION_ACTIVE, NULL, 0 );
            startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_WARNING, message, strlen( message ) );
        }
        else {
            snprintf( message, DIFX_MESSAGE_LENGTH, "monitor_server returned initial status failure (%d) - real time monitor will not run", status );
            startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_MONITOR_CONNECTION_FAILED, NULL, 0 );
            startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_ERROR, message, strlen( message ) );
        }
    }
    else {
        snprintf( message, DIFX_MESSAGE_LENGTH, "connection with monitor_server failed - real time monitor will not run" );
        startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_MONITOR_CONNECTION_FAILED, NULL, 0 );
        startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_ERROR, message, strlen( message ) );
        allIsWell = false;
    }
    
    //  Generate a list of data products.  This code is basically swiped from Chris Phillips'
    //  difx_config program.
    if ( allIsWell ) {
        config = new Configuration( startInfo->inputFile, 0 );
        if ( !config->consistencyOK() ) {
            snprintf( message, DIFX_MESSAGE_LENGTH, "Configuration had a problem parsing input file %s - real time monitor will not run",
                startInfo->inputFile );
            startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_MONITOR_CONNECTION_FAILED, NULL, 0 );
            startInfo->jobMonitor->sendPacket( JobMonitorConnection::DIFX_ERROR, message, strlen( message ) );
            allIsWell = false;
        }
        else {
            //  Send the possible data products and codes.  This I grabbed (mostly) from the monserver_productconfig()
            //  function in difx_monitor/monserver.cpp.  Without know entirely what I was doing.
            int currentscan = 0;  //  Is there ever more than one??
            int configindex = config->getScanConfigIndex( currentscan );
            char polpair[3];
            polpair[2] = 0;
            printf( "config index is %d\n", configindex );

            int binloop = 1;
            if ( config->pulsarBinOn( configindex ) && !config->scrunchOutputOn( configindex ) )
                binloop = config->getNumPulsarBins( configindex );
            printf( "binloop is %d\n", binloop );
            printf( "num baselines is %d\n", config->getNumBaselines() );

            for ( int i = 0; i < config->getNumBaselines(); i++ ) {
                int ds1index = config->getBDataStream1Index( configindex, i );
                int ds2index = config->getBDataStream2Index( configindex, i );
                printf( "telescopes...%s, %s\n", config->getTelescopeName( ds1index ).c_str(), config->getTelescopeName( ds2index ).c_str() );

                for( int j = 0; j < config->getBNumFreqs( configindex, i ); j++ ) {
                    int freqindex = config->getBFreqIndex( configindex, i, j );
                    int resultIndex = config->getCoreResultBaselineOffset( configindex, freqindex, i );
                    int freqchannels = config->getFNumChannels( freqindex ) / config->getFChannelsToAverage( freqindex );

                    for( int s = 0; s < config->getMaxPhaseCentres( configindex ); s++ ) {
	                    for( int b = 0; b < binloop; b++ ) {
	                        for( int k = 0; k < config->getBNumPolProducts( configindex, i, j ); k++ ) {
	                            config->getBPolPair( configindex, i, j, k, polpair );


	    products.push_back(DIFX_ProdConfig(ds1index,
					       ds2index,
					       config->getTelescopeName(ds1index), 
					       config->getTelescopeName(ds2index),
					       config->getFreqTableFreq(freqindex),
					       config->getFreqTableBandwidth(freqindex),
					       polpair,
					       s,
					       b,
					       resultindex,
					       freqchannels,
					       config->getFreqTableLowerSideband(freqindex)));
                    printf( "frequency....%.1f\n", config->getFreqTableFreq( freqindex ) );
                                printf( "result index is %d\n", resultIndex );
	                            resultIndex += freqchannels;
                            }
                        }
                    }
                }
            }
        }
            
    }

    bool keepGoing = true;
    while ( keepGoing ) {
        //  Doing this here causes the loop to run one more time after it has been
        //  terminated by the parent function.
        if ( !startInfo->runMonitor )
            keepGoing = false;

        usleep( 1000000 );
        
    }
    
    delete config;
    printf( "closing the monitor socket\n" );
    monitorServerClient->closeConnection();
    delete monitorServerClient;
    printf( "exiting monitor thread\n" );
    
}
*/

/*  This is a previous idea for a monitor.  It was meant to report the size and content
    of output files as DiFX ran.  Might be useful someday.
//-----------------------------------------------------------------------------
//!  Thread to monitor a running DiFX job.  The idea here was to monitor the
//!  size of output files in the .difx directory for this job, and possibly
//!  provide data from them.  At the moment this is not used.
//!
//!  All of the necessary setup for this thread should be
//!  done already - the "startInfo" structure contains all information needed
//!  to run.  
//-----------------------------------------------------------------------------	
void ServerSideConnection::runDifxMonitor( DifxStartInfo* startInfo ) {
    char dataDir[MAX_COMMAND_SIZE];
    char longName[MAX_COMMAND_SIZE];
    char sendData[MAX_COMMAND_SIZE];
    char buffer[1024];
    std::set<std::string> pathSet;
    std::vector<int> fileDescriptors;

    //  Form the difx data directory name.
    snprintf( dataDir, MAX_COMMAND_SIZE, "%s.difx", startInfo->filebase );

    bool keepGoing = true;
    while ( keepGoing ) {
        //  Doing this here causes the loop to run one more time after it has been
        //  terminated by the parent function.
        if ( !startInfo->runMonitor )
            keepGoing = false;

        //  Get a list of all files in the data directory along with their sizes.
        usleep( 1000000 );
        struct dirent **namelist;
        int n = scandir( dataDir, &namelist, 0, alphasort );
        if ( n > 0) {
            while( n-- ) { 
                if ( strcmp( namelist[n]->d_name, "." ) && strcmp( namelist[n]->d_name, ".." ) ) {
                    struct stat buf;
                    snprintf( longName, MAX_COMMAND_SIZE, "%s/%s", dataDir, namelist[n]->d_name );
                    stat( longName, &buf );
                    snprintf( sendData, MAX_COMMAND_SIZE, "%s\n%ld\n", namelist[n]->d_name, buf.st_size );
                    printf( "%s\n", sendData );
                    //startInfo->jobMonitor->sendPacket( JobMonitorConnection::DATA_FILE_SIZE, sendData, strlen( sendData ) );
                    //  See if we've not seen this file before...
                    if ( pathSet.find( namelist[n]->d_name ) == pathSet.end() ) {
                        pathSet.insert( namelist[n]->d_name );
                        //  Open the file for reading and add the new file pointer to our list.
                        int fd = open( longName, O_RDONLY );
                        fileDescriptors.push_back( fd );
                    }
                    free( namelist[n] );
                }
            }
            free( namelist );
        }
        
        //  Do a select on each file descriptor and read any new data available.
        for ( std::vector<int>::iterator iter = fileDescriptors.begin(); iter != fileDescriptors.end(); ++iter ) {
            fd_set rfds;
            int fd = *iter;
            FD_ZERO( &rfds );
            FD_SET( fd, &rfds );
            if ( select( 1, &rfds, NULL, NULL, NULL ) ) {
                while ( read( fd, buffer, 1024 ) ) {
                    //  Send data to the gui!
                }
            }
        }

    }

    //  Close any file descriptors...
    for ( std::vector<int>::iterator iter = fileDescriptors.begin(); iter != fileDescriptors.end(); ++iter ) {
        close( *iter );
    }    
    
}
*/

