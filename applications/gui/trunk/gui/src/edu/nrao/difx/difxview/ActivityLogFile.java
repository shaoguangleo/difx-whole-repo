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
 * The Log File is used by the GUI to track activities related to a pass or job.
 * It is stored on the DiFX host in the same directory as the other files for the
 * pass/job (.input, etc).  Essentially it is a human-readable list of activities.  This
 * class can both parse an existing file (in the form of a String) and create a
 * new one, as well as add and extract specific bits of data.
 * 
 * Although begun for a specific purpose, this class has evolved into a pretty
 * generic thing, and is used in multiple places.
 */
package edu.nrao.difx.difxview;

import java.util.Calendar;
import java.util.ArrayDeque;
import java.text.SimpleDateFormat;

public class ActivityLogFile {
    
    /*
     * An instance is created with a full path file name.
     */
    public ActivityLogFile( String filename ) {
        _filename = filename;
        createNew();
    }
    
    /*
     * Create a new job log file.
     */
    public void createNew() {
        _content = "# Log File for \"" 
                + _filename.substring( 0, _filename.lastIndexOf( "." ) ) + "\"\n"
                + "#\n"
                + "# This file is generated by the GUI to keep track of activities related to the\n"
                + "# named pass/job/experiment/etc.  Items are appended to this file by the GUI\n"
                + "# chronologically with time stamps produced from the GUI (i.e. the clock on the\n"
                + "# GUI platform).\n"
                + "#\n"
                + "# Each logged \"item\" starts with the key \"<LOG>\" (use of this key within the\n"
                + "# text of items will cause serious confusion - don't do it), followed by the time\n"
                + "# stamp, and a block of text of any length (multiple lines are allowed).\n"
                + "#\n"
                + "# Comments can be inserted between logged items.  Unlike the logged items,\n"
                + "# comments ARE line-based.  Anything following a \"#\" at the first character position\n"
                + "# in a line is ignored.\n"
                + "#\n"
                + "# The GUI expects this format and parses the file regularly - hand-edit only with caution!\n"
                + "#\n";
        addLabelItem( "LOG FILE NAME", _filename );
        addLabelItem( "LOG FILE CREATED", "" );
    }
    
    /*
     * Use the given string as content.  This is assumed to be a complete log file,
     * so any existing content is over-written.
     */
    public void content( String text ) {
        _content = "";
        createNew();
        _content += text;
    }
    
    public String content() {
        return _content;
    }
    
    /*
     * Add a text item to the log file.  Items can contain any amount of text, including
     * newlines.  Avoid the character string "<LOG>" as that is used to indicate the start
     * of an item.
     */
    public void addItem( String text ) {
        Calendar cal = Calendar.getInstance();
        long time = cal.getTimeInMillis();
        SimpleDateFormat sdf = new SimpleDateFormat( "yyyy-MM-dd" );
        String dateString = sdf.format( time );
        sdf = new SimpleDateFormat( "HH:mm:ss.SSS" );
        String timeString = sdf.format( time );
        _content += "<LOG> " + dateString + " " + timeString + " " + text + "\n";
    }
    
    /*
     * Add an item with a "label".
     */
    public void addLabelItem( String label, String text ) {
        if ( text == null )
            text = "";
        addItem( label + ": " + text );
    }
    
    public class LogItem {
        public String label;
        public String time;
        public String text;
    }
    /*
     * Obtain all logged items where the label contains the given string.  Returned is
     * a structure containing the time stamp and text of each item (see above).
     */
    public ArrayDeque<LogItem> getItem( String labelKey ) {
        ArrayDeque<LogItem> itemList = null;
        boolean keepGoing = true;
        int offset = 0;
        while ( keepGoing ) {
            //  Find the next instance of the "<LOG>" key sequence, which indicates
            //  the start of a new item.  This should be at the start of a line.  If
            //  no instance was found (-1 returned) we should stop.
            int nextIndex = _content.indexOf( "\n<LOG>", offset );
            if ( nextIndex == -1 ) {
                keepGoing = false;
            }
            else {
                //  Create a new item structure to hold this item.
                LogItem newItem = new LogItem();
                //  Get the time stamp, label, and then the text of the item.  Comments
                //  may be embedded in the text - ignore them.
                offset = nextIndex + 7;
                newItem.time = _content.substring( offset, offset + 23 );
                offset += 24;
                newItem.label = _content.substring( offset, _content.indexOf( ":", offset ) );
                offset = _content.indexOf( ":", offset );
                //  This is a check for erroneous formatting of the file - shouldn't
                //  happen.  Just gets us out of the endless loop.
                if ( offset == -1 )
                    keepGoing = false;
                offset += 2;
                //  Extract the text (less comments).
                int nextLog = _content.indexOf( "\n<LOG>", offset );
                int nextComment = _content.indexOf( "\n#", offset );
                //  Maybe there were no further comments or log items - end of file.
                if ( nextLog == -1 && nextComment == -1 ) {
                    newItem.text = _content.substring( offset, _content.length() - 1 ); // gets rid of trailing newline!
                    keepGoing = false;
                }
                //  Next item might be a log...        
                else if ( nextComment == -1 || ( ( nextLog < nextComment ) && nextLog != -1 ) ) {
                    newItem.text = _content.substring( offset, nextLog );
                }
                //  If not, it must have been a comment.
                else {
                    newItem.text = _content.substring( offset, nextComment );
                }
                if ( newItem.label.contains( labelKey ) ) {
                    if ( itemList == null )
                        itemList = new ArrayDeque<LogItem>();
                    itemList.addLast( newItem );
                }
                    
            }
        }
        return itemList;
    }
    
    public String filename() {
        return _filename;
    }
    
    public boolean downloadExisting() { return _downloadExisting; }
    public void downloadExisting( boolean newVal ) { _downloadExisting = newVal; }
    
    protected String _filename;
    protected String _content;
    
    protected boolean _downloadExisting;

}
