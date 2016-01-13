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
/*
 * This window contains all of the settings for a single job, as well as controls
 * and displays to run it and monitor its progress.
 */
package edu.nrao.difx.difxview;

import javax.swing.Timer;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.plaf.basic.ComboPopup;
import javax.swing.plaf.basic.BasicComboBoxUI;
import javax.swing.plaf.basic.BasicComboPopup;

import edu.nrao.difx.difxutilities.DiFXCommand;
import edu.nrao.difx.difxutilities.InputFileParser;
import edu.nrao.difx.difxutilities.CalcFileParser;
import edu.nrao.difx.difxutilities.V2dFileParser;
import edu.nrao.difx.difxutilities.ChannelServerSocket;
import edu.nrao.difx.difxutilities.DiFXCommand_ls;
import edu.nrao.difx.difxutilities.DiFXCommand_mv;
import edu.nrao.difx.difxutilities.DiFXCommand_vex2difx;
import static edu.nrao.difx.difxview.JobNode.AUTOSTATE_STOPPED;

import edu.nrao.difx.xmllib.difxmessage.DifxMessage;
import edu.nrao.difx.xmllib.difxmessage.DifxMachinesDefinition;
import edu.nrao.difx.xmllib.difxmessage.DifxStart;
import edu.nrao.difx.xmllib.difxmessage.DifxStop;
import edu.nrao.difx.xmllib.difxmessage.DifxJobLog;
import edu.nrao.difx.xmllib.difxmessage.DifxJobLog.Data.*;
import edu.nrao.difx.xmllib.difxmessage.ObjectFactory;

import java.awt.*;

import javax.swing.JFrame;
import javax.swing.JMenuBar;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JTextField;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JCheckBox;
import javax.swing.JPopupMenu;
import javax.swing.JButton;
import javax.swing.JProgressBar;
import javax.swing.BorderFactory;
import javax.swing.border.Border;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;

import java.text.SimpleDateFormat;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import javax.swing.event.EventListenerList;

//import java.io.PrintWriter;
//import java.io.FileWriter;
//import java.io.BufferedWriter;
import java.net.SocketTimeoutException;
//import java.net.InetAddress;
//import java.net.UnknownHostException;

import java.util.*;

import java.awt.event.ComponentEvent;
//import java.io.IOException;

//import javax.xml.bind.Marshaller;

//import java.io.StringWriter;
//import java.net.SocketException;

import mil.navy.usno.widgetlib.IndexedPanel;
import mil.navy.usno.widgetlib.NodeBrowserScrollPane;
import mil.navy.usno.widgetlib.NumberBox;
import mil.navy.usno.widgetlib.BrowserNode;
import mil.navy.usno.widgetlib.SimpleTextEditor;
import mil.navy.usno.widgetlib.MessageDisplayPanel;
//import mil.navy.usno.widgetlib.JulianCalendar;
import mil.navy.usno.widgetlib.ZButton;
import mil.navy.usno.widgetlib.ZCheckBox;
import mil.navy.usno.widgetlib.ZLabel;

public class JobEditorMonitor extends JFrame {
    
    /*
     * The JobNode gives us access to all of the data known about this job.
     */
    public JobEditorMonitor( JobNode newNode, SystemSettings settings ) {
        super( "Job Editor/Monitor" );
        _this = this;
        _jobNode = newNode;
        _settings = settings;
        _settings.setLookAndFeel();
        this.setLayout( null );
        this.setBounds( 500, 100, _settings.windowConfiguration().jobEditorMonitorWindowW,
                _settings.windowConfiguration().jobEditorMonitorWindowH );
    	this.addComponentListener( new java.awt.event.ComponentAdapter() {
            public void componentResized( ComponentEvent e ) {
                _settings.windowConfiguration().jobEditorMonitorWindowW = _this.getWidth();
                _settings.windowConfiguration().jobEditorMonitorWindowH = _this.getHeight();
                newSize();
            }
        });
        this.setTitle( "Controls for " + _jobNode.name() );
        _menuBar = new JMenuBar();
        _menuBar.setVisible( true );
        JMenu helpMenu = new JMenu( "  Help  " );
        _menuBar.add( helpMenu );
        JMenuItem controlHelpItem = new JMenuItem( "Control/Monitor Help" );
        controlHelpItem.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _settings.launchGUIHelp( "Job_Control_Window.html" );
            }
        } );
        helpMenu.add( controlHelpItem );
        JMenuItem helpIndexItem = new JMenuItem( "GUI Documentation" );
        helpIndexItem.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _settings.launchGUIHelp( "intro.html" );
            }
        } );
        helpMenu.add( helpIndexItem );
        this.add( _menuBar );

        _scrollPane = new NodeBrowserScrollPane();
        _scrollPane.respondToResizeEvents( true );
        _scrollPane.addTimeoutEventListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                newSize();
            }
        } );
        _scrollPane.addResizeEventListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                newSize();
            }
        } );
        this.add( _scrollPane );
        
        //  This panel shows us the input file, which can be edited and sent to
        //  the DiFX host.  The existing file on the DiFX host can also be downloaded.
        IndexedPanel inputFilePanel = new IndexedPanel( "Input File" );
        inputFilePanel.openHeight( 400 );
        inputFilePanel.closedHeight( 20 );
        inputFilePanel.open( false );
        _scrollPane.addNode( inputFilePanel );
        _inputFileEditor = new SimpleTextEditor();
        inputFilePanel.add( _inputFileEditor );
        _inputFileName = new JLabel( "" );
        inputFilePanel.add( _inputFileName );
        _refreshInputButton = new JButton( "Refresh" );
        _refreshInputButton.setToolTipText( "Read the Input File stored on the DiFX host." );
        _refreshInputButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                statusInfo( "Obtaining file \"" + _inputFileName.getText() + "\" from DiFX host." );
                Component comp = _refreshInputButton;
                while ( comp.getParent() != null )
                    comp = comp.getParent();
                Point pt = _refreshInputButton.getLocationOnScreen();
                GetFileMonitor getFile = new GetFileMonitor(  (Frame)comp, pt.x + 25, pt.y + 25,
                        _inputFileName.getText(), _settings, false );
                if ( getFile.inString() != null && getFile.inString().length() > 0 ) {
                    _inputFileEditor.text( getFile.inString() );
                    parseInputFile();
                }
            }
        } );
        inputFilePanel.add( _refreshInputButton );
        _uploadInputButton = new JButton( "Save" );
        _uploadInputButton.setToolTipText( "Parse all settings from the editor text and upload to the Input File location on the DiFX host (not necessary unless you have changed the text)." );
        _uploadInputButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                parseInputFile();
                Component comp = _uploadInputButton;
                while ( comp.getParent() != null )
                    comp = comp.getParent();
                Point pt = _uploadInputButton.getLocationOnScreen();
                SendFileMonitor sendFile = new SendFileMonitor(  (Frame)comp, pt.x + 25, pt.y + 25,
                        _inputFileName.getText(), _inputFileEditor.text(), _settings );
            }
        } );
        inputFilePanel.add( _uploadInputButton );

        //  This panel shows us the calc file, which can be edited and sent to
        //  the DiFX host.  The existing file on the DiFX host can also be downloaded.
        IndexedPanel calcFilePanel = new IndexedPanel( "Calc File" );
        calcFilePanel.openHeight( 400 );
        calcFilePanel.closedHeight( 20 );
        calcFilePanel.open( false );
        _scrollPane.addNode( calcFilePanel );
        _calcFileEditor = new SimpleTextEditor();
        calcFilePanel.add( _calcFileEditor );
        _calcFileName = new JLabel( "" );
        calcFilePanel.add( _calcFileName );
        _refreshCalcButton = new JButton( "Refresh" );
        _refreshCalcButton.setToolTipText( "Read the Calc File as it is stored on the DiFX host." );
        _refreshCalcButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                Component comp = _refreshCalcButton;
                while ( comp.getParent() != null )
                    comp = comp.getParent();
                Point pt = _refreshCalcButton.getLocationOnScreen();
                GetFileMonitor getFile = new GetFileMonitor(  (Frame)comp, pt.x + 25, pt.y + 25,
                        _calcFileName.getText(), _settings, false );
                if ( getFile.inString() != null && getFile.inString().length() > 0 ) {
                    _calcFileEditor.text( getFile.inString() );
                    parseCalcFile( _calcFileEditor.text() );
                }
            }
        } );
        calcFilePanel.add( _refreshCalcButton );
        _uploadCalcButton = new JButton( "Save" );
        _uploadCalcButton.setToolTipText( "Parse all settings from the editor text and upload to the Calc File location on the DiFX host (not necessary unless you have changed the text)." );
        _uploadCalcButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                parseCalcFile( _calcFileEditor.text() );
                Component comp = _uploadCalcButton;
                while ( comp.getParent() != null )
                    comp = comp.getParent();
                Point pt = _uploadCalcButton.getLocationOnScreen();
                SendFileMonitor sendFile = new SendFileMonitor(  (Frame)comp, pt.x + 25, pt.y + 25,
                        _calcFileName.getText(), _calcFileEditor.text(), _settings );
            }
        } );
        calcFilePanel.add( _uploadCalcButton );

        IndexedPanel machinesListPanel = new IndexedPanel( "Machines List" );
        machinesListPanel.openHeight( 425 );
        machinesListPanel.closedHeight( 20 );
        _scrollPane.addNode( machinesListPanel );
        _dataSourcesPane = new NodeBrowserScrollPane();
        _dataSourcesPane.setBackground( Color.WHITE );
        _dataSourcesPane.decayCount( 5 );
        machinesListPanel.add( _dataSourcesPane );
        _dataSourcesLabel = new ZLabel( "Data Sources:" );
        _dataSourcesLabel.setHorizontalAlignment( JLabel.LEFT );
        machinesListPanel.add( _dataSourcesLabel );
        _skipStationLabel = new ZLabel( "Skip" );
        _skipStationLabel.toolTip( "Eliminate checked stations from the job processing.  This requires\n"
                + "rebuilding the job using the \"Rebuild Job\" button.", null );
        _skipStationLabel.setHorizontalAlignment( JLabel.LEFT );
        machinesListPanel.add( _skipStationLabel );
        _processorsPane = new NodeBrowserScrollPane();
        _processorsPane.setBackground( Color.WHITE );
        _processorsPane.decayCount( 5 );
        machinesListPanel.add( _processorsPane );
        _processorsLabel = new JLabel( "Processor Nodes:" );
        _processorsLabel.setHorizontalAlignment( JLabel.LEFT );
        machinesListPanel.add( _processorsLabel );
        _threadsLabel = new JLabel( "Threads:" );
        _threadsLabel.setHorizontalAlignment( JLabel.RIGHT );
        machinesListPanel.add( _threadsLabel );
        _coresLabel = new JLabel( "Cores:" );
        _coresLabel.setHorizontalAlignment( JLabel.RIGHT );
        machinesListPanel.add( _coresLabel );
        _cpuUsageLabel = new JLabel( "% CPU Usage:" );
        _cpuUsageLabel.setHorizontalAlignment( JLabel.RIGHT );
        machinesListPanel.add( _cpuUsageLabel );
        _mpiTestLabel = new JLabel( "mpi Test:" );
        _mpiTestLabel.setHorizontalAlignment( JLabel.RIGHT );
        machinesListPanel.add( _mpiTestLabel );
        _headNode = new JFormattedTextField();
        _headNode.setFocusLostBehavior( JFormattedTextField.COMMIT );
        _headNode.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _settings.headNode( _headNode.getText() );
            }
        } );
        _headNode.setText( _settings.headNode() );
        machinesListPanel.add( _headNode );
        _headNodeLabel = new JLabel( "HeadNode:" );
        _headNodeLabel.setHorizontalAlignment( JLabel.LEFT );
        machinesListPanel.add( _headNodeLabel );
        _applyMachinesButton = new JButton( "Apply" );
        _applyMachinesButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                //  Track whether this button was ever pushed - indicating the user
                //  has already generated the .threads and .machines files.  This
                //  is used later when the "Start" button is pushed - it will need
                //  to generate these files if the user has not.
                _machinesAppliedByHand = true;
                applyMachinesAction();
            }
        } );
        machinesListPanel.add( _applyMachinesButton );
        _restrictHeadnodeProcessing = new JCheckBox( "Restrict Headnode Processing" );
        _restrictHeadnodeProcessing.setToolTipText( "Do not do any multicore DiFX processing on the headnode." );
        _restrictHeadnodeProcessing.setSelected( _settings.defaultNames().restrictHeadnodeProcessing );
        _restrictHeadnodeProcessing.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _settings.defaultNames().restrictHeadnodeProcessing = _restrictHeadnodeProcessing.isSelected();
            }
        } );
        machinesListPanel.add( _restrictHeadnodeProcessing );
        _eliminateNonrespondingProcessors = new JCheckBox( "Eliminate Non-Responding Processors" );
        _eliminateNonrespondingProcessors.setSelected( _settings.defaultNames().eliminateNonrespondingProcessors );
        _eliminateNonrespondingProcessors.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _settings.defaultNames().eliminateNonrespondingProcessors = _eliminateNonrespondingProcessors.isSelected();
            }
        } );
        machinesListPanel.add( _eliminateNonrespondingProcessors );
        _eliminateBusyProcessors = new JCheckBox( "Eliminate Processors Over" );
        _eliminateBusyProcessors.setSelected( _settings.defaultNames().eliminateNonrespondingProcessors );
        _eliminateBusyProcessors.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                changeBusyProcessorsSettings();
                _settings.defaultNames().eliminateBusyProcessors = _eliminateBusyProcessors.isSelected();
            }
        } );
        machinesListPanel.add( _eliminateBusyProcessors );
        _busyPercentage = new NumberBox();
        _busyPercentage.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                changeBusyProcessorsSettings();
                _settings.defaultNames().busyPercentage = _busyPercentage.value();
            }
        } );
        _busyPercentage.precision( 0 );
        _busyPercentage.value( _settings.defaultNames().busyPercentage );
        _busyPercentage.limits( 0.0, 100.0 );
        machinesListPanel.add( _busyPercentage );
        _busyPercentageLabel = new JLabel( "% Busy" );
        machinesListPanel.add( _busyPercentageLabel );
        _chooseBasedOnModule = new JCheckBox( "Choose Data Source Based on Module" );
        _chooseBasedOnModule.setSelected( _settings.defaultNames().chooseBasedOnModule );
        _chooseBasedOnModule.setToolTipText( "Pick Mark5 data sources that contain required modules." );
        _chooseBasedOnModule.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                if ( _chooseBasedOnModule.isSelected() )
                    buildDataSourceList();
                _settings.defaultNames().chooseBasedOnModule = _chooseBasedOnModule.isSelected();
            }
        } );
        _defaultMachinesButton = new ZButton( "Use Defaults" );
        _defaultMachinesButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                selectNodeDefaults( true, false );
            }
        } );
        _defaultMachinesButton.toolTip( "Set data source and processing nodes to sensible defaults\n"
                + "base on user preferences and current node usage.\n"
                + "This will replace any user-selected nodes.", null );
        machinesListPanel.add( _defaultMachinesButton );
        _removeSkippedStationsButton = new ZButton( "Rebuild Job" );
        _removeSkippedStationsButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                removeSkippedStations( false );
            }
        } );
        _removeSkippedStationsButton.toolTip( "Rebuild the job with \"skipped\" stations eliminated from\n"
                + "the processing.", null );
        machinesListPanel.add( _removeSkippedStationsButton );
        _selectAllProcessorsButton = new JButton( "Select All" );
        _selectAllProcessorsButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                selectAllProcessors();
            }
        } );
        machinesListPanel.add( _selectAllProcessorsButton );
        _deselectAllProcessorsButton = new JButton( "Deselect All" );
        _deselectAllProcessorsButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                deselectAllProcessors();
            }
        } );
        machinesListPanel.add( _deselectAllProcessorsButton );
        machinesListPanel.add( _chooseBasedOnModule );

        //  This panel shows us the machines file, which can be edited and sent to
        //  the DiFX host.  The existing file on the DiFX host can also be downloaded.
        IndexedPanel machinesFilePanel = new IndexedPanel( "Machines File" );
        machinesFilePanel.openHeight( 300 );
        machinesFilePanel.closedHeight( 20 );
        machinesFilePanel.open( false );
        _scrollPane.addNode( machinesFilePanel );
        _machinesFileEditor = new SimpleTextEditor();
        machinesFilePanel.add( _machinesFileEditor );
        _machinesFileName = new JLabel( "" );
        machinesFilePanel.add( _machinesFileName );
        _refreshMachinesButton = new JButton( "Refresh" );
        _refreshMachinesButton.setToolTipText( "Read the Machines File as it is stored on the DiFX host." );
        _refreshMachinesButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                Component comp = _refreshMachinesButton;
                while ( comp.getParent() != null )
                    comp = comp.getParent();
                Point pt = _refreshMachinesButton.getLocationOnScreen();
                GetFileMonitor getFile = new GetFileMonitor(  (Frame)comp, pt.x + 25, pt.y + 25,
                        _machinesFileName.getText(), _settings, false );
                if ( getFile.inString() != null && getFile.inString().length() > 0 ) {
                    _machinesFileEditor.text( getFile.inString() );
                }
            }
        } );
        machinesFilePanel.add( _refreshMachinesButton );
        _uploadMachinesButton = new JButton( "Save" );
        _uploadMachinesButton.setToolTipText( "Parse all settings from the editor text and upload to the Machines File location on the DiFX host (not necessary unless you have changed the text)." );
        _uploadMachinesButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _machinesAppliedByHand = true;
                Component comp = _uploadMachinesButton;
                while ( comp.getParent() != null )
                    comp = comp.getParent();
                Point pt = _uploadMachinesButton.getLocationOnScreen();
                SendFileMonitor sendFile = new SendFileMonitor(  (Frame)comp, pt.x + 25, pt.y + 25,
                        _machinesFileName.getText(), _machinesFileEditor.text(), _settings );
                statusInfo( "Sent machines file content (" + _machinesFileEditor.text().length()
                        + " chars) to \"" + _machinesFileName.getText() + "\" on DiFX host." );
            }
        } );
        machinesFilePanel.add( _uploadMachinesButton );

        //  This panel shows us the threads file, which can be edited and sent to
        //  the DiFX host.  The existing file on the DiFX host can also be downloaded.
        IndexedPanel threadsFilePanel = new IndexedPanel( "Threads File" );
        threadsFilePanel.openHeight( 300 );
        threadsFilePanel.closedHeight( 20 );
        threadsFilePanel.open( false );
        _scrollPane.addNode( threadsFilePanel );
        _threadsFileEditor = new SimpleTextEditor();
        threadsFilePanel.add( _threadsFileEditor );
        _threadsFileName = new JLabel( "" );
        threadsFilePanel.add( _threadsFileName );
        _refreshThreadsButton = new JButton( "Refresh" );
        _refreshThreadsButton.setToolTipText( "Read the Threads File as it is stored on the DiFX host." );
        _refreshThreadsButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                statusInfo( "Reading thread file \"" + _threadsFileName.getText() + "\" from DiFX host." );
                Component comp = _refreshThreadsButton;
                while ( comp.getParent() != null )
                    comp = comp.getParent();
                Point pt = _refreshThreadsButton.getLocationOnScreen();
                GetFileMonitor getFile = new GetFileMonitor(  (Frame)comp, pt.x + 25, pt.y + 25,
                        _threadsFileName.getText(), _settings, false );
                if ( getFile.inString() != null && getFile.inString().length() > 0 ) {
                    statusInfo( "\"" + _threadsFileName.getText() + "\" (" + getFile.inString().length() + " chars) read from DiFX host." );
                    _threadsFileEditor.text( getFile.inString() );
                }
                else {
                    if ( getFile.inString() == null )
                        statusWarning( "No \"" + _threadsFileName.getText() + "\" file found on DiFX host." );
                    else
                        statusWarning( "\"" + _threadsFileName.getText() + "\" was zero length on DiFX host." );
                }
            }
        } );
        threadsFilePanel.add( _refreshThreadsButton );
        _uploadThreadsButton = new JButton( "Save" );
        _uploadThreadsButton.setToolTipText( "Parse all settings from the editor text and upload to the Threads File location on the DiFX host (not necessary unless you have changed the text)." );
        _uploadThreadsButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _machinesAppliedByHand = true;
                Component comp = _uploadThreadsButton;
                while ( comp.getParent() != null )
                    comp = comp.getParent();
                Point pt = _uploadThreadsButton.getLocationOnScreen();
                SendFileMonitor sendFile = new SendFileMonitor(  (Frame)comp, pt.x + 25, pt.y + 25,
                        _threadsFileName.getText(), _threadsFileEditor.text(), _settings );
                statusInfo( "Sent threads file content (" + _threadsFileEditor.text().length()
                        + " chars) to \"" + _threadsFileName.getText() + "\" on DiFX host." );
            }
        } );
        threadsFilePanel.add( _uploadThreadsButton );

        IndexedPanel runControlPanel = new IndexedPanel( "Run Controls" );
        runControlPanel.openHeight( 100 );
        runControlPanel.closedHeight( 20 );
        _scrollPane.addNode( runControlPanel );
        _startButton = new JButton( "Start" );
        _startButton.setBounds( 10, 30, 110, 25 );
        _startButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                startJob( true, false );
            }
        } );
        runControlPanel.add( _startButton );
        _configButton = new ZButton( "Config Test" );
        _configButton.toolTip( "Run a configuration test on the .input file and return.\n"
                + "Except for the test, the job will not be run.", null );
        _configButton.setBounds( 420, 30, 110, 25 );
        _configButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                startJob( true, true );
            }
        } );
        runControlPanel.add( _configButton );
        _stopButton = new JButton( "Stop" );
        _stopButton.setBounds( 10, 60, 110, 25 );
        _stopButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                stopJob();
            }
        } );
        _stopButton.setEnabled( false );
        runControlPanel.add( _stopButton );
        _restartAt = new JCheckBox( "Restart" );
        _restartAt.setBounds( 130, 30, 75, 25 );
        _restartAt.setSelected( false );
        runControlPanel.add( _restartAt );
        _restartSeconds = new NumberBox();
        _restartSeconds.precision( 4 );
        _restartSeconds.value( 0.0 );
        _restartSeconds.setBounds( 230, 30, 100, 25 );
        runControlPanel.add( _restartSeconds );
        _forceOverwrite = new JCheckBox( "Force Overwrite" );
        _forceOverwrite.setToolTipText( "Force the overwrite of the \".difx\" output file if one already exists." );
        _forceOverwrite.setSelected( true );
        _forceOverwrite.setBounds( 130, 60, 150, 25 );
        runControlPanel.add( _forceOverwrite );
        JLabel restartAtLabel = new JLabel( "at:" );
        restartAtLabel.setBounds( 180, 30, 45, 25 );
        restartAtLabel.setHorizontalAlignment( JLabel.RIGHT );
        runControlPanel.add( restartAtLabel );
        JLabel restartSecondsLabel = new JLabel( "seconds" );
        restartSecondsLabel.setBounds( 335, 30, 90, 25 );
        restartSecondsLabel.setHorizontalAlignment( JLabel.LEFT );
        runControlPanel.add( restartSecondsLabel );
        _showMonitorButton = new JButton( "Show Monitor" );
        _showMonitorButton.setToolTipText( "Launch real-time monitoring for this job." );
        _showMonitorButton.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                showLiveMonitor();
            }
        } );
        runControlPanel.add( _showMonitorButton );
        _runMonitor = new JCheckBox( "Run With Monitor" );
        _runMonitor.setToolTipText( "Run the background software required for real-time monitoring." );
        _runMonitor.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                if ( _runMonitor.isSelected() )
                    _showMonitorButton.setEnabled( true );
                else
                    _showMonitorButton.setEnabled( false );
                _settings.defaultNames().runMonitor = _runMonitor.isSelected();
            }
        } );
        runControlPanel.add( _runMonitor );
        _runMonitor.setSelected( _settings.defaultNames().runMonitor ); 
        if ( _runMonitor.isSelected() )
            _showMonitorButton.setEnabled( true );
        else
            _showMonitorButton.setEnabled( false );
        _runMonitor.setSelected( false );
        //_runMonitor.setEnabled( false );
        
        //  The Status Panel shows the current state of the job.
        _statusPanel = new IndexedPanel( "" );
        _scrollPane.addNode( _statusPanel );
        _statusPanel.openHeight( 60 );
        _statusPanel.alwaysOpen( true );
        _statusPanel.noArrow( true );
        _statusPanelBackground = _statusPanel.getBackground();
        _statusPanel.setBackground( _statusPanelBackground );
        _statusLabel = new JLabel( "" );
        _statusLabel.setHorizontalAlignment( JLabel.RIGHT );
        _statusPanel.add( _statusLabel );
        _state = new ColumnTextArea();
        _state.justify( ColumnTextArea.CENTER );
        _state.setText( "not started" );
        _statusPanel.add( _state );
        _progress = new JProgressBar( 0, 100 );
        _progress.setValue( 0 );
        _progress.setStringPainted( true );
        _statusPanel.add( _progress );
        
        //  The message panel shows raw message data pertaining to the job.
        IndexedPanel messagePanel = new IndexedPanel( "Messages" );
        _scrollPane.addNode( messagePanel );
        messagePanel.openHeight( 200 );
        messagePanel.closedHeight( 20 );
        _messageDisplayPanel = new MessageDisplayPanel();
        messagePanel.add( _messageDisplayPanel );
 
        _allObjectsBuilt = true;
        
        //  Start a thread that can be used to trigger repeated updates.
//        _updateThread = new UpdateThread( 1000 );
//        _updateThread.start();
        
        newSize();

    }
    
    public void close() {
        _scrollPane.close();
        _messageDisplayPanel.close();
        _dataSourcesPane.close();
        _processorsPane.close();
    }
    
    protected class UpdateThread extends Thread {
        protected int _interval;
        protected boolean _keepGoing;
        public UpdateThread( int i ) {
            _interval = i;
            _keepGoing = true;
        }
        public void keepGoing( boolean newVal ) {
            _keepGoing = newVal;
        }
        @Override
        public void run() {
            while ( _keepGoing ) {
                timeoutIntervalEvent();
                try {
                    Thread.sleep( _interval );
                } catch ( Exception e ) {
                    _keepGoing = false;
                }
            }
        }
    }
                
    
    @Override
    public void setBounds( int x, int y, int w, int h ) {
        super.setBounds( x, y, w, h );
        newSize();
    }
    
    public void newSize() {
        //  The current sizes are saved in the settings...but we can't be certain
        //  the _settings variable has been set yet.
        if ( _allObjectsBuilt ) {
            int w = this.getContentPane().getSize().width;
            int h = this.getContentPane().getSize().height;
            _menuBar.setBounds( 0, 0, w, 25 );
            _scrollPane.setBounds( 0, 25, w, h - 25 );
            int thirdSize = ( w - 60 ) / 3;
            _dataSourcesLabel.setBounds( 10, 25, 2 * thirdSize, 25 );
            _skipStationLabel.setBounds( 220, 25, 50, 25 );
            _dataSourcesPane.setBounds( 10, 50, 2 * thirdSize, 150 );
            _chooseBasedOnModule.setBounds( 30 + 2 * thirdSize, 50, thirdSize, 25 );
            _defaultMachinesButton.setBounds( 30 + 2 * thirdSize, 80, thirdSize/2 - 5, 25 );
            _removeSkippedStationsButton.setBounds( 30 + 2 * thirdSize, 110, thirdSize/2 - 5, 25 );
            _processorsLabel.setBounds( 10, 205, 2 * thirdSize, 25 );
            _processorsPane.setBounds( 10, 230, 2 * thirdSize, 150 );
            _threadsLabel.setBounds( 170, 205, 80, 25 );
            _coresLabel.setBounds( 240, 205, 80, 25 );
            _cpuUsageLabel.setBounds( 340, 205, 100, 25 );
            _mpiTestLabel.setBounds( 440, 205, 100, 25 );
            _headNodeLabel.setBounds( 30 + 2 * thirdSize, 205, thirdSize, 25 );
            _headNode.setBounds( 30 + 2 * thirdSize, 230, thirdSize, 25 );
            _applyMachinesButton.setBounds( 30 + 2 * thirdSize, 355, thirdSize/2 - 5, 25 );
            _restrictHeadnodeProcessing.setBounds( 30 + 2 * thirdSize, 265, thirdSize, 25 );
            _eliminateNonrespondingProcessors.setBounds( 30 + 2 * thirdSize, 295, thirdSize, 25 );
            _eliminateBusyProcessors.setBounds( 30 + 2 * thirdSize, 325, thirdSize - 110, 25 );
            _selectAllProcessorsButton.setBounds( 10, 390, 115, 25 );
            _deselectAllProcessorsButton.setBounds( 135, 390, 115, 25 );
            _busyPercentage.setBounds( w - 135, 325, 30, 25 );
            _busyPercentageLabel.setBounds( w - 100, 325, 100, 25 );
            _inputFileEditor.setBounds( 10, 60, w - 35, 330 );
            _calcFileEditor.setBounds( 10, 60, w - 35, 330 );
            _inputFileName.setBounds( 10, 30, w - 350, 25 );
            _calcFileName.setBounds( 10, 30, w - 350, 25 );
            _refreshInputButton.setBounds( w - 125, 30, 100, 25 );
            _refreshCalcButton.setBounds( w - 125, 30, 100, 25 );
            _uploadInputButton.setBounds( w - 230, 30, 100, 25 );
            _uploadCalcButton.setBounds( w - 230, 30, 100, 25 );
            _machinesFileEditor.setBounds( 10, 60, w - 35, 230 );
            _machinesFileName.setBounds( 10, 30, w - 350, 25 );
            _refreshMachinesButton.setBounds( w - 125, 30, 100, 25 );
            _uploadMachinesButton.setBounds( w - 230, 30, 100, 25 );
            _threadsFileEditor.setBounds( 10, 60, w - 35, 230 );
            _threadsFileName.setBounds( 10, 30, w - 350, 25 );
            _refreshThreadsButton.setBounds( w - 125, 30, 100, 25 );
            _uploadThreadsButton.setBounds( w - 230, 30, 100, 25 );
            _runMonitor.setBounds( w - 150, 30, 125, 25 );
            _showMonitorButton.setBounds( w - 150, 60, 125, 25 );
            _messageDisplayPanel.setBounds( 2, 25, w - 23, 173 );
            _state.setBounds( 10, 30, 200, 25 );
            _progress.setBounds( 220, 30, w - 245, 25 );
            _statusLabel.setBounds( 10, 0, w - 35, 25 );
        }
    }
    
    public void showLiveMonitor() {
        if ( _liveMonitorWindow == null )
            _liveMonitorWindow = new LiveMonitorWindow( MouseInfo.getPointerInfo().getLocation().x, 
                MouseInfo.getPointerInfo().getLocation().y, _settings, _inputFileName.getText() );
        _liveMonitorWindow.setVisible( true );
    }
    
    public void statusInfo( String newText ) {
        _statusLabel.setForeground( Color.BLACK );
        _statusLabel.setText( newText );
    }
    
    public void statusWarning( String newText ) {
        _statusLabel.setForeground( Color.YELLOW );
        _statusLabel.setText( newText );
    }
    
    public void statusError( String newText ) {
        _statusLabel.setForeground( Color.RED );
        _statusLabel.setText( newText );
    }
    
    public void statusPanelColor( Color newColor ) {
        _statusPanel.setBackground( newColor );
    }
    
    public void inputFileName( String newName, String content ) { 
        _inputFileName.setText( newName );
        _inputFileEditor.text( content );
    }
    
    public void calcFileName( String newName ) { _calcFileName.setText( newName ); }
    
    public PaneProcessorNode processorNodeByName( String name ) {
        for ( Iterator<BrowserNode> iter = _processorsPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            PaneProcessorNode thisNode = (PaneProcessorNode)(iter.next());
            if ( thisNode.name().contentEquals( name ) )
                return thisNode;
        }
        return null;
    }

    public DataSource dataNodeByName( String name ) {
        for ( Iterator<BrowserNode> iter = _dataSourcesPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            DataSource thisSource = (DataSource)(iter.next());
            if ( thisSource.name().contentEquals( name ) )
                return thisSource;
        }
        return null;
    }

    public BrowserNode nodeByName( String name ) {
        for ( Iterator<BrowserNode> iter = _processorsPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            BrowserNode thisNode = iter.next();
            if ( thisNode.name().contentEquals( name ) )
                return thisNode;
        }
        for ( Iterator<BrowserNode> iter = _dataSourcesPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            BrowserNode thisNode = iter.next();
            if ( thisNode.name().contentEquals( name ) )
                return thisNode;
        }
        return null;
    }
    
    public void selectAllProcessors() {
        for ( Iterator<BrowserNode> iter = _processorsPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            PaneProcessorNode thisNode = (PaneProcessorNode)(iter.next());
            thisNode.handSelected( true );
        }
    }

    public void deselectAllProcessors() {
        for ( Iterator<BrowserNode> iter = _processorsPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            PaneProcessorNode thisNode = (PaneProcessorNode)(iter.next());
            thisNode.handSelected( false );
        }
    }
    public void zeroAllProcessorThreads() {
        for ( Iterator<BrowserNode> iter = _processorsPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            PaneProcessorNode thisNode = (PaneProcessorNode)(iter.next());
            thisNode.threads( 0 );
        }
    }
    
    /*
     * Examine the list of stations used for this job and "skip" any that do no currently
     * have a good data source associated with them.  This process will also "not skip"
     * stations that DO have data (i.e. it will check them all).  A station is "skipped"
     * if its skip checkbox is checked.  Return an indication of whether the job is still
     * "runnable" after removing the missing data - i.e. there are at least two stations
     * left.
     */
    public boolean skipStationsMissingData() {
        int goodCount = 0;
        for ( Iterator<BrowserNode> iter = _dataSourcesPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            DataSource thisSource = (DataSource)(iter.next());
            if ( thisSource.missing() )
                thisSource.skipStation( true );
            if ( !thisSource.skipStation() )
                goodCount += 1;
        }
        return ( goodCount > 1 );
    }
    
    protected boolean _rebuildSuccess;
    protected boolean _rebuildFailed;
    public boolean rebuildSuccess() { return _rebuildSuccess; }
    public boolean rebuildFailed() { return _rebuildFailed; }
    public void rebuildSuccess( boolean newVal ) { _rebuildSuccess = newVal; }
    public void rebuildFailed( boolean newVal ) { _rebuildFailed = newVal; }
    
    public String _newStartSeries;
    public String _oldStartSeries;
    public String _oldDataName;
    public boolean _startSeriesMatch;
    /*
     * Using the existing "general" .v2d file, create a .v2d file that is specific to this job
     * and includes only those stations in the data source list that have not been "skipped".
     * Old versions of files (.calc, .input, .difx, etc) are moved to new locations and given
     * a "version" number.  
     */
    public void removeSkippedStations( boolean externalOperation ) {
        boolean _rebuildSuccess = false;
        boolean _rebuildFailed = false;
        //  Get the existing .v2d file for this experiment.
        _messageDisplayPanel.message( 0, "Job Editor/Monitor", "Rebuilding job with current station specifications" );
        Component comp = this;
        while ( comp.getParent() != null )
            comp = comp.getParent();
        final Frame frm = (Frame)comp;
        final Point pt = _jobNode.getLocationOnScreen();
        _removedList = null;
        GetFileMonitor fileGet = new GetFileMonitor( (Frame)comp, pt.x + 25, pt.y + 25, 
                _jobNode.passNode().fullPath() + "/" + _jobNode.passNode().v2dFileName(), _settings, false );
        //  We only use the content of the file if it was read successfully.
        if ( fileGet.success() ) {
            V2dFileParser v2dParser = new V2dFileParser( fileGet.inString() );
            //  Eliminate stations in the .v2d file that are not selected.
            Vector<String> stationList = v2dParser.antennaList();
            for ( Iterator<String> iter = stationList.iterator(); iter.hasNext(); ) {
                String v2dAntenna = iter.next();
                //  First we have to locate each antenna in our .v2d in the source list.  Many of these
                //  might not be found, in which case they should be eliminated.
                DataSource foundSource = null;
                for ( Iterator<BrowserNode> iter1 = _dataSourcesPane.browserTopNode().children().iterator();
                        iter1.hasNext() && foundSource == null; ) {
                    DataSource thisSource = (DataSource)(iter1.next());
                    if ( thisSource.antenna().equalsIgnoreCase( v2dAntenna ) ) {
                        foundSource = thisSource;
                    }
                }
                //  If the antenna was not among our sources, remove it silently from the .v2d.
                if ( foundSource == null )
                    v2dParser.removeAntenna( v2dAntenna );
                //  If it was "skipped", remove it as well.  This produces a message.
                else if ( foundSource.skipStation() ) {
                    v2dParser.removeAntenna( v2dAntenna );
                    _messageDisplayPanel.message( 0, "Job Editor/Monitor", "removing station \"" + foundSource.antenna() + "\"" );
                    if ( _removedList == null ) {
                        _removedList = foundSource.antenna();
                    }
                    else {
                        _removedList += ", " + foundSource.antenna();
                    }
                }
            }
            //  Get the "root" name of the files associated with this job.
            String inputFile = _inputFileName.getText();
            String scanFileName = inputFile.substring( inputFile.lastIndexOf( '/' ) + 1, inputFile.lastIndexOf( '.' ) );
            //  Use this to find the "jobSeries" and "startSeries" values.
            v2dParser.jobSeries( scanFileName.substring( 0, scanFileName.indexOf( "_" ) ) );
            v2dParser.startSeries( Integer.valueOf( scanFileName.substring( scanFileName.indexOf( "_" ) + 1 ) ) );
            //  Use only the scan name(s) for this job in the .v2d file.
            String scanList = "";
            for ( int i = 0; i < _calcFileParser.numScans(); ++i ) {
                if ( i > 0 )
                    scanList += ", ";
                scanList += _calcFileParser.scans()[i].identifier;
            }
            v2dParser.ruleScan( "scansubset", scanList );
            //  See if the final "series" name is what we expect it to be.  If not, we'll have
            //  to move files around (see below).
            _newStartSeries = scanFileName.substring( scanFileName.indexOf( "_" ) + 1 );
            _oldStartSeries = v2dParser.startSeries().toString();
            _startSeriesMatch = false;
            if ( _oldStartSeries.contentEquals( _newStartSeries ) )
                _startSeriesMatch = true;
            v2dParser.headerComment( "#   -- Configuration file for vex2difx --\n"
                    + "#   This file was automatically generated by the DiFX GUI\n"
                    + "#   specifically to run a new session of the job " 
                    + v2dParser.jobSeries() + "_" + _newStartSeries + ".\n"
                    + "#   Created: " + new SimpleDateFormat( "MMMMM dd, yyyy hh:mm:ss z" ).format( new Date() ) + "\n"
                    + "#   By:      " + System.getProperty("user.name") + "\n\n" );                    
            final String newV2dName = _jobNode.passNode().fullPath() + "/" + v2dParser.jobSeries()
                    + "-" + _newStartSeries;
            final String newDataName = _jobNode.passNode().fullPath() + "/" + v2dParser.jobSeries()
                    + "_" + _newStartSeries;
            if ( !_startSeriesMatch )
                _oldDataName = _jobNode.passNode().fullPath() + "/" + v2dParser.jobSeries()
                    + "_" + _oldStartSeries;
            final String v2dContent = v2dParser.content();
            //  Move existing output files and any job-specific .v2d files to new names
            //  that have a "run index" stuck on the end of them.  The run index will be
            //  0 unless we've already gone through this process.  To see if we have, get
            //  a list of all .v2d files that are specific to this job (if there are any).
            //  Note that because this is a remote operation, it is done in a thread and
            //  the results may not occur immediately.
            final ArrayList<String> v2dList = new ArrayList<String>();
            DiFXCommand_ls ls = new DiFXCommand_ls( _jobNode.passNode().fullPath() + "/" + _newStartSeries
                    + "-" + _newStartSeries + ".v2d*", _settings );
            ls.addEndListener( new ActionListener() {
                public void actionPerformed( ActionEvent e ) {
                    //  This is called when the "ls" is done.  There should be a list of
                    //  all .v2d files associated with this job.  Possibly there are zero.
                    //  The purpose of all of this is to rename existing output files and
                    //  .v2d files (if there are any) to names with numeric extension indicating
                    //  the order in which they were run...first run will be .0, second .1, etc.
                    //  The most recent run will have no extension at all.
                    String runExtension = ".0";
                    if ( v2dList.size() > 0 ) {
                        int ext = 0;
                        for ( Iterator<String> iter = v2dList.iterator(); iter.hasNext(); ) {
                            String newStr = iter.next();
                            //  Find any integer appended to this - we need the largest one.
                            try {
                                Integer tryNum = Integer.parseInt( newStr.substring( newStr.lastIndexOf( "." ) + 1 ) );
                                if ( tryNum > ext )
                                    ext = tryNum;
                            } catch ( java.lang.NumberFormatException ex ) {}
                        }
                        //  Form a string out of the highest number we found.
                        runExtension = "." + ( ext + 1 );
                    }
                    //  Move existing output files and .v2d file to this new extension.
                    _messageDisplayPanel.message( 0, "Job Editor/Monitor", 
                            "Moving existing job files to have extension \"" + runExtension + "\"" );
                    DiFXCommand_mv mv1 = new DiFXCommand_mv( newV2dName + ".v2d", newV2dName + ".v2d" + runExtension, _settings );
                    try { mv1.send(); } catch ( Exception ex1 ) {}
                    DiFXCommand_mv mv2 = new DiFXCommand_mv( newDataName + ".calc", newDataName + ".calc" + runExtension, _settings );
                    try { mv2.send(); } catch ( Exception ex1 ) {}
                    DiFXCommand_mv mv3 = new DiFXCommand_mv( newDataName + ".difxlog", newDataName + ".difxlog" + runExtension, _settings );
                    try { mv3.send(); } catch ( Exception ex1 ) {}
                    DiFXCommand_mv mv4 = new DiFXCommand_mv( newDataName + ".flag", newDataName + ".flag" + runExtension, _settings );
                    try { mv4.send(); } catch ( Exception ex1 ) {}
                    DiFXCommand_mv mv5 = new DiFXCommand_mv( newDataName + ".im", newDataName + ".im" + runExtension, _settings );
                    try { mv5.send(); } catch ( Exception ex1 ) {}
                    DiFXCommand_mv mv6 = new DiFXCommand_mv( newDataName + ".difx", newDataName + ".difx" + runExtension, _settings );
                    try { mv6.send(); } catch ( Exception ex1 ) {}
                    DiFXCommand_mv mv7 = new DiFXCommand_mv( newDataName + ".input", newDataName + ".input" + runExtension, _settings );
                    try { mv7.send(); } catch ( Exception ex1 ) {}
                    DiFXCommand_mv mv8 = new DiFXCommand_mv( newDataName + ".machines", newDataName + ".machines" + runExtension, _settings );
                    try { mv8.send(); } catch ( Exception ex1 ) {}
                    DiFXCommand_mv mv9 = new DiFXCommand_mv( newDataName + ".threads", newDataName + ".threads" + runExtension, _settings );
                    try { mv9.send(); } catch ( Exception ex1 ) {}
                    //  Wait a moment to make sure that's done!
                    try { Thread.sleep( 100 ); } catch ( Exception ex1 ) {}
                    //  Make a new .v2d file specific to this job.
                    _messageDisplayPanel.message( 0, "Job Editor/Monitor", 
                            "Creating new .v2d file." + runExtension + "\"" );
                    SendFileMonitor sendFile = new SendFileMonitor(  frm, pt.x + 25, pt.y + 25,
                            newV2dName + ".v2d", v2dContent, _settings );
                    //  Run vex2difx and calcif2 on this new .v2d file.
                    _messageDisplayPanel.message( 0, "Job Editor/Monitor", "Running vex2difx and calcif2." );
                    DiFXCommand_vex2difx v2d = new DiFXCommand_vex2difx( _jobNode.passNode().fullPath(),
                            newV2dName.substring( newV2dName.lastIndexOf( "/" ) + 1 ) + ".v2d", _settings, false );
                    v2d.addEndListener( new ActionListener() {
                        public void actionPerformed( ActionEvent action ) {
                            _messageDisplayPanel.message( 0, "Job Editor/Monitor", "Vex2difx completed" );
                            //  Kludgey mess due to the way vex2difx handles a single scan...change the
                            //  names of files to match what they "should" be if, for instance, they started
                            //  with names like "Blah_01.*" and the new, truncated .v2d file produced
                            //  "Blah_1.*".  Trouble with this - running the .v2d file again will produce
                            //  different names (if you do it by hand).  Can't be helped.
                            if ( !_startSeriesMatch ) {
                                DiFXCommand_mv mv1 = new DiFXCommand_mv( _oldDataName + ".calc", newDataName + ".calc", _settings );
                                try { mv1.send(); } catch ( Exception ex1 ) {}
                                DiFXCommand_mv mv2 = new DiFXCommand_mv( _oldDataName + ".flag", newDataName + ".flag", _settings );
                                try { mv2.send(); } catch ( Exception ex1 ) {}
                                DiFXCommand_mv mv3 = new DiFXCommand_mv( _oldDataName + ".im", newDataName + ".im", _settings );
                                try { mv3.send(); } catch ( Exception ex1 ) {}
                                DiFXCommand_mv mv4 = new DiFXCommand_mv( _oldDataName + ".input", newDataName + ".input", _settings );
                                try { mv4.send(); } catch ( Exception ex1 ) {}
                                try { Thread.sleep( 2000 ); } catch( Exception e ) {}
                            }
                            //  Download .input and .calc files.
                            GetFileMonitor getFile = new GetFileMonitor( frm, pt.x + 25, pt.y + 25,
                                    _inputFileName.getText(), _settings, false );
                            if ( getFile.inString() != null && getFile.inString().length() > 0 ) {
                                _inputFileEditor.text( getFile.inString() );
//                                parseInputFile();
                            }
                            getFile = new GetFileMonitor(  frm, pt.x + 25, pt.y + 25,
                                    _calcFileName.getText(), _settings, false );
                            if ( getFile.inString() != null && getFile.inString().length() > 0 ) {
                                _calcFileEditor.text( getFile.inString() );
//                                parseCalcFile( _calcFileEditor.text() );
                            }
                            //  Now things get even sillier.  The new .input and .calc files have to be adjusted
                            //  if the file names were wrong.
                            if ( !_startSeriesMatch ) {
                                //  Grab the text of the .input file, locate any instance of the "wrong"
                                //  filename, and replace it with the "right" filename.
                                _inputFileEditor.text( _inputFileEditor.text().replaceAll( _oldDataName, newDataName ) );
                                _calcFileEditor.text( _calcFileEditor.text().replaceAll( _oldDataName, newDataName ) );
//                                parseInputFile();
                                try { Thread.sleep( 2000 ); } catch( Exception e ) {}
                                SendFileMonitor sendFile = new SendFileMonitor(  frm, pt.x + 25, pt.y + 25,
                                        _inputFileName.getText(), _inputFileEditor.text(), _settings );
                                //parseCalcFile( _calcFileEditor.text() );
                                SendFileMonitor sendFile2 = new SendFileMonitor(  frm, pt.x + 25, pt.y + 25,
                                        _calcFileName.getText(), _calcFileEditor.text(), _settings );
                            }
                            _messageDisplayPanel.message( 0, "Job Editor/Monitor", "Rebuild complete!" );
                            rebuildSuccess( true );
                        }
                    });
                    try {
                        v2d.send();
                        try { Thread.sleep( 8000 ); } catch ( Exception ex ) {}
                    } catch ( java.net.UnknownHostException ex ) {
                        _messageDisplayPanel.error( 0, "Job Editor/Monitor", "Error - DiFX host \"" + _settings.difxControlAddress()
                             + "\" unknown." );
                        rebuildFailed( true );
                    }
                }
            });
            ls.addIncrementalListener( new ActionListener() {
                public void actionPerformed( ActionEvent e ) {
                    v2dList.add( e.getActionCommand().trim() );
                }
            });
            try {
                ls.send();
            } catch ( java.net.UnknownHostException e ) {
                //  BLAT handle this
                rebuildFailed( true );
            }            
        }
        else {
            _messageDisplayPanel.error( 0, "Job Editor/Monitor", "No source .v2d file to work with (" +
                    _jobNode.passNode().fullPath() + "/" + _jobNode.passNode().v2dFileName() + ")" );
            rebuildFailed( true );
        }
    }
    
    /*
     * Select "default" settings for the data source and processor node selections.  These
     * selections are made based on the current state of affairs - i.e. nodes and threads that
     * are currently busy will not be used.  This information is used when creating the machines
     * and threads files - if that is done by hand (using the "Apply" button) then these default
     * selections will not be made.
     */
    public boolean selectNodeDefaults( boolean doAlways, boolean externalOperation ) {
        _nodeRestrictionFailure = false;
        //  Bail out of here if the user has set the machines already.  This is overridden
        //  if "doAlways" is true.
        if ( !doAlways && _machinesAppliedByHand == true ) 
            return true;
        //  These lists keep track of nodes we are using and how many threads we are using
        //  on each.  
        class UsedNode {
            public UsedNode( ProcessorNode newProcessorNode, int newThreads ) {
                processorNode = newProcessorNode;
                usedThreads = newThreads;
            }
            public ProcessorNode processorNode;
            public int usedThreads;
        };
        //  Set the data source nodes to reasonable defaults for each data source requirement.
        ArrayList<UsedNode> usedNodes = new ArrayList<UsedNode>();
        ArrayList<UsedNode> dataSourceNodes = new ArrayList<UsedNode>();
        for ( Iterator<BrowserNode> iter = _dataSourcesPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            DataSource thisSource = (DataSource)(iter.next());
            //  Currently we are only selecting the defaults for "FILE" data sources.
            if ( thisSource.sourceType().equalsIgnoreCase( "FILE" ) ) {
                FileSource fileSource = (FileSource)thisSource;
                //  Now we choose a node based on user instructions, and what is available.
                ProcessorNode foundNode = null;
                //  First...see if specific paths have been linked to specific nodes.  This
                //  will override any other considerations.
                if ( _settings.assignBasedOnPath() ) {
                    String nodeFromPath = _settings.nodeFromPath( fileSource.commonFilePath() );
                    if ( nodeFromPath != null ) {
                        for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
                                iter2.hasNext() && foundNode == null; ) {
                            ProcessorNode thisNode = (ProcessorNode)(iter2.next());
                            if ( thisNode.name().contentEquals( nodeFromPath ) )
                                foundNode = thisNode;
                        }
                    }
                }
                //  Look through the list of available nodes that can act as data sources
                //  and pick one that is appropriate.
                if ( foundNode != null )
                    _dataSourcesTested = true;
                else
                    _dataSourcesTested = false;
                for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
                        iter2.hasNext() && foundNode == null; ) {
                    _dataSourcesTested = true;
                    ProcessorNode thisNode = (ProcessorNode)(iter2.next());
                    //  We start by assuming we can use this node, then eliminate it based on rules set
                    //  by the user.
                    boolean useNode = true;
                    //  If we are not allowed to use the headnode as a data source, eliminate this node if
                    //  it is the headnode.
                    if ( !_settings.useHeadNodeCheck() && thisNode.name().contentEquals( _settings.headNode() ) )
                        useNode = false;
                    //  If we are restricted to a subset of the processor nodes, eliminate this item if
                    //  it is not among those allowed.
                    if ( _settings.restrictSources() && !_settings.sourceNodePermitted( thisNode.name() ) )
                        useNode = false;
                    //  If we are not allowing multiple data sources to share a node as a data source
                    //  within a job, eliminate this node if we are already using it for this job
                    //  (by consulting a locally-generated list).
                    if ( useNode && _settings.uniqueDataSource() ) {
                        for ( Iterator<UsedNode> iter3 = usedNodes.iterator(); iter3.hasNext() && useNode; ) {
                            UsedNode testNode = iter3.next();
                            if ( testNode.processorNode == thisNode )
                                useNode = false;
                        }
                    }
                    //  If we are not allowed to use this node when it is used as a data source for
                    //  other jobs, check that the node is not being used as a data source for another
                    //  job.
                    if ( useNode && !_settings.shareDataSourcesBetweenJobs() && thisNode.isDataSource() ) {
                        useNode = false;
                    }
                    //  If an independent process (someone else running DiFX) appears to be using this node,
                    //  and we are not stepping on what others are doing, don't use it.
                    if ( useNode && _settings.yieldToOtherSessions() && thisNode.activeJob() != null && thisNode.activeJob().trim() != "" ) {
                        //  Okay, processor is working on something...but it isn't one of our jobs, is it?
                        if ( !_settings.queueBrowser().isJobInSchedule( thisNode.activeJob() ) )
                            useNode = false;
                    }
                    //  Make sure the minimum number of threads required is avaible on this node.
                    if ( useNode ) {
                        int totalThreadsUsed = thisNode.threadsUsed();
                        //  We have to check the threads we are reserving locally, too.
                        for ( Iterator<UsedNode> iter3 = usedNodes.iterator(); iter3.hasNext() && useNode; ) {
                            UsedNode testNode = iter3.next();
                            if ( testNode.processorNode == thisNode )
                                totalThreadsUsed += testNode.usedThreads;
                        }
                        //  Subtract one more thread if this is the headnode.
                        if (  thisNode.name().contentEquals( _settings.headNode() ) )
                            ++totalThreadsUsed;
                        //  We assume one thread per core, always reserving one core for mpi activities.
                        if ( thisNode.numCores() - 1 <= totalThreadsUsed + _settings.threadsPerDataSource() )
                            useNode = false;
                    }
                    //  If we are allowed to use the node, do so.  We add it to our "local" list of used
                    //  nodes and set "foundNode" so the search is stopped.
                    if ( useNode ) {
                        foundNode = thisNode;
                    }
                }
                //  If a node has been chosen, set it.  If we haven't found a node, stick with whatever
                //  the initial default value was (i.e. don't change anything).
                if ( foundNode != null ) {
                    thisSource.setSourceNode( foundNode.name() );
                    //  Reserve thread(s) for reading based on user requests.
                    usedNodes.add( new UsedNode( foundNode, _settings.threadsPerDataSource() ) );
                    dataSourceNodes.add( new UsedNode( foundNode, _settings.threadsPerDataSource() ) );
                }
                else {
                    _nodeRestrictionFailure = true;
                    if ( externalOperation )
                        return false;
                    if ( _dataSourcesTested )
                        _jobNode.warningMessage( "Unable to select a node that meets data source requirements" );
                    else
                        _jobNode.errorMessage( "No nodes are available to act as data sources." );
                }
            }
        }
        //  Now set the processors.  The following is a list of processors we "intend"
        //  to use - we can't actually reserve the processors until we've decided what
        //  we want to do completely.
        ArrayList<UsedNode> processingNodes = new ArrayList<UsedNode>();
        boolean threadBasedFailure = false;
        boolean nodeBasedFailure = false;
        //  First determine a "process number" based on either the job
        //  (which is always 1, as this is 1 job), or baselines, depending on the user
        //  selection.
        int processNum = 1;
        if ( _settings.baselineCheck() ) {
            //  Based on baselines...
            processNum = processNum * _inputFile.baselineTable().num;
        }
        //  This is a quick test to indicate whether there are enough processors THEORETICALLY
        //  available (assuming they are idle, have threads, whatever) to do what is requested.
        //  This is different from not having the resources presently because they are busy -
        //  a failure here will ALWAYS fail.
        _processorsSufficient = false;
        if ( _settings.allNodesCheck() ) {
            if ( _settings.hardwareMonitor().processorNodes().children().size() > 0 )
                _processorsSufficient = true;
        }
        else if ( _settings.nodesPerCheck() ) {
             if ( _settings.hardwareMonitor().processorNodes().children().size() >= _settings.nodesPer() * processNum )
                _processorsSufficient = true;
        }
        else {
             int neededThreads = _settings.threadsPerNode();
             int numGoodNodes = 0;
             for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
                  iter2.hasNext(); ) {
                  ProcessorNode thisNode = (ProcessorNode)(iter2.next());
                  if ( thisNode.numCores() >= neededThreads )
                      ++numGoodNodes;
             }
             if ( numGoodNodes >= _settings.nodesPer() * processNum )
                 _processorsSufficient = true;
        }
        //  For each "process number" reserve a node/thread set, or simply a number
        //  of threads, depending on user preference.
        for ( int i = 0; i < processNum; ++i ) {
            //  We do this if all nodes are selected for each "process".
            if ( _settings.allNodesCheck() ) {
                ProcessorNode foundNode = null;
                //  Look at each available node and see if we can use it.
                for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
                        iter2.hasNext(); ) {
                    ProcessorNode thisNode = (ProcessorNode)(iter2.next());
                    //  Make sure this node is in the pane that displays processor nodes.
                    //  If for some reason it is not, we can't use it.
                    PaneProcessorNode thisPaneNode = processorNodeByName( thisNode.name() );
                    if ( thisPaneNode != null ) {
                        //  Eliminate the node if it is the head node.
                        if ( _settings.useHeadNodeCheck() || !thisNode.name().contentEquals( _settings.headNode() ) ) {
                            //  Eliminate this node if it does not fall into a "restricted" set of nodes.
                            boolean useThis = _settings.processorNotRestricted( thisNode.name() );
                            //  Eliminate this node if it is used as a data source and we aren't sharing.
                            if ( !_settings.shareDataSourcesAsProcessors() ) {
                                //  Is the node already used as a data source?
                                if ( thisNode.isDataSource() )
                                    useThis = false;
                                //  Is it "reserved" as a data source above?
                                for ( Iterator<UsedNode> iter3 = dataSourceNodes.iterator(); iter3.hasNext(); ) {
                                    if ( iter3.next().processorNode.name().contentEquals( thisNode.name() ) )
                                        useThis = false;
                                }
                            }
                            //  If an independent process (someone else running DiFX) appears to be using this node,
                            //  and we are not stepping on what others are doing, don't use it.
                            if ( useThis && _settings.yieldToOtherSessions() && thisNode.activeJob() != null && thisNode.activeJob().trim() != "" ) {
                                //  Okay, processor is working on something...but it isn't one of our jobs, is it?
                                if ( !_settings.queueBrowser().isJobInSchedule( thisNode.activeJob() ) )
                                    useThis = false;
                            }
                            //  See if the number of available threads is sufficient to match
                            //  what the user wants.
                            if ( useThis ) {
                                //  See how many threads this node has free.
                                int threadsFree = thisNode.numCores() - 1 - thisNode.threadsUsed();
                                //  Subtract one more thread if this is the headnode.
                                if (  thisNode.name().contentEquals( _settings.headNode() ) )
                                    --threadsFree;
                                //  Subtract any we have reserved for this job.
                                for ( Iterator<UsedNode> iter3 = usedNodes.iterator(); iter3.hasNext(); ) {
                                    UsedNode testNode = iter3.next();
                                    if ( testNode.processorNode == thisNode ) {
                                        threadsFree -= testNode.usedThreads;
                                    }
                                }
                                //  If the number of available threads meets requirements,
                                //  use the node!
                                if ( _settings.threadsPerCheck() && threadsFree >= _settings.threadsPerNode() ) {
                                    usedNodes.add( new UsedNode( thisNode, _settings.threadsPerNode() ) );
                                    processingNodes.add( new UsedNode( thisNode, _settings.threadsPerNode() ) );
                                    foundNode = thisNode;
                                }
                                else if ( _settings.allThreadsCheck() && threadsFree >= _settings.minThreadsPerNode() ) {
                                    usedNodes.add( new UsedNode( thisNode, threadsFree ) );
                                    processingNodes.add( new UsedNode( thisNode, threadsFree ) );
                                    foundNode = thisNode;
                                }
                            }
                        }
                    }
                }
                if ( foundNode == null ) {
                    _nodeRestrictionFailure = true;
                    nodeBasedFailure = true;
                }
            }
            //  We do this if the user wants a specific number of nodes devoted to each
            //  process.
            else if ( _settings.nodesPerCheck() ) {
                //  Check if there are enough nodes available to do this (busy or not)
                int availableHW = 0;
                for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
                        iter2.hasNext(); ) {
                    ProcessorNode thisNode = (ProcessorNode)(iter2.next());
                    //  Make sure this node is in the pane that displays processor nodes.
                    //  If for some reason it is not, we can't use it.
                    PaneProcessorNode thisPaneNode = processorNodeByName( thisNode.name() );
                    if ( thisPaneNode != null ) {
                        //  Eliminate the node if it is the head node.
                        if ( _settings.useHeadNodeCheck() || !thisNode.name().contentEquals( _settings.headNode() ) ) {
                            //  Eliminate this node if it does not fall into a "restricted" set of nodes.
                            boolean useThis = _settings.processorNotRestricted( thisNode.name() );
                            //  Eliminate this node if it is used as a data source and we aren't sharing.
                            if ( useThis && !_settings.shareDataSourcesAsProcessors() ) {
                                //  Is the node already used as a data source?
                                if ( thisNode.isDataSource() )
                                    useThis = false;
                                //  Is it "reserved" as a data source above?
                                for ( Iterator<UsedNode> iter3 = dataSourceNodes.iterator(); iter3.hasNext(); ) {
                                    if ( iter3.next().processorNode.name().contentEquals( thisNode.name() ) )
                                        useThis = false;
                                }
                            }
                            //  If an independent process (someone else running DiFX) appears to be using this node,
                            //  and we are not stepping on what others are doing, don't use it.
                            if ( useThis && _settings.yieldToOtherSessions() && thisNode.activeJob() != null && thisNode.activeJob().trim() != "" ) {
                                //  Okay, processor is working on something...but it isn't one of our jobs, is it?
                                if ( !_settings.queueBrowser().isJobInSchedule( thisNode.activeJob() ) )
                                    useThis = false;
                            }
                            if ( useThis )
                                ++availableHW;
                        }
                    }
                }
                //  So...how'd that go?  Is there enough hardware?  If so, follow user specifications.
                //  If not, use all processors available to get as close as possible to what
                //  the user wants.
                int nodesToUse = _settings.nodesPer();
                if ( availableHW == 0 ) {
                    _jobNode.errorMessage( "Zero processors available under current specifications!" );
                    nodesToUse = 0;
                }
                else if ( availableHW < nodesToUse ) {
                    nodesToUse = availableHW;
                    _jobNode.warningMessage( "Processor specification cannot be met!  Processing with available " + availableHW + " nodes." );
                }
                //  We want to reserve a specific number of nodes for each "process number".
                for ( int j = 0; j < nodesToUse; ++j ) {
                    ProcessorNode foundNode = null;
                    //  Look at each available node and see if we can use it (until we find one
                    //  that works).
                    for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
                            iter2.hasNext() && foundNode == null; ) {
                        ProcessorNode thisNode = (ProcessorNode)(iter2.next());
                        //  Make sure this node is in the pane that displays processor nodes.
                        //  If for some reason it is not, we can't use it.
                        PaneProcessorNode thisPaneNode = processorNodeByName( thisNode.name() );
                        if ( thisPaneNode != null ) {
                            //  Eliminate the node if it is the head node.
                            if ( _settings.useHeadNodeCheck() || !thisNode.name().contentEquals( _settings.headNode() ) ) {
                                //  Eliminate this node if it does not fall into a "restricted" set of nodes.
                                boolean useThis = _settings.processorNotRestricted( thisNode.name() );
                                //  Eliminate this node if it is used as a data source and we aren't sharing.
                                if ( !_settings.shareDataSourcesAsProcessors() ) {
                                    //  Is the node already used as a data source?
                                    if ( thisNode.isDataSource() )
                                        useThis = false;
                                    //  Is it "reserved" as a data source above?
                                    for ( Iterator<UsedNode> iter3 = dataSourceNodes.iterator(); iter3.hasNext(); ) {
                                        if ( iter3.next().processorNode.name().contentEquals( thisNode.name() ) )
                                            useThis = false;
                                    }
                                }
                                //  Make sure we haven't already used this node for processing (both
                                //  for other jobs and reserved for this one).
                                if ( useThis ) {
                                    if ( thisNode.isProcessor() )
                                        useThis = false;
                                    for ( Iterator<UsedNode> iter3 = processingNodes.iterator(); iter3.hasNext(); ) {
                                        if ( iter3.next().processorNode.name().contentEquals( thisNode.name() ) )
                                            useThis = false;
                                    }
                                }
                                //  If an independent process (someone else running DiFX) appears to be using this node,
                                //  and we are not stepping on what others are doing, don't use it.
                                if ( useThis && _settings.yieldToOtherSessions() && thisNode.activeJob() != null && thisNode.activeJob().trim() != "" ) {
                                    //  Okay, processor is working on something...but it isn't one of our jobs, is it?
                                    if ( !_settings.queueBrowser().isJobInSchedule( thisNode.activeJob() ) )
                                        useThis = false;
                                }
                                //  See if the number of available threads is sufficient to match
                                //  what the user wants.
                                if ( useThis ) {
                                    //  See how many threads this node has free.
                                    int threadsFree = thisNode.numCores() - 1 - thisNode.threadsUsed();
                                    //  Subtract one more thread if this is the headnode.
                                    if (  thisNode.name().contentEquals( _settings.headNode() ) )
                                        --threadsFree;
                                    //  Subtract any we have reserved for this job.
                                    for ( Iterator<UsedNode> iter3 = usedNodes.iterator(); iter3.hasNext(); ) {
                                        UsedNode testNode = iter3.next();
                                        if ( testNode.processorNode == thisNode )
                                            threadsFree -= testNode.usedThreads;
                                    }
                                    //  If the number of available threads meets requirements,
                                    //  use the node!
                                    if ( _settings.threadsPerCheck() && threadsFree >= _settings.threadsPerNode() ) {
                                        usedNodes.add( new UsedNode( thisNode, _settings.threadsPerNode() ) );
                                        processingNodes.add( new UsedNode( thisNode, _settings.threadsPerNode() ) );
                                        foundNode = thisNode;
                                    }
                                    else if ( _settings.allThreadsCheck() && threadsFree >= _settings.minThreadsPerNode() ) {
                                        usedNodes.add( new UsedNode( thisNode, threadsFree ) );
                                        processingNodes.add( new UsedNode( thisNode, threadsFree ) );
                                        foundNode = thisNode;
                                    }
                                }
                            }
                        }
                    }
                    if ( foundNode == null ) {
                        _nodeRestrictionFailure = true;
                        nodeBasedFailure = true;
                    }
                }
            }
            //  This is done if each process is assigned a specific number of threads.
            //  These can be split between processors.
            else {
                //  We need this number of threads total.
                int neededThreads = _settings.threadsPerNode();
                //  Go through each available node and see if we can use some threads.
                for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
                        iter2.hasNext(); ) {
                    ProcessorNode thisNode = (ProcessorNode)(iter2.next());
                    //  Make sure this node is in the pane that displays processor nodes.
                    //  If for some reason it is not, we can't use it.
                    PaneProcessorNode thisPaneNode = processorNodeByName( thisNode.name() );
                    if ( thisPaneNode != null ) {
                        //  Eliminate the node if it is the head node.
                        if ( _settings.useHeadNodeCheck() || !thisNode.name().contentEquals( _settings.headNode() ) ) {
                            //  Eliminate this node if it does not fall into a "restricted" set of nodes.
                            boolean useThis = _settings.processorNotRestricted( thisNode.name() );
                            //  Eliminate this node if it is used as a data source and we aren't sharing.
                            if ( !_settings.shareDataSourcesAsProcessors() ) {
                                //  Is the node already used as a data source?
                                if ( thisNode.isDataSource() )
                                    useThis = false;
                                //  Is it "reserved" as a data source above?
                                for ( Iterator<UsedNode> iter3 = dataSourceNodes.iterator(); iter3.hasNext(); ) {
                                    if ( iter3.next().processorNode.name().contentEquals( thisNode.name() ) )
                                        useThis = false;
                                }
                            }
                            //  If an independent process (someone else running DiFX) appears to be using this node,
                            //  and we are not stepping on what others are doing, don't use it.
                            if ( useThis && _settings.yieldToOtherSessions() && thisNode.activeJob() != null && thisNode.activeJob().trim() != "" ) {
                                //  Okay, processor is working on something...but it isn't one of our jobs, is it?
                                if ( !_settings.queueBrowser().isJobInSchedule( thisNode.activeJob() ) )
                                    useThis = false;
                            }
                            if ( useThis ) {
                                //  See how many threads this node has free.
                                int threadsFree = thisNode.numCores() - 1 - thisNode.threadsUsed();
                                //  Subtract one more thread if this is the headnode.
                                if (  thisNode.name().contentEquals( _settings.headNode() ) )
                                    --threadsFree;
                                //  Subtract any we have reserved for this job.
                                for ( Iterator<UsedNode> iter3 = usedNodes.iterator(); iter3.hasNext(); ) {
                                    UsedNode testNode = iter3.next();
                                    if ( testNode.processorNode == thisNode )
                                        threadsFree -= testNode.usedThreads;
                                }
                                //  If there are any available, reserve as many as we can/need to for this job.
                                if ( threadsFree > 0 && neededThreads > 0 ) {
                                    int reserveThreads = neededThreads;
                                    if ( threadsFree < neededThreads )
                                        reserveThreads = threadsFree;
                                    usedNodes.add( new UsedNode( thisNode, reserveThreads ) );
                                    processingNodes.add( new UsedNode( thisNode, reserveThreads ) );
                                    neededThreads -= reserveThreads;
                                }
                            }
                        }
                    }
                }
                if ( neededThreads > 0 ) {
                    _nodeRestrictionFailure = true;
                    threadBasedFailure = true;
                }
            }
        }
        //  Now actually reserve the processors.  We do this first by clearing all
        //  currently-selected processors.
        deselectAllProcessors();
        zeroAllProcessorThreads();
        //  Now run through our list of "intentions" and actually reserve the
        //  processors.
        for ( Iterator<UsedNode> iter = processingNodes.iterator(); iter.hasNext(); ) {
            UsedNode usedNode = (UsedNode)iter.next();
            //  Locate this node in the processor pane and reserve it (if necessary).
            //  Add the number of threads we need for this "intention".
            PaneProcessorNode thisNode = processorNodeByName( usedNode.processorNode.name() );
            if ( thisNode != null ) {  //  shouldn't happen!
                if ( thisNode.selected() )
                    thisNode.threads( thisNode.threads() + usedNode.usedThreads );
                else {
                    thisNode.selected( true );
                    thisNode.threads( usedNode.usedThreads );
                }
            }
        }
        //  Warn the user if the available resources are insufficient to process the
        //  job as requested.  This is interpeted as an error if there simply are not enough
        //  hardware resources to do the trick even if everything is idle.
        if ( _processorsSufficient ) {
            if ( threadBasedFailure ) {
                if ( externalOperation )
                    return false;
                _jobNode.warningMessage( "Not enough free threads to meet processing requirements." );
            }
            if ( nodeBasedFailure ) {
                if ( externalOperation )
                    return false;
                _jobNode.warningMessage( "Not enough free nodes to meet processing requirements." );
            }
        }
        else {
            if ( externalOperation )
                return false;
            _jobNode.errorMessage( "Not enough nodes to EVER meet processing requirements." );
        }
        return true;
    }
    
    protected boolean _nodeRestrictionFailure;
    public boolean nodeRestrictionFailure() { return _nodeRestrictionFailure; }
    protected boolean _dataSourcesTested;
    public boolean dataSourcesTested() { return _dataSourcesTested; }
    protected boolean _processorsSufficient;
    public boolean processorsSufficient() { return _processorsSufficient; }

    protected int _threadsInUse;
    public int threadsInUse() { return _threadsInUse; }
    
    /*
     * Send the current machines and thread settings to guiServer to produce .machines
     * and .threads files.
     */
    public MachinesDefinitionMonitor applyMachinesAction() {
        DiFXCommand command = new DiFXCommand( _settings );
        command.header().setType( "DifxMachinesDefinition" );
        command.mpiProcessId( "-1" );
        command.identifier( _jobNode.name() );
        int monitorPort = 0;

        //  We are going to test processor nodes, so remove the results of any previous
        //  tests.
        for ( Iterator<BrowserNode> iter = _processorsPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            ( (PaneProcessorNode)(iter.next()) ).clearTest();
        }
        
        // Create machines definition command
        DifxMachinesDefinition cmd = command.factory().createDifxMachinesDefinition();
        cmd.setInput( _jobNode.inputFile() );
        cmd.setDifxVersion( _settings.difxVersion() );

        // If we are using the TCP connection, set the address and port for diagnostic
        // reporting.
        if ( _settings.sendCommandsViaTCP() ) {
            cmd.setAddress( _settings.guiServerConnection().myIPAddress() );
            monitorPort = _settings.newDifxTransferPort( 0, 100, true, true );
            cmd.setPort( monitorPort );
        }
        
        // -- manager, enabled only
        DifxMachinesDefinition.Manager manager = command.factory().createDifxMachinesDefinitionManager();
        manager.setNode( _headNode.getText() );
        cmd.setManager( manager );

        // -- datastreams, enabled only
        DifxMachinesDefinition.Datastream dataStream = command.factory().createDifxMachinesDefinitionDatastream();

        //  Include all of the "checked" data stream node names...
        String dataNodeNames = "";
        for ( Iterator<BrowserNode> iter = _dataSourcesPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            DataSource thisSource = (DataSource)(iter.next());
            dataNodeNames += thisSource.sourceNode() + " ";
        }
        dataStream.setNodes( dataNodeNames );
        cmd.setDatastream(dataStream);

        // Add enabled processors and threads.  Don't include processors that have no
        // threads!  Also avoid the headnode if the user has indicated it should be
        // avoided (recommended if there are other processors).
        _threadsInUse = 0;
        for ( Iterator<BrowserNode> iter = _processorsPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            PaneProcessorNode thisNode = (PaneProcessorNode)(iter.next());
            if ( thisNode.selected() ) {
                //  Avoid the headnode - if there are other nodes
                if ( _restrictHeadnodeProcessing.isSelected() && _headNode.getText().contentEquals( thisNode.name() ) &&
                        _processorsPane.browserTopNode().children().size() > 1 ) {}
                else {
                    DifxMachinesDefinition.Process process = command.factory().createDifxMachinesDefinitionProcess();
                    process.setNodes( thisNode.name() );
                    process.setThreads( thisNode.threadsText() );
                    _threadsInUse += thisNode.threads();
                    cmd.getProcess().add( process );
                }
            }
        }
        
        //  Test the processors if the user wants to (this will generate feedback that
        //  will be reflected in the processors list).
        if ( _eliminateNonrespondingProcessors.isSelected() )
            cmd.setTestProcessors( true );
        else
            cmd.setTestProcessors( false );
        
        //  Send the names for the machine and thread files.  For the moment we are basing
        //  these on the input file name.
        String fileStr = _inputFileName.getText();
        cmd.setMachinesFile( fileStr.substring( 0, fileStr.lastIndexOf( '.' ) + 1 ).trim() + "machines" );
        cmd.setThreadsFile( fileStr.substring( 0, fileStr.lastIndexOf( '.' ) + 1 ).trim() + "threads" );

        //  Set up a monitor thread to collect and interpret diagnostic messages from
        //  guiServer as it sets up the threads and machine files.
        MachinesDefinitionMonitor monitor = null;
        if ( _settings.sendCommandsViaTCP() ) {
            monitor = new MachinesDefinitionMonitor( monitorPort );
            monitor.start();
        }
        
        statusInfo( "Using criteria to create .machines and .threads files." );

        // -- Create the XML defined messages and process through the system
        command.body().setDifxMachinesDefinition( cmd );
        try {
            command.send();
        } catch ( java.net.UnknownHostException e ) {
            java.util.logging.Logger.getLogger("global").log(java.util.logging.Level.SEVERE, null,
                    e.getMessage() );  //BLAT should be a pop-up
        }    
        return monitor;
    }
    
    /*
     * This thread opens and monitors a TCP socket for diagnostic reports from the
     * guiServer as it sets up thread and machine files.  The opposite side of this
     * communication link is in the file "guiServer/src/machinesDefinition.cpp" in
     * the DiFX source tree.
     */
    protected class MachinesDefinitionMonitor extends Thread {
        
        public MachinesDefinitionMonitor( int port ) {
            _port = port;
        }
        
        /*
         * These packet types are sent by the "JobMonitorConnection" class in the
         * guiServer application on the DiFX host.
         */
        protected final int MACHINE_DEF_TASK_TERMINATED                     = 100;
        protected final int MACHINE_DEF_TASK_ENDED_GRACEFULLY               = 101;
        protected final int MACHINE_DEF_TASK_STARTED                        = 102;
        protected final int MACHINE_DEF_PARAMETER_CHECK_IN_PROGRESS         = 103;
        protected final int MACHINE_DEF_PARAMETER_CHECK_SUCCESS             = 104;
        protected final int MACHINE_DEF_FAILURE_NO_HEADNODE                 = 105;
        protected final int MACHINE_DEF_FAILURE_NO_DATASOURCES              = 106;
        protected final int MACHINE_DEF_FAILURE_NO_PROCESSORS               = 107;
        protected final int MACHINE_DEF_WARNING_NO_MACHINES_FILE_SPECIFIED  = 108;
        protected final int MACHINE_DEF_WARNING_NO_THREADS_FILE_SPECIFIED   = 109;
        protected final int MACHINE_DEF_THREADS_FILE_NAME                   = 110;
        protected final int MACHINE_DEF_MACHINES_FILE_NAME                  = 111;
        protected final int MACHINE_DEF_FAILURE_NO_FILES_SPECIFIED          = 112;
        protected final int MACHINE_DEF_FAILURE_OPEN_MACHINES_FILE          = 113;
        protected final int MACHINE_DEF_FAILURE_OPEN_THREADS_FILE           = 114;
        protected final int MACHINE_DEF_MACHINES_FILE_CREATED               = 115;
        protected final int MACHINE_DEF_THREADS_FILE_CREATED                = 116;
        protected final int MACHINE_DEF_FAILURE_FILE_REMOVAL                = 117;
        protected final int MACHINE_DEF_FAILURE_POPEN                       = 118;
        protected final int MACHINE_DEF_FAILURE_MPIRUN                      = 119;
        protected final int MACHINE_DEF_SUCCESS_MPIRUN                      = 120;
        protected final int MACHINE_DEF_LOW_THREAD_COUNT                    = 121;
        protected final int MACHINE_DEF_RUNNING_MPIRUN_TESTS                = 122;
        
        @Override
        public void run() {
            //  Open a new server socket and await a connection.  The connection
            //  will timeout after a given number of seconds (nominally 10).
            try {
                ChannelServerSocket ssock = new ChannelServerSocket( _port, _settings );
                ssock.setSoTimeout( 10000 );  //  timeout is in millisec
                try {
                    ssock.accept();
                    //  Loop collecting diagnostic packets from the guiServer.  These
                    //  are identified by an initial integer, and then are followed
                    //  by a data length, then data.
                    boolean connected = true;
                    int workState = 0;
                    while ( connected ) {
                        //  Read the packet type as an integer.  The packet types
                        //  are defined above (within this class).
                        int packetType = ssock.readInt();
                        //  Read the size of the incoming data (bytes).
                        int packetSize = ssock.readInt();
                        //  Read the data (as raw bytes)
                        byte [] data = null;
                        if ( packetSize > 0 ) {
                            data = new byte[packetSize];
                            ssock.readFully( data, 0, packetSize );
                        }
                        //  Interpret the packet type.
                        if ( packetType == MACHINE_DEF_TASK_TERMINATED ) {
                            _messageDisplayPanel.warning( 0, "machines monitor", "Task terminated prematurely." );
                            connected = false;
                        }
                        else if ( packetType == MACHINE_DEF_TASK_ENDED_GRACEFULLY ) {
                            _messageDisplayPanel.warning( 0, "machines monitor", "Task finished gracefully." );
                            statusInfo( ".machines and .threads files created." );
                            connected = false;
                        }
                        else if ( packetType == MACHINE_DEF_TASK_STARTED ) {
                            _messageDisplayPanel.message( 0, "machines monitor", "Task started by guiServer." );
                        }
                        else if ( packetType == MACHINE_DEF_PARAMETER_CHECK_IN_PROGRESS ) {
                            _messageDisplayPanel.message( 0, "machines monitor", "Checking parameters." );
                        }
                        else if ( packetType == MACHINE_DEF_PARAMETER_CHECK_SUCCESS ) {
                            _messageDisplayPanel.message( 0, "machines monitor", "Parameter check successful." );
                        }
                        else if ( packetType == MACHINE_DEF_FAILURE_NO_HEADNODE ) {
                            _messageDisplayPanel.error( 0, "machines monitor", "No headnone was specified." );
                            statusError( "Headnode needs to be specified to create .machines and .threads files." );
                        }
                        else if ( packetType == MACHINE_DEF_FAILURE_NO_DATASOURCES ) {
                            _messageDisplayPanel.error( 0, "machines monitor", "No valid data streams were specified." );
                            statusError( "No valid data streams were specified - could not create .mahcines and .threads files." );
                        }
                        else if ( packetType == MACHINE_DEF_FAILURE_NO_PROCESSORS ) {
                            _messageDisplayPanel.error( 0, "machines monitor", "No valid processors were specified." );
                            statusError( "No valid processors were specified - could not create .mahcines and .threads files." );
                        }
                        else if ( packetType == MACHINE_DEF_WARNING_NO_MACHINES_FILE_SPECIFIED ) {
                            workState = packetType;
                            _messageDisplayPanel.message( 0, "machines monitor", "No machines file name was specified - forming one using input file name." );
                        }
                        else if ( packetType == MACHINE_DEF_MACHINES_FILE_NAME ) {
                            _machinesFileName.setText( new String( data ) );
                            _messageDisplayPanel.message( 0, "machines monitor", "Creating machines file \"" + _machinesFileName.getText() + "\"" );
                            statusInfo( "creating \"" + _machinesFileName.getText() + "\"" );
                        }
                        else if ( packetType == MACHINE_DEF_THREADS_FILE_NAME ) {
                            _threadsFileName.setText( new String( data ) );
                            _messageDisplayPanel.message( 0, "machines monitor", "Creating threads file \"" + _threadsFileName.getText() + "\"" );
                            statusInfo( "creating \"" + _threadsFileName.getText() + "\"" );
                        }
                        else if ( packetType == MACHINE_DEF_WARNING_NO_THREADS_FILE_SPECIFIED ) {
                            workState = packetType;
                            _messageDisplayPanel.message( 0, "machines monitor", "No threads file name was specified - forming one using input file name." );
                        }
                        else if ( packetType == MACHINE_DEF_FAILURE_NO_FILES_SPECIFIED ) {
                            if ( workState == MACHINE_DEF_WARNING_NO_MACHINES_FILE_SPECIFIED )
                                _messageDisplayPanel.message( 0, "machines monitor", "No input file name specified - unable to form machines file name." );
                            else if ( workState == MACHINE_DEF_WARNING_NO_THREADS_FILE_SPECIFIED )
                                _messageDisplayPanel.message( 0, "machines monitor", "No input file name specified - unable to form threads file name." );
                            else
                                _messageDisplayPanel.message( 0, "machines monitor", "Unknown error involving missing file names." );
                        }
                        else if ( packetType == MACHINE_DEF_FAILURE_OPEN_MACHINES_FILE ) {
                            _machinesFileName.setText( new String( data ) );
                            _messageDisplayPanel.message( 0, "machines monitor", "Failure to open machines file (" + new String( data ) + ")" );
                            statusError( "could not open machines file \"" + new String( data ) + "\"" );
                        }
                        else if ( packetType == MACHINE_DEF_FAILURE_OPEN_THREADS_FILE ) {
                            _machinesFileName.setText( new String( data ) );
                            _messageDisplayPanel.message( 0, "machines monitor", "Failure to open threads file (" + new String( data ) + ")" );
                            statusError( "could not open threads file \"" + new String( data ) + "\"" );
                        }
                        else if ( packetType == MACHINE_DEF_MACHINES_FILE_CREATED ) {
                            _messageDisplayPanel.message( 0, "machines monitor", "machines file created" );
                            //  Download the machines file to its editor.
                            Component comp = _applyMachinesButton;
                            while ( comp.getParent() != null )
                                comp = comp.getParent();
                            Point pt = null;
                            try {
                                pt = _applyMachinesButton.getLocationOnScreen();
                            }
                            catch ( java.awt.IllegalComponentStateException e ) {
                                pt = new Point( 100, 100 );
                            }
                            GetFileMonitor getFile = new GetFileMonitor(  (Frame)comp, pt.x + 25, pt.y + 25,
                                    _machinesFileName.getText(), _settings, false );
                            if ( getFile.inString() != null && getFile.inString().length() > 0 ) {
                                _machinesFileEditor.text( getFile.inString() );
                            }
                            _messageDisplayPanel.message( 0, "machines monitor", "machines file successfully downloaded" );
                            statusInfo( ".machines file created" );
                        }
                        else if ( packetType == MACHINE_DEF_THREADS_FILE_CREATED ) {
                            _messageDisplayPanel.message( 0, "machines monitor", "threads file created" );
                            //  Download the threads file to its editor.
                            Component comp = _applyMachinesButton;
                            while ( comp.getParent() != null )
                                comp = comp.getParent();
                            Point pt = null;
                            try {
                                pt = _applyMachinesButton.getLocationOnScreen();
                            }
                            catch ( java.awt.IllegalComponentStateException e ) {
                                pt = new Point( 100, 100 );
                            }
                            GetFileMonitor getFile = new GetFileMonitor(  (Frame)comp, pt.x + 25, pt.y + 25,
                                    _threadsFileName.getText(), _settings, false );
                            if ( getFile.inString() != null && getFile.inString().length() > 0 ) {
                                _threadsFileEditor.text( getFile.inString() );
                            }
                            _messageDisplayPanel.message( 0, "machines monitor", "threads file successfully downloaded" );
                            statusInfo( ".threads file created" );
                        }
                        else if ( packetType == MACHINE_DEF_FAILURE_FILE_REMOVAL ) {
                            _messageDisplayPanel.error( 0, "machines monitor", "Failed to remove file on DiFX host: " + new String( data ) );
                            statusError( "permissions prevent removal of a file on DiFX host" );
                        }
                        else if ( packetType == MACHINE_DEF_FAILURE_POPEN ) {
                            _messageDisplayPanel.error( 0, "machines monitor", "Popen failed on DiFX host: " + new String( data ) );
                        }
                        else if ( packetType == MACHINE_DEF_FAILURE_MPIRUN ) {
                            PaneProcessorNode node = processorNodeByName( new String( data ) );
                            if ( node != null )
                                node.mpiTest( false, _eliminateNonrespondingProcessors.isSelected() );
                            if ( _eliminateNonrespondingProcessors.isSelected() )
                                _messageDisplayPanel.warning( 0, "machines monitor", "Processing node " + new String( data ) + " failed mpirun test - it will not be used" );
                            else
                                _messageDisplayPanel.warning( 0, "machines monitor", "Processing node " + new String( data ) + " failed mpirun test" );
                        }
                        else if ( packetType == MACHINE_DEF_SUCCESS_MPIRUN ) {
                            PaneProcessorNode node = processorNodeByName( new String( data ) );
                            if ( node != null )
                                node.mpiTest( true, true );
                        }
                        else if ( packetType == MACHINE_DEF_LOW_THREAD_COUNT ) {
                            _messageDisplayPanel.error( 0, "machines monitor", "Number of processing threads is zero" );
                        }
                        else if ( packetType == MACHINE_DEF_RUNNING_MPIRUN_TESTS ) {
                            _messageDisplayPanel.message( 0, "machines monitor", "Running mpirun tests." );
                            statusInfo( "running mpirun tests" );
                        }
                        else {
                            _messageDisplayPanel.warning( 0, "GUI", "Ignoring unrecongized job monitor packet type (" + packetType + ")." );
                        }
                    }
                } catch ( SocketTimeoutException e ) {
                }
                ssock.close();
            } catch ( java.io.IOException e ) {
                e.printStackTrace();
            }
            _settings.releaseTransferPort( _port );
        }
        
        protected int _port;
        
    }
    
    public void applyToAction() {
        _machinesApplyPopup.show( _applyToButton, 25, 25 );
    }
    
    /*
     * Called with the settings governing whether to check for "busy" processors
     * or how to define them are changed.
     */
    public void changeBusyProcessorsSettings() {
        for ( Iterator<BrowserNode> iter = _processorsPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            PaneProcessorNode thisNode = (PaneProcessorNode)(iter.next());
            boolean keepIt = thisNode.cpuTest( (float)_busyPercentage.value(),
                    _eliminateBusyProcessors.isSelected()  );
        }
    }
    
    public Iterator<BrowserNode> dataSourcesIterator() {
        return _dataSourcesPane.browserTopNode().children().iterator();
    }
    
    public Iterator<BrowserNode> processorsIterator() {
        return _processorsPane.browserTopNode().children().iterator();
    }
    
    public String headNode() { return _headNode.getText(); }
    public void headNode( String newVal ) { _headNode.setText( newVal ); }
    
    /*
     * Simple class to run and monitor the machines files generation routine in
     * a thread.
     */
    protected class ApplyMachinesThenStart extends Thread {
        public boolean configTestOnly;
        @Override
        public void run() {
            MachinesDefinitionMonitor machines = applyMachinesAction();
            while ( machines.isAlive() )
                try { Thread.sleep( 100 ); } catch ( Exception e ) {}
            _machinesAppliedByHand = true;
            startJob( true, configTestOnly );
        }
    }
    
    /*
     * This function runs the job based on all current settings.  Jobs can be
     * run using a TCP connection to guiServer, or through mk5daemon via UDP.  If
     * using the former, a thread is started to monitor the job progress via a
     * dedicated socket and report any errors.  In the latter case the job start
     * instruction is more of a "set and forget" kind of operation.
     */
    public void startJob( boolean applyMachinesInThread, boolean configTestOnly ) {
        _jobNode.setJobStart();
        _jobNode.setState( "Node Allocation", Color.YELLOW );
        //  "Reserve" the resources we need for this process - we tell the nodes themselves
        //  that they are "busy".  Headnode first.
        for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
                iter2.hasNext(); ) {
            ProcessorNode usedNode = (ProcessorNode)(iter2.next());
            if ( usedNode.name().contentEquals( _headNode.getText() ) )
                usedNode.addJob( this, 1, ProcessorNode.CurrentUse.HEADNODE );
        }
        //  Then all of the "checked" data stream node names.
        for ( Iterator<BrowserNode> iter = _dataSourcesPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            DataSource thisNode = (DataSource)(iter.next());
            //  Tell this node it is being used as a data source, and add it to the list
            //  of nodes employed by this job.
            for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
                    iter2.hasNext(); ) {
                ProcessorNode usedNode = (ProcessorNode)(iter2.next());
                if ( usedNode.name().contentEquals( thisNode.sourceNode() ) )
                    usedNode.addJob( this, _settings.threadsPerDataSource(), ProcessorNode.CurrentUse.DATASOURCE );
            }
        }
        //  And the processing nodes.
        for ( Iterator<BrowserNode> iter = _processorsPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            PaneProcessorNode thisNode = (PaneProcessorNode)(iter.next());
            if ( thisNode.selected() ) {
                for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
                        iter2.hasNext(); ) {
                    ProcessorNode usedNode = (ProcessorNode)(iter2.next());
                    if ( usedNode.name().contentEquals( thisNode.name() ) )
                        usedNode.addJob( this, thisNode.threads(), ProcessorNode.CurrentUse.PROCESSOR );
                }
            }
        }
        
        //  Has the user already generated .threads and .machines files (which is
        //  done when the "Apply" button in the Machines List settings is pushed)?
        //  Alternatively, has the user edited and uploaded .machines and .threads
        //  files by hand?  If these things have not been done, the files need to
        //  be generated before running.  The thread we create will do this and then
        //  restart the job.  We also have the option of NOT doing this in a thread,
        //  which is used by the scheduler.
        if ( !_machinesAppliedByHand ) {
            if ( applyMachinesInThread ) {
                ApplyMachinesThenStart applyMachinesThread = new ApplyMachinesThenStart();
                applyMachinesThread.configTestOnly = configTestOnly;
                applyMachinesThread.start();
                return;
            }
            else {
                MachinesDefinitionMonitor machines = applyMachinesAction();
                while ( machines.isAlive() )
                    try { Thread.sleep( 100 ); } catch ( Exception e ) {}
                _machinesAppliedByHand = true;
            }
        }
        //_jobNode.estimateJobTime();
        DiFXCommand command = new DiFXCommand( _settings );
        command.header().setType( "DifxStart" );
        command.mpiProcessId( "-1" );
        command.identifier( _jobNode.name() );
        int monitorPort = 0;

        // Create start job command
        DifxStart jobStart = command.factory().createDifxStart();
        jobStart.setInput( _jobNode.inputFile() );

        // If we are using the TCP connection, set the address and port for diagnostic
        // reporting.
        if ( _settings.sendCommandsViaTCP() ) {
            jobStart.setAddress( _settings.guiServerConnection().myIPAddress() );
            monitorPort = _settings.newDifxTransferPort( 0, 100, true, true );
            jobStart.setPort( monitorPort );
        }
        
        if ( _runMonitor.isSelected() )
            jobStart.setFunction( "RUN_MONITOR" );
        else if ( configTestOnly )
            jobStart.setFunction( "CONFIG_ONLY" );
        else if ( _settings.stopWhenConfigFails() )
            jobStart.setFunction( "BAIL_ON_CONFIG_FAIL" );
        else
            jobStart.setFunction( "USNO" ); 

        // -- manager, enabled only
        DifxStart.Manager manager = command.factory().createDifxStartManager();
        manager.setNode( _headNode.getText() );
//        //  Tell this node it is being used as a head node, and add it to the list
//        //  of nodes employed by this job.
//        for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
//                iter2.hasNext(); ) {
//            ProcessorNode usedNode = (ProcessorNode)(iter2.next());
//            if ( usedNode.name().contentEquals( _headNode.getText() ) )
//                usedNode.addJob( this, 1, ProcessorNode.CurrentUse.HEADNODE );
//        }
        jobStart.setManager( manager );

        // -- set difx version to use
        jobStart.setDifxVersion( _settings.difxVersion() );
        
        //  Set the "restart" time in seconds from the job start, if this has been
        //  requested.
        if ( _restartAt.isSelected() )
            jobStart.setRestartSeconds( _restartSeconds.value() );

        // -- datastreams, enabled only
        DifxStart.Datastream dataStream = command.factory().createDifxStartDatastream();

        //  Include all of the "checked" data stream node names.
        String dataNodeNames = "";
        for ( Iterator<BrowserNode> iter = _dataSourcesPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            DataSource thisNode = (DataSource)(iter.next());
            dataNodeNames += thisNode.sourceNode() + " ";
//            //  Tell this node it is being used as a data source, and add it to the list
//            //  of nodes employed by this job.
//            for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
//                    iter2.hasNext(); ) {
//                ProcessorNode usedNode = (ProcessorNode)(iter2.next());
//                if ( usedNode.name().contentEquals( thisNode.sourceNode() ) )
//                    usedNode.addJob( this, _settings.threadsPerDataSource(), ProcessorNode.CurrentUse.DATASOURCE );
//            }
        }
        dataStream.setNodes( dataNodeNames );
        jobStart.setDatastream(dataStream);

        // Add enabled processors and threads.  Don't include processors that have no
        // threads!
        for ( Iterator<BrowserNode> iter = _processorsPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            PaneProcessorNode thisNode = (PaneProcessorNode)(iter.next());
            if ( thisNode.selected() ) {
                DifxStart.Process process = command.factory().createDifxStartProcess();
                process.setNodes( thisNode.name() );
                process.setThreads( thisNode.threadsText() );
                jobStart.getProcess().add( process );
//                //  Tell this node it is being used as a processor, and add it to the list
//                //  of nodes employed by this job.
//                for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
//                        iter2.hasNext(); ) {
//                    ProcessorNode usedNode = (ProcessorNode)(iter2.next());
//                    if ( usedNode.name().contentEquals( thisNode.name() ) )
//                        usedNode.addJob( this, thisNode.threads(), ProcessorNode.CurrentUse.PROCESSOR );
//                }
            }
        }

        // force deletion of existing output file if this box has been checked.
        if ( _forceOverwrite.isSelected() )
            jobStart.setForce( 1 );
        else
            jobStart.setForce( 0 ); 
        
        //  Set up a monitor thread if this job is being run using guiServer.  This
        //  thread will collect and interpret diagnostic messages directly from
        //  guiServer as it sets up and runs the job.
        boolean cleanStart = true;
        if ( _settings.sendCommandsViaTCP() ) {
            RunningJobMonitor runMonitor = new RunningJobMonitor( monitorPort );
            cleanStart = runMonitor.socketInit();
            if ( cleanStart )
                runMonitor.start();
        }
        
        if ( cleanStart ) {
            // -- Create the XML defined messages and process through the system
            command.body().setDifxStart( jobStart );
            try {
                //command.sendPacket( _settings.guiServerConnection().COMMAND_PACKET );
                command.send();
            } catch ( java.net.UnknownHostException e ) {
                java.util.logging.Logger.getLogger("global").log(java.util.logging.Level.SEVERE, null,
                        e.getMessage() );  //BLAT should be a pop-up
                _jobNode.setState( "Failed Start", Color.RED );
                _jobNode.autostate( JobNode.AUTOSTATE_FAILED );
            }
        }
        else {
            _jobNode.setState( "Socket Failure", Color.RED );
            _jobNode.autostate( JobNode.AUTOSTATE_FAILED );
        }
    }
    
    /*
     * This thread opens and monitors a TCP socket for diagnostic reports from the
     * guiServer as it runs a job.
     */
    protected class RunningJobMonitor extends Thread {
        
        public RunningJobMonitor( int port ) {
            _port = port;
        }
        
        public int port() { return _port; }
        
        protected ChannelServerSocket _ssock;
        protected boolean _socketGood;
        
        /*
         * Set up the server socket for the guiServer.  This was split out from the
         * "run()" thread because it would fail in rare instances.  The return value
         * from this function indicates whether the socket worked.
         */
        public boolean socketInit() {
            _socketGood = false;
            int socketTryCount = 0;
            _ssock = null;
            while ( !_socketGood && socketTryCount < 10 ) {
                try {
                    _ssock = new ChannelServerSocket( _port, _settings );
                    _socketGood = true;
                }
                catch ( java.net.BindException e ) { 
                    ++socketTryCount;
                    try { Thread.sleep( 100 ); } catch ( Exception ex ) {}
                    _settings.releaseTransferPort( _port );
                } 
                catch ( java.io.IOException e ) {
                    e.printStackTrace();
                }

            }
            return _socketGood;
        }
        
        /*
         * These packet types are sent by the "JobMonitorConnection" class in the
         * guiServer application on the DiFX host.
         */
        protected final int RUN_DIFX_JOB_TERMINATED                   = 100;
        protected final int RUN_DIFX_JOB_ENDED_GRACEFULLY             = 101;
        protected final int RUN_DIFX_JOB_STARTED                      = 102;
        protected final int RUN_DIFX_PARAMETER_CHECK_IN_PROGRESS      = 103;
        protected final int RUN_DIFX_PARAMETER_CHECK_SUCCESS          = 104;
        protected final int RUN_DIFX_FAILURE_NO_HEADNODE              = 105;
        protected final int RUN_DIFX_FAILURE_NO_DATASOURCES           = 106;
        protected final int RUN_DIFX_FAILURE_NO_PROCESSORS            = 107;
        protected final int RUN_DIFX_FAILURE_NO_INPUTFILE_SPECIFIED   = 108;
        protected final int RUN_DIFX_FAILURE_INPUTFILE_NOT_FOUND      = 109;
        protected final int RUN_DIFX_FAILURE_INPUTFILE_NAME_TOO_LONG  = 110;
        protected final int RUN_DIFX_FAILURE_OUTPUT_EXISTS            = 111;
        protected final int RUN_DIFX_DELETING_PREVIOUS_OUTPUT         = 112;
        protected final int RUN_DIFX_STARTING_DIFX                    = 113;
        protected final int RUN_DIFX_DIFX_MESSAGE                     = 114;
        protected final int RUN_DIFX_DIFX_WARNING                     = 115;
        protected final int RUN_DIFX_DIFX_ERROR                       = 116;
        protected final int RUN_DIFX_DIFX_COMPLETE                    = 117;
        protected final int RUN_DIFX_DATA_FILE_SIZE                   = 118;
        protected final int RUN_DIFX_JOB_FAILED                       = 119;
        protected final int RUN_DIFX_JOB_ENDED_WITH_ERRORS            = 120;
        protected final int RUN_DIFX_DIFX_MONITOR_CONNECTION_ACTIVE   = 121;
        protected final int RUN_DIFX_DIFX_MONITOR_CONNECTION_BROKEN   = 122;
        protected final int RUN_DIFX_DIFX_MONITOR_CONNECTION_FAILED   = 123;
        protected final int RUN_DIFX_FAILURE_INPUTFILE_BAD_CONFIG     = 124;
        protected final int RUN_DIFX_CONFIG_PASSED                    = 125;
        protected final int RUN_DIFX_CONFIG_COMMAND_NOT_FOUND         = 126;
        protected final int RUN_DIFX_RUN_CONFIG_CHECK                 = 127;
        protected final int RUN_DIFX_STOPPED_AFTER_CONFIG             = 128;
                
        @Override
        public void run() {
            //  Open a new server socket and await a connection.  The connection
            //  will timeout after a given number of seconds (nominally 10).
//            try {
//                //  Because this is an important socket, and because it sometimes fails,
//                //  repeatedly try to open it on failure.  Give up after 10.
//                boolean socketGood = false;
//                int socketTryCount = 0;
//                ChannelServerSocket ssock = null;
//                while ( !socketGood && socketTryCount < 10 ) {
//                    try {
//                        ssock = new ChannelServerSocket( _port, _settings );
//                        socketGood = true;
//                    }
//                    catch ( java.net.BindException e ) { 
//                        ++socketTryCount;
//                        try { Thread.sleep( 100 ); } catch ( Exception ex ) {}
//                        _settings.releaseTransferPort( _port );
//                    }
//                }
                if ( _socketGood ) {
                    try {
                        _ssock.setSoTimeout( 10000 );  //  timeout is in millisec
                    } catch ( java.net.SocketException e ) {
                        e.printStackTrace();
                    }
                    try {
                        _ssock.accept();
                        //  Loop collecting diagnostic packets from the guiServer.  These
                        //  are identified by an initial integer, and then are followed
                        //  by a data length, then data.
                        boolean connected = true;
                        while ( connected ) {
                            //  Read the packet type as an integer.  The packet types
                            //  are defined above (within this class).
                            int packetType = _ssock.readInt();
                            //  Read the size of the incoming data (bytes).
                            int packetSize = _ssock.readInt();
                            //  Read the data (as raw bytes)
                            byte [] data = null;
                            if ( packetSize > 0 ) {
                                data = new byte[packetSize];
                                _ssock.readFully( data, 0, packetSize );
                            }
                            //  Interpret the packet type.
                            if ( packetType == RUN_DIFX_JOB_FAILED ) {
                                _messageDisplayPanel.error( 0, "job monitor", "Job failed to complete." );
                                statusError( "job failed to complete" );
                                statusPanelColor( _statusPanelBackground.darker()  );
                                connected = false;
                                _jobNode.setState( "Failed", Color.RED );
                                _jobNode.lockState( true );
                            }
                            else if ( packetType == RUN_DIFX_JOB_TERMINATED ) {
                                _messageDisplayPanel.warning( 0, "job monitor", "Job terminated by user." );
                                statusWarning( "job terminated by user" );
                                statusPanelColor( _statusPanelBackground.darker() );
                                connected = false;
                                _jobNode.setState( "Terminated", Color.RED );
                                _jobNode.lockState( true );
                            }
                            else if ( packetType == RUN_DIFX_JOB_ENDED_GRACEFULLY ) {
                                _messageDisplayPanel.warning( 0, "job monitor", "Job finished gracefully." );
                                statusInfo( "job completed" );
                                connected = false;
                                statusPanelColor( _statusPanelBackground.darker() );
                                lockState( true );
                            }
                            else if ( packetType == RUN_DIFX_JOB_STARTED ) {
                                _doneWithErrors = false;
                                _messageDisplayPanel.message( 0, "job monitor", "Job started by guiServer." );
                                _jobNode.lockState( false );
                                statusInfo( "job started" );
                            }
                            else if ( packetType == RUN_DIFX_JOB_ENDED_WITH_ERRORS ) {
                                _doneWithErrors = true;
                                connected = false;
                            }
                            else if ( packetType == RUN_DIFX_PARAMETER_CHECK_IN_PROGRESS ) {
                                _messageDisplayPanel.message( 0, "job monitor", "Checking parameters." );
                                statusInfo( "checking parameters..." );
                            }
                            else if ( packetType == RUN_DIFX_PARAMETER_CHECK_SUCCESS ) {
                                _messageDisplayPanel.message( 0, "job monitor", "Parameter check successful." );
                            }
                            else if ( packetType == RUN_DIFX_FAILURE_NO_HEADNODE ) {
                                _messageDisplayPanel.error( 0, "job monitor", "No headnone was specified." );
                            }
                            else if ( packetType == RUN_DIFX_FAILURE_NO_DATASOURCES ) {
                                _messageDisplayPanel.error( 0, "job monitor", "No valid data sources were specified." );
                            }
                            else if ( packetType == RUN_DIFX_FAILURE_NO_PROCESSORS ) {
                                _messageDisplayPanel.error( 0, "job monitor", "No valid processors were specified." );
                            }
                            else if ( packetType == RUN_DIFX_FAILURE_NO_INPUTFILE_SPECIFIED ) {
                                _messageDisplayPanel.error( 0, "job monitor", "No input file was specified." );
                            }
                            else if ( packetType == RUN_DIFX_FAILURE_INPUTFILE_NOT_FOUND ) {
                                _messageDisplayPanel.error( 0, "job monitor", "Input file " + _jobNode.inputFile() + " was not found on DiFX host." );
                            }
                            else if ( packetType == RUN_DIFX_FAILURE_INPUTFILE_NAME_TOO_LONG ) {
                                _messageDisplayPanel.message( 0, "job monitor", "Input file name \"" + _jobNode.inputFile() + "\" is too long for DiFX." );
                            }
                            else if ( packetType == RUN_DIFX_FAILURE_OUTPUT_EXISTS ) {
                                _messageDisplayPanel.error( 0, "job monitor", "Output exists for this job on DiFX host - use \"force\" to replace." );
                            }
                            else if ( packetType == RUN_DIFX_DELETING_PREVIOUS_OUTPUT ) {
                                statusInfo( "force output - deleting existing output files" );
                                _messageDisplayPanel.warning( 0, "job monitor", "force output - deleting existing output files" );
                            }
                            else if ( packetType == RUN_DIFX_STARTING_DIFX ) {
                                statusInfo( "DiFX running!" );
                                _messageDisplayPanel.warning( 0, "job monitor", "DiFX started!" );
                                statusPanelColor( Color.GREEN );                            //  turn the frame green!!!!
                                _jobNode.setState( "Starting", Color.YELLOW );
                            }
                            else if ( packetType == RUN_DIFX_DIFX_MESSAGE ) {
                                if ( data != null )
                                    _messageDisplayPanel.message( 0, "job monitor", new String( data ) );
                                else
                                    _messageDisplayPanel.message( 0, "job monitor", "" );
                            }
                            else if ( packetType == RUN_DIFX_DIFX_WARNING ) {
                                if ( data != null )
                                    _messageDisplayPanel.warning( 0, "job monitor", new String( data ) );
                                else
                                    _messageDisplayPanel.warning( 0, "job monitor", "" );
                            }
                            else if ( packetType == RUN_DIFX_DIFX_ERROR ) {
                                if ( data != null )
                                    _messageDisplayPanel.error( 0, "job monitor", new String( data ) );
                                else
                                    _messageDisplayPanel.error( 0, "job monitor", "" );
                                statusPanelColor( Color.ORANGE );
                                _jobNode.setState( "DiFX running with errors", Color.ORANGE );
                            }
                            else if ( packetType == RUN_DIFX_DIFX_COMPLETE ) {
                                statusInfo( "DiFX compete!" );
                                _messageDisplayPanel.warning( 0, "job monitor", "DiFX complete!" );
                                statusPanelColor( _statusPanelBackground.darker() );
                            }
                            else if ( packetType == RUN_DIFX_DIFX_MONITOR_CONNECTION_ACTIVE ) {
                                if ( _liveMonitorWindow == null )
                                    _liveMonitorWindow = new LiveMonitorWindow( MouseInfo.getPointerInfo().getLocation().x, 
                                        MouseInfo.getPointerInfo().getLocation().y, _settings, _inputFileName.getText() );
                                _liveMonitorWindow.connectionInfo( "CONNECTED", "connected" );
                            }
                            else if ( packetType == RUN_DIFX_DIFX_MONITOR_CONNECTION_BROKEN ) {
                                if ( _liveMonitorWindow == null )
                                    _liveMonitorWindow = new LiveMonitorWindow( MouseInfo.getPointerInfo().getLocation().x, 
                                        MouseInfo.getPointerInfo().getLocation().y, _settings, _inputFileName.getText() );
                                _liveMonitorWindow.connectionInfo( "NOT CONNECTED", "connection broken" );
                            }
                            else if ( packetType == RUN_DIFX_DIFX_MONITOR_CONNECTION_FAILED ) {
                                if ( _liveMonitorWindow == null )
                                    _liveMonitorWindow = new LiveMonitorWindow( MouseInfo.getPointerInfo().getLocation().x, 
                                        MouseInfo.getPointerInfo().getLocation().y, _settings, _inputFileName.getText() );
                                _liveMonitorWindow.connectionInfo( "NOT CONNECTED", "connection failed" );
                            }
                            else if ( packetType == RUN_DIFX_FAILURE_INPUTFILE_BAD_CONFIG ) {
                                _messageDisplayPanel.error( 0, "job monitor", "Job .input file failed configuration test." );
                                statusError( "configuration failure" );
                                statusPanelColor( _statusPanelBackground.darker() );
                                connected = false;
                                _jobNode.setState( "Config Failed", Color.RED );
                                _jobNode.lockState( true );
                            }
                            else if ( packetType == RUN_DIFX_STOPPED_AFTER_CONFIG ) {
                                lockState( true );
                                connected = false;
                            }
                            else if ( packetType == RUN_DIFX_CONFIG_PASSED ) {
                                _messageDisplayPanel.warning( 0, "job monitor", "Job .input file configuration test passed." );
                                statusInfo( "configuration passed" );
                                _jobNode.setState( "Config Passed", Color.YELLOW );
                            }
                            else if ( packetType == RUN_DIFX_RUN_CONFIG_CHECK ) {
                                _messageDisplayPanel.warning( 0, "job monitor", "Running configuration test." );
                                statusInfo( "configuration test" );
                                _jobNode.setState( "Config Check", Color.YELLOW );
                            }
                            else if ( packetType == RUN_DIFX_CONFIG_COMMAND_NOT_FOUND ) {
                                _messageDisplayPanel.warning( 0, "job monitor", "Configuration test program does not exist." );
                            }
                            else {
                                _messageDisplayPanel.warning( 0, "GUI", "Ignoring unrecongized job monitor packet type (" + packetType + ")." );
                            }
                        }
                    } catch ( SocketTimeoutException e ) {
                        e.printStackTrace();
                    } catch ( java.io.IOException e ) {
                        e.printStackTrace();
                    }
                    try { _ssock.close(); }
                    catch ( java.io.IOException e ) {
                        e.printStackTrace();
                    }
                }
            //  We keep the state of this job "running" for a little bit so that we properly
            //  process any late messages.
            try { Thread.sleep( 1000 ); } catch ( Exception e ) {}
            _settings.releaseTransferPort( _port );
            _jobNode.setJobEnd();
            //  This eliminates this job from any nodes where it is running.
            //  Note that this will not exactly work if you are running the same
            //  job multiple times, but if you are doing that many other things are
            //  probably not going to work, so this should be minor.
            for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
                    iter2.hasNext(); ) {
                ProcessorNode usedNode = (ProcessorNode)(iter2.next());
                usedNode.removeJob( _this );
            }
            _jobNode._scheduleJobItem.setEnabled( true );
            _jobNode._scheduleConfigTestItem.setEnabled( true );
            if ( _jobNode.schedulerConfigOnly() )
                _jobNode.autostate( JobNode.AUTOSTATE_DONE );
            else {
                //  Base the final "autostate" on the color of the display.  This is pretty
                //  kludgey, although it has the advantage that it will look consistent to the user.
                if ( _jobNode.state().getBackground() == Color.RED )
                    _jobNode.autostate( JobNode.AUTOSTATE_FAILED );
                else
                    _jobNode.autostate( JobNode.AUTOSTATE_DONE );
            }
        }
        
        protected int _port;
        
    }
    
    //--------------------------------------------------------------------------
    //  This is used to remove this job from the nodes where it has reserved resources,
    //  freeing up those resources for other jobs.  This should only be called when the
    //  job is stopped, or when it is assumed to have stopped.
    //--------------------------------------------------------------------------
    public void flushFromActiveNodes() {
        for ( Iterator<BrowserNode> iter2 = _settings.hardwareMonitor().processorNodes().children().iterator();
                iter2.hasNext(); ) {
            ProcessorNode usedNode = (ProcessorNode)(iter2.next());
            usedNode.removeJob( _this );
        }
    }
    
    //--------------------------------------------------------------------------
    //  Set the text and color of the "state" display.  This should only be called
    //  by a function setting the state both here in the monitor/editor and
    //  on the JobNode.  See _jobNode.setState().
    //--------------------------------------------------------------------------
    public void setState( String newState, Color newColor ) {
        _state.setText( newState );
        _state.setBackground( newColor );
        _state.updateUI();
    }
    
    //--------------------------------------------------------------------------
    //  Set the progress displayed on the progress bar.  This should only be called
    //  by a function setting progress values both here in the monitor/editor and
    //  on the JobNode.  See _jobNode.setProgress().
    //--------------------------------------------------------------------------
    public void setProgress( int i ) {
        _progress.setValue( i );
    }
    
    //--------------------------------------------------------------------------
    //  The "lockstate" indicates that running a job is "locked" - which means
    //  you can't do it.  This is done when the job is running.  
    //--------------------------------------------------------------------------
    public void lockState( boolean newVal ) {
        _startButton.setEnabled( newVal );
        _stopButton.setEnabled( !newVal );
    }
    
    public void pauseJob() {}
    
    public void stopJob() {
        _jobNode.setState( "Terminated", Color.RED );
        _jobNode.lockState( true );
        DiFXCommand command = new DiFXCommand( _settings );
        command.header().setType( "DifxStop" );
        command.mpiProcessId( "-1" );
        command.identifier( _jobNode.name() );

        // Create start job command
        DifxStop jobStop = command.factory().createDifxStop();
        jobStop.setInput( _jobNode.inputFile() );
        jobStop.setDifxVersion( _settings.difxVersion() );

        // -- Create the XML defined messages and process through the system
        command.body().setDifxStop( jobStop );
        try {
            //command.sendPacket( _settings.guiServerConnection().COMMAND_PACKET );
            command.send();
        } catch ( java.net.UnknownHostException e ) {
            java.util.logging.Logger.getLogger("global").log(java.util.logging.Level.SEVERE, null,
                    e.getMessage() );  //BLAT should be a pop-up
        }
        //  Clear the job from the nodes in the hardware monitor.  This should
        //  occur on its own, but sometimes it doesn't.
        Thread clearThread = new Thread() {
            public void run() {
                //  Give the "stop" command a chance to work.
                try { Thread.sleep( 500 ); } catch ( Exception ex ) {}
                for ( Iterator<BrowserNode> iter = _settings.hardwareMonitor().processorNodes().children().iterator();
                        iter.hasNext(); ) {
                    BrowserNode thisModule = iter.next();
                    if ( ((ProcessorNode)(thisModule)).activeJob().contentEquals( _jobNode.name() ) )
                        ((ProcessorNode)(thisModule)).clearActiveJob();
                }
                for ( Iterator<BrowserNode> iter = _settings.hardwareMonitor().mk5Modules().children().iterator();
                        iter.hasNext(); ) {
                    BrowserNode thisModule = iter.next();
                    if ( ((ProcessorNode)(thisModule)).activeJob().contentEquals( _jobNode.name() ) )
                        ((ProcessorNode)(thisModule)).clearActiveJob();
                }
                //  Redo this, just in case.
                _jobNode.autostate( AUTOSTATE_STOPPED );
                //  Free up resources...this should happen elsewhere, but doing it here shouldn't
                //  hurt.
                flushFromActiveNodes();
                //  Safe to let the user start it again.
                _jobNode._scheduleJobItem.setEnabled( true );
                _jobNode._scheduleConfigTestItem.setEnabled( true );
            }
        };
        clearThread.start();
    }
    
    /*
     * Add a listener to "state change" events.  These occur when this class starts,
     * pauses, or stops a job, or recognizes that a job has finished running.
     */
    public void addActionListener( ActionListener a ) {
        _stateChangeListeners.add( ActionListener.class, a );
    }

    /*
     * Send a "state change" event to all listeners.
     */
    protected void stateChangeEvent() {
        Object[] listeners = _stateChangeListeners.getListenerList();
        int numListeners = listeners.length;
        for ( int i = 0; i < numListeners; i+=2 ) {
            if ( listeners[i] == ActionListener.class )
                ((ActionListener)listeners[i+1]).actionPerformed( null );
        }
    }
    
    /*
     * This class provides information about a "Processor".  It allows
     * the user to use the processor as the "head node" and tracks the number of
     * cores available.
     */
    protected class PaneProcessorNode extends BrowserNode {
        
        public PaneProcessorNode( String name ) {
            super( name );
        }
        
        @Override
        public void createAdditionalItems() {
            _selected = new JCheckBox();
            _selected.setBackground( Color.WHITE );
            //  Any time the user specifically selects an item, we change it state
            //  slightly - it can't be automatically selected or unselected based
            //  on cpu load.
            _selected.addActionListener( new ActionListener() {
                public void actionPerformed( ActionEvent e ) {
                    _handSelected = true;
                }
            });
            this.add( _selected );
            _popup = new JPopupMenu();
            JMenuItem menuItem2 = new JMenuItem( "Make " + this.name() + " the head node" );
            menuItem2.addActionListener( new ActionListener() {
                public void actionPerformed( ActionEvent e ) {
                    _headNode.setText( name() );
                }
            });
            _popup.add( menuItem2 );
            _threads = new NumberBox();
            _threads.setHorizontalAlignment( JTextField.RIGHT );
            _threads.intValue( 0 );
            _threads.minimum( 0 );
            this.add( _threads );
            _coresDisplay = new JLabel("");
            _coresDisplay.setHorizontalAlignment( JLabel.RIGHT );
            this.add( _coresDisplay );
            _cpuDisplay = new JLabel("");
            _cpuDisplay.setHorizontalAlignment( JLabel.RIGHT );
            this.add( _cpuDisplay );
            _mpiTestDisplay = new JLabel("");
            _mpiTestDisplay.setHorizontalAlignment( JLabel.RIGHT );
            this.add( _mpiTestDisplay );
        }
        
        @Override
        public void positionItems() {
            super.positionItems();
            _selected.setBounds( 7, 2, 18, 18 );
            _threads.setBounds( 210, 1, 30, 18 );
            _coresDisplay.setBounds( 280, 1, 30, 18 );
            _cpuDisplay.setBounds( 360, 1, 70, 18 );
            _mpiTestDisplay.setBounds( 460, 1, 70, 18 );
        }
        
        public void cores( int newVal ) { 
            _cores = newVal;
            _coresDisplay.setText( String.valueOf( newVal ) );
        }
        public int cores() { return _cores; }
        public void threads( int newVal ) { _threads.intValue( newVal ); }
        public Integer threads() { return _threads.intValue(); }
        public String threadsText() { return _threads.getText(); }
        public float cpu() { return Float.parseFloat( _cpuDisplay.getText() ); }
        public void cpu( String newVal ) {
            _cpuDisplay.setText( newVal );
        }
        /*
         * Change the color of the cpu load if it exceeds a limit.  Also, select
         * or deselect if desired.
         */
        public boolean cpuTest( float limit, boolean selectLimit ) {
            if ( _cpuDisplay.getText().length() > 0 && Float.parseFloat( _cpuDisplay.getText() ) > limit ) {
                _cpuDisplay.setForeground( Color.RED );
                if ( selectLimit && !_handSelected )
                    _selected.setSelected( false );
                return false;
            }
            else {
                _cpuDisplay.setForeground( Color.BLACK );
                if ( selectLimit && !_handSelected )
                    _selected.setSelected( true );
                return true;
            }
        }
        /*
         * Set the result of the mpi test (and govern the selection if the test
         * failed).
         */
        public void mpiTest( boolean passed, boolean baseSelect ) {
            if ( passed ) {
                _mpiTestDisplay.setText( "OK" );
                _mpiTestDisplay.setForeground( Color.BLACK );
            }
            else {
                _mpiTestDisplay.setText( "Fail" );
                _mpiTestDisplay.setForeground( Color.RED );
                if ( baseSelect && !_handSelected ) {
                    _selected.setSelected( false );
                    _handSelected = true;
                }
            }           
        }
        
        public void clearTest() {
            _mpiTestDisplay.setText( "" );
        }
        
        public boolean selected() { return _selected.isSelected(); }
        public void selected( boolean newVal ) { _selected.setSelected( newVal ); }
        public void handSelected( boolean newVal ) { 
            _selected.setSelected( newVal );
            _handSelected = true;
        }        
        protected int _cores;
        protected NumberBox _threads;
        public boolean foundIt;
        protected JCheckBox _selected;
        protected JLabel _coresDisplay;
        protected JLabel _cpuDisplay;
        protected boolean _handSelected;
        protected JLabel _mpiTestDisplay;
        
    }
    
    /*
     * See if data are available to run this job as currently defined.  This isn't
     * a perfect check, in that it will under many circumstances succeed when there
     * isn't really data.  But it will catch some things. Failure changes the state of
     * the job, both in the monitor window and in the job node.
     */
    public boolean dataAvailableCheck() {
        boolean dataOK = true;
        String dataMissing = "";
        for ( Iterator<BrowserNode> iter = _dataSourcesPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            DataSource thisSource = (DataSource)(iter.next());
            if ( thisSource.missing() ) {
                if ( dataMissing.length() > 0 )
                    dataMissing += ", ";
                dataMissing += thisSource.antenna();
                dataOK = false;
            }
        }
        if ( !dataOK ) {
            _jobNode.setState( dataMissing + " data Missing", Color.RED );
        }
        return dataOK;
    }
    
    /*
     * Fill the processor list from the Hardware Monitor and the data sources
     * list from our collected information.
     */
    public void loadHardwareLists() {
        
        //  We need to "relocate" everything in the existing processor list, so unset a
        //  "found" flag for each.
        for ( Iterator<BrowserNode> iter = _processorsPane.browserTopNode().children().iterator();
                iter.hasNext(); )
            ( (PaneProcessorNode)(iter.next()) ).foundIt = false;        
        //  These are all of the processing nodes that the hardware monitor knows
        //  about.  See if they are in the list.
        for ( Iterator<BrowserNode> iter = _settings.hardwareMonitor().processorNodes().children().iterator();
                iter.hasNext(); ) {
            BrowserNode thisModule = iter.next();
            if ( !( (ProcessorNode)(thisModule) ).ignore() ) {
                BrowserNode foundNode = null;
                //  Is this processor in our list?
                for ( Iterator<BrowserNode> iter2 = _processorsPane.browserTopNode().children().iterator();
                        iter2.hasNext() && foundNode == null; ) {
                    BrowserNode testNode = iter2.next();
                    if ( testNode.name().contentEquals( thisModule.name() ) ) {
                        foundNode = testNode;
                        ( (PaneProcessorNode)(testNode) ).foundIt = true;
                    }
                }
                //  New node?  Then add it to the list.
                if ( foundNode == null ) {
                    PaneProcessorNode newNode = new PaneProcessorNode( thisModule.name() );
                    newNode.cores( ((ProcessorNode)(thisModule)).numCores() );
                    //  Eliminate this node if it matches the headnode and we aren't using the headnode.
                    if ( _restrictHeadnodeProcessing.isSelected() && _headNode.getText().contentEquals( thisModule.name() ) )
                        newNode.threads( 0 );
                    else {
                        if ( ((ProcessorNode)(thisModule)).numCores() > 1 )
                            newNode.threads( ((ProcessorNode)(thisModule)).numCores() - 1 );
                        else
                            newNode.threads( 1 );
                    }
                    newNode.cpu( ((ProcessorNode)(thisModule)).cpuUsage() );
                    newNode.cpuTest( (float)_busyPercentage.value(),
                            _eliminateBusyProcessors.isSelected() );
                    newNode.foundIt = true;
                    newNode.selected( !_processorsEdited );
                    _processorsPane.addNode( newNode );
                }
                else {
                    ( (PaneProcessorNode)foundNode ).cpu( ((ProcessorNode)(thisModule)).cpuUsage() );
                    ( (PaneProcessorNode)foundNode ).cpuTest( (float)_busyPercentage.value(),
                            _eliminateBusyProcessors.isSelected() );
                }
            }
        }
        //  Now purge the list of any items that were not "found"....
        for ( Iterator<BrowserNode> iter = _processorsPane.browserTopNode().children().iterator();
                iter.hasNext(); ) {
            try {
                PaneProcessorNode testNode = (PaneProcessorNode)(iter.next());
                if ( !testNode.foundIt )
                    _processorsPane.browserTopNode().removeChild( testNode );
                else {
                    //  This lets us know if anyone is editing the list.  If so, we
                    //  don't add new items "selected" by default.
                    if ( !testNode.selected() )
                        _processorsEdited = true;
                }
            } catch ( java.util.ConcurrentModificationException e ) {}
        }
        
        //  Sort the remaining items.
        _processorsPane.browserTopNode().sortByName();
        
        updateDataSourceList();
    }
    
    /*
     * This class is used to contain the name of a single data source.  The data
     * source can be a module, file or list of files, or network connection.  The 
     * controls necessary for each of these options differ quite a bit, so they 
     * will be in inheriting classes.
     */
    protected class DataSource extends IndexedPanel {
        
        public DataSource( int index ) {
            super( "" );
            closedHeight( 20 );
            openHeight( 20 );
            drawFrame( false );
            noArrow( true );
            darkTitleBar( false );
            alwaysOpen( true );
            setBackground( Color.WHITE );
            _index = index;
            _antenna.setText( _inputFile.telescopeTable().idx[_index].name );
            _sourceFormat.setText( _inputFile.datastreamTable().idx[_index].dataFormat );
            _sourceType.setText( _inputFile.datastreamTable().idx[_index].dataSource );
            buildSourceNodeList();
        }
        
        @Override
        public void createAdditionalItems() {
            _sourceNode = new StyledComboBox();
            _sourceNode.addPopupMenuListener( new PopupMenuListener() {
                public void popupMenuCanceled( PopupMenuEvent e) {
                }
                public void popupMenuWillBecomeInvisible( PopupMenuEvent e) {
                    setSelectedSourceNode();
                }
                public void popupMenuWillBecomeVisible( PopupMenuEvent e) {
                    buildSourceNodeList();
                }
            });
            _sourceNode.addActionListener( new ActionListener() {
                public void actionPerformed( ActionEvent e ) {
                }
            });
            this.add( _sourceNode );
            _this = this;
            _skipStation = new ZCheckBox( "" );
            _skipStation.addActionListener( new ActionListener() {
                public void actionPerformed( ActionEvent e ) {
                    if ( _skipStation.isSelected() ) {
                        _this.setForeground( Color.LIGHT_GRAY );
                    }
                    else {
                        _this.setForeground( Color.BLACK );
                    }
                }
            });
            this.add( _skipStation );
            _antenna = new JLabel( "" );
            _antenna.setHorizontalAlignment( JLabel.CENTER );
            this.add( _antenna );
            _sourceFormat = new JLabel( "" );
            _sourceFormat.setHorizontalAlignment( JLabel.LEFT );
            this.add( _sourceFormat );
            _sourceType = new JLabel( "" );
            _sourceType.setHorizontalAlignment( JLabel.LEFT );
            this.add( _sourceType );
        }
        
        @Override
        public void positionItems() {
            super.positionItems();
            _sourceNode.setBounds( 0, 1, 210, 18 );
            _skipStation.setBounds( 210, 1, 18, 18 );
            _antenna.setBounds( 230, 1, 40, 18 );
            _sourceFormat.setBounds( 270, 1, 70, 18 );
            _sourceType.setBounds( 350, 1, 70, 18 );
        }
        
        /*
         * Using all existing machines that can be used for the current data type,
         * build a list of nodes the user can pick from.  This generic form of the
         * function includes everything - Mark5s and processors.
         */
        public void buildSourceNodeList() {
            _sourceNode.removeAllItems();
            for ( Iterator<BrowserNode> iter = _settings.hardwareMonitor().processorNodes().children().iterator();
                    iter.hasNext(); ) {
                ProcessorNode thisModule = (ProcessorNode)(iter.next());
                _sourceNode.addItem( new NodePanel( thisModule.name(), null, null, null ) );
            }
            for ( Iterator<BrowserNode> iter = _settings.hardwareMonitor().mk5Modules().children().iterator();
                    iter.hasNext(); ) {
                Mark5Node thisModule = (Mark5Node)(iter.next());
                _sourceNode.addItem( new NodePanel( thisModule.name(), null, null, null ) );
            }
        }
        
        /*
         * This function is called when an item in the source node list is selected.
         * Some of the inheriting classes might do something special with this.
         */
        public void setSelectedSourceNode() {
        }
        
        /*
         * Of the current list of source nodes, pick the most appropriate one.  Which
         * one this is depends on the data source type so this is another inherited
         * class.
         */
        public void pickAppropriateSourceNode() {
        }
        
        /*
         * Set the source node to an item corresponding to a specified string - the source
         * node name.
         */
        public void setSourceNode( String name ) {
            int num = _sourceNode.getItemCount();
            boolean found = false;
            for ( int i = 0; i < num && !found; ++i ) {
                NodePanel node = _sourceNode.getItemAt( i );
                if ( node.name().contentEquals( name ) ) {
                    found = true;
                    _sourceNode.setSelectedIndex( i );
                }
            }
        }
        
        public String sourceNode() { return ((NodePanel)_sourceNode.getSelectedItem()).name(); }
        public String sourceType() { return _sourceType.getText(); }
        public String antenna() { return _antenna.getText(); }
        public boolean missing() { return _missing; }
        public boolean skipStation() { return _skipStation.isSelected(); };
        public void skipStation( boolean newVal ) {
            _skipStation.setSelected( newVal );
        }
        
        protected int _index;
        protected boolean _missing;
        protected StyledComboBox _sourceNode;
        protected JLabel _antenna;
        protected JLabel _sourceType;
        protected JLabel _sourceFormat;
        protected ZCheckBox _skipStation;
        protected DataSource _this;
 
        //  This class and the next make the popup list for the combo box as big as
        //  the items in it (as opposed to the same size as the combo box itself).
        class StyledComboBoxUI extends BasicComboBoxUI {
            protected ComboPopup createPopup() {
                BasicComboPopup popup = new BasicComboPopup( comboBox ) {
                    @Override
                    protected Rectangle computePopupBounds(int px,int py,int pw,int ph) {
                        return super.computePopupBounds(
                            px,py,Math.max(comboBox.getPreferredSize().width,pw),ph
                        );
                    }
                };
                popup.getAccessibleContext().setAccessibleParent(comboBox);
                return popup;
            }
        }

        class StyledComboBox extends JComboBox<JobEditorMonitor.NodePanel> {
            public StyledComboBox() {
                setUI( new StyledComboBoxUI() );
                setBackground( Color.WHITE );
                this.setFont( new Font( this.getFont().getName(), Font.BOLD, this.getFont().getSize() ) );
            }
        }
    }
    
    /*
     * Data source from a module.
     */
    public class ModuleSource extends DataSource {
        
        public ModuleSource( int index ) {
            super( index );
            moduleName( _inputFile.dataTable().idx[index].file[0] );
            if ( _chooseBasedOnModule.isSelected() )
                pickAppropriateSourceNode();
        }
        
        public void createAdditionalItems() {
            super.createAdditionalItems();
            _moduleName = new JLabel( "" );
            _moduleName.setHorizontalAlignment( JLabel.LEFT );
            this.add( _moduleName );
        }
        
        public void positionItems() {
            super.positionItems();
            _moduleName.setBounds( 430, 1, 70, 18 );
        }

        /*
         * Search through the list of known source nodes and pick the best of them
         * for this module (the one that contains this module!).
         */
        public void pickAppropriateSourceNode() {
            for ( int i = 0; i < _sourceNode.getItemCount(); ++i ) {
                NodePanel selection = (NodePanel)_sourceNode.getItemAt( i );
                if ( selection.moduleA().contentEquals( _moduleName.getText() ) ||
                    selection.moduleB().contentEquals( _moduleName.getText() ) )
                    _sourceNode.setSelectedIndex( i );
            }
            checkSelectedSourceNode();  
        }
        
        /*
         * Check the modules contained in the selected source against the one we want.
         * Change the color of the label for the module we want to red if it doesn't
         * match one of those in the source.
         */
        public void checkSelectedSourceNode() {
            NodePanel selection = (NodePanel)_sourceNode.getSelectedItem();
            if ( selection == null ) return;
            if ( selection.moduleA().contentEquals( _moduleName.getText() ) ||
                 selection.moduleB().contentEquals( _moduleName.getText() ) ) {
                _moduleName.setForeground( Color.BLUE );
                _moduleName.setToolTipText( "" );
                _missing = false;
            }
            else {
                _moduleName.setForeground( Color.RED );
                _moduleName.setToolTipText( "Required module is not owned by this Mark5." );
                _missing = true;
            }
        }
        
        /*
         * Set the modules contained in the selected source.  The modules can change
         * over time (people can remove them or put new ones in) - calling this
         * function can respond to these changes.
         */
        public void updateSelectedSourceNode() {
            NodePanel selection = (NodePanel)_sourceNode.getSelectedItem();
            if ( selection == null ) return;
            for ( Iterator<BrowserNode> iter = _settings.hardwareMonitor().mk5Modules().children().iterator();
                    iter.hasNext(); ) {
                Mark5Node thisModule = (Mark5Node)(iter.next());
                if ( thisModule.name().contentEquals( selection.name() ) ) {
                    selection.moduleA( thisModule.bankAVSN() );
                    selection.moduleB( thisModule.bankBVSN() );
                }
            }
            checkSelectedSourceNode();
        }
        
        /*
         * Modules only exist on Mark5 machines - so only include them in the list
         * of possible machines.
         */
        public void buildSourceNodeList() {
            _sourceNode.removeAllItems();
            for ( Iterator<BrowserNode> iter = _settings.hardwareMonitor().mk5Modules().children().iterator();
                    iter.hasNext(); ) {
                Mark5Node thisModule = (Mark5Node)(iter.next());
                _sourceNode.addItem( new NodePanel( thisModule.name(), thisModule.bankAVSN(), thisModule.bankBVSN(),
                        thisModule.activeBank() ) );
            }
        }
        
        /*
         * We have to override this because we have complex objects instead of simple
         * strings in our source node list.
         */
        public String sourceNode() { 
            NodePanel selection = (NodePanel)_sourceNode.getSelectedItem();
            return selection.name();
        }
        
        public void moduleName( String newVal ) { _moduleName.setText( newVal ); }
        public String moduleName() { return _moduleName.getText(); }
        
        protected JLabel _moduleName;
        
    }
    
    public class NodePanel extends JComponent {
        public NodePanel( String name, String moduleA, String moduleB, String activeBank ) {
            _name = name;
            _moduleA = moduleA;
            _moduleB = moduleB;
        }
        //  Override the "toString()" method to create a string out of the
        //  name and any modules contained.
        public String toString() {
            //  Try to get things to align properly...
            String rtn = _name + "                     ";
            if ( _moduleA != null && _moduleA.length() > 0 && !_moduleA.equalsIgnoreCase( "none" ) )
                rtn += "          " + _moduleA;
            if ( _moduleB != null && _moduleB.length() > 0 && !_moduleB.equalsIgnoreCase( "none" ) )
                rtn += "          " + _moduleB;
            return rtn;
        }
        public String name() { return _name; }
        public String moduleA() { return _moduleA; }
        public String moduleB() { return _moduleB; }
        public void moduleA( String newVal ) { _moduleA = newVal; }
        public void moduleB( String newVal ) { _moduleB = newVal; }
        public String activeBank() { return _activeBank; }
        protected String _name;
        protected String _moduleA;
        protected String _moduleB;
        protected String _activeBank;

    }
        
    /*
     * Data source from a file.
     */
    public class FileSource extends DataSource {
        
        public FileSource( int index ) {
            super( index );
            //  File data can include a list of files, each of which we display.
            //  Find out how many files we have and create a label for each one.
            int n = _inputFile.dataTable().idx[index].numFiles;
            if ( n == 0 ) {
                _missing = true;
                this.setBackground( new Color( 255, 100, 100 ) );
            }
            else
                _missing = false;
            _file = new JLabel[n];
            for ( int i = 0; i < n; ++i ) {
                //  This first file in the list needs to indicate that the list is expandable.
                if ( i == 0 && n > 1 ) {
                    _shortLabel = _inputFile.dataTable().idx[index].file[i];
                    _longLabel = _shortLabel + "...";
                    _file[i] = new JLabel( _longLabel );
                    alwaysOpen( false );
                    _open = false;
                    _sillyFrame = new JLabel( "" );
                    _sillyFrame.setBounds( 410, 3, 14, 14 );
                    Border border = BorderFactory.createLineBorder( Color.BLACK );
                    _sillyFrame.setBorder(border);
                    this.add( _sillyFrame );
                    _openClose = new JLabel( "+" );
                    _openClose.setBounds( 411, 3, 12, 12 );
                    _openClose.setHorizontalAlignment( JLabel.CENTER );
                    _openClose.setVerticalAlignment( JLabel.CENTER );
                    this.add( _openClose );
                }
                else {
                    _file[i] = new JLabel( _inputFile.dataTable().idx[index].file[i] );
                    if ( n > 1 )
                        openHeight( openHeight() + 20 );
                }
                _file[i].setHorizontalAlignment( JLabel.LEFT );
                this.add( _file[i] );
            }
        }
        
        public void openCloseButton() {
            _open = ! _open;
            if ( _open ) {
                _openClose.setText( "-" );
                _file[0].setText( _shortLabel );
            }
            else {
                _openClose.setText( "+" );
                _file[0].setText( _longLabel );
            }
            this.updateUI();
        }
        
        public void positionItems() {
            super.positionItems();
            if ( _file != null ) {
                for ( int i = 0; i < _file.length; ++i )
                    _file[i].setBounds( 430, i * 20 + 1, 1000, 18 );
            }
        }
        
        /*
         * Generate the "common path" for all files...this is the portion of the full
         * path of each file that is common to all.
         */
        public String commonFilePath() {
            if ( _file.length == 0 )
                return null;
            String commonStr = _file[0].getText();
            for ( int i = 1; i < _file.length; ++i ) {
                int sameIdx = 0;
                while ( sameIdx < commonStr.length() && commonStr.charAt( sameIdx ) == _file[0].getText().charAt( sameIdx ) )
                    ++sameIdx;
                commonStr = commonStr.substring( 0, sameIdx );
            }
            return commonStr;
        }

        /*
         * Override the mouseClicked event to make it work only when the "open/close"
         * button area is pushed.
         */
        @Override
        public void mouseClicked( MouseEvent e ) {
            if ( _openClose != null && e.getX() > 410 && e.getX() < 425 && e.getY() < 20 )
                openCloseButton();
        }

        protected JLabel[] _file;
        protected JLabel _openClose;
        protected JLabel _sillyFrame;
        protected String _shortLabel;
        protected String _longLabel;
    }
    
    /*
     * Data source from a network.
     */
    public class NetworkSource extends DataSource {
        
        public NetworkSource( int index ) {
            super( index );
        }
        
    }
    
    /*
     * Examine the current list of data sources for existence, sanity, and whatever
     * else.
     */
     public void buildDataSourceList() {
        //  Don't do anything if reading the input file failed.
        if ( _inputFile == null )
            return;
        //  Clear the existing data source list.
        _dataSourcesPane.clear();
        //  Add a new node for each data stream.  This is done based on source type.
        for ( int i = 0; i < _inputFile.commonSettings().activeDataStreams; ++i ) {
            if ( _inputFile.datastreamTable().idx[i].dataSource.contentEquals( "MODULE" ) )
                _dataSourcesPane.addNode( new ModuleSource( i ) );
            else if ( _inputFile.datastreamTable().idx[i].dataSource.contentEquals( "FILE" ) )
                _dataSourcesPane.addNode( new FileSource( i ) );
            else if ( _inputFile.datastreamTable().idx[i].dataSource.contentEquals( "NETWORK" ) )
                _dataSourcesPane.addNode( new NetworkSource( i ) );
            else //  Generic data source...not sure what good this is!
                _dataSourcesPane.addNode( new DataSource( i ) );
        }
    }
    
    /*
     * The data source list is built using the known data requirements (from
     * the .input file) in the "fillDataSourceList()" function.  Here we reflect any
     * changes in the state of data source machines or whatever associated with the
     * items on this list.
     */
    public void updateDataSourceList() {
        
        //  Look at each item in the data source list and check whether the node
        //  specified is appropriate to the data source itself - e.g. whether a chosen
        //  Mark5 has a needed module.
        for ( Iterator<BrowserNode> iter = _dataSourcesPane.browserTopNode().children().iterator();
            iter.hasNext(); ) {
            DataSource thisSource = (DataSource)iter.next();
            //  If this is a module source we check the modules "owned" by the source
            //  against those required.
            if ( thisSource.sourceType().contentEquals( "MODULE" ) ) {
                ModuleSource thisModuleSource = (ModuleSource)thisSource;
                thisModuleSource.updateSelectedSourceNode();
            }
            else if ( thisSource.sourceType().contentEquals( "FILE" ) ) {
            }
            else if ( thisSource.sourceType().contentEquals( "NETWORK" ) ) {
            }
            else {
            }
        }

    }
                
    /*
     * Called once a second to update things.
     */
    protected void timeoutIntervalEvent() {
        if ( this.isVisible() )
            loadHardwareLists();
    }
    
    /*
     * Override to load hardware lists before display.
     */
    @Override
    public void setVisible( boolean newVal ) {
        if ( newVal )
            loadHardwareLists();
        selectNodeDefaults( false, false );
        super.setVisible( newVal );
    }
    
    /*
     * Parse the string data in the .input file editor.
     */
    public void parseInputFile() {
        
        String str = _inputFileEditor.text();
        _inputFile = new InputFileParser( str );
        
        //  Extract some items from the .input file data.  
        _jobNode.calcFile( _inputFile.commonSettings().calcFile, true );
         _jobNode.outputFile( _inputFile.commonSettings().outputFile );
        executeTime( _inputFile.commonSettings().executeTime );
        startMJD( _inputFile.commonSettings().startMJD );
        startSeconds( _inputFile.commonSettings().startSeconds );
        _jobNode.numAntennas( _inputFile.telescopeTable().num );
        for ( int i = 0; i < _inputFile.telescopeTable().num; ++i )
            _jobNode.antennaName( i, _inputFile.telescopeTable().idx[i].name );
        
        
        _jobNode.jobStart( (double)startMJD() + (double)startSeconds() / 24.0 / 3600.0 );
        _jobNode.jobDuration( (double)executeTime() );
        _jobNode.updateDatabase( "outputFile", _jobNode.outputFile() );
        _jobNode.updateDatabase( "jobStart", _jobNode.jobStart().toString() );
        _jobNode.updateDatabase( "jobDuration", _jobNode.jobDuration().toString() );
        _jobNode.updateDatabase( "numAntennas", _jobNode.numAntennas().toString() );
        
        buildDataSourceList();
        
        _inputFileParsed = true;
        
    }
    
    /*
     * Give this job a link to the editor, which we need for some things.
     */
    public void editor( ExperimentEditor newVal ) { _editor = newVal; }

    /*
     * Parse the string data as if it came from an .input file (which, presumably,
     * it did).
     */
    public void parseCalcFile( String str ) {

        _calcFileEditor.text( str );
        Scanner strScan = new Scanner( str );
        strScan.useDelimiter( System.getProperty( "line.separator" ) );

        while ( strScan.hasNext() ) {
            String sCalc = strScan.next();
                if (sCalc.contains("JOB ID:")) {
                    sCalc = sCalc.substring(sCalc.indexOf(":") + 1);
//                    setJobID(sCalc.trim());
                } else if (sCalc.contains("OBSCODE:")) {
                    sCalc = sCalc.substring(sCalc.indexOf(":") + 1);
//                    setObsCode(sCalc.trim());
                } else if (sCalc.contains("JOB START TIME:")) {
                    sCalc = sCalc.substring(sCalc.indexOf(":") + 1);
//                    setJobStartTimeMJD( new BigDecimal( sCalc.trim() ) );
                } else if (sCalc.contains("JOB STOP TIME:")) {
                    sCalc = sCalc.substring(sCalc.indexOf(":") + 1);
//                    setJobStopTimeMJD(new BigDecimal(sCalc.trim()));
                } else if (sCalc.contains("NUM TELESCOPES:")) {
                    sCalc = sCalc.substring(sCalc.indexOf(":") + 1);
//                    setNumTelescopes(Integer.parseInt(sCalc.trim()));
                } else if (sCalc.contains("DIFX VERSION:")) {
                    sCalc = sCalc.substring(sCalc.indexOf(":") + 1);
                    _jobNode.difxVersion( sCalc.trim() );
//                    setDifxVersion(sCalc.trim());
                } else if ( sCalc.contains( "DUTY CYCLE:" ) ) {
                    sCalc = sCalc.substring( sCalc.indexOf( ":" ) + 1 );
                    _jobNode.dutyCycle( Double.parseDouble( sCalc.trim() ) );
                } else if (sCalc.contains("NAME:")) {
                    sCalc = sCalc.substring(sCalc.indexOf(":") + 1);
                } else if (sCalc.contains("SHELF:")) {
                    String sCalcObjID = sCalc.substring(sCalc.indexOf("SHELF") - 3, sCalc.indexOf("SHELF"));
                    String sCalcShelf = sCalc.substring(sCalc.indexOf(":") + 1);

                    // Find antenna via object ID
///                    Module curMod = getModule(Integer.parseInt(sCalcObjID.trim()));

                    // update the antenna's shelf
//                    curMod.setShelf(sCalcShelf.trim());

                    //newJob.setNumTelescopes(Integer.parseInt(trimmed));
                }
        }
        
        _calcFileParser = new CalcFileParser( str );

        _jobNode.updateDatabase( "difxVersion", _jobNode.difxVersion() );
        _jobNode.updateDatabase( "dutyCycle", _jobNode.dutyCycle().toString() );

        _calcFileParsed = true;
    }
    
    public Integer startMJD() { return _startMJD; }
    public void startMJD( Integer newVal ) { _startMJD = newVal; }
    public Integer startSeconds() { return _startSeconds; }
    public void startSeconds( Integer newVal ) { _startSeconds = newVal; }
    public Integer executeTime() { return _executeTime; }
    public void executeTime( Integer newVal ) { _executeTime = newVal; }
   
    protected EventListenerList _stateChangeListeners;
    protected JobNode _jobNode;
    protected SystemSettings _settings;
    
    protected NodeBrowserScrollPane _scrollPane;
    
    protected JMenuBar _menuBar;
    protected JFormattedTextField _headNode;
    protected JLabel _headNodeLabel;
    protected NodeBrowserScrollPane _dataSourcesPane;
    protected ZLabel _dataSourcesLabel;
    protected ZLabel _skipStationLabel;
    protected NodeBrowserScrollPane _processorsPane;
    protected JLabel _processorsLabel;
    protected JLabel _threadsLabel;
    protected JLabel _coresLabel;
    protected JLabel _cpuUsageLabel;
    protected JLabel _mpiTestLabel;
    protected JPopupMenu _machinesApplyPopup;
    protected JMenuItem _thisJobItem;
    protected JMenuItem _passJobsItem;
    protected JMenuItem _selectedJobsItem;
    protected JMenuItem _allJobsItem;
    protected JButton _applyToButton;
    protected JButton _applyMachinesButton;
    protected ZButton _defaultMachinesButton;
    protected ZButton _removeSkippedStationsButton;
    protected boolean _machinesAppliedByHand;
    protected JCheckBox _machinesLock;
    protected JCheckBox _forceOverwrite;
    protected JButton _selectAllProcessorsButton;
    protected JButton _deselectAllProcessorsButton;

    
    protected JButton _startButton;
    protected ZButton _configButton;
    protected JButton _stopButton;
    protected NumberBox _restartSeconds;
    protected JCheckBox _restartAt;
    
    protected boolean _allObjectsBuilt;
    
    protected HardwareMonitorPanel _hardwareMonitor;
    
    protected boolean _processorsEdited;
    protected SimpleTextEditor _inputFileEditor;
    protected SimpleTextEditor _calcFileEditor;
    protected JButton _refreshInputButton;
    protected JButton _refreshCalcButton;
    protected JButton _uploadInputButton;
    protected JButton _uploadCalcButton;
    protected JLabel _calcFileName;
    protected JLabel _inputFileName;
    protected SimpleTextEditor _machinesFileEditor;
    protected JButton _refreshMachinesButton;
    protected JButton _uploadMachinesButton;
    protected JLabel _machinesFileName;
    protected SimpleTextEditor _threadsFileEditor;
    protected JButton _refreshThreadsButton;
    protected JButton _uploadThreadsButton;
    protected JLabel _threadsFileName;
    protected ExperimentEditor _editor;
    protected JCheckBox _restrictHeadnodeProcessing;
    protected JCheckBox _eliminateNonrespondingProcessors;
    protected JCheckBox _eliminateBusyProcessors;
    protected NumberBox _busyPercentage;
    protected JLabel _busyPercentageLabel;
    protected JCheckBox _chooseBasedOnModule;
    protected JLabel _statusLabel;
    protected JCheckBox _runMonitor;
    protected JButton _showMonitorButton;
    
    protected ArrayList<String> _dataObjects;
    
    protected Integer _executeTime;
    protected Integer _startMJD;
    protected Integer _startSeconds;
    
    protected UpdateThread _updateThread;
    protected Timer _timeoutTimer;
    
    protected MessageDisplayPanel _messageDisplayPanel;
    protected IndexedPanel _statusPanel;
    protected Color _statusPanelBackground;
    
    protected JProgressBar _progress;
    protected ColumnTextArea _state;
    
    protected LiveMonitorWindow _liveMonitorWindow;
    
    protected boolean _doneWithErrors;
    public boolean doneWithErrors() { return _doneWithErrors; }
    
    protected InputFileParser _inputFile;
    
    protected JobEditorMonitor _this;
    
    protected int _doneCount;
    
    protected String _removedList;
    public String removedList() { return _removedList; }
    
    protected boolean _inputFileParsed;
    protected boolean _calcFileParsed;
    
    protected CalcFileParser _calcFileParser;
    
    public boolean inputFileParsed() {
        return _inputFileParsed;
    }
    public boolean calcFileParsed() {
        return _calcFileParsed;
    }
    
}
