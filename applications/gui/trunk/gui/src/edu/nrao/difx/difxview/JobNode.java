/***************************************************************************
 *   Copyright (C) 2016 by John Spitzak                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
package edu.nrao.difx.difxview;

import mil.navy.usno.widgetlib.BrowserNode;
import mil.navy.usno.widgetlib.ActivityMonitorLight;
import mil.navy.usno.widgetlib.ZMenuItem;
import mil.navy.usno.widgetlib.JulianCalendar;

import java.util.Date;

import mil.navy.usno.plotlib.PlotWindow;
import mil.navy.usno.plotlib.Plot2DObject;
import mil.navy.usno.plotlib.Track2D;

import edu.nrao.difx.difxutilities.DiFXCommand_rm;

import javax.swing.JButton;
import javax.swing.JPopupMenu;
import javax.swing.JMenuItem;
import javax.swing.JProgressBar;
import javax.swing.JSeparator;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import java.util.List;
import java.util.Iterator;
import java.awt.Point;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.Color;
import java.awt.Component;

import java.text.NumberFormat;
import java.math.RoundingMode;

import java.io.File;

import java.util.Vector;
import java.util.Iterator;

import edu.nrao.difx.xmllib.difxmessage.DifxMessage;
import edu.nrao.difx.xmllib.difxmessage.DifxAlert;
import edu.nrao.difx.xmllib.difxmessage.DifxStatus;
import edu.nrao.difx.difxutilities.DiFXCommand_getFile;

import edu.nrao.difx.difxdatabase.QueueDBConnection;
import javax.swing.JOptionPane;

public class JobNode extends QueueBrowserNode {
    
    public JobNode( String name, SystemSettings settings ) {
        super( name );
        this.setHeight( 20 );
        _columnColor = Color.LIGHT_GRAY;
        _settings = settings;
        _this = this;
        _autostate = new Integer( AUTOSTATE_UNDETERMINED );
        //  Override the selection button action to look for shift events.
        //  Remove old ones.
        ActionListener[] actionListeners = _selectedButton.getActionListeners();
        for ( int i = 0; i < actionListeners.length; ++i )
            _selectedButton.removeActionListener( actionListeners[i] );
        _selectedButton.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                selectionButtonAction();
                checkForShift();
            }
        });
    }
    
    @Override
    public void createAdditionalItems() {
        addSelectionButton( null, null );
        //  Create a popup menu appropriate to a "job".
        _networkActivity = new ActivityMonitorLight();
        _networkActivity.warningTime( 0 );
        _networkActivity.alertTime( 0 );
        showNetworkActivity( true );
        this.add( _networkActivity );
        _state = new ColumnTextArea();
        _state.justify( ColumnTextArea.CENTER );
        _state.setText( "not started" );
        _state.setToolTipText( "" );
        this.add( _state );
        _progress = new JProgressBar( 0, 100 );
        _progress.setValue( 0 );
        _progress.setStringPainted( true );
        this.add( _progress );
        _experiment = new ColumnTextArea();
        _experiment.justify( ColumnTextArea.RIGHT );
        _experiment.setText( "" );
        showExperiment( false );
        this.add( _experiment );
        _pass = new ColumnTextArea();
        _pass.justify( ColumnTextArea.RIGHT );
        _pass.setText( "" );
        showPass( false );
        this.add( _pass );
        _priority = new ColumnTextArea();
        _priority.justify( ColumnTextArea.RIGHT );
        _priority.setText( "" );
        showPriority( false );
        this.add( _priority );
        _queueTime = new ColumnTextArea();
        _queueTime.justify( ColumnTextArea.RIGHT );
        _queueTime.setText( "" );
        showQueueTime( true );
        this.add( _queueTime );
        _correlationStart = new ColumnTextArea();
        _correlationStart.justify( ColumnTextArea.RIGHT );
        _correlationStart.setText( "" );
        showCorrelationStart( false );
        this.add( _correlationStart );
        _correlationEnd = new ColumnTextArea();
        _correlationEnd.justify( ColumnTextArea.RIGHT );
        _correlationEnd.setText( "" );
        showCorrelationEnd( false );
        this.add( _correlationEnd );
        _correlationTime = new ColumnTextArea();
        _correlationTime.justify( ColumnTextArea.RIGHT );
        _correlationTime.setText( "" );
        showCorrelationTime( false );
        this.add( _correlationTime );
        _jobStartText = new ColumnTextArea();
        _jobStartText.justify( ColumnTextArea.RIGHT );
        _jobStartText.setText( "" );
        showJobStart( true );
        this.add( _jobStartText );
        _jobDurationText = new ColumnTextArea();
        _jobDurationText.justify( ColumnTextArea.RIGHT );
        _jobDurationText.setText( "" );
        showJobDuration( true );
        this.add( _jobDurationText );
        _inputFile = new ColumnTextArea();
        _inputFile.justify( ColumnTextArea.RIGHT );
        _inputFile.setText( "" );
        showInputFile( true );
        this.add( _inputFile );
        _outputFile = new ColumnTextArea();
        _outputFile.justify( ColumnTextArea.RIGHT );
        _outputFile.setText( "" );
        showOutputFile( false );
        this.add( _outputFile );
        _outputSize = new ColumnTextArea();
        _outputSize.justify( ColumnTextArea.RIGHT );
        _outputSize.setText( "" );
        showOutputSize( false );
        this.add( _outputSize );
        _difxVersion = new ColumnTextArea();
        _difxVersion.justify( ColumnTextArea.RIGHT );
        _difxVersion.setText( "" );
        showDifxVersion( true );
        this.add( _difxVersion );
        _speedUpFactor = new ColumnTextArea();
        _speedUpFactor.justify( ColumnTextArea.RIGHT );
        _speedUpFactor.setText( "" );
        showSpeedUpFactor( true );
        this.add( _speedUpFactor );
        _timeRemaining = new ColumnTextArea();
        _timeRemaining.justify( ColumnTextArea.RIGHT );
        _timeRemaining.setText( "" );
        showTimeRemaining( true );
        this.add( _timeRemaining );
        _numAntennas = new ColumnTextArea();
        _numAntennas.justify( ColumnTextArea.RIGHT );
        _numAntennas.setText( "" );
        showNumAntennas( true );
        this.add( _numAntennas );
        _numForeignAntennas = new ColumnTextArea();
        _numForeignAntennas.justify( ColumnTextArea.RIGHT );
        _numForeignAntennas.setText( "" );
        showNumForeignAntennas( true );
        this.add( _numForeignAntennas );
        _dutyCycleText = new ColumnTextArea();
        _dutyCycleText.justify( ColumnTextArea.RIGHT );
        _dutyCycleText.setText( "" );
        showDutyCycle( true );
        this.add( _dutyCycleText );
        _status = new ColumnTextArea();
        _status.justify( ColumnTextArea.RIGHT );
        _status.setText( "" );
        showStatus( true );
        this.add( _status );
                boolean active = false;
        _statusId = new ColumnTextArea();
        _statusId.justify( ColumnTextArea.RIGHT );
        _statusId.setText( "" );
        showStatusId( false );
        this.add( _statusId );
        _active = new ActivityMonitorLight();
        showActive( true );
        this.add( _active );
        _popup = new JPopupMenu();
        _monitorMenuItem = new JMenuItem( "Controls for " + name() );
        _monitorMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                if ( updateEditorMonitor( 1000 ) ) {                    
                    _editorMonitor.setVisible( true );
                }
                else
                    JOptionPane.showMessageDialog( _this, "Timeout reading .input file data",
                            "Failed", JOptionPane.WARNING_MESSAGE );
            }
        });
        _monitorMenuItem.setEnabled( false );
        _popup.add( _monitorMenuItem );
        _liveMonitorMenuItem = new JMenuItem( "Real-time Job Monitor" );
        _liveMonitorMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                if ( updateEditorMonitor( 1000) )
                    _editorMonitor.showLiveMonitor();
            }
        });
        _liveMonitorMenuItem.setEnabled( false );
        _popup.add( _liveMonitorMenuItem );
        _popup.add( new JSeparator() );
        JMenuItem selectMenuItem = new ZMenuItem( "Toggle Selection" );
        selectMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                selectionButtonAction();
            }
        });
        _popup.add( selectMenuItem );
        JMenuItem removeMenuItem = new ZMenuItem( "Remove from Queue Browser" );
        removeMenuItem.setToolTipText( "Remove this job from the Queue Browser.  This does not delete\n"
                + "associated files and database entries." );
        removeMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                removeAction();
            }
        });
        _popup.add( removeMenuItem );
        JMenuItem deleteItem = new ZMenuItem( "Delete" );
        deleteItem.setToolTipText( "Delete this experiment and all files associated with it.  Deletions\n"
                + "also apply to the database (if used).  This process is <<red>>NOT REVERSIBLE!<</color>>" );
        deleteItem.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                deleteAction();
            }
        });
        _popup.add( deleteItem );
        _popup.add( new JSeparator() );
        _runCalcOnJob = new ZMenuItem( "Run Calc on Job" );
        _runCalcOnJob.setToolTipText( "Run, or re-run, the calc process on this job.  Calc is required before\n"
                + "a job can be processed by DiFX.  The Calc process used is set in the\n"
                + "Job Creation Settings section of the Settings menu." );
        _runCalcOnJob.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                if ( passNode() != null ) {
                    if ( passNode().experimentNode() != null ) {
                        if ( passNode().experimentNode().editor() != null ) {
                            state().setText( "await .im file" );
                            state().setBackground( Color.YELLOW );
                            passNode().experimentNode().editor().runCalcOnly( passNode(), name() );
                        }
                    }
                }
            }
        });
        _runCalcOnJob.setEnabled( false );
        _popup.add( _runCalcOnJob );
        _scheduleJobItem = new ZMenuItem( "Schedule to Run" );
        _scheduleJobItem.setToolTipText( "Schedule this job to the be run using the automated scheduler\n"
                + "according to user settings that govern scheduled jobs." );
        _scheduleJobItem.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                autoStartJob( false );
            }
        });
        _scheduleJobItem.setEnabled( false );
        _popup.add( _scheduleJobItem );
        _scheduleConfigTestItem = new ZMenuItem( "Schedule Configuration Test" );
        _scheduleConfigTestItem.setToolTipText( "Schedule the configuration test to be run on the .input file for this job.\n"
                + "Only the test will run - the job will not." );
        _scheduleConfigTestItem.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                autoStartJob( true );
            }
        });
        _scheduleConfigTestItem.setEnabled( false );
        _popup.add( _scheduleConfigTestItem );
        _unscheduleJobItem = new ZMenuItem( "Remove from Schedule" );
        _unscheduleJobItem.setToolTipText( "Remove this job from the scheduler list, if it is there.\n"
                + "Removing a non-listed job is harmless." );
        _unscheduleJobItem.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                unschedule();
            }
        });
        _popup.add( _unscheduleJobItem );
        _stopJobItem = new ZMenuItem( "Stop" );
        _stopJobItem.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                if ( updateEditorMonitor( 1000 ) )
                    _editorMonitor.stopJob();
                //  This will force the scheduler to stop paying attention to the job.
                autostate( AUTOSTATE_STOPPED );
            }
        });
        _popup.add( _stopJobItem );
        
    }
    
    @Override
    public void positionItems() {
        if ( _antennaLock == null )
            _antennaLock = new Object();
        _colorColumn = false;
        _xOff = _level * 30;
        _networkActivity.setBounds( _xOff, 6, 10, 10 );
        _xOff += 14;
        _label.setBounds( _xOff, 0, _widthName, _ySize );
        _xOff += _widthName;
        _state.setBounds( _xOff + 1, 1, _widthState - 2, 18 );
        _xOff += _widthState;
        _progress.setBounds( _xOff + 1, 1, _widthProgressBar - 2, 18 );
        _xOff += _widthProgressBar;
        if ( _showWeights && _weightsBuilt ) {
            //  The weights are a bit complicated...
            if ( _weights.length > 0 ) {
                //  Antenna labels are (for the moment) assumed to be only two letters
                //  long.  Thus they don't need a lot of room.  We try to adjust here to
                //  give as much room as possible to the plots or weight outputs.
                int boxSize = _widthWeights / _weights.length / 2;
                int labelSize = boxSize;
                if ( boxSize > 50 ) {
                    boxSize += labelSize - 50;
                    labelSize = 50;
                }
                if ( _antenna != null && _weight != null ) {
                    synchronized ( _antennaLock ) {
                        for ( int i = 0; i < _weights.length; ++i ) {
//                            if ( _antenna[i] != null ) { // why is this necessary??
                                setTextArea( _antenna[i], labelSize );
                                _antenna[i].setVisible( true );
//                            }
                            if ( _showWeightsAsPlots ) {
//                                if ( _weightPlotWindow[i] != null ) {
                                    setTextArea( _weightPlotWindow[i], boxSize );
                                    _weightPlotWindow[i].setVisible( true );
                                    _weight[i].setVisible( false );
//                                }
                            }
                            else {
//                                if ( _weight[i] != null ) {
                                    setTextArea( _weight[i], boxSize );
                                    _weight[i].setVisible( true );
                                    _weightPlotWindow[i].setVisible( false );
//                                }

                            }
                        }
                    }
                }
            }
            else
                _xOff += _widthWeights;
        }
        else {
            if ( _antenna != null && _weight != null ) {
            synchronized ( _antennaLock ) {
                for ( int i = 0; i < _antenna.length && i < _weight.length; ++i ) {
                    if ( _antenna[i] != null )
                        _antenna[i].setVisible( false );
                    if ( _weight[i] != null )
                        _weight[i].setVisible( false );
                    if ( _weightPlotWindow[i] != null )
                        _weightPlotWindow[i].setVisible( false );
                }
            }
            }
        }
//        _startButton.setBounds( _level * 30 + 150, 0, 70, 20 );
//        _editButton.setBounds( _level * 30 + 230, 0, 70, 20 );
        //if ( _state.isVisible() )
        //    setTextArea( _state, _widthState );
        //if ( _progress.isVisible() )
        //    setTextArea( _progress, _widthProgressBar );
        if ( _experiment.isVisible() )
            setTextArea( _experiment, _widthExperiment );
        if ( _pass.isVisible() )
            setTextArea( _pass, _widthPass );
        if ( _priority.isVisible() )
            setTextArea( _priority, _widthPriority );
        if ( _queueTime.isVisible() )
            setTextArea( _queueTime, _widthQueueTime );
        if ( _correlationStart.isVisible() )
            setTextArea( _correlationStart, _widthCorrelationStart );
        if ( _correlationEnd.isVisible() )
            setTextArea( _correlationEnd, _widthCorrelationEnd );
        if ( _correlationTime.isVisible() )
            setTextArea( _correlationTime, _widthCorrelationTime );
        if ( _jobStartText.isVisible() )
            setTextArea( _jobStartText, _widthJobStart );
        if ( _jobDurationText.isVisible() )
            setTextArea( _jobDurationText, _widthJobDuration );
        if ( _inputFile.isVisible() )
            setTextArea( _inputFile, _widthInputFile );
        if ( _outputFile.isVisible() )
            setTextArea( _outputFile, _widthOutputFile );
        if ( _outputSize.isVisible() )
            setTextArea( _outputSize, _widthOutputSize );
        if ( _difxVersion.isVisible() )
            setTextArea( _difxVersion, _widthDifxVersion );
        if ( _speedUpFactor.isVisible() )
            setTextArea( _speedUpFactor, _widthSpeedUpFactor );
        if ( _timeRemaining.isVisible() )
            setTextArea( _timeRemaining, _widthTimeRemaining );
        if ( _numAntennas.isVisible() )
            setTextArea( _numAntennas, _widthNumAntennas );
        if ( _numForeignAntennas.isVisible() )
            setTextArea( _numForeignAntennas, _widthNumForeignAntennas );
        if ( _dutyCycleText.isVisible() )
            setTextArea( _dutyCycleText, _widthDutyCycle );
        if ( _status.isVisible() )
            setTextArea( _status, _widthStatus );
        if ( _active.isVisible() )
            setTextArea( _active, _widthActive );
        if ( _statusId.isVisible() )
            setTextArea( _statusId, _widthStatusId );
    }
    
    static JobNode _lastSelection = null;
    static JobNode _lastDeselection = null;
    
    /*
     * Look for uses of the shift key that indicate "regions" of selected jobs.
     */
    public void checkForShift() {
        if ( _selected ) {
            //  Tell the top-level queue browser that it looks like the user may
            //  be selecting a bunch of items if the shift key is down.  This requires
            //  both this selection and the previous one (if it exists).
            if ( _lastSelection != null && _selectedButton.shiftDown() ) {
                _settings.queueBrowser().groupSelection( this, _lastSelection, true );
            }
            _lastSelection = this;
            _lastDeselection = null;
        }
        else {
            if ( _lastDeselection != null && _selectedButton.shiftDown() ) {
                _settings.queueBrowser().groupSelection( this, _lastDeselection, false );
            }
            _lastSelection = null;
            _lastDeselection = this;
        }
    }
    
    /*
     * These values track the "state" of the job for autostart purposes.
     */
    public final static int AUTOSTATE_UNDETERMINED     = 0;
    public final static int AUTOSTATE_SCHEDULED        = 1;
    public final static int AUTOSTATE_INITIALIZING     = 2;
    public final static int AUTOSTATE_READY            = 3;
    public final static int AUTOSTATE_RUNNING          = 4;
    public final static int AUTOSTATE_DONE             = 5;
    public final static int AUTOSTATE_UNSCHEDULED      = 6;
    public final static int AUTOSTATE_FAILED           = 7;
    public final static int AUTOSTATE_RESOURCE_TIMEOUT = 8;
    public final static int AUTOSTATE_STOPPED          = 9;
    
    protected Integer _autostate;
    public int autostate() {
        int ret = AUTOSTATE_UNDETERMINED;
        synchronized ( _autostate ) {
            ret = _autostate;
        }
        return ret;
    }
    public void autostate( int newState ) { _autostate = newState; }
    
    //  The "idle time" is used by the scheduler to determine when jobs have been
    //  doing nothing for periods of time that are too long.  When not running 
    //  with the scheduler it should be harmless.
    protected Long _idleTime = new Long( 0 );
    public long idleTime() { 
        long ret = (long)0;
        synchronized ( _idleTime ) {
            ret = _idleTime;
        }
        return ret;
    }
    public void resetIdleTime() {
        synchronized ( _idleTime ) {
            _idleTime = (long)0;
        }
    }
    public void incrementIdleTime() {
        synchronized ( _idleTime ) {
            ++_idleTime;
        }
    }
    boolean _schedulerConfigOnly;
    public boolean schedulerConfigOnly() { return _schedulerConfigOnly; }
    /* 
     * Function to start a job - this checks that the "editor" exists (which includes
     * downloading necessary files), chooses default nodes (if the user hasn't picked
     * them), and runs the job, all in a thread.
     */
    public void autoStartJob( boolean configOnly ) {
        if ( _settings.queueBrowser().addJobToSchedule( this ) ) {
            _scheduleJobItem.setEnabled( false );
            _scheduleConfigTestItem.setEnabled( false );
            _runCalcOnJob.setEnabled( false );
            state().setText( "Scheduled" );
            state().setBackground( Color.YELLOW );
            state().updateUI();
            warningMessage( "" );
            if ( configOnly )
                _schedulerConfigOnly = true;
            else
                _schedulerConfigOnly = false;
            autostate( AUTOSTATE_SCHEDULED );
        }
    }

    /*
     * This is the thread that runs a job completely.  Might have other purposes.
     */
    public class JobThread extends Thread {
        public void run() {
            state().setText( "Scheduled" );
            state().setBackground( Color.YELLOW );
            state().updateUI();
            if ( updateEditorMonitor( 1000 ) ) {
                _editorMonitor.setState( "Check Resources", Color.YELLOW );
                _editorMonitor.loadHardwareLists();
                _editorMonitor.selectNodeDefaults( false, true );
                _editorMonitor.setState( "Pre-Start", Color.YELLOW );
                _editorMonitor.startJob( true, false );
            }     
        }
    }
    
    /*
     * Unschedule a job - this will only do things (like changing the state
     * readout) if the job is actually scheduled.
     */
    public void unschedule() {
        autostate( AUTOSTATE_UNSCHEDULED );
        if ( _settings.queueBrowser().removeJobFromSchedule( _this ) ) {
            setState( "Unscheduled", Color.GRAY );
            _scheduleJobItem.setEnabled( true );
            _scheduleConfigTestItem.setEnabled( true );
            _runCalcOnJob.setEnabled( true );
        }
    }
    
    /*
     * Thread to run the "resource allocation" portion of a job - for automatic job
     * running.  A large number of checks have been put in place to catch when the
     * user has "unscheduled" a job during this process.  Probably these are overkill.
     */
    public class CheckResourcesThread extends Thread {
        public void run() {
            if ( autostate() != AUTOSTATE_RESOURCE_TIMEOUT ) {
                if ( autostate() == AUTOSTATE_UNSCHEDULED )
                    setState( "Unscheduled", Color.GRAY );
                else {
                    setState( "Check Resources", Color.YELLOW );
                }
                if ( updateEditorMonitor( 1000 ) ) {
                    if ( autostate() == AUTOSTATE_UNSCHEDULED )
                        setState( "Unscheduled", Color.GRAY );
                    else {
                        setState( "Check Resources", Color.YELLOW );
                        //  Run a "data" check to see if, for whatever reason, data are not
                        //  available for this job.
                        if ( !_editorMonitor.dataAvailableCheck() ) {
                            //  We no longer want to do this but I'm clinging to it because it took
                            //  so long to code correctly.  Commented out for now...
//                            //  See if it is possible to skip stations and continue running
//                            //  if the appropriate setting allows this.
//                            if ( _settings.tryToSkipMissingStations() && _editorMonitor.skipStationsMissingData() ) {
//                                _editorMonitor.removeSkippedStations( true );
//                                setState( "Rebuilding Job", Color.YELLOW );
//                                while ( autostate() != AUTOSTATE_RESOURCE_TIMEOUT && !_editorMonitor.rebuildFailed() &&
//                                        !_editorMonitor.rebuildSuccess() ) {
//                                    try { Thread.sleep( 100 ); } catch ( Exception e ) {}
//                                }
//                                if ( autostate() == AUTOSTATE_RESOURCE_TIMEOUT ) {
//                                    return;
//                                }
//                                else if ( _editorMonitor.rebuildFailed() ) {
//                                    setState( "Rebuild Failed", Color.RED );
//                                    autostate( AUTOSTATE_FAILED );
//                                }
//                            }
//                            else {
//                                autostate( AUTOSTATE_FAILED );
//                                return;
//                            }
                            autostate( AUTOSTATE_FAILED );
                            return;
                        }
                        _editorMonitor.loadHardwareLists();
                    }
                    if ( _editorMonitor.selectNodeDefaults( true, true ) ) {
                        if ( autostate() == AUTOSTATE_UNSCHEDULED )
                            setState( "Unscheduled", Color.GRAY );
                        else {
                            setState( "Pre-Start", Color.YELLOW );
                            autostate( AUTOSTATE_READY );
                        }
                    }
                    else if ( autostate() == AUTOSTATE_UNSCHEDULED )
                        setState( "Unscheduled", Color.GRAY );
                    else {
                        if ( !_editorMonitor.dataSourcesTested() ) {
                            setState( "Data Source Fail", Color.RED );
                            autostate( AUTOSTATE_FAILED );
                        }
                        else if ( !_editorMonitor.processorsSufficient() ) {
                            setState( "Processor Fail", Color.RED );
                            autostate( AUTOSTATE_FAILED );
                        }
                        else {
                            setState( "Resource Wait", Color.YELLOW );
                            autostate( AUTOSTATE_SCHEDULED );
                        }
                    }
                }
                else {
                    if ( autostate() == AUTOSTATE_UNSCHEDULED )
                        setState( "Unscheduled", Color.GRAY );
                    else {
                        setState( "Monitor Error", Color.RED );
                        autostate( AUTOSTATE_FAILED );
                    }
                }
            }
        schedulerWarningMessages();
        schedulerErrorMessages();
        }
    }
    
    /*
     * Function to trigger the CheckResourcesThread.
     */
    public void autostartCheckResources() {
        resetIdleTime();
        autostate( AUTOSTATE_INITIALIZING );
        CheckResourcesThread checkResources = new CheckResourcesThread();
        checkResources.start();
    }
    
    class JobStartThread extends Thread {
        public void run() {
            if ( _editorMonitor != null ) {
                if ( _schedulerConfigOnly )
                    _editorMonitor.startJob( false, true );
                else
                    _editorMonitor.startJob( false, false );
            }
        }
    }
    /*
     * Function to run a job that has resources allocated.  The startJob() function
     * already runs a thread to do the delayed work.
     */
    public void autostartJobStart() {
        resetIdleTime();
        autostate( AUTOSTATE_RUNNING );
        JobStartThread jobStartThread = new JobStartThread();
        jobStartThread.start();
        //  Give the thread time to "reserve" necessary data and processing nodes.
        try { Thread.sleep( 1000 ); } catch ( Exception e ) {}
    }
    
    /*
     * Used by the scheduler to unschedule a job that appears to be stuck during
     * hardware resource allocation.
     */
    public void autoUnscheduleResourceAllocation() {
        autostate( AUTOSTATE_RESOURCE_TIMEOUT );
        state().setText( "Auto Timeout (HW)" );
        state().setBackground( Color.RED );
        state().updateUI();
    }
    
    /*
     * Used by the scheduler to unschedule a job that appears to be stuck during
     * a run.
     */
    public void autoUnscheduleProcessing() {
        autostate( AUTOSTATE_UNSCHEDULED );
        _editorMonitor.flushFromActiveNodes();
        state().setText( "Auto Timeout (Proc)" );
        state().setBackground( Color.RED );
        state().updateUI();
    }
    
    //--------------------------------------------------------------------------
    //  Catch-all when the scheduler wants to free resources allocated to this
    //  job (won't hurt if none are allocated).
    //--------------------------------------------------------------------------
    public void flushFromActiveNodes() {
        if ( _editorMonitor != null )
            _editorMonitor.flushFromActiveNodes();
    }
    
    String _currentWarningMessage = "";
    String _currentErrorMessage = "";
    boolean _warningNew;
    boolean _errorNew;
    
    //--------------------------------------------------------------------------
    //  Set the warning message.  If it is a change from what existed, set an
    //  appropriate flag.
    //--------------------------------------------------------------------------
    void warningMessage( String newMessage ) {
        if ( !_currentWarningMessage.contentEquals( newMessage ) ) {
            _currentWarningMessage = newMessage;
            if ( _currentWarningMessage != "" )
                _warningNew = true;
        }
    }
    
    //--------------------------------------------------------------------------
    //  Send any (new) warning message to the messaging system.  
    //--------------------------------------------------------------------------
    void schedulerWarningMessages() {
        if ( _warningNew )
            _settings.messageCenter().warning( 0, "Scheduler: " + name(), _currentWarningMessage );
        _warningNew = false;
    }
    
    //--------------------------------------------------------------------------
    //  Set the error message.  If it is a change from what existed, set an
    //  appropriate flag.
    //--------------------------------------------------------------------------
    void errorMessage( String newMessage ) {
        if ( !_currentErrorMessage.contentEquals( newMessage ) ) {
            _currentErrorMessage = newMessage;
            if ( _currentErrorMessage != "" )
                _errorNew = true;
        }
    }
    
    //--------------------------------------------------------------------------
    //  Send any (new) error message to the messaging system.  
    //--------------------------------------------------------------------------
    void schedulerErrorMessages() {
        if ( _errorNew )
            _settings.messageCenter().error( 0, "Scheduler: " + name(), _currentErrorMessage );
        _errorNew = false;
    }
    
    /*
     * Delete the editor monitor, which should free up any associated memory requirements
     * (if the garbage collector does its job).  This is performed in a thread after the
     * given number of seconds.
     */
    public void freeMonitor( int delay ) {
        FreeMonitorThread freeThread = new FreeMonitorThread( delay );
        freeThread.start();
    }
    
    /*
     * This thread sleeps for a given number of seconds and then deletes the EditorMonitor
     * instance associated with this job.  This is useful if you are running tons of jobs
     * as the EditorMonitor can swallow lots of memory.
     */
    public class FreeMonitorThread extends Thread {
        public FreeMonitorThread( int delay ) {
            _delay = delay;
        }
        public void run() {
            try { Thread.sleep( _delay * 1000 ); } catch ( Exception e ) {}
//            Runtime rt = Runtime.getRuntime();
//            long usedMB = (rt.totalMemory() - rt.freeMemory()) / 1024 / 1024;
//            System.gc();
//            System.out.println( "memory usage before " + usedMB);
            if ( _editorMonitor != null )
                _editorMonitor.close();
            _editorMonitor = null;
//            System.gc();
//            usedMB = (rt.totalMemory() - rt.freeMemory()) / 1024 / 1024;
//            System.out.println( "memory usage after " + usedMB);
        }
        int _delay;
    }
    /*
     * Private function used repeatedly in positionItems().
     */
    protected void setTextArea( Component area, int xSize ) {
        area.setBounds( _xOff, 1, xSize, _ySize - 2);
        _xOff += xSize;
        if ( _colorColumn )
            area.setBackground( _columnColor );
        else
            area.setBackground( Color.WHITE );
        _colorColumn = !_colorColumn;
    }
    
    @Override
    public void paintComponent( Graphics g ) {
        //  Use anti-aliasing on the text (looks much better)
        Graphics2D g2 = (Graphics2D)g;
        g2.setRenderingHint( RenderingHints.KEY_ANTIALIASING,
                     RenderingHints.VALUE_ANTIALIAS_ON );
        super.paintComponent( g );
    }
    
    //--------------------------------------------------------------------------
    //!  Remove this job from the queue browser.  This just makes the job
    //!  invisible - all associated files and database entries still exist.
    //--------------------------------------------------------------------------
    public void removeAction() {
        _settings.queueBrowser().removeJobFromSchedule( this );
        ((BrowserNode)(this.getParent())).removeChild( this );
    }
    
    /*
     * Delete this job.  It must be removed from the database, files associated with
     * it should be removed from the DiFX host, and then it is removed from
     * its parent "pass".
     */
    public void deleteAction() {
        removeFromDatabase();
        removeFromHost();
        _settings.queueBrowser().removeJobFromSchedule( this );
        ((BrowserNode)(this.getParent())).removeChild( this );
    }
    
    /*
     * Remove this job from the database.  This is probably done prior to deleting
     * this job.
     */
    public void removeFromDatabase() {
        if ( this.inDatabase() ) {
            QueueDBConnection db = null;
            if ( _settings.useDatabase() ) {
                db = new QueueDBConnection( _settings );
                if ( db.connected() ) {
                    db.deleteJob( _id );
                }
            }
        }
    }
    
    /*
     * Remove the files associated with this job from the DiFX host.  This is done
     * by deleting files of all extensions that match the input file name.  This
     * makes the assumption that this covers everything, which under normal operation
     * is correct.
     */
    public void removeFromHost() {
        String pathname = this.inputFile().substring( 0, this.inputFile().lastIndexOf( '.' ) ) + "*";
        DiFXCommand_rm rm = new DiFXCommand_rm( pathname, "-rf", _settings );
        try { rm.send(); } catch ( Exception e ) {}
    }
    
    /*
     * This is a generic database update function for this object.  It will change
     * a specific field to a specific value - both are strings.  This is only done if
     * this job is in the database.
     */
    public void updateDatabase( String param, String setting ) {
        if ( this.inDatabase() ) {
            QueueDBConnection db = null;
            if ( _settings.useDatabase() ) {
                db = new QueueDBConnection( _settings );
                if ( db.connected() ) {
                    db.updateJob( _id, param, setting );
                }
            }
        }
    }

    /*
     * Internal function used to generate an editor/monitor for this job if one
     * does not exists and update it with current settings, as far as we know them.
     * If called with a "delay" value (in milliseconds) the function will wait around
     * for the "delay" amount until the requestInputFile() call has completed.
     */
    protected boolean updateEditorMonitor( Integer delay ) {
        if ( _editorMonitor == null ) {
            _editorMonitor = new JobEditorMonitor( this, _settings );
            if ( delay != null )
                _inputFileRequestComplete = false;
            requestInputFile();
            if ( delay != null ) {
                //  Wait for the request (above) to be completed.
                int waitSoFar = 0;
                while ( waitSoFar < delay ) {
                    try { Thread.sleep( 100 ); } catch ( Exception e ) {}
                    waitSoFar += 100;
                    if ( _inputFileRequestComplete )
                        return true;
                }
                _editorMonitor = null;  //  Get rid of this - so next time it will be generated
                return false;  // failed!
            }
            else
                return true;
        }
        else
            return true;
    }
    
    /*
     * This function sends a request to mk5daemon for a copy of an .input file
     * associated with this job.  Mk5daemon will hopefully respond some time soon
     * with the actual data.  
     */
    protected void requestInputFile() {
        if ( _inputFile != null && _inputFile.getText().trim().length() > 0 ) {
            requestFile( _inputFile.getText().trim() );
        }
    }
    
    protected boolean _inputFileRequestComplete;
    
    /*
     * Same function, but applied to the .calc file.
     */
    protected void requestCalcFile() {
        if ( _calcFile != null )
            requestFile( _calcFile.trim() );
    }
    
    /*
     * Allows an external function to disable this job node so that data may be
     * read in from the guiServer.
     */
    public void readDisable() {
        if ( !_readingDataFile ) {
            _readingDataFile = true;
            _thisJobNode = this;
            _thisJobNode.setEnabled( false );
            _saveStateText = _state.getText();
            _saveStateColor = _state.getBackground();
            setState( "reading data", Color.YELLOW );
        }
    }
    
    protected JobNode _thisJobNode;
    protected String _saveStateText;
    protected Color _saveStateColor;
    
    /*
     * Request an input file from the DiFX Host.  The file will be parsed based on its
     * extension - .input and .calc files are recognized.
     */
    protected void requestFile( String filename ) {
        //  prevent user interaction with the job node while it is reading the file
        readDisable();
        //  Make sure an editor/monitor exists (so we can fill it with data).
        updateEditorMonitor( null );
        Component comp = _this;
        while ( comp.getParent() != null )
            comp = comp.getParent();
        Point pt = new Point( 100, 100 );
        GetFileMonitor getFile = new GetFileMonitor( (Frame)comp, pt.x + 25, pt.y + 25,
                filename, _settings, false );
        if ( getFile.inString() != null && getFile.inString().length() > 0 && getFile.success() ) {
            //  Parse the file content based on the extension.
            String ext = filename.substring( filename.lastIndexOf( '.' ) + 1 ).trim();
            if ( ext.contentEquals( "input" ) ) {
                _editorMonitor.inputFileName( filename, getFile.inString() );
                _editorMonitor.parseInputFile();
                _inputFileRequestComplete = true;
                //_scheduleJobItem.setEnabled( true );
                //_scheduleConfigTestItem.setEnabled( true );
                //_stopJobItem.setEnabled( true );
                //_monitorMenuItem.setEnabled( true );
                //_liveMonitorMenuItem.setEnabled( true );
            }
            else if ( ext.contentEquals( "calc" ) ) {
                _editorMonitor.calcFileName( filename );
                _editorMonitor.parseCalcFile( getFile.inString() );
            }
            //  Set the state to reflect which files have been parsed - there should be
            //  both a .input and a .calc file.
            if ( _editorMonitor.inputFileParsed() && _editorMonitor.calcFileParsed() ) {
                setState( _saveStateText, _saveStateColor );
            }
            else if ( _editorMonitor.inputFileParsed() ) {
                setState( "No Calc File", Color.orange );
            }
            else if ( _editorMonitor.calcFileParsed() ) {
                setState( "No Input File", Color.orange );
            }
        }
        else {
            setState( "file xfer error", Color.ORANGE );
        }
        _readingDataFile = false;
    }
    
    protected boolean _readingDataFile;
    public boolean readingDataFile() { return _readingDataFile; }
    
    //--------------------------------------------------------------------------
    //  Test if this message is intended for a job or not.  Currently we recognize
    //  Status, Alert, and Diagnostic messages as being for jobs.
    //--------------------------------------------------------------------------
    static boolean testJobMessage( DifxMessage difxMsg ) {
        if ( ( difxMsg.getBody().getDifxStatus() != null ) ||
             ( difxMsg.getBody().getDifxAlert() != null ) || 
             ( difxMsg.getBody().getDifxDiagnostic() != null ) ) {
            //  Eliminate some identifiers that are not jobs.  The only trouble with this is
            //  a user theoretically *can* name their jobs one of these things, but if they do
            //  do they get what they deserve.
            if ( difxMsg.getHeader().getIdentifier().equalsIgnoreCase( "mk5dir" ) )
                return false;
            if ( difxMsg.getHeader().getIdentifier().equalsIgnoreCase( "mk5cp" ) )
                return false;
            return true;
        }
        return false;
    }
    
    //--------------------------------------------------------------------------
    //  Interpret a (presumably) job-related message.
    //--------------------------------------------------------------------------
    public void consumeMessage( DifxMessage difxMsg, boolean unknown ) {
        
        resetIdleTime();
        
        //  If this job is "running" (it was started by the job editor/monitor) 
        //  then send the message to the monitor.  We don't want to do this if the
        //  job is "unknown" (which is to say we didn't create it) because the
        //  editor/monitor is meaningless.
        if ( !unknown )
            updateEditorMonitor( 1000 );
        
        if ( !lockState() ) {
            JulianCalendar thisTime = new JulianCalendar();
            thisTime.setTime( new Date() );
            if ( _startTime != null ) {
                _jobCorrelationTime = 24.0 * 3600.0 * ( thisTime.mjd() - _startTime.mjd() );
                correlationTime( _jobCorrelationTime );
            }
        }
        
        //  Got something...
        _networkActivity.data();
        
        //  See what kind of message this is...try status first.
        if ( difxMsg.getBody().getDifxStatus() != null ) {
            //  Only change the "state" of this job if it hasn't been "locked" by the GUI.  This
            //  happens when the GUI detects an error or completes.  If this job is "starting" the state should
            //  be unlocked - it means another attempt is being made to run it.
            if ( difxMsg.getBody().getDifxStatus().getState().equalsIgnoreCase( "starting" ) ) {
                setProgress( 0 );
                lockState( false );
            }
            if ( !lockState() ) {
                //  Set the "complete" percentage.
                if ( difxMsg.getBody().getDifxStatus().getVisibilityMJD() != null &&
                        difxMsg.getBody().getDifxStatus().getJobstartMJD() != null &&
                        difxMsg.getBody().getDifxStatus().getJobstopMJD() != null ) {
                    _jobTotalTime = 24.0 * 3600.0 * ( Double.valueOf( difxMsg.getBody().getDifxStatus().getJobstopMJD() ) -
                            Double.valueOf( difxMsg.getBody().getDifxStatus().getJobstartMJD() ) );
                    _fractionComplete = ( Double.valueOf( difxMsg.getBody().getDifxStatus().getVisibilityMJD() ) -
                            Double.valueOf( difxMsg.getBody().getDifxStatus().getJobstartMJD() ) ) /
                            ( Double.valueOf( difxMsg.getBody().getDifxStatus().getJobstopMJD() ) -
                            Double.valueOf( difxMsg.getBody().getDifxStatus().getJobstartMJD() ) );
                    setProgress( (int)( 0.5 + 100.0 * _fractionComplete ) );
                    //  Take a stab at figuring out how much time is left in this job.  We use the fraction
                    //  of the job completed, but need to account for the (rather long in some cases) block
                    //  of time that occurs before any "fraction" of the job is processed (the fraction is
                    //  based on the timestamps of date being processed and does not account for setup time).
                    //  Until we have a first couple of "fraction complete" values, things are less accurate.
                    //  We really only can do this if we started the job (which we figure out by looking at
                    //  _jobStartTime).
                    if ( _jobStartTime != null ) {
                        if ( _firstFraction == null || _firstTime == null ) {
                            _firstFraction = _fractionComplete;
                            _firstTime = (double)System.currentTimeMillis() / 1000.0;
                        }
                        else if ( _fractionComplete > _firstFraction && _fractionComplete < 0.8) {
                            //  We can now figure out how fast the fractiong completed is going by in
                            //  time, the size of the start buffer, and a reasonable estimate of the
                            //  time remaining.  This calculation is only made until the job is 80%
                            //  complete because "time remaining"/"fraction complete" goes totally
                            //  non-linear after about then.
                            _timeRemainingTime = (double)System.currentTimeMillis() / 1000.0;
                            _timePerFrac = ( _timeRemainingTime - _firstTime ) / ( _fractionComplete - _firstFraction );
                            _startTimeBuffer = _firstTime - _jobStartTime / 1000.0 - _timePerFrac * _firstFraction;
                            _currentTimeRemaining = _timePerFrac * ( 1.0 - _fractionComplete ) - _settings.queueBrowser().estimateExtraTime();
                        }
                    }
                }
                //  And the state itself.  We try to report some information upon completion.
                setState( difxMsg.getBody().getDifxStatus().getState(), Color.GREEN );
                if ( _state.getText().trim().equalsIgnoreCase( "done" ) || _state.getText().trim().equalsIgnoreCase( "mpidone" ) ) {
                    if ( _editorMonitor != null && _editorMonitor.doneWithErrors() ) {
                        setState( "complete w/errors", Color.ORANGE );
                        _state.setToolTipText( "The job completed with some errors." );
                    }
                    else if ( _editorMonitor != null && _editorMonitor.removedList() != null ) {
                        setState( "Complete w/o " + _editorMonitor.removedList(), Color.ORANGE );
                        lockState( true );
                    }
                    else {
                        _state.setToolTipText( "The job completed gracefully." );
                    }
                    setProgress( 100 );  
                }
                else if ( _state.getText().equalsIgnoreCase( "running" ) || _state.getText().equalsIgnoreCase( "Starting" ) 
                        || _state.getText().equalsIgnoreCase( "ending" ) ) {
                    setState( _state.getText(), Color.YELLOW );
                    _state.setToolTipText( "" );
                }
                else {
                    setState( _state.getText(), Color.LIGHT_GRAY );
                    _state.setToolTipText( "" );
                }
            }
            List<DifxStatus.Weight> weightList = difxMsg.getBody().getDifxStatus().getWeight();
            //  Create a new list of antennas/weights if one hasn't been created yet.
            //System.out.println( "size of weight list is " + weightList.size() );
            if ( !_weightsBuilt )
                newWeightDisplay( weightList.size() );
            for ( Iterator<DifxStatus.Weight> iter = weightList.iterator(); iter.hasNext(); ) {
                DifxStatus.Weight thisWeight = iter.next();
                //System.out.println( thisWeight.getAnt() +  "  " + thisWeight.getWt() );
                weight( thisWeight.getAnt(), thisWeight.getWt() );
            }
        }
        else if ( difxMsg.getBody().getDifxAlert() != null ) {
            //System.out.println( "this is an alert" );
            //System.out.println( difxMsg.getBody().getDifxAlert().getAlertMessage() );
            //System.out.println( difxMsg.getBody().getDifxAlert().getSeverity() );
        }
        else if ( difxMsg.getBody().getDifxDiagnostic() != null ) {
//            if ( difxMsg.getBody().getDifxDiagnostic().getDiagnosticType().equalsIgnoreCase( "DataConsumed" ) ) {
//                long bytes = difxMsg.getBody().getDifxDiagnostic().getBytes();
//                System.out.println( "data consumed: " + bytes + " bytes" );
//            }
//            else if ( difxMsg.getBody().getDifxDiagnostic().getDiagnosticType().equalsIgnoreCase( "InputDatarate" ) ) {
//                long bytes = difxMsg.getBody().getDifxDiagnostic().getBytes();
//                System.out.println( "input data rate: " + bytes + " bytes" );
//            }
//            else if ( difxMsg.getBody().getDifxDiagnostic().getDiagnosticType().equalsIgnoreCase( "MemoryUsage" ) ) {
//                long bytes = difxMsg.getBody().getDifxDiagnostic().getBytes();
//                System.out.println( "memory usage: " + bytes + " bytes" );
//            }
//            else if ( difxMsg.getBody().getDifxDiagnostic().getDiagnosticType().equalsIgnoreCase( "NumSubintsLost" ) ) {
//                long n = difxMsg.getBody().getDifxDiagnostic().getNumSubintsLost();
//                System.out.println( "num subints lost: " + n );
//            }
//            else if ( difxMsg.getBody().getDifxDiagnostic().getDiagnosticType().equalsIgnoreCase( "ProcessingTime" ) ) {
//                int threadId = difxMsg.getBody().getDifxDiagnostic().getThreadId();
//                double microsec = difxMsg.getBody().getDifxDiagnostic().getMicrosec();
//                System.out.println( "processing time (thread " + threadId + ": " + microsec );
//            }
//            else if ( difxMsg.getBody().getDifxDiagnostic().getDiagnosticType().equalsIgnoreCase( "BufferStatus" ) ) {
//                int nbuff = difxMsg.getBody().getDifxDiagnostic().getNumBufElements();
//                int startbuff = difxMsg.getBody().getDifxDiagnostic().getStartBufElement();
//                int activebuff = difxMsg.getBody().getDifxDiagnostic().getActiveBufElements();
//                System.out.println( "buffer status num: " + nbuff + " start: " + startbuff + " active: " + activebuff );
//            }
        }
        
        //  Figure out the speed up factor and time remaining, if we can.
        if ( _jobCorrelationTime != null && _fractionComplete != null && _jobTotalTime != null ) {
            _speedUp = ( _fractionComplete * _jobTotalTime )  / _jobCorrelationTime;
            speedUpFactor( _speedUp );
        }
        timeRemaining( _currentTimeRemaining - ( (double)System.currentTimeMillis() / 1000.0 - _timeRemainingTime ) );
        _calculatedTimeRemaining = _currentTimeRemaining - ( (double)System.currentTimeMillis() / 1000.0 - _timeRemainingTime );

    }
    
    //  Set the progress bar of this job node and the editor/monitor progress bar.
    void setProgress( int i ) {
        _progress.setValue( i );
        if ( _editorMonitor != null ) {
            _editorMonitor.setProgress( i );
        }
    }
    
    /*
     * This function is used to generate antenna/weight display areas.
     */
    protected void newWeightDisplay( int numAntennas ) {
        if ( _weightsBuilt )
            return;
//        synchronized ( _antennaLock ) {
        _weights = new double[ numAntennas ];
        _antennas = new String[ numAntennas ];
        _weight = new ColumnTextArea[ numAntennas ];
        _antenna = new ColumnTextArea[ numAntennas ];
        _weightPlotWindow = new PlotWindow[ numAntennas ];
        _weightPlot = new Plot2DObject[ numAntennas ];
        _weightTrack = new Track2D[ numAntennas ];
        _weightTrackSize = new int[ numAntennas ];
        //  Give the antennas "default" names.
        for ( Integer i = 0; i < numAntennas; ++i ) {
            _antenna[i] = new ColumnTextArea( i.toString() + ": " );
            _antenna[i].justify( ColumnTextArea.RIGHT );
            this.add( _antenna[i] );
            _weight[i] = new ColumnTextArea( "" );
            this.add( _weight[i] );
            _antennas[i] = i.toString();
//            //  This stuff is used to make a plot of the weight.
            _weightPlotWindow[i] = new PlotWindow();
            this.add( _weightPlotWindow[i] );
            _weightPlot[i] = new Plot2DObject();
            _weightPlotWindow[i].add2DPlot( _weightPlot[i] );
            _weightPlot[i].name( "Weight Plot " + i.toString() );
            _weightPlot[i].drawBackground( true );
            _weightPlot[i].drawFrame( true );
            _weightPlot[i].frameColor( Color.GRAY );
            _weightPlot[i].clip( true );
            _weightPlot[i].addTopGrid( Plot2DObject.X_AXIS, 10.0, Color.BLACK );
            _weightTrack[i] = new Track2D();
            _weightTrack[i].fillCurve( true );
            _weightPlot[i].addTrack( _weightTrack[i] );
            _weightTrack[i].color( Color.GREEN );
            _weightTrack[i].sizeLimit( 200 );
            _weightPlot[i].frame( 0.0, 0.0, 1.0, 1.0 );
            _weightPlot[i].backgroundColor( Color.BLACK );
            _weightTrackSize[i] = 0;
        }
//        }
        _weightsBuilt = true;
    }
    
    public void experiment( String newVal ) { _experiment.setText( newVal ); }
    public String experiment() { return _experiment.getText(); }
    public void pass( String newVal ) { _pass.setText( newVal ); }
    public String pass() { return _pass.getText(); }
    public void priority( int newVal ) { _priority.setText( String.format( "%10d", newVal ) ); }
    public int priority() { return new Integer( _priority.getText() ).intValue(); }
    public void queueTime( String newVal ) { _queueTime.setText( newVal ); }
    public String queueTime() { return _queueTime.getText(); }
    public void correlationStart( String newVal ) { _correlationStart.setText( newVal ); }
    public String correlationStart() { return _correlationStart.getText(); }
    public void correlationStart( double newVal ) { 
        _correlationStart.setText( String.format( "%10.5f", newVal ) );
        _correlationStart.updateUI();
    }
    public void correlationEnd( String newVal ) { _correlationEnd.setText( newVal ); }
    public void correlationEnd( double newVal ) { 
        _correlationEnd.setText( String.format( "%10.5f", newVal ) );
        _correlationEnd.updateUI();
    }
    public String correlationEnd() { return _correlationEnd.getText(); }
    public void correlationTime( String newVal ) { _correlationTime.setText( newVal ); }
    public void correlationTime( double newVal ) { 
        _correlationTime.setText( fromSeconds( newVal, 1 ) );
        _correlationTime.updateUI();
    }
    public String correlationTime() { return _correlationTime.getText(); }
    public void jobStart( double newVal ) { 
        _jobStartText.setText( String.format( "%10.5f", newVal ) );
        _jobStart = newVal;
    }
    public Double jobStart() { 
        return _jobStart;
    }
    public void jobDuration( Double newVal ) { 
        _jobDurationText.setText( fromSeconds( newVal, 1 ) );//String.format( "%10.1f", newVal ) );
        _jobDuration = newVal;
    }
    public Double jobDuration() { return _jobDuration; }
    
    //--------------------------------------------------------------------------
    //!  Convert a double-precision number of seconds into a number of hours, minutes,
    //!  and seconds (with specified precision).  
    //--------------------------------------------------------------------------
    public static String fromSeconds( double seconds, int precision ) {
        String ret = "";
        NumberFormat nf = NumberFormat.getInstance();
        nf.setMaximumFractionDigits( precision );
        nf.setRoundingMode( RoundingMode.FLOOR );
        if ( precision > 0 )
            nf.setMinimumFractionDigits( precision );
        if ( seconds <= 0.0 )
            return ret;
        //  Figure out the number of hours, minutes and seconds.
        int hours = (int)(seconds/3600.0);
        seconds -= 3600.0 * (double)hours;
        int minutes = (int)(seconds/60.0);
        seconds -= 60.0 * (double)minutes;
        if ( hours > 0 ) {
            ret += hours + ":";
            if ( minutes > 9 )
                ret += minutes + ":";
            else if ( minutes > 0 )
                ret += "0" + minutes + ":";
            else
                ret += "00:";
            if ( (int)seconds > 9 )
                ret += nf.format( seconds );
            else
                ret += "0" + nf.format( seconds );
        }
        else if ( minutes > 0 ) {
            ret += minutes + ":";
            if ( (int)seconds > 9 )
                ret += nf.format( seconds );
            else
                ret += "0" + nf.format( seconds );
        }
        else
            ret += nf.format( seconds );
        return ret;
    }
    
    DiFXCommand_getFile _fileGet;
    static Object _fileGetSync;
    static Boolean _fileGetLocked;
    static int _threadCount;
    //--------------------------------------------------------------------------
    //  Download and parse the "difxlog" file for this job, if it exists.  This
    //  gives us the job run history, which is used (among other things) to set
    //  the current job state.
    //--------------------------------------------------------------------------
    public void parseLogFile() {
        //  Set up a lock so we only do one of these at a time.
        if ( _fileGetSync == null ) {
            _fileGetSync = new Object();
            _fileGetLocked = new Boolean( false );
        }
        //  Do the operation in a thread using the lock.
        Thread parseLogThread = new Thread() {
            public void run() {
                ++_threadCount;
                //  Wait for the lock to be free so only one of these is done at a time.
                boolean blocked = true;
                while ( blocked ) {
                    synchronized( _fileGetSync ) {
                        if ( !_fileGetLocked ) {
                            _fileGetLocked = true;
                            blocked = false;
                            System.out.println( "   LOCK: thread count is " + _threadCount );
                        }
                    }
                    try { Thread.sleep( 100 ); } catch ( Exception e ) {}
                }
                //  Create a log file name from the input file name.  A null pointer exception
                //  indicates the input file name has not been set or has not been set correctly.
                try {
                    _logFile = _inputFile.getText().replace( ".input", ".difxlog" );
                } catch ( NullPointerException e ) {
                    return;
                }
                _fileGet = new DiFXCommand_getFile( _logFile, _settings );
                if ( _fileGet.error() != null ) {
                    return;
                }
                _fileGet.addEndListener( new ActionListener() {
                    public void actionPerformed( ActionEvent e ) {
                        //  This is where we actually parse the file.  Only do so if there is a file and
                        //  it was read completely.
                        if ( _fileGet.fileSize() > 0 && _fileGet.fileSize() == _fileGet.inString().length() ) {
                            setState( "LOG", Color.GREEN );
                        }
                        else
                            setState( "not Started", Color.LIGHT_GRAY );
                        //  Release the lock.
                        System.out.println( "done reading" );
                        synchronized( _fileGetSync ) {
                            _fileGetLocked = false;
                            System.out.println( "RELEASE: thread count is " + _threadCount );
                        }
                    }
                });
                try { 
                    _fileGet.readString();
                } catch ( java.net.UnknownHostException e ) {}
                --_threadCount;
            }
        };
        parseLogThread.start();
    }
    
    public void inputFile( String newVal, boolean loadNow ) { 
        _inputFile.setText( newVal );
        //  Convert to a file to extract the directory path...
        File tryFile = new File( newVal );
        _directoryPath = tryFile.getParent();
        //  Now that we have an input file, enable controls associated with it.
        _scheduleJobItem.setEnabled( true );
        _scheduleConfigTestItem.setEnabled( true );
        _runCalcOnJob.setEnabled( true );
        _stopJobItem.setEnabled( true );
        _monitorMenuItem.setEnabled( true );
        _liveMonitorMenuItem.setEnabled( true );
        //  Request the contents of this input file immediately if "loadNow" is
        //  true.  This will tend to be done for jobs that are newly created, but
        //  not for those that are loaded from databases or elsewhere.
        if ( loadNow )
            requestInputFile();
        updateUI();
    }
    public String inputFile() { return _inputFile.getText(); }
    public void calcFile( String newVal, boolean loadNow ) {
        _calcFile = newVal;
        if ( loadNow )
            requestCalcFile();
    }
    public String calcFile() { return _calcFile; }
    public void fullName( String newVal ) { _fullName = newVal; }
    public String fullName() { return _fullName; }
    public void outputFile( String newVal ) { _outputFile.setText( newVal ); }
    public String outputFile() { return _outputFile.getText(); }
    public void outputSize( int newVal ) { _outputSize.setText( String.format( "%10d", newVal ) ); }
    public int outputSize() { return new Integer( _outputSize.getText() ).intValue(); }
    public void difxVersion( String newVal ) { _difxVersion.setText( newVal ); }
    public String difxVersion() { return _difxVersion.getText(); }
    public void speedUpFactor( double newVal ) { _speedUpFactor.setText( String.format( "%10.3f", newVal ) ); }
    public void timeRemaining( double newVal ) { _timeRemaining.setText( fromSeconds( newVal, 0 ) ); }
    public double speedUpFactor() { return new Double( _speedUpFactor.getText() ).doubleValue(); }
    public void numAntennas( int newVal ) {
        _numAntennas.setText( String.format( "%10d", newVal ) );
        newWeightDisplay( newVal );
    }
    public Integer numAntennas() { return Integer.parseInt( _numAntennas.getText().trim() ); }
    public void weight( String antenna, String newString ) {
        double newVal = Double.valueOf( newString );
        int i = Integer.valueOf( antenna );
//        synchronized ( _antenna ) {
        if ( i < _weights.length ) {
            _weights[i] = newVal;
            _weight[i].setText( newString );
            _weightPlot[i].limits( (double)(_weightTrackSize[i] - 20), (double)(_weightTrackSize[i]), 0.0, 1.05 );
            _weightTrack[i].add( (double)(_weightTrackSize[i]), newVal );
            _weightTrackSize[i] += 1;
            _weightPlotWindow[i].updateUI();
            this.updateUI();
        }
//        }
//        for ( int i = 0; i < _weights.length; ++i ) {
//            if ( _antennas[i].contentEquals( antenna ) ) {
//                _weights[i] = newVal;
//                _weight[i].setText( newString );
//                _weightPlot[i].limits( (double)(_weightTrackSize[i] - 20), (double)(_weightTrackSize[i]), 0.0, 1.05 );
//                _weightTrack[i].add( (double)(_weightTrackSize[i]), newVal );
//                _weightTrackSize[i] += 1;
//                _weightPlotWindow[i].updateUI();
//            }
//        }
    }
    public double weight( String antenna ) {
        double ret = 0.0;
        boolean found = false;
//        synchronized ( _antenna ) {
        for ( int i = 0; i < _weights.length && !found; ++i )
            if ( _antennas[i].contentEquals( antenna ) ) {
                ret = _weights[i];
                found = true;
            }
//        }
        return ret;
    }
    public void antennaName( int i, String name ) {
//        synchronized ( _antenna ) {
        if ( i < _antennas.length ) {
            _antennas[i] = name;
            _antenna[i].setText( name + ": " );
            _antenna[i].updateUI();
        }
//        }
    }
    public String antennaName( int i ) {
        String ret = null;
//        synchronized ( _antenna ) {
        if ( i < _antennas.length )
            ret = _antennas[i];
//        }
        return ret;
    }
    public void numForeignAntennas( int newVal ) { _numForeignAntennas.setText( String.format( "%10d", newVal ) ); }
    public int numForeignAntennas() { return new Integer( _numForeignAntennas.getText() ).intValue(); }
    public void dutyCycle( Double newVal ) { 
        _dutyCycleText.setText( String.format( "%10.3f", newVal ) );
        _dutyCycle = newVal;
    }
    public Double dutyCycle() { return _dutyCycle; }
    public void status( String newVal ) { _status.setText( newVal ); }
    public String status() { return _status.getText(); }
    public void active( boolean newVal ) { _active.on( newVal ); }
    public boolean active() { return _active.on(); }
    public void statusId( int newVal ) { _statusId.setText( String.format( "%10d", newVal ) ); }
    public int statusId() { return new Integer( _statusId.getText() ).intValue(); }
    public String directoryPath() { return _directoryPath; }
    
    public void showNetworkActivity( boolean newVal ) { _networkActivity.setVisible( newVal ); }
    public void showName( boolean newVal ) { _label.setVisible( newVal ); }
    public void showProgressBar( boolean newVal ) { _progress.setVisible( newVal ); }
    public void showState( boolean newVal ) { _state.setVisible( newVal ); }
    public void showExperiment( boolean newVal ) { _experiment.setVisible( newVal ); }
    public void showPass( boolean newVal ) { _pass.setVisible( newVal ); }
    public void showPriority( boolean newVal ) { _priority.setVisible( newVal ); }
    public void showQueueTime( boolean newVal ) { _queueTime.setVisible( newVal ); }
    public void showCorrelationStart( boolean newVal ) { _correlationStart.setVisible( newVal ); }
    public void showCorrelationEnd( boolean newVal ) { _correlationEnd.setVisible( newVal ); }
    public void showCorrelationTime( boolean newVal ) { _correlationTime.setVisible( newVal ); }
    public void showJobStart( boolean newVal ) { _jobStartText.setVisible( newVal ); }
    public void showJobDuration( boolean newVal ) { _jobDurationText.setVisible( newVal ); }
    public void showInputFile( boolean newVal ) { _inputFile.setVisible( newVal ); }
    public void showOutputFile( boolean newVal ) { _outputFile.setVisible( newVal ); }
    public void showOutputSize( boolean newVal ) { _outputSize.setVisible( newVal ); }
    public void showDifxVersion( boolean newVal ) { _difxVersion.setVisible( newVal ); }
    public void showSpeedUpFactor( boolean newVal ) { _speedUpFactor.setVisible( false ); }
    public void showTimeRemaining( boolean newVal ) { _timeRemaining.setVisible( false ); }
    public void showNumAntennas( boolean newVal ) { _numAntennas.setVisible( newVal ); }
    public void showNumForeignAntennas( boolean newVal ) { _numForeignAntennas.setVisible( newVal ); }
    public void showDutyCycle( boolean newVal ) { _dutyCycleText.setVisible( newVal ); }
    public void showStatus( boolean newVal ) { _status.setVisible( newVal ); }
    public void showActive( boolean newVal ) { _active.setVisible( newVal ); }
    public void showStatusId( boolean newVal ) { _statusId.setVisible( newVal ); }
    public void showWeights( boolean newVal ) { 
        _showWeights = newVal;
        this.updateUI();
    }
    public void showWeightsAsPlots( boolean newVal ) { 
        _showWeightsAsPlots = newVal;
        this.updateUI();
    }
    
    public void widthName( int newVal ) { _widthName = newVal; }
    public void widthProgressBar( int newVal ) { _widthProgressBar = newVal; }
    public void widthState( int newVal ) { _widthState = newVal; }
    public void widthExperiment( int newVal ) { _widthExperiment = newVal; }
    public void widthPass( int newVal ) { _widthPass = newVal; }
    public void widthPriority( int newVal ) { _widthPriority = newVal; }
    public void widthQueueTime( int newVal ) { _widthQueueTime = newVal; }
    public void widthCorrelationStart( int newVal ) { _widthCorrelationStart = newVal; }
    public void widthCorrelationEnd( int newVal ) { _widthCorrelationEnd = newVal; }
    public void widthCorrelationTime( int newVal ) { _widthCorrelationTime = newVal; }
    public void widthJobStart( int newVal ) { _widthJobStart = newVal; }
    public void widthJobDuration( int newVal ) { _widthJobDuration = newVal; }
    public void widthInputFile( int newVal ) { _widthInputFile = newVal; }
    public void widthOutputFile( int newVal ) { _widthOutputFile = newVal; }
    public void widthOutputSize( int newVal ) { _widthOutputSize = newVal; }
    public void widthDifxVersion( int newVal ) { _widthDifxVersion = newVal; }
    public void widthSpeedUpFactor( int newVal ) { _widthSpeedUpFactor = newVal; }
    public void widthTimeRemaining( int newVal ) { _widthTimeRemaining = newVal; }
    public void widthNumAntennas( int newVal ) { _widthNumAntennas = newVal; }
    public void widthNumForeignAntennas( int newVal ) { _widthNumForeignAntennas = newVal; }
    public void widthDutyCycle( int newVal ) { _widthDutyCycle = newVal; }
    public void widthStatus( int newVal ) { _widthStatus = newVal; }
    public void widthActive( int newVal ) { _widthActive = newVal; }
    public void widthStatusId( int newVal ) { _widthStatusId = newVal; }
    public void widthWeights( int newVal ) { _widthWeights = newVal; }
    
    public PassNode passNode() {
        return _passNode;
    }
    public void passNode( PassNode newNode ) {
        _passNode = newNode;
    }
    
    public JobEditorMonitor editorMonitor() { 
        updateEditorMonitor( null );
        return _editorMonitor;
    }
    
    public boolean running() {
        return _running;
    }
    public void running( boolean newVal ) {
        _running = newVal;
    }
    
    public boolean lockState() {
        return _lockState;
    }
    public void lockState( boolean newVal ) {
        _lockState = newVal;
        //_stopJobItem.setEnabled( !newVal );
        if ( _editorMonitor != null )
            _editorMonitor.lockState( newVal );
    }
    
    //--------------------------------------------------------------------------
    //  Called to set all displayed items ("state" values, progress bars, etc.) both
    //  in the JobNode and the Editor/Monitor to indicate a job is starting.
    //--------------------------------------------------------------------------
    public void setJobStart() {
        _startTime = new JulianCalendar();
        _startTime.setTime( new Date() );
        correlationStart( _startTime.mjd() );
        running( true );
//        setState( "Initializing", Color.YELLOW );
        setProgress( 0 );
        _jobTotalTime = null;
        _jobCorrelationTime = null;
        _fractionComplete = null;
        _computationalStartTime = null;
        _speedUp = null;
        _jobStartTime = (double)System.currentTimeMillis();
        
        _firstFraction = null;
        _timePerFrac = null;
        _firstTime = null;
        _startTimeBuffer = _settings.queueBrowser().estimateStartBuffer();
        lockState( false );
    }
        
    //--------------------------------------------------------------------------
    //  Initial estimate the time remaining based on previous work.  Might be good,
    //  might really stink.
    //--------------------------------------------------------------------------
    public void estimateJobTime() {
        _timeRemainingTime = (double)System.currentTimeMillis() / 1000.0;
        _currentTimeRemaining = _settings.queueBrowser().estimateProcessTime( editorMonitor()._inputFile.baselineTable().num,
                _jobDuration, editorMonitor().threadsInUse() );
        timeRemaining( _currentTimeRemaining );
    }
    
    //--------------------------------------------------------------------------
    //  Called to indicate in the displays that a job has completed.
    //--------------------------------------------------------------------------
    public void setJobEnd() {
        running( false );
        JulianCalendar endTime = new JulianCalendar();
        endTime.setTime( new Date() );
        correlationEnd( endTime.mjd() );
        correlationTime( 24.0 * 3600.0 * ( endTime.mjd() - _startTime.mjd() ) );
        //  Send statistics to the queue browser.  These are used to provide initial estimates
        //  of the time required for future jobs.
        _settings.queueBrowser().jobRunStats( _startTimeBuffer, 
                (double)System.currentTimeMillis() / 1000.0 - _jobStartTime / 1000.0,
                editorMonitor()._inputFile.baselineTable().num,
                _jobDuration, editorMonitor().threadsInUse(),
                _currentTimeRemaining - ( (double)System.currentTimeMillis() / 1000.0 - _timeRemainingTime ) );
        timeRemaining( 0.0 );
        _calculatedTimeRemaining = 0.0;
    }
    
    /*
     * Set the "state" of this job.  The state appears on both the editor/monitor
     * and the queue browser line.  It has a background color.
     */
    public void setState( String newState, Color newColor ) {
        _state.setText( newState );
        _state.setBackground( newColor );
        _state.updateUI();  //  necessary??
        if ( _editorMonitor != null )
            _editorMonitor.setState( newState, newColor );
    }
    
    public ColumnTextArea state() { return _state; }
    public JProgressBar progress() { return _progress; }
    
    protected PassNode _passNode;
    
    protected Double _jobTotalTime;
    protected Double _fractionComplete;
    protected Double _jobStartTime;
    protected Double _speedUp;
    protected Double _jobCorrelationTime;
    
    protected Double _firstFraction;
    protected Double _timePerFrac;
    protected Double _firstTime;
    protected Double _startTimeBuffer;
    
    protected JulianCalendar _startTime;
    
    protected JButton _startButton;
    protected JButton _editButton;
    protected JobEditorMonitor _editorMonitor;
    protected int _xOff;
    protected int _widthName;
    protected JProgressBar _progress;
    protected int _widthProgressBar;
    protected ActivityMonitorLight _networkActivity;
    protected ColumnTextArea _state;
    protected int _widthState;
    protected ColumnTextArea _experiment;
    protected int _widthExperiment;
    protected ColumnTextArea _pass;
    protected int _widthPass;
    protected ColumnTextArea _priority;
    protected int _widthPriority;
    protected ColumnTextArea _queueTime;
    protected int _widthQueueTime;
    protected ColumnTextArea _correlationStart;
    protected int _widthCorrelationStart;
    protected ColumnTextArea _correlationEnd;
    protected int _widthCorrelationEnd;
    protected ColumnTextArea _correlationTime;
    protected int _widthCorrelationTime;
    protected ColumnTextArea _jobStartText;
    protected int _widthJobStart;
    protected Double _jobStart;
    protected ColumnTextArea _jobDurationText;
    protected int _widthJobDuration;
    protected Double _jobDuration;
    protected ColumnTextArea _inputFile;
    protected int _widthInputFile;
    protected ColumnTextArea _outputFile;
    protected int _widthOutputFile;
    protected ColumnTextArea _outputSize;
    protected int _widthOutputSize;
    protected ColumnTextArea _difxVersion;
    protected int _widthDifxVersion;
    protected ColumnTextArea _speedUpFactor;
    protected int _widthSpeedUpFactor;
    protected ColumnTextArea _timeRemaining;
    protected int _widthTimeRemaining;
    protected ColumnTextArea _numAntennas;
    protected int _widthNumAntennas;
    protected ColumnTextArea _numForeignAntennas;
    protected int _widthNumForeignAntennas;
    protected ColumnTextArea _dutyCycleText;
    protected int _widthDutyCycle;
    protected Double _dutyCycle;
    protected ColumnTextArea _status;
    protected int _widthStatus;
    protected ActivityMonitorLight _active;
    protected int _widthActive;
    protected ColumnTextArea _statusId;
    protected int _widthStatusId;
    protected double[] _weights;
    protected String[] _antennas;
    protected boolean _showWeights;
    protected boolean _showWeightsAsPlots;
    protected int _widthWeights;
    protected ColumnTextArea[] _weight;
    protected ColumnTextArea[] _antenna;
    protected PlotWindow[] _weightPlotWindow;
    protected Plot2DObject[] _weightPlot;
    protected Track2D[] _weightTrack;
    protected int[] _weightTrackSize;
    
    protected JMenuItem _monitorMenuItem;
    protected JMenuItem _liveMonitorMenuItem;
    
    protected boolean _colorColumn;
    protected Color _columnColor;

    protected String _directoryPath;
    protected String _calcFile;
    protected String _fullName;
    
    protected SystemSettings _settings;
    
    protected boolean _running;
    protected boolean _lockState;
//    protected Integer _databaseJobId;
    
    protected ZMenuItem _scheduleJobItem;
    protected ZMenuItem _runCalcOnJob;
    protected ZMenuItem _scheduleConfigTestItem;
    protected ZMenuItem _unscheduleJobItem;
    protected ZMenuItem _stopJobItem;
    
    protected boolean _weightsBuilt;
    
    protected JobNode _this;
    protected String _logFile;
    
    protected Object _antennaLock;
        
    protected double _timeRemainingTime;
    protected double _currentTimeRemaining;
    protected Double _computationalStartTime;
    protected double _computationalFraction;
    
    protected Double _calculatedTimeRemaining;
    
    public Double calculatedTimeRemaining() { return _calculatedTimeRemaining; }
    public void initializeTimeRemaining() {
        _calculatedTimeRemaining = null;
    }
}
