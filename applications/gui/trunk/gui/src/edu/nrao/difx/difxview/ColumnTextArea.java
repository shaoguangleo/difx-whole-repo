/*
 * This class formats text for display as part of a column display.
 */
package edu.nrao.difx.difxview;

import java.awt.*;
import javax.swing.JPanel;
import javax.swing.text.JTextComponent;
import javax.swing.JToolTip;

import mil.navy.usno.widgetlib.ComplexToolTip;

import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseEvent;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

public class ColumnTextArea extends JPanel implements MouseListener, MouseMotionListener {
    
    public ColumnTextArea() {
        super();
        _text = "";
        _margin = 2;
        _justify = CENTER;
    }
    
    public ColumnTextArea( String newText ) {
        super();
        _text = newText;
    }
    
    public void justify( int newVal ) {
        _justify = newVal;
    }
    
    public void margin( int newVal ) {
        _margin = newVal;
    }
    
    public void setText( String newText ) {
        _text = newText;
    }
    
    public String getText() {
        return _text;
    }
    
    public void addKillButton( ActionListener a ) {
        _killListener = a;
        addMouseListener( this );
        addMouseMotionListener( this );
    }
    
    @Override
    public void paintComponent( Graphics g1 ) {
        Graphics2D g = (Graphics2D)g1;
        super.paintComponent( g );
        Dimension d = this.getSize();
        int height = g.getFontMetrics().getHeight();
        int descent = g.getFontMetrics().getDescent();
        String drawText = _text;
        if ( drawText == null )
            drawText = "null";
        int width = g.getFontMetrics().stringWidth( drawText );
        if ( _justify == LEFT )
            g.drawString( drawText, _margin, ( d.height - height ) / 2 + ( height - descent ) );
        else if ( _justify == RIGHT )
            g.drawString( drawText, d.width - _margin - width, ( d.height - height ) / 2 + ( height - descent ) );
        else
            g.drawString( drawText, _margin + ( d.width - width ) / 2, ( d.height - height ) / 2 + ( height - descent ) );
        //  The "kill button" is a little X in a circle that can be used to
        //  trigger an event when pressed.  It shows up on the right hand side
        //  of the text field unless the text is right justified (in which case
        //  it is on the left).
        if ( _showKillButton ) {
            int x = d.width - 12;
            if ( _justify == RIGHT )
                x = 3;
            if ( _darkKillButton )
                g.setColor( Color.DARK_GRAY );
            else
                g.setColor( Color.LIGHT_GRAY );
            g.fillOval( x, 2, 9, 9 );
            g.setColor( Color.WHITE );
            g.drawLine( x + 2, 4, x + 6, 8 );
            g.drawLine( x + 2, 8, x + 6, 4 );
        }
                        
            
    }
    
    @Override
    public void mouseClicked( MouseEvent e ) {
        if ( _darkKillButton ) {
            _killListener.actionPerformed( new ActionEvent( this, ActionEvent.ACTION_PERFORMED, "" ) );
        }
        else {
            Container foo = this.getParent();
            foo.dispatchEvent( e );
        }
    }
    
    @Override
    public void mouseEntered( MouseEvent e ) {
        _showKillButton = true;
        Container foo = this.getParent();
        foo.dispatchEvent( e );
    }
    
    @Override
    public void mouseExited( MouseEvent e ) {
        _showKillButton = false;
        Container foo = this.getParent();
        foo.dispatchEvent( e );
    }
    
    @Override
    public void mousePressed( MouseEvent e ) {
        Container foo = this.getParent();
        foo.dispatchEvent( e );
    }
    
    @Override
    public void mouseReleased( MouseEvent e ) {
        Container foo = this.getParent();
        foo.dispatchEvent( e );
    }
    
    @Override
    public void mouseMoved( MouseEvent e ) {
        Dimension d = this.getSize();
        if ( e.getX() < d.width - 2 && e.getX() > d.width - 12 &&
                e.getY() > 2 && e.getY() < 10 )
            _darkKillButton = true;
        else {
            _darkKillButton = false;
            Container foo = this.getParent();
            foo.dispatchEvent( e );
        }
    }
    
    @Override
    public void mouseDragged( MouseEvent e ) {
        Container foo = this.getParent();
        foo.dispatchEvent( e );
    }
    
    @Override
    public JToolTip createToolTip() {
        _tip = new ComplexToolTip();
        _tip.setComponent( this );
        _tip.dynamicLinkPath( _dynamicLinkPath );
        return _tip;
    }
    @Override
    public Point getToolTipLocation( MouseEvent e) {
        return new Point( 10, getHeight() );
    }
    
    public ComplexToolTip toolTip() { return _tip; }
    
    /*
     * Allows the user of this widget to set a "dynamic path" for links in the
     * tooltip string.  See the ComplexToolTip class for details.
     */
    public void toolTipDynamicLinkPath( JTextComponent textField ) {
        _dynamicLinkPath = textField;
    }
    
    /*
     * A tooltip-setting function that includes the above.
     */
    public void toolTip( String str, JTextComponent textField ) {
        this.setToolTipText( str );
        this.toolTipDynamicLinkPath( textField );
    }

    ComplexToolTip _tip;
    JTextComponent _dynamicLinkPath;

    static int CENTER = 0;
    static int LEFT = 1;
    static int RIGHT = 2;
    int _justify;
    String _text;
    int _margin;
    boolean _activeKillButton;
    boolean _showKillButton;
    boolean _darkKillButton;
    ActionListener _killListener;
}
