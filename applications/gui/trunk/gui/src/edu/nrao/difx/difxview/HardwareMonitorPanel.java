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

import mil.navy.usno.widgetlib.NodeBrowserScrollPane;
import mil.navy.usno.widgetlib.BrowserNode;
import mil.navy.usno.widgetlib.TearOffPanel;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import javax.swing.JLabel;

import java.awt.Font;
import java.awt.Color;

import java.util.Iterator;

import edu.nrao.difx.difxcontroller.DiFXMessageProcessor;
import edu.nrao.difx.difxcontroller.AttributedMessageListener;

import edu.nrao.difx.difxutilities.SMARTMonitor;

import edu.nrao.difx.xmllib.difxmessage.DifxMessage;

public class HardwareMonitorPanel extends TearOffPanel {

    public HardwareMonitorPanel( SystemSettings settings ) {
        _settings = settings;
        _settings.hardwareMonitor( this );
        initComponents();
    }

    /*
     * This method allows me to control resize behavior.  Otherwise I have to
     * leave it up to the layouts, which is a disaster.
     */
    @Override
    public void setBounds(int x, int y, int width, int height) {
        _browserPane.setBounds( 0, 30, width, height - 30 );
        super.setBounds( x, y, width, height );
    }


    private void initComponents() {
        setLayout( null );
        _browserPane = new NodeBrowserScrollPane();
        this.add( _browserPane );
        _browserPane.setBackground( Color.WHITE );
        _mainLabel = new JLabel( "Hardware Monitor" );
        _mainLabel.setBounds( 5, 0, 150, 20 );
        _mainLabel.setFont( new Font( "Dialog", Font.BOLD, 14 ) );
        add( _mainLabel );
        _clusterNodes = new ProcessorNodesHeader( "Processor Nodes", _settings );
        _clusterNodes.addColumnChangeListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _mk5Modules.initializeDisplaySettings();
                _mk5Modules.updateDisplayedData();
                _mk5Modules.setChildColumnWidths();
            }
        } );
        _clusterNodes.backgroundColor( new Color( 255, 204, 153 ) );
        _browserPane.addNode( _clusterNodes );
        _mk5Modules = new Mark5NodesHeader( "Mark5 Nodes", _settings );
        _mk5Modules.addColumnChangeListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _clusterNodes.initializeDisplaySettings();
                _clusterNodes.updateDisplayedData();
                _clusterNodes.setChildColumnWidths();
            }
        } );
        _mk5Modules.backgroundColor( new Color( 255, 204, 153 ) );
        _browserPane.addNode( _mk5Modules );
        //  Callback when the user changes the length of network inactivity intervals.
        //  These are measured in seconds in the Settings window, but are set in tenths
        //  of seconds.
        _settings.inactivityChangeListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _clusterNodes.inactivitySettings( _settings.inactivityWarning() * 10,
                        _settings.inactivityError() * 10 );
                _mk5Modules.inactivitySettings( _settings.inactivityWarning() * 10,
                        _settings.inactivityError() * 10 );
            }
        } );
        addInvisibleNodesFromSettings();
    }
    
    /*
     * Force the display of "header" lines for each type of hardware.  This might
     * be useful if you aren't getting any messages from any type of hardware - by
     * default the headers won't show up unless some hardware is out there.
     */
    public void forceHeaders() {
        _clusterNodes.showThis( true );
        _mk5Modules.showThis( true );
    }
    
    /*
     * See if the settings file has any evidence of invisible nodes.
     */
    public void addInvisibleNodesFromSettings() {
        if ( _settings.invisibleProcessors() != null ) {
            String [] procList = _settings.invisibleProcessors().split( "," );
            String [] coreList = _settings.invisibleProcessorCores().split( "," );
            for ( int i = 0; i < procList.length; ++i ) {
                if ( procList[i].length() > 0 )
                    _clusterNodes.checkAddNode( procList[i], new Integer( coreList[i] ) );
            }
        }
        if ( _settings.invisibleMark5s() != null ) {
            String [] mk5List = _settings.invisibleMark5s().split( "," );
            for ( int i = 0; i < mk5List.length; ++i ) {
                if ( mk5List[i].length() > 0 )
                    _mk5Modules.checkAddNode( mk5List[i] );
            }
        }
    }
         
    /*
     * Sign up for callbacks from the DiFX message processor for different types
     * of messages.
     */
    public void difxMessageProcessor( DiFXMessageProcessor processor ) {
        processor.addDifxAlertMessageListener(new AttributedMessageListener() {
            @Override
            public void update( DifxMessage difxMsg ) {
                processDifxAlertMessage( difxMsg );
            }
        } );
        processor.addDifxLoadMessageListener(new AttributedMessageListener() {
            @Override
            public void update( DifxMessage difxMsg ) {
                processDifxLoadMessage( difxMsg );
            }
        } );
        processor.addMark5StatusMessageListener(new AttributedMessageListener() {
            @Override
            public void update( DifxMessage difxMsg ) {
                processMark5StatusMessage( difxMsg );
            }
        } );
        //  This may not be the place to absorb DiFX Info messages
        processor.addDifxInfoMessageListener(new AttributedMessageListener() {
            @Override
            public void update( DifxMessage difxMsg ) {
                processDifxInfoMessage( difxMsg );
            }
        } );
        //  Jobs are also interested in diagnostic messages as they pertain to a job.
        processor.addDifxDiagnosticMessageListener(new AttributedMessageListener() {
            @Override
            public void update( DifxMessage difxMsg ) {
                processDifxDiagnosticMessage( difxMsg );
            }
        } );
        _smartMonitor = new SMARTMonitor( _settings );
        _smartMonitor.difxMessageProcessor( processor );
    }
    
    protected void processDifxAlertMessage( DifxMessage difxMsg ) {
        
        if ( isFromMark5( difxMsg ) ) {
            //  Find the node that created this message.
            Mark5Node mk5Module = null;
            for ( Iterator<BrowserNode> iter = _mk5Modules.children().iterator(); iter.hasNext() && mk5Module == null; ) {
                BrowserNode thisModule = iter.next();
                if ( thisModule.name().contentEquals( difxMsg.getHeader().getFrom() ) )
                    mk5Module = (Mark5Node)thisModule;
            }
            if ( mk5Module == null ) {
                mk5Module = new Mark5Node( difxMsg.getHeader().getFrom(), _settings, _smartMonitor );
                _mk5Modules.addChild( mk5Module );
            }
            mk5Module.alertMessage( difxMsg );
        }
            
        else {

            ProcessorNode processor = null;
            for ( Iterator<BrowserNode> iter = _clusterNodes.children().iterator(); iter.hasNext() && processor == null; ) {
                BrowserNode thisModule = iter.next();
                if ( thisModule.name().contentEquals( difxMsg.getHeader().getFrom() ) )
                    processor = (ProcessorNode)thisModule;
            }
            if ( processor == null ) {
                processor = new ProcessorNode( difxMsg.getHeader().getFrom(), _settings );
                _clusterNodes.addChild( processor );
            }
            processor.alertMessage( difxMsg );

        }
    }
    
    /*
     * DiFX Info messages are probably from DiFX hardware, thus I'm including them here.
     * This may ultimately not be the correct place to respond to them.  In any case, they
     * are assumed to be important and are forwarded as "warnings" to the message center.
     */
    protected void processDifxInfoMessage( DifxMessage difxMsg ) {
        _settings.messageCenter().warning( 0, difxMsg.getHeader().getFrom(), difxMsg.getBody().getDifxInfo().getMessage() );
    }
    
    public synchronized void processMark5StatusMessage( DifxMessage difxMsg ) {

        if ( isFromMark5( difxMsg ) ) {
            
            Mark5Node mk5Module = null;
            for ( Iterator<BrowserNode> iter = _mk5Modules.children().iterator(); iter.hasNext() && mk5Module == null; ) {
                BrowserNode thisModule = iter.next();
                if ( thisModule.name().contentEquals( difxMsg.getHeader().getFrom() ) )
                    mk5Module = (Mark5Node)thisModule;
            }
            if ( mk5Module == null ) {
                mk5Module = new Mark5Node( difxMsg.getHeader().getFrom(), _settings, _smartMonitor );
                _mk5Modules.addChild( mk5Module );
            }
            mk5Module.statusMessage( difxMsg );
            
        } else {

            //  Mark5 status messages may actually be from processors.
            ProcessorNode processor = null;
            for ( Iterator<BrowserNode> iter = _clusterNodes.children().iterator(); iter.hasNext() && processor == null; ) {
                BrowserNode thisModule = iter.next();
                if ( thisModule.name().contentEquals( difxMsg.getHeader().getFrom() ) )
                    processor = (ProcessorNode)thisModule;
            }
            if ( processor == null ) {
                processor = new ProcessorNode( difxMsg.getHeader().getFrom(), _settings );
                _clusterNodes.addChild( processor );
            }
            processor.statusMessage( difxMsg ); 

        }

    }

    public synchronized void processDifxLoadMessage( DifxMessage difxMsg ) {

        if ( isFromMark5( difxMsg ) ) {

            Mark5Node mk5Module = null;
            for ( Iterator<BrowserNode> iter = _mk5Modules.children().iterator(); iter.hasNext() && mk5Module == null; ) {
                BrowserNode thisModule = iter.next();
                if ( thisModule.name().contentEquals( difxMsg.getHeader().getFrom() ) )
                    mk5Module = (Mark5Node)thisModule;
            }
            if ( mk5Module == null ) {
                mk5Module = new Mark5Node( difxMsg.getHeader().getFrom(), _settings, _smartMonitor );
                _mk5Modules.addChild( mk5Module );
            }
            mk5Module.loadMessage( difxMsg );
            
        } else {

            ProcessorNode processor = null;
            for ( Iterator<BrowserNode> iter = _clusterNodes.children().iterator(); iter.hasNext() && processor == null; ) {
                BrowserNode thisModule = iter.next();
                if ( thisModule.name().contentEquals( difxMsg.getHeader().getFrom() ) )
                    processor = (ProcessorNode)thisModule;
            }
            if ( processor == null ) {
                processor = new ProcessorNode( difxMsg.getHeader().getFrom(), _settings );
                _clusterNodes.addChild( processor );
            }
            processor.loadMessage( difxMsg ); 

        }

    }

    //--------------------------------------------------------------------------
    //!  Process a diagnostic message.  Diagnostic messages are produced by running
    //!  jobs, but they have information that is specific to processing and data-reading
    //!  nodes.  A nice side-benefit is that they can tell us what job(s) a node is
    //!  working on.
    //--------------------------------------------------------------------------
    public synchronized void processDifxDiagnosticMessage( DifxMessage difxMsg ) {
        
        if ( isFromMark5( difxMsg ) ) {
            //  Find the node that created this message.
            Mark5Node mk5Module = null;
            for ( Iterator<BrowserNode> iter = _mk5Modules.children().iterator(); iter.hasNext() && mk5Module == null; ) {
                BrowserNode thisModule = iter.next();
                if ( thisModule.name().contentEquals( difxMsg.getHeader().getFrom() ) )
                    mk5Module = (Mark5Node)thisModule;
            }
            if ( mk5Module == null ) {
                mk5Module = new Mark5Node( difxMsg.getHeader().getFrom(), _settings, _smartMonitor );
                _mk5Modules.addChild( mk5Module );
            }
            mk5Module.diagnosticMessage( difxMsg );
        }
            
        else {

            ProcessorNode processor = null;
            for ( Iterator<BrowserNode> iter = _clusterNodes.children().iterator(); iter.hasNext() && processor == null; ) {
                BrowserNode thisModule = iter.next();
                if ( thisModule.name().contentEquals( difxMsg.getHeader().getFrom() ) )
                    processor = (ProcessorNode)thisModule;
            }
            if ( processor == null ) {
                processor = new ProcessorNode( difxMsg.getHeader().getFrom(), _settings );
                _clusterNodes.addChild( processor );
            }
            processor.diagnosticMessage( difxMsg );

        }
    }
    
    /*
     * DiFX messages come from CPUs, either processors or the Mark5 systems.  At
     * the moment (and this is gross, and will be fixed) we are determining whether
     * the source of a message is a cluster node or a Mark5 unit by looking at its
     * name.  Anything that is not a Mark5 is assumed to be a cluster node.
     */
    public boolean isFromMark5( DifxMessage difxMsg ) {
        return _settings.isMark5Name( difxMsg.getHeader().getFrom() );
        //return difxMsg.getHeader().getFrom().substring(0, 5).equalsIgnoreCase( "mark5" );
    }
    
    public BrowserNode processorNodes() { return _clusterNodes; }
    public BrowserNode mk5Modules() { return _mk5Modules; }
    
    private NodeBrowserScrollPane _browserPane;
    protected ProcessorNodesHeader _clusterNodes;
    protected Mark5NodesHeader _mk5Modules;
    private JLabel _mainLabel;
    protected SystemSettings _settings;
    protected SMARTMonitor _smartMonitor;
    
}
